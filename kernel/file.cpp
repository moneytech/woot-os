#include <character.hpp>
#include <dentry.hpp>
#include <errno.h>
#include <file.hpp>
#include <filesystem.hpp>
#include <inode.hpp>
#include <mutex.hpp>
#include <process.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <sysdefs.h>
#include <tokenizer.hpp>
#include <volume.hpp>

File *File::open(::DEntry *parent, const char *name, int flags)
{

    //printf("open %s\n", name);
    if(!parent || !parent->INode || !name)
        return nullptr;

    Tokenizer path(name, PATH_SEPARATORS, 0);

    ::DEntry *dentry = FileSystem::GetDEntry(parent);
    if(!dentry) return nullptr;
    for(Tokenizer::Token t : path.Tokens)
    {
        if(!String::Compare(".", t.String))
            continue;
        else if(!String::Compare("..", t.String))
        {
            if(dentry->Parent)
            {
                ::DEntry *nextDe = FileSystem::GetDEntry(dentry->Parent);
                FileSystem::PutDEntry(dentry);
                dentry = nextDe;
            }
            continue;
        }
        ::DEntry *nextDe = FileSystem::GetDEntry(dentry, t.String);
        if(!nextDe)
        {
            FileSystem::PutDEntry(dentry);
            return nullptr;
        }
        dentry = nextDe;
    }
    if((flags & O_ACCMODE) != O_RDONLY && flags & O_TRUNC)
    {
        if(dentry->INode->Resize(0) != 0)
        {
            FileSystem::PutDEntry(dentry);
            return nullptr;
        }
    }
    mode_t mode = dentry->INode->GetMode();
    if((flags & O_DIRECTORY && !S_ISDIR(mode)))
    {
        FileSystem::PutDEntry(dentry);
        return nullptr;
    }
    File *file = new File(dentry, flags, mode);
    if(flags & O_APPEND)
        file->Position = file->GetSize();
    return file;
}

File::File(::DEntry *dentry, int flags, mode_t mode) :
    DEntry(dentry),
    Flags(flags),
    Position(0),
    Mode(mode)
{
}

int64_t File::getSize()
{
    int64_t size = DEntry && DEntry->INode ? DEntry->INode->GetSize() : -EINVAL;
    return size;
}

File *File::Open(::DEntry *parent, const char *name, int flags)
{
    //printf("open %s\n", name);
    if(!FileSystem::Lock())
        return nullptr;
    File *file = open(parent, name, flags);
    FileSystem::UnLock();
    return file;
}

File *File::Open(const char *name, int flags)
{
    if(!name || !String::Length(name))
        name = ".";
    Tokenizer path(name, PATH_SEPARATORS, 0);
    if(!path[0]) return nullptr;
    char *fsSep = path[0] ? String::Find(path[0], FS_SEPARATOR, false) : nullptr;
    if(fsSep) *fsSep = 0;

    if(!FileSystem::Lock())
        return nullptr;
    bool hasFs = fsSep;
    FileSystem *fs = nullptr;
    if(hasFs)
    {
        StringBuilder sb(MAX_PATH_LENGTH);
        sb.WriteFmt("%s/%s", FS_DIR, fsSep ? path[0] : "0");
        fs = (FileSystem *)ObjectTree::Objects->Get(sb.String());
        if(!fs) return nullptr;
    }
    bool absolute = !hasFs && path[0][0] == '/';
    ::DEntry *dentry = hasFs || absolute ? fs->GetRoot() : Process::GetCurrentDir();

    if(!dentry)
        return nullptr;

    File *file = open(dentry, name + (hasFs ? path.Tokens[1].Offset : 0), flags);
    FileSystem::UnLock();
    return file;
}

int64_t File::GetSize()
{
    if(!FileSystem::Lock())
        return -EBUSY;
    int64_t size = getSize();
    FileSystem::UnLock();
    return size;
}

bool File::SetAccessTime(time_t time)
{
    if(!FileSystem::Lock())
        return -EBUSY;
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetAccessTime(time) : -EINVAL;
    FileSystem::UnLock();
    return res;
}

bool File::SetModifyTime(time_t time)
{
    if(!FileSystem::Lock())
        return -EBUSY;
    bool res = DEntry && DEntry->INode ? DEntry->INode->SetModifyTime(time) : -EINVAL;
    FileSystem::UnLock();
    return res;
}

bool File::Create(const char *name, mode_t mode)
{
    if(!FileSystem::Lock())
        return false;
    bool res = DEntry && DEntry->INode ? DEntry->INode->Create(name, mode) : false;
    FileSystem::UnLock();
    return res;
}

int File::Remove(const char *name)
{
    if(!FileSystem::Lock())
        return false;
    int res = DEntry && DEntry->INode ? DEntry->INode->Remove(name) : -EINVAL;
    FileSystem::UnLock();
    return res;
}

int64_t File::Seek(int64_t offs, int loc)
{
    if(S_ISDIR(Mode) && (offs != 0 || loc != SEEK_SET))
        return -EISDIR;
    switch(loc)
    {
    case SEEK_SET:
        Position = offs;
        break;
    case SEEK_CUR:
        Position += offs;
        break;
    case SEEK_END:
        Position = getSize() - offs;
        break;
    default:
        break;
    }
    if(Position < 0) Position = 0;
    int64_t res = Position;
    return res;
}

int64_t File::Read(void *buffer, int64_t n)
{
    if(!DEntry) return -EINVAL;
    if(S_ISDIR(Mode))
        return -EISDIR;
    if((Flags & O_ACCMODE) == O_WRONLY)
        return -EINVAL;
    if(!FileSystem::Lock())
        return -EBUSY;
    int64_t res = DEntry->INode ? DEntry->INode->Read(buffer, Position, n) : -EINVAL;
    if(res > 0) Position += res;
    FileSystem::UnLock();
    return res;
}

int64_t File::Write(const void *buffer, int64_t n)
{
    if(!DEntry) return -EINVAL;
    if(S_ISDIR(Mode))
        return -EISDIR;
    if((Flags & O_ACCMODE) == O_RDONLY)
        return -EINVAL;
    if(!FileSystem::Lock())
        return -EBUSY;
    int64_t res = DEntry->INode ? DEntry->INode->Write(buffer, Position, n) : -EINVAL;
    if(res > 0) Position += res;
    FileSystem::Lock();
    return res;
}

int64_t File::Rewind()
{
    return Seek(0, SEEK_SET);
}

DirectoryEntry *File::ReadDir()
{
    if(!DEntry)
        return nullptr;
    if(!(S_ISDIR(Mode)))
    {   // not a directory
        return nullptr;
    }
    if((Flags & O_ACCMODE) == O_WRONLY)
        return nullptr;
    if(!FileSystem::Lock())
        return nullptr;
    int64_t newPos = Position;
    DirectoryEntry *res = DEntry->INode ? DEntry->INode->ReadDir(Position, &newPos) : nullptr;
    if(res) Position = newPos;
    FileSystem::Lock();
    return res;
}

File::~File()
{
    //printf("close\n");
    FileSystem::PutDEntry(DEntry);
}

