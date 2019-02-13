#include <drive.hpp>
#include <errno.h>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <volume.hpp>
#include <volumetype.hpp>

Sequencer<int> Volume::ids(0);

Volume::Volume(Drive *drive, VolumeType *type) :
    id(ids.GetNext()), drive(drive), type(type), FS(nullptr)
{
}

int Volume::DetectAll()
{
    if(!ObjectTree::Objects->Lock())
        return -EBUSY;
    ObjectTree::Item *vtDir = ObjectTree::Objects->Get(VOL_TYPE_DIR);
    ObjectTree::Item *drvDir = ObjectTree::Objects->Get(DRV_DIR);
    int found = 0;
    for(ObjectTree::Item *item : drvDir->GetChildren())
    {
        Drive *drv = (Drive *)item;
        for(ObjectTree::Item *item : vtDir->GetChildren())
        {
            VolumeType *vt = (VolumeType *)item;
            int res = vt->Detect(drv);
            if(res <= 0) continue;
            found += res;
        }
    }
    ObjectTree::Objects->UnLock();
    return found;
}

size_t Volume::GetSectorSize()
{
    return drive->SectorSize;
}

uint64_t Volume::GetSectorCount()
{
    return drive->SectorCount;
}

ssize_t Volume::ReadSectors(void *buffer, uint64_t firstSector, size_t n)
{
    return -ENOSYS;
}

ssize_t Volume::WriteSectors(const void *buffer, uint64_t firstSector, size_t n)
{
    return -ENOSYS;
}

ssize_t Volume::Read(void *buffer, uint64_t position, size_t n)
{
    return -ENOSYS;
}

ssize_t Volume::Write(const void *buffer, uint64_t position, size_t n)
{
    return -ENOSYS;
}

int Volume::Synchronize()
{
    return 0;
}

bool Volume::KeyCheck(const char *name)
{
    char buf[16]; StringBuilder sb(buf, sizeof(buf));
    sb.WriteFmt("%d", id);
    return !String::Compare(sb.String(), name);
}

void Volume::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%.2f MiB)", id, (double)(GetSectorCount() * GetSectorSize()) / (double)(1 << 20));
}
