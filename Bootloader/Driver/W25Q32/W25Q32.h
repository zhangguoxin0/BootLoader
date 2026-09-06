#ifndef __W25Q32_H__
#define __W25Q32_H__

#include "spi.h"
#include "gpio.h"

// W25Q32 参数
#define W25Q32_BLOCK_SIZE       (64 * 1024)   // 64KB
#define W25Q32_SECTOR_SIZE      (4 * 1024)    // 4KB
#define W25Q32_PAGE_SIZE        256           // 256 Bytes
#define W25Q32_BLOCK_COUNT      64
#define W25Q32_SECTOR_COUNT     1024

// CS引脚控制
#define W25Q32_CS_LOW()         HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_RESET)
#define W25Q32_CS_HIGH()        HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_SET)

// W25Q32 指令
#define W25Q32_CMD_WRITE_ENABLE     0x06
#define W25Q32_CMD_WRITE_DISABLE    0x04
#define W25Q32_CMD_READ_STATUS      0x05
#define W25Q32_CMD_READ_DATA        0x03
#define W25Q32_CMD_PAGE_PROGRAM     0x02
#define W25Q32_CMD_SECTOR_ERASE     0x20
#define W25Q32_CMD_BLOCK_ERASE      0xD8
#define W25Q32_CMD_CHIP_ERASE       0xC7
#define W25Q32_CMD_READ_ID          0x9F

// 函数声明
void W25Q32_Init(void);
void W25Q32_ReadID(uint8_t *mid, uint16_t *did);
void W25Q32_WriteEnable(void);
void W25Q32_WaitNotBusy(void);
uint8_t W25Q32_EraseSector(uint32_t sector_addr);
uint8_t W25Q32_EraseBlock(uint32_t block_addr);
uint8_t W25Q32_ChipErase(void);
uint8_t W25Q32_PageWrite(uint32_t addr, uint8_t *data, uint16_t len);
uint8_t W25Q32_Write(uint32_t addr, uint8_t *data, uint32_t len);
uint8_t W25Q32_Read(uint32_t addr, uint8_t *buffer, uint32_t len);

#endif /* __W25Q32_H__ */
