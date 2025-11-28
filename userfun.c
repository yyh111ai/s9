#include "userfun.h"
/**
* @brief	毫秒级的延时
* @note			
* @param	time:延时time毫秒
* @retval	
*/
void delay_ms(uint16_t time)
{    
   uint16_t i=0;  
   while(time--)
   {
      i=6000;  //自己定义
      while(i--) ;    
   }
}
/**
* @brief	计算字符串中特定字符的个数
* @note			
* @param	buff:输入需要处理的字符串
*	@param	ch:需要寻找的特定字符
* @retval	
*/
char strcnt(const char *buff, int ch)
{
	uint16_t i = 0;
	char cnt = 0;
	
	if(NULL == buff)
	{
		return NULL;
	}	
	for(i = 0; buff[i] != 0; i ++)
	{
		if(ch == buff[i])
		{
			cnt ++;
		}
	}
	
	return cnt;
}
