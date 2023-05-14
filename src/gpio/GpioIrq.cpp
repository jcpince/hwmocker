#include "GpioIrq.hpp"

#include <bits/stdc++.h>

using namespace HWMocker;

// Constructors/Destructors

GpioIrq::GpioIrq(IrqController *irq_controller, unsigned int pin_idx) : Gpio(pin_idx) {
    this->irq_controller = irq_controller;
}

GpioIrq::GpioIrq(IrqController *irq_controller, Gpio *gpio) : Gpio(gpio->pin_idx) {
    this->irq_controller = irq_controller;
    for (Pin *pin : gpio->get_connected_pins()) {
        auto it = find(pin->get_connected_pins().begin(), pin->get_connected_pins().end(), gpio);
        int old_idx = it - pin->get_connected_pins().begin();
        pin->get_connected_pins()[old_idx] = this;
        connected_pins.push_back(pin);
    }
}

GpioIrq::~GpioIrq() {}

// Methods

// Accessor methods

// Other methods
