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
#include "HwIrq.hpp"
#include "IrqController.hpp"

namespace HWMocker {

///
/// class SpiDevice

class SpiDevice : virtual public HwElement {
  public:
    // Constructors/Destructors

    ///
    /// Empty Constructor
    SpiDevice(IrqController *irq_controller, HwIrq *irq = new HwIrq());

    ///
    /// Empty Destructor
    virtual ~SpiDevice();

    void enable_interrupt() { irq->enable(); }
    void disable_interrupt() { irq->disable(); }

    // Static Public attributes

    // Public attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods
    unsigned int get_spi_index() { return spi_index; }

    ///
    /// @return int
    /// @param  json_cfg
    int load_config(json config);

    ///
    /// @return int
    /// @param  txbuf
    /// @param  rxbuf
    /// @param  size
    int sync_xfer(const void *txbuf, void *rxbuf, size_t size);

    ///
    /// @return int
    /// @param  txbuf
    /// @param  rxbuf
    /// @param  size
    /// @param  _callback_ctx_void_
    /// @param  ctx
    int async_xfer(const void *txbuf, void *rxbuf, size_t size, int (*callback)(void *ctx),
                   void *ctx);

    static bool config_has_device(json config) { return config.contains("spi"); }

    Pin *getPin(unsigned int pin_idx) {
        if (mosi && mosi->pin_idx == pin_idx)
            return mosi;
        if (miso && miso->pin_idx == pin_idx)
            return miso;
        if (clk && clk->pin_idx == pin_idx)
            return clk;
        if (csn && csn->pin_idx == pin_idx)
            return csn;
        return nullptr;
    }

    bool set_remote(SpiDevice *remote_spi_dev);

  protected:
    // Static Protected attributes

    // Protected attributes

    // Public static attribute accessor methods

    // Public attribute accessor methods

  private:
    // Static Private attributes

    // Private attributes
    unsigned int spi_index;
    bool is_master;
    HwIrq *irq = nullptr;
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t cond_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    void *current_rx = nullptr;
    const void *current_tx = nullptr;
    size_t current_xfer_size = 0;
    volatile bool is_listening = false;
    Pin *miso = nullptr;
    Pin *mosi = nullptr;
    Pin *clk = nullptr;
    Gpio *csn = nullptr;
    SpiDevice *remote_spi_dev;
    int (*slave_callback)(void *) = nullptr;
    void *slave_callback_ctx = nullptr;
    IrqController *irq_controller = nullptr;

    static int spi_irq_handler(void *ctx);

    void xmit_locked(const void *txbuf, void *rxbuf, size_t size);
    void wait_xfer_done();

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
