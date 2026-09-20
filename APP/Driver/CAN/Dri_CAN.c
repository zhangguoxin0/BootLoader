#include "Dri_CAN.h"

void Dri_CAN_Init(void)
{
    // 配置过滤器
    CAN_FilterTypeDef filterConfig = {0};
    filterConfig.FilterBank = 0;                      // 过滤器编号(0~13)
    filterConfig.FilterMode = CAN_FILTERMODE_IDMASK;  // 掩码模式
    filterConfig.FilterScale = CAN_FILTERSCALE_32BIT; // 选择使用32位过滤器
    filterConfig.FilterIdHigh = 0x0000;               // ID高16位
    filterConfig.FilterIdLow = 0x0000;                // ID低16位
    filterConfig.FilterMaskIdHigh = 0x0000;           // 掩码高16位：0表示不需要匹配，1表示需要匹配
    filterConfig.FilterMaskIdLow = 0x0000;            // 掩码低16位：0表示不需要匹配，1表示需要匹配
    filterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // 使用接收队列0
    filterConfig.FilterActivation = ENABLE;           // 使能过滤器
    HAL_CAN_ConfigFilter(&hcan, &filterConfig);
    // 开启CAN
    HAL_CAN_Start(&hcan);
}

/**
 * @brief 发送消息
 *
 * @param message 消息结构体
 */
void Dri_CAN_Send(Dri_CAN_Message *message)
{
    uint32_t mailbox = 0; // 使用的发送邮箱编号

    // 等待发送邮箱空闲
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    {
    }
    // 将发送的学校添加到发送邮箱
    CAN_TxHeaderTypeDef txHeader = {0};
    txHeader.StdId = message->StdId;
    txHeader.ExtId = message->ExtId;
    txHeader.IDE = message->IDE;
    txHeader.RTR = message->RTR;
    txHeader.DLC = message->DLC;
    HAL_CAN_AddTxMessage(&hcan, &txHeader, message->data, &mailbox);
}

/**
 * @brief 接收消息
 *
 * @param message 消息结构体
 *
 * @retval 0 读取失败
 * @retval 1 读取成功
 */
uint8_t Dri_CAN_Receive(Dri_CAN_Message *message)
{
    uint8_t msg_count = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0);
    if (msg_count == 0)
    {
        return 0;
    }
    else if (msg_count > 0)
    {
        CAN_RxHeaderTypeDef rxheader = {0};
        HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxheader, message->data);
        message->StdId = rxheader.StdId;
        message->ExtId = rxheader.ExtId;
        message->IDE = rxheader.IDE;
        message->RTR = rxheader.RTR;
        message->DLC = rxheader.DLC;
    }

    return 1;
}
