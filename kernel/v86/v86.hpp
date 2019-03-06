#pragma once

#include <ints.hpp>
#include <mutex.hpp>
#include <types.hpp>

class V86
{
    static const uint16_t codeSegment;
    static const uint16_t stackSegment;
    static Ints::State cpuState;
    static Ints::Handler enterHandler;
    static Ints::Handler exceptionHandler;
    static Mutex mutex;

    static bool enterCallback(Ints::State *state, void *context);
    static bool exceptionCallback(Ints::State *state, void *context);
    static uint8_t peekb(uint16_t seg, uint16_t offs);
    static uint16_t peekw(uint16_t seg, uint16_t offs);
    static void pokeb(uint16_t seg, uint16_t offs, uint8_t val);
    static void pokew(uint16_t seg, uint16_t offs, uint16_t val);
    static void pushw(Ints::State *state, uint16_t val);
    static uint16_t popw(Ints::State *state);
public:
    struct RealModeFarPointer
    {
        uint16_t Offset;
        uint16_t Segment;

        uintptr_t ToLinear();
    };

    typedef struct Regs
    {
        union
        {
            uint32_t EAX;
            uint16_t AX;
            struct
            {
                uint8_t AL;
                uint8_t AH;
            };
        };
        union
        {
            uint32_t EBX;
            uint16_t BX;
            struct
            {
                uint8_t BL;
                uint8_t BH;
            };
        };
        union
        {
            uint32_t ECX;
            uint16_t CX;
            struct
            {
                uint8_t CL;
                uint8_t CH;
            };
        };
        union
        {
            uint32_t EDX;
            uint16_t DX;
            struct
            {
                uint8_t DL;
                uint8_t DH;
            };
        };
        union
        {
            uint32_t ESI;
            uint16_t SI;
        };
        union
        {
            uint32_t EDI;
            uint16_t DI;
        };
        union
        {
            uint32_t EBP;
            uint16_t BP;
        };
        uint32_t DS;
        uint32_t ES;
        uint32_t FS;
        uint32_t GS;
    } Regs;

    static void Initialize();
    static void Cleanup();
    static bool Enter(uint16_t ss, uint16_t sp, uint16_t cs, uint16_t ip, uint32_t arg);
    static bool Int(uint8_t intNo, Regs *regs);
};
