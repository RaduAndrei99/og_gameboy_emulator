#include<iostream>
#include "cpu_sharpsm83.hpp"
#include "opcode_to_string.hpp"

//##############################################################################
sharpsm83::sharpsm83()
{
    initialize_opcodes();
    initialize_cbopcodes();
}
//##############################################################################
sharpsm83::~sharpsm83() = default;
//##############################################################################
void sharpsm83::set_bus(const std::shared_ptr<gb_bus>& b)
{
    bus = b;
}
//##############################################################################
void sharpsm83::enable_interrupts()
{
    interrupts_enabled = true;
    emulate_cycles(1);
    PC.b0_15 += 1;    
}
//##############################################################################
void sharpsm83::disable_interrupts()
{
    interrupts_enabled = false;
    emulate_cycles(1);
    PC.b0_15 += 1;
}
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
bool sharpsm83::get_half_carry_flag()
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
    if(bus)
        fetched_data = bus->bus_read(address);
}
//##############################################################################
void sharpsm83::write_data(const uint16_t& address, const uint8_t& data)
{
    if(bus)
        bus->bus_write(address, data);
}
//##############################################################################
void sharpsm83::tick()
{
    handle_interrupts();

    if(is_halted) return;

    fetch_data(PC.b0_15);
    uint8_t op1 = fetched_data;

    fetch_data(PC.b0_15 + 1);
    uint8_t op2 = fetched_data;

    fetch_data(PC.b0_15 + 2);
    uint8_t op3 = fetched_data;

    // if(op1 == 0x01 && op2 == 0x00 && op3 == 0x12)
    // {
    //     std::cout<<"Reached 0x010012: "<<std::hex<<static_cast<int>(PC.b0_15)<<std::endl;
    //     exit(0);
    // }

    fetch_data(PC.b0_15);
    uint8_t opcode = fetched_data;

    execute(opcode);
}
//##############################################################################
void sharpsm83::execute(uint8_t opcode) 
{
    if(opcode == 0xCB)
    {
        fetch_data(PC.b0_15 + 1);
        opcode = fetched_data;

        execute_0xCB_instruction(opcode);

        //std::cout<<"->"<<std::hex<<static_cast<int>(PC.b0_15)<<": "<<cbOpcodeTable[opcode]<<" ["<<"$CB"<<std::hex<<static_cast<int>(opcode)<<']'<<'\n';

        last_opcode = 0xCB00 | opcode;
    }
    else
    {   
        //std::cout<<"->"<<std::hex<<static_cast<int>(PC.b0_15)<<": "<<opcodeTable[opcode]<<" ["<<std::hex<<static_cast<int>(opcode)<<']'<<'\n';

        execute_normal_instruction(opcode);

        last_opcode = opcode;
    }
}
//##############################################################################
void sharpsm83::execute_normal_instruction(uint8_t opcode)
{
    if(opcode_table[opcode])
    {
        opcode_table[opcode]();
    }
    else
    {
        std::cout<<"Unrecognized instruction: 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';
        exit(-1);
    }
}
//##############################################################################
void sharpsm83::execute_0xCB_instruction(uint8_t opcode)
{
    if(CB_opcode_table[opcode])
    {
        CB_opcode_table[opcode]();
    }
    else
    {
        std::cout<<"Unrecognized instruction: 0xCB 0x"<<std::hex<<static_cast<int>(opcode)<<'\n';
        exit(-1);
    }
}
//##############################################################################
void sharpsm83::execute_nop()
{
    emulate_cycles(1);
    PC.b0_15 += 1;
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
    int8_t offset = fetched_data;
    
    if (exit_on_infinite_jr && offset == -2) { exit(0); }

    uint16_t starting_point = PC.b0_15 + 2;

    uint16_t new_pc = (uint16_t)(starting_point + offset);// ???

    if(cond)
    {
        PC.b0_15 = new_pc;
        emulate_cycles(3);

    }
    else
    {
        PC.b0_15 += 2;
        emulate_cycles(2);
    }    
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
    fetch_data(PC.b0_15 + 1);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 2);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    emulate_cycles(3);
    
    PC.b0_15 = cond ? address : PC.b0_15 + 3;
}
//##############################################################################
void sharpsm83::call(bool cond)
{
    fetch_data(PC.b0_15 + 1);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 2);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    if(cond)
    {
        emulate_cycles(6);

        stack_push(PC.b0_15 + 3);
        
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
    stack_push(PC.b0_15 + 3);

    emulate_cycles(6);

    PC.b0_15 = address;
}
//##############################################################################
void sharpsm83::rst(const uint16_t& address)
{
    stack_push(PC.b0_15 + 1);

    emulate_cycles(6);

    PC.b0_15 = address;
}
//##############################################################################
void sharpsm83::da(reg8& reg)
{
    uint8_t a = reg.b0_7;
    bool n = get_subtraction_flag();
    bool h = get_half_carry_flag();
    bool c = get_carry_flag();
    uint8_t adjust = 0;
    bool new_carry = c;

    if (!n) {
        if (h || (a & 0x0F) > 0x09)
            adjust |= 0x06;
        if (c || a > 0x99) {
            adjust |= 0x60;
            new_carry = true;
        }
        a += adjust;
    } else {
        if (h)
            adjust |= 0x06;
        if (c)
            adjust |= 0x60;
        a -= adjust;
        // new_carry stays as previous carry
    }

    reg.b0_7 = a;
    set_zero_flag(a == 0);
    set_half_carry_flag(false);
    set_carry_flag(new_carry);

    emulate_cycles(1);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::complement(reg8& reg)
{
    reg.b0_7 = ~reg.b0_7;

    set_subtraction_flag(true); // N flag
    set_half_carry_flag(true); // H flag

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
    uint16_t address = imm8 + 0xFF00;

    write_data(address, reg.b0_7);

    emulate_cycles(3);
    
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ldh_to_address(const reg8& to, const reg8& reg)
{
    uint16_t address = to.b0_7 + 0xFF00;

    write_data(address, reg.b0_7);

    emulate_cycles(2);
    
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

    write_data(address, SP.b0_15 & 0xFF);        // low byte
    write_data(address + 1, (SP.b0_15 >> 8));   // high byte

    emulate_cycles(2);

    PC.b0_15 += 3;
}
//##############################################################################
void sharpsm83::ldh_from_address(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);

    uint8_t imm8 = fetched_data;
    uint16_t address = imm8 + 0xFF00;

    fetch_data(address);
    reg.b0_7 = fetched_data;

    emulate_cycles(3);
    
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ldh_from_address(reg8& to, const reg8& reg)
{
    uint16_t address = reg.b0_7 + 0xFF00;

    fetch_data(address);
    to.b0_7 = fetched_data;

    emulate_cycles(2);
    
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::ld_hl_reg_e8(const reg16& reg)
{
    fetch_data(PC.b0_15 + 1);
    int8_t imm8 = static_cast<int8_t>(fetched_data);

    uint16_t sp = reg.b0_15;
    uint16_t result = sp + imm8;

    HL.b0_15 = result;

    set_zero_flag(false);        // Z is always 0
    set_subtraction_flag(false); // N is always 0

    // Half-carry from bit 3->4 of low byte
    set_half_carry_flag(((sp & 0x0F) + (imm8 & 0x0F)) > 0x0F);

    // Carry from bit 7->8 of low byte
    set_carry_flag(((sp & 0xFF) + (uint8_t)imm8) > 0xFF);

    emulate_cycles(3);
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::ld_a_memimm16_op()
{
    fetch_data(PC.b0_15 + 1);
    uint8_t low = fetched_data;
    fetch_data(PC.b0_15 + 2);
    uint8_t high = fetched_data;

    uint16_t address = (high << 0x8) | low;

    fetch_data(address);
    AF.Hi.b0_7 = fetched_data;

    emulate_cycles(4);

    PC.b0_15 += 3;
}
//##############################################################################
void sharpsm83::ld_sp_hl_op()
{
    SP.b0_15 = HL.b0_15;

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
    uint8_t lo = fetched_data;

    fetch_data(PC.b0_15 + 2);
    uint8_t hi = fetched_data;

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
    set_half_carry_flag(((reg & 0x0F) + 1) > 0x0F); // H flag

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
    set_half_carry_flag(((fetched_data & 0x0F) + 1) > 0x0F); // H flag

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
    set_half_carry_flag(((reg & 0xF) == 0)); // H flag

    reg = result;

    emulate_cycles(1);

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
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((fetched_data & 0x0F) == 0x00); // H flag

    write_data(address, result);

    emulate_cycles(3);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add(uint16_t& op1, uint16_t& op2)
{
    uint32_t result = op1 + op2;

    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((op1 & 0xFFF) + (op2 & 0xFFF)) > 0xFFF); // H flag ???
    set_carry_flag(result > UINT16_MAX); // C flag

    op1 = result & 0xFFFF;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::add(uint8_t& op1, uint8_t& op2)
{   
    uint16_t result = op1 + op2;

    set_zero_flag((result & 0xFF) == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((op1 & 0xF) + (op2 & 0xF)) > 0xF); // H flag ???
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

    set_zero_flag((result & 0xFF) == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((op1 & 0xF) + (fetched_data & 0xF)) > 0xF); // H flag ???
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

    set_zero_flag((result & 0xFF) == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((reg.b0_7 & 0xF) + (fetched_data & 0xF)) > 0xF); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    reg.b0_7 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::add_e8(reg16& reg)
{
    fetch_data(PC.b0_15 + 1);
    int8_t e8 = static_cast<int8_t>(fetched_data);

    uint16_t sp = reg.b0_15;
    uint16_t result = sp + e8;

    set_zero_flag(false);        // Z flag always cleared
    set_subtraction_flag(false); // N flag always cleared

    // Half carry: check carry from bit 3 -> 4 in low byte
    set_half_carry_flag(((sp & 0x0F) + (e8 & 0x0F)) > 0x0F);

    // Carry: check carry from bit 7 -> 8 in low byte
    set_carry_flag(((sp & 0xFF) + (uint8_t)e8) > 0xFF);

    reg.b0_15 = result;

    emulate_cycles(4);
    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::adc(uint8_t& op1, uint8_t& op2)
{
    bool old_carry = get_carry_flag();
    uint16_t sum = op1 + op2 + (old_carry ? 1 : 0);

    set_zero_flag((sum & 0xFF) == 0);
    set_subtraction_flag(false);
    set_half_carry_flag(((op1 & 0xF) + (op2 & 0xF) + (old_carry ? 1 : 0)) > 0xF);
    set_carry_flag(sum > 0xFF);

    op1 = sum & 0xFF;

    emulate_cycles(1);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::adc_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);
    bool old_carry = get_carry_flag();
    uint16_t result = op1 + fetched_data + old_carry;

    set_zero_flag((result & 0xFF) == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((op1 & 0xF) + (fetched_data & 0xF) + old_carry) > 0xF); // H flag ???
    set_carry_flag(result > UINT8_MAX); // C flag

    op1 = result & 0xFF;

    emulate_cycles(2);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::adc(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    bool old_carry = get_carry_flag();

    uint16_t result = reg.b0_7 + fetched_data + old_carry;

    set_zero_flag((result & 0xFF) == 0); // Z FLAG
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(((reg.b0_7 & 0xF) + (fetched_data & 0xF) + old_carry) > 0xF); // H flag ???
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
    set_half_carry_flag((op1 & 0xF) < (op2 & 0xF)); // H flag ???
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
    uint8_t value = fetched_data;

    uint8_t result = reg.b0_7 - value;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((reg.b0_7 & 0xF) < (value & 0xF)); // H flag
    set_carry_flag(value > reg.b0_7);           // C flag

    reg.b0_7 = result;

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::sbc(uint8_t& op1, uint8_t& op2)
{
    bool carry = get_carry_flag();

    int result_full = op1 - op2 - carry;
    uint8_t result = result_full & 0xFF;

    set_zero_flag(result == 0);
    set_subtraction_flag(true);
    set_half_carry_flag(((op1 & 0xf) - (op2 & 0xf) - carry) < 0);
    set_carry_flag(result_full < 0);

    op1 = result;

    emulate_cycles(2);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::sbc_from_address(uint8_t& op1, uint16_t& address)
{
    fetch_data(address);
    bool carry = get_carry_flag();
    uint8_t value = fetched_data;

    int result_full = op1 - value -carry;
    uint8_t result = result_full & 0xFF;

    set_zero_flag(result == 0);
    set_subtraction_flag(true);
    set_half_carry_flag(((op1 & 0xf) - (value & 0xf) - carry) < 0);
    set_carry_flag(result_full < 0);

    op1 = result;

    emulate_cycles(2);
    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::sbc(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    bool carry = get_carry_flag();
    uint8_t value = fetched_data;

    int result_full = reg.b0_7 - value - carry;
    uint8_t result = result_full & 0xFF;

    set_zero_flag(result == 0);
    set_subtraction_flag(true);
    set_half_carry_flag(((reg.b0_7 & 0xf) - (value & 0xf) - carry) < 0);
    set_carry_flag(result_full < 0);

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

    PC.b0_15 += 2;
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
    set_half_carry_flag((op1 & 0xF) < (op2 & 0xF)); // H flag ???
    set_carry_flag(op2 > op1); // C flag

    emulate_cycles(1);

    PC.b0_15 += 1;
}
//##############################################################################
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
void sharpsm83::cp_op(reg8& reg)
{
    fetch_data(PC.b0_15 + 1);
    uint8_t result = reg.b0_7 - fetched_data;

    set_zero_flag(result == 0); // Z FLAG
    set_subtraction_flag(true); // N flag
    set_half_carry_flag((fetched_data & 0xF) > (reg.b0_7 & 0xF)); // H flag ???
    set_carry_flag(fetched_data > reg.b0_7); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}
//##############################################################################
void sharpsm83::stack_pop(uint16_t& val)
{
    fetch_data(SP.b0_15);
    uint8_t low = fetched_data;
    SP.b0_15 += 1;

    fetch_data(SP.b0_15);
    uint8_t high = fetched_data;
    SP.b0_15 += 1;

    val = (high << 0x8) | low;
}
//##############################################################################
void sharpsm83::pop(reg16& reg)
{
    stack_pop(reg.b0_15); 

    emulate_cycles(3);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::pop_af_op()
{
    stack_pop(AF.b0_15);

    // the lower nibble of F is always zero
    AF.b0_15 &= 0xFFF0; 
    
    emulate_cycles(3);

    PC.b0_15 += 1;

}
//##############################################################################
void sharpsm83::push_af_op()
{
    // the lower nibble of F is always zero
    AF.b0_15 &= 0xFFF0; 

    stack_push(AF.b0_15);

    emulate_cycles(4);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::push(reg16& reg)
{
    stack_push(reg.b0_15);

    emulate_cycles(4);

    PC.b0_15 += 1;
}
//##############################################################################
void sharpsm83::stack_push(const uint16_t& value)
{
    SP.b0_15 -= 1;
    write_data(SP.b0_15, value >> 0x8);
    SP.b0_15 -= 1;
    write_data(SP.b0_15, value & 0xFF);
}
//##############################################################################
void sharpsm83::ret_op()
{
    stack_pop(PC.b0_15);   
    emulate_cycles(4);
}
//##############################################################################
void sharpsm83::ret_condition(bool condition)
{
    if(condition) 
    {
        stack_pop(PC.b0_15);   
        
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

    ret_op();
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
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::rrc_param(reg8& reg)
{
    bool lsb = reg.b0;
    reg.b0_7 >>= 1;
    reg.b7 = lsb;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::rrcmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool lsb = result.b0;
    result.b0_7 >>= 1;
    result.b7 = lsb;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::rl_param(reg8& reg)
{
    bool msb = reg.b7;
    reg.b0_7 <<= 1;
    reg.b0 = get_carry_flag();

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::rlmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool msb = result.b7;
    result.b0_7 <<= 1;
    result.b0 = get_carry_flag();

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::rr_param(reg8& reg)
{
    bool lsb = reg.b0;
    reg.b0_7 >>= 1;
    reg.b7 = get_carry_flag();

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::rrmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool lsb = result.b0;
    result.b0_7 >>= 1;
    result.b7 = get_carry_flag();

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::sla_param(reg8& reg)
{
    bool msb = reg.b7;
    reg.b0_7 <<= 1;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::slamem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool msb = result.b7;
    result.b0_7 <<= 1;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(msb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::sra_param(reg8& reg)
{
    bool msb = reg.b7;
    bool lsb = reg.b0;
    reg.b0_7 >>= 1;
    reg.b7 = msb;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::sramem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool msb = result.b7;
    bool lsb = result.b0;
    result.b0_7 >>= 1;
    result.b7 = msb;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::swap_param(reg8& reg)
{
    uint8_t upper = (reg.b0_7 & 0xF0) >> 4;
    uint8_t lower = (reg.b0_7 & 0x0F) << 4;
    reg.b0_7 = upper | lower;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(false); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::swapmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    uint8_t upper = (result.b0_7 & 0xF0) >> 4;
    uint8_t lower = (result.b0_7 & 0x0F) << 4;
    result.b0_7 = upper | lower;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(false); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::srl_param(reg8& reg)
{
    bool lsb = reg.b0;
    reg.b0_7 >>= 1;
    reg.b7 = 0;

    set_zero_flag(reg.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::srlmem_param(uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool lsb = result.b0;
    result.b0_7 >>= 1;
    result.b7 = 0;

    set_zero_flag(result.b0_7 == 0); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(false); // H flag
    set_carry_flag(lsb); // C flag
    write_data(address, result.b0_7);
    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::bit_param(uint8_t bit, reg8& reg)
{
    bool bit_value = (reg.b0_7 >> bit) & 0x01;

    set_zero_flag(!bit_value); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(true); // H flag

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::bitmem_param(uint8_t bit, uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    bool bit_value = (result.b0_7 >> bit) & 0x01;

    set_zero_flag(!bit_value); // Z flag
    set_subtraction_flag(false); // N flag
    set_half_carry_flag(true); // H flag

    emulate_cycles(3);

    PC.b0_15 += 2;
}

void sharpsm83::res_param(uint8_t bit, reg8& reg)
{
    reg.b0_7 &= ~(1 << bit);

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::resmem_param(uint8_t bit, uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    result.b0_7 &= ~(1 << bit);
    write_data(address, result.b0_7);

    emulate_cycles(4);

    PC.b0_15 += 2;
}

void sharpsm83::set_param(uint8_t bit, reg8& reg)
{
    reg.b0_7 |= (1 << bit);

    emulate_cycles(2);

    PC.b0_15 += 2;
}

void sharpsm83::setmem_param(uint8_t bit, uint16_t& address)
{
    fetch_data(address);
    reg8 result;
    
    result.b0_7 = fetched_data;
    result.b0_7 |= (1 << bit);
    write_data(address, result.b0_7);

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
void sharpsm83::ld_e_imm8() { ld(DE.Lo.b0_7); } // 0x1E
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
void sharpsm83::ret_nz() { ret_condition(!get_zero_flag()); } // 0xC0
void sharpsm83::pop_bc() { pop(BC); } // 0xC1
void sharpsm83::jp_nz_imm16() { jp(!get_zero_flag()); } // 0xC2
void sharpsm83::jp_imm16() { jp(true); } // 0xC3
void sharpsm83::call_nz_imm16() { call(!get_zero_flag()); } // 0xC4
void sharpsm83::push_bc() { push(BC); } // 0xC5
void sharpsm83::add_a_imm8() { add(AF.Hi); } // 0xC6
void sharpsm83::rst_0x00() { rst( (uint16_t)0x0000 ); }// 0xC7
void sharpsm83::ret_z() { ret_condition(get_zero_flag());  } //0xC8
void sharpsm83::ret() { ret_op(); } //0xC9
void sharpsm83::jp_z_imm16() { jp(get_zero_flag()); }  //0xCA
void sharpsm83::prefix() { }  // 0xCB
void sharpsm83::call_z_imm16() { call(get_zero_flag()); } // 0xCC
void sharpsm83::call_imm16() { call(true); } // 0xCD
void sharpsm83::adc_a_imm8() { adc(AF.Hi); } // 0xCE
void sharpsm83::rst_0x08() { rst( (uint16_t)0x0008 ); } // 0xCF
//##############################################################################
void sharpsm83::ret_nc() { ret_condition(!get_carry_flag()); } // 0xD0
void sharpsm83::pop_de() { pop(DE); } // 0xD1
void sharpsm83::jp_nc_imm16() { jp(!get_carry_flag()); } // 0xD2
void sharpsm83::op_0xD3() {  } // 0xD3
void sharpsm83::call_nc_imm16() { call(!get_carry_flag()); } // 0xD4
void sharpsm83::push_de() { push(DE); } // 0xD5
void sharpsm83::sub_a_imm8() { sub(AF.Hi); } // 0xD6
void sharpsm83::rst_0x10() {  rst( (uint16_t)0x0010 ); } // 0xD7
void sharpsm83::ret_c() { ret_condition(get_carry_flag()); } // 0xD8
void sharpsm83::reti() { reti_op(); } // 0xD9
void sharpsm83::jp_c_imm16() { jp(get_carry_flag()); } // 0xDA
void sharpsm83::op_0xDB() { } // 0xDB
void sharpsm83::call_c_a16() { call(get_carry_flag()); } // 0xDC
void sharpsm83::op_0xDD() { } // 0xDD
void sharpsm83::sbc_a_imm8() { sbc(AF.Hi); }// 0xDE
void sharpsm83::rst_0x18() { rst( (uint16_t)0x0018 ); } // 0xDF
//##############################################################################
void sharpsm83::ldh_memimm8_a() { ldh_to_address(AF.Hi); } // 0xE0
void sharpsm83::pop_hl() { pop(HL); } // 0xE1
void sharpsm83::ldh_memc_a() { ldh_to_address(BC.Lo, AF.Hi); } // 0xE2
void sharpsm83::op_0xE3() {  } // 0xE3
void sharpsm83::op_0xE4() {  } // 0xE4
void sharpsm83::push_hl() { push(HL); } // 0xE5
void sharpsm83::and_a_imm8() { and_op(AF.Hi); } // 0xE6
void sharpsm83::rst_0x20() { rst( (uint16_t)0x0020 ); } // 0xE7
void sharpsm83::add_sp_e8() { add_e8(SP); } // 0xE8
void sharpsm83::jp_hl() { jp(HL.b0_15); } // 0xE9
void sharpsm83::ld_memimm16_a() { ld_to_address(AF.Hi); } // 0xEA
void sharpsm83::op_0xEB() { } // 0xEB
void sharpsm83::op_0xEC() { } // 0xEC
void sharpsm83::op_0xED() { } // 0xED
void sharpsm83::xor_a_imm8() { xor_op(AF.Hi); } // 0xEE
void sharpsm83::rst_0x28() { rst( (uint16_t)0x0028 ); }// 0xEF
//##############################################################################
void sharpsm83::ldh_a_memimm8(){ ldh_from_address(AF.Hi); } // 0xF0
void sharpsm83::pop_af() { pop_af_op(); } // 0xF1
void sharpsm83::ldh_a_memc() { ldh_from_address(AF.Hi, BC.Lo); } // 0xF2
void sharpsm83::di() { disable_interrupts(); } // 0xF3
void sharpsm83::op_0xF4() { }
void sharpsm83::push_af() { push_af_op(); } // 0xF5
void sharpsm83::or_a_imm8() { or_op(AF.Hi); } // 0xF6
void sharpsm83::rst_0x30() { rst( (uint16_t)0x0030 ); } // 0xF7
void sharpsm83::ld_hl_sp_e8() { ld_hl_reg_e8(SP); } // 0xF8
void sharpsm83::ld_sp_hl() { ld_sp_hl_op(); } // 0xF9   
void sharpsm83::ld_a_memimm16() { ld_a_memimm16_op(); } // 0xFA
void sharpsm83::ei() { enable_interrupts(); } // 0xFB
void sharpsm83::op_0xFC() { } // 0xFC
void sharpsm83::op_0xFD() { } // 0xFD
void sharpsm83::cp_a_imm8() { cp_op(AF.Hi); } // 0xFE
void sharpsm83::rst_0x38() { rst( (uint16_t)0x0038 ); } // 0xFF
//##############################################################################
// 0xCB instructions
//##############################################################################
void sharpsm83::rlc_b() { rlc_param(BC.Hi); } // Ox00
void sharpsm83::rlc_c() { rlc_param(BC.Lo); } // Ox01
void sharpsm83::rlc_d() { rlc_param(DE.Hi); } // Ox02
void sharpsm83::rlc_e() { rlc_param(DE.Lo); } // Ox03
void sharpsm83::rlc_h() { rlc_param(HL.Hi); } // Ox04
void sharpsm83::rlc_l() { rlc_param(HL.Lo); } // Ox05
void sharpsm83::rlc_memhl() { rlcmem_param(HL.b0_15); } // Ox06
void sharpsm83::rlc_a() { rlc_param(AF.Hi); } // Ox07

void sharpsm83::rrc_b() { rrc_param(BC.Hi); } // Ox08
void sharpsm83::rrc_c() { rrc_param(BC.Lo); } // Ox09
void sharpsm83::rrc_d() { rrc_param(DE.Hi); } // Ox0A
void sharpsm83::rrc_e() { rrc_param(DE.Lo); } // Ox0B
void sharpsm83::rrc_h() { rrc_param(HL.Hi); } // Ox0C
void sharpsm83::rrc_l() { rrc_param(HL.Lo); } // Ox0D
void sharpsm83::rrc_memhl() {rrcmem_param(HL.b0_15);} // Ox0E
void sharpsm83::rrc_a() { rrc_param(AF.Hi); } // Ox0F

void sharpsm83::rl_b() { rl_param(BC.Hi); } // Ox10
void sharpsm83::rl_c() { rl_param(BC.Lo); } // Ox11
void sharpsm83::rl_d() { rl_param(DE.Hi); } // Ox12
void sharpsm83::rl_e() { rl_param(DE.Lo); } // Ox13
void sharpsm83::rl_h() { rl_param(HL.Hi); } // Ox14
void sharpsm83::rl_l() { rl_param(HL.Lo); } // Ox15
void sharpsm83::rl_memhl() { rlmem_param(HL.b0_15); } // Ox16
void sharpsm83::rl_a() { rl_param(AF.Hi); } // Ox17
void sharpsm83::rr_b() { rr_param(BC.Hi); } // Ox18
void sharpsm83::rr_c() { rr_param(BC.Lo); } // Ox19
void sharpsm83::rr_d() { rr_param(DE.Hi); } // Ox1A
void sharpsm83::rr_e() { rr_param(DE.Lo); } // Ox1B
void sharpsm83::rr_h() { rr_param(HL.Hi); } // Ox1C
void sharpsm83::rr_l() { rr_param(HL.Lo); } // Ox1D
void sharpsm83::rr_memhl() { rrmem_param(HL.b0_15); } // Ox1E
void sharpsm83::rr_a() { rr_param(AF.Hi); } // Ox1F

void sharpsm83::sla_b() { sla_param(BC.Hi); } // Ox20
void sharpsm83::sla_c() { sla_param(BC.Lo); } // Ox21
void sharpsm83::sla_d() { sla_param(DE.Hi); } // Ox22
void sharpsm83::sla_e() { sla_param(DE.Lo); } // Ox23
void sharpsm83::sla_h() { sla_param(HL.Hi); } // Ox24
void sharpsm83::sla_l() { sla_param(HL.Lo); } // Ox25
void sharpsm83::sla_memhl() { slamem_param(HL.b0_15); } // Ox26
void sharpsm83::sla_a() { sla_param(AF.Hi); } // Ox27

void sharpsm83::sra_b() { sra_param(BC.Hi); } // Ox28
void sharpsm83::sra_c() { sra_param(BC.Lo); } // Ox29
void sharpsm83::sra_d() { sra_param(DE.Hi); } // Ox2A
void sharpsm83::sra_e() { sra_param(DE.Lo); } // Ox2B
void sharpsm83::sra_h() { sra_param(HL.Hi); } // Ox2C
void sharpsm83::sra_l() { sra_param(HL.Lo); } // Ox2D
void sharpsm83::sra_memhl() {sramem_param(HL.b0_15);} // Ox2E
void sharpsm83::sra_a() { sra_param(AF.Hi); } // Ox2F

void sharpsm83::swap_b() { swap_param(BC.Hi); } // Ox30
void sharpsm83::swap_c() { swap_param(BC.Lo); } // Ox31
void sharpsm83::swap_d() { swap_param(DE.Hi); } // Ox32
void sharpsm83::swap_e() { swap_param(DE.Lo); } // Ox33
void sharpsm83::swap_h() { swap_param(HL.Hi); } // Ox34
void sharpsm83::swap_l() { swap_param(HL.Lo); } // Ox35
void sharpsm83::swap_memhl() { swapmem_param(HL.b0_15); } // Ox36
void sharpsm83::swap_a() { swap_param(AF.Hi); } // Ox37

void sharpsm83::srl_b() { srl_param(BC.Hi); } // Ox38
void sharpsm83::srl_c() { srl_param(BC.Lo); } // Ox39
void sharpsm83::srl_d() { srl_param(DE.Hi); } // Ox3A
void sharpsm83::srl_e() { srl_param(DE.Lo); } // Ox3B
void sharpsm83::srl_h() { srl_param(HL.Hi); } // Ox3C
void sharpsm83::srl_l() { srl_param(HL.Lo); } // Ox3D
void sharpsm83::srl_memhl() { srlmem_param(HL.b0_15); } // Ox3E
void sharpsm83::srl_a() { srl_param(AF.Hi); } // Ox3F

void sharpsm83::bit_0_b() { bit_param(0, BC.Hi); } // Ox40
void sharpsm83::bit_0_c() { bit_param(0, BC.Lo); } // Ox41
void sharpsm83::bit_0_d() { bit_param(0, DE.Hi); } // Ox42
void sharpsm83::bit_0_e() { bit_param(0, DE.Lo); } // Ox43
void sharpsm83::bit_0_h() { bit_param(0, HL.Hi); } // Ox44
void sharpsm83::bit_0_l() { bit_param(0, HL.Lo); } // Ox45
void sharpsm83::bit_0_memhl() { bitmem_param(0, HL.b0_15); } // Ox46
void sharpsm83::bit_0_a() { bit_param(0, AF.Hi); } // Ox47

void sharpsm83::bit_1_b() { bit_param(1, BC.Hi); } // Ox48
void sharpsm83::bit_1_c() { bit_param(1, BC.Lo); } // Ox49
void sharpsm83::bit_1_d() { bit_param(1, DE.Hi); } // Ox4A
void sharpsm83::bit_1_e() { bit_param(1, DE.Lo); } // Ox4B
void sharpsm83::bit_1_h() { bit_param(1, HL.Hi); } // Ox4C
void sharpsm83::bit_1_l() { bit_param(1, HL.Lo); } // Ox4D
void sharpsm83::bit_1_memhl() { bitmem_param(1, HL.b0_15); } // Ox4E
void sharpsm83::bit_1_a() { bit_param(1, AF.Hi); } // Ox4F

void sharpsm83::bit_2_b() { bit_param(2, BC.Hi); } // Ox50
void sharpsm83::bit_2_c() { bit_param(2, BC.Lo); } // Ox51
void sharpsm83::bit_2_d() { bit_param(2, DE.Hi); } // Ox52
void sharpsm83::bit_2_e() { bit_param(2, DE.Lo); } // Ox53
void sharpsm83::bit_2_h() { bit_param(2, HL.Hi); } // Ox54
void sharpsm83::bit_2_l() { bit_param(2, HL.Lo); } // Ox55
void sharpsm83::bit_2_memhl() { bitmem_param(2, HL.b0_15); } // Ox56
void sharpsm83::bit_2_a() { bit_param(2, AF.Hi); } // Ox57

void sharpsm83::bit_3_b() { bit_param(3, BC.Hi); } // Ox58
void sharpsm83::bit_3_c() { bit_param(3, BC.Lo); } // Ox59
void sharpsm83::bit_3_d() { bit_param(3, DE.Hi); } // Ox5A
void sharpsm83::bit_3_e() { bit_param(3, DE.Lo); } // Ox5B
void sharpsm83::bit_3_h() { bit_param(3, HL.Hi); } // Ox5C
void sharpsm83::bit_3_l() { bit_param(3, HL.Lo); } // Ox5D
void sharpsm83::bit_3_memhl() { bitmem_param(3, HL.b0_15); } // Ox5E
void sharpsm83::bit_3_a() { bit_param(3, AF.Hi); } // Ox5F

void sharpsm83::bit_4_b() { bit_param(4, BC.Hi); } // Ox60
void sharpsm83::bit_4_c() { bit_param(4, BC.Lo); } // Ox61
void sharpsm83::bit_4_d() { bit_param(4, DE.Hi); } // Ox62
void sharpsm83::bit_4_e() { bit_param(4, DE.Lo); } // Ox63
void sharpsm83::bit_4_h() { bit_param(4, HL.Hi); } // Ox64
void sharpsm83::bit_4_l() { bit_param(4, HL.Lo); } // Ox65
void sharpsm83::bit_4_memhl() { bitmem_param(4, HL.b0_15); } // Ox66
void sharpsm83::bit_4_a() { bit_param(4, AF.Hi); } // Ox67

void sharpsm83::bit_5_b() { bit_param(5, BC.Hi); } // Ox68
void sharpsm83::bit_5_c() { bit_param(5, BC.Lo); } // Ox69
void sharpsm83::bit_5_d() { bit_param(5, DE.Hi); } // Ox6A
void sharpsm83::bit_5_e() { bit_param(5, DE.Lo); } // Ox6B
void sharpsm83::bit_5_h() { bit_param(5, HL.Hi); } // Ox6C
void sharpsm83::bit_5_l() { bit_param(5, HL.Lo); } // Ox6D
void sharpsm83::bit_5_memhl() { bitmem_param(5, HL.b0_15); } // Ox6E
void sharpsm83::bit_5_a() { bit_param(5, AF.Hi); } // Ox6F

void sharpsm83::bit_6_b() { bit_param(6, BC.Hi); } // Ox70
void sharpsm83::bit_6_c() { bit_param(6, BC.Lo); } // Ox71
void sharpsm83::bit_6_d() { bit_param(6, DE.Hi); } // Ox72
void sharpsm83::bit_6_e() { bit_param(6, DE.Lo); } // Ox73
void sharpsm83::bit_6_h() { bit_param(6, HL.Hi); } // Ox74
void sharpsm83::bit_6_l() { bit_param(6, HL.Lo); } // Ox75
void sharpsm83::bit_6_memhl() { bitmem_param(6, HL.b0_15); } // Ox76
void sharpsm83::bit_6_a() { bit_param(6, AF.Hi); } // Ox77

void sharpsm83::bit_7_b() { bit_param(7, BC.Hi); } // Ox78
void sharpsm83::bit_7_c() { bit_param(7, BC.Lo); } // Ox79
void sharpsm83::bit_7_d() { bit_param(7, DE.Hi); } // Ox7A
void sharpsm83::bit_7_e() { bit_param(7, DE.Lo); } // Ox7B
void sharpsm83::bit_7_h() { bit_param(7, HL.Hi); } // Ox7C
void sharpsm83::bit_7_l() { bit_param(7, HL.Lo); } // Ox7D
void sharpsm83::bit_7_memhl() { bitmem_param(7, HL.b0_15); } // Ox7E
void sharpsm83::bit_7_a() { bit_param(7, AF.Hi); } // Ox7F

void sharpsm83::res_0_b() { res_param(0, BC.Hi); } // Ox80
void sharpsm83::res_0_c() { res_param(0, BC.Lo); } // Ox81
void sharpsm83::res_0_d() { res_param(0, DE.Hi); } // Ox82
void sharpsm83::res_0_e() { res_param(0, DE.Lo); } // Ox83
void sharpsm83::res_0_h() { res_param(0, HL.Hi); } // Ox84
void sharpsm83::res_0_l() { res_param(0, HL.Lo); } // Ox85
void sharpsm83::res_0_memhl() { resmem_param(0, HL.b0_15); } // Ox86
void sharpsm83::res_0_a() { res_param(0, AF.Hi); } // Ox87
void sharpsm83::res_1_b() { res_param(1, BC.Hi); } // Ox88
void sharpsm83::res_1_c() { res_param(1, BC.Lo); } // Ox89
void sharpsm83::res_1_d() { res_param(1, DE.Hi); } // Ox8A
void sharpsm83::res_1_e() { res_param(1, DE.Lo); } // Ox8B
void sharpsm83::res_1_h() { res_param(1, HL.Hi); } // Ox8C
void sharpsm83::res_1_l() { res_param(1, HL.Lo); } // Ox8D
void sharpsm83::res_1_memhl() { resmem_param(1, HL.b0_15); } // Ox8E
void sharpsm83::res_1_a() { res_param(1, AF.Hi); } // Ox8F
void sharpsm83::res_2_b() { res_param(2, BC.Hi); } // Ox90
void sharpsm83::res_2_c() { res_param(2, BC.Lo); } // Ox91
void sharpsm83::res_2_d() { res_param(2, DE.Hi); } // Ox92
void sharpsm83::res_2_e() { res_param(2, DE.Lo); } // Ox93
void sharpsm83::res_2_h() { res_param(2, HL.Hi); } // Ox94
void sharpsm83::res_2_l() { res_param(2, HL.Lo); } // Ox95
void sharpsm83::res_2_memhl() { resmem_param(2, HL.b0_15); } // Ox96
void sharpsm83::res_2_a() { res_param(2, AF.Hi); } // Ox97
void sharpsm83::res_3_b() { res_param(3, BC.Hi); } // Ox98
void sharpsm83::res_3_c() { res_param(3, BC.Lo); } // Ox99
void sharpsm83::res_3_d() { res_param(3, DE.Hi); } // Ox9A
void sharpsm83::res_3_e() { res_param(3, DE.Lo); } // Ox9B
void sharpsm83::res_3_h() { res_param(3, HL.Hi); } // Ox9C
void sharpsm83::res_3_l() { res_param(3, HL.Lo); } // Ox9D
void sharpsm83::res_3_memhl() { resmem_param(3, HL.b0_15); } // Ox9E
void sharpsm83::res_3_a() { res_param(3, AF.Hi); } // Ox9F
void sharpsm83::res_4_b() { res_param(4, BC.Hi); } // OxA0
void sharpsm83::res_4_c() { res_param(4, BC.Lo); } // OxA1
void sharpsm83::res_4_d() { res_param(4, DE.Hi); } // OxA2
void sharpsm83::res_4_e() { res_param(4, DE.Lo); } // OxA3
void sharpsm83::res_4_h() { res_param(4, HL.Hi); } // OxA4
void sharpsm83::res_4_l() { res_param(4, HL.Lo); } // OxA5
void sharpsm83::res_4_memhl() { resmem_param(4, HL.b0_15); } // OxA6
void sharpsm83::res_4_a() { res_param(4, AF.Hi); } // OxA7
void sharpsm83::res_5_b() { res_param(5, BC.Hi); } // OxA8
void sharpsm83::res_5_c() { res_param(5, BC.Lo); } // OxA9
void sharpsm83::res_5_d() { res_param(5, DE.Hi); } // OxAA
void sharpsm83::res_5_e() { res_param(5, DE.Lo); } // OxAB
void sharpsm83::res_5_h() { res_param(5, HL.Hi); } // OxAC
void sharpsm83::res_5_l() { res_param(5, HL.Lo); } // OxAD
void sharpsm83::res_5_memhl() { resmem_param(5, HL.b0_15); } // OxAE
void sharpsm83::res_5_a() { res_param(5, AF.Hi); } // OxAF
void sharpsm83::res_6_b() { res_param(6, BC.Hi); } // OxB0
void sharpsm83::res_6_c() { res_param(6, BC.Lo); } // OxB1
void sharpsm83::res_6_d() { res_param(6, DE.Hi); } // OxB2
void sharpsm83::res_6_e() { res_param(6, DE.Lo); } // OxB3
void sharpsm83::res_6_h() { res_param(6, HL.Hi); } // OxB4
void sharpsm83::res_6_l() { res_param(6, HL.Lo); } // OxB5
void sharpsm83::res_6_memhl() { resmem_param(6, HL.b0_15); } // OxB6
void sharpsm83::res_6_a() { res_param(6, AF.Hi); } // OxB7
void sharpsm83::res_7_b() { res_param(7, BC.Hi); } // OxB8
void sharpsm83::res_7_c() { res_param(7, BC.Lo); } // OxB9
void sharpsm83::res_7_d() { res_param(7, DE.Hi); } // OxBA
void sharpsm83::res_7_e() { res_param(7, DE.Lo); } // OxBB
void sharpsm83::res_7_h() { res_param(7, HL.Hi); } // OxBC
void sharpsm83::res_7_l() { res_param(7, HL.Lo); } // OxBD
void sharpsm83::res_7_memhl() { resmem_param(7, HL.b0_15); } // OxBE
void sharpsm83::res_7_a() { res_param(7, AF.Hi); } // OxBF
void sharpsm83::set_0_b() { set_param(0, BC.Hi); } // OxC0
void sharpsm83::set_0_c() { set_param(0, BC.Lo); } // OxC1
void sharpsm83::set_0_d() { set_param(0, DE.Hi); } // OxC2
void sharpsm83::set_0_e() { set_param(0, DE.Lo); } // OxC3
void sharpsm83::set_0_h() { set_param(0, HL.Hi); } // OxC4
void sharpsm83::set_0_l() { set_param(0, HL.Lo); } // OxC5
void sharpsm83::set_0_memhl() { setmem_param(0, HL.b0_15); } // OxC6
void sharpsm83::set_0_a() { set_param(0, AF.Hi); } // OxC7
void sharpsm83::set_1_b() { set_param(1, BC.Hi); } // OxC8
void sharpsm83::set_1_c() { set_param(1, BC.Lo); } // OxC9
void sharpsm83::set_1_d() { set_param(1, DE.Hi); } // OxCA
void sharpsm83::set_1_e() { set_param(1, DE.Lo); } // OxCB
void sharpsm83::set_1_h() { set_param(1, HL.Hi); } // OxCC
void sharpsm83::set_1_l() { set_param(1, HL.Lo); } // OxCD
void sharpsm83::set_1_memhl() { setmem_param(1, HL.b0_15); } // OxCE
void sharpsm83::set_1_a() { set_param(1, AF.Hi); } // OxCF
void sharpsm83::set_2_b() { set_param(2, BC.Hi); } // OxD0
void sharpsm83::set_2_c() { set_param(2, BC.Lo); } // OxD1
void sharpsm83::set_2_d() { set_param(2, DE.Hi); } // OxD2
void sharpsm83::set_2_e() { set_param(2, DE.Lo); } // OxD3
void sharpsm83::set_2_h() { set_param(2, HL.Hi); } // OxD4
void sharpsm83::set_2_l() { set_param(2, HL.Lo); } // OxD5
void sharpsm83::set_2_memhl() { setmem_param(2, HL.b0_15); } // OxD6
void sharpsm83::set_2_a() { set_param(2, AF.Hi); } // OxD7
void sharpsm83::set_3_b() { set_param(3, BC.Hi); } // OxD8
void sharpsm83::set_3_c() { set_param(3, BC.Lo); } // OxD9
void sharpsm83::set_3_d() { set_param(3, DE.Hi); } // OxDA
void sharpsm83::set_3_e() { set_param(3, DE.Lo); } // OxDB
void sharpsm83::set_3_h() { set_param(3, HL.Hi); } // OxDC
void sharpsm83::set_3_l() { set_param(3, HL.Lo); } // OxDD
void sharpsm83::set_3_memhl() { setmem_param(3, HL.b0_15); } // OxDE
void sharpsm83::set_3_a() { set_param(3, AF.Hi); } // OxDF
void sharpsm83::set_4_b() { set_param(4, BC.Hi); } // OxE0
void sharpsm83::set_4_c() { set_param(4, BC.Lo); } // OxE1
void sharpsm83::set_4_d() { set_param(4, DE.Hi); } // OxE2
void sharpsm83::set_4_e() { set_param(4, DE.Lo); } // OxE3
void sharpsm83::set_4_h() { set_param(4, HL.Hi); } // OxE4
void sharpsm83::set_4_l() { set_param(4, HL.Lo); } // OxE5
void sharpsm83::set_4_memhl() { setmem_param(4, HL.b0_15); } // OxE6
void sharpsm83::set_4_a() { set_param(4, AF.Hi); } // OxE7
void sharpsm83::set_5_b() { set_param(5, BC.Hi); } // OxE8
void sharpsm83::set_5_c() { set_param(5, BC.Lo); } // OxE9
void sharpsm83::set_5_d() { set_param(5, DE.Hi); } // OxEA
void sharpsm83::set_5_e() { set_param(5, DE.Lo); } // OxEB
void sharpsm83::set_5_h() { set_param(5, HL.Hi); } // OxEC
void sharpsm83::set_5_l() { set_param(5, HL.Lo); } // OxED
void sharpsm83::set_5_memhl() { setmem_param(5, HL.b0_15); } // OxEE
void sharpsm83::set_5_a() { set_param(5, AF.Hi); } // OxEF
void sharpsm83::set_6_b() { set_param(6, BC.Hi); } // OxF0
void sharpsm83::set_6_c() { set_param(6, BC.Lo); } // OxF1
void sharpsm83::set_6_d() { set_param(6, DE.Hi); } // OxF2
void sharpsm83::set_6_e() { set_param(6, DE.Lo); } // OxF3
void sharpsm83::set_6_h() { set_param(6, HL.Hi); } // OxF4
void sharpsm83::set_6_l() { set_param(6, HL.Lo); } // OxF5
void sharpsm83::set_6_memhl() { setmem_param(6, HL.b0_15); } // OxF6
void sharpsm83::set_6_a() { set_param(6, AF.Hi); } // OxF7
void sharpsm83::set_7_b() { set_param(7, BC.Hi); } // OxF8
void sharpsm83::set_7_c() { set_param(7, BC.Lo); } // OxF9
void sharpsm83::set_7_d() { set_param(7, DE.Hi); } // OxFA
void sharpsm83::set_7_e() { set_param(7, DE.Lo); } // OxFB
void sharpsm83::set_7_h() { set_param(7, HL.Hi); } // OxFC
void sharpsm83::set_7_l() { set_param(7, HL.Lo); } // OxFD
void sharpsm83::set_7_memhl() { setmem_param(7, HL.b0_15); } // OxFE
void sharpsm83::set_7_a() { set_param(7, AF.Hi); } // OxFF



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
    opcode_table[0x1E] = std::bind(&sharpsm83::ld_e_imm8, this);
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

    opcode_table[0xF0] = std::bind(&sharpsm83::ldh_a_memimm8, this);
    opcode_table[0xF1] = std::bind(&sharpsm83::pop_af, this);
    opcode_table[0xF2] = std::bind(&sharpsm83::ldh_a_memc, this);
    opcode_table[0xF3] = std::bind(&sharpsm83::di, this);
    opcode_table[0xF4] = std::bind(&sharpsm83::op_0xF4, this);
    opcode_table[0xF5] = std::bind(&sharpsm83::push_af, this);
    opcode_table[0xF6] = std::bind(&sharpsm83::or_a_imm8, this);
    opcode_table[0xF7] = std::bind(&sharpsm83::rst_0x30, this);
    opcode_table[0xF8] = std::bind(&sharpsm83::ld_hl_sp_e8, this);
    opcode_table[0xF9] = std::bind(&sharpsm83::ld_sp_hl, this);
    opcode_table[0xFA] = std::bind(&sharpsm83::ld_a_memimm16, this);
    opcode_table[0xFB] = std::bind(&sharpsm83::ei, this);
    opcode_table[0xFC] = std::bind(&sharpsm83::op_0xFC, this);
    opcode_table[0xFD] = std::bind(&sharpsm83::op_0xFD, this);
    opcode_table[0xFE] = std::bind(&sharpsm83::cp_a_imm8, this);
    opcode_table[0xFF] = std::bind(&sharpsm83::rst_0x38, this); 

}

void sharpsm83::initialize_cbopcodes() 
{
    CB_opcode_table[0x00] = std::bind(&sharpsm83::rlc_b, this);
    CB_opcode_table[0x01] = std::bind(&sharpsm83::rlc_c, this);
    CB_opcode_table[0x02] = std::bind(&sharpsm83::rlc_d, this);
    CB_opcode_table[0x03] = std::bind(&sharpsm83::rlc_e, this);
    CB_opcode_table[0x04] = std::bind(&sharpsm83::rlc_h, this);
    CB_opcode_table[0x05] = std::bind(&sharpsm83::rlc_l, this);
    CB_opcode_table[0x06] = std::bind(&sharpsm83::rlc_memhl, this);
    CB_opcode_table[0x07] = std::bind(&sharpsm83::rlc_a, this);
    CB_opcode_table[0x08] = std::bind(&sharpsm83::rrc_b, this);
    CB_opcode_table[0x09] = std::bind(&sharpsm83::rrc_c, this);
    CB_opcode_table[0x0A] = std::bind(&sharpsm83::rrc_d, this);
    CB_opcode_table[0x0B] = std::bind(&sharpsm83::rrc_e, this);
    CB_opcode_table[0x0C] = std::bind(&sharpsm83::rrc_h, this);
    CB_opcode_table[0x0D] = std::bind(&sharpsm83::rrc_l, this);
    CB_opcode_table[0x0E] = std::bind(&sharpsm83::rrc_memhl, this);
    CB_opcode_table[0x0F] = std::bind(&sharpsm83::rrc_a, this);

    CB_opcode_table[0x10] = std::bind(&sharpsm83::rl_b, this);
    CB_opcode_table[0x11] = std::bind(&sharpsm83::rl_c, this);
    CB_opcode_table[0x12] = std::bind(&sharpsm83::rl_d, this);
    CB_opcode_table[0x13] = std::bind(&sharpsm83::rl_e, this);
    CB_opcode_table[0x14] = std::bind(&sharpsm83::rl_h, this);
    CB_opcode_table[0x15] = std::bind(&sharpsm83::rl_l, this);
    CB_opcode_table[0x16] = std::bind(&sharpsm83::rl_memhl, this);
    CB_opcode_table[0x17] = std::bind(&sharpsm83::rl_a, this);
    CB_opcode_table[0x18] = std::bind(&sharpsm83::rr_b, this);
    CB_opcode_table[0x19] = std::bind(&sharpsm83::rr_c, this);
    CB_opcode_table[0x1A] = std::bind(&sharpsm83::rr_d, this);
    CB_opcode_table[0x1B] = std::bind(&sharpsm83::rr_e, this);
 
    CB_opcode_table[0x1C] = std::bind(&sharpsm83::rr_h, this);
    CB_opcode_table[0x1D] = std::bind(&sharpsm83::rr_l, this);
    CB_opcode_table[0x1E] = std::bind(&sharpsm83::rr_memhl, this);
    CB_opcode_table[0x1F] = std::bind(&sharpsm83::rr_a, this);

    CB_opcode_table[0x20] = std::bind(&sharpsm83::sla_b, this);
    CB_opcode_table[0x21] = std::bind(&sharpsm83::sla_c, this);
    CB_opcode_table[0x22] = std::bind(&sharpsm83::sla_d, this);
    CB_opcode_table[0x23] = std::bind(&sharpsm83::sla_e, this);
    CB_opcode_table[0x24] = std::bind(&sharpsm83::sla_h, this);
    CB_opcode_table[0x25] = std::bind(&sharpsm83::sla_l, this);
    CB_opcode_table[0x26] = std::bind(&sharpsm83::sla_memhl, this);
    CB_opcode_table[0x27] = std::bind(&sharpsm83::sla_a, this);
    CB_opcode_table[0x28] = std::bind(&sharpsm83::sra_b, this);
    CB_opcode_table[0x29] = std::bind(&sharpsm83::sra_c, this);
    CB_opcode_table[0x2A] = std::bind(&sharpsm83::sra_d, this);
    CB_opcode_table[0x2B] = std::bind(&sharpsm83::sra_e, this);
    CB_opcode_table[0x2C] = std::bind(&sharpsm83::sra_h, this);
    CB_opcode_table[0x2D] = std::bind(&sharpsm83::sra_l, this);
    CB_opcode_table[0x2E] = std::bind(&sharpsm83::sra_memhl, this);
    CB_opcode_table[0x2F] = std::bind(&sharpsm83::sra_a, this);

    CB_opcode_table[0x30] = std::bind(&sharpsm83::swap_b, this);
    CB_opcode_table[0x31] = std::bind(&sharpsm83::swap_c, this);
    CB_opcode_table[0x32] = std::bind(&sharpsm83::swap_d, this);
    CB_opcode_table[0x33] = std::bind(&sharpsm83::swap_e, this);
    CB_opcode_table[0x34] = std::bind(&sharpsm83::swap_h, this);
    CB_opcode_table[0x35] = std::bind(&sharpsm83::swap_l, this);
    CB_opcode_table[0x36] = std::bind(&sharpsm83::swap_memhl, this);
    CB_opcode_table[0x37] = std::bind(&sharpsm83::swap_a, this);
    CB_opcode_table[0x38] = std::bind(&sharpsm83::srl_b, this);
    CB_opcode_table[0x39] = std::bind(&sharpsm83::srl_c, this);
    CB_opcode_table[0x3A] = std::bind(&sharpsm83::srl_d, this);
    CB_opcode_table[0x3B] = std::bind(&sharpsm83::srl_e, this);
    CB_opcode_table[0x3C] = std::bind(&sharpsm83::srl_h, this);
    CB_opcode_table[0x3D] = std::bind(&sharpsm83::srl_l, this);
    CB_opcode_table[0x3E] = std::bind(&sharpsm83::srl_memhl, this);
    CB_opcode_table[0x3F] = std::bind(&sharpsm83::srl_a, this);

    CB_opcode_table[0x40] = std::bind(&sharpsm83::bit_0_b, this);
    CB_opcode_table[0x41] = std::bind(&sharpsm83::bit_0_c, this);
    CB_opcode_table[0x42] = std::bind(&sharpsm83::bit_0_d, this);
    CB_opcode_table[0x43] = std::bind(&sharpsm83::bit_0_e, this);
    CB_opcode_table[0x44] = std::bind(&sharpsm83::bit_0_h, this);
    CB_opcode_table[0x45] = std::bind(&sharpsm83::bit_0_l, this);
    CB_opcode_table[0x46] = std::bind(&sharpsm83::bit_0_memhl, this);
    CB_opcode_table[0x47] = std::bind(&sharpsm83::bit_0_a, this);
    CB_opcode_table[0x48] = std::bind(&sharpsm83::bit_1_b, this);
    CB_opcode_table[0x49] = std::bind(&sharpsm83::bit_1_c, this);
    CB_opcode_table[0x4A] = std::bind(&sharpsm83::bit_1_d, this);
    CB_opcode_table[0x4B] = std::bind(&sharpsm83::bit_1_e, this);
    CB_opcode_table[0x4C] = std::bind(&sharpsm83::bit_1_h, this);
    CB_opcode_table[0x4D] = std::bind(&sharpsm83::bit_1_l, this);
    CB_opcode_table[0x4E] = std::bind(&sharpsm83::bit_1_memhl, this);
    CB_opcode_table[0x4F] = std::bind(&sharpsm83::bit_1_a, this);

    CB_opcode_table[0x50] = std::bind(&sharpsm83::bit_2_b, this);
    CB_opcode_table[0x51] = std::bind(&sharpsm83::bit_2_c, this);
    CB_opcode_table[0x52] = std::bind(&sharpsm83::bit_2_d, this);
    CB_opcode_table[0x53] = std::bind(&sharpsm83::bit_2_e, this);
    CB_opcode_table[0x54] = std::bind(&sharpsm83::bit_2_h, this);
    CB_opcode_table[0x55] = std::bind(&sharpsm83::bit_2_l, this);
    CB_opcode_table[0x56] = std::bind(&sharpsm83::bit_2_memhl, this);
    CB_opcode_table[0x57] = std::bind(&sharpsm83::bit_2_a, this);
    CB_opcode_table[0x58] = std::bind(&sharpsm83::bit_3_b, this);
    CB_opcode_table[0x59] = std::bind(&sharpsm83::bit_3_c, this);
    CB_opcode_table[0x5A] = std::bind(&sharpsm83::bit_3_d, this);
    CB_opcode_table[0x5B] = std::bind(&sharpsm83::bit_3_e, this);
    CB_opcode_table[0x5C] = std::bind(&sharpsm83::bit_3_h, this);
    CB_opcode_table[0x5D] = std::bind(&sharpsm83::bit_3_l, this);
    CB_opcode_table[0x5E] = std::bind(&sharpsm83::bit_3_memhl, this);
    CB_opcode_table[0x5F] = std::bind(&sharpsm83::bit_3_a, this);

    CB_opcode_table[0x60] = std::bind(&sharpsm83::bit_4_b, this);
    CB_opcode_table[0x61] = std::bind(&sharpsm83::bit_4_c, this);
    CB_opcode_table[0x62] = std::bind(&sharpsm83::bit_4_d, this);
    CB_opcode_table[0x63] = std::bind(&sharpsm83::bit_4_e, this);
    CB_opcode_table[0x64] = std::bind(&sharpsm83::bit_4_h, this);
    CB_opcode_table[0x65] = std::bind(&sharpsm83::bit_4_l, this);
    CB_opcode_table[0x66] = std::bind(&sharpsm83::bit_4_memhl, this);
    CB_opcode_table[0x67] = std::bind(&sharpsm83::bit_4_a, this);
    CB_opcode_table[0x68] = std::bind(&sharpsm83::bit_5_b, this);
    CB_opcode_table[0x69] = std::bind(&sharpsm83::bit_5_c, this);
    CB_opcode_table[0x6A] = std::bind(&sharpsm83::bit_5_d, this);
    CB_opcode_table[0x6B] = std::bind(&sharpsm83::bit_5_e, this);
    CB_opcode_table[0x6C] = std::bind(&sharpsm83::bit_5_h, this);
    CB_opcode_table[0x6D] = std::bind(&sharpsm83::bit_5_l, this);
    CB_opcode_table[0x6E] = std::bind(&sharpsm83::bit_5_memhl, this);
    CB_opcode_table[0x6F] = std::bind(&sharpsm83::bit_5_a, this);

    CB_opcode_table[0x70] = std::bind(&sharpsm83::bit_6_b, this);
    CB_opcode_table[0x71] = std::bind(&sharpsm83::bit_6_c, this);
    CB_opcode_table[0x72] = std::bind(&sharpsm83::bit_6_d, this);
    CB_opcode_table[0x73] = std::bind(&sharpsm83::bit_6_e, this);
    CB_opcode_table[0x74] = std::bind(&sharpsm83::bit_6_h, this);
    CB_opcode_table[0x75] = std::bind(&sharpsm83::bit_6_l, this);
    CB_opcode_table[0x76] = std::bind(&sharpsm83::bit_6_memhl, this);
    CB_opcode_table[0x77] = std::bind(&sharpsm83::bit_6_a, this);
    CB_opcode_table[0x78] = std::bind(&sharpsm83::bit_7_b, this);
    CB_opcode_table[0x79] = std::bind(&sharpsm83::bit_7_c, this);
    CB_opcode_table[0x7A] = std::bind(&sharpsm83::bit_7_d, this);
    CB_opcode_table[0x7B] = std::bind(&sharpsm83::bit_7_e, this);
    CB_opcode_table[0x7C] = std::bind(&sharpsm83::bit_7_h, this);
    CB_opcode_table[0x7D] = std::bind(&sharpsm83::bit_7_l, this);
    CB_opcode_table[0x7E] = std::bind(&sharpsm83::bit_7_memhl, this);
    CB_opcode_table[0x7F] = std::bind(&sharpsm83::bit_7_a, this);

    CB_opcode_table[0x80] = std::bind(&sharpsm83::res_0_b, this);
    CB_opcode_table[0x81] = std::bind(&sharpsm83::res_0_c, this);
    CB_opcode_table[0x82] = std::bind(&sharpsm83::res_0_d, this);
    CB_opcode_table[0x83] = std::bind(&sharpsm83::res_0_e, this);
    CB_opcode_table[0x84] = std::bind(&sharpsm83::res_0_h, this);
    CB_opcode_table[0x85] = std::bind(&sharpsm83::res_0_l, this);
    CB_opcode_table[0x86] = std::bind(&sharpsm83::res_0_memhl, this);
    CB_opcode_table[0x87] = std::bind(&sharpsm83::res_0_a, this); 
    CB_opcode_table[0x88] = std::bind(&sharpsm83::res_1_b, this);
    CB_opcode_table[0x89] = std::bind(&sharpsm83::res_1_c, this);
    CB_opcode_table[0x8A] = std::bind(&sharpsm83::res_1_d, this);
    CB_opcode_table[0x8B] = std::bind(&sharpsm83::res_1_e, this);
    CB_opcode_table[0x8C] = std::bind(&sharpsm83::res_1_h, this);
    CB_opcode_table[0x8D] = std::bind(&sharpsm83::res_1_l, this);
    CB_opcode_table[0x8E] = std::bind(&sharpsm83::res_1_memhl, this);
    CB_opcode_table[0x8F] = std::bind(&sharpsm83::res_1_a, this);

    CB_opcode_table[0x90] = std::bind(&sharpsm83::res_2_b, this);
    CB_opcode_table[0x91] = std::bind(&sharpsm83::res_2_c, this);
    CB_opcode_table[0x92] = std::bind(&sharpsm83::res_2_d, this);
    CB_opcode_table[0x93] = std::bind(&sharpsm83::res_2_e, this);
    CB_opcode_table[0x94] = std::bind(&sharpsm83::res_2_h, this);
    CB_opcode_table[0x95] = std::bind(&sharpsm83::res_2_l, this);
    CB_opcode_table[0x96] = std::bind(&sharpsm83::res_2_memhl, this);
    CB_opcode_table[0x97] = std::bind(&sharpsm83::res_2_a, this);
    CB_opcode_table[0x98] = std::bind(&sharpsm83::res_3_b, this);
    CB_opcode_table[0x99] = std::bind(&sharpsm83::res_3_c, this);
    CB_opcode_table[0x9A] = std::bind(&sharpsm83::res_3_d, this);
    CB_opcode_table[0x9B] = std::bind(&sharpsm83::res_3_e, this);
    CB_opcode_table[0x9C] = std::bind(&sharpsm83::res_3_h, this);
    CB_opcode_table[0x9D] = std::bind(&sharpsm83::res_3_l, this);
    CB_opcode_table[0x9E] = std::bind(&sharpsm83::res_3_memhl, this);
    CB_opcode_table[0x9F] = std::bind(&sharpsm83::res_3_a, this);

    CB_opcode_table[0xA0] = std::bind(&sharpsm83::res_4_b, this);
    CB_opcode_table[0xA1] = std::bind(&sharpsm83::res_4_c, this);
    CB_opcode_table[0xA2] = std::bind(&sharpsm83::res_4_d, this);
    CB_opcode_table[0xA3] = std::bind(&sharpsm83::res_4_e, this);
    CB_opcode_table[0xA4] = std::bind(&sharpsm83::res_4_h, this);
    CB_opcode_table[0xA5] = std::bind(&sharpsm83::res_4_l, this);
    CB_opcode_table[0xA6] = std::bind(&sharpsm83::res_4_memhl, this);
    CB_opcode_table[0xA7] = std::bind(&sharpsm83::res_4_a, this);
    CB_opcode_table[0xA8] = std::bind(&sharpsm83::res_5_b, this);
    CB_opcode_table[0xA9] = std::bind(&sharpsm83::res_5_c, this);
    CB_opcode_table[0xAA] = std::bind(&sharpsm83::res_5_d, this);
    CB_opcode_table[0xAB] = std::bind(&sharpsm83::res_5_e, this);
    CB_opcode_table[0xAC] = std::bind(&sharpsm83::res_5_h, this);
    CB_opcode_table[0xAD] = std::bind(&sharpsm83::res_5_l, this);
    CB_opcode_table[0xAE] = std::bind(&sharpsm83::res_5_memhl, this);
    CB_opcode_table[0xAF] = std::bind(&sharpsm83::res_5_a, this);

    CB_opcode_table[0xB0] = std::bind(&sharpsm83::res_6_b, this);
    CB_opcode_table[0xB1] = std::bind(&sharpsm83::res_6_c, this);
    CB_opcode_table[0xB2] = std::bind(&sharpsm83::res_6_d, this);
    CB_opcode_table[0xB3] = std::bind(&sharpsm83::res_6_e, this);
    CB_opcode_table[0xB4] = std::bind(&sharpsm83::res_6_h, this);
    CB_opcode_table[0xB5] = std::bind(&sharpsm83::res_6_l, this);
    CB_opcode_table[0xB6] = std::bind(&sharpsm83::res_6_memhl, this);
    CB_opcode_table[0xB7] = std::bind(&sharpsm83::res_6_a, this);
    CB_opcode_table[0xB8] = std::bind(&sharpsm83::res_7_b, this);
    CB_opcode_table[0xB9] = std::bind(&sharpsm83::res_7_c, this);
    CB_opcode_table[0xBA] = std::bind(&sharpsm83::res_7_d, this);
    CB_opcode_table[0xBB] = std::bind(&sharpsm83::res_7_e, this);
    CB_opcode_table[0xBC] = std::bind(&sharpsm83::res_7_h, this);
    CB_opcode_table[0xBD] = std::bind(&sharpsm83::res_7_l, this);
    CB_opcode_table[0xBE] = std::bind(&sharpsm83::res_7_memhl, this);
    CB_opcode_table[0xBF] = std::bind(&sharpsm83::res_7_a, this);

    CB_opcode_table[0xC0] = std::bind(&sharpsm83::set_0_b, this);
    CB_opcode_table[0xC1] = std::bind(&sharpsm83::set_0_c, this);
    CB_opcode_table[0xC2] = std::bind(&sharpsm83::set_0_d, this);
    CB_opcode_table[0xC3] = std::bind(&sharpsm83::set_0_e, this);
    CB_opcode_table[0xC4] = std::bind(&sharpsm83::set_0_h, this);
    CB_opcode_table[0xC5] = std::bind(&sharpsm83::set_0_l, this);
    CB_opcode_table[0xC6] = std::bind(&sharpsm83::set_0_memhl, this);
    CB_opcode_table[0xC7] = std::bind(&sharpsm83::set_0_a, this);
    CB_opcode_table[0xC8] = std::bind(&sharpsm83::set_1_b, this);
    CB_opcode_table[0xC9] = std::bind(&sharpsm83::set_1_c, this);
    CB_opcode_table[0xCA] = std::bind(&sharpsm83::set_1_d, this);
    CB_opcode_table[0xCB] = std::bind(&sharpsm83::set_1_e, this);
    CB_opcode_table[0xCC] = std::bind(&sharpsm83::set_1_h, this);
    CB_opcode_table[0xCD] = std::bind(&sharpsm83::set_1_l, this);
    CB_opcode_table[0xCE] = std::bind(&sharpsm83::set_1_memhl, this);
    CB_opcode_table[0xCF] = std::bind(&sharpsm83::set_1_a, this);

    CB_opcode_table[0xD0] = std::bind(&sharpsm83::set_2_b, this);
    CB_opcode_table[0xD1] = std::bind(&sharpsm83::set_2_c, this);
    CB_opcode_table[0xD2] = std::bind(&sharpsm83::set_2_d, this);
    CB_opcode_table[0xD3] = std::bind(&sharpsm83::set_2_e, this);
    CB_opcode_table[0xD4] = std::bind(&sharpsm83::set_2_h, this);
    CB_opcode_table[0xD5] = std::bind(&sharpsm83::set_2_l, this);
    CB_opcode_table[0xD6] = std::bind(&sharpsm83::set_2_memhl, this);
    CB_opcode_table[0xD7] = std::bind(&sharpsm83::set_2_a, this);
    CB_opcode_table[0xD8] = std::bind(&sharpsm83::set_3_b, this);
    CB_opcode_table[0xD9] = std::bind(&sharpsm83::set_3_c, this);
    CB_opcode_table[0xDA] = std::bind(&sharpsm83::set_3_d, this);
    CB_opcode_table[0xDB] = std::bind(&sharpsm83::set_3_e, this);
    CB_opcode_table[0xDC] = std::bind(&sharpsm83::set_3_h, this);
    CB_opcode_table[0xDD] = std::bind(&sharpsm83::set_3_l, this);
    CB_opcode_table[0xDE] = std::bind(&sharpsm83::set_3_memhl, this);
    CB_opcode_table[0xDF] = std::bind(&sharpsm83::set_3_a, this);

    CB_opcode_table[0xE0] = std::bind(&sharpsm83::set_4_b, this);
    CB_opcode_table[0xE1] = std::bind(&sharpsm83::set_4_c, this);
    CB_opcode_table[0xE2] = std::bind(&sharpsm83::set_4_d, this);
    CB_opcode_table[0xE3] = std::bind(&sharpsm83::set_4_e, this);
    CB_opcode_table[0xE4] = std::bind(&sharpsm83::set_4_h, this);
    CB_opcode_table[0xE5] = std::bind(&sharpsm83::set_4_l, this);
    CB_opcode_table[0xE6] = std::bind(&sharpsm83::set_4_memhl, this);
    CB_opcode_table[0xE7] = std::bind(&sharpsm83::set_4_a, this);
    CB_opcode_table[0xE8] = std::bind(&sharpsm83::set_5_b, this);
    CB_opcode_table[0xE9] = std::bind(&sharpsm83::set_5_c, this);
    CB_opcode_table[0xEA] = std::bind(&sharpsm83::set_5_d, this);
    CB_opcode_table[0xEB] = std::bind(&sharpsm83::set_5_e, this);
    CB_opcode_table[0xEC] = std::bind(&sharpsm83::set_5_h, this);
    CB_opcode_table[0xED] = std::bind(&sharpsm83::set_5_l, this);
    CB_opcode_table[0xEE] = std::bind(&sharpsm83::set_5_memhl, this);
    CB_opcode_table[0xEF] = std::bind(&sharpsm83::set_5_a, this);

    CB_opcode_table[0xF0] = std::bind(&sharpsm83::set_6_b, this);
    CB_opcode_table[0xF1] = std::bind(&sharpsm83::set_6_c, this);
    CB_opcode_table[0xF2] = std::bind(&sharpsm83::set_6_d, this);
    CB_opcode_table[0xF3] = std::bind(&sharpsm83::set_6_e, this);
    CB_opcode_table[0xF4] = std::bind(&sharpsm83::set_6_h, this);
    CB_opcode_table[0xF5] = std::bind(&sharpsm83::set_6_l, this);
    CB_opcode_table[0xF6] = std::bind(&sharpsm83::set_6_memhl, this);
    CB_opcode_table[0xF7] = std::bind(&sharpsm83::set_6_a, this);
    CB_opcode_table[0xF8] = std::bind(&sharpsm83::set_7_b, this);
    CB_opcode_table[0xF9] = std::bind(&sharpsm83::set_7_c, this);
    CB_opcode_table[0xFA] = std::bind(&sharpsm83::set_7_d, this);
    CB_opcode_table[0xFB] = std::bind(&sharpsm83::set_7_e, this);
    CB_opcode_table[0xFC] = std::bind(&sharpsm83::set_7_h, this);
    CB_opcode_table[0xFD] = std::bind(&sharpsm83::set_7_l, this);
    CB_opcode_table[0xFE] = std::bind(&sharpsm83::set_7_memhl, this);
    CB_opcode_table[0xFF] = std::bind(&sharpsm83::set_7_a, this);
}

//##############################################################################
void sharpsm83::print_registers()
{
    std::cout<<"##############################################################################"<<'\n';
    std::cout<<"AF: 0x" << std::hex<<static_cast<int>(AF.b0_15)<<'\n';
    std::cout<<"BC: 0x" << std::hex<<static_cast<int>(BC.b0_15)<<'\n';
    std::cout<<"DE: 0x" << std::hex<<static_cast<int>(DE.b0_15)<<'\n';
    std::cout<<"HL: 0x" << std::hex<<static_cast<int>(HL.b0_15)<<'\n';
    std::cout<<"PC: 0x" << std::hex<<static_cast<int>(PC.b0_15)<<'\n';
    std::cout<<"SP: 0x" << std::hex<<static_cast<int>(SP.b0_15)<<'\n';
    std::cout<<"Flags: Z: " << get_zero_flag() <<" N: " << get_subtraction_flag() 
        << " H: "<<get_half_carry_flag() << " C: "<<get_carry_flag()<<'\n';

    std::cout<<"##############################################################################"<<'\n';

    //bus->mem.print_memory_layout();
}
//##############################################################################
void sharpsm83::reset()
{
    PC.b0_15 = 0x0100;
    SP.b0_15 = 0xFFFE;

    AF.b0_15 = 0x01B0;
    BC.b0_15 = 0x0013;
    DE.b0_15 = 0x00D8;
    HL.b0_15 = 0x014D;

    fetched_data = 0x0;

    interrupts_enabled = false;
    is_halted = false;
}
//##############################################################################
const uint16_t& sharpsm83::get_last_opcode()
{
    return last_opcode;
}
//##############################################################################
const uint16_t& sharpsm83::get_current_adrress()
{
    return PC.b0_15;
}
//##############################################################################
void sharpsm83::set_IE(uint8_t value)
{
   IE.b0_7 = value;
}
//##############################################################################
uint8_t sharpsm83::get_IE()
{
    return IE.b0_7;
}
//##############################################################################
void sharpsm83::set_IF(uint8_t value)
{
    IF.b0_7 = value;
}
//##############################################################################
uint8_t sharpsm83::get_IF()
{
    return IF.b0_7;
}
//##############################################################################
void sharpsm83::handle_interrupts()
{
    reg8 interrupt_request;
    interrupt_request.b0_7 = IE.b0_7 & IF.b0_7;
    if(interrupt_request.b0_7 == 0) return;

    // TODO: there is a bug in the original hardware to implement
    if(is_halted && interrupt_request.b0_7 != 0x0) { return;}

    if(!interrupts_enabled) return;

    stack_push(PC.b0_15);

    interrupts_enabled = false;

    if(interrupt_request.b0)      //V-Blank
    {
        IF.b0 = 0;
        PC.b0_15 = interrupt_address::VBLANK;
    }
    else if(interrupt_request.b1)  //LCD STAT
    {
        IF.b1 = 0;
        PC.b0_15 = interrupt_address::LCDC;
    }
    else if(interrupt_request.b2)  //Timer
    {
        IF.b2 = 0;
        PC.b0_15 = interrupt_address::TIMER;
    }
    else if(interrupt_request.b3)  //Serial
    {
        IF.b3 = 0;
        PC.b0_15 = interrupt_address::SERIAL;
    }
    else if(interrupt_request.b4)  //Joypad
    {
        IF.b4 = 0;
        PC.b0_15 = interrupt_address::JOYPAD;
    }
}
//##############################################################################
