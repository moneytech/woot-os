#include <debug.hpp>
#include <dentry.hpp>
#include <errno.h>
#include <filesystem.hpp>
#include <filesystemtype.hpp>
#include <inode.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <volume.hpp>

Sequencer<int> FileSystem::ids(0);
Mutex FileSystem::mutex(true, "filesystem");
List<DEntry *> FileSystem::dentryCache;
List<INode *> FileSystem::inodeCache;

FileSystem::FileSystem(Volume *volume, FileSystemType *type) :
    id(ids.GetNext()), volume(volume), type(type)
{
}

void FileSystem::Initialize()
{
    // do nothing
}

void FileSystem::Cleanup()
{
    Lock();
    ObjectTree::Objects->Lock();
    ObjectTree::Item *fsDir = ObjectTree::Objects->Get(FS_DIR);
    if(fsDir)
    {
        for(ObjectTree::Item *item : fsDir->GetChildren())
        {
            FileSystem *fs = (FileSystem *)item;
            if(fs->volume)
                fs->volume->FS = nullptr;
            delete fs;
        }
    }
    for(DEntry *dentry : dentryCache)
        DEBUG("[filesystem] WARNING: DEntry still in cache! (ref count: %d)\n", dentry->ReferenceCount);
    for(INode *inode : inodeCache)
        DEBUG("[filesystem] WARNING: INode still in cache! (ref count: %d)\n", inode->ReferenceCount);
    ObjectTree::Objects->UnLock();
    UnLock();
}

bool FileSystem::Lock()
{
    return mutex.Acquire(10000);
}

void FileSystem::UnLock()
{
    mutex.Release();
}

int FileSystem::DetectAll()
{
    if(!ObjectTree::Objects->Lock())
        return -EBUSY;
    ObjectTree::Item *fstDir = ObjectTree::Objects->Get(FS_TYPE_DIR);
    if(!fstDir)
    {
        ObjectTree::Objects->UnLock();
        return 0;
    }
    ObjectTree::Item *volDir = ObjectTree::Objects->Get(VOLUME_DIR);
    if(!volDir)
    {
        ObjectTree::Objects->UnLock();
        return 0;
    }
    int found = 0;
    for(ObjectTree::Item *item : volDir->GetChildren())
    {
        Volume *vol = (Volume *)item;
        if(vol->FS)
            continue; // already has a filesystem

        for(ObjectTree::Item *item : fstDir->GetChildren())
        {
            FileSystemType *fst = (FileSystemType *)item;
            int res = fst->Detect(vol);
            if(res <= 0) continue;
            found += res;
        }
    }
    ObjectTree::Objects->UnLock();
    return found;
}

void FileSystem::PutINode(INode *inode)
{
    if(!Lock()) return;
    if(!--inode->ReferenceCount)
    {
        inodeCache.Remove(inode, nullptr, false);
        if(inode->Dirty)
            inode->FS->WriteINode(inode);
        inode->Release();
        delete inode;
    }
    UnLock();
}

DEntry *FileSystem::GetDEntry(DEntry *parent, const char *name)
{
    if(!Lock() || !parent || !parent->INode || !parent->INode->FS || !name)
        return nullptr;
    for(DEntry *dentry : dentryCache)
    {
        if(parent == dentry->Parent && !String::Compare(name, dentry->Name))
        {
            ++dentry->ReferenceCount;
            UnLock();
            return dentry;
        }
    }
    ino_t ino = parent->INode->Lookup(name);
    if(ino <= 0)
    {
        UnLock();
        return nullptr;
    }
    DEntry *dentry = new DEntry(name, parent, parent->INode->FS->GetINode(ino));
    dentryCache.Prepend(dentry);
    ++dentry->ReferenceCount;
    UnLock();
    return dentry;
}

DEntry *FileSystem::GetDEntry(DEntry *dentry)
{
    if(!Lock()) return nullptr;
    DEntry *res = dentryCache.Find(dentry, nullptr);
    if(res) ++res->ReferenceCount;
    UnLock();
    return res;
}

void FileSystem::PutDEntry(DEntry *dentry)
{
    if(!Lock()) return;
    if(!--dentry->ReferenceCount)
    {
        dentryCache.Remove(dentry, nullptr, false);
        delete dentry;
    }
    UnLock();
}

INode *FileSystem::GetINode(ino_t number)
{
    if(!Lock()) return nullptr;
    for(INode *inode : inodeCache)
    {
        if(inode->FS == this && inode->Number == number)
        {
            ++inode->ReferenceCount;
            UnLock();
            return inode;
        }
    }
    INode *inode = ReadINode(number);
    if(!inode)
    {
        UnLock();
        return nullptr;
    }
    inodeCache.Prepend(inode);
    ++inode->ReferenceCount;
    UnLock();
    return inode;
}

int FileSystem::GetID()
{
    return id;
}

DEntry *FileSystem::GetRoot()
{
    return root;
}

bool FileSystem::GetLabel(char *buf, size_t bufSize)
{
    return false;
}

UUID FileSystem::GetUUID()
{
    return UUID::nil;
}

INode *FileSystem::ReadINode(ino_t number)
{
    return nullptr;
}

bool FileSystem::WriteINode(INode *inode)
{
    return false;
}

bool FileSystem::WriteSuperBlock()
{
    return false;
}

int FileSystem::Synchronize()
{
    return 0;
}

bool FileSystem::KeyCheck(const char *name)
{
    char buf[16]; StringBuilder sb(buf, sizeof(buf));
    sb.WriteFmt("%d", id);
    return !String::Compare(sb.String(), name);
}

void FileSystem::GetDisplayName(char *buf, size_t bufSize)
{
    char fstName[64]; type->GetDisplayName(fstName, sizeof(fstName));
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", id, fstName);
}
