#pragma once

#include <list.hpp>
#include <mutex.hpp>
#include <types.hpp>

#define OBJTREE_MAX_NAME_LEN 63

class StringBuilder;

class ObjectTree
{
public:
    class Item
    {
        friend class ObjectTree;
        void debugDump(char *workBuffer, size_t bufSize, int indent);
        void addChild(Item *item);
        bool containsChild(const char *name);
        Item *getChild(const char *name, bool create);
        bool removeChild(Item *item);
        bool unRegister(Item *item);
    protected:
        ObjectTree *tree;
        Item *parent;
        List<Item *> children;

        Item();
    public:
        bool AddChild(Item *item);
        bool ContainsChild(const char *name);
        Item *GetChild(const char *name);
        bool RemoveChild(const char *name);

        // these 2 functions should be only used when tree is locked
        Item *GetParent();
        List<Item *> &GetChildren();

        virtual bool KeyCheck(const char *name);
        virtual void GetDisplayName(char *buf, size_t bufSize);
        virtual ~Item();
    };

    class Directory : public Item
    {
        char *name;
    public:
        Directory(const char *name);
        virtual bool KeyCheck(const char *name);
        virtual void GetDisplayName(char *buf, size_t bufSize);
        virtual ~Directory();
    };
private:
    Mutex mutex;
    Directory root;

public:
    static ObjectTree *Objects;

    static void Initialize(); // initialize main object tree;

    ObjectTree();
    bool Lock();
    void UnLock();
    bool Register(const char *dir, Item *item);
    bool UnRegister(Item *item);
    bool Contains(const char *path);
    Item *Get(const char *path);
    Item *MakeDir(const char *path);
    void DebugDump();
    ~ObjectTree();
};
