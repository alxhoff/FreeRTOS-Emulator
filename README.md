# FreeRTOS-Graphics-Simulator

An implementation of POSIX based FreeRTOS with the combination of SDL2 graphics. Aimed at providing an x86 simulation solution for teaching FreeRTOS to students without the need of embedded hardware.

Based on the FreeRTOS (V5.X) simulator developed by William Davy. Updated to use FreeRTOS V9.0.0.

## Dependencies

The simulator uses the SDL2 graphics libraries.

```bash
sudo apt-get install libsdl2-ttf-dev libsdl2-mixer-dev libsdl2-image-dev libsdl2-gfx-dev libsdl2-dev

```

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

an appropriate .gdbinit is in the bin directory.

## Example

Pong game implementation on pong branch

Work in progress.
