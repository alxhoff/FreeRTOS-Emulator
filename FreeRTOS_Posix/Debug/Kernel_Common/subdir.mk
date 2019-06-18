################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/._croutine.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/._list.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/._queue.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/._tasks.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/croutine.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/list.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/queue.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/tasks.c 

OBJS += \
./Kernel_Common/._croutine.o \
./Kernel_Common/._list.o \
./Kernel_Common/._queue.o \
./Kernel_Common/._tasks.o \
./Kernel_Common/croutine.o \
./Kernel_Common/list.o \
./Kernel_Common/queue.o \
./Kernel_Common/tasks.o 

C_DEPS += \
./Kernel_Common/._croutine.d \
./Kernel_Common/._list.d \
./Kernel_Common/._queue.d \
./Kernel_Common/._tasks.d \
./Kernel_Common/croutine.d \
./Kernel_Common/list.d \
./Kernel_Common/queue.d \
./Kernel_Common/tasks.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel_Common/._croutine.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/._croutine.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/._list.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/._list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/._queue.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/._queue.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/._tasks.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/._tasks.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/croutine.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/croutine.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/list.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/queue.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/queue.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/tasks.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/tasks.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


