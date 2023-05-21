#include "System.hpp"
#include "SpiDevice.hpp"

#include <hwmocker_internal.h>

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>

using namespace std;
using namespace HWMocker;

/// @brief Build a new system
/// @param hwmcnf hardware mocker configuration file
System::System(const char *hwmcnf, int (*host_main)(void *), void *host_arg,
               int (*soc_main)(void *), void *soc_arg) {
    int rc = pthread_setname_np(pthread_self(), "main");
    if (rc) {
        stringstream reason;
        reason << "pthread_setname_np failed with " << strerror(rc) << endl
               << get_stacktrace_str(64) << endl;
        throw new runtime_error(reason.str());
    }

    soc = new ProcessingUnit("soc");
    host = new ProcessingUnit("host");
    host->set_dest_processing_unit(soc);
    soc->set_main_function(soc_main);
    soc->set_main_arg(soc_arg);
    soc->set_dest_processing_unit(host);
    host->set_main_function(host_main);
    host->set_main_arg(host_arg);

    std::ifstream f(hwmcnf);
    if (!f) {
        stringstream reason;
        reason << "Wrong configuration file " << hwmcnf << endl << get_stacktrace_str(64) << endl;
        throw new std::invalid_argument(reason.str());
    }
    config = json::parse(f);
    load_config(config["system"]);
}

/// @brief Destroy a system
System::~System() {
    stop();
    delete soc;
    delete host;
}

/// @brief Loads a json configuration and build
/// the system from it.
/// @param config json configuration
/// @return 0 on success
int System::load_config(json config) {

    int rc;
    printf("Loading the soc config...\n");
    rc = soc->load_config(config["soc"]);
    if (rc)
        return rc;

    printf("Loading the host config...\n");
    rc = host->load_config(config["host"]);
    if (rc)
        return rc;

    for (string connection : config["host-soc-pin-connections"]) {
        regex re("^[0-9]+:[0-9]+$");
        cmatch m;
        if (regex_match(connection.c_str(), m, re)) {
            regex re("[0-9]+");
            sregex_iterator it = sregex_iterator(connection.begin(), connection.end(), re);
            Pin *host_pin = host->get_pin(stoi((*it++).str()));
            Pin *soc_pin = soc->get_pin(stoi((*it).str()));
            bool found = true;
            if (!soc_pin) {
                found = false;
                printf("Connection %s: soc pin not found\n", connection.c_str());
            }
            if (!host_pin) {
                found = false;
                printf("Connection %s: host pin not found\n", connection.c_str());
            }
            if (found) {
                printf("Connection %d -> %d\n", soc_pin->pin_idx, host_pin->pin_idx);
                soc_pin->connect(host_pin);
                host_pin->connect(soc_pin);
            }
        } else
            printf("%s doesn't match\n", connection.c_str());
    }

#ifdef CONFIG_HWMOCK_SPI
    // Connect the spi devices
    for (SpiDevice *host_spi : host->spi_devs) {
        for (SpiDevice *soc_spi : soc->spi_devs) {
            if (host_spi->set_remote(soc_spi)) {
                assert(soc_spi->set_remote(host_spi));
                printf("Host spi %d connected to soc spi %d\n", host_spi->get_spi_index(),
                       soc_spi->get_spi_index());
            }
        }
    }
#endif
    return 0;
}

int System::start() {
    try {
        host->start();
        soc->start();
    } catch (runtime_error *e) {
        cerr << e->what() << endl;
        print_stacktrace(stderr, 63);
        return -1;
    }
    return 0;
}

void System::wait() {
    soc->wait();
    host->wait();
}

void System::stop() {
    soc->stop();
    host->stop();
}