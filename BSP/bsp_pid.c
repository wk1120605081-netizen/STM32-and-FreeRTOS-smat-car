#include "bsp_pid.h"
#include "bsp_motor.h"
#include "bsp_encoder.h"

PID_t PID_Left;
PID_t PID_Right;

void PID_Init(void)
{
    PID_Left.target = PID_Right.target = 20;   /* 默认目标cnt, 标定后改 */

    PID_Left.Kp = PID_Right.Kp = 1.2f;         /* 速度环一般只用PI */
    PID_Left.Ki = PID_Right.Ki = 0.6f;
    PID_Left.Kd = PID_Right.Kd = 0.0f;

    PID_Left.pwm_base = PID_Right.pwm_base = 35;  /* 前馈基础PWM */

    PID_Left.err = PID_Left.err_last = PID_Left.err_prev = 0;
    PID_Right.err = PID_Right.err_last = PID_Right.err_prev = 0;
    PID_Left.out = PID_Right.out = 0;
    PID_Left.pwm = PID_Right.pwm = 0;
}

void PID_SetTarget(int16_t l, int16_t r)
{
    PID_Left.target = l;
    PID_Right.target = r;
}

static void PID_Calc(PID_t *p)
{
    float delta;

    p->err = (float)(p->target - p->feedback);

    /* 增量式PID */
    delta = p->Kp * (p->err - p->err_last)
          + p->Ki *  p->err
          + p->Kd * (p->err - 2.0f * p->err_last + p->err_prev);

    p->out += delta;

    p->err_prev = p->err_last;
    p->err_last = p->err;

    p->pwm = p->pwm_base + (int8_t)(p->out);

    if (p->pwm > 100) p->pwm = 100;
    if (p->pwm < 0)   p->pwm = 0;       /* 前进不倒退 */
}

void PID_Update(void)
{
    Encoder_Update();

    PID_Left.feedback  = Encoder_Left.cnt;
    PID_Right.feedback = Encoder_Right.cnt;

    PID_Calc(&PID_Left);
    PID_Calc(&PID_Right);

    Motor_SetLeft(PID_Left.pwm);
    Motor_SetRight(PID_Right.pwm);
}

void PID_RunFeedback(int16_t lfb, int16_t rfb)
{
    PID_Left.feedback  = lfb;
    PID_Right.feedback = rfb;
    PID_Calc(&PID_Left);
    PID_Calc(&PID_Right);
    Motor_SetLeft(PID_Left.pwm);
    Motor_SetRight(PID_Right.pwm);
}

void PID_Stop(void)
{
    Motor_Stop();
    PID_Left.out = PID_Right.out = 0;
    PID_Left.pwm = PID_Right.pwm = 0;
}
