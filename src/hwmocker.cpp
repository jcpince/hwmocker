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

#include <System.hpp>
#include <hwmocker/hwmocker.h>
#include <hwmocker/irq.h>
#include <hwmocker_internal.h>

#ifdef CONFIG_HWMOCK_SPI
#include <SpiDevice.hpp>
#endif

#include <signal.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

#include <cxxabi.h>
#include <execinfo.h>

using namespace HWMocker;
using namespace std;

struct hwmocker *hwmocker_create(const char *hwmcnf, int (*host_main)(void *), void *host_arg,
                                 int (*soc_main)(void *), void *soc_arg) {
    struct hwmocker *mocker = (struct hwmocker *)calloc(1, sizeof(struct hwmocker));
    if (!mocker)
        return mocker;

    try {
        mocker->system = new System(hwmcnf, host_main, host_arg, soc_main, soc_arg);
        if (!mocker->system) {
            free(mocker);
            mocker = NULL;
        }
    } catch (runtime_error *e) {
        free(mocker);
        mocker = NULL;
        cerr << e->what() << endl;
    }
    return mocker;
}

void hwmocker_destroy(struct hwmocker *mocker) {
    delete mocker->system;
    free(mocker);
}

int hwmocker_start(struct hwmocker *mocker) { return mocker->system->start(); }

void hwmocker_stop(struct hwmocker *mocker) { mocker->system->stop(); }

void hwmocker_wait(struct hwmocker *mocker) { mocker->system->wait(); }

string get_stacktrace_str(unsigned int max_frames) {
    stringstream out;
    out << "stack trace:" << endl;

    // storage array for stack trace address data
    void **addrlist = (void **)calloc(max_frames + 1, sizeof(void *));

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, max_frames + 1);

    if (addrlen == 0) {
        out << "  <empty, possibly corrupt>\n";
        free(addrlist);
        return out.str();
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char **symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char *funcname = (char *)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 2; i < addrlen; i++) {
        char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

        // find parentheses and +address offset surrounding the mangled name:
        // ./module(function+0x15c) [0x8048a6d]
        for (char *p = symbollist[i]; *p; ++p) {
            if (*p == '(')
                begin_name = p;
            else if (*p == '+')
                begin_offset = p;
            else if (*p == ')' && begin_offset) {
                end_offset = p;
                break;
            }
        }

        if (begin_name && begin_offset && end_offset && begin_name < begin_offset) {
            *begin_name++ = '\0';
            *begin_offset++ = '\0';
            *end_offset = '\0';

            // mangled name is now in [begin_name, begin_offset) and caller
            // offset in [begin_offset, end_offset). now apply
            // __cxa_demangle():

            int status;
            char *ret = abi::__cxa_demangle(begin_name, funcname, &funcnamesize, &status);
            if (status == 0) {
                funcname = ret; // use possibly realloc()-ed string
                out << "  " << symbollist[i] << " : " << funcname << "+" << begin_offset << endl;
            } else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                out << "  " << symbollist[i] << " : " << begin_name << "()+" << begin_offset
                    << endl;
            }
        } else {
            // couldn't parse the line? print the whole line.
            out << "  " << symbollist[i] << endl;
        }
    }

    free(funcname);
    free(symbollist);
    free(addrlist);
    return out.str();
}

void print_stacktrace(FILE *out, unsigned int max_frames) {
    string str = get_stacktrace_str(max_frames);
    fprintf(out, "%s", str.c_str());
}

int hwmocker_set_gpio_irq_handler(void *hw_element, unsigned int pin_idx, int (*handler)(void)) {
    ProcessingUnit *processing_unit = (ProcessingUnit *)hw_element;
    return processing_unit->set_gpio_irq(pin_idx, handler);
}

void *hwmocker_get_soc(struct hwmocker *mocker) { return (void *)mocker->system->get_soc(); }

void *hwmocker_get_host(struct hwmocker *mocker) { return (void *)mocker->system->get_host(); }

void hwmocker_set_soc_ready(struct hwmocker *mocker) { mocker->system->set_soc_ready(); }

void hwmocker_set_host_ready(struct hwmocker *mocker) { mocker->system->set_host_ready(); }

void hwmocker_wait_soc_ready(struct hwmocker *mocker) { mocker->system->wait_soc_ready(); }

void hwmocker_wait_host_ready(struct hwmocker *mocker) { mocker->system->wait_host_ready(); }

void hwmocker_set_gpio_level(void *hw_element, unsigned int pin_idx, bool level) {
    ProcessingUnit *processing_unit = (ProcessingUnit *)hw_element;
    processing_unit->set_gpio_value(pin_idx, level);
}

#ifdef CONFIG_HWMOCK_SPI
void *hwmocker_get_spi_device(void *hw_element, unsigned int spi_idx) {
    ProcessingUnit *processing_unit = (ProcessingUnit *)hw_element;
    return processing_unit->get_spi_device(spi_idx);
}

int hwmocker_spi_xfer(void *_spi_dev, const void *txbuf, void *rxbuf, size_t size) {
    SpiDevice *spi_dev = (SpiDevice *)_spi_dev;
    return spi_dev->sync_xfer(txbuf, rxbuf, size);
}

int hwmocker_spi_xfer_async(void *_spi_dev, const void *txbuf, void *rxbuf, size_t size,
                            int (*callback)(void *ctx), void *ctx) {
    SpiDevice *spi_dev = (SpiDevice *)_spi_dev;
    return spi_dev->async_xfer(txbuf, rxbuf, size, callback, ctx);
}
#endif