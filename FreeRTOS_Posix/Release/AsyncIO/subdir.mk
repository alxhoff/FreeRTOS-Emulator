################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../AsyncIO/AsyncIO.c \
../AsyncIO/AsyncIOSerial.c \
../AsyncIO/AsyncIOSocket.c \
../AsyncIO/PosixMessageQueueIPC.c 

OBJS += \
./AsyncIO/AsyncIO.o \
./AsyncIO/AsyncIOSerial.o \
./AsyncIO/AsyncIOSocket.o \
./AsyncIO/PosixMessageQueueIPC.o 

C_DEPS += \
./AsyncIO/AsyncIO.d \
./AsyncIO/AsyncIOSerial.d \
./AsyncIO/AsyncIOSocket.d \
./AsyncIO/PosixMessageQueueIPC.d 


# Each subdirectory must supply rules for building sources it contributes
AsyncIO/%.o: ../AsyncIO/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DUSE_STDIO=1 -D__GCC_POSIX__=1 -I../Common_Demo/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O2 -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


