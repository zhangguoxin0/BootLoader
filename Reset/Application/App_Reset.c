#include "App_Reset.h"
#include <string.h>

#define BUFFER_SIZE 512 // 缓冲区大小

static uint8_t buffer[BUFFER_SIZE];         // 接收固件切片缓冲区
static uint16_t firmwareCarve_len = 0;      // 接收固件切片大小
static uint32_t firmwareCarve_totalLen = 0; // 接收固件总大小

/**
 * @brief 从串口接收数字字符串并转换为uint32_t
 * 只接受数字字符，遇到非法字符返回失败
 * @param number 输出参数，接收的数字
 * @return 0:成功 1:输入非法 2:输入为空
 */
static uint8_t receive_number_from_uart(uint32_t *number)
{
    uint8_t ch;
    uint8_t has_digit = 0; // 是否有数字输入
    *number = 0;

    while (1)
    {
        HAL_UART_Receive(&huart1, &ch, 1, HAL_MAX_DELAY);

        if (ch == '\r' || ch == '\n') // 回车结束
        {
            printf("\r\n");
            if (!has_digit)
                return 2; // 输入为空
            return 0;     // 成功
        }
        else if (ch == '\b' || ch == 0x7F) // 退格删除
        {
            if (has_digit)
            {
                *number /= 10;
                if (*number == 0)
                    has_digit = 0;
                printf("\b \b"); // 终端删除显示
            }
        }
        else if (ch >= '0' && ch <= '9') // 数字字符
        {
            has_digit = 1;
            *number = *number * 10 + (ch - '0');
            printf("%c", ch); // 回显
        }
        else // 非法字符
        {
            printf("\r\nError: Invalid character '%c'. Please input numbers only.\r\n", ch);
            return 1;
        }
    }
}

/**
 * @brief 接收固件切片
 *
 * @param buffer 存放固件切片缓冲区
 * @param len 固件切片大小
 * @return 0:收到切片 1:传输结束
 */
static uint8_t receive_firmwareCarve(uint8_t *buffer, uint16_t *len)
{
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);

    HAL_UARTEx_ReceiveToIdle(&huart1, buffer, BUFFER_SIZE, len, 200);

    if (*len > 0)
    {
        // 检查是否是结束标志 "done"
        if (*len == 4 && memcmp(buffer, "done", 4) == 0)
        {
            return 1; // 传输结束
        }
        return 0; // 表示收到固件数据
    }
    else
    {
        return 0; // 继续等待
    }
}

/**
 * @brief 将接收的固件写入到A区指定位置
 *
 * @param addr 写入地址
 * @param data 写入数据
 * @param len 写入数据长度
 */
static void write_firmwareCarve(uint32_t addr, uint8_t *data, uint32_t len)
{
    Flash_Unlock();
    if (len % 2 != 0)
    {
        data[len] = 0; // 补0对齐
        Flash_Write(addr, data, len + 1);
    }
    else
    {
        Flash_Write(addr, data, len);
    }
    Flash_Lock();
}

/**
 * @brief 接收固件并写入到A区指定位置
 * 发送方每间隔一定时间发送固件的切片，大小不超过512k
 * 接收方接收固件切片并将切片写入到指定位置，后来的内容需要拼接写入
 */
void App_Reset(void)
{
    // 用户输入的固件大小
    uint32_t expected_size = 0;

    printf("Reset begin\r\n");

    // 1.一次性擦除A区内容
    printf("Erasing...\n");
    BootLoader_erase_flash();

    // 2.用户输入固件大小（循环直到输入合法）
    while (1)
    {
        printf("Please input firmware size (bytes): ");
        uint8_t ret = receive_number_from_uart(&expected_size);

        if (ret == 1)
        {
            printf("Please try again.\r\n");
            continue; // 非法输入，重试
        }
        else if (ret == 2)
        {
            printf("Error: Empty input. Please try again.\r\n");
            continue; // 空输入，重试
        }

        // 校验固件大小是否超出A区
        if (expected_size > (APP_END_ADDR - APP_START_ADDR + 1))
        {
            printf("Error: Firmware too large! Max: %lu bytes\r\n", APP_END_ADDR - APP_START_ADDR + 1);
            continue; // 超出范围，重试
        }

        if (expected_size == 0)
        {
            printf("Error: Firmware size cannot be 0. Please try again.\r\n");
            continue;
        }

        break; // 输入合法，退出循环
    }
    printf("Expected: %lu bytes\r\n", expected_size);

    // 3.循环接收固件并拼接写入
    while (1)
    {
        // 接收
        LED3_ON();
        uint8_t ret = receive_firmwareCarve(buffer, &firmwareCarve_len);
        LED3_OFF();
        if (ret == 1)
        {
            break; // 传输结束
        }

        // 写入
        LED2_ON();
        write_firmwareCarve(APP_START_ADDR + firmwareCarve_totalLen, buffer, firmwareCarve_len);
        LED2_OFF();

        firmwareCarve_totalLen += firmwareCarve_len;
        memset(buffer, 0, BUFFER_SIZE);
        firmwareCarve_len = 0;

        printf("Progress: %lu / %lu bytes (%d%%)\r\n", firmwareCarve_totalLen, expected_size, (int)(firmwareCarve_totalLen * 100 / expected_size));
    }

    // 4.校验大小
    if (firmwareCarve_totalLen == expected_size)
    {
        // 校验通过，执行跳转
        printf("OK! Jumping to App...\r\n");
        BootLoader_jump_to_App(APP_START_ADDR);
    }
    else
    {
        // 校验未通过，打印错误信息
        printf("Error: size mismatch (%lu / %lu)\r\n", firmwareCarve_totalLen, expected_size);
    }
}
