#include <debug.hpp>
#include <filesystemtype.hpp>
#include <stringbuilder.hpp>

FileSystemType::FileSystemType(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        ObjectTree::Objects->Register(FS_TYPE_DIR, this);
}

int FileSystemType::Detect(Volume *volume)
{
    return 0;
}

void FileSystemType::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%s", name);
}
