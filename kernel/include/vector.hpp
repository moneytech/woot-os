#pragma once

#include <heap.hpp>
#include <memory.hpp>
#include <types.hpp>

template<class T>
class Vector
{
    class iterator
    {
        Vector *vec;
        uint idx;
    public:
        iterator(Vector *vec) : iterator(vec, 0) {}
        iterator(Vector *vec, uint idx) : vec(vec), idx(idx) {}
        bool operator !=(iterator b) { return idx != b.idx || vec != b.vec; }
        bool operator ==(iterator b) { return vec == b.vec && idx == b.idx; }
        iterator operator ++() { idx++; return *this; }
        T operator *() { return vec->data[idx]; }
    };

    size_t maxSize;
    size_t currentSize;
    size_t allocatedSize;
    size_t allocIncr;
    T *data;

    bool upSize()
    {
        size_t newSize = currentSize + 1;
        if(newSize > maxSize) return false;
        if(newSize > allocatedSize)
        {
            size_t newAllocSize = allocatedSize + allocIncr;
            if(newAllocSize > maxSize) newAllocSize = maxSize;
            data = (T *)Heap::Resize(data, newAllocSize * sizeof(T), 0, false);
            if(newAllocSize && !data) return false;
            allocatedSize = newAllocSize;
        }
        return true;
    }
public:
    Vector() : Vector(0, 1, __SIZE_MAX__) {}
    Vector(size_t preAlloc, size_t allocIncr, size_t maxSize) :
        maxSize(maxSize),
        currentSize(0),
        allocatedSize(preAlloc > maxSize ? maxSize : preAlloc),
        allocIncr(allocIncr),
        data((T *)Heap::Allocate(allocatedSize * sizeof(T), 0, false))
    {
    }

    size_t Size() const
    {
        return currentSize;
    }

    size_t AllocSize() const
    {
        return allocatedSize;
    }

    bool Append(T value)
    {
        if(!upSize()) return false;
        data[currentSize++] = value;
        return true;
    }

    bool Prepend(T value)
    {
        if(!upSize()) return false;
        Memory::Move(data + 1, data, currentSize * sizeof(T));
        data[0] = value;
        ++currentSize;
        return true;
    }

    bool InsertBefore(uint idx, T value)
    {
        if(idx > currentSize) idx = currentSize;
        if(!upSize()) return false;
        Memory::Move(data + 1 + idx, data + idx, (currentSize - idx) * sizeof(T));
        data[idx] = value;
        ++currentSize;
        return true;
    }

    bool InsertAfter(uint idx, T value)
    {
        if(idx >= currentSize) idx = currentSize - 1;
        if(!upSize()) return false;
        Memory::Move(data + 2 + idx, data + 1 + idx, (currentSize - idx - 1) * sizeof(T));
        data[idx + 1] = value;
        ++currentSize;
        return true;
    }

    bool RemoveOne(T value)
    {
        for(uint i = 0; i < currentSize; ++i)
        {
            if(value == data[i])
                return RemoveAt(i);
        }
        return false;
    }

    size_t RemoveAll(T value)
    {
        size_t res = 0;
        for(uint i = 0; i < currentSize; ++i)
        {
            if(value == data[i])
                res += RemoveAt(i--) ? 1 : 0;
        }
        return res;
    }

    bool RemoveAt(uint idx)
    {
        if(idx >= currentSize) return false;
        Memory::Move(data + idx, data + idx + 1, (currentSize - idx - 1) * sizeof(T));
        --currentSize;
        return true;
    }

    void Trim()
    {
        if(currentSize < allocatedSize)
        {
            data = Heap::Resize(data, currentSize * sizeof(T), 0, true);
            allocatedSize = currentSize;
        }
    }

    bool Swap(uint idx1, uint idx2)
    {
        if(idx1 >= currentSize || idx2 >= currentSize)
            return false;
        T tmp = data[idx2];
        data[idx2] = data[idx1];
        data[idx1] = tmp;
        return true;
    }

    iterator begin()
    {
        return iterator(this, 0);
    }

    iterator end()
    {
        return iterator(this, currentSize);
    }

    ~Vector()
    {
        if(data) Heap::Free(data);
    }
};
