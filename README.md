# FreeRTOS CORTEX MPS2 QEMU Project

This repository contains the FreeRTOS project for the ARM Cortex-M7 using QEMU for emulation. This includes all previous iterations of code and several self-made demos to test different features.

## Goal of the Project

The goal of this project is to demonstrate a simulated LED blinker. Users can run the project with the debugger and toggle the `xBlinkingEnabled` variable to simulate toggling the LED.

## Directory Structure

- **FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC**: Contains all previous iterations of code and various self-made demos.
- **FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc**: Current version directory.

## Building the Project

To build the project, navigate to the build directory and run the following commands:

```bash
cd FreeRTOSv202212.01/FreeRTOS/Demo/CORTEX_MPS2_QEMU_IAR_GCC/build/gcc
make
make clean
```
## Running the Project
### Running QEMU without Debugger

To run QEMU without the debugger, use the following command:
```bash
qemu-system-arm -machine mps2-an500 -cpu cortex-m7 -kernel output/RTOSDemo.out -monitor none -nographic -serial stdio
```
### Running QEMU with Debugger

To run QEMU with the debugger, use the following commands:

First, in one terminal, start QEMU with the following command:
```bash
qemu-system-arm -machine mps2-an500 -cpu cortex-m7 -kernel output/RTOSDemo.out -monitor none -nographic -serial stdio -s -S
```
Then, in a separate terminal, start GDB with the following command:
```bash
gdb-multiarch output/RTOSDemo.out
```
Then connect to QEMU
```bash
target remote:1234
```
Set breakpoints and debug

