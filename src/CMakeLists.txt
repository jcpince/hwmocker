add_library(hwmocker
    hwmocker.cpp)

add_subdirectory_ifdef(CONFIG_HWMOCK_IRQ irq)

target_link_libraries(hwmocker PRIVATE
    yaml-cpp)
