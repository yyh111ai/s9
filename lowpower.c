#include "lowpower.h"
#include "usart.h"
#include "sys_mode.h"
uint8_t wake_up_sem = 0;//退出低功耗信号量


/**
* @brief	进入低功耗模式
* @note			
* @param	
* @retval	
*/

void Enter_To_Stopmode(void)
{
    // 配置 LPUART1 唤醒
    HAL_UART_DMAStop(&hlpuart1);
    __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_FLAG_WUF | UART_FLAG_IDLE);
    __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_WUF); // 睡前开唤醒中断
    HAL_UARTEx_EnableStopMode(&hlpuart1);

    // 配置 EXTI 唤醒
    __HAL_GPIO_EXTI_CLEAR_IT(PLT_EXTI_Pin);
    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);

    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
    
    // 醒来...
    WakeUp_From_Stopmode();
}
	

/**
* @brief	退出低功耗模式
* @note			
* @param	
* @retval	
*/
void WakeUp_From_Stopmode(void)
{
    HAL_ResumeTick();
    SystemClock_Config();

    // 1. 默认先开调试口 (方便看Log)
    Debug_Mode_ON(); 
    
    // 2. 判断唤醒源
    if (__HAL_UART_GET_FLAG(&hlpuart1, UART_FLAG_WUF))
    {
        // PC 唤醒
        __HAL_UART_CLEAR_FLAG(&hlpuart1, UART_FLAG_WUF);
        g_wakeup_source = 1;
        
        // PC唤醒无需暖机，直接交互或监听
        System_Set_Mode(SYS_MODE_INTERACTIVE);
        Uart_Printf(&hlpuart1, "[Wake] PC\r\n");
    }
    else if (extflag)
    {
        // EXTI (水下) 唤醒
        extflag = 0;
        g_wakeup_source = 2;
        
        // 【关键】进入暖机模式
        // 只开 PLC 电源，不开串口，利用主循环的 Sleep 等待电压稳定
        PLT_Mode_ON(); 
        g_current_mode = SYS_MODE_WARMUP;
        g_state_start_tick = HAL_GetTick();

        Uart_Printf(&hlpuart1, "[Wake] EXTI. Warmup...\r\n");
    }
    
    wake_up_sem = 1;
}
