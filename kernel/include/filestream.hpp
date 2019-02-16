#pragma once

#include <file.hpp>
#include <stream.hpp>
#include <types.hpp>

class File;

class FileStream : public Stream
{
    File *file;
public:
    FileStream(File *file);
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
};
