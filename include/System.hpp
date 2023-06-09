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

namespace HWMocker {

///
/// class System

class System : virtual public HwElement {
  public:
    System(const char *hwmcnf, int (*host_main)(void *), void *host_arg, int (*soc_main)(void *),
           void *soc_arg);
    virtual ~System();
    int start();
    void stop();
    void wait();

    ///
    /// Get the value of soc
    /// @return the value of soc
    ProcessingUnit *get_soc() { return soc; }

    ///
    /// Get the value of host
    /// @return the value of host
    ProcessingUnit *get_host() { return host; }

    void set_soc_ready() { soc->set_ready(); }
    void set_host_ready() { host->set_ready(); }
    void wait_soc_ready() { soc->wait_ready(); }
    void wait_host_ready() { host->wait_ready(); }

  private:
    ProcessingUnit *soc = nullptr;
    ProcessingUnit *host = nullptr;
    std::vector<HwElement> hw_elements;

    int load_config(json config);
};
} // namespace HWMocker

#endif // __HWMOCKER_SYSTEM_HPP
