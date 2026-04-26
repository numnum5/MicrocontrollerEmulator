#include <iostream>
#include <vector>
#include <limits>
// #include <format>
#include <bitset>
#include <bit>

using bit = bool;
constexpr uint8_t PC_INDEX = 15;
typedef enum {
    SRType_LSL,
    SRType_LSR,
    SRType_ASR,
    SRType_ROR,
    SRType_RRX

} SRType;
typedef struct {

    bit t;
} EPSR;

typedef struct
{
    bit N;
    bit C;
    bit Z;
    bit V;
    bit Q;
} ASPR;

struct AddCarryResult
{
    uint32_t result;
    bool carry_out;
    bool overflow;
};

typedef struct {
    SRType type;
    uint8_t n;
} DecodeImmShiftResult;

struct Shift_c
{
    uint32_t result;
    bool carry_out;
};




class Cpu
{
    public:
        uint8_t flash[0x1000];

        Cpu()
        {
            for(uint8_t i = 0; i < 16; i++)
            {
                this->regs[i] = 0;
            }

            this->aspr.C = 0;
            this->aspr.V = 0;
            this->aspr.N = 0;
            this->aspr.Z = 0;
        }   

        uint32_t getSP(void) const
        {
            return this->regs[15];
        }

        uint32_t read32(uint32_t address) const
        {
            uint32_t data = this->ram[address] |
                (this->ram[address + 1] << 8) |
                (this->ram[address + 2] << 16) |
                (this->ram[address + 3] << 24);
            
            return data;
        }

        uint8_t read8(uint32_t address) const
        {
            uint8_t data = this->ram[address];
            
            return data;
        }

        uint16_t read16(uint32_t address) const 
        {
            uint32_t data = this->ram[address] | (this->ram[address + 1] << 8);

            return data;
        }   


        uint32_t fetch(void) const
        {
            // little endians lsb first

            uint32_t pc = this->regs[15];
            uint32_t instr = this->flash[pc] |
                            (this->flash[pc + 1] << 8) |
                            (this->flash[pc + 2] << 16) |
                            (this->flash[pc + 3] << 24);
            return instr;
        }

        uint8_t currentCond(uint32_t instruction)
        {
            return (instruction >> 7) & 0b1111;
        }

        uint32_t shift(uint32_t value, SRType type, uint8_t amount, bit carry_in)
        {
            Shift_c shifted = this->shift_c(value, type, amount, carry_in);
            return shifted.result;
        }
        
        uint32_t sign_extend(uint32_t value, int bits)
        {
            if (value & (1u << (bits - 1))) {
                value |= (~0u << bits);
            }
            return value;
        }

        

        DecodeImmShiftResult decodeImmShift(uint8_t type, uint8_t imm5)
        {
            SRType shift_t;
            uint8_t shift_n;
            switch(type)
            {
                case 0b00:
                    shift_t = SRType_LSL;
                    shift_n = imm5;
                    break;
                case 0b01:
                                    shift_t = SRType_LSR;

                    if (imm5 == 0x0)
                    {
                        shift_n = 32;
                    }
                    else
                    {
                        shift_n = imm5;
                    }
                    
                    break;

                case 0b10:
                    shift_t = SRType_ASR;

                    if (imm5 == 0x0)
                    {
                        shift_n = 32;
                    }
                    else
                    {
                        shift_n = imm5;
                    }
                    
                    break;

                case 0b11:

                if (imm5 == 0x0)
                {
                    shift_t = SRType_RRX;
                    shift_n = 1;
                }
                else
                {
                    shift_t = SRType_ROR;
                    shift_n = imm5;
                }
            }

            return {shift_t, shift_n};
        }

        void lsl_c()
        {


        }

        Shift_c shift_c(uint32_t value, SRType type, uint8_t amount, bit carry_in)
        {

            
            if (amount == 0)
            {
                return {value, carry_in};
            }
            else
            {
                uint32_t result;
                bit carry_out;

                switch(type)
                {
                    case SRType_ASR:
                        break;
                    case SRType_LSL:
                        result = value << amount;
                        carry_out = (value >> (32 - amount)) & 1;
                        break;
                    case SRType_LSR:
                        break;
                    case SRType_ROR:
                        break;
                    case SRType_RRX:
                        break;
                    default:
                        result = value;
                        carry_out = carry_in;
                        break;
                }

                return {result, carry_out};
            }
        }
        
        // ARM-style AddWithCarry for 32-bit values
        AddCarryResult addWithCarry(uint32_t x, uint32_t y, bool carry_in)
        {
            // Full unsigned sum (needs 33 bits max, uint64_t is enough)
            uint64_t unsigned_sum =
                (uint64_t)x +
                (uint64_t)y +
                (uint64_t)(carry_in ? 1 : 0);

            // Low 32 bits become result
            uint32_t result = (uint32_t)unsigned_sum;

            // Carry out = bit 32 of the full sum
            bool carry_out = (unsigned_sum >> 32) & 1;

            // Signed interpretation of inputs
            int64_t signed_sum =
                (int64_t)(int32_t)x +
                (int64_t)(int32_t)y +
                (carry_in ? 1 : 0);

            // Signed overflow if outside int32_t range
            bool overflow =
                signed_sum > std::numeric_limits<int32_t>::max() ||
                signed_sum < std::numeric_limits<int32_t>::min();

            return { result, carry_out, overflow };
        }

        bool inITBlock(void)
        {
            return false;
        }

        void decode()
        {
            
            uint32_t instruction = this->fetch();

            printf("Instruction: %lX\n", instruction);

            // return;
            uint8_t format_id = (instruction >> 11) & 0x1F;

            // uint8_t instruction_type = (instruction >> 11) & 0b11111;
            // std::cout << std::bitset<32>(thumb_mode) << "\n";
            std::cout << "THum mode:" << std::bitset<5>(format_id) << std::endl;


            // printf("%x\n", thumb_mode);
            switch(format_id)
            {
                case 0b11101:
                // BL
                case 0b11110:
                {
                    bit tenth = instruction & (0b1 << 10);

                    uint8_t nine_to_five = ((instruction >> 0xFFFF) >> 5) & 0b11111; 

                    uint16_t imm11 = instruction & 0b11111111111;
                    uint16_t imm10 = (instruction >> 0xFF) & 0b1111111111;

                    bit S = (instruction >> 0xFF) & (0b1 << 10);
                    bit J2 = (instruction) &  (0b1 << 11);
                    bit J1 =  (instruction) &  (0b1 << 13);


                    bit I1 = ~(J1 ^ S);
                    bit I2 = ~(J2 ^ S);

                    //  ...... .......

                    uint32_t imm32 =  (S << 24) | (I2 << 23) | (I1 << 22) | (imm10 << 12 ) | (imm11 << 1);

                    uint32_t next_instruction_addr = this->regs[13];
                    this->regs[14] = next_instruction_addr | 1;
                    this->regs[13] += imm32;

                    break;

                }
                case 0b11111:
                    // 32 bit thumb mode
                    // 
                    break;

 
                    

                case 0b01000:
                {   
                    // check if its also AND instruction


                    uint8_t is_and = (instruction >> 6) & 0b1111;

                    bit tenthBit = (instruction & 0b1 << 10);

                    if (tenthBit == 1)
                    {
                        // check 9, 8 and 7th bit
                        // BLX


                        // MOV (register)
                        uint8_t nineEight = (instruction >> 8) & 0b11;

                        switch(nineEight)
                        {
                            case 0b10:
                            {
                                bit D = (instruction) & (0b1 << 7);
                                uint8_t d = (instruction >> 3) & 0b111;
                                uint8_t m = (instruction ) & 0b111;
                                d = (D << 3) | d; 

                                uint32_t result = regs[m];

                                if (d == 15)
                                {
                                    this->regs[15] = result;
                                }
                                else
                                {
                                    this->regs[d] = result;
                                    this->aspr.N = result >> 31;
                                    this->aspr.Z = result == 0x0;
                                }

                            }
                        }

                        // check 9, 8 and 7th bit
                        // BLX

                        uint8_t blx_bits = (instruction >> 7) & 0b111;


                        // this is BLX instruction
                        if (blx_bits == 0b111)
                        {
                            uint8_t m = instruction & 0b111;

                            if (m == 15)
                            {
                                std::cout << "Unpredictable behaviour!\n";
                                return;
                            }


                            uint32_t target = this->regs[m];

                            uint32_t next_pc = this->regs[13] - 2;
                            this->regs[14] = next_pc | 1; 

                            this->espr.t = target & 0b1;
                            this->regs[13] = target & ~0b1;
                            break;
                        }
                        else if (blx_bits == 0b110)
                        {
                            
                             uint8_t m = instruction & 0b111;
                            if (m == 15)
                            {
                                std::cout << "Unpredictable behaviour!\n";
                                return;
                            }

                            this->regs[13] = this->regs[m];
                            break;
                        }
                    }
                    // Tenth bit is 0
                    else
                    {
                        switch(is_and)
                        {

                            case 0b1000:
                            {
                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                auto shifted = this->shift_c(this->regs[m], SRType_LSL, 0, aspr.C);     

                                uint32_t result = this->regs[n] & shifted.result;

                                this->aspr.N = result >> 31;
                                this->aspr.C = shifted.carry_out;
                                this->aspr.Z = result == 0x0;

                                break;
                            }
                            // LSL (Register)
                            case 0b0010:
                            {
                                bool setflags = ! this->inITBlock();
                                
                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;


                                uint8_t shift_n = 0;
                                SRType type = SRType_LSL;

                                uint32_t Rm = this->regs[m];
                                uint32_t Rn = this->regs[n];
                                uint32_t shifted = this->shift(Rm, type, shift_n, this->aspr.C);

                                AddCarryResult carry_result = this->addWithCarry(Rn,shifted, this->aspr.C);

                                this->regs[d] = carry_result.result;

                                // would perform shift but it wont do anything 

                                if (setflags)
                                {
                                    this->aspr.N = carry_result.result >> 31;
                                    this->aspr.C = carry_result.carry_out;
                                    this->aspr.Z = carry_result.result == 0x0;
                                    this->aspr.V = carry_result.overflow;
                                }
                            }

                            case 0b1111:
                            {
                                uint8_t d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;

                                auto shifted = this->shift_c(this->regs[m], SRType_LSL, 0, aspr.C);
                                
                                uint32_t result = ~shifted.result;

                                this->regs[d] = result;

                                this->aspr.N = result >> 31;
                                this->aspr.Z = result == 0x0;
                                this->aspr.C = shifted.carry_out;
                        
                                break;
                            }

                            // case 0b1101:
                            // {

                            //     uint8_t n, d = instruction & 0b111;
                            //     uint8_t m = (instruction >> 3) & 0b111;

                            //     uint32_t operand1 = this->regs[n];

                            //     uint32_t operand2 = this->regs[m];

                                
                            //     uint64_t result = operand2 * operand1;

                            //     this->regs[d] = (uint32_t) result;

                            //     this->aspr.N = result >> 31;

                            //     this->aspr.Z = (uint32_t) result == 0;

                            //     break;
                            // }

                            case 0b0000:
                            {
                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                uint32_t Rm = this->regs[m];

                                uint8_t shift_n = 0;
                                SRType type = SRType_LSL;    
                                Shift_c shifted = this->shift_c(Rm, type, shift_n, this->aspr.C);                              

                                uint32_t result = shifted.result & this->regs[n];

                                this->regs[d] = result;
                                this->aspr.N = shifted.result >> 31;
                                this->aspr.C = shifted.carry_out;
                                this->aspr.Z = shifted.result == 0x0;
                                break;
                            }
                            // wtf

                            case 0b1110:
                            {
                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                uint32_t Rm = this->regs[m];

                                uint8_t shift_n = 0;
                                SRType type = SRType_LSL;    
                                Shift_c shifted = this->shift_c(Rm, type, shift_n, this->aspr.C);                              

                                uint32_t result = shifted.result & ~(this->regs[n]);

                                this->regs[d] = result;

                                this->aspr.N = shifted.result >> 31;
                                this->aspr.C = shifted.carry_out;
                                this->aspr.Z = shifted.result == 0x0;
                                break;
                            }

                            // CMN
                            case 0b1101:
                            {
                                uint8_t n = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                
                                uint32_t Rm = this->regs[m];
                                uint32_t Rn = this->regs[n];
                                uint32_t shifted = this->shift(Rm, SRType_LSL, 0, aspr.C);
                                
                                const AddCarryResult result = this->addWithCarry(Rn, shifted, 0);

                                this->aspr.N = result.result >> 31;
                                this->aspr.V = result.overflow;
                                this->aspr.C = result.carry_out;
                                this->aspr.Z = result.result == 0x0;
                                break;
                            }

                            // ORR (Register)
                            case 0b1100:
                            {

                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                
                                uint32_t Rm = this->regs[m];
                                uint32_t Rn = this->regs[n];
                                Shift_c shifted = this->shift_c(Rm, SRType_LSL, 0, aspr.C);

                                uint32_t result = Rn | shifted.result;

                                this->regs[d] = result;
                                this->aspr.N = result >> 31;
                                this->aspr.C = shifted.carry_out;
                                this->aspr.Z = result == 0x0;
                                break;
                            }

                            case 0b0001:
                            {
                                uint8_t n, d = instruction & 0b111;
                                uint8_t m = (instruction >> 3) & 0b111;
                                
                                uint32_t Rm = this->regs[m];
                                uint32_t Rn = this->regs[n];
                                Shift_c shifted = this->shift_c(Rm, SRType_LSL, 0, aspr.C);

                                uint32_t result = Rn ^ shifted.result;

                                this->regs[d] = result;
                                this->aspr.N = result >> 31;
                                this->aspr.C = shifted.carry_out;
                                this->aspr.Z = result == 0x0;
                                break;
                            }

                        }
               

                    }

                  
                    break;
                }

                // CMP (immediate)
                case 0b00101:
                {

                    uint8_t n = (instruction >> 8) & 0b111;
                    uint8_t imm8 = instruction & 0xFF;
                    uint32_t imm32 = (uint32_t) imm8;

                    auto result = this->addWithCarry(this->regs[n], ~imm32, 1);

                    this->aspr.N = result.result >> 31;
                    this->aspr.C = result.carry_out;
                    this->aspr.Z = result.result == 0x0;
                    this->aspr.V = result.overflow;
                    break;
                }
                // ADD encoding T1
                case 0b00011:
                {
                    printf("ADD T1\n");

                    uint8_t op = (instruction >> 9) & 0b11;
                    
                    uint8_t d = instruction & 0b111;
                    
                    uint8_t n = (instruction >> 3) & 0b111;

                    uint8_t imm3 = (instruction >> 6) & 0b111;

                    uint32_t extended_imm3 = (uint32_t) imm3;

                    
                    switch(op)
                    {
                        case 0b10:
                        {
                            uint32_t Rn = this->regs[n];

                            auto result = this->addWithCarry(Rn, extended_imm3, 0);
                            
                            this->regs[d] = result.result;

                            this->aspr.N = result.result >> 31;
                            this->aspr.C = result.carry_out;
                            this->aspr.Z = result.result == 0x0;
                            this->aspr.V = result.overflow;
                            break;
                        }
                        case 0b00:
                        {
                            uint32_t Rn = this->regs[n];
                            uint32_t Rm = this->regs[imm3];
                            uint8_t shift_n = 0;
                            SRType type = SRType_LSL;
                            uint32_t shifted = this->shift(Rm, type, shift_n, this->aspr.C);
                            auto result = this->addWithCarry(Rn, shifted, 0);

                            if (d == 15)
                            {
                                this->regs[d] = result.result;
                            }

                            this->regs[d] = result.result;

                            this->aspr.N = result.result >> 31;
                            this->aspr.C = result.carry_out;
                            this->aspr.Z = result.result == 0x0;
                            this->aspr.V = result.overflow;
                            break;
                        }

                        default:
                            std::cout << "default op, somethings wrong!!!\n";
                            break;
                    }
                    break;
                }

                // ADD encoding T2
                case 0b00110:
                    break;

                // LDM
                case 0b11001:
                {
                    uint8_t n = (instruction >> 8) & 0b111;
                    uint8_t register_list = instruction & 0xFF;

                    uint16_t registers = (uint16_t) register_list;

                    bool wback = (registers & 0b1 << n) == 0;
                    if (std::bitset<32>(registers).count() < 1)
                    {
                        
                        return;
                    }

                    uint32_t address = this->regs[n];

                    for (uint8_t i = 0; i < 8 ; i++)
                    {
                        // Isolate the ith bit
    
                        if (registers & (0b1 << i))
                        {
                            this->regs[i] = this->read32(address);
                            address += 4;
                        }
                    }

                    if (wback && (registers & 0b1 << n) == 0)
                    {
                        this->regs[n] = this->regs[n] + (4 * std::bitset<32>(registers).count());
                    }

                    break;

                }

                case 0b10111:
                {
                    // NOP
                    uint8_t ten_nine = (instruction >> 9) & 0b11;
                    // 0b0111

                    // POP
                    if (ten_nine == 0b10)
                    {
                        uint8_t register_list = instruction & 0x7F;
                        uint32_t registers = register_list << 7;

                        if (std::bitset<32>(registers).count() < 1)
                        {
                            return;
                        }   


                        uint32_t address = this->getSP();


 
                        for (uint8_t i = 0; i < 8 ; i++)
                        {
                            // Isolate the ith bit
        
                            if (registers & (0b1 << i))
                            {
                                this->regs[i] = this->read32(address);
                                address += 4;
                            }
                        }

                         if (registers & (0b1 << 15))
                         {

                         }
                    }

  

                    break;
                }
                case 0b10100:
                {
                    uint8_t imm8 = instruction & 0xFF;
                    uint8_t d = (instruction >> 8) & 0b111;
                    uint32_t imm32 = ((uint32_t) imm8) << 2;
                    uint32_t result = this->regs[13] + imm32;
                    this->regs[d] = result;


                    break;
                }


                // ADD (SP plus immediate);
                // Encoding T1
                case 0b10101:
                {
                    uint8_t imm8 = instruction & 0xFF;
                    uint8_t d = (instruction >> 8) & 0b111;

                    uint32_t imm32 = ((uint32_t) imm8) << 2;
                    uint32_t SP = this->regs[13];

                    auto result = this->addWithCarry(SP, imm32, 0);
                    this->regs[d] = result.result;

                }

                // LDRH (Immediate)
                case 0b10001:
                {
                    uint8_t t = (instruction) & 0b111;
                    uint8_t n = (instruction >> 3) & 0b111;
                     uint8_t imm5 = (instruction >> 6) & 0b11111;
                     uint32_t imm32 = (uint32_t) (imm5 << 1);

                    bool index = true;
                    bool add = true;
                    bool wback = false;

                    uint32_t Rn = this->regs[n];

                    uint32_t offset_addr =  add ? Rn + imm32 : Rn - imm32;
                    uint32_t address = index ? offset_addr : Rn;
                    
                    uint32_t data = (uint32_t) this->read16(address);

                    if (wback)
                    {
                        this->regs[n] = offset_addr;
                    }

                    this->regs[t] = data;

                    break;
                }

                // LDR register
                case 0b01011:
                {
                    
                    uint8_t t = (instruction) & 0b111;
                    uint8_t n = (instruction >> 3) & 0b111;
                    uint8_t m = (instruction >> 6) & 0b111;


                    uint8_t tenNine = (instruction >> 9) & 0b11;
                    
                    bool index = true;
                    bool add = true;
                    bool wback = false;

                    auto offset = this->shift(this->regs[m], SRType_LSL, 0, aspr.C);

                    uint32_t Rn = this->regs[n];
                    uint32_t offset_addr =  add ? Rn + offset : Rn - offset;
                    uint32_t address = index ? offset_addr : Rn;

                    switch(tenNine)
                    {
                        case 0b00:
                            this->regs[t] = this->read32(address);
                            break;

                        case 0b10:
                            this->regs[t] = (uint32_t) this->read8(address);
                            break;

                        case 0b01:
                             this->regs[t] = (uint32_t) this->read16(address);
                            break;

                        
                        case 0b11:
                             this->regs[t] = sign_extend(this->read8(address), 32);
                            break;

                        default:

                            break;
                    }

                    if (wback)
                    {
                        this->regs[n] = offset_addr;
                    }

                    break;
                }

                // LDRB
                case 0b01111:
                {
                    uint8_t t = (instruction) & 0b111;
                    uint8_t n = (instruction >> 3) & 0b111;
                    uint8_t imm5 = (instruction >> 6) & 0b11111;

                    uint32_t imm32 = (uint32_t) imm5;

                    bool index = true;
                    bool add = true;
                    bool wback = false;

                    uint32_t Rn = this->regs[n];

                    uint32_t offset_addr =  add ? Rn + imm32 : Rn - imm32;
                    uint32_t address = index ? offset_addr : Rn;
                    
                    this->regs[t] = (uint32_t) this->read8(address);

                    if (wback)
                    {
                        this->regs[n] = offset_addr;
                    }

                    break;
                }
                // LDR literal
                case 0b01001:
                {
                    uint8_t t = (instruction >> 8) & 0b111;
                    uint8_t imm8 = (instruction) & 0xFF;
                    uint32_t imm32 = (uint32_t)(imm8 << 2); 
                    uint32_t base = this->regs[15] & ~0x3;
                    bool add = true;

                    uint32_t address = add ? base + imm32 : base - imm32;
                    
                    this->regs[t] = read32(address);

                    break;
                }
                // LDR (immediate) T2
                case 0b10011:
                {
                    uint8_t t = (instruction >> 8) & 0b111;
                     uint8_t imm8 = (instruction) & 0xFF;
                    uint32_t imm32 = (uint32_t)(imm8 << 2); 
                    uint8_t n = 13;
                    bool index = true;
                    bool add = true;
                    bool wback = false;

                    uint32_t Rn = this->regs[n];

                    uint32_t offset_addr =  add ? Rn + imm32 : Rn - imm32;
                    uint32_t address = index ? offset_addr : Rn;
                    
                    this->regs[t] = read32(address);

                    if (wback)
                    {
                        this->regs[n] = offset_addr;
                    }
                    break;
                }
                // LDR (Immediate) T1
                case 0b01101:
                {
                    uint8_t t = (instruction) & 0b111;
                    uint8_t n = (instruction >> 3) & 0b111;

                    uint8_t imm5 = (instruction >> 6) & 0b11111;

                    uint32_t imm32 = (uint32_t)(imm5 << 2); 

                    

                    uint32_t Rn = this->regs[n];

                    bool index = true;
                    bool add = true;
                    bool wback = false;

                    uint32_t offset_addr =  add ? Rn + imm32 : Rn - imm32;
                    uint32_t address = index ? offset_addr : Rn;
                    
                    this->regs[t] = read32(address);

                    if (wback)
                    {
                        this->regs[n] = offset_addr;
                    }

                    break;
                }

                // ADD (SP plus immediate);
                // Encoding T1
                case 0b10110:
                {
                    uint8_t imm7 = instruction & 0b1111111;
                    uint32_t imm32 = ((uint32_t) imm7 << 2);
                    uint32_t SP = this->regs[13];
                    auto result = this->addWithCarry(SP, imm32, 0);
                    this->regs[13] = result.result;
                    break;
                }

                // ADD (SP plus register) Encoding T1

                case 0b10000:
                {
                    uint8_t type = (instruction >> 6) & 0b1111;
                    uint8_t rd = instruction & 0b111;
                    uint8_t rm = (instruction >> 3) & 0b111;
                    uint8_t carry = 0;

                    uint32_t & RD = this->regs[rd];
                    uint32_t & RM = this->regs[rm];

                    // switch(type)
                    // {
                    //     case 0b1010:

                    // }
                    break;
                }

                case 0b00010:
                {
                    
                }
                // LSR (Immediate)
                case 0b00001:
                {
                    uint8_t d = instruction & 0b111;
                    uint8_t m = (instruction >> 3) & 0b111; 
                    uint8_t imm5 = (instruction >> 6) & 0b11111;
                    
                    DecodeImmShiftResult result = this->decodeImmShift(01, imm5);

                    uint32_t Rm = this->regs[m];

                    auto shifted = this->shift_c(Rm, SRType_LSR, result.n, aspr.C);

                    this->regs[d] = shifted.result;

                    this->aspr.N = shifted.result >> 31;
                    this->aspr.C = shifted.carry_out;
                    this->aspr.Z = shifted.result == 0x0;
                    break;
                }


                // MOV (Immediate)
                case 0b00100:
                {

                    // print()
                    printf("MOV immediate\n");
                    uint8_t imm8 = (instruction) & 0xFF;
                    uint8_t d = (instruction >> 8) & 0b111;
                    
                    // printf("destination reg %d\n", d);
                    std::cout << "Binary: " << std::bitset<16>(instruction) << std::endl;
                    uint32_t imm32 = (uint32_t) imm8;

                    uint32_t result = imm32;

                    this->regs[d] = result;
                    this->aspr.N = result >> 31;
                    this->aspr.Z = result == 0x0;


                    break;
                }

    
                // LSL
                case 0b00000:
                {
                    uint8_t imm5 = (instruction >> 6) & 0b11111;
                    uint8_t d = instruction & 0b111;
                    uint8_t m = (instruction >> 3) & 0b111;
                    uint8_t carry = 0;

                    uint32_t Rm = this->regs[m];

                    auto result = this->decodeImmShift(0b00, imm5);

                    if (imm5 == 0x0)
                    {
                        // MOV (register) T2
                        uint8_t m = (instruction >> 3) & 0b111;
                        uint8_t d = (instruction ) & 0b111;
        
                        uint32_t result = regs[m];

                        if (d == 15)
                        {
                            this->regs[15] = result;
                        }
                        else
                        {
                            this->regs[d] = result;
                            this->aspr.N = result >> 31;
                            this->aspr.Z = result == 0x0;
                        }
                    }
                    else
                    {
                        auto shifted = this->shift_c(Rm, SRType_LSL, result.n, aspr.C);

                        this->regs[d] = shifted.result;

                        this->aspr.N = shifted.result >> 31;
                        this->aspr.C = shifted.carry_out;
                        this->aspr.Z = shifted.result == 0x0;
                        /* code */
                    }
                    break;
                }
                default:
                    break;
            }


            this->regs[15] += 2;
        }

    void print_state(void) const
    {
        printf("\n--- CPU STATE ---\n");

        // Print General Purpose Registers in a 4x4 Grid
        for(uint8_t i = 0; i < 16; i++)
        {
            printf("r%-2d: 0x%08X    ", i, this->regs[i]);
            
            // Every 4 registers, print a newline to maintain the grid
            if ((i + 1) % 4 == 0) {
                printf("\n");
            }
        }

        // Print Application Program Status Register (APSR) Flags
        // N (Negative), Z (Zero), C (Carry), V (Overflow)
        printf("-----------------\n");
        printf("FLAGS: [ N:%d | Z:%d | C:%d | V:%d ]\n", 
                this->aspr.N, this->aspr.Z, this->aspr.C, this->aspr.V);
        printf("-----------------\n\n");
    }

    private:
        uint32_t regs[16];

        uint8_t ram[0x1000];
        ASPR aspr;
        EPSR espr;

        
};



// Minimal ELF32 manual parser for Cortex-M0 emulator
// Loads PT_LOAD segments into emulator memory

#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <elf.h>

class Emulator {
public:
    std::vector<uint8_t> memory;
    uint32_t sp = 0;
    uint32_t pc = 0;

    Emulator(size_t mem_size = 1024 * 1024)
        : memory(mem_size, 0) {}

    uint32_t read32(uint32_t addr) const {
        return memory[addr]
            | (memory[addr + 1] << 8)
            | (memory[addr + 2] << 16)
            | (memory[addr + 3] << 24);
    }

    void load_elf(const std::string& path) {
        std::ifstream file(path, std::ios::binary);
        if (!file)
            throw std::runtime_error("Failed to open ELF");

        // Read ELF header
        Elf32_Ehdr ehdr{};
        file.read(reinterpret_cast<char*>(&ehdr), sizeof(ehdr));

        if (!file)
            throw std::runtime_error("Failed reading ELF header");

        // Validate ELF magic
        if (std::memcmp(ehdr.e_ident, ELFMAG, SELFMAG) != 0)
            throw std::runtime_error("Not an ELF file");

        if (ehdr.e_ident[EI_CLASS] != ELFCLASS32)
            throw std::runtime_error("Not ELF32");

        if (ehdr.e_machine != EM_ARM)
            throw std::runtime_error("Not ARM ELF");

        std::cout << "Entry point: 0x" << std::hex << ehdr.e_entry << "\n";

        // Read program headers
        file.seekg(ehdr.e_phoff);

        for (int i = 0; i < ehdr.e_phnum; i++) {
            Elf32_Phdr phdr{};
            file.read(reinterpret_cast<char*>(&phdr), sizeof(phdr));

            if (phdr.p_type != PT_LOAD)
                continue;

            std::cout << "Loading segment " << i
                      << " addr=0x" << std::hex << phdr.p_vaddr
                      << " filesz=" << std::dec << phdr.p_filesz
                      << " memsz=" << phdr.p_memsz << "\n";

            if (phdr.p_vaddr + phdr.p_memsz > memory.size())
                throw std::runtime_error("Segment exceeds memory");

            std::streampos current = file.tellg();

            // Read segment data
            file.seekg(phdr.p_offset);
            file.read(reinterpret_cast<char*>(&memory[phdr.p_vaddr]),
                      phdr.p_filesz);

            // Zero-fill .bss region
            if (phdr.p_memsz > phdr.p_filesz) {
                std::memset(&memory[phdr.p_vaddr + phdr.p_filesz],
                            0,
                            phdr.p_memsz - phdr.p_filesz);
            }

            file.seekg(current);
        }

        // Cortex-M boot from vector table
        sp = read32(0x00000000);
        pc = read32(0x00000004);

        std::cout << "Initial SP: 0x" << std::hex << sp << "\n";
        std::cout << "Reset PC : 0x" << std::hex << pc << "\n";
    }
};



int main(void)
{
 

    // std::cout << std::numeric_limits<int32_t>::max() << std::numeric_limits<int32_t>::min() << std::endl;
 Emulator emu;
        emu.load_elf("../src/prog.elf");
    uint32_t test = 0b1111;


    std::bitset<32> test2 = std::bitset<32>(test);

   
  Cpu cpu;
  uint32_t load_address = 0x0000;
    // Store opcodes into memory (little-endian)
    uint32_t program[] =
    {
        // addr 0x0000:
        // 0x2005 = MOVS r0,#5
        // 0x2103 = MOVS r1,#3
        0x21032005,

        // addr 0x0004:
        // 0x1842 = ADDS r2,r0,r1
        // 0x1A43 = SUBS r3,r0,r1
        0x1A431842,

        // addr 0x0008:
        // 0x4048 = ANDS r0,r1
        // 0x430A = ORRS r2,r1
        0x430A4048,

        // addr 0x000C:
        // 0x4059 = EORS r1,r3
        // 0xE7FE = B .
        0xE7FE4059
    };

    // Example loading into RAM
    for (int i = 0; i < 4; i++)
    {
        uint32_t word = program[i];
        cpu.flash[i * 4 + 0] = word & 0xFF;
        cpu.flash[i * 4 + 1] = (word >> 8) & 0xFF;
        cpu.flash[i * 4 + 2] = (word >> 16) & 0xFF;
        cpu.flash[i * 4 + 3] = (word >> 24) & 0xFF;
    }

    
    printf("%LX\n", cpu.flash[0]);
        // Print memory contents
    std::cout << "Loaded machine code:\n";

    // for (uint32_t i = 0; i < load_address; i += 2)
    // {
        cpu.decode();
        
        cpu.print_state();

        cpu.decode();
        
        cpu.print_state();

        cpu.decode();
        
        cpu.print_state();

    // }

    // printf("%d\n", test2.count());
  
    // cpu.decode(0100000001001000);




}