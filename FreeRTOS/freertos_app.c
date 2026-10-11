/*====================================================================
 *  智能小车 FreeRTOS 多任务版
 *
 *  任务划分:
 *    MotorCtrl  100ms  优先级5  编码器+PID+循迹/避障决策+电机输出
 *    Bluetooth   50ms  优先级4  读蓝牙, 模式切换/遥控
 *    Beep        阻塞  优先级3  等信号量, 收到鸣叫
 *    OLED       200ms  优先级2  收队列刷新显示
 *
 *  通信:
 *    g_uiQueue   : MotorCtrl -> OLED  传显示数据(队列)
 *    g_beepSem   : 模式切换/急停 -> Beep(二值信号量)
 *
 *  蓝牙指令: '1'循迹 '2'避障 '3'遥控; 遥控 'F/B/L/R/S'
 *==================================================================*/
#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"
#include "bsp_pid.h"
#include "bsp_tracking.h"
#include "bsp_ir.h"
#include "bsp_servo.h"
#include "bsp_ultrasonic.h"
#include "bsp_bluetooth.h"
#include "bsp_beep.h"
#include "car_config.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* 显示数据包 */
typedef struct {
    CarMode_t mode;
    int16_t   lc;
    int16_t   rc;
} UiData_t;

static volatile CarMode_t g_mode = MODE_REMOTE;

static QueueHandle_t    g_uiQueue;
static SemaphoreHandle_t g_beepSem;

/*---------------- 遥控 ----------------*/
static void Remote_Handle(char k)
{
    switch (k) {
        case 'F': case 'f': Motor_SetSpeed( RC_FWD,  RC_FWD); break;
        case 'B': case 'b': Motor_SetSpeed(-RC_BACK,-RC_BACK); break;
        case 'L': case 'l': Motor_SetSpeed(-RC_TURN, RC_TURN); break;
        case 'R': case 'r': Motor_SetSpeed( RC_TURN,-RC_TURN); break;
        case 'S': case 's': Motor_Stop(); break;
        default: break;
    }
}

/*---------------- 循迹 ----------------*/
static void Track_Run(void)
{
    uint8_t line = Tracking_GetLine();
    int8_t  e    = Tracking_GetError();
    int16_t lt, rt;

    if (line == 0) { PID_Stop(); return; }

    lt = TRACK_BASE + e * TRACK_TURN;
    rt = TRACK_BASE - e * TRACK_TURN;
    if (lt < 0) lt = 0;
    if (rt < 0) rt = 0;

    PID_SetTarget(lt, rt);
    PID_RunFeedback(Encoder_Left.cnt, Encoder_Right.cnt);
}

/*---------------- 避障(状态机) ----------------*/
typedef enum { AV_CRUISE = 0, AV_DECIDE, AV_TURN, AV_BACK } AvState_t;

static void Avoid_Run(void)
{
    static AvState_t st = AV_CRUISE;
    static int8_t  dir = 0;
    static uint8_t timer = 0;
    float front, dl, dr;
    int16_t lt, rt;

    switch (st) {

    case AV_CRUISE:
        Servo_SetAngle(90);
        front = Ultrasonic_GetCm();

        lt = rt = CRUISE_CNT;
        if (IR_LeftBlocked())  { lt += IR_OFFSET; rt -= IR_OFFSET; }
        if (IR_RightBlocked()) { lt -= IR_OFFSET; rt += IR_OFFSET; }
        if (lt < 0) lt = 0;  if (rt < 0) rt = 0;

        if (front > 0 && front < SAFE_CM) {
            Motor_Stop();
            st = AV_DECIDE;
        } else {
            PID_SetTarget(lt, rt);
            PID_RunFeedback(Encoder_Left.cnt, Encoder_Right.cnt);
        }
        break;

    case AV_DECIDE:
        Servo_SetAngle(SCAN_LEFT);
        vTaskDelay(pdMS_TO_TICKS(350));
        dl = Ultrasonic_GetCm();

        Servo_SetAngle(SCAN_RIGHT);
        vTaskDelay(pdMS_TO_TICKS(350));
        dr = Ultrasonic_GetCm();

        Servo_SetAngle(90);
        vTaskDelay(pdMS_TO_TICKS(250));

        if (dl < 0) dl = 0;
        if (dr < 0) dr = 0;

        if (dl > SAFE_CM && dl >= dr) { dir = -1; timer = TURN_TICKS; st = AV_TURN; }
        else if (dr > SAFE_CM)        { dir =  1; timer = TURN_TICKS; st = AV_TURN; }
        else                          { timer = BACK_TICKS; st = AV_BACK; }
        break;

    case AV_TURN:
        if (dir < 0) Motor_SetSpeed(-TURN_PWM, TURN_PWM);
        else         Motor_SetSpeed( TURN_PWM,-TURN_PWM);
        if (--timer == 0) st = AV_CRUISE;
        break;

    case AV_BACK:
        Motor_SetSpeed(-BACK_PWM, -BACK_PWM);
        if (--timer == 0) st = AV_DECIDE;
        break;
    }
}

/*---------------- 任务: 电机控制 ----------------*/
static void Task_MotorCtrl(void *arg)
{
    UiData_t ui;
    (void)arg;

    while (1) {
        Encoder_Update();

        if      (g_mode == MODE_TRACK) Track_Run();
        else if (g_mode == MODE_AVOID) Avoid_Run();
        /* REMOTE: 由蓝牙任务直接控制 */

        ui.mode = g_mode;
        ui.lc   = Encoder_Left.cnt;   if (ui.lc < 0) ui.lc = -ui.lc;
        ui.rc   = Encoder_Right.cnt;  if (ui.rc < 0) ui.rc = -ui.rc;
        xQueueSend(g_uiQueue, &ui, 0);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/*---------------- 任务: 蓝牙 ----------------*/
static void Task_Bluetooth(void *arg)
{
    char k;
    (void)arg;

    while (1) {
        if (Bluetooth_Available()) {
            k = Bluetooth_Read();

            if (k == '1' || k == '2' || k == '3') {
                PID_Stop();
                Motor_Stop();
                g_mode = (CarMode_t)(k - '1');
                xSemaphoreGive(g_beepSem);
                Bluetooth_SendString("MODE\r\n");
            } else if (g_mode == MODE_REMOTE) {
                Remote_Handle(k);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

/*---------------- 任务: 蜂鸣器 ----------------*/
static void Task_Beep(void *arg)
{
    (void)arg;
    while (1) {
        xSemaphoreTake(g_beepSem, portMAX_DELAY);
        Beep_On();
        vTaskDelay(pdMS_TO_TICKS(80));
        Beep_Off();
    }
}

/*---------------- 任务: OLED ----------------*/
static void Task_OLED(void *arg)
{
    UiData_t ui;
    (void)arg;

    ui.mode = MODE_REMOTE; ui.lc = 0; ui.rc = 0;

    while (1) {
        xQueueReceive(g_uiQueue, &ui, pdMS_TO_TICKS(500));

        OLED_Clear();
        if      (ui.mode == MODE_REMOTE) OLED_ShowString(1, 1, "Remote");
        else if (ui.mode == MODE_TRACK)  OLED_ShowString(1, 1, "Tracking");
        else                             OLED_ShowString(1, 1, "Avoid");

        OLED_ShowString(2, 1, "L:");
        OLED_ShowNum(2, 3, ui.lc, 3);
        OLED_ShowString(3, 1, "R:");
        OLED_ShowNum(3, 3, ui.rc, 3);
        OLED_ShowString(4, 1, "FreeRTOS Run");

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/*---------------- 钩子函数 ----------------*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask; (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    for (;;);
}
void vApplicationMallocFailedHook(void)
{
    taskDISABLE_INTERRUPTS();
    for (;;);
}

/*---------------- 主函数 ----------------*/
int main(void)
{
    /* 硬件初始化(调度器启动前, 可用阻塞延时) */
    Motor_Init();
    Encoder_Init();
    PID_Init();
    Tracking_Init();
    IR_Init();
    Servo_Init();
    Ultrasonic_Init();
    Bluetooth_Init();
    Beep_Init();

    OLED_Init();
    OLED_ShowString(1, 1, "RTOS Boot");
    Servo_SetAngle(90);
    Beep_Blip(80);

    /* 队列与信号量 */
    g_uiQueue = xQueueCreate(2, sizeof(UiData_t));
    g_beepSem = xSemaphoreCreateBinary();

    /* 创建任务 */
    xTaskCreate(Task_MotorCtrl, "Motor",  320, NULL, 5, NULL);
    xTaskCreate(Task_Bluetooth, "Blue",   128, NULL, 4, NULL);
    xTaskCreate(Task_Beep,      "Beep",    64, NULL, 3, NULL);
    xTaskCreate(Task_OLED,      "OLED",   256, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;);   /* 正常不会到这 */
}
