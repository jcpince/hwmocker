#include "System.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <stdexcept>

using namespace std;
using namespace HWMocker;

#include <cxxabi.h>
#include <execinfo.h>

static inline void print_stacktrace(FILE *out = stderr, unsigned int max_frames = 63);

/// @brief Build a new system
/// @param hwmcnf hardware mocker configuration file
System::System(const char *hwmcnf, int (*host_main)(void *), void *host_arg,
               int (*soc_main)(void *), void *soc_arg) {
    int rc = pthread_setname_np(pthread_self(), "main");
    if (rc)
        throw std::runtime_error(string("pthread_setname_np failed with ") + strerror(rc));

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
        std::string reason = std::string("Wrong configuration file ") + hwmcnf;
        throw std::invalid_argument(reason);
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
    return 0;
}

int System::start() {
    try {
        host->start();
        soc->start();
    } catch (runtime_error e) {
        cerr << e.what() << endl;
        print_stacktrace();
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

/** Print a demangled stack backtrace of the caller function to FILE* out. */
static inline void print_stacktrace(FILE *out, unsigned int max_frames) {
    fprintf(out, "stack trace:\n");

    // storage array for stack trace address data
    void *addrlist[max_frames + 1];

    // retrieve current stack addresses
    int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void *));

    if (addrlen == 0) {
        fprintf(out, "  <empty, possibly corrupt>\n");
        return;
    }

    // resolve addresses into strings containing "filename(function+address)",
    // this array must be free()-ed
    char **symbollist = backtrace_symbols(addrlist, addrlen);

    // allocate string which will be filled with the demangled function name
    size_t funcnamesize = 256;
    char *funcname = (char *)malloc(funcnamesize);

    // iterate over the returned symbol lines. skip the first, it is the
    // address of this function.
    for (int i = 1; i < addrlen; i++) {
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
                fprintf(out, "  %s : %s+%s\n", symbollist[i], funcname, begin_offset);
            } else {
                // demangling failed. Output function name as a C function with
                // no arguments.
                fprintf(out, "  %s : %s()+%s\n", symbollist[i], begin_name, begin_offset);
            }
        } else {
            // couldn't parse the line? print the whole line.
            fprintf(out, "  %s\n", symbollist[i]);
        }
    }

    free(funcname);
    free(symbollist);
}