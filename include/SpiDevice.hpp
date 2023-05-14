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

#ifndef __HWMOCKER_SPIDEVICE_HPP
#define __HWMOCKER_SPIDEVICE_HPP

#include "Gpio.hpp"
#include "HwElement.hpp"
#include "Pin.hpp"

namespace HWMocker {

///
/// class SpiDevice

class SpiDevice : virtual public HwElement {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    SpiDevice();

    ///
    /// Empty Destructor
    virtual ~SpiDevice();

    // Static Public attributes

    // Public attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

    ///
    /// @return int
    /// @param  json_cfg
    int load_config(json config);

    ///
    /// @return int
    /// @param  txbuf
    /// @param  rxbuf
    /// @param  size
    int sync_xfer(void *txbuf, void *rxbuf, size_t size);

    ///
    /// @return int
    /// @param  txbuf
    /// @param  rxbuf
    /// @param  size
    /// @param  _callback_ctx_void_
    /// @param  ctx
    int async_xfer(void *txbuf, void *rxbuf, size_t size, int _callback_ctx_void_, void *ctx);

    static bool config_has_device(json config) { return config.contains("spi"); }

  protected:
    // Static Protected attributes

    // Protected attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

  private:
    // Static Private attributes

    // Private attributes

    Pin *miso;
    Pin *mosi;
    Pin *clk;
    Gpio *csn;

    // Public static attribute accessor methods

    // Public attribute accessor methods

    ///
    /// Set the value of miso
    /// @param value the new value of miso
    void setMiso(Pin *value) { miso = value; }

    ///
    /// Get the value of miso
    /// @return the value of miso
    Pin *getMiso() { return miso; }

    ///
    /// Set the value of mosi
    /// @param value the new value of mosi
    void setMosi(Pin *value) { mosi = value; }

    ///
    /// Get the value of mosi
    /// @return the value of mosi
    Pin *getMosi() { return mosi; }

    ///
    /// Set the value of clk
    /// @param value the new value of clk
    void setClk(Pin *value) { clk = value; }

    ///
    /// Get the value of clk
    /// @return the value of clk
    Pin *getClk() { return clk; }

    ///
    /// Set the value of csn
    /// @param value the new value of csn
    void setCsn(Gpio *value) { csn = value; }

    ///
    /// Get the value of csn
    /// @return the value of csn
    Gpio *getCsn() { return csn; }
};
} // namespace HWMocker

#endif // __HWMOCKER_SPIDEVICE_HPP
