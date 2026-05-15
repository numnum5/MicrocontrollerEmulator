#include <cstdint>
#include <cstring>

static constexpr int NUM_IRQS = 64;

struct NVIC
{
    // External interrupt lines
    bool line[NUM_IRQS]{};

    // Previous sampled state (for edge detection)
    bool prevLine[NUM_IRQS]{};

    // Pending bits
    bool pending[NUM_IRQS]{};

    // Enable bits
    bool enabled[NUM_IRQS]{};

    // Active bits
    bool active[NUM_IRQS]{};
};

class CPU
{
public:
    NVIC nvic;

    // =====================================================
    // Raise interrupt line from peripheral/device
    // =====================================================
    void assertIRQ(int irq)
    {
        nvic.line[irq] = true;
    }

    // =====================================================
    // Lower interrupt line
    // =====================================================
    void deassertIRQ(int irq)
    {
        nvic.line[irq] = false;
    }

    // =====================================================
    // Software set-pending (ISPR write)
    // =====================================================
    void setPending(int irq)
    {
        nvic.pending[irq] = true;
    }

    // =====================================================
    // Software clear-pending (ICPR write)
    // =====================================================
    void clearPending(int irq)
    {
        nvic.pending[irq] = false;
    }

    // =====================================================
    // Edge detector
    // =====================================================
    bool edge(int irq)
    {
        return !nvic.prevLine[irq] &&
                nvic.line[irq];
    }

    // =====================================================
    // NVIC update logic
    // =====================================================
    void updateNVIC()
    {
        for (int irq = 0; irq < NUM_IRQS; irq++)
        {
            bool interruptAssertion =
                edge(irq) || nvic.line[irq];

            bool interruptDeassertion =
                !nvic.line[irq];

            bool clearPend =
                interruptDeassertion;

            bool setPend =
                interruptAssertion;

            // Update pending state
            if (clearPend && setPend)
            {
                // Implementation defined
                // Keep pending set here
                nvic.pending[irq] = true;
            }
            else
            {
                nvic.pending[irq] =
                    setPend ||
                    (nvic.pending[irq] && !clearPend);
            }

            // Save sampled state
            nvic.prevLine[irq] = nvic.line[irq];
        }
    }

    // =====================================================
    // Interrupt acceptance
    // =====================================================
    void checkInterrupts()
    {
        for (int irq = 0; irq < NUM_IRQS; irq++)
        {
            if (nvic.pending[irq] &&
                nvic.enabled[irq] &&
                !nvic.active[irq])
            {
                nvic.pending[irq] = false;

                ExceptionTaken(16 + irq);

                break;
            }
        }
    }

    // =====================================================
    // Exception entry
    // =====================================================
    void ExceptionTaken(int exceptionNumber)
    {
        int irq = exceptionNumber - 16;

        if (irq >= 0)
        {
            nvic.active[irq] = true;
        }

        // Push stack frame
        // Update xPSR
        // Load vector
        // Branch to handler
    }

    // =====================================================
    // Exception return
    // =====================================================
    void ExceptionReturn(int exceptionNumber)
    {
        int irq = exceptionNumber - 16;

        if (irq >= 0)
        {
            nvic.active[irq] = false;
        }
    }
};