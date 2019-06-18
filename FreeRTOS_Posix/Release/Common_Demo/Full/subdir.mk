################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common_Demo/Full/BlockQ.c \
../Common_Demo/Full/PollQ.c \
../Common_Demo/Full/death.c \
../Common_Demo/Full/dynamic.c \
../Common_Demo/Full/events.c \
../Common_Demo/Full/flash.c \
../Common_Demo/Full/flop.c \
../Common_Demo/Full/integer.c \
../Common_Demo/Full/print.c \
../Common_Demo/Full/semtest.c 

OBJS += \
./Common_Demo/Full/BlockQ.o \
./Common_Demo/Full/PollQ.o \
./Common_Demo/Full/death.o \
./Common_Demo/Full/dynamic.o \
./Common_Demo/Full/events.o \
./Common_Demo/Full/flash.o \
./Common_Demo/Full/flop.o \
./Common_Demo/Full/integer.o \
./Common_Demo/Full/print.o \
./Common_Demo/Full/semtest.o 

C_DEPS += \
./Common_Demo/Full/BlockQ.d \
./Common_Demo/Full/PollQ.d \
./Common_Demo/Full/death.d \
./Common_Demo/Full/dynamic.d \
./Common_Demo/Full/events.d \
./Common_Demo/Full/flash.d \
./Common_Demo/Full/flop.d \
./Common_Demo/Full/integer.d \
./Common_Demo/Full/print.d \
./Common_Demo/Full/semtest.d 


# Each subdirectory must supply rules for building sources it contributes
Common_Demo/Full/%.o: ../Common_Demo/Full/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DUSE_STDIO=1 -D__GCC_POSIX__=1 -I../Common_Demo/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O2 -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


