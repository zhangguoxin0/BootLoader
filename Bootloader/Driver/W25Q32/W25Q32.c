#include "W25Q32.h"

/**
 * @brief SPI发送接收一个字节
 *
 * @param byte 发送的字节
 * @return uint8_t 接收到的字节
 */
static uint8_t W25Q32_SwapByte(uint8_t byte)
{
    uint8_t r_byte;
    HAL_SPI_TransmitReceive(&hspi1, &byte, &r_byte, 1, 1000);
    return r_byte;
}

/**
 * @brief 初始化W25Q32
 *
 */
void W25Q32_Init(void)
{
    MX_SPI1_Init();
    W25Q32_CS_HIGH();
}

/**
 * @brief 读取ID
 *
 * @param mid 制造商ID
 * @param did 设备ID
 */
void W25Q32_ReadID(uint8_t *mid, uint16_t *did)
{
    W25Q32_CS_LOW();

    W25Q32_SwapByte(W25Q32_CMD_READ_ID);
    *mid = W25Q32_SwapByte(0xFF);
    *did = 0;
    *did |= W25Q32_SwapByte(0xFF) << 8;
    *did |= W25Q32_SwapByte(0xFF) & 0xFF;

    W25Q32_CS_HIGH();
}

/**
 * @brief 写使能
 *
 */
void W25Q32_WriteEnable(void)
{
    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_WRITE_ENABLE);
    W25Q32_CS_HIGH();
}

/**
 * @brief 等待不忙
 *
 */
void W25Q32_WaitNotBusy(void)
{
    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_READ_STATUS);
    while ((W25Q32_SwapByte(0xFF) & 0x01))
    {
    }
    W25Q32_CS_HIGH();
}

/**
 * @brief 扇区擦除(4KB)
 *
 * @param sector_addr 扇区地址(任意地址，自动对齐到扇区边界)
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_EraseSector(uint32_t sector_addr)
{
    // 对齐到扇区边界
    sector_addr &= ~(W25Q32_SECTOR_SIZE - 1);

    W25Q32_WaitNotBusy();
    W25Q32_WriteEnable();

    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_SECTOR_ERASE);
    W25Q32_SwapByte((sector_addr >> 16) & 0xFF);
    W25Q32_SwapByte((sector_addr >> 8) & 0xFF);
    W25Q32_SwapByte(sector_addr & 0xFF);
    W25Q32_CS_HIGH();

    W25Q32_WaitNotBusy();
    return 0;
}

/**
 * @brief 块擦除(64KB)
 *
 * @param block_addr 块地址
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_EraseBlock(uint32_t block_addr)
{
    // 对齐到块边界
    block_addr &= ~(W25Q32_BLOCK_SIZE - 1);

    W25Q32_WaitNotBusy();
    W25Q32_WriteEnable();

    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_BLOCK_ERASE);
    W25Q32_SwapByte((block_addr >> 16) & 0xFF);
    W25Q32_SwapByte((block_addr >> 8) & 0xFF);
    W25Q32_SwapByte(block_addr & 0xFF);
    W25Q32_CS_HIGH();

    W25Q32_WaitNotBusy();
    return 0;
}

/**
 * @brief 全片擦除
 *
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_ChipErase(void)
{
    W25Q32_WaitNotBusy();
    W25Q32_WriteEnable();

    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_CHIP_ERASE);
    W25Q32_CS_HIGH();

    W25Q32_WaitNotBusy();
    return 0;
}

/**
 * @brief 页写入(最大256字节，不能跨页)
 *
 * @param addr 写入地址
 * @param data 数据指针
 * @param len 数据长度(最大256字节)
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_PageWrite(uint32_t addr, uint8_t *data, uint16_t len)
{
    if (len == 0 || len > W25Q32_PAGE_SIZE)
    {
        return 1;
    }

    W25Q32_WaitNotBusy();
    W25Q32_WriteEnable();

    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_PAGE_PROGRAM);
    W25Q32_SwapByte((addr >> 16) & 0xFF);
    W25Q32_SwapByte((addr >> 8) & 0xFF);
    W25Q32_SwapByte(addr & 0xFF);

    for (uint16_t i = 0; i < len; i++)
    {
        W25Q32_SwapByte(data[i]);
    }
    W25Q32_CS_HIGH();

    W25Q32_WaitNotBusy();
    return 0;
}

/**
 * @brief 写入数据(自动处理跨页)
 *
 * @param addr 写入起始地址
 * @param data 数据指针
 * @param len 数据长度
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
    uint32_t written = 0;

    while (written < len)
    {
        // 计算当前页剩余空间
        uint16_t page_remain = W25Q32_PAGE_SIZE - (addr % W25Q32_PAGE_SIZE);
        uint16_t to_write = (len - written > page_remain) ? page_remain : (len - written);

        if (W25Q32_PageWrite(addr, data + written, to_write) != 0)
        {
            return 1;
        }

        addr += to_write;
        written += to_write;
    }

    return 0;
}

/**
 * @brief 读取数据
 *
 * @param addr 读取起始地址
 * @param buffer 接收缓冲区
 * @param len 读取长度
 * @return uint8_t 0:成功
 */
uint8_t W25Q32_Read(uint32_t addr, uint8_t *buffer, uint32_t len)
{
    W25Q32_WaitNotBusy();

    W25Q32_CS_LOW();
    W25Q32_SwapByte(W25Q32_CMD_READ_DATA);
    W25Q32_SwapByte((addr >> 16) & 0xFF);
    W25Q32_SwapByte((addr >> 8) & 0xFF);
    W25Q32_SwapByte(addr & 0xFF);

    for (uint32_t i = 0; i < len; i++)
    {
        buffer[i] = W25Q32_SwapByte(0xFF);
    }
    W25Q32_CS_HIGH();

    return 0;
}
