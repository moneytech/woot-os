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

        // these 2 functions should be only used when tree is locked
        Item *GetParent();
        List<Item *> &GetChildren();

        virtual bool KeyCheck(const char *name);
        virtual bool GetDisplayName(char *buf, size_t bufSize);
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

public:
    static ObjectTree *Objects;

    static void Initialize(); // initialize main object tree;

    ObjectTree();
    bool Lock();
    void UnLock();
    Item *Get(const char *path);
    ~ObjectTree();
};
