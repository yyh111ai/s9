#ifndef __SYS_MODE_H__
#define __SYS_MODE_H__

#include "main.h" 

// --- 系统模式 ---
typedef enum {
    SYS_MODE_STOP = 0,
    SYS_MODE_WARMUP,
    SYS_MODE_LISTEN,         // 第一步：被唤醒，听第一句指令
    SYS_MODE_HANDSHAKE,      // 【新增】第二步：持续发送应答，等待确认
    SYS_MODE_ACQUIRE,        
    SYS_MODE_INTERACTIVE     // 第三步：连接成功
} SystemMode_t;

extern volatile SystemMode_t g_current_mode;
extern volatile uint32_t g_state_start_tick; // 状态进入时间(用于超时/延时)
extern volatile uint8_t g_wakeup_source;     // 1:PC, 2:EXTI
//extern volatile uint8_t wake_up_sem;

// --- 参数配置 ---
#define WARMUP_TIME_MS      200  // 上电稳定时间
#define LISTEN_TIMEOUT_MS   2000 // 握手超时时间
// 【优化】急速握手：每 100ms 发送一次 ACK
// 这样水上节点最快能在 100ms 内收到响应
#define HANDSHAKE_TX_INTERVAL_MS  100 

// 握手总超时 (尝试 5秒 还没通就放弃，太长了也浪费电)
#define HANDSHAKE_TIMEOUT_MS      5000

// --- 接口 ---
void System_Set_Mode(SystemMode_t next_mode);
void Goto_Stop_Safe(void);   // 进深度睡眠 (STOP)
void Goto_Sleep_Wait(void);  // 进浅睡眠 (SLEEP) - 用于等待DMA或延时

// 硬件控制
void Device_Power_OFF_All(void);
void PLT_Mode_ON(void);
void PLT_Mode_OFF(void);
void Sensor_Mode_ON(void);
void Sensor_Mode_OFF(void);
void Debug_Mode_ON(void);
void Debug_Mode_OFF(void);

#endif /* __SYS_MODE_H__ */
