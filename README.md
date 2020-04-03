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
</p>

An implementation of POSIX based FreeRTOS with the combination of SDL2 graphics. Aimed at providing an x86 emulation solution for teaching FreeRTOS to students without the need of embedded hardware.

Based on the FreeRTOS (V5.X) simulator developed by William Davy. Updated to use FreeRTOS V9.0.0.


Checkout the [Wiki page](../../wiki) for a detailed Documentation!

## Dependencies

The simulator uses the SDL2 graphics libraries.

### Debian/Ubuntu

```bash
sudo apt-get install libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev

```
### Arch

```bash
sudo pacman -S sdl2 sdl2_gfx sdl2_image sdl2_mixer sdl2_ttf
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

## Debugging

The emulator uses the signals `SIGUSR1` and `SIG34` and as such GDB needs to be told to ignore the signal.
An appropriate `.gdbinit` is in the `bin` directory.
Copy the `.gdbinit` into your home directory or make sure to debug from the `bin` directory.
Such that GDB does not get interrupted by the POSIX signals used by the emulator for IPC.

If using an IDE, make sure to configure your debug to load the gdbinit file.

---

<a href="https://www.buymeacoffee.com/xmyWYwD" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/lato-green.png" alt="Buy Me A Coffee" style="height: 11px !important;" ></a>
