#include <gtest/gtest.h>
/* Add your project include files here */
#include "common/types/CameraTypes.h"

using namespace service::common::types;

class CameraTypesTests : public ::testing::Test {
};

TEST_F(CameraTypesTests, ZoomRangeDefaultConstruction) {
    const ZoomRange range{};
    EXPECT_EQ(range.min, 0u);
    EXPECT_EQ(range.max, 0u);
}

TEST_F(CameraTypesTests, ZoomRangeInitialization) {
    constexpr ZoomRange range{.min = 10, .max = 100};
    EXPECT_EQ(range.min, 10u);
    EXPECT_EQ(range.max, 100u);
}

TEST_F(CameraTypesTests, ZoomRangeValidRange) {
    constexpr ZoomRange range{.min = 0, .max = 1000};
    EXPECT_LE(range.min, range.max);
}

TEST_F(CameraTypesTests, ZoomRangeSameMinMax) {
    constexpr ZoomRange range{.min = 50, .max = 50};
    EXPECT_EQ(range.min, range.max);
}

TEST_F(CameraTypesTests, ZoomRangeMaxValue) {
    constexpr ZoomRange range{.min = 0, .max = UINT32_MAX};
    EXPECT_EQ(range.max, UINT32_MAX);
}

TEST_F(CameraTypesTests, ZoomRangeCopyConstruction) {
    constexpr ZoomRange original{.min = 10, .max = 100};
    const ZoomRange copy = original;
    EXPECT_EQ(copy.min, original.min);
    EXPECT_EQ(copy.max, original.max);
}

TEST_F(CameraTypesTests, ZoomRangeAssignment) {
    constexpr ZoomRange range1{.min = 10, .max = 100};
    ZoomRange range2{.min = 0, .max = 0};
    range2 = range1;
    EXPECT_EQ(range2.min, range1.min);
    EXPECT_EQ(range2.max, range1.max);
}

TEST_F(CameraTypesTests, FocusRangeDefaultConstruction) {
    const FocusRange range{};
    EXPECT_EQ(range.min, 0u);
    EXPECT_EQ(range.max, 0u);
}

TEST_F(CameraTypesTests, FocusRangeInitialization) {
    constexpr FocusRange range{.min = 5, .max = 50};
    EXPECT_EQ(range.min, 5u);
    EXPECT_EQ(range.max, 50u);
}

TEST_F(CameraTypesTests, FocusRangeValidRange) {
    constexpr FocusRange range{.min = 0, .max = 500};
    EXPECT_LE(range.min, range.max);
}

TEST_F(CameraTypesTests, FocusRangeSameMinMax) {
    constexpr FocusRange range{.min = 25, .max = 25};
    EXPECT_EQ(range.min, range.max);
}

TEST_F(CameraTypesTests, FocusRangeMaxValue) {
    constexpr FocusRange range{.min = 0, .max = UINT32_MAX};
    EXPECT_EQ(range.max, UINT32_MAX);
}

TEST_F(CameraTypesTests, FocusRangeCopyConstruction) {
    constexpr FocusRange original{.min = 5, .max = 50};
    const FocusRange copy = original;
    EXPECT_EQ(copy.min, original.min);
    EXPECT_EQ(copy.max, original.max);
}

TEST_F(CameraTypesTests, FocusRangeAssignment) {
    constexpr FocusRange range1{.min = 5, .max = 50};
    FocusRange range2{.min = 0, .max = 0};
    range2 = range1;
    EXPECT_EQ(range2.min, range1.min);
    EXPECT_EQ(range2.max, range1.max);
}

TEST_F(CameraTypesTests, ZoomTypeIsUint32) {
    constexpr zoom z = 100u;
    EXPECT_EQ(z, 100u);
    static_assert(std::is_same_v<zoom, uint32_t>, "zoom should be uint32_t");
}

TEST_F(CameraTypesTests, FocusTypeIsUint32) {
    constexpr focus f = 50u;
    EXPECT_EQ(f, 50u);
    static_assert(std::is_same_v<focus, uint32_t>, "focus should be uint32_t");
}

TEST_F(CameraTypesTests, InfoTypeIsString) {
    const info i = "Camera Info";
    EXPECT_EQ(i, "Camera Info");
    static_assert(std::is_same_v<info, std::string>, "info should be std::string");
}

TEST_F(CameraTypesTests, ZoomBoundaryValues) {
    constexpr zoom min_zoom = 0u;
    constexpr zoom max_zoom = UINT32_MAX;
    EXPECT_EQ(min_zoom, 0u);
    EXPECT_EQ(max_zoom, UINT32_MAX);
}

TEST_F(CameraTypesTests, FocusBoundaryValues) {
    constexpr focus min_focus = 0u;
    constexpr focus max_focus = UINT32_MAX;
    EXPECT_EQ(min_focus, 0u);
    EXPECT_EQ(max_focus, UINT32_MAX);
}

TEST_F(CameraTypesTests, InfoEmptyString) {
    const info empty_info = "";
    EXPECT_TRUE(empty_info.empty());
}

TEST_F(CameraTypesTests, InfoLongString) {
    const info long_info = std::string(1000, 'x');
    EXPECT_EQ(long_info.length(), 1000u);
}
