#include <../ext2/ext2.hpp>

EXT2FileSystemType::EXT2FileSystemType(bool autoRegister) :
    FileSystemType("ext2", autoRegister)
{
}

int EXT2FileSystemType::Detect(Volume *vol)
{
    return 0;
}
