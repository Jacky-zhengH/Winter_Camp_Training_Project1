################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/Debug" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/DSP/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/driver" -DARM_MATH_CM0 -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

build-885341976: ../cmsis_dsp_empty.syscfg
	@echo 'Building file: "$<"'
	@echo 'Invoking: SysConfig'
	"C:/ti/sysconfig_1.25.0/sysconfig_cli.bat" -s "D:/ti/mspm0_sdk_2_09_00_01/.metadata/product.json" --script "C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/cmsis_dsp_empty.syscfg" -o "." --compiler ticlang
	@echo 'Finished building: "$<"'
	@echo ' '

device_linker.cmd: build-885341976 ../cmsis_dsp_empty.syscfg
device.opt: build-885341976
device.cmd.genlibs: build-885341976
ti_msp_dl_config.c: build-885341976
ti_msp_dl_config.h: build-885341976
Event.dot: build-885341976

%.o: ./%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/Debug" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/DSP/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/driver" -DARM_MATH_CM0 -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

startup_mspm0g350x_ticlang.o: D:/ti/mspm0_sdk_2_09_00_01/source/ti/devices/msp/m0p/startup_system_files/ticlang/startup_mspm0g350x_ticlang.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/Debug" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/DSP/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/driver" -DARM_MATH_CM0 -gdwarf-3 -MMD -MP -MF"$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


