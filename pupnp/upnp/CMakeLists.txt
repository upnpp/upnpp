# Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2022-01-22

cmake_minimum_required(VERSION 3.18)
include(../../cmake/project-header.cmake)

project(UPNP_CORE VERSION 0007
        DESCRIPTION "Binary build of the UPnP library core"
        HOMEPAGE_URL "https://github.com/upnplib")

# This contains all settings to compile the library dynamically and static.
#==========================================================================

# Source files and compile settings
#----------------------------------
# TODO: Remove references to '${PUPNP_IXML_SOURCE_DIR}' if possible

include_directories(
    ${UPnPlib_BINARY_DIR}
    ${UPnPlib_BINARY_DIR}/upnp/inc
    ${PROJECT_SOURCE_DIR}/inc
    ${PROJECT_SOURCE_DIR}/src/inc
    ${PROJECT_SOURCE_DIR}/src/threadutil
    ${PUPNP_IXML_SOURCE_DIR}/inc
    ${PUPNP_IXML_SOURCE_DIR}/src/inc
    # if pthreads4w isn't installed this path is empty. The linker
    # doesn't find "pthread.h" and falls back to look at <pthread.h>.
    ${pthreads4w_SOURCE_DIR}/
)

set(UPNP_SOURCE_FILES
    ${PUPNP_IXML_SOURCE_DIR}/src/attr.c
    ${PUPNP_IXML_SOURCE_DIR}/src/document.c
    ${PUPNP_IXML_SOURCE_DIR}/src/element.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixml.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlmembuf.c
    ${PUPNP_IXML_SOURCE_DIR}/src/ixmlparser.c
    ${PUPNP_IXML_SOURCE_DIR}/src/namedNodeMap.c
    ${PUPNP_IXML_SOURCE_DIR}/src/node.c
    ${PUPNP_IXML_SOURCE_DIR}/src/nodeList.c
    $<$<CONFIG:Debug>:${PUPNP_IXML_SOURCE_DIR}/src/ixmldebug.c>

    ${PROJECT_SOURCE_DIR}/src/threadutil/FreeList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/LinkedList.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/ThreadPool.cpp
    ${PROJECT_SOURCE_DIR}/src/threadutil/TimerThread.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/sock.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/uri/uri.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpparser.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/httpreadwrite.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/net/http/statcodes.cpp

    ${PROJECT_SOURCE_DIR}/src/genlib/util/list.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/util/membuffer.cpp
    ${PROJECT_SOURCE_DIR}/src/genlib/util/strintmap.cpp

    ${PROJECT_SOURCE_DIR}/src/api/UpnpExtraHeaders.c
    ${PROJECT_SOURCE_DIR}/src/api/UpnpString.cpp
    ${PROJECT_SOURCE_DIR}/src/api/upnpapi.cpp
    $<$<CONFIG:Debug>:${PROJECT_SOURCE_DIR}/src/api/upnpdebug.cpp>

    ${PROJECT_SOURCE_DIR}/src/global.cpp
    ${PROJECT_SOURCE_DIR}/src/mockObj.cpp
    $<$<BOOL:${WIN32}>:${UPNP_CORE_SOURCE_DIR}/src/mockObj_win32.cpp>
    $<$<NOT:$<BOOL:${WIN32}>>:${UPNP_CORE_SOURCE_DIR}/src/mockObj_unix.cpp>
)

add_compile_options(
        # suppress warning 'type cast': pointer truncation
        $<$<CXX_COMPILER_ID:MSVC>:/wd4311>
)

# Create libraries and create test executable for information
#============================================================
if(NOT WIN32)

    # Shared library
    #---------------
    add_library(upnplib_shared SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared PROPERTIES
            OUTPUT_NAME upnplib  # lib*.so will be added
            LIBRARY_OUTPUT_DIRECTORY lib  # used if not WIN32
    #       RUNTIME_OUTPUT_DIRECTORY lib  # used if WIN32
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_run ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_run PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_run
            upnplib_shared
            ${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Static library
    #---------------
    add_library(upnplib_static STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static PROPERTIES
            OUTPUT_NAME upnplib  # lib*.a will be added
            ARCHIVE_OUTPUT_DIRECTORY lib
    )

else() # WIN32

    # Link with POSIX Threads for Windows (pthreads4w)
    #=================================================
    # Pthreads are not supported by MS Windows. So we have installed the
    # third party library pthreads4w.

    # Shared library
    #---------------
    add_library(upnplib_shared_win SHARED ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_shared_win PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_compile_definitions(upnplib_shared_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    # In addition to pthreads we need some more system libraries here
    # ws2_32: winsock to support sockets
    # iphlpapi: ip helper interface to get network management tools
    target_link_libraries(upnplib_shared_win ws2_32 iphlpapi
            ${UPNP_PTHREADS_STATIC_LIBRARY}
            # If you link with the shared pthreads4w library then its .dll file
            # must also be found.
            #${UPNP_PTHREADS_SHARED_LIBRARY}
    )

    # Test executable linked with the shared library
    add_executable(upnplib_shared_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_shared_win_exe PROPERTIES
            OUTPUT_NAME upnplib
    )
    target_link_libraries(upnplib_shared_win_exe
            upnplib_shared_win
    )

    # Static library
    #---------------
    add_library(upnplib_static_win STATIC ${UPNP_SOURCE_FILES})
    set_target_properties(upnplib_static_win PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    target_compile_definitions(upnplib_static_win
            #PRIVATE $<$<BOOL:${script_support}>:IXML_HAVE_SCRIPTSUPPORT>
            PUBLIC $<$<BOOL:${MSVC}>:LIBUPNP_EXPORTS>
    )
    target_link_libraries(upnplib_static_win
            ${UPNP_PTHREADS_STATIC_LIBRARY}
    )

    # Test executable linked with the static library
    add_executable(upnplib_static_win_exe ${PROJECT_SOURCE_DIR}/src/upnplib.cpp)
    set_target_properties(upnplib_static_win_exe PROPERTIES
            OUTPUT_NAME upnplibStatic
    )
    target_link_libraries(upnplib_static_win_exe
            upnplib_static_win
            ws2_32
    )

endif() # WIN32