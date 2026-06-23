#include "w25q32.h"

void SPI_Start(void)
{
    HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_RESET);
}

void SPI_Stop(void)
{
    HAL_GPIO_WritePin(W25Q32_CS_GPIO_Port, W25Q32_CS_Pin, GPIO_PIN_SET);
}

uint8_t SPI_SwapByte(uint8_t byte)
{
    uint8_t r_byte;

    HAL_SPI_TransmitReceive(&hspi1, &byte, &r_byte, 1, 1000);

    return r_byte;
}

// 初始化
void W25Q32_Init(void)
{
    MX_SPI1_Init();
}

// 读取ID
void W25Q32_ReadID(uint8_t *mid, uint16_t *did)
{
    SPI_Start();

    // 1. 发送指令 9fh
    SPI_SwapByte(0x9f);

    // 2. 获取制造商ID（为了读取数据，发送什么不重要）
    *mid = SPI_SwapByte(0xff);

    // 3. 获取设备ID
    *did = 0;
    *did |= SPI_SwapByte(0xff) << 8;
    *did |= SPI_SwapByte(0xff) & 0xff;

    SPI_Stop();
}

// 开启写使能
void W25Q32_WriteEnable(void)
{
    SPI_Start();
    SPI_SwapByte(0x06);
    SPI_Stop();
}

// 关闭写使能
void W25Q32_WriteDisable(void)
{
    SPI_Start();
    SPI_SwapByte(0x04);
    SPI_Stop();
}

// 等待状态不为忙（busy）
void W25Q32_WaitNotBusy(void)
{
    SPI_Start();

    // 发送读取状态寄存器指令
    SPI_SwapByte(0x05);

    // 等待收到的数据末位变成0
    while ((SPI_SwapByte(0xff) & 0x01))
    {
    }

    SPI_Stop();
}

/**
 * @brief 扇区擦除
 *
 * @param block 块号
 * @param sector 扇区号
 */
void W25Q32_EraseSector(uint8_t block, uint8_t sector)
{
    // 首先等待状态不为忙
    W25Q32_WaitNotBusy();

    // 开启写使能
    W25Q32_WriteEnable();

    // 计算要发送的地址（段首地址）
    uint32_t addr = (block << 16) + (sector << 12);

    SPI_Start();

    // 发送指令
    SPI_SwapByte(0x20);

    SPI_SwapByte(addr >> 16 & 0xff); // 第一个字节
    SPI_SwapByte(addr >> 8 & 0xff);  // 第二个字节
    SPI_SwapByte(addr >> 0 & 0xff);  // 第三个字节

    SPI_Stop();

    W25Q32_WaitNotBusy();
}

// 写入（页写）
void W25Q32_PageWrite(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *data, uint16_t len)
{
    // 首先等待状态不为忙
    W25Q32_WaitNotBusy();

    // 开启写使能
    W25Q32_WriteEnable();

    // 计算要发送的地址（页首地址）
    uint32_t addr = (block << 16) + (sector << 12) + (page << 8) + innerAddr;

    SPI_Start();

    // 发送指令
    SPI_SwapByte(0x02);

    // 发送24位地址
    SPI_SwapByte(addr >> 16); // 第一个字节
    SPI_SwapByte(addr >> 8);  // 第二个字节
    SPI_SwapByte(addr >> 0);  // 第三个字节

    //  依次发送数据
    for (uint16_t i = 0; i < len; i++)
    {
        SPI_SwapByte(data[i]);
    }

    SPI_Stop();

    W25Q32_WriteDisable();
}

/**
 * @brief 读取数据
 *
 * @param block 块地址
 * @param sector 扇区地址
 * @param page 页地址
 * @param innerAddr 页内地址
 * @param buffer 接收缓冲区
 * @param len 读取长度
 */
void W25Q32_Read(uint8_t block, uint8_t sector, uint8_t page, uint8_t innerAddr, uint8_t *buffer, uint16_t len)
{
    // 首先等待状态不为忙
    W25Q32_WaitNotBusy();

    // 计算要发送的地址
    uint32_t addr = (block << 16) + (sector << 12) + (page << 8) + innerAddr;

    SPI_Start();

    // 发送指令
    SPI_SwapByte(0x03);

    // 发送24位地址
    SPI_SwapByte(addr >> 16 & 0xff); // 第一个字节
    SPI_SwapByte(addr >> 8 & 0xff);  // 第二个字节
    SPI_SwapByte(addr >> 0 & 0xff);  // 第三个字节

    //  依次读取数据
    for (uint16_t i = 0; i < len; i++)
    {
        buffer[i] = SPI_SwapByte(0xff);
    }

    SPI_Stop();
}
