#ifndef __EMULATOR_CONFIG_H__
#define __EMULATOR_CONFIG_H__

#define WINDOW_TITLE "My FreeRTOS Emulator"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
// Relative to bin directory
#define RESOURCES_DIRECTORY "../resources"
// Relative to resources directory
#define FONTS_DIRECTORY "fonts"
#define DEFAULT_FONT "IBMPlexSans-Medium.ttf"
#define DEFAULT_FONT_SIZE 15

#define configFPS_LIMIT 1
#define configFPS_LIMIT_RATE 50

#endif //__EMULATOR_CONFIG_H__
