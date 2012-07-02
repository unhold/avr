################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../bootloader_firmware/main.o 

C_SRCS += \
../bootloader_firmware/main.c 

OBJS += \
./bootloader_firmware/main.o 

C_DEPS += \
./bootloader_firmware/main.d 


# Each subdirectory must supply rules for building sources it contributes
bootloader_firmware/%.o: ../bootloader_firmware/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


