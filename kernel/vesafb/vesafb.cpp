#include <vesafb.hpp>

#pragma pack(push, 1)
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
};
#pragma pack(pop)

VESAFB::VESAFB(bool autoRegister) :
    FrameBuffer(autoRegister)
{
}
