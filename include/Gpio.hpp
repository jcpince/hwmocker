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

#ifndef __HWMOCKER_GPIO_HPP
#define __HWMOCKER_GPIO_HPP

#include "Pin.hpp"

namespace HWMocker {

///
/// class Gpio

class Gpio : public Pin {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    Gpio(unsigned int pin_idx);

    ///
    /// Empty Destructor
    virtual ~Gpio();

    // Static Public attributes

    // Public attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

    ///
    /// @param  value
    void set_value(bool value) {
        /*if (!input)*/ {
            level = value;
            change(value);
        }
    }

    ///
    /// @return bool
    bool get_value() { return level; }

  protected:
    bool input;
    bool level;

    void on_change(bool value) { level = value; }
};
} // namespace HWMocker

#endif // __HWMOCKER_GPIO_HPP
