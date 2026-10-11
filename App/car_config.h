#ifndef __CAR_CONFIG_H
#define __CAR_CONFIG_H

/* ===== 全车可调参数(标定后改这里) ===== */

/* 编码器: 码盘缝数在 bsp_encoder.h 的 ENCODER_LINES */

/* PID 速度环参数在 bsp_pid.c PID_Init */

/* 循迹 */
#define TRACK_BASE     18      /* 直行目标 cnt(100ms脉冲), 慢一点好调 */
#define TRACK_TURN     8       /* 弯道差速强度 */

/* 避障 */
#define SAFE_CM        20.0f   /* 前方安全距离 */
#define CRUISE_CNT     20      /* 巡航目标 cnt */
#define TURN_PWM       45      /* 原地/绕弯转向PWM */
#define BACK_PWM       35      /* 后退PWM */
#define IR_OFFSET      10      /* 红外盲区触发时的差速 */
#define SCAN_LEFT      40      /* 舵机看左角度(反了对调) */
#define SCAN_RIGHT     140     /* 舵机看右角度 */
#define TURN_TICKS     8       /* 转向持续(×100ms) */
#define BACK_TICKS     4

/* 遥控速度(开环PWM) */
#define RC_FWD         45
#define RC_BACK        35
#define RC_TURN        40

/* 模式 */
typedef enum { MODE_REMOTE = 0, MODE_TRACK = 1, MODE_AVOID = 2 } CarMode_t;

#endif
