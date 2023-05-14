add_library(
  hwmocker
  hwmocker.cpp
  gpio/Gpio.cpp
  gpio/GpioIrq.cpp
  irq/IrqController.cpp
  irq/HwIrq.cpp
  pin/Pin.cpp
  processingunit/ProcessingUnit.cpp
  system/System.cpp)

add_subdirectory_ifdef(CONFIG_HWMOCK_SPI spi)
