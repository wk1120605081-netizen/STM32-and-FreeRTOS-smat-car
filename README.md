# STM32F103C8T6 + FreeRTOS 多功能智能小车

基于 STM32 标准库与 FreeRTOS 的两轮差速智能小车，实现蓝牙遥控、五路红外循迹、超声波避障、编码器速度闭环与 OLED 状态显示。

## ✨ 功能
- **蓝牙遥控**：HC-05 经典蓝牙，手机串口 APP 控制前进/后退/转向/停车
- **五路红外循迹**：加权误差 + 差速转向，全丢线自动停车
- **超声波避障**：SG90 舵机左右扫描前方障碍，红外盲区辅助，状态机决策
- **速度闭环**：槽型光电编码器 + 增量式 PID，左右轮独立定速
- **OLED 显示**：实时显示当前模式、左右轮速、蓝牙状态
- **FreeRTOS 多任务**：电机控制、蓝牙、蜂鸣、显示并行调度

## 🔧 硬件清单
| 模块 | 型号 | 说明 |
|---|---|---|
| 主控 | STM32F103C8T6 | Cortex-M3 @72MHz |
| 电机驱动 | TB6612FNG | 双路 H 桥 |
| 电机 | TT 减速电机 ×2 | 带码盘 |
| 编码器 | 槽型光电 ×2 | TIM2/TIM4 外部计数 |
| 循迹 | 五路红外 | HW-871 |
| 避障 | HC-SR04 + SG90 | 超声波+舵机扫描 |
| 红外 | TCRT5000 ×2 | 左右盲区 |
| 蓝牙 | HC-05 | SPP 透传 |
| 显示 | 0.96" OLED 128×64 | I2C 软件模拟 |
| 其他 | 有源蜂鸣器 | 模式提示 |

## 🧩 FreeRTOS 任务划分
| 任务 | 周期 | 优先级 | 功能 |
|---|---|---|---|
| MotorCtrl | 100ms | 5 | 编码器采集 + PID + 循迹/避障决策 + 电机输出 |
| Bluetooth | 50ms | 4 | 读蓝牙，模式切换 / 遥控 |
| Beep | 阻塞等待 | 3 | 拿到信号量后鸣叫提示 |
| OLED | 200ms | 2 | 从队列取数据刷新显示 |

任务间通信：
- `g_uiQueue`（队列）：MotorCtrl → OLED，传递显示数据
- `g_beepSem`（二值信号量）：模式切换/急停 → Beep

## 📡 蓝牙指令
- 模式切换：`1`=循迹  `2`=避障  `3`=遥控
- 遥控动作：`F`前 `B`后 `L`左 `R`右 `S`停

## 🛠 开发环境
- Keil μVision5 + ST-Link V2（SWD 下载）
- STM32 标准库 StdPeriph
- FreeRTOS V10.4.6（ARM_CM3 移植，heap_4）
- 启动文件 `startup_stm32f10x_md.s`

## 📁 目录结构
```
SmartCar/
├── User/            # main、中断、配置
├── BSP/             # 各模块驱动（motor/encoder/pid/tracking/ir/
│                    #   servo/ultrasonic/bluetooth/beep/OLED）
├── App/             # 裸机集成 main.c、参数配置
├── FreeRTOS/        # 内核源码 + freertos_app.c + FreeRTOSConfig.h
└── Doc/             # 接线表、说明文档
```

> 注：仓库中包含裸机版（App/main.c）与 FreeRTOS 版（FreeRTOS/freertos_app.c）两套，二选一加入工程即可。
