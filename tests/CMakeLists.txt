add_executable(test_gpio_irq test_gpio_irq.c)
target_link_libraries(test_gpio_irq hwmocker)

if(CONFIG_HWMOCK_SPI)
  add_executable(test_spi test_spi.c)
  target_link_libraries(test_spi hwmocker)
endif(CONFIG_HWMOCK_SPI)
