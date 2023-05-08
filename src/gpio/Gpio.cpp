#include "Gpio.hpp"

HWMocker::Gpio::Gpio(unsigned int pin_idx) : Pin(pin_idx), input(true), level(false) {}

HWMocker::Gpio::~Gpio() {}
