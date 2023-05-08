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

#ifndef __HWMOCKER_SYSTEM_HPP
#define __HWMOCKER_SYSTEM_HPP

#include "HwElement.hpp"
#include "ProcessingUnit.hpp"

#include <vector>

namespace HWMocker {

///
/// class System

class System : virtual public HwElement {
  public:
    System(const char *hwmcnf);
    ~System();

  private:
    HWMocker::ProcessingUnit *soc = nullptr;
    HWMocker::ProcessingUnit *host = nullptr;
    HWMocker::GpioController gpio_controller;
    std::vector<HWMocker::HwElement> hw_elements;

    int load_config(json config);

    ///
    /// Set the value of soc
    /// @param value the new value of soc
    void setSoc(HWMocker::ProcessingUnit *soc) { this->soc = soc; }

    ///
    /// Get the value of soc
    /// @return the value of soc
    HWMocker::ProcessingUnit *getSoc() { return soc; }

    ///
    /// Set the value of host
    /// @param value the new value of host
    void setHost(HWMocker::ProcessingUnit *host) { this->host = host; }

    ///
    /// Get the value of host
    /// @return the value of host
    HWMocker::ProcessingUnit *getHost() { return host; }

    ///
    /// Set the value of gpio_controller
    /// @param value the new value of gpio_controller
    void setGpio_controller(HWMocker::GpioController value) { gpio_controller = value; }

    ///
    /// Get the value of gpio_controller
    /// @return the value of gpio_controller
    HWMocker::GpioController getGpio_controller() { return gpio_controller; }
};
} // namespace HWMocker

#endif // __HWMOCKER_SYSTEM_HPP