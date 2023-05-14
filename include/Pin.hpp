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

#ifndef __HWMOCKER_PIN_HPP
#define __HWMOCKER_PIN_HPP

#include <vector>

namespace HWMocker {

///
/// class Pin

class Pin {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    Pin(unsigned int pin_idx);

    ///
    /// Empty Destructor
    virtual ~Pin();

    // Static Public attributes

    // Public attributes
    unsigned int pin_idx;

    // Public static attribute accessor methods

    // Public attribute accessor methods
    void connect(Pin *pin);

    std::vector<Pin *> &get_connected_pins() { return connected_pins; }

  protected:
    // Static Protected attributes

    // Protected attributes
    std::vector<Pin *> connected_pins;

    void change(bool value) {
        for (Pin *pin : connected_pins)
            pin->on_change(value);
    }
    virtual void on_change(bool value) = 0;

    // Public static attribute accessor methods

    // Public attribute accessor methods

  private:
    // Static Private attributes

    // Private attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods
};
} // namespace HWMocker

#endif // __HWMOCKER_PIN_HPP
