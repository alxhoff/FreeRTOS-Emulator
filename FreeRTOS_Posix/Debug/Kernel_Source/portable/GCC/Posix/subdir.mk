################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Kernel_Source/portable/GCC/Posix/port.c 

OBJS += \
./Kernel_Source/portable/GCC/Posix/port.o 

C_DEPS += \
./Kernel_Source/portable/GCC/Posix/port.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel_Source/portable/GCC/Posix/%.o: ../Kernel_Source/portable/GCC/Posix/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../../../Source/include -I../../../Source/portable/GCC/Posix -I../../Common/include -I../. -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


