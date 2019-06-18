################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/portable/GCC/Posix/._port.c \
/home/williamd/tmp/FreeRTOSV5.2.0/Source/portable/GCC/Posix/port.c 

OBJS += \
./Kernel_Common/portable/GCC/Posix/._port.o \
./Kernel_Common/portable/GCC/Posix/port.o 

C_DEPS += \
./Kernel_Common/portable/GCC/Posix/._port.d \
./Kernel_Common/portable/GCC/Posix/port.d 


# Each subdirectory must supply rules for building sources it contributes
Kernel_Common/portable/GCC/Posix/._port.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/portable/GCC/Posix/._port.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Kernel_Common/portable/GCC/Posix/port.o: /home/williamd/tmp/FreeRTOSV5.2.0/Source/portable/GCC/Posix/port.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


