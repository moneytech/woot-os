#pragma once

#include <list.hpp>
#include <mutex.hpp>
#include <types.hpp>

class ObjectTree
{
public:
    class Item
    {
        friend class ObjectTree;
        void addChild(Item *item);
        Item *getChild(const char *name);
        bool removeChild(Item *item);
    protected:
        ObjectTree *tree;
        Item *parent;
        List<Item *> children;

        Item(ObjectTree *tree);
        Item(Item *parent);
    public:
        Item *GetChild(const char *name);
        bool RemoveChild(const char *name);

        virtual bool KeyCheck(const char *name);
        virtual ~Item();
    };

    class Directory : public Item
    {
        char *name;
    public:
        Directory(Item *parent, const char *name);
        virtual bool KeyCheck(const char *name);
        virtual ~Directory();
    };
private:
    Mutex mutex;
    Item root;

    bool lock();
    void unLock();
public:
    static ObjectTree *Objects;

    static void Initialize(); // initialize main object tree;

    ObjectTree();
    Item *Get(const char *path);
    ~ObjectTree();
};
