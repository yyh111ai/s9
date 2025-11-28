/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "sys_mode.h"
#include "usart.h"
#include "gpio.h"
#include "dma.h"
#include "sub.h"
#include "cmd.h"
#include "ctd.h" // 假设有传感器处理

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
volatile uint8_t g_rate_test_active = 0;
// **新增：定义一个全局变量来控制发送延时，默认为50ms**
volatile uint32_t g_rate_test_delay = 500; 
volatile uint8_t g_new_cmd_data_flag = 0; // 新数据标志
uint32_t packet_counter = 0; 
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
char ICPTID = 2;
int parsed_id; // sub.c 解析出来的ID
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
//void Send_Rate_Test_Packet(void)
//{
//    char test_packet[65] = {0};
//    static uint32_t packet_counter = 0;

//    const char* header = "[STX]";
//    uint32_t count = packet_counter++;
//    const char* data_payload = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
//    const char* footer = "[ETX]";
//    
//    snprintf(test_packet, sizeof(test_packet),
//             "%-5s %08lu %-45.45s %-5s",
//             header, count, data_payload, footer);
//		const uint16_t packet_len_to_send = 64 + 2;
//		Uart_Printf(&huart4, "%s\r\n", test_packet);
////		Uart_Printf(&hlpuart1, "%s\r\n", test_packet);

////    HAL_UART_Transmit(&huart4, (uint8_t*)test_packet, 64, HAL_MAX_DELAY);
////    rate_monitor_process_tx_packet(packet_len_to_send);
//    // **关键修改：使用全局变量 g_rate_test_delay 来控制延时**
////    HAL_Delay(g_rate_test_delay);
//}
void Send_Rate_Test_Packet(void)
{
    // 增加缓冲区大小以容纳 序号 + CTD数据
    char test_packet[512] = {0}; 
    // 静态变量，用于保存数据包序号
uint16_t len = 0; // 用于存储包的实际长度
    // 检查是否有新的CTD数据
    if (g_new_cmd_data_flag)
    {
        g_new_cmd_data_flag = 0; // 清除新数据标志
        
        // 关键：只有在有新数据时，才增加序号
        packet_counter++; 
			// 格式化: [STX] (8位序号) (状态) (CTD数据) [ETX]
    // 无论是 "DATA" 还是 "STALE" 状态，
    // 都使用 packet_counter 的当前值。
		// 1. 格式化数据包，包含 \r\n
//			snprintf(test_packet, sizeof(test_packet),
//             "%-5s %08lu %-45.45s %-5s",
//             header, count, Cmd_Rcv_Array, footer);
 snprintf(test_packet, sizeof(test_packet),
                   "[STX] %08lu %s [ETX]\r\n", // <-- 关键：在此处添加 \r\n
                   packet_counter,
                   Cmd_Rcv_Array); //
             
    // 2. 检查 snprintf 是否成功
//    if (len > 0 && len < sizeof(test_packet))
//    {//		
 Uart_Printf(&huart4, "%s", test_packet);
        // 3. 调用 Uart_Send_Data 发送
//        Uart_Send_Data(&huart4, (uint8_t*)test_packet, len);
//    }
    }
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
uint32_t last_send_tick = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_LPUART1_UART_Init();
  
  // 2. 初始全关
  Device_Power_OFF_All(); 
  
  Uart_Printf(&hlpuart1, "Boot. Sleep.\r\n");
  HAL_Delay(50);
  
  Goto_Stop_Safe(); // 初始进入 STOP
//  MX_USART1_UART_Init();
//  MX_USART4_UART_Init();
  /* USER CODE BEGIN 2 */

//	__HAL_UART_ENABLE_IT(&huart1,UART_IT_RXNE);  // 开启串口接收中断
//  __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);  // 开启串口空闲中断
//	__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);  // 移除 RXNE 中断
//	  HAL_UART_DeInit(&huart1);
//    HAL_UART_DeInit(&huart4);
//  __HAL_UART_ENABLE_IT(&huart4,UART_IT_IDLE);  // 开启串口空闲中断
//	  __HAL_UART_ENABLE_IT(&hlpuart1,UART_IT_IDLE);  // 开启串口空闲中断
//		    __HAL_GPIO_EXTI_CLEAR_IT(PLT_EXTI_Pin);
//    HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
//    HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);	

//__HAL_RCC_ADC1_CLK_DISABLE(); 

// 关闭未使用的 GPIO 端口时钟 (假设 GPIOC 未使用)
//__HAL_RCC_GPIOC_CLK_DISABLE();

//__HAL_RCC_GPIOB_CLK_DISABLE();
    // 1. 启动 huart1 (PC) 的 DMA 接收
//    if (HAL_UART_Receive_DMA(&huart1, (uint8_t*)Cmd_Rx_Buffer, CMD_RCV_DAT_MAX) != HAL_OK)
//    {
//        Error_Handler();
//    }

    // 2. 启动 huart4 (水下节点) 的 DMA 接收
//    if (HAL_UART_Receive_DMA(&huart4, (uint8_t*)Sub_Rx_Buffer, SUB_RCV_DAT_MAX) != HAL_OK)
//    {
//        Error_Handler();
//    }
//	 wake_up_sem=1;
//	 Uart_Printf(&hlpuart1, "--- Serial Test Program Started ---\r\n");
//	 Uart_Printf(&hlpuart1, "--- LPUART1 Echo is ON ---\r\n");
//		 HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE BEGIN 3 */
    uint32_t current_tick = HAL_GetTick();

    // ========================================================
    // 状态: 暖机 (WARMUP) - 奇妙想法：用 Sleep 代替 Delay
    // ========================================================
    if (g_current_mode == SYS_MODE_WARMUP)
    {
        // 检查是否达到 200ms
        if ((current_tick - g_state_start_tick) >= WARMUP_TIME_MS)
        {
            // 时间到！电源稳了，开启通信接口
            if (g_wakeup_source == 2) { 
                PLT_Comm_Start(); // 初始化 USART4
                System_Set_Mode(SYS_MODE_LISTEN); // 转入监听
            }
            else {
                // 如果是采集任务的暖机
                Sensor_Comm_Start(); // 初始化 USART1
                // 此时 DMA 已经开启接收，状态转为 ACQUIRE 等待数据
                // 但这里需要微调状态机，暂且假设转 LISTEN
            }
        }
        else
        {
            // 时间没到，进浅睡眠 (CPU停，SysTick走)
            Goto_Sleep_Wait(); 
        }
    }

    // ========================================================
    // 状态: 监听/握手 (LISTEN)
    // ========================================================
    else if (g_current_mode == SYS_MODE_LISTEN)
    {
        // 1. 检查水下数据
        if (sub_any_sem)
        {
            sub_any_sem = 0;
            Sub_Analyze_Fun(Sub_Rcv_Array); // 解析 ID
            
            // --- 握手逻辑 ---
            if (parsed_id == ICPTID) {
                Uart_Printf(&hlpuart1, "ID Match! ACK.\r\n");
                // 发送 ACK ...
                // 切换到交互模式
                System_Set_Mode(SYS_MODE_INTERACTIVE);
            } else {
                Uart_Printf(&hlpuart1, "Wrong ID. Sleep.\r\n");
                Goto_Stop_Safe();
            }
        }

        // 2. 超时回睡
        if ((current_tick - g_state_start_tick) > LISTEN_TIMEOUT_MS) {
            Uart_Printf(&hlpuart1, "Handshake Timeout. Sleep.\r\n");
            Goto_Stop_Safe();
        }
        
        // 没数据就浅睡等待
        Goto_Sleep_Wait();
    }

    // ========================================================
    // 状态: 静默采集 (ACQUIRE) - CPU低功耗采集
    // ========================================================
    else if (g_current_mode == SYS_MODE_ACQUIRE)
    {
        // 进入此模式前，Sensor_Mode_ON 已经开启了 DMA 接收
        
        // 1. 检查传感器数据 (由 USART1 IDLE 中断置位 ctd_any_sem)
        if (ctd_any_sem) // 假设 stm32l0xx_it.c 里处理了 USART1
        {
            ctd_any_sem = 0;
            Uart_Printf(&hlpuart1, "Data Recv!\r\n");
            
            // 处理/存储数据...
            // Save_Data(Ctd_Rcv_Array);
            
            // 任务完成，回睡
            Goto_Stop_Safe();
        }
        
        // 2. 超时保护 (比如传感器坏了，一直不发数据)
        if ((current_tick - g_state_start_tick) > 2000) {
            Uart_Printf(&hlpuart1, "Sensor Timeout.\r\n");
            Goto_Stop_Safe();
        }

        // 3. 【关键】等待期间进入 Sleep Mode
        // DMA 依然在后台搬运数据，CPU 睡觉省电
        Goto_Sleep_Wait();
    }

    // ========================================================
    // 状态: 交互 (INTERACTIVE)
    // ========================================================
    else if (g_current_mode == SYS_MODE_INTERACTIVE)
    {
        // 只要有数据就处理
        if (sub_any_sem) {
            sub_any_sem = 0;
            Sub_Analyze_Fun(Sub_Rcv_Array);
        }
        
        if (cmd_any_sem) { // 调试口
            cmd_any_sem = 0;
            Cmd_Analyze_Fun(Cmd_Rcv_Array);
        }
        
        // 交互模式下通常使用 Sleep 等待，响应中断唤醒
        Goto_Sleep_Wait();
    }
    
    // 异常处理
    else 
    {
        Goto_Stop_Safe();
    }
    
    /* USER CODE END 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
	__HAL_RCC_GPIOB_CLK_ENABLE(); 
  // 设置 LED1 引脚为输出
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);
  
  // 强制点亮 LED
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
  while (1)
  {

      // 翻转 LED1 (假设 LED1_Pin 和 LED1_GPIO_Port 已在 main.h 中定义)
 
  
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
