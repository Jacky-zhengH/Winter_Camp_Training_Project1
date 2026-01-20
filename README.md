
## 简易信号分析仪 
![Last Commit](https://img.shields.io/github/last-commit/Jacky-zhengH/Winter_Camp_Training_Project1)
![Status](https://img.shields.io/badge/Status-Under_Development-orange)
> 完成情况
> - [x] UART+DMA 不定长不定参数
> - [x] VCA程控放大器增益
> - [ ] 定时器捕获模式+比较器:测频率（进阶高低频采用不同采集算法）
> - [ ] adc+定时器触发+dma：测峰峰值（进阶：实现动态调整采样率）
> - [ ] FFT算法识别波形
> 进阶要求
> - [ ] 频率范围：1K~100KHz
> - [ ] 语音识别功能
> - [ ] 蓝牙模块传递参数+上位机再现

---
### 目录
- [智能车电机开发](#智能车电机开发)
  - [目录](#目录)
  - [电机驱动](#电机驱动)
    - [第一层：电机基础驱动](#第一层电机基础驱动)
  - [PID控制算法](#pid控制算法)
---


## Example Summary
Empty project using CMSIS DSP.
This example shows a basic empty project using CMSIS DSP with just main file
and SysConfig initialization.


## Peripherals & Pin Assignments

| Peripheral | Pin  | Function          |
| ---------- | ---- | ----------------- |
| SYSCTL     |      |                   |
| DEBUGSS    | PA20 | Debug Clock       |
| DEBUGSS    | PA19 | Debug Data In Out |

## BoosterPacks, Board Resources & Jumper Settings

Visit [LP_MSPM0G3507](https://www.ti.com/tool/LP-MSPM0G3507) for LaunchPad information, including user guide and hardware files.

| Pin  | Peripheral | Function | LaunchPad Pin | LaunchPad Settings                                                                                                                                                                                           |
| ---- | ---------- | -------- | ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| PA20 | DEBUGSS    | SWCLK    | N/A           | <ul><li>PA20 is used by SWD during debugging<br><ul><li>`J101 15:16 ON` Connect to XDS-110 SWCLK while debugging<br><li>`J101 15:16 OFF` Disconnect from XDS-110 SWCLK if using pin in application</ul></ul> |
| PA19 | DEBUGSS    | SWDIO    | N/A           | <ul><li>PA19 is used by SWD during debugging<br><ul><li>`J101 13:14 ON` Connect to XDS-110 SWDIO while debugging<br><li>`J101 13:14 OFF` Disconnect from XDS-110 SWDIO if using pin in application</ul></ul> |

### Device Migration Recommendations
This project was developed for a superset device included in the LP_MSPM0G3507 LaunchPad. Please
visit the [CCS User's Guide](https://software-dl.ti.com/msp430/esd/MSPM0-SDK/latest/docs/english/tools/ccs_ide_guide/doc_guide/doc_guide-srcs/ccs_ide_guide.html#sysconfig-project-migration)
for information about migrating to other MSPM0 devices.

### Low-Power Recommendations
TI recommends to terminate unused pins by setting the corresponding functions to
GPIO and configure the pins to output low or input with internal
pullup/pulldown resistor.

SysConfig allows developers to easily configure unused pins by selecting **Board**→**Configure Unused Pins**.

For more information about jumper configuration to achieve low-power using the
MSPM0 LaunchPad, please visit the [LP-MSPM0G3507 User's Guide](https://www.ti.com/lit/slau873).

## Example Usage
Compile and load the example.
