################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../FreeRTOS_Kernel/portable/MemMang/heap_3.c 

OBJS += \
./FreeRTOS_Kernel/portable/MemMang/heap_3.o 

C_DEPS += \
./FreeRTOS_Kernel/portable/MemMang/heap_3.d 


# Each subdirectory must supply rules for building sources it contributes
FreeRTOS_Kernel/portable/MemMang/%.o: ../FreeRTOS_Kernel/portable/MemMang/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DUSE_STDIO=1 -D__GCC_POSIX__=1 -I../Common_Demo/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O2 -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


