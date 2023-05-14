set(CONFIG_HWMOCK_SPI
    ON
    CACHE INTERNAL "SPI hw support")

set(CONFIG_HWMOCK_TESTS
    ON
    CACHE INTERNAL "hwmock unit tests")

set(CONFIG_HWMOCK_IRQ_SIGNUM
    "(SIGRTMIN + 3)"
    CACHE INTERNAL "Irq signal number used")
