project(FreeRTOS_Emulator)

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

option(TRACE_FUNCTIONS "Trace function calls using instrument-functions")

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

include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/GCC/Posix)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/Gfx/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/AsyncIO/include)
include_directories(${PROJECT_SOURCE_DIR}/lib/tracer/include)

file(GLOB FREERTOS_SOURCES "lib/FreeRTOS_Kernel/*.c"
    "lib/FreeRTOS_Kernel/portable/GCC/Posix/*.c"
    "lib/FreeRTOS_Kernel/portable/MemMang/*.c")
file(GLOB GFX_SOURCES "lib/Gfx/*.c")
file(GLOB ASYNC_SOURCES "lib/AsyncIO/*.c")
file(GLOB SIMULATOR_SOURCES "src/*.c")
add_executable(FreeRTOS_Emulator ${SIMULATOR_SOURCES} ${FREERTOS_SOURCES} ${GFX_SOURCES} ${ASYNC_SOURCES})

if(TRACE_FUNCTIONS)
    add_definitions(-DTRACE_FUNCTIONS)
    SET(GCC_COVERAGE_COMPILE_FLAGS "-finstrument-functions")
    target_compile_options(FreeRTOS_Emulator PUBLIC ${GCC_COVERAGE_COMPILE_FLAGS})
endif(TRACE_FUNCTIONS)

target_link_libraries(FreeRTOS_Emulator ${SDL2_LIBRARIES} ${SDL2_GFX_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
    m ${CMAKE_THREAD_LIBS_INIT} rt)


