# Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-10-22

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UMOCK VERSION 0001
        DESCRIPTION "UPnP library mocking subproject"
        HOMEPAGE_URL "https://github.com/upnplib"
)

set(UMOCK_SOURCE_FILES
    ./src/string.cpp
    ./src/stdlib.cpp
    ./src/sys_socket.cpp
    ./src/sys_select.cpp
    ./src/netdb.cpp
    ./src/pthread.cpp
    #./src/stdio.cpp
    #./src/unistd.cpp
    #$<$<NOT:$<PLATFORM_ID:Windows>>:./src/ifaddrs.cpp>
    #$<$<NOT:$<PLATFORM_ID:Windows>>:./src/net_if.cpp>
    #$<$<PLATFORM_ID:Windows>:./src/iphlpapi.cpp>
    #$<$<PLATFORM_ID:Windows>:./src/winsock2.cpp>
)

add_library(umock_shared SHARED
        ${UMOCK_SOURCE_FILES}
        ${UPnPlib_SOURCE_DIR}/core/src/strndup.cpp
)
target_include_directories(umock_shared
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        # if pthreads4w isn't installed this path is empty. The linker
        # doesn't find "pthread.h" and falls back to look at <pthread.h>.
        $<$<PLATFORM_ID:Windows>:${pthreads4w_SOURCE_DIR}> # needed for pthread.h
)
target_compile_definitions(umock_shared
    PRIVATE
        UPNPLIB_SHARED
        UPNPLIB_EXPORTS
)
target_link_libraries(umock_shared
    PUBLIC
        #ixml_shared
        # In addition to pthreads we need some more system libraries on WIN32
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
        # On MS Windows we link with the static pthreads4w library to
        # avoid error prone managing access to its .dll file.
        # On Unix platforms we cannot link with the static pthreads library because
        # then we have also to link with a static libc that's not available by default.
        $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>

)

# UMock STatic library: postfix -umst
#------------------------------------
add_library(umock_static STATIC
        ${UMOCK_SOURCE_FILES}
)
target_include_directories(umock_static
    PRIVATE
        ${UPnPlib_SOURCE_DIR}/include
        $<$<PLATFORM_ID:Windows>:${pthreads4w_SOURCE_DIR}> # needed for pthread.h
)
target_link_libraries(umock_static
    PUBLIC
        # In addition to pthreads we need some more system libraries on WIN32
        $<$<PLATFORM_ID:Windows>:ws2_32>   # winsock to support sockets
        $<$<PLATFORM_ID:Windows>:iphlpapi> # ip helper interface to get network management tools
    INTERFACE
        #ixml_static
        # On MS Windows we link with the static pthreads4w library to
        # avoid error prone managing access to its .dll file.
        # On Unix platforms we cannot link with the static pthreads library because
        # then we have also to link with a static libc that's not available by default.
        $<IF:$<PLATFORM_ID:Windows>,${UPNP_PTHREADS_STATIC_LIBRARY},${UPNP_PTHREADS_SHARED_LIBRARY}>
)