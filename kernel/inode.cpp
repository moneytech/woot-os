#include <directoryentry.hpp>
#include <errno.h>
#include <inode.hpp>
#include <string.hpp>

INode::INode(ino_t number, FileSystem *fs) :
    Number(number), FS(fs),
    ReferenceCount(0),
    Dirty(false)
{
}

uint64_t INode::GetSize()
{
    return 0;
}

mode_t INode::GetMode()
{
    return 0;
}

time_t INode::GetCreateTime()
{
    return 0;
}

time_t INode::GetModifyTime()
{
    return 0;
}

time_t INode::GetAccessTime()
{
    return 0;
}

int INode::GetLinkCount()
{
    return -ENOSYS;
}

uid_t INode::GetUID()
{
    return 0;
}

gid_t INode::GetGID()
{
    return 0;
}

bool INode::SetCreateTime(time_t t)
{
    return false;
}

bool INode::SetModifyTime(time_t t)
{
    return false;
}

bool INode::SetAccessTime(time_t t)
{
    return false;
}

bool INode::Create(const char *name, mode_t mode)
{
    return false;
}

ino_t INode::Lookup(const char *name)
{   // generic implementation
    int64_t position = 0;
    uint64_t size = GetSize();
    ino_t res = -1;
    while(position < size)
    {
        DirectoryEntry *de = ReadDir(position, &position);
        if(!de) break;
        if(!String::Compare(name, de->Name))
        {
            res = de->INode;
            delete de;
            break;
        }
        delete de;
    }
    return res;
}

int64_t INode::Read(void *buffer, int64_t position, int64_t n)
{
    return -ENOSYS;
}

int64_t INode::Write(const void *buffer, int64_t position, int64_t n)
{
    return -ENOSYS;
}

DirectoryEntry *INode::ReadDir(int64_t position, int64_t *newPosition)
{
    return nullptr;
}

int64_t INode::Resize(int64_t size)
{
    return -ENOSYS;
}

int INode::Remove(const char *name)
{
    return -ENOSYS;
}

int INode::Release()
{
    return -ENOSYS;
}

INode::~INode()
{
}
