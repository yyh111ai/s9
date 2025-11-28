#include "sys_mode.h"
#include "usart.h"
#include "gpio.h"
#include "lowpower.h"

volatile SystemMode_t g_current_mode = SYS_MODE_STOP;
volatile uint32_t g_state_start_tick = 0;
volatile uint8_t g_wakeup_source = 0;

// ==========================================
// 硬件控制 (原子操作)
// ==========================================

void Debug_Mode_ON(void)
{
    MX_LPUART1_UART_Init();
    __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_FLAG_IDLE);
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE); // 醒来后必须开IDLE
    HAL_UART_Receive_DMA(&hlpuart1, (uint8_t*)Cmd_Rx_Buffer, CMD_RCV_DAT_MAX);
}

void Debug_Mode_OFF(void)
{
    HAL_UART_DMAStop(&hlpuart1);
    HAL_UART_DeInit(&hlpuart1);
    // LPUART1 引脚通常为了唤醒不设为模拟，或者由Enter_Stopmode专门处理
}

void PLT_Mode_ON(void)
{
    // 只开电源，串口初始化留给状态机延时后来做
    HAL_GPIO_WritePin(PLC_EN_GPIO_Port, PLC_EN_Pin, GPIO_PIN_SET);
}

void PLT_Mode_OFF(void)
{
    HAL_UART_DMAStop(&huart4);
    HAL_UART_DeInit(&huart4);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1; 
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(PLC_EN_GPIO_Port, PLC_EN_Pin, GPIO_PIN_RESET);
}

void Sensor_Mode_ON(void)
{
    // 只开电源 (含TTL)，串口初始化延时后再做
    HAL_GPIO_WritePin(SENS_EN_GPIO_Port, SENS_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(TTL_232_EN_GPIO_Port, TTL_232_EN_Pin, GPIO_PIN_SET);
}

void Sensor_Mode_OFF(void)
{
    HAL_UART_DMAStop(&huart1);
    HAL_UART_DeInit(&huart1);
    
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10; 
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_GPIO_WritePin(SENS_EN_GPIO_Port, SENS_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TTL_232_EN_GPIO_Port, TTL_232_EN_Pin, GPIO_PIN_RESET);
}

// ==========================================
// 系统级控制
// ==========================================

void Device_Power_OFF_All(void)
{
    PLT_Mode_OFF();
    Sensor_Mode_OFF();
    HAL_GPIO_WritePin(POWER_TEST_EN_GPIO_Port, POWER_TEST_EN_Pin, GPIO_PIN_RESET);
}

// 开启 USART4 (PLC) 的通信功能 (上电延时后调用)
void PLT_Comm_Start(void)
{
    MX_USART4_UART_Init();
    __HAL_UART_CLEAR_FLAG(&huart4, UART_FLAG_IDLE);
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
    HAL_UART_Receive_DMA(&huart4, (uint8_t*)Sub_Rx_Buffer, SUB_RCV_DAT_MAX);
}

// 开启 USART1 (传感器) 的通信功能 (上电延时后调用)
void Sensor_Comm_Start(void)
{
    MX_USART1_UART_Init();
    __HAL_UART_CLEAR_FLAG(&huart1, UART_FLAG_IDLE);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE); // 【关键】开启 IDLE 中断
    
    // 【关键】开启 DMA 接收，CPU 就可以去睡了
    HAL_UART_Receive_DMA(&huart1, (uint8_t*)Ctd_Rx_Buffer, CTD_RCV_DAT_MAX); 
}

void System_Set_Mode(SystemMode_t next_mode)
{
    if (g_current_mode == next_mode && next_mode != SYS_MODE_STOP) return;
    
    if(hlpuart1.gState != HAL_UART_STATE_RESET)
        Uart_Printf(&hlpuart1, "[SYS] %d->%d\r\n", g_current_mode, next_mode);

    switch (next_mode)
    {
        case SYS_MODE_STOP:
            Device_Power_OFF_All();
            break;
            
        case SYS_MODE_WARMUP:
            if (g_wakeup_source == 2) PLT_Mode_ON(); 
            break;

        case SYS_MODE_ACQUIRE:
            PLT_Mode_OFF();
            Sensor_Mode_ON(); 
            break;
            
        case SYS_MODE_LISTEN:
            // 保持现状
            break;

        case SYS_MODE_HANDSHAKE:
            // 【新增】握手模式：必须开启 PLC 电源和串口，准备疯狂发送
            PLT_Mode_ON();
            // 确保串口接收也开着 (PLT_Mode_ON 里已经开了)
            break;
            
        case SYS_MODE_INTERACTIVE:
            PLT_Mode_ON(); 
            break;
    }
    
    g_current_mode = next_mode;
    g_state_start_tick = HAL_GetTick(); // 重置状态进入时间
}

void Goto_Stop_Safe(void)
{
    System_Set_Mode(SYS_MODE_STOP); 
    Enter_To_Stopmode();            
}

void Goto_Sleep_Wait(void)
{
    // 进入浅睡眠 (Sleep Mode)，外设(DMA/UART)继续运行
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
}
