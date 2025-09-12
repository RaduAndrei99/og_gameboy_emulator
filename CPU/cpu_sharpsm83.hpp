#ifndef _CPU_SHARPSM83_
#define _CPU_SHARPSM83_

#include <cstdint>
#include <memory>
#include <functional>
#include <array>

#include "../BUS/gb_bus.hpp"
#include "../CARTRIDGE/gb_cartridge.hpp"

#define CPU_BITS 8

struct interrupt_address
{
    static constexpr uint16_t VBLANK = 0x40;
    static constexpr uint16_t LCDC   = 0x48;
    static constexpr uint16_t TIMER  = 0x50;
    static constexpr uint16_t SERIAL = 0x58;
    static constexpr uint16_t JOYPAD = 0x60;
};

class gb_bus;

struct sharpsm83
{
private:
    union reg8
    {
        struct 
        { 
            uint8_t b0 : 1;
            uint8_t b1 : 1;
            uint8_t b2 : 1;
            uint8_t b3 : 1;
            uint8_t b4 : 1;
            uint8_t b5 : 1;
            uint8_t b6 : 1;
            uint8_t b7 : 1;
        };
        uint8_t b0_7;
    };

    union reg16
    {
        struct 
        {
            reg8 Lo;
            reg8 Hi;
        };
        uint16_t b0_15;
    };

    // SM83 registers

    // acumulator and flags
    reg16 AF;
    // general purpose 
    reg16 BC;
    reg16 DE;
    reg16 HL;
    // stack pointer
    reg16 SP;
    //program counter
    reg16 PC;

    // interrupt enable register
    reg8 IE; 
    // interrupt flags register
    reg8 IF;

    void handle_interrupts();

    bool is_halted;
    bool interrupts_enabled;

    bool exit_on_infinite_jr = true;

    uint16_t last_opcode;

    // cpu chip enable  
    bool ce;

    std::shared_ptr<gb_bus> bus;

    uint8_t fetched_data;

    void enable_interrupts();
    void disable_interrupts();

    // getters for the flags
    bool get_zero_flag();
    void set_zero_flag(bool val);

    bool get_subtraction_flag();
    void set_subtraction_flag(bool val);

    bool get_half_carry_flag();
    void set_half_carry_flag(bool val);

    bool get_carry_flag();
    void set_carry_flag(bool val);
    // cpu instructions to set carry flag
    void set_carry_flag();
    // cpu instructions to set complement flag
    void complement_carry_flag();

    void fetch_data(const uint16_t& address);
    void write_data(const uint16_t& address, const uint8_t& data);

    void execute_nop();
    void execute_halt();
    void execute_stop();

    // generic left/right rotations with or without carry
    void rlc(reg8& reg);
    void rl(reg8& reg);
    void rrc(reg8& reg);
    void rr(reg8& reg);

    // generic jump instruction
    void jr(bool cond);

    void jp(const uint16_t& address);
    void jp(bool cond);

    void call(bool cond);
    void call(const uint16_t& address);

    void rst(const uint16_t& address);

    // generic decimal adjust
    void da(reg8& r);

    // generic complement register
    void complement(reg8& reg);

    // writes data to address
    void ld_to_address(const uint16_t& address, const uint8_t& data);
    // writes imm8 data to address indicated by reg
    void ld_to_address(const reg16& reg);

    void ld_to_address(const reg8& reg);


    void ldh_to_address(const reg8& reg);
    void ldh_to_address(const reg8& to, const reg8& reg);
    void ldh_from_address(reg8& reg);
    void ldh_from_address(reg8& to, const reg8& reg);

    void ld_hl_reg_e8(const reg16& reg);

    void ld_a_memimm16_op();

    void ld_sp_hl_op();

    // writes data from address
    void ld_from_address(uint8_t& reg, const uint16_t& address);
    // writes 16 bits from memory to the destination
    void ld(uint16_t& to);
    // writes 8 bits from memory to the destination
    void ld(uint8_t& to);
    // writes data from a 8 bit location to a 8 bit location
    void ld(uint8_t& to, const uint8_t& from);

    // special function to write sp in the memory
    void ld_sp();

    void inc(uint16_t& reg);
    void inc(uint8_t& reg);
    void inc_mem(uint16_t& address);
    void dec(uint8_t& reg);
    void dec(uint16_t& reg);
    void dec_mem(uint16_t& address);

    void add(uint16_t& op1, uint16_t& op2);

    void add(uint8_t& op1, uint8_t& op2);
    void add_from_address(uint8_t& op1, uint16_t& address);
    void add(reg8& reg);
    void add_e8(reg16& reg);

    void adc(uint8_t& op1, uint8_t& op2);
    void adc_from_address(uint8_t& op1, uint16_t& address);
    void adc(reg8& reg);

    void sub(uint8_t& op1, uint8_t& op2);
    void sub_from_address(uint8_t& op1, uint16_t& address);
    void sub(reg8& reg);

    void sbc(uint8_t& op1, uint8_t& op2);
    void sbc_from_address(uint8_t& op1, uint16_t& address);
    void sbc(reg8& reg);

    void and_op(uint8_t& op1, uint8_t& op2);
    void and_op_from_address(uint8_t& op1, uint16_t& address);
    void and_op(reg8& reg);

    void xor_op(uint8_t& op1, uint8_t& op2);
    void xor_op_from_address(uint8_t& op1, uint16_t& address);
    void xor_op(reg8& reg);

    void or_op(uint8_t& op1, uint8_t& op2);
    void or_op_from_address(uint8_t& op1, uint16_t& address);
    void or_op(reg8& reg);

    void cp_op(uint8_t& op1, uint8_t& op2);
    void cp_op_from_address(uint8_t& op1, uint16_t& address);
    void cp_op(reg8& reg);

    void rlc_param(reg8& reg);

    void ret_op();
    void ret_condition(bool condition);
    void reti_op();
    void rlcmem_param(uint16_t& address);

    void rrc_param(reg8& reg);

    void rrcmem_param(uint16_t& address);

    void rl_param(reg8& reg);

    void rlmem_param(uint16_t& address);

    void rr_param(reg8& reg);

    void rrmem_param(uint16_t& address);

    void sla_param(reg8& reg);

    void slamem_param(uint16_t& address);

    void sra_param(reg8& reg);

    void sramem_param(uint16_t& address);

    void swap_param(reg8& reg);

    void swapmem_param(uint16_t& address);

    void srl_param(reg8& reg);

    void srlmem_param(uint16_t& address);

    void bit_param(uint8_t bit, reg8& reg);

    void bitmem_param(uint8_t bit, uint16_t& address);\

    void res_param(uint8_t bit, reg8& reg);

    void resmem_param(uint8_t bit, uint16_t& address);

    void set_param(uint8_t bit, reg8& reg);

    void setmem_param(uint8_t bit, uint16_t& address);

    void stack_push(const uint16_t& value);
    void stack_pop(uint16_t& value);

    void pop(reg16& reg);
    void push(reg16& reg);

    void pop_af_op();
    void push_af_op();

    // instructions
    // 0x00 - 0x0F
    void nop();
    void ld_bc_imm16();
    void ld_membc_a();
    void inc_bc();
    void inc_b();
    void dec_b();
    void ld_b_imm8();
    void rlca();
    void ld_memimm16_sp();
    void add_hl_bc();
    void ld_a_membc();
    void dec_bc();
    void inc_c();
    void dec_c();
    void ld_c_imm8();
    void rrca();

    // 0x10 - 0x1F
    void stop_imm8();
    void ld_de_imm16(); 
    void ld_memde_a();
    void inc_de();
    void inc_d();
    void dec_d();
    void ld_d_imm8();
    void rla();
    void jr_e8();
    void add_hl_de();
    void ld_a_memde();
    void dec_de();
    void inc_e();
    void dec_e();
    void ld_e_imm8();
    void rra();

    // 0x20 - 0x2F
    void jr_nz_e8();
    void ld_hl_imm16();
    void ld_memhlinc_a();
    void inc_hl();
    void inc_h();
    void dec_h();
    void ld_h_imm8();
    void daa();
    void jr_z_e8();
    void add_hl_hl();
    void ld_a_memhlinc();
    void dec_hl();
    void inc_l();
    void dec_l();
    void ld_l_imm8();
    void cpl();

    // 0x30 - 0x3F
    void jr_nc_e8();
    void ld_sp_imm16();
    void ld_memhldec_a();
    void inc_sp();
    void inc_memhl();
    void dec_memhl();
    void ld_memhl_imm8();
    void scf();
    void jr_c_e8();
    void add_hl_sp();
    void ld_a_memhldec();
    void dec_sp();
    void inc_a();
    void dec_a();
    void ld_a_imm8();
    void ccf();

    // 0x40 - 0x4F
    void ld_b_b();
    void ld_b_c();
    void ld_b_d();
    void ld_b_e();
    void ld_b_h();
    void ld_b_l();
    void ld_b_memhl();
    void ld_b_a();
    void ld_c_b();
    void ld_c_c();
    void ld_c_d();
    void ld_c_e();
    void ld_c_h();
    void ld_c_l();
    void ld_c_memhl();
    void ld_c_a();

    // 0x50 - 0x5F
    void ld_d_b();
    void ld_d_c();
    void ld_d_d();
    void ld_d_e();
    void ld_d_h();
    void ld_d_l();
    void ld_d_memhl();
    void ld_d_a();
    void ld_e_b();
    void ld_e_c();
    void ld_e_d();
    void ld_e_e();
    void ld_e_h();
    void ld_e_l();
    void ld_e_memhl();
    void ld_e_a();

    // 0x60 - 0x6F
    void ld_h_b();
    void ld_h_c();
    void ld_h_d();
    void ld_h_e();
    void ld_h_h();
    void ld_h_l();
    void ld_h_memhl();
    void ld_h_a();
    void ld_l_b();
    void ld_l_c();
    void ld_l_d();
    void ld_l_e();
    void ld_l_h();
    void ld_l_l();
    void ld_l_memhl();
    void ld_l_a();

    // 0x70 - 0x7F
    void ld_memhl_b();
    void ld_memhl_c();
    void ld_memhl_d();
    void ld_memhl_e();
    void ld_memhl_h();
    void ld_memhl_l();
    void halt();
    void ld_memhl_a();
    void ld_a_b();
    void ld_a_c();
    void ld_a_d();
    void ld_a_e();
    void ld_a_h();
    void ld_a_l();
    void ld_a_memhl();
    void ld_a_a();

    // 0x80 - 0x8F
    void add_a_b();
    void add_a_c();
    void add_a_d();
    void add_a_e();
    void add_a_h();
    void add_a_l();
    void add_a_memhl();
    void add_a_a();
    void adc_a_b();
    void adc_a_c();
    void adc_a_d();
    void adc_a_e();
    void adc_a_h();
    void adc_a_l();
    void adc_a_memhl();
    void adc_a_a();

    // 0x90 - 0x9F
    void sub_a_b();
    void sub_a_c();
    void sub_a_d();
    void sub_a_e();
    void sub_a_h();
    void sub_a_l();
    void sub_a_memhl();
    void sub_a_a();
    void sbc_a_b();
    void sbc_a_c();
    void sbc_a_d();
    void sbc_a_e();
    void sbc_a_h();
    void sbc_a_l();
    void sbc_a_memhl();
    void sbc_a_a();  
    
    // 0xA0 - 0xAF
    void and_a_b();
    void and_a_c();
    void and_a_d();
    void and_a_e();
    void and_a_h();
    void and_a_l();
    void and_a_memhl();
    void and_a_a();
    void xor_a_b();
    void xor_a_c();
    void xor_a_d();
    void xor_a_e();
    void xor_a_h();
    void xor_a_l();
    void xor_a_memhl();
    void xor_a_a(); 

    // 0xB0 - 0xBF
    void or_a_b();
    void or_a_c();
    void or_a_d();
    void or_a_e();
    void or_a_h();
    void or_a_l();
    void or_a_memhl();
    void or_a_a();
    void cp_a_b();
    void cp_a_c();
    void cp_a_d();
    void cp_a_e();
    void cp_a_h();
    void cp_a_l();
    void cp_a_memhl();
    void cp_a_a(); 

    // 0xC0 - 0xCF
    void ret_nz();
    void pop_bc();
    void jp_nz_imm16();
    void jp_imm16();
    void call_nz_imm16();
    void push_bc();
    void add_a_imm8();
    void rst_0x00();
    void ret_z();
    void ret();
    void jp_z_imm16();
    void prefix();
    void call_z_imm16();
    void call_imm16();
    void adc_a_imm8();
    void rst_0x08();

    // 0xD0 - 0xDF
    void ret_nc();
    void pop_de();
    void jp_nc_imm16();
    void op_0xD3();
    void call_nc_imm16();
    void push_de();
    void sub_a_imm8();
    void rst_0x10();
    void ret_c();
    void reti();
    void jp_c_imm16();
    void op_0xDB();
    void call_c_a16();
    void op_0xDD();
    void sbc_a_imm8();
    void rst_0x18();

    // 0xE - 0xEF
    void ldh_memimm8_a();
    void pop_hl();
    void ldh_memc_a();
    void op_0xE3();
    void op_0xE4();
    void push_hl();
    void and_a_imm8();
    void rst_0x20();
    void add_sp_e8();
    void jp_hl();
    void ld_memimm16_a();
    void op_0xEB();
    void op_0xEC();
    void op_0xED();
    void xor_a_imm8();
    void rst_0x28();

    // OxF0 - 0xFF
    void ldh_a_memimm8();
    void pop_af();
    void ldh_a_memc();
    void di();
    void op_0xF4();
    void push_af();
    void or_a_imm8();
    void rst_0x30();
    void ld_hl_sp_e8();
    void ld_sp_hl();
    void ld_a_memimm16();
    void ei();
    void op_0xFC();
    void op_0xFD();
    void cp_a_imm8();
    void rst_0x38();

    // CB prefixed instructions

    // 0x00 - 0x0F
    void rlc_b();
    void rlc_c();
    void rlc_d();
    void rlc_e();
    void rlc_h();
    void rlc_l();
    void rlc_memhl();
    void rlc_a();

    void rrc_b();
    void rrc_c();
    void rrc_d();
    void rrc_e();
    void rrc_h();
    void rrc_l();
    void rrc_memhl();
    void rrc_a();

    // 0x10 - 0x1F
    void rl_b();
    void rl_c();
    void rl_d();
    void rl_e();
    void rl_h();
    void rl_l();
    void rl_memhl();
    void rl_a();

    void rr_b();
    void rr_c();
    void rr_d();
    void rr_e();
    void rr_h();
    void rr_l();
    void rr_memhl();
    void rr_a();

    // 0x20 - 0x2F
    void sla_b();
    void sla_c();
    void sla_d();
    void sla_e();
    void sla_h();
    void sla_l();
    void sla_memhl();
    void sla_a();

    void sra_b();
    void sra_c();
    void sra_d();
    void sra_e();
    void sra_h();
    void sra_l();
    void sra_memhl();
    void sra_a();

    // 0x30 - 0x3F
    void swap_b();
    void swap_c();
    void swap_d();
    void swap_e();
    void swap_h();
    void swap_l();
    void swap_memhl();
    void swap_a();

    void srl_b();
    void srl_c();
    void srl_d();
    void srl_e();
    void srl_h();
    void srl_l();
    void srl_memhl();
    void srl_a();

    // 0x40 - 0x4F
    void bit_0_b();
    void bit_0_c();
    void bit_0_d();
    void bit_0_e();
    void bit_0_h();
    void bit_0_l();
    void bit_0_memhl();
    void bit_0_a();

    void bit_1_b();
    void bit_1_c();
    void bit_1_d();
    void bit_1_e();
    void bit_1_h();
    void bit_1_l();
    void bit_1_memhl();
    void bit_1_a();

    // 0x50 - 0x5F
    void bit_2_b();
    void bit_2_c();
    void bit_2_d();
    void bit_2_e();
    void bit_2_h();
    void bit_2_l();
    void bit_2_memhl();
    void bit_2_a();

    void bit_3_b();
    void bit_3_c();
    void bit_3_d();
    void bit_3_e();
    void bit_3_h();
    void bit_3_l();
    void bit_3_memhl();
    void bit_3_a();

    // 0x60 - 0x6F
    void bit_4_b();
    void bit_4_c();
    void bit_4_d();
    void bit_4_e();
    void bit_4_h();
    void bit_4_l();
    void bit_4_memhl();
    void bit_4_a();

    void bit_5_b();
    void bit_5_c();
    void bit_5_d();
    void bit_5_e();
    void bit_5_h();
    void bit_5_l();
    void bit_5_memhl();
    void bit_5_a();

    // 0x70 - 0x7F
    void bit_6_b();
    void bit_6_c();
    void bit_6_d();
    void bit_6_e();
    void bit_6_h();
    void bit_6_l();
    void bit_6_memhl();
    void bit_6_a();

    void bit_7_b();
    void bit_7_c();
    void bit_7_d();
    void bit_7_e();
    void bit_7_h();
    void bit_7_l();
    void bit_7_memhl();
    void bit_7_a();

    // 0x80 - 0x8F
    void res_0_b();
    void res_0_c();
    void res_0_d();
    void res_0_e();
    void res_0_h();
    void res_0_l();
    void res_0_memhl();
    void res_0_a();

    void res_1_b();
    void res_1_c();
    void res_1_d();
    void res_1_e();
    void res_1_h();
    void res_1_l();
    void res_1_memhl();
    void res_1_a();

    // 0x90 - 0x9F
    void res_2_b();
    void res_2_c();
    void res_2_d();
    void res_2_e();
    void res_2_h();
    void res_2_l();
    void res_2_memhl();
    void res_2_a();

    void res_3_b();
    void res_3_c();
    void res_3_d();
    void res_3_e();
    void res_3_h();
    void res_3_l();
    void res_3_memhl();
    void res_3_a();

    // 0xA0 - 0xAF
    void res_4_b();
    void res_4_c();
    void res_4_d();
    void res_4_e();
    void res_4_h();
    void res_4_l();
    void res_4_memhl();
    void res_4_a();

    void res_5_b();
    void res_5_c();
    void res_5_d();
    void res_5_e();
    void res_5_h();
    void res_5_l();
    void res_5_memhl();
    void res_5_a();

    // 0xB0 - 0xBF
    void res_6_b();
    void res_6_c();
    void res_6_d();
    void res_6_e();
    void res_6_h();
    void res_6_l();
    void res_6_memhl();
    void res_6_a();

    void res_7_b();
    void res_7_c();
    void res_7_d();
    void res_7_e();
    void res_7_h();
    void res_7_l();
    void res_7_memhl();
    void res_7_a();

    // 0xC0 - 0xCF
    void set_0_b();
    void set_0_c();
    void set_0_d();
    void set_0_e();
    void set_0_h();
    void set_0_l();
    void set_0_memhl();
    void set_0_a();

    void set_1_b();
    void set_1_c();
    void set_1_d();
    void set_1_e();
    void set_1_h();
    void set_1_l();
    void set_1_memhl();
    void set_1_a();

    // 0xD0 - 0xDF
    void set_2_b();
    void set_2_c();
    void set_2_d();
    void set_2_e();
    void set_2_h();
    void set_2_l();
    void set_2_memhl();
    void set_2_a();

    void set_3_b();
    void set_3_c();
    void set_3_d();
    void set_3_e();
    void set_3_h();
    void set_3_l();
    void set_3_memhl();
    void set_3_a();

    // 0xE0 - 0xEF
    void set_4_b();
    void set_4_c();
    void set_4_d();
    void set_4_e();
    void set_4_h();
    void set_4_l();
    void set_4_memhl();
    void set_4_a();

    void set_5_b();
    void set_5_c();
    void set_5_d();
    void set_5_e();
    void set_5_h();
    void set_5_l();
    void set_5_memhl();
    void set_5_a();

    // 0xF0 - 0xFF
    void set_6_b();
    void set_6_c();
    void set_6_d();
    void set_6_e();
    void set_6_h();
    void set_6_l();
    void set_6_memhl();
    void set_6_a();

    void set_7_b();
    void set_7_c();
    void set_7_d();
    void set_7_e();
    void set_7_h();
    void set_7_l();
    void set_7_memhl();
    void set_7_a();


    using OpcodeHandler = std::function<void(void)>;

    // normal instructions
    std::array< OpcodeHandler, 0x1<<CPU_BITS > opcode_table;

    // 0xCB prefix instructions
    std::array< OpcodeHandler, 0x1<<CPU_BITS > CB_opcode_table;

    void initialize_opcodes();
    void initialize_cbopcodes();

    void emulate_cycles(int cycles);
public:
    sharpsm83();
    ~sharpsm83();

    void set_bus(const std::shared_ptr<gb_bus>& b);

    void tick();

    void execute(uint8_t opcode);
    void execute_normal_instruction(uint8_t opcode);
    void execute_0xCB_instruction(uint8_t opcode);

    const uint16_t& get_last_opcode();
    const uint16_t& get_current_adrress();

    void print_registers();

    void reset();

    void set_IE(uint8_t value);
    uint8_t get_IE();
    void set_IF(uint8_t value);
    uint8_t get_IF();
};



#endif