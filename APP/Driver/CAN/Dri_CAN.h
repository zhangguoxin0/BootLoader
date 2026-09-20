#ifndef __DRI_CAN_H__
#define __DRI_CAN_H__

#include "can.h"
#include "main.h"
#include <string.h>

typedef struct
{
    uint32_t StdId;  // 标准消息ID
    uint32_t ExtId;  // 扩展消息ID
    uint32_t IDE;    // 标准格式
    uint32_t RTR;    // 数据帧
    uint32_t DLC;    // 数据长度
    uint8_t data[8]; // 数据
} Dri_CAN_Message;

void Dri_CAN_Init(void);

void Dri_CAN_Send(Dri_CAN_Message *message);
uint8_t Dri_CAN_Receive(Dri_CAN_Message *message);

#endif /* __DRI_CAN_H__ */
