################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include usbdrv/subdir.mk
-include uart_io/subdir.mk
-include obj/usbdrv/subdir.mk
-include obj/uart_io/subdir.mk
-include obj/subdir.mk
-include obj/irrx/subdir.mk
-include irrx/subdir.mk
-include bootloader_firmware/usbdrv/subdir.mk
-include bootloader_firmware/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: usb-ir-remote

# Tool invocations
usb-ir-remote: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc  -o"usb-ir-remote" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) usb-ir-remote
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
