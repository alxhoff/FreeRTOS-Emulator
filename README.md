# FreeRTOS Emulator

<p>
  <a href="https://travis-ci.com/alxhoff/FreeRTOS-Emulator">
  <img src="https://travis-ci.com/alxhoff/FreeRTOS-Emulator.svg?branch=master">
  </a>
  <a href="https://github.com/alxhoff/FreeRTOS-Emulator/blob/master/LICENSE">
    <img src="https://img.shields.io/badge/license-GPLv3-blue.svg" />
  </a>
  <a href="https://scan.coverity.com/projects/alxhoff-freertos-emulator">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/20757/badge.svg"/>
  </a>
  <a href="https://codecov.io/gh/alxhoff/FreeRTOS-Emulator">
  <img src="https://codecov.io/gh/alxhoff/FreeRTOS-Emulator/branch/master/graph/badge.svg" />
</a>
</p>

An implementation of POSIX based FreeRTOS with the combination of SDL2 graphics. Aimed at providing an x86 emulation solution for teaching FreeRTOS to students without the need of embedded hardware.

Based on the FreeRTOS (V5.X) simulator developed by William Davy. Updated to use FreeRTOS V9.0.0.

Checkout the [Wiki page](../../wiki) for a detailed Documentation!

Doxygen documentation can also be found on the [GitHub Pages](ihttps://alxhoff.github.io/FreeRTOS-Emulator/) page.

## Dependencies

The simulator uses the SDL2 graphics libraries.

### Debian/Ubuntu

```bash
sudo apt-get install libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev
```

Additional requirements for development:
```bash
# Depending on your OS version you might have to add the llvm-toolchain-4.0 APT source before
sudo apt-get install clang-4.0 clang-tidy-4.0
```

### Arch

```bash
sudo pacman -S sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_ttf
```

Additional requirements for development:
``` bash
sudo pacman -S clang
```


### Windows/Mac
¯\\(°_o)/¯

....install linux?

## Building

```bash
cd build
cmake ..
make
```

For those requiring an IDE run
```bash
cmake -G "Eclipse CDT4 - Unix Makefiles" ./
```
to generate the appropriate project files to allow for the emulator to be imported into Eclipse.

### Additional targets

In [`test.cmake`](cmake/test.cmake) a number of extra targets are provided to help with linting.

#### Git --check

``` bash
make commit
```

Checks for whitespaces and empty lines.

#### Astyle Formatting

``` bash
cmake -DENABLE_ASTYLE=ON ..
make format
```

Invokes the Astyle formatter.

#### Clang Tidy

``` bash
cmake -DENABLE_CLANG_TIDY=ON ..
make tidy
```

Uses clang tidy to find style violations, interface misuse of bugs found via static analysis.

To generate a list of warnings/errors use the build target `tidy_list` and then view the file `tidy.fixes`.

#### CppCheck

``` bash
cmake -DENABLE_CPPCHECK=ON ..
make check
```

Code analysis with CppCheck, focusing on undefined behaviour bugs.

#### Valgrind (memcheck)

``` bash
cmake -DENABLE_MEMCHECK=ON ..
make memcheck
```

Memory checker.

#### Google Tests/Coverage

Coverage

``` bash
cmake -DENABLE_COVERAGE=ON ..
make
```

Each sanitizer must be run stand alone, thus you cannot run them together.

Address sanitizer

``` bash
cmake -DENABLE_ASAN=ON ..
make
```

Undefined behaviour sanitizer

``` bash
cmake -DENABLE_USAN=ON ..
make
```

Thread sanitizer

``` bash
cmake -DENABLE_TSAN=ON ..
make
```

### All checks

The target `make all_checks`

``` bash
cmake -DALL_CHECKS=ON ..
make
```

will perform all checks

## Running

The binary will be created inside a `bin` folder. The emulator should be run from this folder becuase at the moment the Gfx libraries rely on hardcoded resource paths for things such as fonts. As such to run perform the following.

``` bash
cd bin
./FreeRTOS_Emulator
```

## Debugging

The emulator uses the signals `SIGUSR1` and `SIG34` and as such GDB needs to be told to ignore the signal.
An appropriate `.gdbinit` is in the `bin` directory.
Copy the `.gdbinit` into your home directory or make sure to debug from the `bin` directory.
Such that GDB does not get interrupted by the POSIX signals used by the emulator for IPC.

If using an IDE, make sure to configure your debug to load the gdbinit file.

---

<a href="https://www.buymeacoffee.com/xmyWYwD" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/lato-green.png" alt="Buy Me A Coffee" style="height: 11px !important;" ></a>
