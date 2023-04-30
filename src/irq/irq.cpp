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

#include <hwmocker/irq.h>
#include <hwmocker_internal.h>

#include <list>
#include <map>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define SIGNUM_BASE (SIGRTMIN + 3)

using namespace std;

list<struct hwmocker_irq_desc *> pending_descs;
pthread_mutex_t pending_descs_mutex;
#define APPEND_PENDING_DESC(d)                                                                    \
    ({                                                                                            \
        int rc;                                                                                   \
        rc = pthread_mutex_lock(&pending_descs_mutex);                                            \
        if (!rc) {                                                                                \
            pending_descs.push_back(d);                                                           \
            rc = pthread_mutex_unlock(&pending_descs_mutex);                                      \
        }                                                                                         \
        rc;                                                                                       \
    })
#define GET_PENDING_DESC()                                                                        \
    ({                                                                                            \
        int rc;                                                                                   \
        struct hwmocker_irq_desc *desc = NULL;                                                    \
        rc = pthread_mutex_lock(&pending_descs_mutex);                                            \
        if (!rc) {                                                                                \
            desc = pending_descs.front();                                                         \
            pending_descs.pop_front();                                                            \
            rc = pthread_mutex_unlock(&pending_descs_mutex);                                      \
        }                                                                                         \
        desc;                                                                                     \
    })

struct irq_handler_data {
    void *data;
    enum hwmocker_irq_type irq_type;
    irq_handler_t handler;
};

struct hwmocker_irq_desc {
    list<struct irq_handler_data *> handlers_data;
    struct hwmocker_irq_handler *irqh;
    int irq_number;
    bool is_exclusive;
    bool masked;
};

struct hwmocker_irq_handler {
    map<int, struct hwmocker_irq_desc *> irq_descs;
    pthread_t dest_thread;
    int signum;
};

void handler(int signo, siginfo_t *info, void *extra) {
    list<struct irq_handler_data *>::iterator it;
    struct irq_handler_data *data = NULL;
    list<struct hwmocker_irq_desc *> postponed_descs;
    struct hwmocker_irq_desc *desc;

    while ((desc = GET_PENDING_DESC()) != NULL) {
        if (desc->masked) {
            postponed_descs.push_back(desc);
            continue;
        }
        for (it = desc->handlers_data.begin(); it != desc->handlers_data.end(); it++) {
            data = *it;
            if (data->irq_type == HWMOCKER_IRQ_RISING_EDGE ||
                data->irq_type == HWMOCKER_IRQ_HIGH_LEVEL)
                data->handler(desc->irq_number, data->data);
        }
    }
    while (!postponed_descs.empty()) {
        desc = postponed_descs.front();
        postponed_descs.pop_front();
        APPEND_PENDING_DESC(desc);
    }
}

static int hwmocker_irq_handler_register_sig_handlers(struct hwmocker_irq_handler *irqh) {
    struct sigaction action;
    static int signum = SIGNUM_BASE;
    int rc;

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = handler;

    rc = sigaction(signum++, &action, NULL);
    return rc;
}

#include <cstdio>

struct hwmocker_irq_handler *hwmocker_irq_handler_create(struct hwmocker *mocker, int is_device) {
    int rc;
    struct hwmocker_irq_handler *irqh =
        (struct hwmocker_irq_handler *)calloc(1, sizeof(struct hwmocker_irq_handler));
    if (!irqh)
        return irqh;

    new (&irqh->irq_descs) map<int, struct hwmocker_irq_desc *>;

    rc = hwmocker_irq_handler_register_sig_handlers(irqh);
    if (rc) {
        free(irqh);
        irqh = NULL;
    } else {
        irqh->dest_thread = is_device ? mocker->device_thread : mocker->host_thread;
    }
    return irqh;
}

void hwmocker_irq_handler_destroy(struct hwmocker_irq_handler *irqh) {
    map<int, struct hwmocker_irq_desc *>::iterator it;
    for (it = irqh->irq_descs.begin(); it != irqh->irq_descs.end(); ++it) {
        struct hwmocker_irq_desc *desc = it->second;
        while (desc->handlers_data.empty()) {
            struct irq_handler_data *data = desc->handlers_data.back();
            desc->handlers_data.pop_back();
            free(data);
        }
        desc->handlers_data.clear();
        free(desc);
    }
    irqh->irq_descs.clear();
    free(irqh);
}

int hwmocker_irq_handler_declare_irq(struct hwmocker_irq_handler *irqh, int irq_number) {
    if (irqh->irq_descs.count(irq_number))
        return -EEXIST;

    struct hwmocker_irq_desc *desc =
        (struct hwmocker_irq_desc *)calloc(1, sizeof(struct hwmocker_irq_desc));
    if (!desc)
        return -ENOMEM;

    desc->irq_number = irq_number;
    desc->irqh = irqh;
    irqh->irq_descs[irq_number] = desc;
    return 0;
}

int hwmocker_irq_handler_register_handler(struct hwmocker_irq_handler *irqh, int irq_number,
                                          irq_handler_t handler, void *irq_data,
                                          enum hwmocker_irq_type irq_type, int is_exclusive) {
    struct irq_handler_data *data;
    struct hwmocker_irq_desc *desc;

    if (!irqh->irq_descs.count(irq_number))
        return -ENOENT;

    desc = irqh->irq_descs[irq_number];
    if (is_exclusive && !desc->handlers_data.empty())
        return -EINVAL;

    if (desc->is_exclusive)
        return -EINVAL;

    data = (struct irq_handler_data *)malloc(sizeof(struct irq_handler_data));
    if (!data)
        return -ENOMEM;

    desc->is_exclusive = is_exclusive;
    data->irq_type = irq_type;
    desc->handlers_data.push_back(data);
    return 0;
}

int hwmocker_irq_handler_unregister_handler(struct hwmocker_irq_handler *irqh, int irq_number,
                                            irq_handler_t handler) {
    list<struct irq_handler_data *>::iterator it;
    struct irq_handler_data *data = NULL;
    struct hwmocker_irq_desc *desc;

    if (!irqh->irq_descs.count(irq_number))
        return -ENOENT;

    desc = irqh->irq_descs[irq_number];
    if (!desc->handlers_data.empty())
        return -EINVAL;

    for (it = desc->handlers_data.begin(); it != desc->handlers_data.end(); it++)
        if ((*it)->handler == handler)
            data = *it;

    if (!data)
        return -ENOENT;

    desc->handlers_data.remove(data);
    return 0;
}

struct hwmocker_irq_desc *hwmocker_irq_handler_get_irq_desc(struct hwmocker_irq_handler *irqh,
                                                            int irq_number) {
    if (!irqh->irq_descs.count(irq_number))
        return NULL;

    return irqh->irq_descs[irq_number];
}

int hwmocker_irq_handler_trigger_irq(struct hwmocker_irq_desc *desc) {

    int rc = 0;

    rc = APPEND_PENDING_DESC(desc);
    if (!rc && !desc->masked)
        rc = pthread_kill(desc->irqh->dest_thread, desc->irqh->signum);
    return rc;
}
