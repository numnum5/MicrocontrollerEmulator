
#include <stdint.h>

class SCS
{

    public:
        uint32_t SCB;
        
};

class NVIC
{

    public:
        uint32_t SYST_CSR;
        uint32_t SYST_RVR;
        uint32_t SYST_CVR;
        uint32_t SYST_CALIB;
};
class SysTick
{

    public:

        uint32_t mem_read32(uint32_t addr)
        {
            switch (addr)
            {
                case 0xE000E010:
                    return SYST_CSR;

                case 0xE000E014:
                    return SYST_RVR;

                case 0xE000E018:
                    return SYST_RVR;

                case 0xE000E01C:
                    return SYST_CALIB;
            }
        }

        void mem_write32(uint32_t addr, uint32_t value)
        {
            switch (addr)
            {
                case 0xE000E010:
                    SYST_CSR = value;
                    break;

                case 0xE000E014:
                    SYST_RVR = value;
                    break;

                case 0xE000E018:
                    SYST_CALIB = 0;
                    break;
            }
        }
        uint32_t SYST_CSR;
        uint32_t SYST_RVR;
        uint32_t SYST_CVR;
        uint32_t SYST_CALIB;
};
class SCB
{
public:

    // 0xE000ED00
    volatile uint32_t CPUID;

    // 0xE000ED04
    volatile uint32_t ICSR;

    // 0xE000ED08
    volatile uint32_t VTOR;

    // 0xE000ED0C
    volatile uint32_t AIRCR;

    // 0xE000ED10
    volatile uint32_t SCR;

    // 0xE000ED14
    volatile uint32_t CCR;

    // 0xE000ED18
    volatile uint32_t SHPR1;

    // 0xE000ED1C
    volatile uint32_t SHPR2;

    // 0xE000ED20
    volatile uint32_t SHPR3;

    // 0xE000ED24
    volatile uint32_t SHCSR;

    // 0xE000ED28
    volatile uint32_t CFSR;

    // 0xE000ED2C
    volatile uint32_t HFSR;

    // 0xE000ED30
    volatile uint32_t DFSR;

    // singleton-style access
    static SCB& get()
    {
        return *reinterpret_cast<SCB*>(0xE000ED00);
    }

public:

    void setVectorTable(uint32_t address)
    {
        VTOR = address;
    }

    void triggerPendSV()
    {
        constexpr uint32_t PENDSVSET = (1 << 28);

        ICSR |= PENDSVSET;
    }

    void clearPendSV()
    {
        constexpr uint32_t PENDSVCLR = (1 << 27);

        ICSR |= PENDSVCLR;
    }

    void triggerSysTick()
    {
        constexpr uint32_t PENDSTSET = (1 << 26);

        ICSR |= PENDSTSET;
    }

    void clearSysTick()
    {
        constexpr uint32_t PENDSTCLR = (1 << 25);

        ICSR |= PENDSTCLR;
    }

    void systemReset()
    {
        constexpr uint32_t VECTKEY =
            (0x5FA << 16);

        constexpr uint32_t SYSRESETREQ =
            (1 << 2);

        AIRCR =
            VECTKEY |
            SYSRESETREQ;
    }

    uint32_t activeException() const
    {
        return ICSR & 0x1FF;
    }

    bool isPendSVPending() const
    {
        return (ICSR >> 28) & 1;
    }

    bool isSysTickPending() const
    {
        return (ICSR >> 26) & 1;
    }
};