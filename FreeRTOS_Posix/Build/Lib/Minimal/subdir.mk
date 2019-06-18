################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/Minimal/countsem.c \
../Lib/Minimal/crflash.c \
../Lib/Minimal/crhook.c \
../Lib/Minimal/recmutex.c 

OBJS += \
./Lib/Minimal/countsem.o \
./Lib/Minimal/crflash.o \
./Lib/Minimal/crhook.o \
./Lib/Minimal/recmutex.o 

C_DEPS += \
./Lib/Minimal/countsem.d \
./Lib/Minimal/crflash.d \
./Lib/Minimal/crhook.d \
./Lib/Minimal/recmutex.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/Minimal/%.o: ../Lib/Minimal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../Lib/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -Wno-pointer-sign -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


