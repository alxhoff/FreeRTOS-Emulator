################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Kernel_Source/croutine.c \
../Kernel_Source/list.c \
../Kernel_Source/queue.c \
../Kernel_Source/tasks.c 

OBJS += \
./Kernel_Source/croutine.o \
./Kernel_Source/list.o \
./Kernel_Source/queue.o \
./Kernel_Source/tasks.o 

C_DEPS += \
./Kernel_Source/croutine.d \
./Kernel_Source/list.d \
./Kernel_Source/queue.d \
./Kernel_Source/tasks.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel_Source/%.o: ../Kernel_Source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../../../Source/include -I../../../Source/portable/GCC/Posix -I../../Common/include -I../. -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


