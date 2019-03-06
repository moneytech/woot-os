#pragma once

#include <framebuffer.hpp>
#include <types.hpp>

class VESAFB : public FrameBuffer
{
public:
    VESAFB(bool autoRegister);
};
