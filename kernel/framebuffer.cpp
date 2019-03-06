#include <framebuffer.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>

Sequencer<int> FrameBuffer::ids(0);

FrameBuffer::FrameBuffer(bool autoRegister) :
    id(ids.GetNext())
{
    if(autoRegister)
        ObjectTree::Objects->Register(FB_DIR, this);
}

bool FrameBuffer::KeyCheck(const char *name)
{
    char buf[OBJTREE_MAX_NAME_LEN + 1];
    StringBuilder sb(buf, sizeof(buf));
    sb.WriteFmt("%d", id);
    return !String::Compare(name, sb.String());
}

void FrameBuffer::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (vesafb)", id);
}

FrameBuffer::~FrameBuffer()
{
}
