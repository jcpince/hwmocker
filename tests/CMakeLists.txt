if (CONFIG_HWMOCK_SPI)

add_executable(test_irq
    test_irq.c)

target_link_libraries(test_irq
    hwmocker)

endif(CONFIG_HWMOCK_SPI)
