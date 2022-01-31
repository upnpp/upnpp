// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-01-22

// Yes, we have this global varibles in namespace ::upnplib, but they are only
// needed as long as the old C sources not re-engeneered to C++ objects. We need
// these globals to mock calls of system functions from the old C sources
// because there is no dependency injection possible. In the production
// environment these globels are never touched after instantiation. Its objects
// wrap the system calls and are only statically used to call them.
//
// For further information look at the header files in upnp/include/upnpmock/.

#include "upnpmock/pthread.hpp"
#include "upnpmock/stdio.hpp"
#include "upnpmock/stdlib.hpp"
#include "upnpmock/string.hpp"
#include "upnpmock/netdb.hpp"
#include "upnpmock/sys_socket.hpp"
#include "upnpmock/sys_select.hpp"
#include "upnpmock/unistd.hpp"

namespace upnplib {

Bpthread pthreadObj{};
Bpthread* pthread_h = &pthreadObj;

Bstdio stdioObj{};
Bstdio* stdio_h = &stdioObj;

Bstdlib stdlibObj{};
Bstdlib* stdlib_h = &stdlibObj;

Bstring stringObj{};
Bstring* string_h = &stringObj;

Bnetdb netdbObj{};
Bnetdb* netdb_h = &netdbObj;

Bsys_socket sys_socketObj{};
Bsys_socket* sys_socket_h = &sys_socketObj;

Bsys_select sys_selectObj{};
Bsys_select* sys_select_h = &sys_selectObj;

Bunistd unistdObj{};
Bunistd* unistd_h = &unistdObj;

} // namespace upnplib