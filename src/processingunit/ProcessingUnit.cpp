#include "ProcessingUnit.hpp"
#ifdef CONFIG_HWMOCK_SPI
#include "SpiDevice.hpp"
#endif

#include <iostream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace HWMocker;

ProcessingUnit::ProcessingUnit(const char *name) : name(name) {

    int rc;

    irq_controller = new IrqController();
    if (!irq_controller) {
        throw std::runtime_error("Could not allocate irq controller");
    }

    rc = pthread_mutex_lock(&start_mutex);
    if (rc)
        throw std::runtime_error(string("pthread_mutex_lock(start_mutex) failed with ") +
                                 strerror(rc));

    rc = pthread_create(&pthread, NULL, processing_unit_thread_fn, this);
    if (rc)
        throw std::runtime_error(string("pthread_create failed with ") + strerror(rc));

    rc = pthread_setname_np(pthread, name);
    if (rc)
        throw std::runtime_error(string("pthread_setname_np failed with ") + strerror(rc));
}

ProcessingUnit::~ProcessingUnit() {
    for (Gpio *gpio : gpios)
        delete gpio;

    if (irq_controller)
        delete irq_controller;
}

/// @brief Loads a json configuration and build
/// the processing unit from it.
/// @param config json configuration
/// @return 0 on success
int ProcessingUnit::load_config(json config) {
    // printf("config %s\n", config.c_str());
    std::cout << "config " << config << std::endl;
    for (int pin_idx : config["gpio-pins"]) {
        gpios.push_back(new Gpio(pin_idx));
    }

#ifdef CONFIG_HWMOCK_SPI
    if (SpiDevice::config_has_device(config))
        printf("Has spi\n");
#endif

    return 0;
}

Pin *ProcessingUnit::get_pin(unsigned int pin_idx) {
    for (Gpio *gpio : gpios) {
        if (gpio->pin_idx == pin_idx)
            return gpio;
    }
    // Not found, checks the devices pins
    return nullptr;
}

void ProcessingUnit::start() {
    stopped = false;
    pthread_mutex_unlock(&start_mutex);
}

void ProcessingUnit::stop() {
    stopped = true;
    pthread_mutex_unlock(&start_mutex);

    pthread_join(pthread, NULL);
}

int ProcessingUnit::run_thread() {
    printf("%s(%s) called\n", __func__, name);
    irq_controller->start();

    // Wait for the start to be called
    int rc = pthread_mutex_lock(&start_mutex);
    if (rc)
        throw std::runtime_error(string("pthread_mutex_lock(start_mutex) failed with ") +
                                 strerror(rc));

    if (stopped) {
        printf("%s(%s) started but stopped\n", __func__, name);
        return 0;
    }

    if (!main_func)
        throw std::runtime_error(string("main function not set yet"));

    printf("%s(%s) starting\n", __func__, name);
    return main_func(main_arg);
}

void *HWMocker::processing_unit_thread_fn(void *data) {
    ProcessingUnit *pu = (ProcessingUnit *)data;
    pu->run_thread();
    return NULL;
}