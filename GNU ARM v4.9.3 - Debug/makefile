################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include FreeRTOS/efm32gg/subdir.mk
-include Drivers/subdir.mk
-include CMSIS/EFM32GG/subdir.mk
-include BSP/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: HumanDelay.axf

# Tool invocations
HumanDelay.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g -gdwarf-2 -mcpu=cortex-m3 -mthumb -T "HumanDelay.ld" -Xlinker --gc-sections -Xlinker -Map="HumanDelay.map" --specs=nano.specs -o HumanDelay.axf "./BSP/bsp_trace.o" "./CMSIS/EFM32GG/startup_efm32gg.o" "./CMSIS/EFM32GG/system_efm32gg.o" "./Drivers/retargetserial.o" "./Drivers/segmentlcd.o" "./Drivers/sleep.o" "./FreeRTOS/efm32gg/croutine.o" "./FreeRTOS/efm32gg/event_groups.o" "./FreeRTOS/efm32gg/heap_1.o" "./FreeRTOS/efm32gg/list.o" "./FreeRTOS/efm32gg/port.o" "./FreeRTOS/efm32gg/queue.o" "./FreeRTOS/efm32gg/tasks.o" "./FreeRTOS/efm32gg/timers.o" "./emlib/em_assert.o" "./emlib/em_burtc.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_emu.o" "./emlib/em_gpio.o" "./emlib/em_lcd.o" "./emlib/em_rmu.o" "./emlib/em_rtc.o" "./emlib/em_system.o" "./emlib/em_timer.o" "./emlib/em_usart.o" "./src/main.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: HumanDelay.hex'
	arm-none-eabi-objcopy -O ihex "HumanDelay.axf" "HumanDelay.hex"
	@echo ' '

	@echo 'Building bin file: HumanDelay.bin'
	arm-none-eabi-objcopy -O binary "HumanDelay.axf" "HumanDelay.bin"
	@echo ' '

	@echo 'Building s37 file: HumanDelay.s37'
	arm-none-eabi-objcopy -O srec "HumanDelay.axf" "HumanDelay.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "HumanDelay.axf"
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) HumanDelay.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
