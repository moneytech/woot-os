#include <errno.h>
#include <filesystem.hpp>
#include <filesystemtype.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <volume.hpp>

Sequencer<int> FileSystem::ids(0);

FileSystem::FileSystem(Volume *volume, FileSystemType *type) :
    id(ids.GetNext()), volume(volume), type(type)
{
}

int FileSystem::DetectAll()
{
    if(!ObjectTree::Objects->Lock())
        return -EBUSY;
    ObjectTree::Item *fstDir = ObjectTree::Objects->Get(FS_TYPE_DIR);
    ObjectTree::Item *volDir = ObjectTree::Objects->Get(VOLUME_DIR);
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
