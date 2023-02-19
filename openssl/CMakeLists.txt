# Copyright (C) 2023+ GPL 3 and higher by Ingo Höft, <Ingo@Hoeft-online.de>
# Redistribution only with this Copyright remark. Last modified: 2023-02-21

cmake_minimum_required(VERSION 3.18)
include(../cmake/project-header.cmake)

project(UPNPLIB VERSION 0001
        DESCRIPTION "OpenSSL external library for UPnPlib"
        HOMEPAGE_URL "https://github.com/openssl/openssl")


if(NOT WIN32)

    add_library(crypto STATIC IMPORTED GLOBAL)
    set_target_properties(crypto PROPERTIES
        IMPORTED_LOCATION ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/lib/libcrypto.a
    )
    target_include_directories(crypto
        # We use the own build include files from the OpenSSL build on MS Windows
        INTERFACE ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/Windows/include
    )
    target_compile_definitions(crypto
        INTERFACE UPNP_ENABLE_OPEN_SSL
    )
    target_link_libraries(crypto
        INTERFACE dl
    )

    add_library(ssl STATIC IMPORTED GLOBAL)
    set_target_properties(ssl PROPERTIES
        IMPORTED_LOCATION ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/lib/libssl.a
    )
    target_link_libraries(ssl
        INTERFACE crypto
    )

else() # WIN32

    add_library(crypto SHARED IMPORTED GLOBAL)
    set_target_properties(crypto PROPERTIES
        IMPORTED_IMPLIB ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/lib/libcrypto.lib
        # Don't know for what next line is good, still have to be on a search path.
        IMPORTED_LOCATION ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/bin/libcrypto-3-x64.dll
    )
    file(CREATE_LINK ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/bin/libcrypto-3-x64.dll
                     $ENV{SystemRoot}/System32/libcrypto-3-x64.dll COPY_ON_ERROR SYMBOLIC
    )
    target_include_directories(crypto
        INTERFACE ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/include
    )
    target_compile_definitions(crypto
        INTERFACE UPNP_ENABLE_OPEN_SSL
    )

    add_library(ssl SHARED IMPORTED GLOBAL)
    set_target_properties(ssl PROPERTIES
        IMPORTED_IMPLIB ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/lib/libssl.lib
        # Don't know for what next line is good, still have to be on a search path.
        IMPORTED_LOCATION ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/bin/libssl-3-x64.dll
    )
    file(CREATE_LINK ${UPNPLIB_PROJECT_SOURCE_DIR}/openssl/${CMAKE_HOST_SYSTEM_NAME}/bin/libssl-3-x64.dll
                     $ENV{SystemRoot}/System32/libssl-3-x64.dll COPY_ON_ERROR SYMBOLIC
    )
    target_link_libraries(ssl
        INTERFACE crypto
    )

endif()