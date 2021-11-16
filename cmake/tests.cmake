include(ExternalProject)
find_package(Git REQUIRED)

SET(CHECK_FILES
    ${PROJECT_SOURCE_DIR}/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/Gfx/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/Gfx/*.c
    ${PROJECT_SOURCE_DIR}/lib/AsyncIO/include/*.h
    ${PROJECT_SOURCE_DIR}/lib/AsyncIO/*.c
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

    list(APPEND ASTYLE_CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
    )

    ExternalProject_Add(
        astyle
        GIT_REPOSITORY      https://github.com/Bareflank/astyle.git
        GIT_TAG             v1.2
        GIT_SHALLOW         1
        CMAKE_ARGS          ${ASTYLE_CMAKE_ARGS}
        PREFIX              ${CMAKE_BINARY_DIR}/external/astyle/prefix
        TMP_DIR             ${CMAKE_BINARY_DIR}/external/astyle/tmp
        STAMP_DIR           ${CMAKE_BINARY_DIR}/external/astyle/stamp
        DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/astyle/download
        SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/astyle/src
        BINARY_DIR          ${CMAKE_BINARY_DIR}/external/astyle/build
    )

    list(APPEND ASTYLE_ARGS
        --style=linux
        --lineend=linux
        --suffix=none
        --pad-oper
        --unpad-paren
        --break-closing-brackets
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
        --add-brackets
        --break-after-logical
        ${CHECK_FILES}
    )

    if(NOT WIN32 STREQUAL "1")
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    else()
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle.exe ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    endif()

endif()

# ------------------------------------------------------------------------------
# Clang Tidy
# ------------------------------------------------------------------------------

if(ENABLE_CLANG_TIDY)

    find_program(CLANG_TIDY_BIN clang-tidy)
    find_program(RUN_CLANG_TIDY_BIN run-clang-tidy.py
        PATHS /usr/share/clang
    )

    if(CLANG_TIDY_BIN STREQUAL "CLANG_TIDY_BIN-NOTFOUND")
        message(FATAL_ERROR "unable to locate clang-tidy-4.0")
    endif()

    if(RUN_CLANG_TIDY_BIN STREQUAL "RUN_CLANG_TIDY_BIN-NOTFOUND")
        message(FATAL_ERROR "unable to locate run-clang-tidy-4.0.py")
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

    list(APPEND CPPCHECK_CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
    )

    ExternalProject_Add(
        cppcheck
        GIT_REPOSITORY      https://github.com/danmar/cppcheck.git
        GIT_TAG             1.79
        GIT_SHALLOW         1
        CMAKE_ARGS          ${CPPCHECK_CMAKE_ARGS}
        PREFIX              ${CMAKE_BINARY_DIR}/external/cppcheck/prefix
        TMP_DIR             ${CMAKE_BINARY_DIR}/external/cppcheck/tmp
        STAMP_DIR           ${CMAKE_BINARY_DIR}/external/cppcheck/stamp
        DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/cppcheck/download
        SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/src
        BINARY_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/build
    )

    list(APPEND CPPCHECK_ARGS
        --enable=warning,style,performance,portability
        --suppress=arithOperationsOnVoidPointer
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
        COMMAND ${CMAKE_BINARY_DIR}/bin/cppcheck ${CPPCHECK_ARGS}
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
