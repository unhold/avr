################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../uart_io/uart_io.c 

OBJS += \
./uart_io/uart_io.o 

C_DEPS += \
./uart_io/uart_io.d 


# Each subdirectory must supply rules for building sources it contributes
uart_io/%.o: ../uart_io/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


