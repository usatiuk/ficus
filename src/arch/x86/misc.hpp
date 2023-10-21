#ifndef OS1_MISC_H
#define OS1_MISC_H

#include <stdint.h>

extern "C" void _sse_setup();
extern "C" void _hcf();

#define barrier() __asm__ __volatile__("" :: \
                                               : "memory");

static inline uint64_t *get_cr3() {
    uint64_t *cr3;
    asm("mov %%cr3, %0"
        : "=rm"(cr3));
    return cr3;
}


static inline uint64_t flags() {
    uint64_t flags;
    asm volatile("pushf\n\t"
                 "pop %0"
                 : "=g"(flags));
    return flags;
}

static inline int are_interrupts_enabled() {
    return (flags() & (1 << 9));
}

static inline unsigned long save_irqdisable(void) {
    unsigned long flags;
    asm volatile("pushf\n\tcli\n\tpop %0"
                 : "=r"(flags)
                 :
                 : "memory");
    return flags;
}

static inline void irqrestore(unsigned long flags) {
    asm("push %0\n\tpopf"
        :
        : "rm"(flags)
        : "memory", "cc");
}

#define NO_INT(x)                            \
    {                                        \
        unsigned long f = save_irqdisable(); \
        barrier();                           \
        x                                    \
        barrier();                           \
        irqrestore(f);                       \
    }


char *itoa(int value, char *str, int base);

#endif