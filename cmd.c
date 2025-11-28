/* Includes ------------------------------------------------------------------*/
#include "cmd.h"

char cmd_rcv_que_cnt = 0;
char cmd_rcv_dat_cnt = 0;
char Cmd_Rcv_Array[CMD_RCV_DAT_MAX] = {0};
uint8_t cmd_rcv_id = 0;
uint8_t cmd_rcv_sem = 0;		//PC接收完成信号量
uint8_t cmd_any_sem = 0;		//PC分析完成信号量
uint32_t packet_counter1 = 0; 
char *Cmd_Rcv_CMD = NULL;
char *Cmd_Rcv_Data = NULL;
extern volatile uint8_t g_rate_test_active; 
extern volatile uint32_t g_rate_test_delay; 
static uint16_t s_analysis_len = 0;
const char			STOP_CMD[]		=		"STOP";		//系统唤醒
const char			HSKP1_SUB[]		=		"HSKP"; // 握手指令 (注意：去掉了$)
const char			RATE1_SUB[]		=		"RATE"; // 速率测试开关指令
const char			SPED1_SUB[]		=		"SPED"; // 速率调节指令
const char			SLEP1_SUB[]		=		"SLEP"; // 休眠指令
/**
* @brief	串口信息接收函数，判断接收信息格式是否正确		
* @note			
* @param	buff:串口接收到的字符串
* @retval	
*/
void Cmd_Receive_Fun(const uint8_t *buff)
{
memset(Cmd_Rcv_Array, 0, sizeof(Cmd_Rcv_Array));
	 s_analysis_len = cmd_rx_len;
	    if (g_rate_test_active)
    {
				packet_counter1++;
			 Uart_Printf(&hlpuart1, "%d\r\n", packet_counter1);
			}
		Uart_Printf(&hlpuart1, "Data forwarded via huart4 (%d bytes)\r\n", s_analysis_len);
//			Uart_Printf(&hlpuart1, "Data forwarded via hlpuart1 (%d bytes)\r\n", s_analysis_len);
    memcpy(Cmd_Rcv_Array, buff, s_analysis_len);
//		Uart_Send_Data(&huart4, (uint8_t*)Cmd_Rcv_Array, s_analysis_len);
	memset(Cmd_Rx_Buffer, 0x00, CMD_RCV_DAT_MAX);
	g_new_cmd_data_flag=1;
	cmd_any_sem = 1; // 设置分析信号量，通知main循环可以开始解析
}

/**
* @brief	水上节点指令解析与执行函数（最终修正版）
* @note		使用 sscanf 进行健壮的格式化字符串解析，能正确处理2逗号指令
* @param	buff: 指向包含完整指令的处理缓冲区 (Sub_Rcv_Array)
*/
void Cmd_Analyze_Fun(char *buff)
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
            if (strcmp(temp_cmd, HSKP1_SUB) == 0) {
                if (data_val == 1) {
                    // 使用“完美回声”机制：将收到的原始数据包原封不动地发回去
//										Uart_Printf(&huart4, "%s", sub_rx_len);
//                    HAL_UART_Transmit(&huart4, (uint8_t*)buff, sub_rx_len, HAL_MAX_DELAY);
//                    Uart_Printf(&hlpuart1, "%s", g_rate_test_active);
									    HAL_GPIO_WritePin(PLC_EN_GPIO_Port, PLC_EN_Pin, GPIO_PIN_RESET);
//									__WFI();
									Uart_Printf(&hlpuart1, "Test Active: %d\r\n", g_rate_test_active);
                }
            } 
            
            // **处理速率测试开关指令 ($RATE)**
            else if (strcmp(temp_cmd, RATE1_SUB) == 0) {
                if (data_val == 0 || data_val == 1) {
                    g_rate_test_active = data_val;
                    Uart_Printf(&hlpuart1, "Rate Test Active Flag set to: %d\r\n", g_rate_test_active);
										g_new_cmd_data_flag=1;
                }
            }
            
            // **处理速率控制指令 ($SPED)**
            else if (strcmp(temp_cmd, SPED1_SUB) == 0) {
                // 安全检查：确保延时值在一个合理的范围内 (e.g., 10ms to 2000ms)
                if (data_val >= 10 && data_val <= 2000) {
                    g_rate_test_delay = data_val;
                    Uart_Printf(&hlpuart1, "Rate Test Delay set to: %u ms\r\n", (unsigned int)g_rate_test_delay);
                }
            }
						else if (strcmp(temp_cmd, SLEP1_SUB) == 0) {
                if (data_val == 1) {
                    Uart_Printf(&hlpuart1, "Received SLEEP command. Entering STOP mode.\r\n");
//                    HAL_Delay(500); // 延时一小段时间，确保打印信息发送完毕
                    Enter_To_Stopmode(); // 调用进入低功耗函数
                }
            }
        }
    }

}

