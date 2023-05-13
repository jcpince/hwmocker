#include "ProcessingUnit.hpp"
#ifdef CONFIG_HWMOCK_SPI
#include "SpiDevice.hpp"
#endif

#include <iostream>

HWMocker::ProcessingUnit::ProcessingUnit() {}

HWMocker::ProcessingUnit::~ProcessingUnit() {}

/// @brief Loads a json configuration and build
/// the processing unit from it.
/// @param config json configuration
/// @return 0 on success
int HWMocker::ProcessingUnit::load_config(json config) {
    // printf("config %s\n", config.c_str());
    std::cout << "config " << config << std::endl;
    for (int pin_idx : config["gpio-pins"]) {
        gpios.push_back(new Gpio(pin_idx));
    }

#ifdef CONFIG_HWMOCK_SPI
    if (HWMocker::SpiDevice::config_has_device(config))
        printf("Has spi\n");
#endif

    return 0;
}

HWMocker::Pin *HWMocker::ProcessingUnit::get_pin(unsigned int pin_idx) {
    for (HWMocker::Gpio *gpio : gpios) {
        if (gpio->pin_idx == pin_idx)
            return gpio;
    }
    // Not found, checks the devices pins
    return nullptr;
}