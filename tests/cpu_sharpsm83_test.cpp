#include <gtest/gtest.h>
#include "cpu_sharpsm83.hpp"

// Helper to reset and access registers
class Sharpsm83Test : public ::testing::Test {
protected:
    sharpsm83 cpu;
    void SetUp() override { cpu.reset(); }
};

TEST_F(Sharpsm83Test, FlagOperations) {
    cpu.set_zero_flag(true);
    EXPECT_TRUE(cpu.get_zero_flag());
    cpu.set_zero_flag(false);
    EXPECT_FALSE(cpu.get_zero_flag());

    cpu.set_subtraction_flag(true);
    EXPECT_TRUE(cpu.get_subtraction_flag());
    cpu.set_subtraction_flag(false);
    EXPECT_FALSE(cpu.get_subtraction_flag());

    cpu.set_half_carry_flag(true);
    EXPECT_TRUE(cpu.getHalfCarryFlag());
    cpu.set_half_carry_flag(false);
    EXPECT_FALSE(cpu.getHalfCarryFlag());

    cpu.set_carry_flag(true);
    EXPECT_TRUE(cpu.get_carry_flag());
    cpu.set_carry_flag(false);
    EXPECT_FALSE(cpu.get_carry_flag());

    bool carry_before = cpu.get_carry_flag();
    cpu.complement_carry_flag();
    EXPECT_NE(cpu.get_carry_flag(), carry_before);
}

TEST_F(Sharpsm83Test, RLC_RL_RRC_RR) {
    sharpsm83::reg8 reg;
    reg.b0_7 = 0b10000001;
    cpu.rlc(reg);
    EXPECT_EQ(reg.b0_7, 0b00000011);
    EXPECT_TRUE(cpu.get_carry_flag());

    reg.b0_7 = 0b10000001;
    cpu.set_carry_flag(false);
    cpu.rl(reg);
    EXPECT_EQ(reg.b0_7, 0b00000010);

    reg.b0_7 = 0b10000001;
    cpu.rrc(reg);
    EXPECT_EQ(reg.b0_7, 0b11000000);

    reg.b0_7 = 0b10000001;
    cpu.set_carry_flag(false);
    cpu.rr(reg);
    EXPECT_EQ(reg.b0_7, 0b01000000);
}

TEST_F(Sharpsm83Test, INC_DEC_Operations) {
    uint8_t reg8 = 0x0F;
    cpu.inc(reg8);
    EXPECT_EQ(reg8, 0x10);
    EXPECT_FALSE(cpu.get_subtraction_flag());

    cpu.dec(reg8);
    EXPECT_EQ(reg8, 0x0F);
    EXPECT_TRUE(cpu.get_subtraction_flag());

    uint16_t reg16 = 0xFFFF;
    cpu.inc(reg16);
    EXPECT_EQ(reg16, 0x0000);

    cpu.dec(reg16);
    EXPECT_EQ(reg16, 0xFFFF);
}

TEST_F(Sharpsm83Test, ADD_ADC_SUB_SBC_AND_XOR_OR_CP) {
    uint8_t a = 0x10, b = 0x20;
    cpu.add(a, b);
    EXPECT_EQ(a, 0x30);

    a = 0x10; b = 0x20;
    cpu.adc(a, b);
    EXPECT_EQ(a, 0x30);

    a = 0x30; b = 0x10;
    cpu.sub(a, b);
    EXPECT_EQ(a, 0x20);

    a = 0x30; b = 0x10;
    cpu.sbc(a, b);
    EXPECT_EQ(a, 0x20);

    a = 0xF0; b = 0x0F;
    cpu.and_op(a, b);
    EXPECT_EQ(a, 0x00);

    a = 0xF0; b = 0x0F;
    cpu.xor_op(a, b);
    EXPECT_EQ(a, 0xFF);

    a = 0xF0; b = 0x0F;
    cpu.or_op(a, b);
    EXPECT_EQ(a, 0xFF);

    a = 0x10; b = 0x10;
    cpu.cp_op(a, b);
    EXPECT_TRUE(cpu.get_zero_flag());
}

TEST_F(Sharpsm83Test, MemoryReadWrite) {
    uint16_t addr = 0x1234;
    uint8_t val = 0xAB;
    cpu.write_data(addr, val);
    cpu.fetch_data(addr);
    EXPECT_EQ(cpu.fetched_data, val);
}

TEST_F(Sharpsm83Test, LD_Operations) {
    uint8_t val = 0x55;
    uint8_t to = 0x00;
    cpu.ld(to, val);
    EXPECT_EQ(to, val);
}

TEST_F(Sharpsm83Test, Swap_SRL_SRA) {
    sharpsm83::reg8 reg;
    reg.b0_7 = 0xF0;
    cpu.swap_param(reg);
    EXPECT_EQ(reg.b0_7, 0x0F);

    reg.b0_7 = 0x80;
    cpu.srl_param(reg);
    EXPECT_EQ(reg.b0_7, 0x40);

    reg.b0_7 = 0x81;
    cpu.sra_param(reg);
    EXPECT_EQ(reg.b0_7, 0xC0);
}

TEST_F(Sharpsm83Test, Bit_Res_Set) {
    sharpsm83::reg8 reg;
    reg.b0_7 = 0xFF;
    cpu.res_param(0, reg);
    EXPECT_EQ(reg.b0_7, 0xFE);

    cpu.set_param(0, reg);
    EXPECT_EQ(reg.b0_7, 0xFF);

    cpu.bit_param(7, reg);
    EXPECT_FALSE(cpu.get_zero_flag());
    cpu.bit_param(0, reg);
    EXPECT_FALSE(cpu.get_zero_flag());
    reg.b0_7 = 0x00;
    cpu.bit_param(0, reg);
    EXPECT_TRUE(cpu.get_zero_flag());
}