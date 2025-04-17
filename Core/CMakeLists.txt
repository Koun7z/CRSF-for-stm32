cmake_minimum_required(VERSION 3.14)
project(CRSF-for-stm32)

add_library(CRSF-for-stm32 STATIC)

include(Src/Config.cmake)

target_include_directories(CRSF-for-stm32 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Inc)