# STM32 BootLoader HAL

## 项目概述

嵌入式初学者的Bootloader学习项目，从系统自带IAP到串口IAP，最终实现可靠的OTA远程升级。

## 技术栈

- MCU: STM32F103ZE
- 开发环境: Keil MDK-ARM
- HAL库: STM32F1xx HAL Driver
- 外设: USART1、I2C2、GPIO、Flash
- 外部存储: W24C02 EEPROM (I2C)、W25Q32 NOR Flash (SPI)

## 项目结构

```
Bootloader/          # Bootloader工程
├── Core/           # 核心代码(main.c, usart.c等)
├── Driver/         # 外部设备驱动
│   ├── Flash/      # 片上Flash操作
│   ├── M24C02/     # EEPROM驱动(I2C)
│   └── W25Q32/     # NOR Flash驱动(SPI)
├── Interface/      # Bootloader接口层
├── Application/    # 应用逻辑
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
- [x] 启动流程
- [x] 串口重定向(printf)
- [x] W24C02 EEPROM驱动(I2C)
- [x] W25Q32 NOR Flash驱动(SPI)
- [x] 片上Flash操作模块(擦除/写入/读取)
- [x] 程序跳转逻辑
- [x] OTA标志管理
- [ ] CRC32数据校验
- [ ] 触发条件
- [ ] 固件下载
- [ ] 验证机制
- [ ] 安装写入

## 注意事项

- Bootloader和APP需要独立配置中断向量表偏移
- Flash操作前必须先擦除对应扇区
- 跳转前需校验APP入口地址有效性
- Flash模块要求地址和长度半字对齐(2字节边界)
- W25Q32 CS引脚: PC13, SPI1: PA5/PA6/PA7
