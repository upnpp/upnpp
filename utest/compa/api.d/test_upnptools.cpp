// Copyright (C) 2022 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2022-10-09

#include "upnptools.hpp"
#include "upnp.hpp"

#include "gtest/gtest.h"

namespace utest {

TEST(UpnptoolsTestSuite, get_error_string) {
    EXPECT_STREQ(::UpnpGetErrorMessage(UPNP_E_INVALID_PARAM),
                 "UPNP_E_INVALID_PARAM");
    EXPECT_STREQ(::UpnpGetErrorMessage(480895), "Unknown error code");
}

} // namespace utest


// main entry
// ----------
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
