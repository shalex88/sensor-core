#include <gmock/gmock.h>
#include <gtest/gtest.h>
/* Add your project include files here */
#include "common/types/CameraTypes.h"
#include "common/types/Result.h"

using namespace service;
using namespace testing;

class ResultTests : public Test {
};

TEST_F(ResultTests, VoidResultSuccessCreation) {
    const auto result = Result<void>::success();
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
}

TEST_F(ResultTests, VoidResultErrorCreation) {
    const auto result = Result<void>::error("Test error");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Test error");
}

TEST_F(ResultTests, VoidResultErrorWithEmptyString) {
    const auto result = Result<void>::error("");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "");
}

TEST_F(ResultTests, VoidResultMoveError) {
    auto result = Result<void>::error("Test error");
    const auto error = std::move(result).error();
    EXPECT_EQ(error, "Test error");
}

TEST_F(ResultTests, IntResultSuccessCreation) {
    const auto result = Result<int>::success(42);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), 42);
}

TEST_F(ResultTests, IntResultErrorCreation) {
    const auto result = Result<int>::error("Failed to get value");
    EXPECT_FALSE(result.isSuccess());
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Failed to get value");
}

TEST_F(ResultTests, StringResultSuccessCreation) {
    const auto result = Result<std::string>::success("Hello World");
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), "Hello World");
}

TEST_F(ResultTests, StringResultMoveValue) {
    auto result = Result<std::string>::success("Hello World");
    const auto value = std::move(result).value();
    EXPECT_EQ(value, "Hello World");
}

TEST_F(ResultTests, UnsignedIntResultSuccessCreation) {
    const auto result = Result<uint32_t>::success(100u);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.isError());
    EXPECT_EQ(result.value(), 100u);
}

TEST_F(ResultTests, BoolResultSuccessTrue) {
    const auto result = Result<bool>::success(true);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_TRUE(result.value());
}

TEST_F(ResultTests, BoolResultSuccessFalse) {
    const auto result = Result<bool>::success(false);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_FALSE(result.value());
}

TEST_F(ResultTests, AccessingErrorOnSuccessThrows) {
    const auto result = Result<int>::success(42);
    EXPECT_THROW({
        [[maybe_unused]] const auto& err = result.error();
    }, std::bad_variant_access);
}

TEST_F(ResultTests, AccessingValueOnErrorThrows) {
    const auto result = Result<int>::error("Error");
    EXPECT_THROW({
        [[maybe_unused]] const auto& val = result.value();
    }, std::bad_variant_access);
}

TEST_F(ResultTests, ResultWithCustomType) {
    struct CustomData {
        int x;
        std::string name;
        bool operator==(const CustomData& other) const {
            return x == other.x && name == other.name;
        }
    };

    const CustomData data{42, "test"};
    const auto result = Result<CustomData>::success(data);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value().x, 42);
    EXPECT_EQ(result.value().name, "test");
}

TEST_F(ResultTests, ResultWithZeroValue) {
    const auto result = Result<int>::success(0);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 0);
}

TEST_F(ResultTests, ResultWithNegativeValue) {
    const auto result = Result<int>::success(-42);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), -42);
}

TEST_F(ResultTests, ResultWithMaxInt) {
    const auto result = Result<int>::success(std::numeric_limits<int>::max());
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), std::numeric_limits<int>::max());
}

TEST_F(ResultTests, ResultWithMinInt) {
    const auto result = Result<int>::success(std::numeric_limits<int>::min());
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), std::numeric_limits<int>::min());
}

TEST_F(ResultTests, ConstReferenceAccess) {
    const auto result = Result<std::string>::success("const test");
    const std::string& value_ref = result.value();
    EXPECT_EQ(value_ref, "const test");
}

TEST_F(ResultTests, MoveSemantics) {
    auto result = Result<std::string>::success("move test");
    auto moved_value = std::move(result).value();
    EXPECT_EQ(moved_value, "move test");
}

TEST_F(ResultTests, ErrorMoveSemantics) {
    auto result = Result<int>::error("error message");
    auto moved_error = std::move(result).error();
    EXPECT_EQ(moved_error, "error message");
}

TEST_F(ResultTests, ZoomTypeSuccess) {
    const auto result = Result<common::types::zoom>::success(50u);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 50u);
}

TEST_F(ResultTests, FocusTypeSuccess) {
    const auto result = Result<common::types::focus>::success(25u);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 25u);
}

TEST_F(ResultTests, ZoomTypeError) {
    const auto result = Result<common::types::zoom>::error("Invalid zoom value");
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Invalid zoom value");
}

TEST_F(ResultTests, FocusTypeError) {
    const auto result = Result<common::types::focus>::error("Invalid focus value");
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Invalid focus value");
}

TEST_F(ResultTests, CustomErrorType) {
    enum class ErrorCode {
        Unknown,
        InvalidArgument,
        Timeout
    };

    const auto result = Result<int, ErrorCode>::error(ErrorCode::InvalidArgument);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), ErrorCode::InvalidArgument);
}

TEST_F(ResultTests, IntErrorType) {
    const auto result = Result<std::string, int>::error(-1);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), -1);
}
