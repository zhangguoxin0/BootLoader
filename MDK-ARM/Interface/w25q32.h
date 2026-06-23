#ifndef __W25Q32_H__
#define __W25Q32_H__

#include "spi.h"
#include "gpio.h"

// 初始化
void W25Q32_Init(void);

// 读取ID
void W25Q32_ReadID(uint8_t *mid, uint16_t *did);

// 开启写使能
void W25Q32_WriteEnable(void);

// 关闭写使能
void W25Q32_WriteDisable(void);

// 等待状态不为忙（busy）
void W25Q32_WaitNotBusy(void);

// 擦除段（sector erase）,地址只需要块号和段号
void W25Q32_EraseSector(uint8_t block, uint8_t sector);

// 写入（页写）
void W25Q32_PageWrite(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *data, uint16_t len);

// 读取
void W25Q32_Read(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *buffer, uint16_t len);

#endif /* __W25Q32_H__ */
