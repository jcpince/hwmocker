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

static void *device_thread_fn(void *data);
static void *host_thread_fn(void *data);

//#include <iostream>
#include <fstream>

struct hwmocker *hwmocker_create(const char *hwmcnf) {
    struct hwmocker *mocker = (struct hwmocker *)calloc(1, sizeof(struct hwmocker));
    if (!mocker)
        return mocker;

    System *s = new System(hwmcnf);
    delete s;
    free(mocker);
    return NULL;

#ifdef CONFIG_HWMOCK_IRQ
    mocker->device.irq_handler = hwmocker_irq_handler_create(mocker, 1);
    if (!mocker->device.irq_handler)
        goto failure;

    mocker->host.irq_handler = hwmocker_irq_handler_create(mocker, 0);
    if (!mocker->host.irq_handler)
        goto failure;
#endif

    mocker->device.mocker = mocker;
    mocker->host.mocker = mocker;

    return mocker;

failure:
    hwmocker_destroy(mocker);
    return NULL;
}

void hwmocker_destroy(struct hwmocker *mocker) {
    if (mocker->device.irq_handler)
        hwmocker_irq_handler_destroy(mocker->device.irq_handler);
    if (mocker->host.irq_handler)
        hwmocker_irq_handler_destroy(mocker->host.irq_handler);
    free(mocker);
}

struct hwmocker_device *hwmocker_get_device(struct hwmocker *mocker) {
    return &mocker->device;
}

struct hwmocker_host *hwmocker_get_host(struct hwmocker *mocker) {
    return &mocker->host;
}

static void *device_thread_fn(void *data) {
    struct hwmocker_device *device = (struct hwmocker_device *)data;
    device->main_function(device->priv);
    return NULL;
}

static void *host_thread_fn(void *data) {
    struct hwmocker_host *host = (struct hwmocker_host *)data;
    host->main_function(host->priv);
    return NULL;
}

int hwmocker_start(struct hwmocker *mocker) {
    int rc;
    rc = pthread_create(&mocker->device_thread, NULL, device_thread_fn, &mocker->device);
    if (rc)
        return rc;
    rc = pthread_create(&mocker->host_thread, NULL, host_thread_fn, &mocker->host);
    if (rc) {
        pthread_kill(mocker->device_thread, SIGKILL);
        return rc;
    }
    return 0;
}

void hwmocker_stop(struct hwmocker *mocker) {
    pthread_kill(mocker->device_thread, SIGKILL);
    pthread_kill(mocker->host_thread, SIGKILL);
}

void hwmocker_wait(struct hwmocker *mocker) {
    pthread_join(mocker->device_thread, NULL);
    pthread_join(mocker->host_thread, NULL);
}