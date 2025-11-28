/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>	
#include "gpio.h"
#include "adc.h"
#include "usart.h"
//#include "icpt_cfg.h"
#include "cmd.h"
#include "dma.h"
#include "sub.h"
#include "ctd.h"
#include "lowpower.h"
#include "userfun.h"
#include "eeprom.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define CMD_RCV_DAT_MAX 512  //PC信息最大内容字符数
#define CMD_NUM 5		        //PC控制信息内容字符数

#define SUB_RCV_DAT_MAX 512  //水上节点信息最大内容字符数
#define SUB_NUM 5		        //水上节点控制信息内容字符数

#define CTD_RCV_QUE_MAX 3		//CTD信息最大缓存数组个数
#define CTD_RCV_DAT_MAX 512  //CTD信息最大内容字符数

#define CMD_PACKET_MAX_LEN 128  // 最新CTD数据包的最大长度
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SENS_EN_Pin GPIO_PIN_4
#define SENS_EN_GPIO_Port GPIOA
#define PLC_EN_Pin GPIO_PIN_0
#define PLC_EN_GPIO_Port GPIOB
#define POWER_TEST_EN_Pin GPIO_PIN_1
#define POWER_TEST_EN_GPIO_Port GPIOB
#define TTL_232_EN_Pin GPIO_PIN_5
#define TTL_232_EN_GPIO_Port GPIOB
#define PLT_EXTI_Pin GPIO_PIN_6
#define PLT_EXTI_GPIO_Port GPIOB
#define PLT_EXTI_EXTI_IRQn EXTI4_15_IRQn
#define LED1_Pin GPIO_PIN_7
#define LED1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
//main.c
extern char ICPTID;
extern uint32_t packet_counter; 
extern int parsed_id; 
//adc.c
extern char vcc1_value[5]; // 存储vcc1值的字符串数组
extern char temp_value[5]; // 存储temp值的字符串数组

//usart.c
extern uint8_t Cmd_Rx_Buffer[CMD_RCV_DAT_MAX];
extern uint16_t Cmd_Rx_Byte;
extern uint16_t cmd_rx_len;
extern uint8_t Sub_Rx_Buffer[SUB_RCV_DAT_MAX];
extern uint16_t Sub_Rx_Byte;  
extern uint16_t sub_rx_len;
extern uint8_t Ctd_Rx_Buffer[CTD_RCV_DAT_MAX];
extern uint8_t Ctd_Rx_Byte;  
extern uint8_t  ctd_rx_len;

//cmd.c
extern char cmd_rcv_que_cnt;
extern char cmd_rcv_dat_cnt;
extern char Cmd_Rcv_Array[CMD_RCV_DAT_MAX];
extern uint8_t cmd_rcv_sem;
extern uint8_t cmd_any_sem;
extern volatile uint8_t g_new_cmd_data_flag;
extern uint32_t packet_counter1; 
//sub.c
extern char Sub_Rcv_Array[SUB_RCV_DAT_MAX];
extern uint8_t sub_rcv_sem;
extern uint8_t sub_any_sem;
void Send_Rate_Test_Packet(void);//速率发包测试函数

//ctd.c
extern char ctd_rcv_que_cnt;
extern char ctd_rcv_dat_cnt;
extern char Ctd_Rcv_Array[CTD_RCV_QUE_MAX][CTD_RCV_DAT_MAX];
extern uint8_t ctd_rcv_sem;
extern uint8_t ctd_any_sem;
extern char Send_FinalTempArray[20];

//lowpower.c
extern uint8_t wake_up_sem;

//stm32l0xx_it.c
extern uint8_t extflag;
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
