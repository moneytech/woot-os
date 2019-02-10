#include <callbackstream.hpp>
#include <errno.h>

CallBackStream::CallBackStream(CallBackStream::ReadCallBackHandler read, CallBackStream::WriteCallBackHandler write) :
    ReadCallBack(read), WriteCallBack(write)
{
}

int64_t CallBackStream::Read(void *buffer, int64_t n)
{
    if(ReadCallBack)
        return ReadCallBack(buffer, n);
    return -ENOSYS;
}

int64_t CallBackStream::Write(const void *buffer, int64_t n)
{
    if(WriteCallBack)
        return WriteCallBack(buffer, n);
    return -ENOSYS;
}
