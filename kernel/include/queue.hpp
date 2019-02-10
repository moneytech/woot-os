#pragma once

#include <types.hpp>

template<class T>
class Queue
{
    T *Data;
    size_t Capacity;
    uint Head; // Points to slot to be written on next write
    uint Tail; // Points to slot to be read on next read
    bool Full;

    uint getIndex(int elem)
    {
        return (Tail + elem) % Capacity;
    }
public:
    Queue(size_t capacity) :
        Data(new T[capacity]),
        Capacity(capacity),
        Head(0),
        Tail(0),
        Full(false)
    {
    }

    bool IsEmpty()
    {
        return (Head == Tail) && !Full;
    }

    bool IsFull()
    {
        return Full;
    }

    bool Write(T data)
    {
        if(Full) return false;
        Data[Head++] = data;
        Head %= Capacity;
        Full = Head == Tail;
        return true;
    }

    T Read(bool *ok)
    {
        if(ok) *ok = true;
        if(IsEmpty())
        {
            if(ok) *ok = false;
            return T();
        }
        T r = Data[Tail++];
        Tail %= Capacity;
        Full = false;
        return r;
    }

    T Peek(bool *ok)
    {
        if(IsEmpty())
        {
            if(ok) *ok = false;
            return T();
        }
        if(ok) *ok = true;
        return Data[Tail];
    }

    T Peek(int elem, bool *ok)
    {
        if(elem >= Count())
        {
            if(ok) *ok = false;
            return T();
        }
        if(ok) *ok = true;
        return Data[getIndex(elem)];
    }

    uint ReplaceFirst(T value, T replacement)
    {
        if(IsEmpty())
            return 0;

        for(uint i = Tail; i != Head; i = (i + 1) % Capacity)
        {
            if(Data[i] == value)
            {
                Data[i] = replacement;
                return 1;
            }
        }
        return 0;
    }

    uint ReplaceAll(T value, T replacement)
    {
        if(IsEmpty())
            return 0;

        uint replaced = 0;
        for(uint i = Tail; i != Head; i = (i + 1) % Capacity)
        {
            if(Data[i] == value)
            {
                Data[i] = replacement;
                ++replaced;
            }
        }
        return replaced;
    }

    uint Count()
    {
        if(IsEmpty()) return 0;
        if(Head > Tail) return Head - Tail;
        return Capacity - Tail + Head;
    }

    uint Remove(int elem)
    {
        uint count = Count();
        if(elem >= count)
            return 0;
        if(!elem) // first element
            Tail = (Tail + 1) % Capacity;
        else if(elem == (count - 1)) // last element
            Head = Head ? Head - 1 : Capacity - 1;
        else
        {   // somewhere in the middle
            for(uint i = elem; i; --i)
                Data[getIndex(i)] = Data[getIndex(i - 1)];
            Tail = (Tail + 1) % Capacity;
        }
        Full = false;
        return 1;
    }

    uint RemoveFirst(T value)
    {
        if(IsEmpty()) return 0;
        uint count = Count();
        for(uint i = 0; i < count; ++i)
        {
            if(Data[getIndex(i)] == value)
                return Remove(i);
        }
        return 0;
    }

    void Clear()
    {
        for(;;)
        {
            bool ok = false;
            Read(&ok);
            if(!ok) break;
        }
    }

    ~Queue()
    {
        if(Data) delete[] Data;
    }
};
