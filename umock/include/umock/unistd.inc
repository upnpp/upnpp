// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2023-02-03

// There is no include guard '#ifndef ...' because this file shouln't be
// included more than two times as given.

#include "upnplib/port.hpp"
#include "upnplib/visibility.hpp"

namespace umock {

class UnistdInterface {
  public:
    virtual ~UnistdInterface() = default;
    virtual int CLOSE_SOCKET_P(SOCKET fd) = 0;
};

//
// This is the wrapper class for the real (library?) function
// ----------------------------------------------------------
class UnistdReal : public UnistdInterface {
  public:
    virtual ~UnistdReal() override = default;
    int CLOSE_SOCKET_P(SOCKET fd) override;
};

//
// This is the caller or injector class that injects the class (worker) to be
// used, real or mocked functions.
/* Example:
    UnistdReal unistd_realObj;        // already done
    Unistd unistd_h(&unistd_realObj); // already done
    { // Other scope, e.g. within a gtest
        class UnistdMock : public UnistdInterface { ...; MOCK_METHOD(...) };
        UnistdMock unistd_mockObj;
        Unistd unistd_injectObj(&unistd_mockObj); // obj. name doesn't matter
        EXPECT_CALL(unistd_mockObj, ...);
    } // End scope, mock objects are destructed, worker restored to default.
*/ //------------------------------------------------------------------------
class UPNPLIB_API Unistd {
  public:
    // This constructor is used to inject the pointer to the real function. It
    // sets the default used class, that is the real function.
    Unistd(UnistdReal* a_ptr_realObj);

    // This constructor is used to inject the pointer to the mocking function.
    Unistd(UnistdInterface* a_ptr_mockObj);

    // The destructor is ussed to restore the old pointer.
    virtual ~Unistd();

    // Methods
    virtual int CLOSE_SOCKET_P(SOCKET fd);

  private:
    // Next variable must be static. Please note that a static member variable
    // belongs to the class, but not to the instantiated object. This is
    // important here for mocking because the pointer is also valid on all
    // objects of this class. With inline we do not need an extra definition
    // line outside the class. I also make the symbol hidden so the variable
    // cannot be accessed globaly with Unistd::m_ptr_workerObj.
    UPNPLIB_LOCAL static inline UnistdInterface* m_ptr_workerObj;
    UnistdInterface* m_ptr_oldObj{};
};

} // namespace umock
// vim: syntax=cpp