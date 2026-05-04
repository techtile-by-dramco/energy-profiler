cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(fw-usb-connected-shield-ep-1v4)

target_sources(app PRIVATE
  src/main.c
  src/ADS1115/ADS1115.c
  src/twowire/twowire.c
  src/peripherals/peripherals.c
  src/AD524X/AD524X.c
  src/lookup_table/lookup_table.c
)

target_include_directories(app PRIVATE
  src
)
