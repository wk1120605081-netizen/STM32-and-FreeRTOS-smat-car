/*====================================================================
 *  智能小车 裸机集成版 (状态机: 遥控 / 循迹 / 避障)
 *  主控 STM32F103C8T6 + TB6612 + 编码器 + OLED + HC-SR04 + SG90
 *  + HC-05 + 蜂鸣器 + 左右红外
 *
 *  蓝牙指令:
 *    '1'=循迹  '2'=避障  '3'=遥控
 *    遥控: 'F'前 'B'后 'L'左 'R'右 'S'停
 *
 *  默认上电=遥控且停车(安全), 手机连上蓝牙发指令再动
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

static CarMode_t g_mode = MODE_REMOTE;

/* ---------- 遥控 ---------- */
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

/* ---------- 模式切换 ---------- */
static void SwitchMode(CarMode_t m)
{
    PID_Stop();
    Motor_Stop();
    g_mode = m;
    Beep_Blip(60);
    Bluetooth_SendString("MODE\r\n");
}

/* ---------- 循迹(每100ms) ---------- */
static void Track_Run(void)
{
    uint8_t line = Tracking_GetLine();
    int8_t  e    = Tracking_GetError();
    int16_t lt, rt;

    if (line == 0) {                 /* 全丢线: 停车(安全) */
        PID_Stop();
        Motor_Stop();
        return;
    }

    lt = TRACK_BASE + e * TRACK_TURN;
    rt = TRACK_BASE - e * TRACK_TURN;
    if (lt < 0) lt = 0;
    if (rt < 0) rt = 0;

    PID_SetTarget(lt, rt);
    PID_Update();
}

/* ---------- 避障(每100ms, 状态机) ---------- */
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
            PID_Update();
        }
        break;

    case AV_DECIDE:
        Servo_SetAngle(SCAN_LEFT);
        Delay_ms(350);
        dl = Ultrasonic_GetCm();

        Servo_SetAngle(SCAN_RIGHT);
        Delay_ms(350);
        dr = Ultrasonic_GetCm();

        Servo_SetAngle(90);
        Delay_ms(250);

        if (dl < 0) dl = 0;
        if (dr < 0) dr = 0;

        if (dl > SAFE_CM && dl >= dr) { dir = -1; timer = TURN_TICKS; st = AV_TURN; }
        else if (dr > SAFE_CM)        { dir =  1; timer = TURN_TICKS; st = AV_TURN; }
        else                          { timer = BACK_TICKS; st = AV_BACK; }
        break;

    case AV_TURN:
        if (dir < 0) Motor_SetSpeed(-TURN_PWM, TURN_PWM);   /* 左转 */
        else         Motor_SetSpeed( TURN_PWM,-TURN_PWM);   /* 右转 */
        if (--timer == 0) st = AV_CRUISE;
        break;

    case AV_BACK:
        Motor_SetSpeed(-BACK_PWM, -BACK_PWM);
        if (--timer == 0) st = AV_DECIDE;
        break;
    }
}

/* ---------- OLED 显示(每200ms) ---------- */
static void Show_UI(void)
{
    int16_t lc, rc;

    if (g_mode == MODE_REMOTE) Encoder_Update();

    lc = Encoder_Left.cnt;  if (lc < 0) lc = -lc;
    rc = Encoder_Right.cnt; if (rc < 0) rc = -rc;

    OLED_Clear();
    if (g_mode == MODE_REMOTE)     OLED_ShowString(1, 1, "Remote");
    else if (g_mode == MODE_TRACK) OLED_ShowString(1, 1, "Tracking");
    else                           OLED_ShowString(1, 1, "Avoid");

    OLED_ShowString(2, 1, "L:");
    OLED_ShowNum(2, 3, lc, 3);
    OLED_ShowString(3, 1, "R:");
    OLED_ShowNum(3, 3, rc, 3);
    OLED_ShowString(4, 1, "HC-05 Ready");
}

/* ---------- 主函数 ---------- */
int main(void)
{
    uint8_t div = 0;

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
    OLED_ShowString(1, 1, "SmartCar Boot");
    Beep_Blip(80);

    Servo_SetAngle(90);

    while (1) {

        /* 蓝牙按键(50ms响应) */
        if (Bluetooth_Available()) {
            char k = Bluetooth_Read();
            if      (k == '1') SwitchMode(MODE_TRACK);
            else if (k == '2') SwitchMode(MODE_AVOID);
            else if (k == '3') SwitchMode(MODE_REMOTE);
            else if (g_mode == MODE_REMOTE) Remote_Handle(k);
        }

        div++;

        /* 运动控制 100ms */
        if (div % 2 == 0) {
            if      (g_mode == MODE_TRACK) Track_Run();
            else if (g_mode == MODE_AVOID) Avoid_Run();
        }

        /* 显示 200ms */
        if (div >= 4) {
            div = 0;
            Show_UI();
        }

        Delay_ms(50);
    }
}
