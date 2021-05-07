if (NOT PUPNP_VERSION_STRING)
    file (GLOB_RECURSE MACROFILES
        ${CMAKE_CURRENT_SOURCE_DIR}/../
        *.m4
    )

    list (APPEND MACROFILES ${CMAKE_CURRENT_SOURCE_DIR}/../configure.ac)
    list (APPEND WRITTEN_VARS DEBUG)
    list (APPEND WRITTEN_VARS NDEBUG)

    foreach (MACROFILE ${MACROFILES})
        file (STRINGS ${MACROFILE} configure)
        file (REMOVE ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm)

        foreach (line ${configure})
            string (REGEX REPLACE "\\]" "" line ${line})
            string (REGEX REPLACE "\\[" "" line ${line})
            string (REGEX REPLACE ";" "" line ${line})
            string (REGEX REPLACE "[ \t\r\n] *" " " line ${line})

            if (line MATCHES "AC_INIT.* ([0-9]*\\.[0-9]*\\.[0-9]*).*")
                # found in ./configure.ac
                message (STATUS "Setting package-version to ${CMAKE_MATCH_1}")
                set (PUPNP_VERSION_STRING ${CMAKE_MATCH_1} CACHE STRING "Version of the whole package" FORCE)
            elseif (line MATCHES "[. \t]*AC_DEFINE_UNQUOTED *\\(([^,]*), *([^,]*), *([^\\)]*)")
                set (SAVED_MATCH ${CMAKE_MATCH_1})

                if ("${CMAKE_MATCH_1}" IN_LIST WRITTEN_VARS)
                    continue()
                endif()

                string (SUBSTRING ${CMAKE_MATCH_2} 0 1 FIRSTCHAR)
                string (STRIP ${CMAKE_MATCH_3} ${CMAKE_MATCH_3})
                file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "/* ${CMAKE_MATCH_3} */\n")

                if (FIRSTCHAR STREQUAL "\"")
                    file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "#cmakedefine ${CMAKE_MATCH_1} \"\$\{${CMAKE_MATCH_1}\}\"\n\n")
                else()
                    if (${CMAKE_MATCH_1} MATCHES VERSION AND NOT ${${CMAKE_MATCH_1}})
                        file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "#cmakedefine01 ${SAVED_MATCH}\n\n")
                    else()
                        file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "#cmakedefine ${SAVED_MATCH} \$\{${SAVED_MATCH}\}\n\n")
                    endif()
                endif()

                list (APPEND WRITTEN_VARS ${SAVED_MATCH})
            elseif (line MATCHES "[. \t]*AC_DEFINE *\\(([^,]*), *([^,]*), *([^\\)]*)")
                if ("${CMAKE_MATCH_1}" IN_LIST WRITTEN_VARS)
                    continue()
                endif()

                string (STRIP ${CMAKE_MATCH_3} ${CMAKE_MATCH_3})
                file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "/* ${CMAKE_MATCH_3} */\n")
                file (APPEND ${CMAKE_CURRENT_BINARY_DIR}/autoconfig.h.cm "#cmakedefine ${CMAKE_MATCH_1} 1\n\n")
                list (APPEND WRITTEN_VARS ${CMAKE_MATCH_1})
            elseif (line MATCHES "^AC_SUBST.*LT_VERSION_IXML, ([0-9]*):([0-9]*):([0-9]*).*")
                set (IXML_VERSION_MAJOR ${CMAKE_MATCH_1} CACHE STRING "Majorversion of libixml" FORCE)
                set (IXML_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3} CACHE STRING "Version of libixml" FORCE)
                message (STATUS "Setting ixml-version to ${IXML_VERSION}")
            elseif (line MATCHES "^AC_SUBST.*LT_VERSION_UPNP, ([0-9]*):([0-9]*):([0-9]*).*")
                set (UPNP_VERSION_MAJOR ${CMAKE_MATCH_1} CACHE STRING "Majorversion of libupnp" FORCE)
                set (UPNP_VERSION ${CMAKE_MATCH_1}.${CMAKE_MATCH_2}.${CMAKE_MATCH_3} CACHE STRING "Version of libupnp" FORCE)
                set (UPNP_VERSION_STRING ${UPNP_VERSION} CACHE STRING "see upnpconfig.h" FORCE)
                message (STATUS "Setting upnp-version to ${UPNP_VERSION}")
            endif()
        endforeach()
    endforeach()
endif()

