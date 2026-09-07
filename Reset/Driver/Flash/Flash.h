#ifndef __FLASH_H__
#define __FLASH_H__

#include "stm32f1xx_hal.h"

/**
 * @brief Flash操作接口
 *
 * 注意：STM32F103 Flash写入必须是半字(16位)对齐
 * - 地址必须是2的倍数
 * - 数据长度必须是2的倍数
 * - 调用者需保证对齐，否则返回错误
 */

void Flash_Unlock(void);
void Flash_Lock(void);
uint8_t Flash_ErasePage(uint32_t page_addr);
uint8_t Flash_ErasePages(uint32_t page_addr, uint16_t pages);
uint8_t Flash_WriteHalfWord(uint32_t addr, uint16_t data);
uint8_t Flash_Write(uint32_t addr, uint8_t *data, uint32_t len);
uint8_t Flash_Read(uint32_t addr, uint8_t *buffer, uint32_t len);
uint8_t Flash_IsErased(uint32_t addr, uint32_t len);

#endif /* __FLASH_H__ */
