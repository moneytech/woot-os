#include <../v86/v86.hpp>
#include <errno.h>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <sysdefs.h>
#include <vesafb.hpp>

VESAFB::VESAFB(bool autoRegister) :
    FrameBuffer(autoRegister),
    driverModeInfos(16, 4, 1024)
{
    String::Copy(OEMName, "vesafb");

    V86::RealModeFarPointer realPtr = { 0x0000, 0x4000 }; // FIXME: Shouldn't be hardcoded
    V86::Regs regs;
    regs.AX = 0x4F00;
    regs.ES = realPtr.Segment;
    regs.DI = realPtr.Offset;
    V86::Int(0x10, &regs);
    SVGAInfo *si = (SVGAInfo *)(KERNEL_BASE + realPtr.ToLinear());
    const char *oemName = (const char *)(KERNEL_BASE + si->OEMNamePtr.ToLinear());
    String::Copy(OEMName, oemName, sizeof(OEMName));
    OEMName[sizeof(OEMName) - 1] = 0;
    uint16_t *modes = (uint16_t *)(KERNEL_BASE + si->ModePtr.ToLinear());
    realPtr.Offset = 0x0200;
    VESAModeInfo *mi = (VESAModeInfo *)(KERNEL_BASE + realPtr.ToLinear());
    for(int i = 0; i < 1024; ++i)
    {
        if(modes[i] == 0xFFFF)
            break;
        regs.AX = 0x4F01;
        regs.CX = modes[i];
        regs.ES = realPtr.Segment;
        regs.DI = realPtr.Offset;
        V86::Int(0x10, &regs);
        if(mi->MemoryModel != 0x03 && mi->MemoryModel != 0x04 && mi->MemoryModel != 0x06)
            continue;
        if(!mi->LFBAddress)
            continue; // not LFB mode
        driverModeInfos.Append(DriverModeInfo { modes[i], *mi });
    }
}

int VESAFB::GetModeCount()
{
    return driverModeInfos.Size();
}

int VESAFB::GetModeInfo(int mode, FrameBuffer::ModeInfo *info)
{
    if(mode < 0 || mode >= driverModeInfos.Size())
        return -EINVAL;
    DriverModeInfo mi = driverModeInfos.Get(mode);
    info->Width = mi.ModeInfo.Width;
    info->Height = mi.ModeInfo.Height;
    info->BitsPerPixel = mi.ModeInfo.BitsPerPixel;
    info->RefreshRate = 0;
    info->Pitch = mi.ModeInfo.BytesPerScanLine;
    info->Flags = mi.ModeInfo.MemoryModel != 0x06 ? FBMI_FLAG_INDEX_COLOR : 0;
    info->AlphaBits = mi.ModeInfo.ReservedMaskSize;
    info->RedBits = mi.ModeInfo.RedMaskSize;
    info->GreenBits = mi.ModeInfo.GreenMaskSize;
    info->BlueBits = mi.ModeInfo.BlueMaskSize;
    info->AlphaShift = mi.ModeInfo.ReservedFieldPosition;
    info->RedShift = mi.ModeInfo.RedFieldPosition;
    info->GreenShift = mi.ModeInfo.GreenFieldPosition;
    info->BlueShift = mi.ModeInfo.BlueFieldPosition;
    return ESUCCESS;
}

int VESAFB::SetMode(int mode)
{
    if(mode < 0 || mode >= driverModeInfos.Size())
        return -EINVAL;
    DriverModeInfo mi = driverModeInfos.Get(mode);
    V86::Regs regs;
    regs.AX = 0x4F02;
    regs.BX = mi.ModeNumber;
    V86::Int(0x10, &regs);
    return regs.AH ? -EFAULT : 0;
}

void VESAFB::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", id, OEMName);
}
