#include "System.hpp"

#include <fstream>
#include <regex>
#include <stdexcept>

using namespace std;

/// @brief Build a new system
/// @param hwmcnf hardware mocker configuration file
HWMocker::System::System(const char *hwmcnf) {
    soc = new HWMocker::ProcessingUnit();
    host = new HWMocker::ProcessingUnit();

    std::ifstream f(hwmcnf);
    if (!f) {
        throw std::invalid_argument(std::string("Wrong configuration file ") + hwmcnf);
    }
    config = json::parse(f);
    load_config(config["system"]);
}

/// @brief Destroy a system
HWMocker::System::~System() {}

/// @brief Loads a json configuration and build
/// the system from it.
/// @param config json configuration
/// @return 0 on success
int HWMocker::System::load_config(json config) {

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
