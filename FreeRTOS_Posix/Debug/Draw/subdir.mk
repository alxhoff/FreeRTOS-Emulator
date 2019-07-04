################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Draw/Draw_Rect.c \
# ../Draw/Draw_Circle.c \
../Draw/Draw_Ellipse.c \
../Draw/Draw_FillCircle.c \
../Draw/Draw_FillEllipse.c \
../Draw/Draw_FillRound.c \
../Draw/Draw_HLine.c \
../Draw/Draw_Line.c \
../Draw/Draw_Pixel.c \
../Draw/Draw_Round.c \
../Draw/Draw_VLine.c \
../Draw/SDL_draw.c

OBJS += \
../Draw/Draw_Rect.o \
# ../Draw/Draw_Circle.o \
../Draw/Draw_Ellipse.o \
../Draw/Draw_FillCircle.o \
../Draw/Draw_FillEllipse.o \
../Draw/Draw_FillRound.o \
../Draw/Draw_HLine.o \
../Draw/Draw_Line.o \
../Draw/Draw_Pixel.o \
../Draw/Draw_Round.o \
../Draw/Draw_VLine.o \
../Draw/SDL_draw.o

C_DEPS += \
../Draw/Draw_Rect.d \
# ../Draw/Draw_Circle.d \
../Draw/Draw_Ellipse.d \
../Draw/Draw_FillCircle.d \
../Draw/Draw_FillEllipse.d \
../Draw/Draw_FillRound.d \
../Draw/Draw_HLine.d \
../Draw/Draw_Line.d \
../Draw/Draw_Pixel.d \
../Draw/Draw_Round.d \
../Draw/Draw_VLine.d \
../Draw/SDL_draw.d


# Each subdirectory must supply rules for building sources it contributes
Draw/%.o: ../Draw/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D__GCC_POSIX__=1 -DDEBUG_BUILD=1 -DUSE_STDIO=1 -I../Lib/include -I.. -I../FreeRTOS_Kernel/include -I../FreeRTOS_Kernel/portable/GCC/Posix -O0 -g -Wall -c -fmessage-length=0 -pthread -lrt -lSDL2 -Wno-pointer-sign -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


