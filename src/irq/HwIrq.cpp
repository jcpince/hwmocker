#include "HwIrq.hpp"

// Constructors/Destructors

HWMocker::HwIrq::HwIrq() {}

HWMocker::HwIrq::~HwIrq() {}

extern "C" void hwmocker_irq_handler_destroy(struct hwmocker_irq_handler *irqh) {}
