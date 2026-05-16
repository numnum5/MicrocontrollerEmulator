
#include "system.hpp"


uint32_t SCS::read32(uint32_t addr)
{
    switch (addr)
    {
        case 0xE000ED00:
            return scb.CPUID;

        case 0xE000ED04:
            return scb.ICSR;

        case 0xE000ED08:
            return scb.VTOR;

        case 0xE000ED0C:
            return scb.AIRCR;

        case 0xE000ED10:
            return scb.SCR;

        case 0xE000ED14:
            return scb.CCR;

        case 0xE000ED18:
            return scb.SHPR1;

        case 0xE000ED1C:
            return scb.SHPR2;

        case 0xE000ED20:
            return scb.SHPR3;

        case 0xE000ED24:
            return scb.SHCSR;

        case 0xE000ED28:
            return scb.CFSR;

        case 0xE000ED2C:
            return scb.HFSR;

        case 0xE000ED30:
            return scb.DFSR;

        case 0xE000E010:
        {
            uint32_t value = systick.SYST_CSR;

            // COUNTFLAG clears on read
            systick.SYST_CSR &= ~systick.CSR_COUNTFLAG;

            return value;
        }

        case 0xE000E014:
            return systick.SYST_RVR;

        case 0xE000E018:
            return systick.SYST_CVR;

        case 0xE000E01C:
            return systick.SYST_CALIB;

        default:
            return 0;
    }
}

void SCS::write32(uint32_t addr, uint32_t value)
{
    switch (addr)
    {
        case 0xE000ED00:
            scb.CPUID = value;
            break;

        case 0xE000ED04:
            scb.ICSR = value;
            break;

        case 0xE000ED08:
            scb.VTOR = value;
            break;

        case 0xE000ED0C:
            scb.AIRCR = value;
            break;

        case 0xE000ED10:
            scb.SCR = value;
            break;

        case 0xE000ED14:
            scb.CCR = value;
            break;

        case 0xE000ED18:
            scb.SHPR1 = value;
            break;

        case 0xE000ED1C:
            scb.SHPR2 = value;
            break;

        case 0xE000ED20:
            scb.SHPR3 = value;
            break;

        case 0xE000ED24:
            scb.SHCSR = value;
            break;

        case 0xE000ED28:
            scb.CFSR = value;
            break;

        case 0xE000ED2C:
            scb.HFSR = value;
            break;

        case 0xE000ED30:
            scb.DFSR = value;
            break;

        case 0xE000E010:
            // writable bits
            systick.SYST_CSR =
                value &
                (systick.CSR_ENABLE |
                systick.CSR_TICKINT |
                systick.CSR_CLKSOURCE);
            break;

        case 0xE000E014:
            // SysTick reload is 24-bit
            systick.SYST_RVR = value & 0x00FFFFFF;
            break;

        case 0xE000E018:
            // any write clears current counter
            systick.SYST_CVR = 0;

            // COUNTFLAG also cleared
            systick.SYST_CSR &= ~systick.CSR_COUNTFLAG;
            break;
    }
}







void SCB::setVectorTable(uint32_t address)
{
    VTOR = address;
}

void SCB::triggerPendSV()
{
    constexpr uint32_t PENDSVSET = (1 << 28);

    ICSR |= PENDSVSET;
}

void SCB::clearPendSV()
{
    constexpr uint32_t PENDSVCLR = (1 << 27);

    ICSR |= PENDSVCLR;
}

void SCB::triggerSysTick()
{
    constexpr uint32_t PENDSTSET = (1 << 26);

    ICSR |= PENDSTSET;
}

void SCB::clearSysTick()
{
    constexpr uint32_t PENDSTCLR = (1 << 25);

    ICSR |= PENDSTCLR;
}

void SCB::systemReset()
{
    constexpr uint32_t VECTKEY =
        (0x5FA << 16);

    constexpr uint32_t SYSRESETREQ =
        (1 << 2);

    AIRCR =
        VECTKEY |
        SYSRESETREQ;
}

uint32_t SCB::activeException() const
{
    return ICSR & 0x1FF;
}

bool SCB::isPendSVPending() const
{
    return (ICSR >> 28) & 1;
}

bool SCB::isSysTickPending() const
{
    return (ICSR >> 26) & 1;
}
