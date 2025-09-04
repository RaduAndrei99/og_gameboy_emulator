#include<iostream>
#include "cpu_sharpsm83.hpp"

//##############################################################################
sharpsm83::sharpsm83()
{
    mem.init_memory();
    initialize_opcodes();
}
//##############################################################################
sharpsm83::~sharpsm83() = default;
//##############################################################################
bool sharpsm83::get_zero_flag()
{
    // z flag
    return AF.Lo.b7;
}
//##############################################################################
void sharpsm83::set_zero_flag(bool val)
{
    // z flag
    AF.Lo.b7 = val;
}
//##############################################################################
bool sharpsm83::get_subtraction_flag()
{
    // n flag
    return AF.Lo.b6;
}
//##############################################################################
void sharpsm83::set_subtraction_flag(bool val)
{
    // n flag
    AF.Lo.b6 = val;
}
//##############################################################################
bool sharpsm83::getHalfCarryFlag()
{
    // h flag
    return AF.Lo.b5;
}
//##############################################################################
void sharpsm83::set_half_carry_flag(bool val)
{
    // h flag
    AF.Lo.b5 = val;
}
//##############################################################################
bool sharpsm83::get_carry_flag()
{
    // c or Cy flag
    return AF.Lo.b4;
}
//##############################################################################
void sharpsm83::set_carry_flag(bool val)
{
    // c or Cy flag
    AF.Lo.b4 = val;
}
//##############################################################################
void sharpsm83::set_carry_flag()
{
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag

    set_carry_flag(true); // C flag

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::complement_carry_flag()
{
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag

    set_carry_flag(!get_carry_flag()); // C flag

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::fetch_data(const uint16_t& address)
{  
    fetched_data = mem.read(address);
}
//##############################################################################
void sharpsm83::write_data(const uint16_t& address, const uint8_t& data)
{
    mem.write(address, data);
}
//##############################################################################
void sharpsm83::execute(uint8_t opcode) 
{
    if(opcode == 0xCB)
    {
        execute_0xCB_instruction(opcode);
    }
    else
    {
        // TODO: fetch next instruction
        execute_normal_instruction(opcode);
    }
}
//##############################################################################
void sharpsm83::execute_normal_instruction(uint8_t opcode)
{
    std::cout<<"Executing: 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';

    if(opcode_table[opcode])
    {
        opcode_table[opcode]();
    }
    else
    {
        std::cout<<"Unrecognized instruction: 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';
    }
}
//##############################################################################
void sharpsm83::execute_0xCB_instruction(uint8_t opcode)
{
    std::cout<<"Executing: 0xCB 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';

    if(CB_opcode_table[opcode])
    {
        CB_opcode_table[opcode]();
    }
    else
    {
        std::cout<<"Unrecognized instruction: 0xCB 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';
    }
}
//##############################################################################
void sharpsm83::execute_nop()
{
    ///???
}
//##############################################################################
void sharpsm83::execute_halt()
{
    // ???
   is_halted = true;
}
//##############################################################################
void sharpsm83::execute_stop()
{
    // ???
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::rlc(reg8& reg)
{
    bool msb = reg.b7;

    set_zero_flag(false); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag

    reg.b0_7 <<= 1;
    reg.b0 = msb;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::rl(reg8& reg)
{
    bool msb = reg.b7;
    bool old_carry = get_carry_flag();

    set_zero_flag(false); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag

    reg.b0_7 <<= 1;
    reg.b0 = old_carry;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::rrc(reg8& reg)
{
    bool lsb = reg.b0;

    set_zero_flag(false); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    reg.b0_7 >>= 1;
    reg.b7 = lsb;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::rr(reg8& reg)
{
    bool lsb = reg.b0;
    bool old_carry = get_carry_flag();

    set_zero_flag(false); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    reg.b0_7 >>= 1;
    reg.b7 = old_carry;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::jr(bool cond)
{
    fetch_data(PC.b0_15 + 1);
    int16_t offset = fetched_data;

    uint16_t old_pc =  PC.b0_15;

    uint16_t new_pc = (uint16_t)(old_pc + offset);

    emulate_cycles(3);
    
    PC.b0_15 = cond ? new_pc : PC.b0_15 + 2;
}
//##############################################################################
void sharpsm83::jp(const uint16_t& address)
{
    emulate_cycles(1);
    
    PC.b0_15 = address;
}
//##############################################################################
void sharpsm83::jp(bool cond)
{
    fetch_data(PC.b0_15);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 1);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    emulate_cycles(3);
    
    PC.b0_15 = cond ? address : PC.b0_15 + 3;
}
//##############################################################################
void sharpsm83::call(bool cond)
{
    fetch_data(PC.b0_15);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 1);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    if(cond)
    {
        emulate_cycles(6);

        stack_push(PC);
        
        PC.b0_15 = address;
    }
    else
    {
        emulate_cycles(3);

        PC.b0_15 += 3;
    }
}
//##############################################################################
void sharpsm83::call(const uint16_t& address)
{
    stack_push(PC);

    emulate_cycles(4);

    PC.b0_15 = address;
}
//##############################################################################
void sharpsm83::da(reg8& reg)
{ 
    uint8_t adjustment = 0;

    if(get_subtraction_flag())
    {
        if(getHalfCarryFlag()) adjustment += 0x06;
        if(get_carry_flag()) adjustment += 0x60;

        reg.b0_7 -= adjustment;
    }
    else
    {
        if(getHalfCarryFlag() || (reg.b0_7 & 0x0F) > 0x09) adjustment += 0x06;
        if(get_carry_flag() || reg.b0_7 > 0x99)
        {
            adjustment += 0x60;
            set_carry_flag(true);
        }
        reg.b0_7 += adjustment;
    }

    set_half_carry_flag(false);
    set_zero_flag(reg.b0_7 == 0);

    emulate_cycles(1);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::complement(reg8& reg)
{
    reg.b0_7 = ~reg.b0_7;

    emulate_cycles(1);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld_to_address(const uint16_t& address, const uint8_t& data)
{
    write_data(address, data);

    emulate_cycles(2);
    
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld_to_address(const reg16& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t data = fetched_data;

    write_data(reg.b0_15, data);

    emulate_cycles(3);
    
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ld_to_address(const reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 2);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    write_data(address, reg.b0_7);

    emulate_cycles(4);
    
    PC.b0_15 += 3;
}
//##############################################################################
void sharpsm83::ldh_to_address(const reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t imm8 = fetched_data;
    uint16_t address = imm8 + 0xFF0;

    write_data(address, reg.b0_7);

    emulate_cycles(3);
    
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ldh_to_address(const reg8& to, const reg8& reg)
{
    uint16_t address = to.b0_7 + 0xFF0;

    write_data(address, reg.b0_7);

    emulate_cycles(2);
    
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld_from_address(uint8_t& reg, const uint16_t& address)
{
    fetch_data(address);
    emulate_cycles(1);

    reg = fetched_data;
    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld(uint16_t& to)
{
    fetch_data(PC.b0_15 + 1);
    uint16_t lo = fetched_data;

    fetch_data(PC.b0_15 + 2);
    uint16_t hi = fetched_data;

    to = (hi << 8) | lo;

    emulate_cycles(3);

    // increase PC
    PC.b0_15 += 3;
}
//##############################################################################
void sharpsm83::ld(uint8_t& to)
{
    fetch_data(PC.b0_15 + 1);
    to = fetched_data;

    emulate_cycles(2);

    // increase PC
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ld(uint8_t& to, const uint8_t& from)
{
    emulate_cycles(1);

    to = from;

    // increase PC
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld_sp()
{
    fetch_data(PC.b0_15 + 1);
    uint16_t lo = fetched_data;
    emulate_cycles(1);

    fetch_data(PC.b0_15 + 2);
    uint16_t hi = fetched_data;
    emulate_cycles(1);

    uint16_t address = (hi << 8) | lo;
    emulate_cycles(1);

    write_data(address, SP.b0_15);

    emulate_cycles(2);

    PC.b0_15 += 3;
}
//##############################################################################
void sharpsm83::inc(uint16_t& reg)
{
    reg += 1;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::inc(uint8_t& reg)
{
    uint8_t result = reg + 1;

    set_zero_flag(result == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag((result & 0x0F) == 0x00); // H flag

    reg = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::inc_mem(uint16_t& address)
{
    fetch_data(address);
    uint8_t result = fetched_data + 1;

    set_zero_flag(result == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag((result & 0x0F) == 0x00); // H flag

    write_data(address, result);

    emulate_cycles(3);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::dec(uint8_t& reg)
{
    uint8_t result = reg - 1;

    set_zero_flag(result == 0); // Z flag
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((result & 0x0F) == 0x0F); // H flag

    emulate_cycles(1);

    reg = result;

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::dec(uint16_t& reg)
{
    reg -= 1;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::dec_mem(uint16_t& address)
{
    fetch_data(address);
    uint8_t result = fetched_data - 1;

    set_zero_flag(result == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag((result & 0x0F) == 0x0F); // H flag

    write_data(address, result);

    emulate_cycles(3);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add(uint16_t& op1, uint16_t& op2)
{
    uint32_t result = op1 + op2;

    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > UINT8_MAX); // H flag ???
    set_carry_flag(result > UINT16_MAX); // C flag

    op1 = result & 0xFFFF;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add(uint8_t& op1, uint8_t& op2)
{   
    uint16_t result = op1 + op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    op1 = result & 0xFF;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);
    uint16_t result = op1 + fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    op1 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    uint16_t result = reg.b0_7 + fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    reg.b0_7 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::add_e8(reg16& reg)
{
    fetch_data(PC.b0_15 + 1);
    int8_t e8 = fetched_data;

    uint32_t result = reg.b0_15 + e8;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT16_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT16_MAX); // C flag

    reg.b0_15 = result & 0xFFFF;

    emulate_cycles(4);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::adc(uint8_t& op1, uint8_t& op2)
{
    uint16_t result = op1 + op2 + get_carry_flag();

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    op1 = result & 0xFF;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::adc_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);
    uint16_t result = op1 + fetched_data + get_carry_flag();

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    op1 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::adc(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint16_t result = reg.b0_7 + fetched_data + get_carry_flag();

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(result > (UINT8_MAX >> 2)); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    reg.b0_7 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::sub(uint8_t& op1, uint8_t& op2)
{
    uint8_t result = op1 - op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((op2 & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag(op2 > op1); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::sub_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);
    uint8_t result = op1 - fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((fetched_data & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag(fetched_data > op1); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::sub(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    uint16_t result = reg.b0_7 - fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((fetched_data & 0xF) > (reg.b0_7 & 0xF)); // H flag ???
    set_carry_flag(fetched_data > reg.b0_7); // C flag

    reg.b0_7 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 2;

}
//##############################################################################
void sharpsm83::sbc(uint8_t& op1, uint8_t& op2)
{
    bool old_carry = get_carry_flag();
    uint8_t result = op1 - op2 - old_carry;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag(((op2 + old_carry) & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag((op2 + old_carry) > op1); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
void sharpsm83::sbc_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);

    bool old_carry = get_carry_flag();
    uint8_t result = op1 - fetched_data - old_carry;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag(((fetched_data + old_carry) & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag((fetched_data + old_carry) > op1); // C flag

    op1 = result;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::sbc(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    bool old_carry = get_carry_flag();
    uint8_t result = reg.b0_7 - fetched_data - old_carry;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag(((fetched_data + old_carry) & 0xF) > (reg.b0_7 & 0xF)); // H flag ???
    set_carry_flag((fetched_data + old_carry) > reg.b0_7); // C flag

    reg.b0_7 = result;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::and_op(uint8_t& op1, uint8_t& op2)
{
    uint8_t result = op1 & op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(true); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::and_op_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);

    uint8_t result = op1 & fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(true); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::and_op(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t result = reg.b0_7 & fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(true); // H flag ???
    set_carry_flag(false); // C flag

    reg.b0_7 = result;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::xor_op(uint8_t& op1, uint8_t& op2)
{
    uint8_t result = op1 ^ op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::xor_op_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);

    uint8_t result = op1 ^ fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::xor_op(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t result = reg.b0_7 ^ fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    reg.b0_7  = result;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::or_op(uint8_t& op1, uint8_t& op2)
{
    uint8_t result = op1 | op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::or_op_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);

    uint8_t result = op1 | fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    op1 = result;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::or_op(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t result = reg.b0_7 | fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag ???
    set_carry_flag(false); // C flag

    reg.b0_7 = result;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::cp_op(uint8_t& op1, uint8_t& op2)
{
    uint8_t result = op1 - op2;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((op2 & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag(op2 > op1); // C flag

    emulate_cycles(1);

    PC.b0_15 += 1;
}
void sharpsm83::cp_op_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);

    uint8_t result = op1 - fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((fetched_data & 0xF) > (op1 & 0xF)); // H flag ???
    set_carry_flag(fetched_data > op1); // C flag

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::stack_pop(reg16& reg)
{
    fetch_data(SP.b0_15);
    uint8_t low = fetched_data;
    SP.b0_15 += 1;

    fetch_data(SP.b0_15);
    uint8_t high = fetched_data;
    SP.b0_15 += 1;

    reg.b0_15 = (high << 0x8) | low;
}
//##############################################################################
void sharpsm83::pop(reg16& reg)
{
    stack_pop(reg); 

    emulate_cycles(3);

    PC.b0_15 += 1;
}

//##############################################################################
void sharpsm83::push(reg16& reg)
{
    stack_push(reg);

    emulate_cycles(3);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::stack_push(reg16& reg)
{
    write_data(SP.b0_15, reg.Hi.b0_7);
    SP.b0_15 -= 1;

    write_data(SP.b0_15, reg.Lo.b0_7);
    SP.b0_15 -= 1;
}
//##############################################################################
void sharpsm83::ret_condition(bool condition)
{
    if(condition) 
    {
        stack_pop(PC);   
        
        emulate_cycles(5);
    }
    else
    {
        emulate_cycles(2);

        PC.b0_15 += 1;
    }
}
//##############################################################################
void sharpsm83::reti_op()
{
    interrupts_enabled = false;

    ret_condition(true);
}
//##############################################################################
void sharpsm83::rlc_param(reg8& reg)
{
    bool msb = reg.b7;
    reg.b0_7 <<= 1;
    reg.b0 = msb;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::rlcmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool msb = result.b7;
    result.b0_7 <<= 1;
    result.b0 = msb;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    setHalfCarryFlag(false); // H flag
    setCarryFlag(msb); // C flag

    emulate_cycles(4);

    PC.b0_15 += 2;
}

//##############################################################################
void sharpsm83::nop() { execute_nop(); } // 0x00
void sharpsm83::ld_bc_imm16() { ld(BC.b0_15); } // 0x01
void sharpsm83::ld_membc_a() { ld_to_address(BC.b0_15, AF.Hi.b0_7); } // 0x02
void sharpsm83::inc_bc() { inc(BC.b0_15); } // 0x03
void sharpsm83::inc_b() { inc(BC.Hi.b0_7); } // 0x04
void sharpsm83::dec_b() { dec(BC.Hi.b0_7); } // 0x05
void sharpsm83::ld_b_imm8() { ld(BC.Hi.b0_7); } // 0x06
void sharpsm83::rlca() { rlc(AF.Hi); } // 0x07
void sharpsm83::ld_memimm16_sp() { ld_sp();}  // 0x08
void sharpsm83::add_hl_bc() { add(HL.b0_15, BC.b0_15); } // 0x09
void sharpsm83::ld_a_membc() { ld_from_address(AF.Hi.b0_7, BC.b0_15); } // 0x0A
void sharpsm83::dec_bc() { dec(BC.b0_15); } // 0x0B
void sharpsm83::inc_c() { inc(BC.Lo.b0_7); } // 0x0C
void sharpsm83::dec_c() { dec(BC.Lo.b0_7); } // 0x0D
void sharpsm83::ld_c_imm8() { ld(BC.Lo.b0_7); } // 0x0E
void sharpsm83::rrca() { rrc(AF.Hi); } // 0x0F
//##############################################################################
void sharpsm83::stop_imm8() { execute_stop(); }// 0x10
void sharpsm83::ld_de_imm16() {ld(DE.b0_15);} // 0x11
void sharpsm83::ld_memde_a() { ld_to_address(DE.b0_15, AF.Hi.b0_7); } // 0x12
void sharpsm83::inc_de(){ inc(DE.b0_15); }; // 0x13
void sharpsm83::inc_d(){ inc(DE.Hi.b0_7); }; // 0x14
void sharpsm83::dec_d(){ dec(DE.Hi.b0_7); }; // 0x15
void sharpsm83::ld_d_imm8() { ld(DE.Hi.b0_7); } // 0x16
void sharpsm83::rla() { rl(AF.Hi); } // 0x17
void sharpsm83::jr_e8() { jr(true); } // 0x18
void sharpsm83::add_hl_de() { add(HL.b0_15, DE.b0_15); } // 0x19
void sharpsm83::ld_a_memde() { ld_from_address(AF.Hi.b0_7, DE.b0_15); } // 0x1A
void sharpsm83::dec_de() { dec(DE.b0_15); } // 0x1B
void sharpsm83::inc_e() { inc(DE.Lo.b0_7); } // 0x1C
void sharpsm83::dec_e() { dec(DE.Lo.b0_7); } // 0x1D
void sharpsm83::ld_e_immm8() { ld(DE.Lo.b0_7); } // 0x1E
void sharpsm83::rra() { rr(AF.Hi); } // 0x1F
//##############################################################################
void sharpsm83::jr_nz_e8() { jr(!get_zero_flag()); }; // 0x20
void sharpsm83::ld_hl_imm16() { ld(HL.b0_15); } // 0x21
void sharpsm83::ld_memhlinc_a() { ld_to_address(HL.b0_15++, AF.Hi.b0_7); } // 0x22
void sharpsm83::inc_hl() { inc(HL.b0_15); } // 0x23
void sharpsm83::inc_h() { inc(HL.Hi.b0_7); } // 0x24
void sharpsm83::dec_h() { dec(HL.Hi.b0_7); } // 0x25
void sharpsm83::ld_h_imm8() { ld(HL.Hi.b0_7); } // 0x26
void sharpsm83::daa() { da(AF.Hi); } // 0x27
void sharpsm83::jr_z_e8() { jr(get_zero_flag()); } // 0x28
void sharpsm83::add_hl_hl() { add(HL.b0_15, HL.b0_15); } // 0x29
void sharpsm83::ld_a_memhlinc() { ld_from_address(AF.Hi.b0_7, HL.b0_15++); }  // 0x2A
void sharpsm83::dec_hl() { dec(HL.b0_15); } // 0x2B
void sharpsm83::inc_l() { inc(HL.Lo.b0_7);  } // 0x2C
void sharpsm83::dec_l() { dec(HL.Lo.b0_7);  } // 0x2D
void sharpsm83::ld_l_imm8() { ld(HL.Lo.b0_7);  } // 0x2E
void sharpsm83::cpl() { complement(AF.Hi); } // 0x2F
//##############################################################################
void sharpsm83::jr_nc_e8() { jr(!get_carry_flag()); } // 0x30
void sharpsm83::ld_sp_imm16() { ld(SP.b0_15); } // 0x31
void sharpsm83::ld_memhldec_a() { ld_to_address(HL.b0_15--, AF.Hi.b0_7); } // 0x32
void sharpsm83::inc_sp() { inc(SP.b0_15); } // 0x33
void sharpsm83::inc_memhl() { inc_mem(HL.b0_15); } // 0x34
void sharpsm83::dec_memhl() { dec_mem(HL.b0_15); } // 0x35
void sharpsm83::ld_memhl_imm8() { ld_to_address(HL); }  // 0x36
void sharpsm83::scf() { set_carry_flag(); } // 0x37
void sharpsm83::jr_c_e8() { jr(get_carry_flag()); } // 0x38
void sharpsm83::add_hl_sp() { add(HL.b0_15, SP.b0_15); } // 0x39
void sharpsm83::ld_a_memhldec() { ld_from_address(AF.Hi.b0_7, HL.b0_15--); } // 0x3A
void sharpsm83::dec_sp() { dec(SP.b0_15); } // 0x3B
void sharpsm83::inc_a() { inc(AF.Hi.b0_7); } // 0x3C
void sharpsm83::dec_a() { dec(AF.Hi.b0_7); } // 0x3D
void sharpsm83::ld_a_imm8() { ld(AF.Hi.b0_7); } // 0x3E
void sharpsm83::ccf(){ complement_carry_flag(); }// 0x3F
//##############################################################################
void sharpsm83::ld_b_b() { ld(BC.Hi.b0_7, BC.Hi.b0_7); } // 0x40
void sharpsm83::ld_b_c() { ld(BC.Hi.b0_7, BC.Lo.b0_7); } // 0x41
void sharpsm83::ld_b_d() { ld(BC.Hi.b0_7, DE.Hi.b0_7); } // 0x42
void sharpsm83::ld_b_e() { ld(BC.Hi.b0_7, DE.Lo.b0_7); } // 0x43
void sharpsm83::ld_b_h() { ld(BC.Hi.b0_7, HL.Hi.b0_7); } // 0x44
void sharpsm83::ld_b_l() { ld(BC.Hi.b0_7, HL.Lo.b0_7); } // 0x45
void sharpsm83::ld_b_memhl( ) { ld_from_address(BC.Hi.b0_7, HL.b0_15); } // 0x46
void sharpsm83::ld_b_a() { ld(BC.Hi.b0_7, AF.Hi.b0_7); } // 0x47
void sharpsm83::ld_c_b() { ld(BC.Lo.b0_7, BC.Hi.b0_7); } // 0x48
void sharpsm83::ld_c_c() { ld(BC.Lo.b0_7, BC.Lo.b0_7); } // 0x49
void sharpsm83::ld_c_d() { ld(BC.Lo.b0_7, DE.Hi.b0_7); } // 0x4A
void sharpsm83::ld_c_e() { ld(BC.Lo.b0_7, DE.Lo.b0_7); } // 0x4B
void sharpsm83::ld_c_h() { ld(BC.Lo.b0_7, HL.Hi.b0_7); } // 0x4C
void sharpsm83::ld_c_l() { ld(BC.Lo.b0_7, HL.Lo.b0_7); } // 0x4D
void sharpsm83::ld_c_memhl(){ ld_from_address(BC.Lo.b0_7, HL.b0_15); } // 0x4E
void sharpsm83::ld_c_a() { ld(BC.Lo.b0_7, AF.Hi.b0_7); } // 0x4F
//##############################################################################
void sharpsm83::ld_d_b() { ld(DE.Hi.b0_7, BC.Hi.b0_7); } // 0x50
void sharpsm83::ld_d_c() { ld(DE.Hi.b0_7, BC.Lo.b0_7); } // 0x51
void sharpsm83::ld_d_d() { ld(DE.Hi.b0_7, DE.Hi.b0_7); } // 0x52
void sharpsm83::ld_d_e() { ld(DE.Hi.b0_7, DE.Lo.b0_7); } // 0x53
void sharpsm83::ld_d_h() { ld(DE.Hi.b0_7, HL.Hi.b0_7); } // 0x54
void sharpsm83::ld_d_l() { ld(DE.Hi.b0_7, HL.Lo.b0_7); } // 0x55
void sharpsm83::ld_d_memhl() { ld_from_address(DE.Hi.b0_7, HL.b0_15); } // 0x56
void sharpsm83::ld_d_a() { ld(DE.Hi.b0_7, AF.Hi.b0_7); } // 0x57
void sharpsm83::ld_e_b() { ld(DE.Lo.b0_7, BC.Hi.b0_7); } // 0x58
void sharpsm83::ld_e_c() { ld(DE.Lo.b0_7, BC.Lo.b0_7); } // 0x59
void sharpsm83::ld_e_d() { ld(DE.Lo.b0_7, DE.Hi.b0_7); } // 0x5A
void sharpsm83::ld_e_e() { ld(DE.Lo.b0_7, DE.Lo.b0_7); } // 0x5B
void sharpsm83::ld_e_h() { ld(DE.Lo.b0_7, HL.Hi.b0_7); } // 0x5C
void sharpsm83::ld_e_l() { ld(DE.Lo.b0_7, HL.Lo.b0_7); } // 0x5D
void sharpsm83::ld_e_memhl() { ld_from_address(DE.Lo.b0_7, HL.b0_15); } // 0x5E
void sharpsm83::ld_e_a() { ld(DE.Lo.b0_7, AF.Hi.b0_7); } // 0x5F
//##############################################################################
void sharpsm83::ld_h_b() { ld(HL.Hi.b0_7, BC.Hi.b0_7); } // 0x60
void sharpsm83::ld_h_c() { ld(HL.Hi.b0_7, BC.Lo.b0_7); } // 0x61
void sharpsm83::ld_h_d() { ld(HL.Hi.b0_7, DE.Hi.b0_7); } // 0x62
void sharpsm83::ld_h_e() { ld(HL.Hi.b0_7, DE.Lo.b0_7); } // 0x63
void sharpsm83::ld_h_h() { ld(HL.Hi.b0_7, HL.Hi.b0_7); } // 0x64
void sharpsm83::ld_h_l() { ld(HL.Hi.b0_7, HL.Lo.b0_7); } // 0x65
void sharpsm83::ld_h_memhl() { ld_from_address(HL.Hi.b0_7, HL.b0_15); } // 0x66
void sharpsm83::ld_h_a() { ld(HL.Hi.b0_7, AF.Hi.b0_7); } // 0x67
void sharpsm83::ld_l_b() { ld(HL.Lo.b0_7, BC.Hi.b0_7); } // 0x68
void sharpsm83::ld_l_c() { ld(HL.Lo.b0_7, BC.Lo.b0_7); } // 0x69
void sharpsm83::ld_l_d() { ld(HL.Lo.b0_7, DE.Hi.b0_7); } // 0x6A
void sharpsm83::ld_l_e() { ld(HL.Lo.b0_7, DE.Lo.b0_7); } // 0x6B
void sharpsm83::ld_l_h() { ld(HL.Lo.b0_7, HL.Hi.b0_7); } // 0x6C
void sharpsm83::ld_l_l() { ld(HL.Lo.b0_7, HL.Lo.b0_7); } // 0x6D
void sharpsm83::ld_l_memhl() { ld_from_address(HL.Lo.b0_7, HL.b0_15); } // 0x6E
void sharpsm83::ld_l_a() { ld(HL.Lo.b0_7, AF.Hi.b0_7); } // 0x6F
//##############################################################################
void sharpsm83::ld_memhl_b() { ld_to_address(HL.b0_15, BC.Hi.b0_7); } // 0x70
void sharpsm83::ld_memhl_c() { ld_to_address(HL.b0_15, BC.Lo.b0_7); } // 0x71
void sharpsm83::ld_memhl_d() { ld_to_address(HL.b0_15, DE.Hi.b0_7); } // 0x72
void sharpsm83::ld_memhl_e() { ld_to_address(HL.b0_15, DE.Lo.b0_7); } // 0x73
void sharpsm83::ld_memhl_h() { ld_to_address(HL.b0_15, HL.Hi.b0_7); } // 0x74
void sharpsm83::ld_memhl_l() { ld_to_address(HL.b0_15, HL.Lo.b0_7); } // 0x75
void sharpsm83::halt() { execute_halt(); } // 0x76
void sharpsm83::ld_memhl_a(){ ld_to_address(HL.b0_15, AF.Hi.b0_7); } // 0x77
void sharpsm83::ld_a_b() { ld(AF.Hi.b0_7, BC.Hi.b0_7); } // 0x78
void sharpsm83::ld_a_c() { ld(AF.Hi.b0_7, BC.Lo.b0_7); } // 0x79
void sharpsm83::ld_a_d() { ld(AF.Hi.b0_7, DE.Hi.b0_7); } // 0x7A
void sharpsm83::ld_a_e() { ld(AF.Hi.b0_7, DE.Lo.b0_7); } // 0x7B
void sharpsm83::ld_a_h() { ld(AF.Hi.b0_7, HL.Hi.b0_7); } // 0x7C
void sharpsm83::ld_a_l() { ld(AF.Hi.b0_7, HL.Lo.b0_7); } // 0x7D
void sharpsm83::ld_a_memhl() { ld_from_address(AF.Hi.b0_7, HL.b0_15); } // 0x7E
void sharpsm83::ld_a_a() { ld(AF.Hi.b0_7, AF.Hi.b0_7); } // 0x7F
//##############################################################################
void sharpsm83::add_a_b() { add(AF.Hi.b0_7, BC.Hi.b0_7); } //0x80
void sharpsm83::add_a_c() { add(AF.Hi.b0_7, BC.Lo.b0_7); } //0x81
void sharpsm83::add_a_d() { add(AF.Hi.b0_7, DE.Hi.b0_7); } //0x82
void sharpsm83::add_a_e() { add(AF.Hi.b0_7, DE.Lo.b0_7); } //0x83
void sharpsm83::add_a_h() { add(AF.Hi.b0_7, HL.Hi.b0_7); } //0x84
void sharpsm83::add_a_l() { add(AF.Hi.b0_7, HL.Lo.b0_7); } //0x85
void sharpsm83::add_a_memhl() { add_from_address(AF.Hi.b0_7, HL.b0_15); } //0x86
void sharpsm83::add_a_a() { add(AF.Hi.b0_7, AF.Hi.b0_7); } //0x87
void sharpsm83::adc_a_b() { adc(AF.Hi.b0_7, BC.Hi.b0_7); } //0x88
void sharpsm83::adc_a_c() { adc(AF.Hi.b0_7, BC.Lo.b0_7); } //0x89
void sharpsm83::adc_a_d() { adc(AF.Hi.b0_7, DE.Hi.b0_7); } //0x8A
void sharpsm83::adc_a_e() { adc(AF.Hi.b0_7, DE.Lo.b0_7); } //0x8B
void sharpsm83::adc_a_h() { adc(AF.Hi.b0_7, HL.Hi.b0_7); } //0x8C
void sharpsm83::adc_a_l() { adc(AF.Hi.b0_7, HL.Lo.b0_7); } //0x8D
void sharpsm83::adc_a_memhl() { adc_from_address(AF.Hi.b0_7, HL.b0_15); } //0x8E
void sharpsm83::adc_a_a() { adc(AF.Hi.b0_7, AF.Hi.b0_7); } //0x8F
//##############################################################################
void sharpsm83::sub_a_b() { sub(AF.Hi.b0_7, BC.Hi.b0_7); } //0x90
void sharpsm83::sub_a_c() { sub(AF.Hi.b0_7, BC.Lo.b0_7); } //0x91
void sharpsm83::sub_a_d() { sub(AF.Hi.b0_7, DE.Hi.b0_7); } //0x92
void sharpsm83::sub_a_e() { sub(AF.Hi.b0_7, DE.Lo.b0_7); } //0x93
void sharpsm83::sub_a_h() { sub(AF.Hi.b0_7, HL.Hi.b0_7); } //0x94
void sharpsm83::sub_a_l() { sub(AF.Hi.b0_7, HL.Lo.b0_7); } //0x95
void sharpsm83::sub_a_memhl(){ sub_from_address(AF.Hi.b0_7, HL.b0_15); } //0x96
void sharpsm83::sub_a_a() { sub(AF.Hi.b0_7, AF.Hi.b0_7); } //0x97
void sharpsm83::sbc_a_b() { sbc(AF.Hi.b0_7, BC.Hi.b0_7); } //0x98
void sharpsm83::sbc_a_c() { sbc(AF.Hi.b0_7, BC.Lo.b0_7); } //0x99
void sharpsm83::sbc_a_d() { sbc(AF.Hi.b0_7, DE.Hi.b0_7); } //0x9A
void sharpsm83::sbc_a_e() { sbc(AF.Hi.b0_7, DE.Lo.b0_7); } //0x9B
void sharpsm83::sbc_a_h() { sbc(AF.Hi.b0_7, HL.Hi.b0_7); } //0x9C
void sharpsm83::sbc_a_l() { sbc(AF.Hi.b0_7, HL.Lo.b0_7); } //0x9D
void sharpsm83::sbc_a_memhl() { sbc_from_address(AF.Hi.b0_7, HL.b0_15); } //0x9E
void sharpsm83::sbc_a_a() { sbc(AF.Hi.b0_7, AF.Hi.b0_7); } //0x9F
//##############################################################################
void sharpsm83::and_a_b() { and_op(AF.Hi.b0_7, BC.Hi.b0_7); } //0xA0
void sharpsm83::and_a_c() { and_op(AF.Hi.b0_7, BC.Lo.b0_7); } //0xA1
void sharpsm83::and_a_d() { and_op(AF.Hi.b0_7, DE.Hi.b0_7); } //0xA2
void sharpsm83::and_a_e() { and_op(AF.Hi.b0_7, DE.Lo.b0_7); } //0xA3
void sharpsm83::and_a_h() { and_op(AF.Hi.b0_7, HL.Hi.b0_7); } //0xA4
void sharpsm83::and_a_l() { and_op(AF.Hi.b0_7, HL.Lo.b0_7); } //0xA5
void sharpsm83::and_a_memhl() { and_op_from_address(AF.Hi.b0_7, HL.b0_15); } //0xA6
void sharpsm83::and_a_a() { and_op(AF.Hi.b0_7, AF.Hi.b0_7); } //0xA7
void sharpsm83::xor_a_b() { xor_op(AF.Hi.b0_7, BC.Hi.b0_7); } //0xA8
void sharpsm83::xor_a_c() { xor_op(AF.Hi.b0_7, BC.Lo.b0_7); } //0xA9
void sharpsm83::xor_a_d() { xor_op(AF.Hi.b0_7, DE.Hi.b0_7); } //0xAA
void sharpsm83::xor_a_e() { xor_op(AF.Hi.b0_7, DE.Lo.b0_7); } //0xAB
void sharpsm83::xor_a_h() { xor_op(AF.Hi.b0_7, HL.Hi.b0_7); } //0xAC
void sharpsm83::xor_a_l() { xor_op(AF.Hi.b0_7, HL.Lo.b0_7); } //0xAD
void sharpsm83::xor_a_memhl() { xor_op_from_address(AF.Hi.b0_7, HL.b0_15); } //0xAE
void sharpsm83::xor_a_a() { xor_op(AF.Hi.b0_7, AF.Hi.b0_7); } //0xAF
//##############################################################################
void sharpsm83::or_a_b() { or_op(AF.Hi.b0_7, BC.Hi.b0_7); } //0xB0
void sharpsm83::or_a_c() { or_op(AF.Hi.b0_7, BC.Lo.b0_7); } //0xB1
void sharpsm83::or_a_d() { or_op(AF.Hi.b0_7, DE.Hi.b0_7); } //0xB2
void sharpsm83::or_a_e() { or_op(AF.Hi.b0_7, DE.Lo.b0_7); } //0xB3
void sharpsm83::or_a_h() { or_op(AF.Hi.b0_7, HL.Hi.b0_7); } //0xB4
void sharpsm83::or_a_l() { or_op(AF.Hi.b0_7, HL.Lo.b0_7); } //0xB5
void sharpsm83::or_a_memhl() { or_op_from_address(AF.Hi.b0_7, HL.b0_15); } //0xB6
void sharpsm83::or_a_a() { or_op(AF.Hi.b0_7, AF.Hi.b0_7); } //0xB7
void sharpsm83::cp_a_b() { cp_op(AF.Hi.b0_7, BC.Hi.b0_7); } //0xB8
void sharpsm83::cp_a_c() { cp_op(AF.Hi.b0_7, BC.Lo.b0_7); } //0xB9
void sharpsm83::cp_a_d() { cp_op(AF.Hi.b0_7, DE.Hi.b0_7); } //0xBA
void sharpsm83::cp_a_e() { cp_op(AF.Hi.b0_7, DE.Lo.b0_7); } //0xBB
void sharpsm83::cp_a_h() { cp_op(AF.Hi.b0_7, HL.Hi.b0_7); } //0xBC
void sharpsm83::cp_a_l() { cp_op(AF.Hi.b0_7, HL.Lo.b0_7); } //0xBD
void sharpsm83::cp_a_memhl() { cp_op_from_address(AF.Hi.b0_7, HL.b0_15); } //0xBE
void sharpsm83::cp_a_a()  { cp_op(AF.Hi.b0_7, AF.Hi.b0_7); } //0xBF
//##############################################################################
void sharpsm83::ret_nz() { ret_condition(!get_subtraction_flag()); } // 0xC0
void sharpsm83::pop_bc() { pop(BC); } // 0xC1
void sharpsm83::jp_nz_imm16() { jp(!get_zero_flag()); } // 0xC2
void sharpsm83::jp_imm16() { jp(true); } // 0xC3
void sharpsm83::call_nz_imm16() { call(!get_zero_flag()); } // 0xC4
void sharpsm83::push_bc() { push(BC); } // 0xC5
void sharpsm83::add_a_imm8() { add(AF.Hi); } // 0xC6
void sharpsm83::rst_0x00() { call( (uint16_t)0x0000 ); }// 0xC7
void sharpsm83::ret_z() { ret_condition(get_zero_flag());  } //0xC8
void sharpsm83::ret() { ret_condition(true); } //0xC9
void sharpsm83::jp_z_imm16() { jp(get_zero_flag()); }  //0xCA
void sharpsm83::prefix() { }  // 0xCB
void sharpsm83::call_z_imm16() { call(get_zero_flag()); } // 0xCC
void sharpsm83::call_imm16() { call(get_zero_flag()); } // 0xCD
void sharpsm83::adc_a_imm8() { adc(AF.Hi); } // 0xCE
void sharpsm83::rst_0x08() { call( (uint16_t)0x0008 ); } // 0xCF
//##############################################################################
void sharpsm83::ret_nc() { ret_condition(!get_carry_flag()); } // 0xD0
void sharpsm83::pop_de() { pop(DE); } // 0xD1
void sharpsm83::jp_nc_imm16() { jp(!get_carry_flag()); } // 0xD2
void sharpsm83::op_0xD3() {  } // 0xD3
void sharpsm83::call_nc_imm16() { call(!get_carry_flag()); } // 0xD4
void sharpsm83::push_de() { push(DE); } // 0xD5
void sharpsm83::sub_a_imm8() { sub(AF.Hi); } // 0xD6
void sharpsm83::rst_0x10() {  call( (uint16_t)0x0010 ); } // 0xD7
void sharpsm83::ret_c() { ret_condition(get_carry_flag()); } // 0xD8
void sharpsm83::reti() { reti_op(); } // 0xD9
void sharpsm83::jp_c_imm16() { jp(get_carry_flag()); } // 0xDA
void sharpsm83::op_0xDB() { } // 0xDB
void sharpsm83::call_c_a16() { call(get_carry_flag()); } // 0xDC
void sharpsm83::op_0xDD() { } // 0xDD
void sharpsm83::sbc_a_imm8() { sbc(AF.Hi); }// 0xDD
void sharpsm83::rst_0x18() { call( (uint16_t)0x0018 ); } // 0xDF
//##############################################################################
void sharpsm83::ldh_memimm8_a() { ldh_to_address(AF.Hi); } // 0xE0
void sharpsm83::pop_hl() { pop(HL); } // 0xE1
void sharpsm83::ldh_memc_a() { ldh_to_address(BC.Lo, AF.Hi); } // 0xE2
void sharpsm83::op_0xE3() {  } // 0xE3
void sharpsm83::op_0xE4() {  } // 0xE4
void sharpsm83::push_hl() { push(HL); } // 0xE5
void sharpsm83::and_a_imm8() { and_op(AF.Hi); } // 0xE6
void sharpsm83::rst_0x20() { call( (uint16_t)0x0020 ); } // 0xE7
void sharpsm83::add_sp_e8() { add_e8(SP); } // 0xE8
void sharpsm83::jp_hl() { jp(HL.b0_15); } // 0xE9
void sharpsm83::ld_memimm16_a() { ld_to_address(AF.Hi); } // 0xEA
void sharpsm83::op_0xEB() { } // 0xEB
void sharpsm83::op_0xEC() { } // 0xEC
void sharpsm83::op_0xED() { } // 0xED
void sharpsm83::xor_a_imm8() { xor_op(AF.Hi); } // 0xEE
void sharpsm83::rst_0x28() { call( (uint16_t)0x0028 ); }// 0xEF
//##############################################################################
// 0xCB instructions
//##############################################################################
void sharpsm83::rlc_b() { rlc_param(BC.Hi); } // Ox00
void sharpsm83::rlc_c() { rlc_param(BC.Lo); } // Ox01
void sharpsm83::rlc_d() { rlc_param(DE.Hi); } // Ox02
void sharpsm83::rlc_e() { rlc_param(DE.Lo); } // Ox03
void sharpsm83::rlc_h() { rlc_param(HL.Hi); } // Ox04
void sharpsm83::rlc_l() { rlc_param(HL.Lo); } // Ox05
void sharpsm83::rlc_memhl() {rlcmem_param(HL.b0_15);} // Ox06
void sharpsm83::rlc_a() { rlc_param(AF.Hi); } // Ox07
//##############################################################################
void sharpsm83::emulate_cycles(int cycles)
{

}
//##############################################################################
void sharpsm83::initialize_opcodes() 
{
    opcode_table[0x00] = std::bind(&sharpsm83::nop, this);
    opcode_table[0x01] = std::bind(&sharpsm83::ld_bc_imm16, this);
    opcode_table[0x02] = std::bind(&sharpsm83::ld_membc_a, this);
    opcode_table[0x03] = std::bind(&sharpsm83::inc_bc, this);
    opcode_table[0x04] = std::bind(&sharpsm83::inc_b, this);
    opcode_table[0x05] = std::bind(&sharpsm83::dec_b, this);
    opcode_table[0x06] = std::bind(&sharpsm83::ld_b_imm8, this);
    opcode_table[0x07] = std::bind(&sharpsm83::rlca, this);
    opcode_table[0x08] = std::bind(&sharpsm83::ld_memimm16_sp, this);
    opcode_table[0x09] = std::bind(&sharpsm83::add_hl_bc, this);
    opcode_table[0x0A] = std::bind(&sharpsm83::ld_a_membc, this);
    opcode_table[0x0B] = std::bind(&sharpsm83::dec_bc, this);
    opcode_table[0x0C] = std::bind(&sharpsm83::inc_c, this);
    opcode_table[0x0D] = std::bind(&sharpsm83::dec_c, this);
    opcode_table[0x0E] = std::bind(&sharpsm83::ld_c_imm8, this);
    opcode_table[0x0F] = std::bind(&sharpsm83::rrca, this);

    opcode_table[0x10] = std::bind(&sharpsm83::stop_imm8, this);
    opcode_table[0x11] = std::bind(&sharpsm83::ld_de_imm16, this);
    opcode_table[0x12] = std::bind(&sharpsm83::ld_memde_a, this);
    opcode_table[0x13] = std::bind(&sharpsm83::inc_de, this);
    opcode_table[0x14] = std::bind(&sharpsm83::inc_d, this);
    opcode_table[0x15] = std::bind(&sharpsm83::dec_d, this);
    opcode_table[0x16] = std::bind(&sharpsm83::ld_d_imm8, this);
    opcode_table[0x17] = std::bind(&sharpsm83::rla, this);
    opcode_table[0x18] = std::bind(&sharpsm83::jr_e8, this);
    opcode_table[0x19] = std::bind(&sharpsm83::add_hl_de, this);
    opcode_table[0x1A] = std::bind(&sharpsm83::ld_a_memde, this);
    opcode_table[0x1B] = std::bind(&sharpsm83::dec_de, this);
    opcode_table[0x1C] = std::bind(&sharpsm83::inc_e, this);
    opcode_table[0x1D] = std::bind(&sharpsm83::dec_e, this);
    opcode_table[0x1E] = std::bind(&sharpsm83::ld_e_immm8, this);
    opcode_table[0x1F] = std::bind(&sharpsm83::rra, this);
    
    opcode_table[0x20] = std::bind(&sharpsm83::jr_nz_e8, this);
    opcode_table[0x21] = std::bind(&sharpsm83::ld_hl_imm16, this);
    opcode_table[0x22] = std::bind(&sharpsm83::ld_memhlinc_a, this);
    opcode_table[0x23] = std::bind(&sharpsm83::inc_hl, this);
    opcode_table[0x24] = std::bind(&sharpsm83::inc_h, this);
    opcode_table[0x25] = std::bind(&sharpsm83::dec_h, this);
    opcode_table[0x26] = std::bind(&sharpsm83::ld_h_imm8, this);
    opcode_table[0x27] = std::bind(&sharpsm83::daa, this);
    opcode_table[0x28] = std::bind(&sharpsm83::jr_z_e8, this);
    opcode_table[0x29] = std::bind(&sharpsm83::add_hl_hl, this);
    opcode_table[0x2A] = std::bind(&sharpsm83::ld_a_memhlinc, this);
    opcode_table[0x2B] = std::bind(&sharpsm83::dec_hl, this);
    opcode_table[0x2C] = std::bind(&sharpsm83::inc_l, this);
    opcode_table[0x2D] = std::bind(&sharpsm83::dec_l, this);
    opcode_table[0x2E] = std::bind(&sharpsm83::ld_l_imm8, this);
    opcode_table[0x2F] = std::bind(&sharpsm83::cpl, this);

    opcode_table[0x30] = std::bind(&sharpsm83::jr_nc_e8, this);
    opcode_table[0x31] = std::bind(&sharpsm83::ld_sp_imm16, this);
    opcode_table[0x32] = std::bind(&sharpsm83::ld_memhldec_a, this);
    opcode_table[0x33] = std::bind(&sharpsm83::inc_sp, this);
    opcode_table[0x34] = std::bind(&sharpsm83::inc_memhl, this);
    opcode_table[0x35] = std::bind(&sharpsm83::dec_memhl, this);
    opcode_table[0x36] = std::bind(&sharpsm83::ld_memhl_imm8, this);
    opcode_table[0x37] = std::bind(&sharpsm83::scf, this);
    opcode_table[0x38] = std::bind(&sharpsm83::jr_c_e8, this);
    opcode_table[0x39] = std::bind(&sharpsm83::add_hl_sp, this);
    opcode_table[0x3A] = std::bind(&sharpsm83::ld_a_memhldec, this);
    opcode_table[0x3B] = std::bind(&sharpsm83::dec_sp, this);
    opcode_table[0x3C] = std::bind(&sharpsm83::inc_a, this);
    opcode_table[0x3D] = std::bind(&sharpsm83::dec_a, this);
    opcode_table[0x3E] = std::bind(&sharpsm83::ld_a_imm8, this);
    opcode_table[0x3F] = std::bind(&sharpsm83::ccf, this);

    opcode_table[0x40] = std::bind(&sharpsm83::ld_b_b, this);
    opcode_table[0x41] = std::bind(&sharpsm83::ld_b_c, this);
    opcode_table[0x42] = std::bind(&sharpsm83::ld_b_d, this);
    opcode_table[0x43] = std::bind(&sharpsm83::ld_b_e, this);
    opcode_table[0x44] = std::bind(&sharpsm83::ld_b_h, this);
    opcode_table[0x45] = std::bind(&sharpsm83::ld_b_l, this);
    opcode_table[0x46] = std::bind(&sharpsm83::ld_b_memhl, this);
    opcode_table[0x47] = std::bind(&sharpsm83::ld_b_a, this);
    opcode_table[0x48] = std::bind(&sharpsm83::ld_c_b, this);
    opcode_table[0x49] = std::bind(&sharpsm83::ld_c_c, this);
    opcode_table[0x4A] = std::bind(&sharpsm83::ld_c_d, this);
    opcode_table[0x4B] = std::bind(&sharpsm83::ld_c_e, this);
    opcode_table[0x4C] = std::bind(&sharpsm83::ld_c_h, this);
    opcode_table[0x4D] = std::bind(&sharpsm83::ld_c_l, this);
    opcode_table[0x4E] = std::bind(&sharpsm83::ld_c_memhl, this);
    opcode_table[0x4F] = std::bind(&sharpsm83::ld_c_a, this);

    opcode_table[0x50] = std::bind(&sharpsm83::ld_d_b, this);
    opcode_table[0x51] = std::bind(&sharpsm83::ld_d_c, this);
    opcode_table[0x52] = std::bind(&sharpsm83::ld_d_d, this);
    opcode_table[0x53] = std::bind(&sharpsm83::ld_d_e, this);
    opcode_table[0x54] = std::bind(&sharpsm83::ld_d_h, this);
    opcode_table[0x55] = std::bind(&sharpsm83::ld_d_l, this);
    opcode_table[0x56] = std::bind(&sharpsm83::ld_d_memhl, this);
    opcode_table[0x57] = std::bind(&sharpsm83::ld_d_a, this);
    opcode_table[0x58] = std::bind(&sharpsm83::ld_e_b, this);
    opcode_table[0x59] = std::bind(&sharpsm83::ld_e_c, this);
    opcode_table[0x5A] = std::bind(&sharpsm83::ld_e_d, this);
    opcode_table[0x5B] = std::bind(&sharpsm83::ld_e_e, this);
    opcode_table[0x5C] = std::bind(&sharpsm83::ld_e_h, this);
    opcode_table[0x5D] = std::bind(&sharpsm83::ld_e_l, this);
    opcode_table[0x5E] = std::bind(&sharpsm83::ld_e_memhl, this);
    opcode_table[0x5F] = std::bind(&sharpsm83::ld_e_a, this);

    opcode_table[0x60] = std::bind(&sharpsm83::ld_h_b, this);
    opcode_table[0x61] = std::bind(&sharpsm83::ld_h_c, this);
    opcode_table[0x62] = std::bind(&sharpsm83::ld_h_d, this);
    opcode_table[0x63] = std::bind(&sharpsm83::ld_h_e, this);
    opcode_table[0x64] = std::bind(&sharpsm83::ld_h_h, this);
    opcode_table[0x65] = std::bind(&sharpsm83::ld_h_l, this);
    opcode_table[0x66] = std::bind(&sharpsm83::ld_h_memhl, this);
    opcode_table[0x67] = std::bind(&sharpsm83::ld_h_a, this);
    opcode_table[0x68] = std::bind(&sharpsm83::ld_l_b, this);
    opcode_table[0x69] = std::bind(&sharpsm83::ld_l_c, this);
    opcode_table[0x6A] = std::bind(&sharpsm83::ld_l_d, this);
    opcode_table[0x6B] = std::bind(&sharpsm83::ld_l_e, this);
    opcode_table[0x6C] = std::bind(&sharpsm83::ld_l_h, this);
    opcode_table[0x6D] = std::bind(&sharpsm83::ld_l_l, this);
    opcode_table[0x6E] = std::bind(&sharpsm83::ld_l_memhl, this);
    opcode_table[0x6F] = std::bind(&sharpsm83::ld_l_a, this);

    opcode_table[0x70] = std::bind(&sharpsm83::ld_memhl_b, this);
    opcode_table[0x71] = std::bind(&sharpsm83::ld_memhl_c, this);
    opcode_table[0x72] = std::bind(&sharpsm83::ld_memhl_d, this);
    opcode_table[0x73] = std::bind(&sharpsm83::ld_memhl_e, this);
    opcode_table[0x74] = std::bind(&sharpsm83::ld_memhl_h, this);
    opcode_table[0x75] = std::bind(&sharpsm83::ld_memhl_l, this);
    opcode_table[0x76] = std::bind(&sharpsm83::halt, this);
    opcode_table[0x77] = std::bind(&sharpsm83::ld_memhl_a, this);
    opcode_table[0x78] = std::bind(&sharpsm83::ld_a_b, this);
    opcode_table[0x79] = std::bind(&sharpsm83::ld_a_c, this);
    opcode_table[0x7A] = std::bind(&sharpsm83::ld_a_d, this);
    opcode_table[0x7B] = std::bind(&sharpsm83::ld_a_e, this);
    opcode_table[0x7C] = std::bind(&sharpsm83::ld_a_h, this);
    opcode_table[0x7D] = std::bind(&sharpsm83::ld_a_l, this);
    opcode_table[0x7E] = std::bind(&sharpsm83::ld_a_memhl, this);
    opcode_table[0x7F] = std::bind(&sharpsm83::ld_a_a, this);

    opcode_table[0x80] = std::bind(&sharpsm83::add_a_b, this);
    opcode_table[0x81] = std::bind(&sharpsm83::add_a_c, this);
    opcode_table[0x82] = std::bind(&sharpsm83::add_a_d, this);
    opcode_table[0x83] = std::bind(&sharpsm83::add_a_e, this);
    opcode_table[0x84] = std::bind(&sharpsm83::add_a_h, this);
    opcode_table[0x85] = std::bind(&sharpsm83::add_a_l, this);
    opcode_table[0x86] = std::bind(&sharpsm83::add_a_memhl, this);
    opcode_table[0x87] = std::bind(&sharpsm83::add_a_a, this);
    opcode_table[0x88] = std::bind(&sharpsm83::adc_a_b, this);
    opcode_table[0x89] = std::bind(&sharpsm83::adc_a_c, this);
    opcode_table[0x8A] = std::bind(&sharpsm83::adc_a_d, this);
    opcode_table[0x8B] = std::bind(&sharpsm83::adc_a_e, this);
    opcode_table[0x8C] = std::bind(&sharpsm83::adc_a_h, this);
    opcode_table[0x8D] = std::bind(&sharpsm83::adc_a_l, this);
    opcode_table[0x8E] = std::bind(&sharpsm83::adc_a_memhl, this);
    opcode_table[0x8F] = std::bind(&sharpsm83::adc_a_a, this);

    opcode_table[0x90] = std::bind(&sharpsm83::sub_a_b, this);
    opcode_table[0x91] = std::bind(&sharpsm83::sub_a_c, this);
    opcode_table[0x92] = std::bind(&sharpsm83::sub_a_d, this);
    opcode_table[0x93] = std::bind(&sharpsm83::sub_a_e, this);
    opcode_table[0x94] = std::bind(&sharpsm83::sub_a_h, this);
    opcode_table[0x95] = std::bind(&sharpsm83::sub_a_l, this);
    opcode_table[0x96] = std::bind(&sharpsm83::sub_a_memhl, this);
    opcode_table[0x97] = std::bind(&sharpsm83::sub_a_a, this);
    opcode_table[0x98] = std::bind(&sharpsm83::sbc_a_b, this);
    opcode_table[0x99] = std::bind(&sharpsm83::sbc_a_c, this);
    opcode_table[0x9A] = std::bind(&sharpsm83::sbc_a_d, this);
    opcode_table[0x9B] = std::bind(&sharpsm83::sbc_a_e, this);
    opcode_table[0x9C] = std::bind(&sharpsm83::sbc_a_h, this);
    opcode_table[0x9D] = std::bind(&sharpsm83::sbc_a_l, this);
    opcode_table[0x9E] = std::bind(&sharpsm83::sbc_a_memhl, this);
    opcode_table[0x9F] = std::bind(&sharpsm83::sbc_a_a, this);  

    opcode_table[0xA0] = std::bind(&sharpsm83::and_a_b, this);  
    opcode_table[0xA1] = std::bind(&sharpsm83::and_a_c, this);  
    opcode_table[0xA2] = std::bind(&sharpsm83::and_a_d, this);  
    opcode_table[0xA3] = std::bind(&sharpsm83::and_a_e, this);  
    opcode_table[0xA4] = std::bind(&sharpsm83::and_a_h, this);  
    opcode_table[0xA5] = std::bind(&sharpsm83::and_a_l, this);  
    opcode_table[0xA6] = std::bind(&sharpsm83::and_a_memhl, this);  
    opcode_table[0xA7] = std::bind(&sharpsm83::and_a_a, this);  
    opcode_table[0xA8] = std::bind(&sharpsm83::xor_a_b, this);  
    opcode_table[0xA9] = std::bind(&sharpsm83::xor_a_c, this);  
    opcode_table[0xAA] = std::bind(&sharpsm83::xor_a_d, this);  
    opcode_table[0xAB] = std::bind(&sharpsm83::xor_a_e, this);  
    opcode_table[0xAC] = std::bind(&sharpsm83::xor_a_h, this);  
    opcode_table[0xAD] = std::bind(&sharpsm83::xor_a_l, this);  
    opcode_table[0xAE] = std::bind(&sharpsm83::xor_a_memhl, this);  
    opcode_table[0xAF] = std::bind(&sharpsm83::xor_a_a, this);  

    opcode_table[0xB0] = std::bind(&sharpsm83::or_a_b, this);  
    opcode_table[0xB1] = std::bind(&sharpsm83::or_a_c, this);  
    opcode_table[0xB2] = std::bind(&sharpsm83::or_a_d, this);  
    opcode_table[0xB3] = std::bind(&sharpsm83::or_a_e, this);  
    opcode_table[0xB4] = std::bind(&sharpsm83::or_a_h, this);  
    opcode_table[0xB5] = std::bind(&sharpsm83::or_a_l, this);  
    opcode_table[0xB6] = std::bind(&sharpsm83::or_a_memhl, this);  
    opcode_table[0xB7] = std::bind(&sharpsm83::or_a_a, this);  
    opcode_table[0xB8] = std::bind(&sharpsm83::cp_a_b, this);  
    opcode_table[0xB9] = std::bind(&sharpsm83::cp_a_c, this);  
    opcode_table[0xBA] = std::bind(&sharpsm83::cp_a_d, this);  
    opcode_table[0xBB] = std::bind(&sharpsm83::cp_a_e, this);  
    opcode_table[0xBC] = std::bind(&sharpsm83::cp_a_h, this);  
    opcode_table[0xBD] = std::bind(&sharpsm83::cp_a_l, this);
    opcode_table[0xBE] = std::bind(&sharpsm83::cp_a_memhl, this); 
    opcode_table[0xBF] = std::bind(&sharpsm83::cp_a_a, this);

    opcode_table[0xC0] = std::bind(&sharpsm83::ret_nz, this);
    opcode_table[0xC1] = std::bind(&sharpsm83::pop_bc, this);
    opcode_table[0xC2] = std::bind(&sharpsm83::jp_nz_imm16, this);
    opcode_table[0xC3] = std::bind(&sharpsm83::jp_imm16, this);
    opcode_table[0xC4] = std::bind(&sharpsm83::call_nz_imm16, this);
    opcode_table[0xC5] = std::bind(&sharpsm83::push_bc, this);
    opcode_table[0xC6] = std::bind(&sharpsm83::add_a_imm8, this);
    opcode_table[0xC7] = std::bind(&sharpsm83::rst_0x00, this);
    opcode_table[0xC8] = std::bind(&sharpsm83::ret_z, this);
    opcode_table[0xC9] = std::bind(&sharpsm83::ret, this);
    opcode_table[0xCA] = std::bind(&sharpsm83::jp_z_imm16, this);
    opcode_table[0xCB] = std::bind(&sharpsm83::prefix, this);
    opcode_table[0xCC] = std::bind(&sharpsm83::call_z_imm16, this);
    opcode_table[0xCD] = std::bind(&sharpsm83::call_imm16, this);
    opcode_table[0xCE] = std::bind(&sharpsm83::adc_a_imm8, this);
    opcode_table[0xCF] = std::bind(&sharpsm83::rst_0x08, this);

    opcode_table[0xD0] = std::bind(&sharpsm83::ret_nc, this);
    opcode_table[0xD1] = std::bind(&sharpsm83::pop_de, this);
    opcode_table[0xD2] = std::bind(&sharpsm83::jp_nc_imm16, this);
    opcode_table[0xD3] = std::bind(&sharpsm83::op_0xD3, this);
    opcode_table[0xD4] = std::bind(&sharpsm83::call_nc_imm16, this);
    opcode_table[0xD5] = std::bind(&sharpsm83::push_de, this);
    opcode_table[0xD6] = std::bind(&sharpsm83::sub_a_imm8, this);
    opcode_table[0xD7] = std::bind(&sharpsm83::rst_0x10, this);
    opcode_table[0xD8] = std::bind(&sharpsm83::ret_c, this);
    opcode_table[0xD9] = std::bind(&sharpsm83::reti, this);
    opcode_table[0xDA] = std::bind(&sharpsm83::jp_c_imm16, this);
    opcode_table[0xDB] = std::bind(&sharpsm83::op_0xDB, this);
    opcode_table[0xDC] = std::bind(&sharpsm83::call_c_a16, this);
    opcode_table[0xDD] = std::bind(&sharpsm83::op_0xDD, this);
    opcode_table[0xDE] = std::bind(&sharpsm83::sbc_a_imm8, this);
    opcode_table[0xDF] = std::bind(&sharpsm83::rst_0x18, this);

    opcode_table[0xE0] = std::bind(&sharpsm83::ldh_memimm8_a, this);
    opcode_table[0xE1] = std::bind(&sharpsm83::pop_hl, this);
    opcode_table[0xE2] = std::bind(&sharpsm83::ldh_memc_a, this);
    opcode_table[0xE3] = std::bind(&sharpsm83::op_0xE3, this);
    opcode_table[0xE4] = std::bind(&sharpsm83::op_0xE4, this);
    opcode_table[0xE5] = std::bind(&sharpsm83::push_hl, this);
    opcode_table[0xE6] = std::bind(&sharpsm83::and_a_imm8, this);
    opcode_table[0xE7] = std::bind(&sharpsm83::rst_0x20, this);
    opcode_table[0xE8] = std::bind(&sharpsm83::add_sp_e8, this);
    opcode_table[0xE9] = std::bind(&sharpsm83::jp_hl, this);
    opcode_table[0xEA] = std::bind(&sharpsm83::ld_memimm16_a, this);
    opcode_table[0xEB] = std::bind(&sharpsm83::op_0xEB, this);
    opcode_table[0xEC] = std::bind(&sharpsm83::op_0xEC, this);
    opcode_table[0xED] = std::bind(&sharpsm83::op_0xED, this);
    opcode_table[0xEE] = std::bind(&sharpsm83::xor_a_imm8, this);
    opcode_table[0xEF] = std::bind(&sharpsm83::rst_0x28, this);

    CB_opcode_table[0x00] = std::bind(&sharpsm83::rlc_b, this);
    CB_opcode_table[0x01] = std::bind(&sharpsm83::rlc_c, this);
    CB_opcode_table[0x02] = std::bind(&sharpsm83::rlc_d, this);
    CB_opcode_table[0x03] = std::bind(&sharpsm83::rlc_e, this);
    CB_opcode_table[0x04] = std::bind(&sharpsm83::rlc_h, this);
    CB_opcode_table[0x05] = std::bind(&sharpsm83::rlc_l, this);
    CB_opcode_table[0x06] = std::bind(&sharpsm83::rlc_memhl, this);
    CB_opcode_table[0x07] = std::bind(&sharpsm83::rlc_a, this);
}
//##############################################################################
void sharpsm83::printRegisters()
{
    std::cout<<"##############################################################################"<<'\n';
    std::cout<<"AF: 0x" << std::hex<<static_cast<int>(AF.b0_15)<<'\n';
    std::cout<<"BC: 0x" << std::hex<<static_cast<int>(BC.b0_15)<<'\n';
    std::cout<<"DE: 0x" << std::hex<<static_cast<int>(DE.b0_15)<<'\n';
    std::cout<<"HL: 0x" << std::hex<<static_cast<int>(HL.b0_15)<<'\n';
    std::cout<<"PC: 0x" << std::hex<<static_cast<int>(PC.b0_15)<<'\n';
    std::cout<<"SP: 0x" << std::hex<<static_cast<int>(SP.b0_15)<<'\n';
    std::cout<<"Flags: Z: " << get_zero_flag() <<" N: " << get_subtraction_flag() 
        << " H: "<<getHalfCarryFlag() << " C: "<<get_carry_flag()<<'\n';

    std::cout<<"##############################################################################"<<'\n';

    //mem.print_memory_layout();
}
//##############################################################################
void sharpsm83::reset()
{
    PC.b0_15 = 0x00;

    AF.b0_15 = 0xAA;
    BC.b0_15 = 0x00;
    DE.b0_15 = 0x00;
    HL.b0_15 = 0x00;
    PC.b0_15 = 0x00;
    SP.b0_15 = 0x00;

    fetched_data = 0x0;
}
//##############################################################################