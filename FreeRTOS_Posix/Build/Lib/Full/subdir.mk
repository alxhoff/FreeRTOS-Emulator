################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Lib/Full/BlockQ.c \
../Lib/Full/death.c \
../Lib/Full/dynamic.c \
../Lib/Full/flash.c \
../Lib/Full/integer.c \
../Lib/Full/print.c 

OBJS += \
./Lib/Full/BlockQ.o \
./Lib/Full/death.o \
./Lib/Full/dynamic.o \
./Lib/Full/flash.o \
./Lib/Full/integer.o \
./Lib/Full/print.o 

C_DEPS += \
./Lib/Full/BlockQ.d \
./Lib/Full/death.d \
./Lib/Full/dynamic.d \
./Lib/Full/flash.d \
./Lib/Full/integer.d \
./Lib/Full/print.d 


# Each subdirectory must supply rules for building sources it contributes
Lib/Full/%.o: ../Lib/Full/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../Lib/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -Wno-pointer-sign -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


