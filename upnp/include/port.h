// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-03

// Header file for portable definitions
// ====================================
// This header should be included into every source file to have portable
// definitions available. So it cannot have other includes.

#ifndef UPNP_INCLUDE_PORT_H
#define UPNP_INCLUDE_PORT_H

// On MS Windows <unistd.h> isn't availabe. We can use <io.h> instead for most
// functions but it's not 100% compatible.
#if _WIN32
#define PORT_UNISTD_H <io.h>
#else
#define PORT_UNISTD_H <unistd.h>
#endif

#endif // UPNP_INCLUDE_PORT_H
