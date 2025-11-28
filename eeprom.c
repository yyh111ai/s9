#include "eeprom.h"     
#include <string.h>
#include "stdio.h"
#include "usart.h"

/*-------------------------------------------------*/
/*函数名：内部eeprom擦除功能                        */
/*参  数：Address：擦除地址                         */
/*参  数：wData：擦除数据缓冲区                     */
/*参  数：len：擦除数据总长                         */
/*返回值：无                                       */
/*----------------------------------------------- -*/
HAL_StatusTypeDef EEPROM_EraseData(uint32_t start, uint32_t NumberSectors)
{
	uint32_t i;
	uint32_t NbrOfPages = 0;
	uint32_t Address = start;
	HAL_StatusTypeDef status = HAL_OK;
	
	//Uart_Printf(&hlpuart1,"EEPROM_EraseData len:%d\r\n", NumberSectors);
	
	NbrOfPages = (DATA_EEPROM_END_ADDR - Address)/FLASH_PAGE_SIZE;
	if(NumberSectors > NbrOfPages)	return (HAL_ERROR);
	
	for(i=0; i<NumberSectors; i++){                   //for循环，需要写入多少数据，就循环几次
		status = HAL_FLASHEx_DATAEEPROM_Erase(Address + i * 4);
	}
	if (status != HAL_OK)
	{
		//Uart_Printf(&hlpuart1,"Erase Fail!\r\n");     //串口提示写入错误
		return (HAL_ERROR);
	}	
	//Uart_Printf(&hlpuart1,"Erase Success.\r\n\r\n");
	return HAL_OK;
}
 
/*-------------------------------------------------*/
/*函数名：内部eeprom写功能                          */
/*参  数：Address：写入地址                        */
/*参  数：wData：写入数据缓冲区                     */
/*参  数：len：写入数据总长                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
HAL_StatusTypeDef EEPROM_WriteData(uint32_t Address, uint32_t *wData, uint32_t len)
{		
	uint32_t i;
 
	HAL_FLASHEx_DATAEEPROM_Unlock();			//解锁 
	
	EEPROM_EraseData(Address, len);
	
	//Uart_Printf(&hlpuart1,"Write Address:%d\r\n", Address);
	
	//Uart_Printf(&hlpuart1,"Write Data:\r\n");
	for(i=0; i<len; i++)							//for循环，需要写入多少数据，就循环几次
	{
		//Uart_Printf(&hlpuart1,"wData[%d]=%08x\r\n",i,wData[i]);
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD, Address, *(uint32_t*)(wData+i)) == HAL_OK)	//调用写数据函数，如果返回的不是FLASH_COMPLETE，表示写入出错，进入if
		{
			if (*(uint32_t*)Address != *(uint32_t*)(wData+i))
      {
				//Uart_Printf(&hlpuart1,"Write Error!!!\r\n");     //串口提示写入错误
    	  HAL_FLASHEx_DATAEEPROM_Lock();
        /* FLASHEx_DATAEEPROM content doesn't match SRAM content */
        return(HAL_ERROR);
      }
			Address += 4;									  //地址递增4，因为一次写一个字，是4个字节
		}
	}
	//Uart_Printf(&hlpuart1,"Write Success.\r\n\r\n"); 
	
	HAL_FLASHEx_DATAEEPROM_Lock();         //上锁
	return HAL_OK;
}
 
/*-------------------------------------------------*/
/*函数名：内部eeprom读功能                        */
/*参  数：Address：读取地址                        */
/*参  数：rData：保存数据缓冲区                     */
/*参  数：len：读取数据总长                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
HAL_StatusTypeDef EEPROM_ReadData(uint32_t Address, uint32_t *rData, uint32_t len)
{		
	uint32_t i;	
	uint32_t *wAddr = 0;
	
	wAddr = (uint32_t *)(Address);   					
	//Uart_Printf(&hlpuart1,"Read Address:%d\r\n", Address);
	
	for(i=0;i<len;i++){                                     //for循环，需要读取多少数据，就循环几次                      
		*rData++ = *wAddr++;                                  //每次读取的数据保存到rData缓冲区	
	}
	//Uart_Printf(&hlpuart1,"Read Complete.\r\n");
	return HAL_OK;
}
 
 
/*-------------------------------------------------*/
/*函数名：带有校验操作的内部eeprom写功能            */
/*参  数：Address：写入地址                        */
/*参  数：wData：写入数据缓冲区                     */
/*参  数：len：写入数据总长                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
HAL_StatusTypeDef EEPROM_WRITE_Verify_CHECK(uint32_t Address, uint32_t *wData, uint32_t len)
{		
	uint32_t buff[len];
	uint32_t i;
	for (i=0; i < iEEPROM_CHECK_NUM; i++)
	{
		EEPROM_WriteData(Address, wData, len);
		EEPROM_ReadData(Address, buff, len);
		if (memcmp(wData, buff, len)==0)
		{
			//Uart_Printf(&hlpuart1,"\r\nWRITE_Verify Completing Comparative\r\n\r\n"); 
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}
 
/*-------------------------------------------------*/
/*函数名：带有校验操作的内部eeprom读功能            */
/*参  数：Address：读取地址                        */
/*参  数：rData：保存数据缓冲区                     */
/*参  数：len：读取数据总长                         */
/*返回值：无                                       */
/*-------------------------------------------------*/
HAL_StatusTypeDef EEPROM_Read_Verify_CHECK(uint32_t Address, uint32_t *rData, uint32_t len)
{		
  uint32_t buff0[len];
	uint32_t buff1[len];
	uint8_t i,j;
	
	for (i=0; i<iEEPROM_CHECK_NUM; i++)
	{
		//Uart_Printf(&hlpuart1,"First  read Verify\r\n");
		EEPROM_ReadData(Address, buff0, len);
		//Uart_Printf(&hlpuart1,"Second read Verify\r\n"); 
		EEPROM_ReadData(Address, buff1, len);
 
		if (memcmp(buff0, buff1, len)==0)
		{
			//Uart_Printf(&hlpuart1,"Read_Verify Completing Comparative\r\n"); 
			for (j=0; j<len; j++)
			{
				*rData++ = buff0[j];
			}
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}

void write_eeprom(uint32_t write_data){
	EEPROM_WriteData(USER_DATA_EEPROM_ADDR, &write_data, 1);
	HAL_Delay(200);	                 				 //延时
}

uint32_t read_eeprom(void){
	uint32_t read_data    = 0x00000000;
	EEPROM_ReadData(USER_DATA_EEPROM_ADDR, &read_data, 1);
	return read_data;
}
