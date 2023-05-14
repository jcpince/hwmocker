#include "IrqController.hpp"
#include <hwmocker/config.h>

#include <map>

using namespace HWMocker;

std::map<pthread_t, IrqController *> signal_handlers_map;
pthread_mutex_t signal_handlers_map_mutex;

static void signal_handler(int signo, siginfo_t *info, void *extra);

// Constructors/Destructors

IrqController::IrqController() {}

IrqController::~IrqController() {}

void IrqController::start() {
    int rc;

    action.sa_flags = SA_SIGINFO;
    action.sa_sigaction = signal_handler;
    pthread = pthread_self();
    dest_controller = NULL;

    if (sigaction(HWMOCK_IRQ_SIGNUM, &action, NULL))
        throw std::runtime_error(std::string("sigaction failed with ") + strerror(errno));

    rc = pthread_mutex_lock(&signal_handlers_map_mutex);
    if (rc)
        throw std::runtime_error(std::string("pthread_mutex_lock failed with ") + strerror(rc));
    if (signal_handlers_map.count(pthread_self())) {
        pthread_mutex_unlock(&signal_handlers_map_mutex);
        throw std::runtime_error("Irq signal handler already registered");
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
    if (!dest_controller)
        throw std::runtime_error("interrupt_dest failed: No destination irq controller");
    pthread_kill(dest_controller->pthread, HWMOCK_IRQ_SIGNUM);
}

/// Raise qn irq on the destination Hw element
void IrqController::raise(GenericIrq *irq) {
    if (irq->enabled())
        return;

    pending_irqs.insert(irq);
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

void signal_handler(int signo, siginfo_t *info, void *extra) {

    IrqController *controller = signal_handlers_map[pthread_self()];
    controller->handle();
}