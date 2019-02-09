#include <cpu.hpp>
#include <gdt.hpp>
#include <memory.hpp>
#include <sysdefs.h>

GDTEntry *GDT::Entries;
TSS *GDT::MainTSS;

void GDT::Initialize()
{
    // get GDT descriptor
    GDTDescriptor descr;
    cpuSGDT(&descr);

    // get GDT address
    Entries = descr.Entries;

    // initialize TSS
    uint16_t tr = cpuSTR();
    MainTSS = (TSS *)GetEntryBase(tr >> 3);
    MainTSS->IOMapBase = 104;
    MainTSS->CS = SEG_CODE32_KERNEL;
    MainTSS->SS0 = SEG_DATA32_KERNEL;
    MainTSS->DS = SEG_DATA32_KERNEL;
    MainTSS->ES = SEG_DATA32_KERNEL;
    MainTSS->FS = SEG_DATA32_KERNEL;
    MainTSS->GS = SEG_DATA32_KERNEL;
    MainTSS->EIP = cpuGetEIP();
    MainTSS->ESP0 = cpuGetESP();
}

void GDT::SetEntry(uint i, uintptr_t Base, size_t Limit, uint8_t Access, uint8_t Flags)
{
    Entries[i].Base23_0 = Base & 0xFFFFFF;
    Entries[i].Base31_24 = (Base >> 24) & 0xFF;
    Entries[i].Limit15_0 = Limit & 0xFFFF;
    Entries[i].Limit19_16 = (Limit >> 16) & 0x0F;
    Entries[i].Access = Access;
    Entries[i].Flags = Flags;
}

uintptr_t GDT::GetEntryBase(uint i)
{
    return Entries[i].Base23_0 | Entries[i].Base31_24 << 24;
}

void GDT::Load(GDTDescriptor *descr)
{
    cpuLGDT(descr);
    cpuFixSegments();
}
