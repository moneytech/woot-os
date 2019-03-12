#pragma once

#include <errno.h>
#include <semaphore.hpp>

template<class T>
class MessageQueue
{
    T *data;
    Semaphore s, m;
    size_t cap, h, t;
public:
    MessageQueue(size_t capacity) :
        data(new T[capacity]),
        s(capacity), m(0),
        cap(capacity), h(0), t(0)
    {
    }

    T Read(int timeout)
    {
        T val;
        if(!m.Wait(timeout < 0 ? 0 : timeout, timeout >= 0, false))
            return val;
        val = data[t];
        t = (t + 1) % cap;
        s.Signal(nullptr);
        return val;
    }

    int Read(T *buf, int timeout)
    {
        if(!buf) return -EINVAL;
        if(!m.Wait(timeout < 0 ? 0 : timeout, timeout >= 0, false))
            return ETIMEOUT;
        *buf = data[t];
        t = (t + 1) % cap;
        s.Signal(nullptr);
        return ESUCCESS;
    }

    int Write(T val, int timeout)
    {
        if(!s.Wait(timeout < 0 ? 0 : timeout, timeout >= 0, false))
            return ETIMEOUT;
        data[h] = val;
        h = (h + 1) % cap;
        m.Signal(nullptr);
        return ESUCCESS;
    }

    ~MessageQueue()
    {
        if(data) delete[] data;
    }
};
