################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
driver/%.o: ../driver/%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: Arm Compiler'
	"D:/ti/ccs2040/ccs/tools/compiler/ti-cgt-armllvm_4.0.4.LTS/bin/tiarmclang.exe" -c @"device.opt"  -march=thumbv6m -mcpu=cortex-m0plus -mfloat-abi=soft -mlittle-endian -mthumb -O2 -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/Debug" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/Core/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source/third_party/CMSIS/DSP/Include" -I"D:/ti/mspm0_sdk_2_09_00_01/source" -I"C:/Users/hp pc/workspace_ccstheia/Winter_Camp_Training_Project1/driver" -DARM_MATH_CM0 -gdwarf-3 -MMD -MP -MF"driver/$(basename $(<F)).d_raw" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


