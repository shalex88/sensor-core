#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "common/logger/SpdLogAdapter.h"

#include <sstream>

using namespace testing;

class SpdLogAdapterTests : public Test {
protected:
    void SetUp() override {
        // Redirect cout to our stringstream
        old_cout_buf = std::cout.rdbuf(output.rdbuf());
    }

    void TearDown() override {
        // Restore cout's original buffer
        std::cout.rdbuf(old_cout_buf);
    }

    std::stringstream output {};
    std::streambuf* old_cout_buf {};
    SpdLogAdapter logger;
};

TEST_F(SpdLogAdapterTests, LogsTraceMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Trace);
    logger.logImpl(LoggerInterface::LogLevel::Trace, "Test message");
    EXPECT_THAT(output.str(), HasSubstr("Test message"));
    EXPECT_THAT(output.str(), HasSubstr("trace"));
}

TEST_F(SpdLogAdapterTests, LogsDebugMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Debug);
    logger.logImpl(LoggerInterface::LogLevel::Debug, "Test message");
    EXPECT_THAT(output.str(), HasSubstr("Test message"));
    EXPECT_THAT(output.str(), HasSubstr("debug"));
}

TEST_F(SpdLogAdapterTests, LogsInfoMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Info);
    logger.logImpl(LoggerInterface::LogLevel::Info, "Test message");
    EXPECT_THAT(output.str(), HasSubstr("Test message"));
    EXPECT_THAT(output.str(), HasSubstr("info"));
}

TEST_F(SpdLogAdapterTests, LogsWarnMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Warn);
    logger.logImpl(LoggerInterface::LogLevel::Warn, "Test message");
    EXPECT_THAT(output.str(), HasSubstr("Test message"));
    EXPECT_THAT(output.str(), HasSubstr("warn"));
}

TEST_F(SpdLogAdapterTests, LogsErrorMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Error);
    logger.logImpl(LoggerInterface::LogLevel::Error, "Error message");
    EXPECT_THAT(output.str(), HasSubstr("Error message"));
    EXPECT_THAT(output.str(), HasSubstr("error"));
}

TEST_F(SpdLogAdapterTests, LogsCriticalMessage) {
    logger.setLogLevel(LoggerInterface::LogLevel::Critical);
    logger.logImpl(LoggerInterface::LogLevel::Critical, "Error message");
    EXPECT_THAT(output.str(), HasSubstr("Error message"));
    EXPECT_THAT(output.str(), HasSubstr("critical"));
}

TEST_F(SpdLogAdapterTests, DoesNotLogDebugWhenInfoLevel) {
    logger.setLogLevel(LoggerInterface::LogLevel::Info);
    logger.logImpl(LoggerInterface::LogLevel::Debug, "Debug message");
    EXPECT_THAT(output.str(), Not(HasSubstr("Debug message")));
}

TEST_F(SpdLogAdapterTests, LogsAllLevelsWhenTraceEnabled) {
    logger.setLogLevel(LoggerInterface::LogLevel::Trace);

    logger.logImpl(LoggerInterface::LogLevel::Trace, "Trace message");
    logger.logImpl(LoggerInterface::LogLevel::Debug, "Debug message");
    logger.logImpl(LoggerInterface::LogLevel::Info, "Info message");

    EXPECT_THAT(output.str(), HasSubstr("Trace message"));
    EXPECT_THAT(output.str(), HasSubstr("Debug message"));
    EXPECT_THAT(output.str(), HasSubstr("Info message"));
}

TEST_F(SpdLogAdapterTests, LogsHigherSeverityWhenLowerDisabled) {
    logger.setLogLevel(LoggerInterface::LogLevel::Error);

    logger.logImpl(LoggerInterface::LogLevel::Info, "Info message");
    logger.logImpl(LoggerInterface::LogLevel::Error, "Error message");
    logger.logImpl(LoggerInterface::LogLevel::Critical, "Critical message");

    EXPECT_THAT(output.str(), Not(HasSubstr("Info message")));
    EXPECT_THAT(output.str(), HasSubstr("Error message"));
    EXPECT_THAT(output.str(), HasSubstr("Critical message"));
}

TEST_F(SpdLogAdapterTests, DefaultsToInfoOnInvalidLogLevel) {
    logger.logImpl(static_cast<LoggerInterface::LogLevel>(999), "Invalid");
    EXPECT_THAT(output.str(), HasSubstr("Invalid"));
}
