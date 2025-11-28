#include "ctd.h"
char ctd_rcv_que_cnt = 0;
char ctd_rcv_dat_cnt = 0;
char Ctd_Rcv_Array[CTD_RCV_QUE_MAX][CTD_RCV_DAT_MAX] = {0};
uint8_t ctd_rcv_sem = 0;		//接收CTD信息完成信号量
uint8_t ctd_any_sem = 0;		//分析CTD信息完成信号量
char Send_FinalTempArray[20] = {0};


/**
* @brief	CTD信息接收函数，判断接收信息格式是否正确		
* @note			
* @param	buff:串口接收到的字符串
* @retval	
*/
void Ctd_Receive_Fun(const uint8_t *buff)
{
	uint16_t  buff_cnt = 0;
	for(buff_cnt = 0; buff_cnt < ctd_rx_len; buff_cnt ++)
		{
			Ctd_Rcv_Array[ctd_rcv_que_cnt][ctd_rcv_dat_cnt] = buff[buff_cnt];
			ctd_rcv_dat_cnt = ctd_rcv_dat_cnt + 1;
		}
	Ctd_Rcv_Array[ctd_rcv_que_cnt][ctd_rcv_dat_cnt] = 0;
	ctd_rcv_dat_cnt = 0;
/*****************************************调试*****************************************/
//	Uart_Printf(&huart1,"SEND len: %d", ctd_rx_len);		
/**************************************************************************************/		
	if(ctd_rcv_que_cnt < CTD_RCV_QUE_MAX)
	{
		ctd_rcv_que_cnt = ctd_rcv_que_cnt + 1;
	}
	if(CTD_RCV_QUE_MAX == ctd_rcv_que_cnt)
	{
		ctd_rcv_que_cnt = 0;
	}
	while(HAL_UART_GetState(&huart1) == HAL_UART_STATE_BUSY_TX);//检测UART发送结束			
	ctd_any_sem = 1;		//释放信号量
}

///**
//* @brief	CTD信息处理函数，判断接受信息内容
//* @note			
//* @param	buff:UART1串口接收函数输出的字符串
//* @retval	
//*/
//void Ctd_Analyze_Fun(char *buff)
//{
//	HAL_UART_Transmit(&huart4, (uint8_t*)buff, ctd_rx_len, HAL_MAX_DELAY);
//	ctd_rx_len = 0;
//	delay_ms(PLTDELAY);
//}
