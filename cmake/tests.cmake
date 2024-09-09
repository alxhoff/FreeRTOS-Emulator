include(ExternalProject)
find_package(Git REQUIRED)

SET(CHECK_FILES
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/Gfx/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/Gfx/*.c
    ${PROJECT_SOURCE_DIR}/lib/AsyncIO/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/AsyncIO/*.c
    ${PROJECT_SOURCE_DIR}/lib/StateMachine/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/StateMachine/*.c
    ${PROJECT_SOURCE_DIR}/lib/tracer/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/LL/*.h
    ${PROJECT_SOURCE_DIR}/src/*.c)

SET(TIDY_SOURCES
    ${PROJECT_SOURCE_DIR}/lib/Gfx
    ${PROJECT_SOURCE_DIR}/lib/AsyncIO
    ${PROJECT_SOURCE_DIR}/src
)

# ------------------------------------------------------------------------------
# All checks
# ------------------------------------------------------------------------------

if(ALL_CHECKS)
    SET(ENABLE_ASTYLE ON)
    SET(ENABLE_CLANG_TIDY ON)
    SET(ENABLE_CPPCHECK ON)
    SET(ENABLE_MEMCHECK ON)
    SET(ENABLE_COVERAGE ON)
    SET(ENABLE_ASAN ON)
    SET(ENABLE_USAN ON)
    SET(ENABLE_TSAN ON)
endif()

# ------------------------------------------------------------------------------
# Git whitespace
# ------------------------------------------------------------------------------


add_custom_target(
    commit
    COMMAND echo COMMAND ${GIT_EXECUTABLE} diff --check HEAD^
    COMMENT "Running git check"
)

# ------------------------------------------------------------------------------
# Astyle
# ------------------------------------------------------------------------------

if(ENABLE_ASTYLE)

    find_program(ASTYLE_BIN astyle)

    if(ASTYLE_BIN STREQUAL "ASTYLE_BIN-NOTFOUND")
        message(FATAL_ERROR "unable to locate astyle")
    endif()

    message(STATUS "astyle binary: ${ASTYLE_BIN}")

    list(APPEND ASTYLE_ARGS
        --style=linux
        --lineend=linux
        --suffix=none
        --pad-oper
        --unpad-paren
        --break-closing-braces
        --align-pointer=name
        --align-reference=name
        --indent-preproc-define
        --indent-switches
        --indent-col1-comments
        --keep-one-line-statements
        --keep-one-line-blocks
        --pad-header
        --convert-tabs
        --min-conditional-indent=0
        --indent=spaces=4
        --close-templates
        --add-braces
        --break-after-logical
        ${CHECK_FILES}
    )

    list(APPEND ASTYLE_VERSION
        --version
    )

    add_custom_target(
        format
        COMMAND ${ASTYLE_BIN} ${ASTYLE_VERSION}
        COMMAND ${ASTYLE_BIN} ${ASTYLE_ARGS}
        COMMENT "running astyle"
    )


endif()

# ------------------------------------------------------------------------------
# Clang Tidy
# ------------------------------------------------------------------------------

if(ENABLE_CLANG_TIDY)

    find_program(CLANG_TIDY_BIN clang-tidy)
    find_program(RUN_CLANG_TIDY_BIN run-clang-tidy
        PATHS /usr/share/clang
    )

    if(CLANG_TIDY_BIN STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
        message(FATAL_ERROR "unable to locate clang-tidy")
    endif()

    if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
        message(WARNING "unable to locate run-clang-tidy")
        find_program(RUN_CLANG_TIDY_BIN run-clang-tidy.py
            PATHS /usr/share/clang
        )
        if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
            message(WARNING "unable to locate run-clang-tidy.py")
            find_program(RUN_CLANG_TIDY_BIN run-clang-tidy-4.0.py
                PATHS /usr/share/clang
            )
            if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
                message(WARNING "unable to locate run-clang-tidy-4.0.py")
                find_program(RUN_CLANG_TIDY_BIN run-clang-tidy-8
                    PATHS /usr/share/clang
                )
                if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
                    message(FATAL_ERROR "unable to locate run-clang-tidy")
                endif()
            endif()
        endif()
    endif()

    SET(FILTER_REGEX "^((?!Kernel).)*$$")

    list(APPEND RUN_CLANG_TIDY_BIN_ARGS
        -clang-tidy-binary ${CLANG_TIDY_BIN}
        -header-filter="${FILTER_REGEX}"
        -checks=clan*,cert*,misc*,perf*,cppc*,read*,mode*,-cert-err58-cpp,-misc-noexcept-move-constructor
    )

    add_custom_target(
        tidy
        COMMAND ${RUN_CLANG_TIDY_BIN} ${RUN_CLANG_TIDY_BIN_ARGS} ${TIDY_SOURCES}
        COMMENT "running clang tidy"
    )

    add_custom_target(
        tidy_list
        COMMAND ${RUN_CLANG_TIDY_BIN} ${RUN_CLANG_TIDY_BIN_ARGS} -export-fixes=tidy.fixes -style=file ${TIDY_SOURCES}
        COMMENT "running clang tidy"
    )

endif()

# ------------------------------------------------------------------------------
# CppCheck
# ------------------------------------------------------------------------------

if(ENABLE_CPPCHECK)

    find_program(CPP_CHECK_BIN cppcheck)

    if(CPP_CHECK_BIN STREQUAL "CPP_CHECK_BIN-NOTFOUND")
        message(FATAL_ERROR "unable to locate clang-tidy")
    endif()

    message(STATUS "cppcheck binary: ${CPP_CHECK_BIN}")

    list(APPEND CPPCHECK_ARGS
        --enable=warning,style,performance,portability
        --suppress=arithOperationsOnVoidPointer
        --suppress=unusedStructMember
        --suppress=arrayIndexOutOfBounds
        --std=c99
        --verbose
        --error-exitcode=1
        --language=c
        --xml
        --inline-suppr
        -DMAIN=main
        -I ${CHECK_FILES}
    )

    add_custom_target(
        check
        COMMAND ${CPP_CHECK_BIN} ${CPPCHECK_ARGS}
        COMMENT "running cppcheck"
    )

endif()

# ------------------------------------------------------------------------------
# Valgrind
# ------------------------------------------------------------------------------

if(ENABLE_MEMCHECK)

    set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
    set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
    set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
    set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")

    find_program(CTEST ctest)

    if(CTEST STREQUAL "CTEST-NOTFOUND")
        message(FATAL_ERROR "unable to locate ctest")
    endif()

    include(CTest)
    add_test(emulator_memtest ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/FreeRTOS_Emulator)

    add_custom_target(
        memcheck
        COMMAND ${CTEST} -T memcheck
        COMMENT "running cppcheck"
    )
endif()

# ------------------------------------------------------------------------------
# Coverage
# ------------------------------------------------------------------------------

if(ENABLE_COVERAGE)
    set(COVERAGE_FLAGS "-g ")
    set(COVERAGE_FLAGS "${COVERAGE_FLAGS} -O0")
    set(COVERAGE_FLAGS "${COVERAGE_FLAGS} -fprofile-arcs")
    set(COVERAGE_FLAGS "${COVERAGE_FLAGS} -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COVERAGE_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# ------------------------------------------------------------------------------
# Google Sanitizers
# ------------------------------------------------------------------------------

set(OLD_C_FLAGS ${CMAKE_C_FLAGS})

if(ENABLE_ASAN)
    set(ASAN_FLAGS "-g")
    set(ASAN_FLAGS "${ASAN_FLAGS} -O1")
    set(ASAN_FLAGS "${ASAN_FLAGS} -fuse-ld=gold")
    set(ASAN_FLAGS "${ASAN_FLAGS} -fno-omit-frame-pointer")
    set(ASAN_FLAGS "${ASAN_FLAGS} -fsanitize=address")
    set(ASAN_FLAGS "${ASAN_FLAGS} -fsanitize=leak")
    if(ALL_CHECKS)
        set(CMAKE_C_FLAGS ${ASAN_FLAGS})
        add_executable(${CMAKE_PROJECT_NAME}_asan ${PROJECT_SOURCES})
        target_link_libraries(${CMAKE_PROJECT_NAME}_asan ${PROJECT_LIBRARIES})
    else()
        if(ENABLE_USAN OR ENABLE_TSAN)
            MESSAGE(FATAL_ERROR "ENABLE_ASAN, ENABLE_USAN and ENABLE_TSAN are XOR")
        else()
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
        endif()
    endif()
endif()

if(ENABLE_USAN)
    set(USAN_FLAGS "-fuse-ld=gold")
    set(USAN_FLAGS "${USAN_FLAGS} -fsanitize=undefined")

    if(ALL_CHECKS)
        set(CMAKE_C_FLAGS ${USAN_FLAGS})
        add_executable(${CMAKE_PROJECT_NAME}_usan ${PROJECT_SOURCES})
        target_link_libraries(${CMAKE_PROJECT_NAME}_usan ${PROJECT_LIBRARIES})
    else()
        if(ENABLE_ASAN OR ENABLE_TSAN)
            MESSAGE(FATAL_ERROR "ENABLE_ASAN, ENABLE_USAN and ENABLE_TSAN are XOR")
        else()
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USAN_FLAGS}")
        endif()
    endif()
endif()

if(ENABLE_TSAN)
    set(TSAN_FLAGS "-fuse-ld=gold")
    set(TSAN_FLAGS "${TSAN_FLAGS} -fsanitize=thread")

    if(ALL_CHECKS)
        set(CMAKE_C_FLAGS ${TSAN_FLAGS})
        add_executable(${CMAKE_PROJECT_NAME}_tsan ${PROJECT_SOURCES})
        target_link_libraries(${CMAKE_PROJECT_NAME}_tsan ${PROJECT_LIBRARIES})
    else()
        if(ENABLE_USAN OR ENABLE_ASAN)
            MESSAGE(FATAL_ERROR "ENABLE_ASAN, ENABLE_USAN and ENABLE_TSAN are XOR")
        else()
            SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TSAN_FLAGS}")
        endif()
    endif()
endif()

if(ALL_CHECKS)
    set(CMAKE_C_FLAGS ${OLD_C_FLAGS})
endif()
