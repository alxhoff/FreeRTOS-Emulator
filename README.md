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

An implementation of POSIX based FreeRTOS with the combination of SDL2 graphics. Aimed at providing an x86 emulation solution for teaching FreeRTOS to students without the need of embedded hardware. Used at the Technical University of Munich in the teaching of the "Embedded Systems Programming Lab". Please excuse any references to "students" or "course" if you're not one of my students.

Based on the FreeRTOS (V5.X) simulator developed by William Davy. Updated to use FreeRTOS V9.0.0.

Checkout the [Wiki page](../../wiki) for a detailed Documentation!

Doxygen documentation can also be found on the [GitHub Pages](ihttps://alxhoff.github.io/FreeRTOS-Emulator/) page.

## Dependencies

The simulator uses the SDL2 graphics libraries.

### Debian/Ubuntu

Assuming that you have some basic utilities like `make`, `cmake` and `git` already installed, execute:

```bash
sudo apt-get install build-essential libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev
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

(See [Wiki](../../wiki/Home))

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

We also includes templates for Configuring VSCode automatically. Run the following in the `build` directory to install them:

```bash
cmake -DUSE_IDE=vscode ..
```

Further Information: [Development-Environment](../../wiki/Development-Environment)

### Additional targets

#### Documentation

Doxygen documentation, found in the [docs](docs) folder, can be generated from cmake/make by passing the variable `DOCS=on` and making the target `docs`.

``` bash
cmake -DDOCS=on ..
make docs
```

#### Tests

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

**Warning:** The default version of CMake which is installed f.e. on Ubuntu 16.04 will throw an arror when running make to setup the `bin/astyle` binary. Please upgrade to a newrer version manually if required:

```bash
VERSION=3.16
BUILD=5
wget -q https://cmake.org/files/v$VERSION/cmake-$VERSION.$BUILD-Linux-x86_64.sh
mkdir /opt/cmake
sh cmake-$VERSION.$BUILD-Linux-x86_64.sh --prefix=/opt/cmake --skip-license --exclude-subdir
ln -s /opt/cmake/bin/cmake /usr/local/bin/cmake
rm cmake-$VERSION.$BUILD-Linux-x86_64.sh
```

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

## Tracing

*Note: this is experiemental and proves to be unstable with the AIO libraries, it was used during development of the emulator and provides a novel function for small experiements, it should not be used for serious debugging of the entire emulator as this will cause errors.*

Tracing, found in [lib/tracer](lib/tracer) is instrumented using GCC's function instrumentation.

Running

``` bash
cmake -DTRACE_FUNCTIONS=ON ..
````

will include the constructor, destructor and the needed `__cyg_profile_func_xxx` functions that are called upon entry and exit of any functions called during the execution of the program.
These callback functions are implemented to log the function, caller and timestamp of each function calls, written to a file named `trace.out`.
As the values written out are memory offsets and, as such, are not human readable the trace dump must be processed by the script [`readtracelog.sh`](lib/tracer/readtracelog.sh) which uses [`addr2line`](https://linux.die.net/man/1/addr2line) to convert the memory offsets into human readable function calls, done using the memory map of the compiled executable.

### Example

Adding something like

``` bash
+void printhello(void)
+{
+    printf("hello");
+}
+
 int main(int argc, char *argv[])
 {
+    printhello();
```

To your code and then compiling and running the executable, after passing `-DTRACE_FUNCTIONS=ON` to cmake of course, you will be presented with an output similar to

```bash
x 0x55e138f918a9 0x55e138f9fe4d 1587039262
e 0x55e138f92f72 0x7f2117466023 1587039262
e 0x55e138f92f34 0x55e138f92f99 1587039262
x 0x55e138f92f34 0x55e138f92f99 1587039262
e 0x55e138f918f0 0x7f2117bb242b 1587039262
```

After processing using the provided script sorcery, running something such as

``` bash
./readtracelog.sh ../../bin/FreeRTOS_Emulator ../../build/trace.out
```

You will see a human readable output that logs the function entries and exit made in the program

``` bash
Exit  trace_begin at 2020-04-16T14:14:22+02:00
Enter main at 2020-04-16T14:14:22+02:00, called from ?? (??:0)
Enter printhello at 2020-04-16T14:14:22+02:00, called from main (main.c:624)
Exit  printhello at 2020-04-16T14:14:22+02:00
Enter trace_end at 2020-04-16T14:14:22+02:00, called from ?? (??:0)
```

Note that the ?? visible in the output above are the result of the function instrumentation only being able to map to functions compiled using the `-finstrument-functions` compile flag.
Extenal libraries etc are only linked against and not compiled using this flag, therefore they cannot be instrumented.

---

<a href="https://www.buymeacoffee.com/xmyWYwD" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/lato-green.png" alt="Buy Me A Coffee" style="height: 11px !important;" ></a>
