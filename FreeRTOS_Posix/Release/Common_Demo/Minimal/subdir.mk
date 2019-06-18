################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Common_Demo/Minimal/GenQTest.c \
../Common_Demo/Minimal/QPeek.c \
../Common_Demo/Minimal/blocktim.c \
../Common_Demo/Minimal/countsem.c \
../Common_Demo/Minimal/crflash.c \
../Common_Demo/Minimal/crhook.c \
../Common_Demo/Minimal/recmutex.c 

OBJS += \
./Common_Demo/Minimal/GenQTest.o \
./Common_Demo/Minimal/QPeek.o \
./Common_Demo/Minimal/blocktim.o \
./Common_Demo/Minimal/countsem.o \
./Common_Demo/Minimal/crflash.o \
./Common_Demo/Minimal/crhook.o \
./Common_Demo/Minimal/recmutex.o 

C_DEPS += \
./Common_Demo/Minimal/GenQTest.d \
./Common_Demo/Minimal/QPeek.d \
./Common_Demo/Minimal/blocktim.d \
./Common_Demo/Minimal/countsem.d \
./Common_Demo/Minimal/crflash.d \
./Common_Demo/Minimal/crhook.d \
./Common_Demo/Minimal/recmutex.d 


# Each subdirectory must supply rules for building sources it contributes
Common_Demo/Minimal/%.o: ../Common_Demo/Minimal/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DUSE_STDIO=1 -D__GCC_POSIX__=1 -I../Common_Demo/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O2 -Wall -c -fmessage-length=0 -pthread -lrt -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


