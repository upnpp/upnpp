// Copyright (C) 2021 GPL 3 and higher by Ingo Höft,  <Ingo@Hoeft-online.de>
// Redistribution only with this Copyright remark. Last modified: 2021-10-17

#include "upnpmock/pthreadif.hpp"
#include "upnpmock/stdioif.hpp"
#include "upnpmock/stringif.hpp"
#include "gmock/gmock.h"

#include "port_unistd.hpp"
#include "tools.hpp"
#include <string>

#include "api/upnpdebug.cpp"

using ::testing::_;
using ::testing::MatchesRegex;
using ::testing::Return;

namespace upnp {

//
// Interface for the upnpdebug module
// ----------------------------------
class Iupnpdebug {
  public:
    virtual ~Iupnpdebug() {}
    virtual int UpnpInitLog(void) = 0;
    virtual void UpnpSetLogLevel(Upnp_LogLevel log_level) = 0;
    virtual void UpnpCloseLog(void) = 0;
    virtual void UpnpSetLogFileNames(const char* newFileName,
                                     const char* ignored) = 0;
    virtual void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module,
                            const char* DbgFileName, int DbgLineNo,
                            const char* FmtStr, ...) = 0;
    virtual FILE* UpnpGetDebugFile(Upnp_LogLevel DLevel, Dbg_Module Module) = 0;
};

class Cupnpdebug : public Iupnpdebug {
  public:
    virtual ~Cupnpdebug() {}

    int UpnpInitLog(void) override { return ::UpnpInitLog(); }
    void UpnpSetLogLevel(Upnp_LogLevel log_level) override {
        ::UpnpSetLogLevel(log_level);
    }
    void UpnpCloseLog(void) override { ::UpnpCloseLog(); }
    void UpnpSetLogFileNames(const char* newFileName,
                             const char* ignored) override {
        ::UpnpSetLogFileNames(newFileName, ignored);
    }
    void UpnpPrintf(Upnp_LogLevel DLevel, Dbg_Module Module,
                    const char* DbgFileName, int DbgLineNo, const char* FmtStr,
                    ...) override {
        return;
    }

    FILE* UpnpGetDebugFile(Upnp_LogLevel DLevel, Dbg_Module Module) override {
        return ::UpnpGetDebugFile(DLevel, Module);
    }
};

//
// Mocked system calls
// -------------------
// See the respective include files in upnp/include/upnpmock/
class Mock_string : public Istring {
    // Class to mock the free system functions.
    Istring* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_string() {
        m_oldptr = stringif;
        stringif = this;
    }
    virtual ~Mock_string() { stringif = m_oldptr; }

    MOCK_METHOD(char*, strerror, (int errnum), (override));
};

class Mock_stdio : public Istdio {
    // Class to mock the free system functions.
    Istdio* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_stdio() {
        m_oldptr = stdioif;
        stdioif = this;
    }
    virtual ~Mock_stdio() { stdioif = m_oldptr; }

    MOCK_METHOD(FILE*, fopen, (const char* pathname, const char* mode),
                (override));
    MOCK_METHOD(int, fclose, (FILE * stream), (override));
    MOCK_METHOD(int, fflush, (FILE * stream), (override));
};

class Mock_pthread : public Ipthread {
    // Class to mock the free system functions.
    Ipthread* m_oldptr;

  public:
    // Save and restore the old pointer to the production function
    Mock_pthread() {
        m_oldptr = pthreadif;
        pthreadif = this;
    }
    virtual ~Mock_pthread() { pthreadif = m_oldptr; }

    MOCK_METHOD(int, pthread_mutex_init,
                (pthread_mutex_t * mutex, const pthread_mutexattr_t* mutexattr),
                (override));
    MOCK_METHOD(int, pthread_mutex_lock, (pthread_mutex_t * mutex), (override));
    MOCK_METHOD(int, pthread_mutex_unlock, (pthread_mutex_t * mutex),
                (override));
    MOCK_METHOD(int, pthread_mutex_destroy, (pthread_mutex_t * mutex),
                (override));
};

//
// Test class for the debugging and logging module
//------------------------------------------------
class UpnpdebugMockTestSuite : public ::testing::Test {
  protected:
    // Member variables: instantiate the module object
    Cupnpdebug upnpdebugObj;

    // instantiate the mock objects.
    Mock_pthread mocked_pthread;
    Mock_stdio mocked_stdio;
    Mock_string mocked_string;

    // constructor
    UpnpdebugMockTestSuite() {
        // Clear the static variables of the unit
        g_log_level = UPNP_DEFAULT_LOG_LEVEL;
        fp = nullptr;
        is_stderr = 0;
        setlogwascalled = 0;
        initwascalled = 0;
        fileName = nullptr;
    }
};

TEST_F(UpnpdebugMockTestSuite, initlog_but_no_log_wanted)
// For the pthread_mutex_t structure look at
// https://stackoverflow.com/q/23449508/5014688
{
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    // Process unit again
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_all_log_level) {
    // Set logging for all levels
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_info) {
    // Process unit
    upnpdebugObj.UpnpSetLogLevel(UPNP_INFO);

    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");

    // Check if logging to stderr with enabled log_level is now enabled.
    // Setting parameter to NULL returns if a filepointer is set for any of the
    // options.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);
    // You can ask if a filepointer is set for a particular Upnp_logLevel.
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, API), nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);
#ifdef OLD_TEST
    // It seems that option Dbg_Module is ignored. It cannot be set with
    // UpnpSetLogLevel().
    std::cout << "  BUG! Parameter Dbg_Module should not be ignored.\n";
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, DOM), stderr);
#else
    // Only one Dbg_Module should be used.
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, API), stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, DOM), nullptr)
        << "  # Parameter Dbg_Module should not be ignored.";
#endif

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_error) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ERROR);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, set_log_level_critical) {
    // Set logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");

    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_but_not_to_fIle) {
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);

    // Just set the log level but no filename. This should log to stderr.
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);

    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");

    // Check logging by log file pointer
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              stderr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ERROR, (Dbg_Module)NULL),
              nullptr);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_ALL, (Dbg_Module)NULL),
              nullptr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_to_file) {
    // Set the filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    // Check if logging is enabled. It should not.
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x123456abcdef);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x5a5a5a5a5a5a));

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Get file pointer
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        (FILE*)0x5a5a5a5a5a5a);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_not_stderr_but_opening_file_fails) {
    // Set the filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).WillOnce(Return((FILE*)NULL));

    char* errmsg = strdup("mocked error");
    EXPECT_CALL(mocked_string, strerror(_)).WillOnce(Return(errmsg));
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
#ifdef OLD_TEST
    std::cout << "  BUG! UpnpInitLog() should return with failure.\n";
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
#else
    EXPECT_STRNE(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
#endif
    free(errmsg);

    // Will be set to stderr if failed to log to a file
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_using_file) {
    // This should set logging but not enable it
    upnpdebugObj.UpnpSetLogLevel(UPNP_CRITICAL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should enable logging and log to stderr
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // Now we set the filename, second parameter is unused but defined.
    // This set the filename but does not touch previous filepointer.
    upnpdebugObj.UpnpSetLogFileNames("upnpdebug.log", nullptr);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, "a"))
        .WillOnce(Return((FILE*)0x123456abcdef));
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_string, strerror(_)).Times(0);

    // Process unit. This should open a filepointer to file with set filename.
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_CRITICAL, (Dbg_Module)NULL),
              (FILE*)0x123456abcdef);
    EXPECT_EQ(upnpdebugObj.UpnpGetDebugFile(UPNP_INFO, (Dbg_Module)NULL),
              nullptr);

    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, log_stderr_and_to_file_with_wrong_filename) {
    // This should enable logging
    upnpdebugObj.UpnpSetLogLevel(UPNP_ALL);
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(1);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);

    // Process unit
    // No filename set, this should log to stderr
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Log to stderr
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // Now we set a wrong filename, second parameter is unused but defined
    upnpdebugObj.UpnpSetLogFileNames("", nullptr);

    EXPECT_CALL(mocked_pthread, pthread_mutex_init(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fopen(_, _)).Times(0);
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_string, strerror(_)).Times(0);

    // Process unit
    EXPECT_STREQ(UpnpGetErrorMessage(upnpdebugObj.UpnpInitLog()),
                 "UPNP_E_SUCCESS");
    // Filepointer is still set to stderr, that seems to be ok so far ...
    EXPECT_EQ(
        upnpdebugObj.UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
        stderr);

    // ... but it should not try to close stderr.
#ifdef OLD_TEST
    std::cout << "  BUG! UpnpCloseLog() tries to close stderr.\n";
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(1);
#else
    std::cout << "  # UpnpCloseLog() tries to close stderr.\n";
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
#endif
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(1);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(1);
    upnpdebugObj.UpnpCloseLog();
}

TEST_F(UpnpdebugMockTestSuite, close_log_without_init_log) {
    EXPECT_CALL(mocked_stdio, fclose(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_lock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_unlock(_)).Times(0);
    EXPECT_CALL(mocked_pthread, pthread_mutex_destroy(_)).Times(0);
    // Process unit
    upnpdebugObj.UpnpCloseLog();
}

TEST(UpnpdebugTestSuite, UpnpPrintf_without_init) {
    // Process unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s. It should not be called.\n", "UpnpPrintf");
    // This will enable logging but no initializing
    ::UpnpSetLogLevel(UPNP_ALL);
    // Process unit
    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s with logging enabled but not "
                 "initialized. It should not be called.\n",
                 "UpnpPrintf");
}

TEST(UpnpdebugTestSuite, UpnpPrintf_normal_use) {
    CCaptureStdOutErr captureObj(STDERR_FILENO);
    std::string captured;

    // Enable and initialize logging
    ::UpnpSetLogLevel(UPNP_ALL);

    EXPECT_STREQ(UpnpGetErrorMessage(::UpnpInitLog()), "UPNP_E_SUCCESS");
    EXPECT_EQ(::UpnpGetDebugFile((Upnp_LogLevel)NULL, (Dbg_Module)NULL),
              stderr);

    ASSERT_TRUE(captureObj.start());

    ::UpnpPrintf(UPNP_INFO, API, __FILE__, __LINE__,
                 "Unit Test for %s on line %d.\n", "UpnpPrintf", __LINE__);

    ASSERT_TRUE(captureObj.get(captured));

    // Example: "2021-10-17 21:09:01 UPNP-API_-2: Thread:0x7F1366618740
    // [/home/ingo/devel/upnplib-dev/upnplib/gtests/test_upnpdebug.cpp:535]:
    // Unit Test for UpnpPrintf on line 536.\n"
    EXPECT_THAT(
        captured,
        MatchesRegex("....-..-.. ..:..:.. UPNP-API_-2: "
                     "Thread:0x.+ \\[.+\\]: Unit Test for UpnpPrintf on line "
                     ".+\\.\n"));
}

} // namespace upnp

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
