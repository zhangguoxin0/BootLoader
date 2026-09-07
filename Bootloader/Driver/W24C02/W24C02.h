#ifndef __W24C02_H__
#define __W24C02_H__

#include "i2c.h"

#define W24C02_ADDR_W 0xA0
#define W24C02_ADDR_R (W24C02_ADDR_W | 0x01)

#define W24C02_ADDR_SIZE 8
#define W24C02_PAGE_SIZE 16
#define W24C02_SIZE 256

void W24C02_Init(void);
uint8_t W24C02_ReadByte(uint8_t byte_addr);
void W24C02_WriteByte(uint8_t byte_addr, uint8_t data);
void W24C02_ReadStr(uint8_t byte_addr, uint8_t *data, uint16_t len);
void W24C02_WriteStr(uint8_t byte_addr, uint8_t *data, uint16_t len);

#endif /* __W24C02_H__ */
