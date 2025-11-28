#ifndef __EEPROM_H
#define __EEPROM_H

#include "main.h"  //包含需要的头文件
 
#define DATA_EEPROM_START_ADDR     0x08080C00   //起始地址
#define USER_DATA_EEPROM_ADDR			 DATA_EEPROM_START_ADDR + 0x00000000		//用户地址
#define DATA_EEPROM_BYTE_SIZE			 0xC00				//空间
#define DATA_EEPROM_END_ADDR     	 DATA_EEPROM_START_ADDR + DATA_EEPROM_BYTE_SIZE  //结束地址
#define iEEPROM_CHECK_NUM 				 2
 
 
HAL_StatusTypeDef EEPROM_WriteData(uint32_t Address, uint32_t *wData, uint32_t len); 
HAL_StatusTypeDef EEPROM_ReadData(uint32_t Address, uint32_t *rData, uint32_t len);
 
HAL_StatusTypeDef EEPROM_WRITE_Verify_CHECK(uint32_t Address, uint32_t *wData, uint32_t len);
HAL_StatusTypeDef EEPROM_Read_Verify_CHECK(uint32_t Address, uint32_t *rData, uint32_t len);

void write_eeprom(uint32_t write_data);
uint32_t read_eeprom(void);

#endif 
