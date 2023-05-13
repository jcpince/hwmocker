add_library(
  hwmocker
  hwmocker.cpp
  gpio/GpioController.cpp
  gpio/Gpio.cpp
  gpio/GpioIrq.cpp
  irq/HwIrq.cpp
  pin/Pin.cpp
  processingunit/ProcessingUnit.cpp
  system/System.cpp)

add_subdirectory_ifdef(CONFIG_HWMOCK_SPI spi)
