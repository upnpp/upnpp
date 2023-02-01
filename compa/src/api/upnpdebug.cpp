// Copyright (C) 2022+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-01

#include "compa/upnpdebug.hpp"

#include <stdexcept>
#include <string>

namespace compa {

CLogging::CLogging(Upnp_LogLevel a_loglevel) {
    UpnpSetLogLevel(a_loglevel);
    if (UpnpInitLog() != UPNP_E_SUCCESS) {
        throw std::runtime_error(
            std::string("UpnpInitLog(): failed to initialize pupnp logging."));
    }
}

CLogging::~CLogging() { UpnpCloseLog(); }

} // namespace compa
