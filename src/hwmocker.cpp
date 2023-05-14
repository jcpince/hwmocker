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

using namespace HWMocker;

#include <hwmocker_internal.h>

#include <signal.h>
#include <stdlib.h>

#include <fstream>

struct hwmocker *hwmocker_create(const char *hwmcnf, int (*host_main)(void *), void *host_arg,
                                 int (*soc_main)(void *), void *soc_arg) {
    struct hwmocker *mocker = (struct hwmocker *)calloc(1, sizeof(struct hwmocker));
    if (!mocker)
        return mocker;

    mocker->system = new System(hwmcnf, host_main, host_arg, soc_main, soc_arg);
    if (!mocker->system) {
        free(mocker);
        mocker = NULL;
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