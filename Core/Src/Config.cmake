cmake_minimum_required(VERSION 3.14)

set(SRC Src/CRSF_ConnectionRX.c
        Src/CRSF_ConnectionTX.c
        Src/CRSF_CRC_8.c)

target_sources(CRSF-for-stm32 PRIVATE ${SRC})