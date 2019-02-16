#include <file.hpp>
#include <filestream.hpp>

FileStream::FileStream(File *file) :
    file(file)
{
}

int64_t FileStream::Read(void *buffer, int64_t n)
{
    return file->Read(buffer, n);
}

int64_t FileStream::Write(const void *buffer, int64_t n)
{
    return file->Write(buffer, n);
}
