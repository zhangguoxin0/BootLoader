#include "App_update.h"

static uint8_t uart_rec_buf[32] = {0};

static Update_State_t update_state = UPDATE_IDLE;

static app_data_buf[APP_DATA_MAX_LEN] = {0}; // 存放更新固件缓冲区

static Dri_CAN_Message cmdMessage; // 更新指令CAN消息结构体

static Dri_CAN_Message recMessage; // 接收固件CAN消息结构体
static can_rec_msg_len = 0;        // 接收到的更新固件长度
static can_rec_time = 0;           // 记录当前一次接收的时间

void App_update_Init(void)
{
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uart_rec_buf, 32);
}

/**
 * @brief 使用CAN向上位机发送更新命令
 *
 */
void App_update_send_update_cmd(void)
{
    cmdMessage.StdId = 0;
    cmdMessage.IDE = CAN_ID_STD;
    cmdMessage.RTR = CAN_RTR_DATA;
    cmdMessage.DLC = 3;
    memcpy(cmdMessage.data, APP_UPDATE_CMD, 3);
    Dri_CAN_Send(&cmdMessage);

    update_state = UPDATE_RECV_DATA;
}

/**
 * @brief CAN接收程序固件保存到W25Q32中
 *
 */
void App_update_receive_app_data(void)
{
    uint8_t ret = Dri_CAN_Receive(&recMessage);
    if (ret == 1)
    {
        can_rec_time = HAL_GetTick();
        for (uint8_t i = 0; i < recMessage.DLC; i++)
        {
            memcpy(app_data_buf + can_rec_msg_len, recMessage.data, recMessage.DLC);
            can_rec_msg_len += recMessage.DLC;
        }
    }

    // 当超过2s没有新数据则认为接收完成
    if ((can_rec_time != 0) && ((can_rec_time + 2000) < HAL_GetTick()))
    {
        printf("recv data len:%d\r\n", can_rec_msg_len);
        update_state = UPDATE_RECV_CHECK_DATA;
    }
}

static uint32_t App_crc_cal(uint8_t *data, uint16_t len)
{
    uint32_t *p_data = (uint32_t *)data;
    uint32_t word_count = (len + 3) / 4;

    __HAL_CRC_DR_RESET(&hcrc);

    uint32_t crc_val = HAL_CRC_Calculate(&hcrc, p_data, word_count);

    return crc_val;
}

/**
 * @brief 添加校验逻辑
 *
 */
void App_update_check_data(void)
{
    uint8_t ret = Dri_CAN_Receive(&recMessage);
    if (ret == 1)
    {
        uint32_t rec_crc_val = recMessage.data[0] | (recMessage.data[0] << 8) | (recMessage.data[2] << 16) | (recMessage.data[3] << 24);
        uint32_t cru_crc_val = App_crc_cal(app_data_buf, can_rec_msg_len);
        if (rec_crc_val == cru_crc_val)
        {
            printf("crc check pass\r\n");
        }
        else
        {
            printf("crc check fail\r\n");

            memset(app_data_buf, 0, APP_DATA_MAX_LEN);
            can_rec_msg_len = 0;
            can_rec_time = 0;

            update_state = UPDATE_IDLE;
        }
    }
}

/**
 * @brief 修改W24C02中的更新标志位
 *
 */
void App_update_change_boot_mode(void)
{
}

void App_run(void)
{
    LED3_OFF();
    LED1_ON();
    HAL_Delay(200);
    LED1_OFF();
    LED2_ON();
    HAL_Delay(200);
    LED2_OFF();
    LED3_ON();
    HAL_Delay(200);
}

/**
 * @brief 循环调用，执行状态机逻辑
 *
 */
void App_update_work(void)
{
    switch (update_state)
    {
    case UPDATE_IDLE:
        App_run();
        break;
    case UPDATE_RECV_SEND_CMD:
        App_update_send_update_cmd();
        break;
    case UPDATE_RECV_DATA:
        App_update_receive_app_data();
        break;
    case UPDATE_RECV_CHECK_DATA:
        break;
    default:
        break;
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if ((huart->Instance == USART1) && (update_state == UPDATE_IDLE))
    {
        if (strstr((char *)uart_rec_buf, "cmd"))
        {
            update_state = UPDATE_RECV_SEND_CMD;
        }
    }
}
