################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Demo/Common/Minimal/GenQTest.c \
../Demo/Common/Minimal/QPeek.c \
../Demo/Common/Minimal/blocktim.c \
../Demo/Common/Minimal/countsem.c \
../Demo/Common/Minimal/crflash.c \
../Demo/Common/Minimal/crhook.c \
../Demo/Common/Minimal/recmutex.c 

OBJS += \
./Demo/Common/Minimal/GenQTest.o \
./Demo/Common/Minimal/QPeek.o \
./Demo/Common/Minimal/blocktim.o \
./Demo/Common/Minimal/countsem.o \
./Demo/Common/Minimal/crflash.o \
./Demo/Common/Minimal/crhook.o \
./Demo/Common/Minimal/recmutex.o 

C_DEPS += \
./Demo/Common/Minimal/GenQTest.d \
./Demo/Common/Minimal/QPeek.d \
./Demo/Common/Minimal/blocktim.d \
./Demo/Common/Minimal/countsem.d \
./Demo/Common/Minimal/crflash.d \
./Demo/Common/Minimal/crhook.d \
./Demo/Common/Minimal/recmutex.d 


# Each subdirectory must supply rules for building sources it contributes
Demo/Common/Minimal/%.o: ../Demo/Common/Minimal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../../../Source/include -I../../../Source/portable/GCC/Posix -I../../Common/include -I../. -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


