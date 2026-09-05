# STM32 BootLoader HAL

## 项目概述

嵌入式初学者的Bootloader学习项目，从系统自带IAP到串口IAP，最终实现可靠的OTA远程升级。

## 技术栈

- MCU: STM32F103ZE
- 开发环境: Keil MDK-ARM
- HAL库: STM32F1xx HAL Driver
- 外设: USART1、I2C2、GPIO、Flash
- 外部存储: W24C02 EEPROM (I2C)

## 项目结构

```
Bootloader/          # Bootloader工程
├── Core/           # 核心代码(main.c, usart.c等)
├── Driver/         # 外部设备驱动(W24C02等)
└── MDK-ARM/        # Keil工程文件

APP/                # 应用程序工程
├── Core/
├── Driver/
└── MDK-ARM/
```

## 编码规范

- HAL库风格，遵循STM32CubeMX生成的代码结构
- 用户代码放在USER CODE BEGIN/END注释块内
- 驱动模块放在Driver目录，按设备名分类

## 当前开发状态

- [x] 基础工程搭建(Bootloader/APP双工程)
- [x] W24C02 EEPROM驱动(I2C)
- [x] USART1串口重定向(printf)
- [ ] 串口接收Bin文件
- [ ] Flash写入操作
- [ ] 程序跳转逻辑
- [ ] OTA远程升级

## 注意事项

- Bootloader和APP需要独立配置中断向量表偏移
- Flash操作前必须先擦除对应扇区
- 跳转前需校验APP入口地址有效性
