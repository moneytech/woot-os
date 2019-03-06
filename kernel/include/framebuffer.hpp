#pragma once

#define FB_DIR  "/dev/fb"

#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>

class FrameBuffer : public ObjectTree::Item
{
    static Sequencer<int> ids;
protected:
    int id;

    FrameBuffer(bool autoRegister);
public:
    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~FrameBuffer();
};
