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

#ifndef __HWMOCKER_IRQCONTROLLER_HPP
#define __HWMOCKER_IRQCONTROLLER_HPP

#include "GenericIrq.hpp"

#include <cstring>
#include <set>
#include <string>

#include <errno.h>
#include <pthread.h>
#include <signal.h>

namespace HWMocker {

///
/// class IrqController
class IrqController {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    IrqController();

    ///
    /// Empty Destructor
    virtual ~IrqController();

    void start();
    void disableIrq(GenericIrq *irq);
    void enableIrq(GenericIrq *irq);
    void disableIrqs();
    void enableIrqs();

    /// Raise qn irq on the destination Hw element
    void local_raise(GenericIrq *irq);
    void dest_raise(GenericIrq *irq);

    /// Handle the pending irqs
    /// @return int
    int handle();

    void set_dest_irq_controller(IrqController *dest_controller);

  private:
    bool allirqs_enabled;
    std::set<GenericIrq *> pending_irqs;
    struct sigaction action;
    pthread_t pthread = {0};
    IrqController *dest_controller = nullptr;

    void interrupt_self();
    void interrupt_dest();
};
} // namespace HWMocker

#endif // __HWMOCKER_IRQCONTROLLER_HPP
