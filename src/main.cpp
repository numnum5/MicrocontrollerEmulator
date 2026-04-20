#include <iostream>
#include <vector>
#include <limits>
#include <bitset>

using bit = bool;
constexpr uint8_t PC_INDEX = 15;

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

struct Shift_c
{
    uint32_t result;
    bool carry_out;
};


typedef enum {
    SRType_LSL,
    SRType_LSR,
    SRType_ASR,
    SRType_ROR,
    SRType_RRX

} SRType;

class Cpu
{
    public:
        Cpu()
        {

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

        void decode(uint32_t instruction)
        {
            



            // uint32_t instruction = this->fetch();
            uint8_t thumb_mode = (instruction >> 11) & 0xFF;
            std::cout << std::bitset<32>(thumb_mode) << "\n";
            // std::cout << instruction << std::endl;

            // printf("%x\n", thumb_mode);
            switch(thumb_mode)
            {
                case 0b11101:
                // BL
                case 0b11110:
                {

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
                        }
                    }

                    switch(is_and)
                    {
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

                    }
               
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
                    break;
                }

                // ADD encoding T1
                case 0b00011:
                {

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


                case 0b10111:
                {
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

                case 0b00000:
                {
                    uint8_t immediate = (instruction >> 6) & 0b11111;
                    uint8_t rd = instruction & 0b111;
                    uint8_t rm = (instruction >> 3) & 0b111;
                    uint8_t carry = 0;
                    if (immediate == 0x0)
                    {
                        // mov
                    }
                    else
                    {
                        carry = (rm >> (32 - immediate)) & 1;

                        /* code */
                    }
                    break;
                }
                default:
                    break;
            }
        }


    private:
        uint32_t regs[16];
        uint8_t flash[0x1000];
        uint8_t ram[0x1000];
        ASPR aspr;
        EPSR espr;

        
};






int main(void)
{
 

    // std::cout << std::numeric_limits<int32_t>::max() << std::numeric_limits<int32_t>::min() << std::endl;


    Cpu cpu;
    cpu.decode(0b0100000000000000);
}