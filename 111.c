//#include "sub.h"
//#include "main.h" // 确保可以访问全局变量 sub_rx_len
//#include "rate_monitor.h"

//// 外部全局变量
//extern volatile uint8_t g_rate_test_active; 
//extern volatile uint32_t g_rate_test_delay; 

//// 文件内全局变量
//char Sub_Rcv_Array[SUB_RCV_DAT_MAX] = {0};
//uint8_t sub_rcv_sem = 0;
//uint8_t sub_any_sem = 0;

//// 指令定义
//const char			HSKP_SUB[]		=		"$HSKP";
//const char			RATE_SUB[]		=		"$RATE";
//const char			SAMP_SUB[]		=		"$SAMP";
//const char			SPED_SUB[]		=		"$SPED"; // 速率调节指令
///**
//* @brief	水上节点信息接收函数
//*/
//void Sub_Receive_Fun(const uint8_t *buff)
//{
//	rate_monitor_process_packet(sub_rx_len);
//memset(Sub_Rcv_Array, 0, sizeof(Sub_Rcv_Array));
//	// ?? memcpy ????????????
//	// ????????????????
//	uint16_t len_to_copy = sub_rx_len;
//	if (len_to_copy >= SUB_RCV_DAT_MAX)
//	{
//		len_to_copy = SUB_RCV_DAT_MAX - 1; // ???????? '\0'
//	}
//	memcpy(Sub_Rcv_Array, buff, len_to_copy);
//	
//	// ????????????????
//	Sub_Rcv_Array[len_to_copy] = '\0';

//	Uart_Printf(&hlpuart1, "%s", Sub_Rcv_Array);
//	while(HAL_UART_GetState(&hlpuart1) == HAL_UART_STATE_BUSY_TX); // ??:??????
//		
//	sub_any_sem = 1;
//	sub_rx_len = 0;
////	memset(Sub_Rx_Buffer, 0x00, sizeof(Sub_Rx_Buffer));
//}
////void Sub_Receive_Fun(const uint8_t *buff)
////{
////	// 确保在处理前，目标数组是干净的
//////	memset(Sub_Rcv_Array, 0, sizeof(Sub_Rcv_Array));

////	uint16_t len_to_copy = sub_rx_len;
////	if (len_to_copy >= SUB_RCV_DAT_MAX)
////	{
////		len_to_copy = SUB_RCV_DAT_MAX - 1; 
////	}
////	
////		memcpy(Sub_Rcv_Array, buff, len_to_copy);
////	// ????????????????
////	Sub_Rcv_Array[len_to_copy] = '\0';
////	Uart_Printf(&hlpuart1, "%s", Sub_Rcv_Array);

////   while(HAL_UART_GetState(&hlpuart1) == HAL_UART_STATE_BUSY_TX);
////	memcpy(Sub_Rcv_Array, buff, len_to_copy);
////	
////	sub_any_sem = 1; // 释放信号量，让主循环去解析
////	memset(Sub_Rx_Buffer, 0x00, sizeof(Sub_Rx_Buffer));
////}

///**
//* @brief	水上节点信息处理函数（健壮解析版）
//* @note		此版本能正确处理2逗号或3逗号格式的指令
//*/
//void Sub_Analyze_Fun(char *buff) // buff 指向 Sub_Rcv_Array
//{
//    char *p_start = buff;
//    char *p_comma1, *p_comma2, *p_asterisk;
//    
//    int parsed_id = 0;
//    int data_val = 0;
//    
//    char temp_cmd[10] = {0};
//    char temp_id[5] = {0};
//    char temp_data[20] = {0};

//    // --- 1. 查找关键分隔符 ---
//    p_comma1 = strchr(p_start, ',');
//    if (!p_comma1) return;

//    p_comma2 = strchr(p_comma1 + 1, ',');
//    if (!p_comma2) return;

//    p_asterisk = strchr(p_comma2 + 1, '*');
//    if (!p_asterisk) return;

//    // --- 2. 安全地提取所有部分 ---
//    int cmd_len = p_comma1 - p_start;
//    if (cmd_len > 0 && cmd_len < sizeof(temp_cmd)) {
//        strncpy(temp_cmd, p_start, cmd_len);
//        temp_cmd[cmd_len] = '\0';
//    } else return;

//    int id_len = p_comma2 - (p_comma1 + 1);
//    if (id_len > 0 && id_len < sizeof(temp_id)) {
//        strncpy(temp_id, p_comma1 + 1, id_len);
//        temp_id[id_len] = '\0';
//        parsed_id = atoi(temp_id);
//    } else return;
//    
//    int data_len = p_asterisk - (p_comma2 + 1);
//    if (data_len >= 0 && data_len < sizeof(temp_data)) {
//        strncpy(temp_data, p_comma2 + 1, data_len);
//        temp_data[data_len] = '\0';
//        data_val = atoi(temp_data);
//    } else return;


//    // --- 3. 如果ID匹配，则执行指令 ---
//    if (parsed_id == ICPTID) {
//        // **处理握手指令 ($HSKP)**
//        if (strncmp(temp_cmd, HSKP_SUB, strlen(HSKP_SUB)) == 0) {
//            if (data_val == 1) {
//                // 使用“完美回声”，确保回复长度和内容100%正确
//                HAL_UART_Transmit(&huart4, (uint8_t*)buff, sub_rx_len, HAL_MAX_DELAY);
//            }
//        } 
//        
//        // **处理速率测试指令 ($RATE)**
//        else if (strncmp(temp_cmd, RATE_SUB, strlen(RATE_SUB)) == 0) {
//            if (data_val == 0 || data_val == 1) {
//                g_rate_test_active = data_val;
//                Uart_Printf(&hlpuart1, "Rate Test Active Flag set to: %d\r\n", g_rate_test_active);
//            }
//        }
//				  // **处理速率控制指令 ($SPED)**
//        else if (strncmp(temp_cmd, SPED_SUB, strlen(SPED_SUB)) == 0){
//                // 安全检查：确保延时值在一个合理的范围内 (e.g., 10ms to 2000ms)
//                if (data_val >= 10 && data_val <= 2000) {
//                    g_rate_test_delay = data_val;
//                    Uart_Printf(&hlpuart1, "Rate Test Delay set to: %u ms\r\n", (unsigned int)g_rate_test_delay);
//                }
//            }
//        // **处理参数采集指令 ($SAMP)**
//        else if (strncmp(temp_cmd, SAMP_SUB, strlen(SAMP_SUB)) == 0)
//        {
//            // ... 您可以在此添加SAMP指令的处理逻辑 ...
//        }
//    }

//    // 清空接收缓冲区，为下一次接收做准备
//    memset(Sub_Rcv_Array, 0, sizeof(Sub_Rcv_Array));
//    sub_rx_len = 0;
//}
