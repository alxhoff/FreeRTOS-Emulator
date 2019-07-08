################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Draw/TUM_Draw.c 

OBJS += \
./Draw/TUM_Draw.o 

C_DEPS += \
./Draw/TUM_Draw.d 


# Each subdirectory must supply rules for building sources it contributes
Draw/%.o: ../Draw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../Draw/include -I../Lib/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -Wno-pointer-sign -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


