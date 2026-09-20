# 简介

<p style="text-indent:2em;">CAN总线（Controller Area Network Bus）控制器局域网总线是由BOSCH公司开发的一种简洁易用、传输速度快、易扩展、可靠性高的串行通信总线，广泛应用于汽车、嵌入式、工业控制等领域。</p>

## CAN总线特征：

- 两根通信线（CAN_H、CAN_L），线路少，无需共地
- 差分信号通信，抗干扰能力强
- 高速CAN（ISO11898）：125k~1Mbps, <40m
- 低速CAN（ISO11519）：10k~125kbps, <1km
- 异步，无需时钟线，通信速率由设备各自约定
- 半双工，可挂载多设备，多设备同时发送数据时通过仲裁判断先后顺序
- 11位/29位报文ID，用于区分消息功能，同时决定优先级
- 可配置0~8字节的有效载荷
- 可实现广播式和请求式两种传输方式
- 应答、CRC校验、位填充、位同步、错误处理等特性

## 主流特性协议对比

| 名称 | 引脚                | 全/半双工 | 时钟 | 电平 | 设备   | 应用场景         |
| ---- | ------------------- | --------- | ---- | ---- | ------ | ---------------- |
| UART | TX、RX              | 全双工    | 异步 | 单端 | 点对点 | 两个设备相互通信 |
| I2C  | SCL、SDA            | 半双工    | 同步 | 单端 | 多设备 | 多主多从         |
| SPI  | SCK、MOSI、MISO、CS | 全双工    | 同步 | 单端 | 多设备 | 一主多从         |
| CAN  | CAN_H、CAH_L        | 半双工    | 异步 | 差分 | 多设备 | 多对多           |

# 物理层

## 硬件电路

### 闭环总线网络

<p style="text-indent:2em;">CAN闭环通讯是种遵循ISO11898标准的高速、短距离网络，它的总线长度最大为40m，通信速度最高为1Mbps，总线的两段各要求120Ω电阻。</p>

![闭环总线网络](diagrams\CAN\close-loop_bus_network.drawio.png)

### 开环总线网络

<p style="text-indent:2em;">CAN开环通讯是种遵循ISO11519标准的低速、远距离网络，它的总线长度最大为1km，通信速度最高为1kbps，要求每根线上串2.2kΩ电阻。</p>

![开环总线网络](diagrams\CAN\open-loop_bus_network.drawio.png)

<p style="text-indent:2em;">CAN收发器的作用：实现电平转换、输出驱动和输入采样</p>

## 电平标准

<p style="text-indent:2em;">CAN总线采用差分信号，即两线电压差（CAN_H、CAN_L）传输数据位，如：</p>

- 当表示逻辑1时（隐性电平）：CAN_H的电平为2.5V，CAN_L的电平为2.5V，电压差V<sub>H</sub>-V<sub>L</sub>=0V。
- 当表示逻辑0时（显性电平）：CAN_H的电平为3.5V，CAN_L的电平为1.5V，电压差V<sub>H</sub>-V<sub>L</sub>=2V。

<p style="text-indent:2em;">在CAN总线中，必须使它处于隐性电平或显性电平中的一个状态。假如有两个CAN通讯节点，在同一时间一个输出隐性电平，一个输出显性电平，类似I2C总线“线与”特性使它处于显性电平状态。</p>

## 特性

![物理特性](diagrams\CAN\physical_property.drawio.png)

# 协议层

<p style="text-indent:2em;">CAN总线以“帧”形式进行通信。CAN协议定义了5种类型的帧：数据帧、遥控帧、错误帧、过载帧、间隔帧，其中数据帧最为常用。</p>

| 帧类型                    | 帧作用                                         |
| ------------------------- | ---------------------------------------------- |
| 数据帧(Data Frame)        | 用于发送单元向接收单元传输数据的帧             |
| 遥控帧(Remote Frame)      | 用于接收单元向具有相同ID的发送单元请求数据的帧 |
| 错误帧(Error Frame)       | 用于检测到错误时向其他单元通知错误的帧         |
| 过载帧(Overload Frame)    | 用于接收单元通知其尚未做好接收准备的帧         |
| 间隔帧(Inter Frame Space) | 用于将数据帧及遥控帧与前面的帧分离开来的帧     |

## 数据帧

<p style="text-indent:2em;">数据帧由7段组成（帧起始、仲裁段、控制段、数据段、CRC段、ACK段、帧结束）。数据帧又分为标准帧（CAN2.0A）和扩展帧（CAN2.0B）。</p>

![数据帧](diagrams\CAN\data_frame.drawio.png)

- 帧起始（SOF段）：**固定1位显性电平**，用于通知各个节点有数据传输，其他节点通过帧起始信号电平跳变沿来进行硬同步。
- 仲裁段：当同时有多个报文被发送时，总线会根据仲裁段的内容来决定哪个数据报能被传输，**ID越小优先级越高**。
- RTR（远程传输请求位）：用于区分数据帧和遥控帧，**显性表示数据帧，隐性表示遥控帧**。
- IDE（标识符扩展位）：用于区分标准格式与拓展格式，**显性表示标准格式，隐性表示扩展格式**。
- SRR：只存在于扩展格式，用于代替标准格式中的RTR位。由于**扩展帧中的SSR位为隐性位**，RTR在数据帧为显性位，所以**ID相同的标准帧和扩展帧报文中标准帧优先级更高**。
- 控制段：R1和R0为保留位，默认设置为显性。最主要的是DLC段（数据长度码），它由4个数据位组成，用于表示本报文中数据段有多少个字节，**DLC表示的范围为0~8**。
- 数据段：节点要发送的原始信息，由0~8个字节组成，**MSB高位先行**。
- CRC段：为**保证数据正确传输**，CAN的报文包含一段15位的CRC校验码，一旦接收节点算出的CRC码跟接收到的CRC码不同，则会向发送节点反馈出错信息，利用错误帧请求它重新发送。CRC部分计算由CAN控制器硬件完成，出错时处理则由软件控制最大重发数。
- DEL（界定符）：**固定为隐性位**，主要作用是把CRC校验码与后面的ACK段间隔起来。
- ACK段：包括一个ACK位和一个界定符位。类似于I2C总线，在ACK位中**发送节点发送的是隐性位，而接收节点则在这一位中发送显性位以示应答**。
- 帧结束：**由发送节点发送7个隐性位表示结束**。

## 位时序

<p style="text-indent:2em;">CAN总线以“位同步”机制，实现对电平的正确采样。位数据都由四段组成：同步段（SS）、传播时间段（PTS）、相位缓冲段1（PBS1）、相位缓冲段2（PBS2），每段又由8~25个位时序Tq组成。</p>

![位时序](diagrams\CAN\bit_timing.drawio.png)

<p style="text-indent:2em;">采样点是指读取总线电平，并将读到的电平作为位值点。根据位时序就可以计算CAN通信的波特率。</p>

## CAN控制器

### 简介

<p style="text-indent:2em;">STM32 CAN控制器（bxCAN），支持 CAN2.0A 和 CAN2.0B Active 版本协议。CAN2.0A 只能处理标准数据帧且扩展帧的内容会识别错误，而 CAN2.0B Active 可以处理标准数据帧和扩展数据帧。CAN2.0B Passive 只能处理标准数据帧且扩展帧内容会忽略。</p>

**bxCAN主要特点**：

- 波特率最高可达1Mbps
- 支持时间触发通信（CAN的硬件内部定时器可以在TX/RX的帧起始位的采样点位置生成时间戳）
- 具有3级发送邮箱
- 具有3级深度的2个接收FIFO
- 可变的过滤器组（最多28个）

### 工作模式

<p style="text-indent:2em;">CAN控制器的工作模式有3种：初始化模式、正常模式和睡眠模式。</p>

![工作模式](diagrams\CAN\work_pattern.png)

### 测试模式

<p style="text-indent:2em;">CAN控制器的测试模式有3种：静默模式、还回模式和环回静默模式。</p>

![测试模式](diagrams\CAN\test_pattern.drawio.png)

### 发送处理

![发送处理](diagrams\CAN\sending_process.drawio.png)

### 接收处理

![接收处理](diagrams\CAN\receiving_process.drawio.png)

### 接收过滤器

<p style="text-indent:2em;">当总线上报文数量很大时，总线上的设备会频繁获取报文，占用CPU。过滤器的存在，选择性收有效报文，减轻系统负担。</p>

<p style="text-indent:2em;">每个过滤器组都有两个32位寄存器CAN_FxR1和CAN_FxR2。根据过滤器组的工作模式不同，寄存器的作用不尽相同。</p>

<p style="text-indent:2em;">位宽可设置32位或16位，寄存器存储的内容就有所区别。</p>

| 过滤器组Reg | 32位                               | 16位（寄存器由两部分组成）          |
| ----------- | ---------------------------------- | ----------------------------------- |
| CAN_FxR1    | STDID[10:0]、EXTID[17:0]、IDE、RTR | STDID[10:0]、EXTID[17:15]、IDE、RTR |
| CAN_FxR2    | STDID[10:0]、EXTID[17:0]、IDE、RTR | STDID[10:0]、EXTID[17:15]、IDE、RTR |

<p style="text-indent:2em;">选择模式可设置为屏蔽位模式或标识符列表模式，寄存器内容和功能就有所区别。</p>

- **屏蔽位模式**：可以选择一组符合条件的报文。寄存器内容功能相当于是符合条件。
- **标识符列表模式**：可以选择出几个特定ID的报文。寄存器内容功能就是标识符本身。

> REG中的bit值代表匹配与否：1必须匹配 0不用关心

### 控制器位时序

![位时序](diagrams\CAN\controller_timing.drawio.png)

$$
波特率 = \frac{1}{正常位时间}
$$

$$
正常位时间 = 1 \times t_{q} + t_{BS1} + t_{BS2}
$$

其中：

$$
t_{BS1} = t_q \times (TS1[3:0] + 1)
$$

$$
t_{BS2} = t_q \times (TS2[2:0] + 1)
$$

$$
t_q = (BRP[9:0] + 1) \times t_{PCLK}
$$

这里 $t_q$ 表示1个时间单元

$$
t_{PCLK} = APB时钟的时间周期
$$

$$
波特率 = \frac{1}{t_q + t_q \times (TS1[3:0] + 1) + t_q \times(TS2[2:0]+1)}
$$

# 配置过程

## HAL库配置

![CAN总线HAL库配置](diagrams\CAN\CAN_HAL_config.png)

1. 配置预分频系数
2. 配置 BS1 BS2 的 tq 数
3. 开启自动进入睡眠模式和自动唤醒功能
4. 测试模式设置为环回静默模式
5. 引脚重映射到 PA8 PA9

## 用户配置

```c
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
```
