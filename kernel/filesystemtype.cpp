#include <debug.hpp>
#include <filesystemtype.hpp>
#include <stringbuilder.hpp>

FileSystemType::FileSystemType(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        Register();
}

bool FileSystemType::Register()
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(FS_TYPE_DIR);
    if(!dir)
    {
        DEBUG("[filesystemtype] Couldn't open '%s' when registering '%s'\n", FS_TYPE_DIR, name);
        return false;
    }
    if(dir->ContainsChild(name))
    {
        DEBUG("[filesystemtype] Volume type '%s' already exists\n", name);
        return false;
    }
    if(!dir->AddChild(this))
    {
        DEBUG("[filesystemtype] Couldn't register volume type '%s'\n", name);
        return false;
    }
    return true;
}

bool FileSystemType::UnRegister()
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(FS_TYPE_DIR);
    if(!dir)
    {
        DEBUG("[volumetype] Couldn't open '%s' when unregistering '%s'\n", FS_TYPE_DIR, name);
        return false;
    }
    return dir->RemoveChild(name);
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
