#include <cpu.hpp>
#include <drive.hpp>
#include <errno.h>
#include <string.hpp>
#include <stringbuilder.hpp>

Sequencer<int> Drive::id(0);

Drive::Drive(size_t sectorSize, uint64_t sectorCount, const char *model, const char *serial) :
    ID(id.GetNext()),
    SectorSize(sectorSize),
    SectorCount(sectorCount),
    Size(SectorSize * SectorCount),
    Model(model ? String::Duplicate(model) : nullptr),
    Serial(serial ? String::Duplicate(serial) : nullptr)
{
}

int64_t Drive::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

int64_t Drive::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    return -ENOSYS;
}

bool Drive::HasMedia()
{
    return false;
}

bool Drive::KeyCheck(const char *name)
{
    StringBuilder sb(15);
    sb.WriteFmt("%d", ID);
    return !String::Compare(sb.String(), name);
}

void Drive::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d", ID);
}

Drive::~Drive()
{
    if(Model) delete[] Model;
    if(Serial) delete[] Serial;
}
