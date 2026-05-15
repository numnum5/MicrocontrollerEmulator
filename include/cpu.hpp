#include <iostream>
#include <vector>
#include <limits>
#include <bitset>
#include <bit>
#include "registers.hpp"

using bit = bool;

enum class InstrClass {
    SHIFT_IMM,        // LSL, LSR, ASR
    ADD_SUB,          // add/sub (reg + imm3)
    MOV_CMP_ADD_SUB,  // imm8 ops
    ALU,              // AND, ORR, EOR, etc.
    HI_REG,           // high register ops + BX
    LDR_LITERAL,      
    LOAD_STORE_REG,
    LOAD_STORE_IMM,
    LOAD_STORE_HALF,
    SP_REL,
    ADDR,
    MISC,
    MULTIPLE,
    COND_BRANCH,
    SVC,
    UNCOND_BRANCH,
    UNKNOWN
};

typedef enum 
{
    SEV,
    NOP,
    SVC,
    CPS,
    SUBADD,
    PUSHPOP,
    REV,
    EXTEND

} Misc_type;

typedef enum 
{
    MSR,
    MRS,
    UDF,
    BL

} Branch_misc_type;


typedef enum 
{
    SRType_LSL,
    SRType_LSR,
    SRType_ASR,
    SRType_ROR,
    SRType_RRX

} SRType;

constexpr uint8_t PC_INDEX = 15;


typedef struct 
{
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

struct ShiftImmediateInstruction
{
    uint16_t rd : 3;
    uint16_t rm : 3;
    uint16_t reserved : 5;
    uint16_t identifier : 5;
    ShiftImmediateInstruction() = default;
    ShiftImmediateInstruction(uint16_t instr)
    {
        rd = instr & 0x7;
        rm = (instr >> 3) & 0x7;
        reserved = (instr >> 6) & 0x1F;
        identifier = (instr >> 11) & 0x1F;
    }
};

struct SpecialInstruction
{
    uint16_t bits_0_2 : 3;
    uint16_t rm : 3;
    uint16_t bits_7 : 1;
    uint16_t opcode : 2;
    uint16_t identifier : 6;
      SpecialInstruction() = default;
    SpecialInstruction(uint16_t instr)
    {
        bits_0_2 = instr & 0x7;
        rm = (instr >> 3) & 0x7;
        bits_7 = (instr >> 6) & 0x1;
        opcode = (instr >> 7) & 0x3;
        identifier = (instr >> 9) & 0x3F;
    }
};

struct MovCmpAddSubInstruction
{
    uint16_t imm8 : 8;
    uint16_t d : 3;
    uint16_t op : 2;
    uint16_t identifier: 3;
      MovCmpAddSubInstruction() = default;
    MovCmpAddSubInstruction(uint16_t instr)
    {
        imm8 = instr & 0xFF;
        d = (instr >> 8) & 0x7;
        op = (instr >> 11) & 0x3;
        identifier = (instr >> 13) & 0x7;
    }
};

struct LoadStoreRegInstruction 
{
    uint16_t rt : 3;
    uint16_t rn : 3;
    uint16_t rm : 3;
    uint16_t opcode : 3;
    uint16_t identifier : 4;
  LoadStoreRegInstruction() = default;
    LoadStoreRegInstruction(uint16_t instr)
    {
        rt = instr & 0x7;
        rn = (instr >> 3) & 0x7;
        rm = (instr >> 6) & 0x7;
        opcode = (instr >> 9) & 0x7;
        identifier = (instr >> 12) & 0xF;
    }
};

struct LDRLiteralInstruction 
{
    uint16_t imm8 : 8;
    uint16_t rt : 3;
    uint16_t identifier : 5;
  LDRLiteralInstruction() = default;
    LDRLiteralInstruction(uint16_t instr)
    {
        imm8 = instr & 0xFF;
        rt = (instr >> 8) & 0x7;
        identifier = (instr >> 11) & 0x1F;
    }
};

struct AddSubImmediateInstruction
{
    uint16_t rd : 3;
    uint16_t rn : 3;
    uint16_t imm3 : 3;
    uint16_t bits_9 : 1;
    uint16_t bits_10 : 1;
    uint16_t identifier : 5;
    AddSubImmediateInstruction() = default;
    AddSubImmediateInstruction(uint16_t instruction)
    {
        rd         = (instruction >> 0)  & 0x7;
        rn         = (instruction >> 3)  & 0x7;
        imm3       = (instruction >> 6)  & 0x7;
        bits_9     = (instruction >> 9)  & 0x1;
        bits_10    = (instruction >> 10) & 0x1;
        identifier = (instruction >> 11) & 0x1F;
    }
};


struct ALUInstruction
{
    uint16_t rdn : 3;
    uint16_t rm : 3;
    uint16_t opcode : 4;
    uint16_t identifier : 6;
    ALUInstruction() = default;
    ALUInstruction(uint16_t instr)
    {
        rdn = instr & 0x7;
        rm = (instr >> 3) & 0x7;
        opcode = (instr >> 6) & 0xF;
        identifier = (instr >> 10) & 0x3F;
    }
};

struct SpRelativeInstruction
{
    uint16_t imm8 : 8;
    uint16_t rt : 3;
    uint16_t opcode : 1;
    uint16_t identifier : 4;
    SpRelativeInstruction() = default;
    SpRelativeInstruction(uint16_t instr)
    {
        imm8 = instr & 0xFF;
        rt = (instr >> 8) & 0x7;
        opcode = (instr >> 11) & 0x1;
        identifier = (instr >> 12) & 0xF;
    }
};

struct AdrInstruction
{
    uint16_t imm8 : 8;
    uint16_t rd : 3;
    uint16_t bits_11 : 1;
    uint16_t identifier : 4;
    AdrInstruction() = default;
    AdrInstruction(uint16_t instr)
    {
        imm8 = instr & 0xFF;
        rd = (instr >> 8) & 0x7;
        bits_11 = (instr >> 11) & 0x1;
        identifier = (instr >> 12) & 0xF;
    }
};

struct LoadStoreImmInstruction
{
    uint16_t rt : 3;
    uint16_t rn : 3;
    uint16_t imm5 : 5;
    uint16_t opcode : 2;
    uint16_t identifier : 3;
    LoadStoreImmInstruction() = default;
    LoadStoreImmInstruction(uint16_t instr)
    {
        rt = instr & 0x7;
        rn = (instr >> 3) & 0x7;
        imm5 = (instr >> 6) & 0x1F;
        opcode = (instr >> 11) & 0x3;
        identifier = (instr >> 13) & 0x7;
    }
};

struct LoadStoreHalfInstruction
{
    uint16_t rt : 3;
    uint16_t rn : 3;
    uint16_t imm5 : 5;
    uint16_t opcode : 1;
    uint16_t identifier : 4;
    LoadStoreHalfInstruction() = default;
    LoadStoreHalfInstruction(uint16_t instr)
    {
        rt = instr & 0x7;
        rn = (instr >> 3) & 0x7;
        imm5 = (instr >> 6) & 0x1F;
        opcode = (instr >> 11) & 0x1;
        identifier = (instr >> 12) & 0xF;
    }
};

struct CPSInstruction
{
    uint16_t bits_0_3 : 4;
    uint16_t im : 1;
    uint16_t opcode : 4;
    uint16_t identifier : 4;
    CPSInstruction() = default;
    CPSInstruction(uint16_t instr)
    {
        bits_0_3 = instr & 0xF;
        im = (instr >> 4) & 0x1;
        opcode = (instr >> 5) & 0xF;
        identifier = (instr >> 9) & 0xF;
    }
};

struct AddSubSpInstruction
{
    uint16_t imm7 : 7;
    uint16_t bits_7 : 1;
    uint16_t opcode : 4;
    uint16_t identifier : 4;
    AddSubSpInstruction() = default;
    AddSubSpInstruction(uint16_t instr)
    {
        imm7 = instr & 0x7F;
        bits_7 = (instr >> 7) & 0x1;
        opcode = (instr >> 8) & 0xF;
        identifier = (instr >> 12) & 0xF;
    }
};

struct PushPopInstruction
{
    uint16_t register_list : 8;
    uint16_t M : 1;
    uint16_t bits_10_9 : 2;
    uint16_t op : 1;
    uint16_t identifier : 4;
    PushPopInstruction() = default;
    PushPopInstruction(uint16_t instr)
    {
        register_list = instr & 0xFF;
        M = (instr >> 8) & 0x1;
        bits_10_9 = (instr >> 9) & 0x3;
        op = (instr >> 11) & 0x1;
        identifier = (instr >> 12) & 0xF;
    }
};

struct LoadMultipleInstruction
{
    uint16_t register_list : 8;
    uint16_t rn : 3;
    uint16_t op : 1;
    uint16_t identifier : 4;
    LoadMultipleInstruction() = default;
    LoadMultipleInstruction(uint16_t instr)
    {
        register_list = instr & 0xFF;
        rn = (instr >> 8) & 0x7;
        op = (instr >> 11) & 0x1;
        identifier = (instr >> 12) & 0xF;
    }
};

struct ExtendInstruction 
{
    uint16_t rd : 3;
    uint16_t rm : 3;
    uint16_t op : 2;
    uint16_t bits_11_8 : 4;
    uint16_t identifier : 4;
    ExtendInstruction() = default;
    ExtendInstruction(uint16_t instr)
    {
        rd = instr & 0x7;
        rm = (instr >> 3) & 0x7;
        op = (instr >> 6) & 0x3;
        bits_11_8 = (instr >> 8) & 0xF;
        identifier = (instr >> 12) & 0xF;
    }
};

struct RevInstruction
{
    uint16_t rd : 3;
    uint16_t rm : 3;
    uint16_t op : 2;
    uint16_t bits_11_8 : 4;
    uint16_t identifier : 4;
    RevInstruction() = default;
    RevInstruction(uint16_t instr)
    {
        rd = instr & 0x7;
        rm = (instr >> 3) & 0x7;
        op = (instr >> 6) & 0x3;
        bits_11_8 = (instr >> 8) & 0xF;
        identifier = (instr >> 12) & 0xF;
    }
};

struct BranchCondInstruction 
{
    uint16_t imm8 : 8;
    uint16_t cond : 4;
    uint16_t identifier : 4;

    BranchCondInstruction() = default;
    
    BranchCondInstruction(uint16_t instr)
    {
        imm8 = instr & 0xFF;
        cond = (instr >> 8) & 0xF;
        identifier = (instr >> 12) & 0xF;
    }
};

struct BranchUncondInstruction 
{
    uint16_t imm11 : 11;
    uint16_t identifier : 5;

    BranchUncondInstruction() = default;
    
    BranchUncondInstruction(uint16_t instr)
    {
        imm11 = instr & 0x7FF;
        identifier = (instr >> 11) & 0x1F;
    }
};

enum InstructionClassThumb2
{
    BRANCH_MISC
};



union Instruction
{
    struct ShiftImmediateInstruction shiftImmediateInstruction;
    struct SpecialInstruction _SpecialInstruction;
    struct MovCmpAddSubInstruction movCmpAddSubInstruction;
    struct LoadStoreRegInstruction loadStoreRegInstruction;
    struct LDRLiteralInstruction lDRLiteralInstruction;
    struct ALUInstruction _ALUInstruction;
    struct SpRelativeInstruction SpRelativeInstruction;
    struct AdrInstruction adrInstruction;
    struct LoadStoreImmInstruction loadStoreImmInstruction;
    struct LoadStoreHalfInstruction loadStoreHalfInstruction;
    struct CPSInstruction cPSInstruction;


    struct AddSubSpInstruction _AddSubSpInstruction;
    struct PushPopInstruction _PushPopInstruction;
    struct LoadMultipleInstruction _LoadMultipleInstruction;
    struct ExtendInstruction _ExtendInstruction;
    struct RevInstruction _RevInstruction;

    struct BranchCondInstruction _BranchCondInstruction;
    struct BranchUncondInstruction _BranchUncondInstruction;
    struct AddSubImmediateInstruction _AddSubImmediateInstruction;
};


typedef struct
{
    uint32_t result;
    bool carry_out;
} Shift_c;

enum class Mode
{
    MODE_HANDLER,
    MODE_THREAD
};

struct Control
{
    uint32_t nPRIV : 1;
    uint32_t SPSEL : 1;
    uint32_t reserved : 30;
};

union XPSR
{
    uint32_t value;

    struct 
    {
    // IPSR
        uint32_t ispr : 5;

        // EPSR reserved / ICI bits
        uint32_t _reserved0 : 3;

        // EPSR ICI/IT bits
        uint32_t a : 1;

        uint32_t _reserved1 : 15;
        // Thumb state bit
        uint32_t T : 1;

        uint32_t _reserved2 : 3;

        // APSR flags
        union
        {
            uint32_t apsr : 4;

            struct
            {
                uint32_t V : 1;
                uint32_t C : 1;
                uint32_t Z : 1;
                uint32_t N : 1;
            };
        };
    };
    

};

struct BranchLinkInstruction
{
    // first 16-bit halfword
    uint32_t imm11 : 11;   // bits [10:0]
    uint32_t J2    : 1;    // bit 11
    uint32_t one2  : 1;    // bit 12 (should be 1)
    uint32_t J1    : 1;    // bit 13
    uint32_t op2  : 2;    // bits [15:14] (should be 11)

    // second 16-bit halfword
    uint32_t imm10 : 10;   // bits [25:16]
    uint32_t S     : 1;    // bit 26
    uint32_t op1    : 5;    // bits [31:27] (should be 11110)

    BranchLinkInstruction() = default;
    
    BranchLinkInstruction(uint32_t instruction)
    {
        uint16_t first  = instruction & 0xFFFF;
        uint16_t second = instruction >> 16;

        this->op1 = (instruction >> 20) & 0x7F;
        this->op2 = (instruction >> 12) & 0b111;
        this->S  = (first >> 10) & 1;
        this->imm10 = first & 0x03FF;
        this->J1 = (second >> 13) & 1;
        this->J2 = (second >> 11) & 1;
        this->imm11 = second & 0x07FF;
    }
};

union Thumb2Instruction
{

    BranchLinkInstruction _BranchLinkInstruction;
};

enum InstructionType
{
    THUMB1,
    THUMB2
};

class Cpu
{
    public:
        static constexpr uint32_t FLASH_BASE = 0x00000000;
        static constexpr uint32_t FLASH_SIZE = 64 * 1024;

        static constexpr uint32_t RAM_BASE   = 0x20000000;
        static constexpr uint32_t RAM_SIZE   = 16 * 1024;
        // uint8_t flash[0x1000];

        std::vector<uint8_t> flash;
        std::vector<uint8_t> ram;
        Registers registers;
        uint32_t regs[16];
        // uint8_t ram[0x1000];
        ASPR aspr;
        EPSR espr;
        // uint32_t xpsr;
        uint32_t msp;
        uint32_t psp;
        uint32_t primask;
        // uint32_t control;
        Control control;
        Mode currentMode;

        Thumb2Instruction decodedThumb2Instruction;
        
        Instruction decodedInstruction;
        
        
        InstrClass instructionClass;
        
        InstructionClassThumb2 thumb2Class;
        
        DecodeImmShiftResult decodeImmShiftResult;
        
        Misc_type misc_type;

        Branch_misc_type branch_misc_type;
        
        XPSR xpsr;

        InstructionType instructionType;

        uint32_t fetched_instruction;

        Cpu(size_t ram_size, size_t flash_size);

        uint32_t getSP(void) const;
        void setPrimaskPM(bool value);
        bool conditionPassed(uint8_t cond) const;
        void write32(uint32_t address, uint32_t value);
        void write16(uint32_t address, uint16_t value);
        void write8(uint32_t address, uint8_t value);
        uint8_t read8(uint32_t address) const;
        uint16_t read16(uint32_t address) const;
        uint32_t read32(uint32_t address) const;
        uint32_t read32v2(uint32_t address) const;
        uint32_t read32Flash(uint32_t address) const;
        InstrClass classify(uint16_t instr);
        bool currentModeIsPrivileged(void);
        
        void handleSpecialInstructions(uint16_t instruction);

        uint8_t currentCond(uint32_t instruction);
        uint32_t shift(uint32_t value, SRType type, uint8_t amount, bit carry_in);
        uint32_t sign_extend(uint32_t value, int bits);
        DecodeImmShiftResult decodeImmShift(uint8_t type, uint8_t imm5);
        Shift_c shift_c(uint32_t value, SRType type, uint8_t amount, bit carry_in);
        AddCarryResult addWithCarry(uint32_t x, uint32_t y, bool carry_in);

        bool inITBlock(void);
        bool is32bitInstruction(uint8_t thumb_mode);
        void HandleALUinstr(uint16_t instruction);
        void handleLoadStoreHalf(uint16_t instr);
        void handleLoadStoreImm(uint16_t instr);
        void handleLDRLiteral(uint16_t instruction);
        void handleMultiple(uint16_t instr);
        void handleMovCmpAddSub(uint16_t instr);
        void handleUncondBranch(uint16_t instr);
        void handleCondBranch(uint16_t instr);
        void handleAddSub(uint16_t instr);
        void handleShiftImmediate(uint16_t instr);
        void handleMisc(uint16_t instr);
        void handleAdr(uint16_t instr);
        void handleLoadStoreReg(uint16_t instr);
        void handleSpRelative(uint16_t instr);

        void executeMisc(void);
        void executeAdr(void);
        void executeLoadStoreReg(void);
        void executeSpRelative(void);
        void executeALUinstr(void);
        void executeLoadStoreHalf(void);
        void executeLoadStoreImm(void);
        void executeLDRLiteral(void);
        void executeMultiple(void);
        void executeMovCmpAddSub(void);
        void executeUncondBranch(void);
        void executeCondBranch(void);
        void executeAddSub(void);
        void executeShiftImmediate(void);
        void executeSpecialInstructions(void);    

        std::pair<uint32_t, bool> LSL_C(uint32_t x, int shift);
        std::pair<uint32_t, bool> ROR_C(uint32_t x, int shift);
        std::pair<uint32_t, bool> LSR_C(uint32_t x, int shift);
        std::pair<uint32_t, bool> ASR_C(uint32_t x, int shift);
        std::pair<uint32_t, bool> RRX_C(uint32_t x, bool carry_in);
    
        uint32_t RRX(uint32_t x, bool carry_in);
        uint32_t ASR(uint32_t x, int shift);
        uint32_t LSR(uint32_t x, int shift);
        uint32_t ROR(uint32_t x, int shift);
        uint32_t LSL(uint32_t x, int shift);

enum ExceptionType
{
    EXCEPTION_RESET     = 1,
    EXCEPTION_NMI       = 2,
    EXCEPTION_HARDFAULT = 3,
    EXCEPTION_SVCALL    = 11,
    EXCEPTION_PENDSV    = 14,
    EXCEPTION_SYSTICK   = 15
};

uint32_t nextInstrAddr;
uint32_t currentInstrAddr;
bool synchronous_fault;

uint32_t returnAddress(int exceptionType)
{
    uint32_t result = 0;

    switch (exceptionType)
    {
        case EXCEPTION_NMI:
        {
            result = nextInstrAddr;
            break;
        }

        case EXCEPTION_HARDFAULT:
        {
            if (synchronous_fault)
                result = currentInstrAddr;
            else
                result = nextInstrAddr;

            break;
        }

        case EXCEPTION_SVCALL:
        case EXCEPTION_PENDSV:
        case EXCEPTION_SYSTICK:
        {
            result = nextInstrAddr;
            break;
        }

        default:
        {
            // External interrupts
            if (exceptionType >= 16)
            {
                result = nextInstrAddr;
            }
            else
            {
                assert(false && "Unknown exception number");
            }
            break;
        }
    }

    // Return address must always be halfword aligned
    result &= ~1u;

    return result;
}

    void pushStack(int ExceptionType)
    {
        uint32_t frameptr;
        uint32_t frameptralign;

        // Select active stack pointer
        if (this->control.SPSEL && this->currentMode == Mode::MODE_HANDLER)
        {
            // Save original alignment bit
            frameptralign = (this->psp >> 2) & 1;
            // Allocate 0x20 bytes and align to 8-byte boundary
            this->psp = (this->psp - 0x20) & ~0x7u;

            frameptr = this->psp;
        }
        else
        {
            frameptralign = (this->msp >> 2) & 1;

            this->msp = (this->msp - 0x20) & ~0x7u;

            frameptr = this->msp;
        }

        // Hardware-defined stack frame layout
        write32(frameptr + 0x00, regs[0]);
        write32(frameptr + 0x04, regs[1]);
        write32(frameptr + 0x08, regs[2]);
        write32(frameptr + 0x0C, regs[3]);
        write32(frameptr + 0x10, regs[12]);
        write32(frameptr + 0x14, regs[14]); // LR
        write32(frameptr + 0x18, returnAddress(ExceptionType));

        // Insert alignment bit into stacked xPSR bit 9
        uint32_t stacked_xpsr = (this->xpsr.value & ~(1 << 9)) |(frameptralign << 9);

        write32(frameptr + 0x1C, stacked_xpsr);

        if (this->currentMode == Mode::MODE_HANDLER)
        {
            regs[14] = 0xFFFFFFF1;
        }
        else
        {
            if (this->control.SPSEL == 0)
            {
                regs[14] = 0xFFFFFFF9;
            }
            else
            {
                regs[14] = 0xFFFFFFFD;
            }
        }
    }

        bool exceptionActive[48];

    void exceptionTaken(int32_t exceptionNumber)
    {

        for (int i = 0; i <= 3; i++)
        {
            regs[i] = 0;
        }

        regs[12] = 0;

        this->xpsr.apsr = 0;

        // Enter Handler mode
        currentMode = Mode::MODE_HANDLER;

        // IPSR<5:0> = ExceptionNumber<5:0>
        this->xpsr.ispr = exceptionNumber & 0x3F;

        // Use Main Stack Pointer
        control.SPSEL = 0;

        // CONTROL.nPRIV unchanged

        // Mark exception active
        exceptionActive[exceptionNumber] = true;

        // Update system control state
        // SCS_UpdateStatusRegs();

        // Wake Event Register
        // SetEventRegister();

        // Instruction Synchronization Barrier
        // InstructionSynchronizationBarrier(0xF);

        // Load vector table base
        uint32_t vectorTable = 0x0;

        // Read handler address from vector table
        uint32_t handler = read32v2(vectorTable + (4 * exceptionNumber));

        // Branch to handler
        BLXWritePC(handler);
    
    }

    void BLXWritePC(uint32_t address)
    {   
        this->xpsr.T == address & 0b1;
        this->regs[15] = address;
    }


    void exceptionEntry(int32_t ExceptionType)
    {
        uint16_t frameptraling = 0;
        if (this->control.SPSEL == 1 && this->currentMode == Mode::MODE_THREAD)
        {
            frameptraling = this->psp;
        }
        // NOTE: PushStack() can abandon memory accesses if a fault occurs during the stacking
        // sequence.
        // Exception entry is modified according to the behavior of a derived exception.
        pushStack(ExceptionType);
        exceptionTaken(ExceptionType); // ExceptionType is encoded as its exception number
    }

        uint8_t exception_request;
        uint8_t exception_number;
        void BXWritePC(uint32_t address);
        // The 3 stages of the CPU processing
        void fetch(void);
        void decode(void);
        void execute(void);
        void print_state(void) const;

    private:

};
