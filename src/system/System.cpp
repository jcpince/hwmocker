#include "System.hpp"

#include <fstream>
#include <stdexcept>

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

    for (std::string connection : config["pin-connections"]) {
        printf("Connection %s\n", connection.c_str());
    }
    return 0;
}
