#include "sub.h"
#include "main.h" 
#include "rate_monitor.h"
#include "usart.h"
// --- 引用在 main.c 中定义的全局变量 ---
extern volatile uint8_t g_rate_test_active; 
extern volatile uint32_t g_rate_test_delay; 
static uint16_t s_analysis_len = 0;
// --- 文件内全局变量 ---
char Sub_Rcv_Array[SUB_RCV_DAT_MAX] = {0}; // 用于存放从中断接收函数传递过来的完整数据包
uint8_t sub_rcv_sem = 0;		// 接收完成信号量
uint8_t sub_any_sem = 0;		// 分析完成信号量

// --- 指令字符串定义 ---
const char			HSKP_SUB[]		=		"HSKP"; // 握手指令 (注意：去掉了$)
const char			RATE_SUB[]		=		"RATE"; // 速率测试开关指令
const char			SPED_SUB[]		=		"SPED"; // 速率调节指令
const char			SLEP_SUB[]		=		"SLEP"; // 休眠指令
/**
* @brief	水上节点信息接收函数
* @note		将中断接收到的数据安全地转移到处理缓冲区
* @param	buff: 指向原始的UART接收缓冲区 (Sub_Rx_Buffer)
*/
void Sub_Receive_Fun(const uint8_t *buff)
{
//		Uart_Printf(&hlpuart1, "ok\r\n");
	memset(Sub_Rcv_Array, 0, sizeof(Sub_Rcv_Array));
//  rate_monitor_process_rx_packet(sub_rx_len);
	s_analysis_len = sub_rx_len;

//	
//	if (len_to_copy >= SUB_RCV_DAT_MAX)
//	{
//		len_to_copy = SUB_RCV_DAT_MAX - 1; 
//	}
	memcpy(Sub_Rcv_Array, buff, s_analysis_len);

	Uart_Printf(&hlpuart1, "%s\r\n", Sub_Rcv_Array);
//	while(HAL_UART_GetState(&hlpuart1) == HAL_UART_STATE_BUSY_TX);
//		memset(Cmd_Rx_Buffer, 0x00, CMD_RCV_DAT_MAX);
	sub_any_sem = 1; // 设置分析信号量，通知main循环可以开始解析
	memset(Sub_Rx_Buffer, 0x00, SUB_RCV_DAT_MAX);
}

/**
* @brief	水上节点指令解析与执行函数（最终修正版）
* @note		使用 sscanf 进行健壮的格式化字符串解析，能正确处理2逗号指令
* @param	buff: 指向包含完整指令的处理缓冲区 (Sub_Rcv_Array)
*/
void Sub_Analyze_Fun(char *buff)
{
    int parsed_id = 0;
    int data_val = 0;
    char temp_cmd[10] = {0};

    // **关键修复**: 使用能正确解析2逗号指令 "$CMD,ID,DATA,*" 的sscanf格式
    int items_matched = sscanf(buff, "$%[^,],%d,%d,*", temp_cmd, &parsed_id, &data_val);

    // 只有当指令、ID和数据都成功解析时 (匹配到3个项目)，才继续处理
    if (items_matched == 3)
    {
        // 检查指令中的ID是否与本节点的ID匹配
        if (parsed_id == ICPTID) {
            
            // --- 指令判断 ---

            // **处理握手指令 ($HSKP)**
            if (strcmp(temp_cmd, HSKP_SUB) == 0) {
                if (data_val == 1) {
                    // 使用“完美回声”机制：将收到的原始数据包原封不动地发回去
										Uart_Send_Data(&huart4, (uint8_t*)Sub_Rcv_Array, s_analysis_len);
//                    HAL_UART_Transmit(&huart4, (uint8_t*)buff, sub_rx_len, HAL_MAX_DELAY);
//                    Uart_Printf(&hlpuart1, "%s", g_rate_test_active);
//									Uart_Printf(&hlpuart1, "Test Active: %d\r\n", g_rate_test_active);
                }
            } 
            
            // **处理速率测试开关指令 ($RATE)**
            else if (strcmp(temp_cmd, RATE_SUB) == 0) {
                if (data_val == 0 || data_val == 1) {
                    g_rate_test_active = data_val;
									packet_counter=0;
									packet_counter1=0;
                    Uart_Printf(&hlpuart1, "Rate Test Active Flag set to: %d\r\n", g_rate_test_active);
                }
            }
            
            // **处理速率控制指令 ($SPED)**
            else if (strcmp(temp_cmd, SPED_SUB) == 0) {
                // 安全检查：确保延时值在一个合理的范围内 (e.g., 10ms to 2000ms)
                if (data_val >= 10 && data_val <= 2000) {
                    g_rate_test_delay = data_val;
                    Uart_Printf(&hlpuart1, "Rate Test Delay set to: %u ms\r\n", (unsigned int)g_rate_test_delay);
                }
            }
						else if (strcmp(temp_cmd, SLEP_SUB) == 0) {
                if (data_val == 1) {
//                    Uart_Printf(&hlpuart1, "Entering STOP mode.\r\n");
//                    HAL_Delay(1000); // 延时一小段时间，确保打印信息发送完毕
                    Enter_To_Stopmode(); // 调用进入低功耗函数
                }
            }
        }
    }

}
