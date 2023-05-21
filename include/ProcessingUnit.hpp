/*
 * MIT License - Copyright (c) 2023 Jean-Christophe PINCE
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __HWMOCKER_PROCESSINGUNIT_HPP
#define __HWMOCKER_PROCESSINGUNIT_HPP

#include "Gpio.hpp"
#include "GpioIrq.hpp"
#include "HwElement.hpp"
#include "IrqController.hpp"

#include <vector>

#include <pthread.h>

using namespace std;

namespace HWMocker {

void *processing_unit_thread_fn(void *data);

///
/// class ProcessingUnit
class ProcessingUnit : virtual public HwElement {

    friend void *processing_unit_thread_fn(void *data);

  public:
    ///
    /// Empty Constructor
    ProcessingUnit(const char *name);

    ///
    /// Empty Destructor
    virtual ~ProcessingUnit();

    void start();
    void stop();
    void wait() { pthread_join(pthread, NULL); }

    int load_config(json config);

    Pin *get_pin(unsigned int pin_idx);

    bool is_stopped() { return stopped; }

    void set_main_function(int (*main_func)(void *data)) { this->main_func = main_func; }

    void set_main_arg(void *main_arg) { this->main_arg = main_arg; }

    void set_dest_processing_unit(ProcessingUnit *dest_processing_unit) {
        this->dest_processing_unit = dest_processing_unit;
        irq_controller->set_dest_irq_controller(dest_processing_unit->get_irq_controller());
    }

    IrqController *get_irq_controller() { return irq_controller; }

    int set_gpio_irq(unsigned int pin_idx, int (*handler)(void));

    void set_gpio_value(unsigned int pin_idx, bool value);

    void set_ready();
    void wait_ready();

  private:
    // Static Private attributes

    // Private attributes
    const char *name;
    ProcessingUnit *dest_processing_unit = nullptr;
    IrqController *irq_controller = nullptr;
    pthread_t pthread = {0};
    vector<Gpio *> gpios;
    vector<GpioIrq *> gpio_irqs;
    pthread_mutex_t start_mutex;
    pthread_mutex_t ready_mutex;
    bool stopped = true;
    int (*main_func)(void *data) = nullptr;
    void *main_arg = nullptr;

    // Public static attribute accessor methods

    // Public attribute accessor methods
    int run_thread();
};
} // namespace HWMocker

#endif // __HWMOCKER_PROCESSINGUNIT_HPP
