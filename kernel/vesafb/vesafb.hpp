#pragma once

#include <../v86/v86.hpp>
#include <framebuffer.hpp>
#include <types.hpp>
#include <vector.hpp>

class VESAFB : public FrameBuffer
{
#pragma pack(push, 1)
    struct SVGAInfo
    {
        char Signature[4];
        uint8_t VersionMinor;
        uint8_t VersionMajor;
        V86::RealModeFarPointer OEMNamePtr;
        uint32_t Capabilities;
        V86::RealModeFarPointer ModePtr;
        uint16_t MemorySize;
        uint8_t Padding[0x200 - 0x14];
    };

    struct VESAModeInfo
    {
        uint16_t Attributes;
        uint8_t WindowAttributes[2];
        uint16_t WindowGranularity;
        uint16_t WindowSize;
        uint16_t WindowSegment[2];
        uint32_t WindowPositioningFunction;
        uint16_t BytesPerScanLine;
        uint16_t Width;
        uint16_t Height;
        uint8_t CharacterWidth;
        uint8_t CharacterHeight;
        uint8_t PlaneCount;
        uint8_t BitsPerPixel;
        uint8_t BankCount;
        uint8_t MemoryModel;
        uint8_t BankSize;
        uint8_t PageCount;
        uint8_t Reserved;
        uint8_t RedMaskSize;
        uint8_t RedFieldPosition;
        uint8_t GreenMaskSize;
        uint8_t GreenFieldPosition;
        uint8_t BlueMaskSize;
        uint8_t BlueFieldPosition;
        uint8_t ReservedMaskSize;
        uint8_t ReservedFieldPosition;
        uint8_t DirectColorModeInfo;
        uint32_t LFBAddress;
        uint32_t OffscreenMemoryAddress;
        uint32_t OffscreenMemorySize;
        uint8_t Padding[0x100 - 0x32];
    };
#pragma pack(pop)
    struct DriverModeInfo
    {
        uint16_t ModeNumber;
        VESAModeInfo ModeInfo;
    };

    Vector<DriverModeInfo> driverModeInfos;
    char OEMName[64];
public:
    VESAFB(bool autoRegister);

    virtual int GetModeCount();
    virtual int GetModeInfo(int mode, ModeInfo *info);
    virtual int SetMode(int mode);

    virtual void GetDisplayName(char *buf, size_t bufSize);
};
