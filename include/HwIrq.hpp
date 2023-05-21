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

#ifndef __HWMOCKER_HWIRQ_HPP
#define __HWMOCKER_HWIRQ_HPP

#include "GenericIrq.hpp"
#include "IrqController.hpp"

namespace HWMocker {

///
/// class HwIrq

class HwIrq : virtual public GenericIrq {
  public:
    HwIrq();
    virtual ~HwIrq();

    void set_irqn(unsigned int irqn) { this->irqn = irqn; }

    void enable() { is_enabled = true; }
    void disable() { is_enabled = false; }

    bool enabled() { return is_enabled; }

    void set_handler(int (*handler)(void *ctx), void *irq_context) {
        this->handler = handler;
        this->irq_context = irq_context;
    }

    /// Handle the irq
    /// @return int
    int handle() {
        if (handler)
            return handler(irq_context);
        return 0;
    }

  private:
    bool is_enabled = false;
    unsigned int irqn;
    void *irq_context = nullptr;
    int (*handler)(void *ctx) = nullptr;
};

} // namespace HWMocker

#endif // __HWMOCKER_HWIRQ_HPP
