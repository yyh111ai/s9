#include "main.h"
#define CMD_ERROR(fmt,arg...)          Uart_Printf(&hlpuart1,"<<-CMD-ERROR->> "fmt"\r\n",##arg);
void Cmd_Receive_Fun(const uint8_t *buff);
void Cmd_Analyze_Fun(char *buff);
