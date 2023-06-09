#include "ProcessingUnit.hpp"

#include <hwmocker_internal.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;
using namespace HWMocker;

ProcessingUnit::ProcessingUnit(const char *name) : name(name) {

    int rc;

    irq_controller = new IrqController();
    if (!irq_controller) {
        stringstream reason;
        reason << "Could not allocate irq controller" << endl << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    memset(&ready_mutex, 0, sizeof(ready_mutex));
    /* Lock the ready mutex since not ready yet */
    rc = pthread_mutex_lock(&ready_mutex);
    if (rc) {
        delete irq_controller;
        stringstream reason;
        reason << "pthread_mutex_lock(ready_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    memset(&start_mutex, 0, sizeof(start_mutex));
    rc = pthread_mutex_lock(&start_mutex);
    if (rc) {
        pthread_mutex_unlock(&ready_mutex);
        delete irq_controller;
        stringstream reason;
        reason << "pthread_mutex_lock(start_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    rc = pthread_create(&pthread, NULL, processing_unit_thread_fn, this);
    if (rc) {
        delete irq_controller;
        stringstream reason;
        reason << "pthread_create failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    rc = pthread_setname_np(pthread, name);
    if (rc) {
        delete irq_controller;
        stringstream reason;
        reason << "pthread_setname_np failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }
}

ProcessingUnit::~ProcessingUnit() {
    for (Gpio *gpio : gpios)
        delete gpio;

    for (GpioIrq *gpio_irq : gpio_irqs)
        delete gpio_irq;

#ifdef CONFIG_HWMOCK_SPI
    for (SpiDevice *spi_dev : spi_devs)
        delete spi_dev;
#endif

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
    if (SpiDevice::config_has_device(config)) {
        SpiDevice *spi = new SpiDevice(irq_controller);
        if (!spi)
            return -ENOMEM;
        int rc = spi->load_config(config);
        if (!rc)
            spi_devs.push_back(spi);
    }
#endif

    return 0;
}

Pin *ProcessingUnit::get_pin(unsigned int pin_idx) {
    for (Gpio *gpio : gpios) {
        if (gpio->pin_idx == pin_idx)
            return gpio;
    }
    for (GpioIrq *gpio : gpio_irqs) {
        if (gpio->pin_idx == pin_idx)
            return gpio;
    }
    // Not found, checks the devices pins
#ifdef CONFIG_HWMOCK_SPI
    for (SpiDevice *spi_dev : spi_devs) {
        Pin *pin = spi_dev->getPin(pin_idx);
        if (pin)
            return pin;
    }
#endif
    return nullptr;
}

int ProcessingUnit::set_gpio_irq(unsigned int pin_idx, int (*handler)(void)) {
    vector<Gpio *>::iterator it;

    for (it = gpios.begin(); it != gpios.end(); it++) {
        if ((*it)->pin_idx == pin_idx)
            break;
    }
    if (it != gpios.end()) {
        Gpio *gpio = *it;
        GpioIrq *gpio_irq = new GpioIrq(irq_controller, gpio);
        if (!gpio_irq)
            return -ENOMEM;
        gpio_irq->set_handler(handler);
        gpio_irqs.push_back(gpio_irq);
        gpios.erase(it);
        delete gpio;
    } else {
        return -EINVAL;
    }
    return 0;
}

void ProcessingUnit::set_gpio_value(unsigned int pin_idx, bool value) {
    for (Gpio *gpio : gpios) {
        if (gpio->pin_idx == pin_idx)
            return gpio->set_value(value);
    }
    for (GpioIrq *gpio_irq : gpio_irqs) {
        if (gpio_irq->pin_idx == pin_idx)
            return gpio_irq->set_value(value);
    }
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
    if (rc) {
        stringstream reason;
        reason << "pthread_mutex_lock(start_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    if (stopped) {
        printf("%s(%s) started but stopped\n", __func__, name);
        return 0;
    }

    if (!main_func) {
        stringstream reason;
        reason << "main function not set yet" << endl << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    printf("%s(%s) starting\n", __func__, name);
    return main_func(main_arg);
}

void *HWMocker::processing_unit_thread_fn(void *data) {
    ProcessingUnit *pu = (ProcessingUnit *)data;
    pu->run_thread();
    return NULL;
}

void ProcessingUnit::set_ready() {
    int rc = pthread_mutex_unlock(&ready_mutex);
    if (rc) {
        stringstream reason;
        reason << "set_ready: pthread_mutex_lock(ready_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }
}

void ProcessingUnit::wait_ready() {
    int rc = pthread_mutex_lock(&ready_mutex);
    if (rc) {
        stringstream reason;
        reason << "wait_ready: pthread_mutex_lock(ready_mutex) failed with " << strerror(rc)
               << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }
}