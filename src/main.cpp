#include <iostream>
#include <vector>
#include <limits>
#include <bitset>

using bit = bool;
constexpr uint8_t PC_INDEX = 15;

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
                case 0b11110:
                case 0b11111:
                    // 32 bit thumb mode
                    // 
                    break;
                case 0b01000:
                {

                    std::cout << "WORKING!\n";
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


                case 0b10101:
                {
                    uint8_t imm8 = instruction & 0xFF;
                    uint8_t d = (instruction >> 8) & 0b111;

                    uint32_t imm32 = (uint32_t) imm8;
                }

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

        
};




int main(void)
{
 

    // std::cout << std::numeric_limits<int32_t>::max() << std::numeric_limits<int32_t>::min() << std::endl;


    Cpu cpu;
    cpu.decode(0b0100000000000000);
}