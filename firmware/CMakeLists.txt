cmake_minimum_required(VERSION 3.20.0)

list(APPEND DTS_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(ae1200_emulator)

target_sources(app PRIVATE
    src/main.c
    src/lpm013m126a.c
    ../core/src/strata_display.c
    ../core/src/strata_jdi.c
)

target_include_directories(app PRIVATE
    src
    ../core/include
)
