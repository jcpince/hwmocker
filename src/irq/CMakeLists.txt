message(STATUS "Adding sublib irq")

add_library(irq
    irq.cpp)

target_link_libraries(hwmocker PUBLIC
    irq)