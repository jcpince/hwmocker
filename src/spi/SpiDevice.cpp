#include "SpiDevice.hpp"

#include <hwmocker_internal.h>

#include <stdexcept>
#include <string>

using namespace std;
using namespace HWMocker;

// Constructors/Destructors
SpiDevice::SpiDevice(IrqController *irq_controller, HwIrq *irq) {
    if (!irq) {
        stringstream reason;
        reason << "Cannot allocate with a null hw irq" << endl << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    this->irq = irq;
    this->irq_controller = irq_controller;
    irq->set_handler(spi_irq_handler, this);
}

SpiDevice::~SpiDevice() {
    if (mosi)
        delete mosi;
    if (miso)
        delete miso;
    if (clk)
        delete clk;
    if (csn)
        delete csn;
    delete irq;
}

int SpiDevice::spi_irq_handler(void *ctx) {
    SpiDevice *spi_dev = (SpiDevice *)ctx;
    if (spi_dev->slave_callback) {
        int (*slave_callback)(void *) = spi_dev->slave_callback;
        void *slave_callback_ctx = spi_dev->slave_callback_ctx;
        spi_dev->slave_callback = nullptr;
        spi_dev->slave_callback_ctx = nullptr;
        slave_callback(slave_callback_ctx);
    }
    return 0;
}

int SpiDevice::load_config(json config) {
    json spi_config = config["spi"];
    spi_index = spi_config["index"];
    is_master = spi_config["master"];
    mosi = new Gpio(spi_config["mosi-pin"]);
    miso = new Gpio(spi_config["miso-pin"]);
    clk = new Gpio(spi_config["clk-pin"]);
    csn = new Gpio(spi_config["csn-pin"]);
    irq->set_irqn(spi_config["irq"]);
    return 0;
}

bool SpiDevice::set_remote(SpiDevice *remote_spi_dev) {
    // Checks that both devices are connected
    bool found = false;
    for (Pin *connected_pin : mosi->get_connected_pins()) {
        if (connected_pin == remote_spi_dev->mosi) {
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    found = false;
    for (Pin *connected_pin : miso->get_connected_pins()) {
        if (connected_pin == remote_spi_dev->miso) {
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    found = false;
    for (Pin *connected_pin : clk->get_connected_pins()) {
        if (connected_pin == remote_spi_dev->clk) {
            found = true;
            break;
        }
    }
    if (!found)
        return false;

    // Ok...
    this->remote_spi_dev = remote_spi_dev;
    return true;
}

///
/// @return int
/// @param  txbuf
/// @param  rxbuf
/// @param  size
int SpiDevice::sync_xfer(const void *txbuf, void *rxbuf, size_t size) {

    pthread_mutex_lock(&remote_spi_dev->lock);
    current_rx = rxbuf;
    current_tx = txbuf;
    if (is_master) {
        if (remote_spi_dev->is_listening)
            remote_spi_dev->xmit_locked(txbuf, rxbuf, size);
        current_rx = nullptr;
        current_tx = nullptr;
        pthread_mutex_unlock(&remote_spi_dev->lock);
        return size;
    }

    current_xfer_size = size;
    is_listening = true;
    pthread_mutex_unlock(&remote_spi_dev->lock);
    wait_xfer_done();
    return current_xfer_size;
}

///
/// @return int
/// @param  txbuf
/// @param  rxbuf
/// @param  size
/// @param  _callback_ctx_void_
/// @param  ctx
int SpiDevice::async_xfer(const void *txbuf, void *rxbuf, size_t size, int (*callback)(void *),
                          void *ctx) {
    if (!irq->enabled())
        return -EINVAL;

    if (is_master && remote_spi_dev->is_listening) {
        // Do it immediately
        int rc = sync_xfer(txbuf, rxbuf, size);
        if (rc && callback)
            callback(ctx);
        return rc;
    }

    // Slave
    pthread_mutex_lock(&remote_spi_dev->lock);
    current_rx = rxbuf;
    current_tx = txbuf;
    current_xfer_size = size;
    is_listening = true;
    slave_callback = callback;
    slave_callback_ctx = ctx;
    pthread_mutex_unlock(&remote_spi_dev->lock);
    return 0;
}

void SpiDevice::xmit_locked(const void *txbuf, void *rxbuf, size_t size) {
    int rc;

    size = min(size, current_xfer_size);
    if (size && current_tx)
        memcpy(rxbuf, current_tx, size);

    if (size && current_rx)
        memcpy(current_rx, txbuf, size);

    current_rx = nullptr;
    current_tx = nullptr;

    current_xfer_size = size;
    is_listening = false;
    rc = pthread_cond_signal(&cond);
    if (rc) {
        stringstream reason;
        reason << "pthread_cond_signal(cond) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    if (irq_controller)
        irq_controller->local_raise(this->irq);
}

void SpiDevice::wait_xfer_done() {
    int rc = pthread_mutex_lock(&cond_mutex);
    if (rc) {
        stringstream reason;
        reason << "pthread_mutex_lock(cond_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    rc = pthread_cond_wait(&cond, &cond_mutex);
    if (rc) {
        stringstream reason;
        reason << "pthread_cond_wait(cond, cond_mutex) failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }
}