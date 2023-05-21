#include "IrqController.hpp"
#include <hwmocker/config.h>
#include <hwmocker_internal.h>

#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace HWMocker;
using namespace std;

string get_stacktrace_str(unsigned int max_frames);

std::map<pthread_t, IrqController *> signal_handlers_map;
pthread_mutex_t signal_handlers_map_mutex;

static void signal_handler(int signo, siginfo_t *info, void *extra);

// Constructors/Destructors

IrqController::IrqController() {}

IrqController::~IrqController() {}

void IrqController::set_dest_irq_controller(IrqController *dest_controller) {
    if (this->dest_controller) {
        std::stringstream reason;
        reason << "set_dest_irq_controller failed: dest irq controller already set" << std::endl;
        reason << get_stacktrace_str(64) << std::endl;
        throw new std::runtime_error(reason.str());
    }
    this->dest_controller = dest_controller;
}

void IrqController::start() {
    int rc;

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;
    pthread = pthread_self();
    dest_controller = NULL;

    if (sigaction(HWMOCK_IRQ_SIGNUM, &action, NULL)) {
        stringstream reason;
        reason << "sigaction failed with " << strerror(errno) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    rc = pthread_mutex_lock(&signal_handlers_map_mutex);
    if (rc) {
        stringstream reason;
        reason << "sigaction pthread_mutex_lock with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    if (signal_handlers_map.count(pthread_self())) {
        pthread_mutex_unlock(&signal_handlers_map_mutex);
        stringstream reason;
        reason << "Irq signal handler already registered" << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }
    signal_handlers_map[pthread_self()] = this;
    pthread_mutex_unlock(&signal_handlers_map_mutex);
}

void IrqController::disableIrq(GenericIrq *irq) { irq->disable(); }

void IrqController::enableIrq(GenericIrq *irq) { irq->enable(); }

void IrqController::disableIrqs() { allirqs_enabled = false; }

void IrqController::enableIrqs() {
    if (allirqs_enabled)
        return;

    allirqs_enabled = true;

    if (pending_irqs.size())
        interrupt_self();
}

void IrqController::interrupt_self() { pthread_kill(pthread, HWMOCK_IRQ_SIGNUM); }

void IrqController::interrupt_dest() {
    if (!dest_controller) {
        std::stringstream reason;
        reason << "interrupt_dest failed: No destination irq controller" << std::endl;
        reason << get_stacktrace_str(64) << std::endl;
        throw new std::runtime_error(reason.str());
    }
    pthread_kill(dest_controller->pthread, HWMOCK_IRQ_SIGNUM);
}

/// Raise an irq on the destination Hw element
void IrqController::local_raise(GenericIrq *irq) {
    if (!irq->enabled())
        return;

    pending_irqs.insert(irq);
    interrupt_self();
}

void IrqController::dest_raise(GenericIrq *irq) {
    if (!irq->enabled())
        return;

    dest_controller->pending_irqs.insert(irq);
    interrupt_dest();
}

/// Handle a new irq or the pending irqs
/// @return int
int IrqController::handle() {
    int rc;
    // no priority, simply handle the pending one by one

    for (GenericIrq *irq : pending_irqs) {
        rc |= irq->handle();
    }
    return rc;
}

void signal_handler([[maybe_unused]] int signo, [[maybe_unused]] siginfo_t *info,
                    [[maybe_unused]] void *extra) {

    IrqController *controller = signal_handlers_map[pthread_self()];
    controller->handle();
}