// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-09-08

#include "tools.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <stdio.h>

#include "api/upnpdebug.cpp"

using ::testing::_;
using ::testing::Return;

// --- mock strerror ----------------------------------
class CMock_strerror {
  public:
    MOCK_METHOD(char*, strerror, (int errnum));
};
CMock_strerror* ptrMock_strerror = nullptr;
char* strerror(int errnum) { return ptrMock_strerror->strerror(errnum); }

// --- mock fopen -------------------------------------
class CMock_fopen {
  public:
    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode));
};
CMock_fopen* ptrMock_fopen = nullptr;
FILE* fopen(const char* pathname, const char* mode) {
    return ptrMock_fopen->fopen(pathname, mode);
}

// --- mock fclose ------------------------------------
class CMock_fclose {
  public:
    MOCK_METHOD(int, fclose, (FILE * stream));
};
CMock_fclose* ptrMock_fclose = nullptr;
int fclose(FILE* stream) { return ptrMock_fclose->fclose(stream); }

// Tests for the debugging and logging module
//-------------------------------------------
class UpnpdebugMockTestSuite : public ::testing::Test {
  protected:
    // Instantiate the mock objects.
    // The global pointer to them are set in the constructor below.
    CMock_strerror mock_strerror;
    CMock_fopen mock_fopen;
    CMock_fclose mock_fclose;

    UpnpdebugMockTestSuite() {
        // set the global pointer to the mock objects
        ptrMock_strerror = &mock_strerror;
        ptrMock_fopen = &mock_fopen;
        ptrMock_fclose = &mock_fclose;

        // Initialize the global variable
        g_log_level = UPNP_DEFAULT_LOG_LEVEL;
        fp = nullptr;
        is_stderr = 0;
        setlogwascalled = 0;
        initwascalled = 0;
        fileName = nullptr;
    }
};

TEST_F(UpnpdebugMockTestSuite, defaultInitLog)
// For the ithread_mutex_t structure look at
// https://stackoverflow.com/q/23449508/5014688
{
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 0);
    EXPECT_EQ(pthread_mutex_destroy(&GlobalDebugMutex), 0);
}

TEST_F(UpnpdebugMockTestSuite, setLogWasCalled) {
    setlogwascalled = 1;
    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
    EXPECT_EQ(pthread_mutex_destroy(&GlobalDebugMutex), 0);
}

TEST_F(UpnpdebugMockTestSuite, logNotStderrButToFile) {
    initwascalled = 1;
    setlogwascalled = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = (char*)"gtest.log";

    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a"))
        .Times(1)
        .WillOnce(Return((FILE*)0x123456abcdef));
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(1).WillOnce(Return(0));

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(fp, (FILE*)0x123456abcdef);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, logNotStderrButOpeningFileFails) {
    initwascalled = 1;
    setlogwascalled = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = (char*)"gtest.log";

    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a"))
        .Times(1)
        .WillOnce(Return((FILE*)NULL));
    EXPECT_CALL(mock_strerror, strerror(_))
        .Times(1)
        .WillOnce(Return((char*)"mocked error"));
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(1).WillOnce(Return(0));

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
    EXPECT_NE(fp, (FILE*)0x123456abcde0);
    EXPECT_EQ(is_stderr, 1);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, logStderrButNotToFile) {
    initwascalled = 1;
    setlogwascalled = 1;
    is_stderr = 1;

    EXPECT_CALL(mock_fopen, fopen((const char*)nullptr, "a")).Times(0);
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
    EXPECT_EQ(is_stderr, 1);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, logStderrAndUsingFile) {
    initwascalled = 1;
    setlogwascalled = 1;
    is_stderr = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = (char*)"gtest.log";

#ifdef OLD_TEST
    std::cout << "  BUG! fopen should not be called.\n";
    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a")).Times(1);
#else
    EXPECT_CALL(mock_fopen, fopen((const char*)"gtest.log", "a")).Times(0);
#endif
    EXPECT_CALL(mock_strerror, strerror(_))
        .Times(1)
        .WillOnce(Return((char*)"mocked error"));
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
    EXPECT_NE(fp, (FILE*)0x123456abcde0);
    EXPECT_STREQ(fileName, (char*)"gtest.log");
    EXPECT_EQ(is_stderr, 1);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, logStderrButNotUsingFile) {
    initwascalled = 1;
    setlogwascalled = 1;
    is_stderr = 1;
    fp = (FILE*)0x123456abcde0;
    fileName = nullptr;

    EXPECT_CALL(mock_fopen, fopen(nullptr, "a")).Times(0);
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    EXPECT_STREQ(UpnpGetErrorMessage(UpnpInitLog()), "UPNP_E_SUCCESS");

    EXPECT_NE(fp, nullptr);
#ifdef OLD_TEST
    std::cout << "  BUG! fp should be set to stderr.\n";
    EXPECT_EQ(fp, (FILE*)0x123456abcde0);
    EXPECT_EQ(is_stderr, 0);
#else
    EXPECT_NE(fp, (FILE*)0x123456abcde0) << "# fp should be set to stderr.";
    EXPECT_EQ(is_stderr, 1) << "# fp should be set to stderr.";
#endif
    EXPECT_EQ(fileName, nullptr);
    EXPECT_EQ(initwascalled, 1);
    EXPECT_EQ(setlogwascalled, 1);
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLog) {
    initwascalled = 1;
    pthread_mutex_init(&GlobalDebugMutex, NULL);

    // process the unit
    UpnpCloseLog();

    EXPECT_EQ(fp, nullptr);
    EXPECT_EQ(is_stderr, 0);
    EXPECT_EQ(initwascalled, 0);
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLogWithOpenLogfile) {
    pthread_mutex_init(&GlobalDebugMutex, NULL);
    initwascalled = 1;
    fp = (FILE*)0x123456abcdef;
    is_stderr = 0;
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(1);

    // process the unit
    UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, upnpCloseLogStderrOn) {
    pthread_mutex_init(&GlobalDebugMutex, NULL);
    initwascalled = 1;
    fp = (FILE*)0x123456abcdef;
    is_stderr = 1;
    EXPECT_CALL(mock_fclose, fclose(fp)).Times(0);

    // process the unit
    UpnpCloseLog();
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
