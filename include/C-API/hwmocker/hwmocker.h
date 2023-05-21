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

#include <stdbool.h>

struct hwmocker;

struct hwmocker *hwmocker_create(const char *hwmcnf, int (*host_main)(void *), void *host_arg,
                                 int (*soc_main)(void *), void *soc_arg);
void hwmocker_destroy(struct hwmocker *mocker);

void *hwmocker_get_soc(struct hwmocker *mocker);
void *hwmocker_get_host(struct hwmocker *mocker);

int hwmocker_start(struct hwmocker *mocker);
void hwmocker_stop(struct hwmocker *mocker);
void hwmocker_wait(struct hwmocker *mocker);

void hwmocker_set_soc_ready(struct hwmocker *mocker);
void hwmocker_set_host_ready(struct hwmocker *mocker);
void hwmocker_wait_soc_ready(struct hwmocker *mocker);
void hwmocker_wait_host_ready(struct hwmocker *mocker);

int hwmocker_set_gpio_irq_handler(void *hw_element, unsigned int pin_idx, int (*handler)(void));

void hwmocker_set_gpio_level(void *hw_element, unsigned int pin_idx, bool level);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HWMOCKER_H__ */