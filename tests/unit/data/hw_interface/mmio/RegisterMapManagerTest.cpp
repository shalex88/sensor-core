#include <gtest/gtest.h>
#include <gmock/gmock.h>
/* Add your project include files here */
#include "infrastructure/camera/transport/mmio/RegistersMapManager.h"
#include "../../../../Mocks.h"
#include <thread>

using namespace service::infrastructure;

class RegisterMapManagerTest : public Test {
public:
    RegisterMapManagerTest() {
        auto register_impl_obj = std::make_unique<NiceMock<MockRegisterImpl>>();
        register_impl = register_impl_obj.get();
        register_map = std::make_unique<RegistersMapManager>(std::move(register_impl_obj));
    }
    NiceMock<MockRegisterImpl>* register_impl {};
    std::unique_ptr<RegistersMapManager> register_map;
};

TEST_F(RegisterMapManagerTest, GetRegisterValue) {
    EXPECT_CALL(*register_impl, get(_))
            .WillOnce(Return(Result<uint32_t>::success(0xFFFF'FFFFu)));

    const auto result = register_map->getValue(REG::ZOOM);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 0xFFFF'FFFFu);
}

TEST_F(RegisterMapManagerTest, SetRegisterValue) {
    EXPECT_CALL(*register_impl, set(_, 0xFFFF'FFFFu))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->setValue(REG::ZOOM, 0xFFFF'FFFFu);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, ResetRegisterToDefault) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->resetValue(REG::ZOOM);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, ClearRegister) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->clearValue(REG::ZOOM);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, SetBit) {
    EXPECT_CALL(*register_impl, get(_))
            .WillOnce(Return(Result<uint32_t>::success(0x0000'0000u)));
    EXPECT_CALL(*register_impl, set(_, 0x0000'0001u))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->setBit(REG::ZOOM, 0);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, ClearBit) {
    EXPECT_CALL(*register_impl, get(_))
            .WillOnce(Return(Result<uint32_t>::success(0xFFFF'FFFFu)));
    EXPECT_CALL(*register_impl, set(_, 0xFFFF'FFFEu))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->clearBit(REG::ZOOM, 0);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, GetOrSetBitLargerThan31) {
    EXPECT_CALL(*register_impl, get(_)).Times(0);
    EXPECT_CALL(*register_impl, set(_, _)).Times(0);

    const auto setBitResult = register_map->setBit(REG::ZOOM, 32);
    EXPECT_TRUE(setBitResult.isError());
    EXPECT_EQ(setBitResult.error(), "Bit index out of range (0-31)");

    const auto clearBitResult = register_map->clearBit(REG::ZOOM, 32);
    EXPECT_TRUE(clearBitResult.isError());
    EXPECT_EQ(clearBitResult.error(), "Bit index out of range (0-31)");
}

TEST_F(RegisterMapManagerTest, GetNibble) {
    EXPECT_CALL(*register_impl, get(_))
            .WillOnce(Return(Result<uint32_t>::success(0xFFFF'FFFFu)));

    const auto result = register_map->getNibble(REG::ZOOM, 0);
    EXPECT_TRUE(result.isSuccess());
    EXPECT_EQ(result.value(), 0xF);
}

TEST_F(RegisterMapManagerTest, SetNibble) {
    EXPECT_CALL(*register_impl, get(_))
            .WillOnce(Return(Result<uint32_t>::success(0x0000'0000u)));
    EXPECT_CALL(*register_impl, set(_, 0x0000'000Fu))
            .WillOnce(Return(Result<void>::success()));

    const auto result = register_map->setNibble(REG::ZOOM, 0, 0xf);
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, GetSetWrongNibbleIndex) {
    EXPECT_CALL(*register_impl, get(_)).Times(0);
    EXPECT_CALL(*register_impl, set(_, _)).Times(0);

    const auto getNibbleResult = register_map->getNibble(REG::ZOOM, 8);
    EXPECT_TRUE(getNibbleResult.isError());
    EXPECT_EQ(getNibbleResult.error(), "Nibble index out of range (0-7)");

    const auto setNibbleResult = register_map->setNibble(REG::ZOOM, 8, 0xf);
    EXPECT_TRUE(setNibbleResult.isError());
    EXPECT_EQ(setNibbleResult.error(), "Nibble index out of range (0-7)");
}

TEST_F(RegisterMapManagerTest, SetWrongNibbleValue) {
    EXPECT_CALL(*register_impl, get(_)).Times(0);
    EXPECT_CALL(*register_impl, set(_, _)).Times(0);

    const auto result = register_map->setNibble(REG::ZOOM, 0, 0xff);
    EXPECT_TRUE(result.isError());
    EXPECT_EQ(result.error(), "Nibble value out of range (0-15)");
}

TEST_F(RegisterMapManagerTest, ResetAllToDefault) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillRepeatedly(Return(Result<void>::success()));

    const auto result = register_map->resetAll();
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, ClearAllRegisters) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillRepeatedly(Return(Result<void>::success()));

    const auto result = register_map->clearAll();
    EXPECT_TRUE(result.isSuccess());
}

TEST_F(RegisterMapManagerTest, ResetAllToDefaultFails) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillRepeatedly(Return(Result<void>::error("Register access failed")));

    const auto result = register_map->resetAll();
    EXPECT_TRUE(result.isError());
    EXPECT_THAT(result.error(), HasSubstr("Failed to reset register"));
}

TEST_F(RegisterMapManagerTest, ClearAllRegistersFails) {
    EXPECT_CALL(*register_impl, set(_, _))
            .WillRepeatedly(Return(Result<void>::error("Register access failed")));

    const auto result = register_map->clearAll();
    EXPECT_TRUE(result.isError());
    EXPECT_THAT(result.error(), HasSubstr("Failed to clear register"));
}

TEST_F(RegisterMapManagerTest, SetRegisterValueThreadSafety) {
    ON_CALL(*register_impl, set(_, _))
            .WillByDefault(Return(Result<void>::success()));

    ON_CALL(*register_impl, get(_))
            .WillByDefault(Return(Result<uint32_t>::success(0u)));

    std::atomic error_flag(false);
    std::atomic completed_operations(0);
    constexpr int opertions_per_thread = 100;

    std::jthread thread1([&] {
        for (int i = 0; i < opertions_per_thread; ++i) {
            if (const auto result = register_map->setValue(REG::ZOOM, 0xFFFF'FFFF); result.isError()) {
                error_flag.store(true);
                return;
            }
            completed_operations.fetch_add(1);
        }
    });

    std::jthread thread2([&] {
        for (int i = 0; i < opertions_per_thread; ++i) {
            if (const auto result = register_map->setValue(REG::ZOOM, 0x0000'0000); result.isError()) {
                error_flag.store(true);
                return;
            }
            completed_operations.fetch_add(1);
        }
    });

    thread1.join();
    thread2.join();

    EXPECT_FALSE(error_flag.load());
    EXPECT_EQ(completed_operations.load(), opertions_per_thread*2);  // Both threads completed all operations
}