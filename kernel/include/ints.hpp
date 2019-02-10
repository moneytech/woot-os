#pragma once

#include <types.hpp>

class Ints
{
public:
#pragma pack(push, 1)
struct State
{
    union //sESP
    {
        uint32_t ESP;
        uint16_t SP;
    };
    struct //sSS
    {
        uint16_t SS;
        uint16_t hSS;
    };
    struct //sGS
    {
        uint16_t GS;
        uint16_t hGS;
    };
    struct //sFS
    {
        uint16_t FS;
        uint16_t hFS;
    };
    struct //sES
    {
        uint16_t ES;
        uint16_t hES;
    };
    struct //sDS
    {
        uint16_t DS;
        uint16_t hDS;
    };
    union //sEDI
    {
        uint32_t EDI;
        uint16_t DI;
    };
    union //sESI
    {
        uint32_t ESI;
        uint16_t SI;
    };
    union //sEBP
    {
        uint32_t EBP;
        uint16_t BP;
    };
    union //sEBX
    {
        uint32_t EBX;
        uint16_t BX;
        struct
        {
            uint8_t BL;
            uint8_t BH;
        };
    };
    union //sEDX
    {
        uint32_t EDX;
        uint16_t DX;
        struct
        {
            uint8_t DL;
            uint8_t DH;
        };
    };
    union //sECX
    {
        uint32_t ECX;
        uint16_t CX;
        struct
        {
            uint8_t CL;
            uint8_t CH;
        };
    };
    union //sEAX
    {
        uint32_t EAX;
        uint16_t AX;
        struct
        {
            uint8_t AL;
            uint8_t AH;
        };
    };
    uint32_t InterruptNumber;
    uint32_t ErrorCode;

    union //sEIP
    {
        uint32_t EIP;
        uint16_t IP;
    };
    struct //sCS
    {
        uint16_t CS;
        uint16_t hCS;
    };
    union //sEFLAGS
    {
        uint32_t EFLAGS;
        uint16_t FLAGS;
    };
    union //sUserESP
    {
        uint32_t UserESP;
        uint16_t UserSP;
    };
    union //sUserSS
    {
        uint16_t UserSS;
        uint16_t hUserSS;
    };
};
#pragma pack(pop)
private:
    typedef bool (*HandlerCallback)(State *state, void *context);
public:
    struct Handler
    {
        Handler *Next; // set to 0
        HandlerCallback Callback;
        void *Context;
    };
private:
    static Handler *Handlers[];
public:
    static void CommonHandler(State *state);
    static void RegisterHandler(uint intNo, Handler *handler);
    static void UnRegisterHandler(uint intNo, Handler *handler);
    static uint HandlerCount(uint intNo);
    static void DumpState(Ints::State *state);
};
