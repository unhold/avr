################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../bootloader_firmware/usbdrv/oddebug.o \
../bootloader_firmware/usbdrv/usbdrv.o \
../bootloader_firmware/usbdrv/usbdrvasm.o 

C_SRCS += \
../bootloader_firmware/usbdrv/oddebug.c \
../bootloader_firmware/usbdrv/usbdrv.c 

S_UPPER_SRCS += \
../bootloader_firmware/usbdrv/usbdrvasm.S 

ASM_SRCS += \
../bootloader_firmware/usbdrv/usbdrvasm.asm 

OBJS += \
./bootloader_firmware/usbdrv/oddebug.o \
./bootloader_firmware/usbdrv/usbdrv.o \
./bootloader_firmware/usbdrv/usbdrvasm.o 

C_DEPS += \
./bootloader_firmware/usbdrv/oddebug.d \
./bootloader_firmware/usbdrv/usbdrv.d 


# Each subdirectory must supply rules for building sources it contributes
bootloader_firmware/usbdrv/%.o: ../bootloader_firmware/usbdrv/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bootloader_firmware/usbdrv/%.o: ../bootloader_firmware/usbdrv/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

bootloader_firmware/usbdrv/%.o: ../bootloader_firmware/usbdrv/%.asm
	@echo 'Building file: $<'
	@echo 'Invoking: GCC Assembler'
	as  -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


