#pragma once

#include <types.hpp>

#pragma pack(push, 1)

struct GDTEntry
{
    union
    {
        struct
        {
            uint64_t Limit15_0 : 16;
            uint64_t Base23_0 : 24;
            uint64_t Access : 8;
            uint64_t Limit19_16 : 4;
            uint64_t Flags : 4;
            uint64_t Base31_24 : 8;
        };
        uint64_t Value;
    };
};

struct TSS
{
    uint32_t PreviousTSS;
    uint32_t ESP0, SS0;
    uint32_t ESP1, SS1;
    uint32_t ESP2, SS2;
    uint32_t CR3;
    uint32_t EIP;
    uint32_t EFLAGS;
    uint32_t EAX, ECX, EDX, EBX;
    uint32_t ESP, EBP, ESI, EDI;
    uint32_t ES, CS, SS, DS, FS, GS;
    uint32_t LDT;
    uint16_t Trap;
    uint16_t IOMapBase;
};

struct GDTDescriptor
{
    uint16_t Limit;
    GDTEntry *Entries;
};

#pragma pack(pop)

class GDT
{
public:
    static GDTEntry *Entries;
    static TSS *MainTSS;

    static void Initialize();
    static void SetEntry(uint i, uintptr_t Base, size_t Limit, uint8_t Access, uint8_t Flags);
    static uintptr_t GetEntryBase(uint i);
    static void Load(GDTDescriptor *descr);
};
