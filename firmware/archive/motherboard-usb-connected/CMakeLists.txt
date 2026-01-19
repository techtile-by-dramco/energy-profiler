cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(motherboard-usb-connected)

target_sources(app PRIVATE
  src/main.c
  src/ADS1115/ADS1115.c
  src/twowire/twowire.c
  src/peripherals/peripherals.c
  src/AD524X/AD524X.c
)
