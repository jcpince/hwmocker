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

#ifndef __HWMOCKER_GpioIrq_HPP
#define __HWMOCKER_GpioIrq_HPP

#include "GenericIrq.hpp"
#include "Gpio.hpp"

namespace HWMocker {

///
/// class GpioIrq

class GpioIrq : public Gpio, virtual public GenericIrq {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    GpioIrq();

    ///
    /// Empty Destructor
    virtual ~GpioIrq();

    // Static Public attributes

    // Public attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

  protected:
    // Static Protected attributes

    // Protected attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

  private:
    // Static Private attributes

    // Private attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods
};
} // namespace HWMocker

#endif // __HWMOCKER_GpioIrq_HPP
