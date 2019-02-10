#pragma once

#include <stream.hpp>

class CallBackStream : public Stream
{
public:
    typedef int64_t (*ReadCallBackHandler)(void *buffer, int64_t n);
    typedef int64_t (*WriteCallBackHandler)(const void *buffer, int64_t n);

    ReadCallBackHandler ReadCallBack;
    WriteCallBackHandler WriteCallBack;

    CallBackStream(ReadCallBackHandler read, WriteCallBackHandler write);
    virtual int64_t Read(void *buffer, int64_t n);
    virtual int64_t Write(const void *buffer, int64_t n);
};
