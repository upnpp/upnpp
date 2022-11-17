// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-11-19

#include "umock/sys_select.inc"

namespace umock {

int Sys_selectReal::select(SOCKET nfds, fd_set* readfds, fd_set* writefds,
                           fd_set* exceptfds, struct timeval* timeout) {
    // Call real standard library function
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

// This constructor is used to inject the pointer to the real function.
Sys_select::Sys_select(Sys_selectReal* a_ptr_mockObj) {
    m_ptr_workerObj = (Sys_selectInterface*)a_ptr_mockObj;
}

// This constructor is used to inject the pointer to the mocking function.
Sys_select::Sys_select(Sys_selectInterface* a_ptr_mockObj) {
    m_ptr_oldObj = m_ptr_workerObj;
    m_ptr_workerObj = (Sys_selectInterface*)a_ptr_mockObj;
}

// The destructor is ussed to restore the old pointer.
Sys_select::~Sys_select() { m_ptr_workerObj = m_ptr_oldObj; }

// Methods
int Sys_select::select(SOCKET nfds, fd_set* readfds, fd_set* writefds,
                       fd_set* exceptfds, struct timeval* timeout) {
    return m_ptr_workerObj->select(nfds, readfds, writefds, exceptfds, timeout);
}

// On program start create an object and inject pointer to the real function.
// This will exist until program end.
Sys_selectReal sys_select_realObj;
UPNPLIB_API Sys_select sys_select_h(&sys_select_realObj);

} // namespace umock
