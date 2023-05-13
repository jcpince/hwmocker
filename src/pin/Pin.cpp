#include "Pin.hpp"

using namespace HWMocker;

// Constructors/Destructors
Pin::Pin(unsigned int pin_idx) : pin_idx(pin_idx) {}

Pin::~Pin() {}

// Methods
void Pin::connect(Pin *pin) {
    if (!pin)
        return;

    for (Pin *p : connected_pins)
        if (p == pin)
            return;

    connected_pins.push_back(pin);
}

// Accessor methods

// Other methods
