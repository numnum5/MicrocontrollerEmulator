
#include <stdint.h>

class SCB
{
    public:

        // 0xE000ED00
        uint32_t CPUID;

        // 0xE000ED04
        uint32_t ICSR;

        // 0xE000ED08
        uint32_t VTOR;

        // 0xE000ED0C
        uint32_t AIRCR;

        // 0xE000ED10
        uint32_t SCR;

        // 0xE000ED14
        uint32_t CCR;

        // 0xE000ED18
        uint32_t SHPR1;

        // 0xE000ED1C
        uint32_t SHPR2;

        // 0xE000ED20
        uint32_t SHPR3;

        // 0xE000ED24
        uint32_t SHCSR;

        // 0xE000ED28
        uint32_t CFSR;

        // 0xE000ED2C
        uint32_t HFSR;

        // 0xE000ED30
        uint32_t DFSR;

        void setVectorTable(uint32_t address);

        void triggerPendSV();


        void clearPendSV();
 

        void triggerSysTick();
      
        void clearSysTick();
      
        void systemReset();

        uint32_t activeException() const;
  

        bool isPendSVPending() const;

        bool isSysTickPending() const;
};

class SysTick
{
    public:

        static constexpr uint32_t CSR_ENABLE    = (1 << 0);
        static constexpr uint32_t CSR_TICKINT   = (1 << 1);
        static constexpr uint32_t CSR_CLKSOURCE = (1 << 2);
        static constexpr uint32_t CSR_COUNTFLAG = (1 << 16);

        uint32_t SYST_CSR   = 0;
        uint32_t SYST_RVR   = 0;
        uint32_t SYST_CVR   = 0;
        uint32_t SYST_CALIB = 0;
};


class SCS
{
    public:
        SCB scb;
        SysTick systick;
        uint32_t read32(uint32_t addr);
        void write32(uint32_t addr, uint32_t value);
};

