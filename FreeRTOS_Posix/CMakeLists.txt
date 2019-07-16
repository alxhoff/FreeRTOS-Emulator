project(FreeRTOS_Graphics_Simulator)

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(SDL2 REQUIRED)
MESSAGE(STATUS "SDL2 Libraries: " ${SDL2_LIBRARIES})
MESSAGE(STATUS "SDL2 Includes: " ${SDL2_INCLUDE_DIRS})
include_directories(${SDL2_INCLUDE_DIRS})

find_package(SDL2_gfx REQUIRED)
MESSAGE(STATUS "SDL2_gfx Libraries: " ${SDL2_GFX_LIBRARIES})
MESSAGE(STATUS "SDL2_gfx Includes: " ${SDL2_GFX_INCLUDE_DIRS})
include_directories(${SDL2_GFX_INCLUDE_DIRS})

find_package(SDL2_ttf REQUIRED)
MESSAGE(STATUS "SDL2_ttf Libraries: " ${SDL2_TTF_LIBRARIES})
MESSAGE(STATUS "SDL2_ttf Includes: " ${SDL2_TTF_INCLUDE_DIRS})
include_directories(${SDL2_TTF_INCLUDE_DIRS})

find_package(SDL2_image REQUIRED)
MESSAGE(STATUS "SDL2_image Libraries: " ${SDL2_IMAGE_LIBRARIES})
MESSAGE(STATUS "SDL2_image Includes: " ${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

#INCLUDES
include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/GCC/Posix)
include_directories(${PROJECT_SOURCE_DIR}/lib/AsyncIO/include)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/Gfx/include)

file(GLOB FREERTOS_SOURCES "lib/FreeRTOS_Kernel/*.c"
    "lib/FreeRTOS_Kernel/portable/GCC/Posix/*.c"
    "lib/FreeRTOS_Kernel/portable/MemMang/*.c"
    "lib/AsyncIO/*.c")

add_library(FREERTOS SHARED ${FREERTOS_SOURCES})

MESSAGE(STATUS "FreeRTOS Sources: " ${FREERTOS_SOURCES})

file(GLOB GFX_SOURCES "lib/Gfx/*.c")

add_library(GFX SHARED ${GFX_SOURCES})

MESSAGE(STATUS "Gfx Sources: " ${GFX_SOURCES})

file(GLOB SIMULATOR_SOURCES "src/*.c")

MESSAGE(STATUS "Simulator Sources: " ${SIMULATOR_SOURCES})

add_executable(FreeRTOS_Simulator ${SIMULATOR_SOURCES})

target_link_libraries(FreeRTOS_Simulator ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES}
    ${SDL2_TTF_LIBRARIES} ${SDL2_IMAGE_LIBRARIES} FREERTOS GFX)


