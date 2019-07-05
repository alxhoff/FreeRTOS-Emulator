################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include FreeRTOS_Kernel/portable/MemMang/subdir.mk
-include FreeRTOS_Kernel/portable/GCC/Posix/subdir.mk
-include FreeRTOS_Kernel/subdir.mk
-include Draw/subdir.mk
-include Build/FreeRTOS_Kernel/portable/MemMang/subdir.mk
-include Build/FreeRTOS_Kernel/portable/GCC/Posix/subdir.mk
-include Build/FreeRTOS_Kernel/subdir.mk
-include Build/AsyncIO/subdir.mk
-include Build/subdir.mk
-include AsyncIO/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: FreeRTOS_Posix

# Tool invocations
FreeRTOS_Posix: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -pthread -lSDL2 -lSDL2_image -lSDL2_ttf -lrt -o "FreeRTOS_Posix" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) FreeRTOS_Posix
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
