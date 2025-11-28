#include "main.h"
#define SUB_ERROR(fmt,arg...)          Uart_Printf(&huart4,"<<-CMD-ERROR->> "fmt"\r\n",##arg);
void Sub_Receive_Fun(const uint8_t *buff);
void Sub_Analyze_Fun(char *buff);

