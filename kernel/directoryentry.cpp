#include <directoryentry.hpp>
#include <string.hpp>

DirectoryEntry::DirectoryEntry(mode_t mode, time_t atime, time_t ctime, time_t mtime, uint64_t size, ino_t inode, const char *name) :
    Mode(mode),
    AccessTime(atime),
    CreateTime(ctime),
    ModifyTime(mtime),
    Size(size),
    INode(inode),
    Name(String::Duplicate(name))
{
}

DirectoryEntry::~DirectoryEntry()
{
    if(Name) delete[] Name;
}
