#pragma once

#include <types.hpp>

class ObjectQueue
{
public:
    class Item
    {
        friend class ObjectQueue;
    protected:
        Item *Next;
    };
    typedef bool (*ItemComparer)(Item *a, Item *b);
    typedef bool (*ForEachCallback)(Item *a); // return true to restart the loop
private:
    Item *first;
    static bool defaultItemComparer(Item *a, Item *b);
public:
    ObjectQueue();
    void Add(Item *item, bool prepend);
    Item *Get();
    Item *First();
    bool Remove(Item *item, ItemComparer comparer);
    void ForEach(ForEachCallback action);
    bool Contains(Item *item, ItemComparer comparer);
    void Clear();
};
