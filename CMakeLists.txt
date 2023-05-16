project(FreeRTOS_Emulator)

cmake_minimum_required(VERSION 3.4 FATAL_ERROR)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_CONFIG_DIR ${PROJECT_SOURCE_DIR}/config)

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT)
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --remote --merge
            --merge)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()


if("${TOOLCHAIN}" STREQUAL "docker" OR "${TOOLCHAIN}" STREQUAL "vm")
    include(${CMAKE_MODULE_PATH}/init_remote_projects.cmake)
else()

    if(DOCS_ONLY)
        find_package(Doxygen REQUIRED)

        add_custom_target(
            docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/docs/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    else()

        set(CMAKE_EXPORT_COMPILE_COMMANDS ON )
        set(CMAKE_BUILD_TYPE "Debug")
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

        add_compile_options("-Wall" "-O0")

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

        SET(PROJECT_INCLUDES
            ${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/include
            ${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/GCC/Posix
            ${PROJECT_SOURCE_DIR}/lib/Gfx/include
            ${PROJECT_SOURCE_DIR}/lib/AsyncIO/include
            ${PROJECT_SOURCE_DIR}/lib/StateMachine/include
            ${PROJECT_SOURCE_DIR}/lib/tracer/include
            ${PROJECT_SOURCE_DIR}/lib/LL
            ${PROJECT_SOURCE_DIR}/include
        )

        include_directories(${PROJECT_INCLUDES})

        file(GLOB FREERTOS_SOURCES
            "${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/*.c"
            "${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/GCC/Posix/*.c"
            "${PROJECT_SOURCE_DIR}/lib/FreeRTOS_Kernel/portable/MemMang/heap_3.c")
        file(GLOB GFX_SOURCES "${PROJECT_SOURCE_DIR}/lib/Gfx/*.c")
        file(GLOB STATE_MACHINE_SOURCES "${PROJECT_SOURCE_DIR}/lib/StateMachine/*.c")
        file(GLOB ASYNC_SOURCES "${PROJECT_SOURCE_DIR}/lib/AsyncIO/*.c")
        file(GLOB SIMULATOR_SOURCES "${PROJECT_SOURCE_DIR}/src/*.c")

        SET(PROJECT_SOURCES
            ${SIMULATOR_SOURCES} ${FREERTOS_SOURCES} ${GFX_SOURCES} ${STATE_MACHINE_SOURCES} ${ASYNC_SOURCES}
        )

        set(PROJECT_LIBRARIES
            ${SDL2_LIBRARIES}
            ${SDL2_GFX_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
            ${SDL2_MIXER_LIBRARIES}
            ${SDL2_TTF_LIBRARIES}
            m
            ${CMAKE_THREAD_LIBS_INIT}
            rt
        )

        include(${CMAKE_MODULE_PATH}/tests.cmake)

        add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_SOURCES})

        if(TRACE_FUNCTIONS)
            add_definitions(-DTRACE_FUNCTIONS)
            SET(GCC_COVERAGE_COMPILE_FLAGS "-finstrument-functions")
            target_compile_options(FreeRTOS_Emulator PUBLIC ${GCC_COVERAGE_COMPILE_FLAGS})
        endif(TRACE_FUNCTIONS)

        target_link_libraries(${CMAKE_PROJECT_NAME} ${PROJECT_LIBRARIES})

        if(DOCS)
        find_package(Doxygen REQUIRED)

        add_custom_target(
            docs
            COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/docs/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/docs
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
        endif()

        include(${CMAKE_MODULE_PATH}/init_local_projects.cmake)
        endif()

endif()
