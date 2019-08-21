project(FreeRTOS_Graphics_Simulator)

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(Threads)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(SDL2_gfx REQUIRED)
include_directories(${SDL2_GFX_INCLUDE_DIRS})
find_package(SDL2_image REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
find_package(SDL2_mixer REQUIRED)
include_directories(${SDL2_MIXER_INCLUDE_DIRS})
find_package(SDL2_ttf REQUIRED)
include_directories(${SDL2_TTF_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/GCC/Posix)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/Gfx/include)

file(GLOB FREERTOS_SOURCES "lib/FreeRTOS_Kernel/*.c"
    "lib/FreeRTOS_Kernel/portable/GCC/Posix/*.c"
    "lib/FreeRTOS_Kernel/portable/MemMang/*.c")
file(GLOB GFX_SOURCES "lib/Gfx/*.c")
file(GLOB SIMULATOR_SOURCES "src/*.c")
add_executable(FreeRTOS_Simulator ${SIMULATOR_SOURCES} ${FREERTOS_SOURCES} ${GFX_SOURCES})

target_link_libraries(FreeRTOS_Simulator ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
    m ${CMAKE_THREAD_LIBS_INIT})


