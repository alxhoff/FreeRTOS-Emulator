################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Demo/Common/Full/BlockQ.c \
../Demo/Common/Full/PollQ.c \
../Demo/Common/Full/death.c \
../Demo/Common/Full/dynamic.c \
../Demo/Common/Full/events.c \
../Demo/Common/Full/flash.c \
../Demo/Common/Full/flop.c \
../Demo/Common/Full/integer.c \
../Demo/Common/Full/print.c \
../Demo/Common/Full/semtest.c 

OBJS += \
./Demo/Common/Full/BlockQ.o \
./Demo/Common/Full/PollQ.o \
./Demo/Common/Full/death.o \
./Demo/Common/Full/dynamic.o \
./Demo/Common/Full/events.o \
./Demo/Common/Full/flash.o \
./Demo/Common/Full/flop.o \
./Demo/Common/Full/integer.o \
./Demo/Common/Full/print.o \
./Demo/Common/Full/semtest.o 

C_DEPS += \
./Demo/Common/Full/BlockQ.d \
./Demo/Common/Full/PollQ.d \
./Demo/Common/Full/death.d \
./Demo/Common/Full/dynamic.d \
./Demo/Common/Full/events.d \
./Demo/Common/Full/flash.d \
./Demo/Common/Full/flop.d \
./Demo/Common/Full/integer.d \
./Demo/Common/Full/print.d \
./Demo/Common/Full/semtest.d 


# Each subdirectory must supply rules for building sources it contributes
Demo/Common/Full/%.o: ../Demo/Common/Full/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../../../Source/include -I../../../Source/portable/GCC/Posix -I../../Common/include -I../. -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


