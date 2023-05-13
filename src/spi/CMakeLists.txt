message(STATUS "Adding sublib spi")

add_library(spi SpiDevice.cpp)

target_link_libraries(hwmocker PUBLIC spi)
