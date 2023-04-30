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

#ifndef __HWMOCKER_IRQ_H__
#define __HWMOCKER_IRQ_H__

#ifdef __cplusplus
extern "C" {
#endif

struct hwmocker_irq_handler;
struct hwmocker_irq_desc;
struct hwmocker;

enum hwmocker_irq_type {
    HWMOCKER_IRQ_NONE = 0,
    HWMOCKER_IRQ_RISING_EDGE = 1,
    HWMOCKER_IRQ_FALLING_EDGE = 2,
    HWMOCKER_IRQ_HIGH_LEVEL = 4,
    HWMOCKER_IRQ_LOW_LEVEL = 8,
};

typedef int (*irq_handler_t)(int irq_number, void *priv);

struct hwmocker_irq_handler *hwmocker_irq_handler_create(struct hwmocker *mocker, int is_device);
void hwmocker_irq_handler_destroy(struct hwmocker_irq_handler *irqh);

int hwmocker_irq_handler_declare_irq(struct hwmocker_irq_handler *irqh, int irq_number);
int hwmocker_irq_handler_register_handler(struct hwmocker_irq_handler *irqh, int irq_number,
                                          irq_handler_t handler, void *irq_data,
                                          enum hwmocker_irq_type type, int is_exclusive);
int hwmocker_irq_handler_unregister_handler(struct hwmocker_irq_handler *irqh, int irq_number,
                                            irq_handler_t handler);

struct hwmocker_irq_desc *hwmocker_irq_handler_get_irq_desc(struct hwmocker_irq_handler *irqh,
                                                            int irq_number);
int hwmocker_irq_handler_trigger_irq(struct hwmocker_irq_desc *desc);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HWMOCKER_IRQ_H__ */