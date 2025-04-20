# CRSF for STM32

## Description

CRSF for STM32 is a library providing an API to communicate with RC radio receivers
using the [CRSFv2](https://github.com/crsf-wg/crsf?tab=readme-ov-file) serial protocol.
The CRSF protocol is used both by [ExpressLRS](https://www.expresslrs.org) and [TBS Crossfire](https://www.team-blacksheep.com) radio transmitters.

> [!NOTE]
> The library was tested only with RadioMaster ExpressLRS devices, as that is the only hardware that I currently own.

## Features

- Receive data from up to 16, 11-bit channels to control any of your RC projects using an STM32 MCU.
- Gather telemetry data using CRSF defined telemetry frames:
  - Battery data
  - GPS data
  - Barometer data
  - Variometer data

> [!NOTE]
> The library is in a state of constant development. All parts of the library are subject to change,
> and I cannot guarantee every part of it will work perfectly.

### Plans for the future

- Add wiki explaining proper use of the API
- Add support for the rest of telemetry [frame types](https://github.com/crsf-wg/crsf/wiki/Packet-Types)
- Maybe add support for CRSFv3, if I ever get my hands on hardware that actually supports it.

## Compiling the library

CRSF for STM32 is distributed in source form. Brief instructions of how to include it in your project can be found below.

CRSF for STM32 was developed using the [ARM GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads).

### Necessary files:

To compile the library you will need to provide it with necessary HAL drivers, based on specific MCU you're using.
Easiest way to get all needed drivers is to generate project files using the [CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html#overview) environment.

The library is set up to be built with CMake, but if you want to use any other tools,
all files needed to build the library (except HAL Drivers) are inside the `Core` folder.

### Hardware configuration

Hardware peripherals used by the library:
- UART (Full Duplex):
  - Full Duplex
  - Baud Rate - Needs to match the baud rate configured on your RC receiver
    (420k is default for ExpressLRS receivers).
  - Word Length - 8 bits
  - Stop bits - 1
  - Parity - None
  - DMA - add DMA requests for both RX and TX.
  - Interrupts - Enable corresponding global UART and DMA interrupts.

>[!IMPORTANT]
> The priorities on both the UART interrupts and RX DMA request should be set as high as possible, 
> for lowest latency and safest RC link. The TX priority is not as important and can be kept low.

- CRC (Only when using the hardware crc option)
  - To enable the hardware crc implementation add `CRSF_CRC_HARD` definition to the compiler
  - Default Poly - Disable
  - CRC Length - 8-bit
  - CRC Init - 0
  - CRC Poly - 0xD5

>[!NOTE]
> Not all STM32 MCUs with CRC peripheral built in, give you an ability to configure the CRC length and poly (e.g. STM32f411).
> In this case you need to use the software CRC implementation. 

You will also need to add these definitions, so the library can have access to the HAL Drivers:
- `STM32XX` - replace the XX with your MCU series (e.g. STM32F4)
- `STM32xxxx` - replace the xxxx with you exact MCU model (e.g. STM32f411xE)
- `USE_HAL_DRIVER`

### Building with CMake:

To build with CMake add the library as a subdirectory to the CMakeList.txt in your main project.
If you're using `add_compile_option()` or `add_link_options()` make sure to add them before the `add_subdirectory()`
call, if you want to use the same options for compiling this library.
Otherwise, specify compiler and linker flags using `target_compile_options()`/`target_link_options()`.

```cmake
# Example CMakeLists file
cmake_minimum_required(VERSION 3.14)
project(example_project)

.
.
.

# Common flags
add_compile_option(${COMPILE_OPTIONS})
add_link_options(${LINKER_OPTIONS})

# Example global HAL driver definitions
add_compile_definitions(-DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32F4)

# Add the library as subdirectory
# ${CRSF-for-stm32} is a relative path to your instance of the library
add_subdirectory(${CRSF-for-stm32}/Core)

# Link the binary to your project
target_link_libraries(${PROJECT_NAME} PRIVATE CRSF-for-stm32)

# Library specific flags
target_compile_options(CRSF-for-stm32 PRIVATE ${CRSF_COMPILE_OPTIONS})
```

## License

This library is licensed under
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT).
