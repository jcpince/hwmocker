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

#ifndef __HWMOCKER_H__
#define __HWMOCKER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <hwmocker/config.h>
#include <hwmocker/irq.h>

struct hwmocker;

struct hwmocker_device {
    struct hwmocker *mocker;
    struct hwmocker_irq_handler *irq_handler;
    void *priv;
    int (*main_function)(void *priv);
    volatile int stopped;
};

struct hwmocker_host {
    struct hwmocker *mocker;
    struct hwmocker_irq_handler *irq_handler;
    void *priv;
    int (*main_function)(void *priv);
};

struct hwmocker *hwmocker_create(const char *hwmcnf);
void hwmocker_destroy(struct hwmocker *mocker);

struct hwmocker_device *hwmocker_get_device(struct hwmocker *mocker);
struct hwmocker_host *hwmocker_get_host(struct hwmocker *mocker);

int hwmocker_start(struct hwmocker *mocker);
void hwmocker_stop(struct hwmocker *mocker);

void hwmocker_wait(struct hwmocker *mocker);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HWMOCKER_H__ */