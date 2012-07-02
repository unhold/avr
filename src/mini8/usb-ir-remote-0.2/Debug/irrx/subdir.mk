################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../irrx/irrx_bitlength.c 

OBJS += \
./irrx/irrx_bitlength.o 

C_DEPS += \
./irrx/irrx_bitlength.d 


# Each subdirectory must supply rules for building sources it contributes
irrx/%.o: ../irrx/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


