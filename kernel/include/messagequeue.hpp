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

    T Read(int timeout, int *timeleft)
    {
        T val;
        int _timeleft = m.Wait(timeout < 0 ? 0 : timeout, timeout == 0, false);
        if(_timeleft < 0)
            return val;
        val = data[t];
        t = (t + 1) % cap;
        s.Signal(nullptr);
        if(timeleft) *timeleft = _timeleft;
        return val;
    }

    int Read(T *buf, int timeout)
    {
        if(!buf) return -EINVAL;
        int timeleft = m.Wait(timeout < 0 ? 0 : timeout, timeout == 0, false);
        if(timeleft < 0)
            return -ETIMEOUT;
        *buf = data[t];
        t = (t + 1) % cap;
        s.Signal(nullptr);
        return timeleft;
    }

    int Write(T val, int timeout)
    {
        int timeleft = s.Wait(timeout < 0 ? 0 : timeout, timeout == 0, false);
        if(timeleft < 0)
            return -ETIMEOUT;
        data[h] = val;
        h = (h + 1) % cap;
        m.Signal(nullptr);
        return timeleft;
    }

    ~MessageQueue()
    {
        if(data) delete[] data;
    }
};
