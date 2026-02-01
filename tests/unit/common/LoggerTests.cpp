#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "common/logger/Logger.h"

#include "common/logger/LoggerInterface.h"

using namespace testing;

class MockLoggerAdapter : public LoggerInterface {
public:
    MOCK_METHOD(void, setLogLevel, (LogLevel), (override));
    MOCK_METHOD(void, setLogLevel, (const std::string&), (override));
    MOCK_METHOD(void, logImpl, (LogLevel, const std::string&), (override));
};

class LoggerTest: public Test {
protected:
    void SetUp() override {
        auto mock = std::make_shared<NiceMock<MockLoggerAdapter>>(); // Prevent side effects caused by the singleton
        mock_logger = mock.get(); // Keep a raw pointer for expectations
        service::common::LoggerRegistry::instance().setLoggerAdapter(std::move(mock), "info");
    }

    void TearDown() override {
        service::common::LoggerRegistry::instance().setLoggerAdapter(std::make_shared<SpdLogAdapter>(), "info"); // Reset the logger adapter to avoid side effects
    }

    NiceMock<MockLoggerAdapter>* mock_logger {};
};

TEST_F(LoggerTest, SetLogLevelToTrace) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Trace));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Trace);
}

TEST_F(LoggerTest, SetLogLevelToDebug) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Debug));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Debug);
}

TEST_F(LoggerTest, SetLogLevelToInfo) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Info));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Info);
}

TEST_F(LoggerTest, SetLogLevelToWarn) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Warn));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Warn);
}

TEST_F(LoggerTest, SetLogLevelToError) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Error));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Error);
}

TEST_F(LoggerTest, SetLogLevelToCritical) {
    EXPECT_CALL(*mock_logger, setLogLevel(LoggerInterface::LogLevel::Critical));
    SET_LOG_LEVEL(LoggerInterface::LogLevel::Critical);
}

TEST_F(LoggerTest, LogTrace) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Trace, ::testing::HasSubstr("Test message")));
    LOG_TRACE("Test message");
}

TEST_F(LoggerTest, LogDebug) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Debug, ::testing::HasSubstr("Test message")));
    LOG_DEBUG("Test message");
}

TEST_F(LoggerTest, LogInfo) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Info, ::testing::HasSubstr("Test message")));
    LOG_INFO("Test message");
}

TEST_F(LoggerTest, LogWarn) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Warn, ::testing::HasSubstr("Test message")));
    LOG_WARN("Test message");
}

TEST_F(LoggerTest, LogError) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Error, ::testing::HasSubstr("Test message")));
    LOG_ERROR("Test message");
}

TEST_F(LoggerTest, LogCritical) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Critical, ::testing::HasSubstr("Test message")));
    LOG_CRITICAL("Test message");
}

TEST_F(LoggerTest, LogWithFormatting) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Info, ::testing::HasSubstr("Value: 42")));
    LOG_INFO("Value: {}", 42);
}

TEST_F(LoggerTest, LogWithMultipleArgs) {
    EXPECT_CALL(*mock_logger, logImpl(LoggerInterface::LogLevel::Info, ::testing::HasSubstr("Hello World 42")));
    LOG_INFO("{} {} {}", "Hello", "World", 42);
}
