// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-02-20

#ifndef UPNPLIB_UPNPEVENTSUBSCRIBE_HPP
#define UPNPLIB_UPNPEVENTSUBSCRIBE_HPP

/*!
 * \file
 *
 * \brief Header file for UpnpEventSubscribe methods.
 * \author Marcelo Roberto Jimenez
 */
#include <stdlib.h> /* for size_t */

#include "UpnpGlobal.h" /* for EXPORT_SPEC */

#include "UpnpString.h"

/*!
 * UpnpEventSubscribe
 */
typedef struct s_UpnpEventSubscribe UpnpEventSubscribe;

/*! Constructor */
EXPORT_SPEC UpnpEventSubscribe* UpnpEventSubscribe_new();
/*! Destructor */
EXPORT_SPEC void UpnpEventSubscribe_delete(UpnpEventSubscribe* p);
/*! Copy Constructor */
EXPORT_SPEC UpnpEventSubscribe*
UpnpEventSubscribe_dup(const UpnpEventSubscribe* p);
/*! Assignment operator */
EXPORT_SPEC int UpnpEventSubscribe_assign(UpnpEventSubscribe* p,
                                          const UpnpEventSubscribe* q);

/*! UpnpEventSubscribe_get_ErrCode */
EXPORT_SPEC int UpnpEventSubscribe_get_ErrCode(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_ErrCode */
EXPORT_SPEC int UpnpEventSubscribe_set_ErrCode(UpnpEventSubscribe* p, int n);

/*! UpnpEventSubscribe_get_TimeOut */
EXPORT_SPEC int UpnpEventSubscribe_get_TimeOut(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_TimeOut */
EXPORT_SPEC int UpnpEventSubscribe_set_TimeOut(UpnpEventSubscribe* p, int n);

/*! UpnpEventSubscribe_get_SID */
EXPORT_SPEC const UpnpString*
UpnpEventSubscribe_get_SID(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_SID */
EXPORT_SPEC int UpnpEventSubscribe_set_SID(UpnpEventSubscribe* p,
                                           const UpnpString* s);
/*! UpnpEventSubscribe_get_SID_Length */
EXPORT_SPEC size_t
UpnpEventSubscribe_get_SID_Length(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_get_SID_cstr */
EXPORT_SPEC const char*
UpnpEventSubscribe_get_SID_cstr(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_strcpy_SID */
EXPORT_SPEC int UpnpEventSubscribe_strcpy_SID(UpnpEventSubscribe* p,
                                              const char* s);
/*! UpnpEventSubscribe_strncpy_SID */
EXPORT_SPEC int UpnpEventSubscribe_strncpy_SID(UpnpEventSubscribe* p,
                                               const char* s, size_t n);
/*! UpnpEventSubscribe_clear_SID */
EXPORT_SPEC void UpnpEventSubscribe_clear_SID(UpnpEventSubscribe* p);

/*! UpnpEventSubscribe_get_PublisherUrl */
EXPORT_SPEC const UpnpString*
UpnpEventSubscribe_get_PublisherUrl(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_set_PublisherUrl */
EXPORT_SPEC int UpnpEventSubscribe_set_PublisherUrl(UpnpEventSubscribe* p,
                                                    const UpnpString* s);
/*! UpnpEventSubscribe_get_PublisherUrl_Length */
EXPORT_SPEC size_t
UpnpEventSubscribe_get_PublisherUrl_Length(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_get_PublisherUrl_cstr */
EXPORT_SPEC const char*
UpnpEventSubscribe_get_PublisherUrl_cstr(const UpnpEventSubscribe* p);
/*! UpnpEventSubscribe_strcpy_PublisherUrl */
EXPORT_SPEC int UpnpEventSubscribe_strcpy_PublisherUrl(UpnpEventSubscribe* p,
                                                       const char* s);
/*! UpnpEventSubscribe_strncpy_PublisherUrl */
EXPORT_SPEC int UpnpEventSubscribe_strncpy_PublisherUrl(UpnpEventSubscribe* p,
                                                        const char* s,
                                                        size_t n);
/*! UpnpEventSubscribe_clear_PublisherUrl */
EXPORT_SPEC void UpnpEventSubscribe_clear_PublisherUrl(UpnpEventSubscribe* p);

#endif /* UPNPEVENTSUBSCRIBE_H */
