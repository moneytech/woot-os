#include <memory.hpp>
#include <objecttree.hpp>
#include <string.hpp>

ObjectTree *ObjectTree::Objects;

void ObjectTree::Initialize()
{
    Objects = new ObjectTree();
    new Directory(Objects->Get(nullptr), "dev");
    new Directory(Objects->Get(nullptr), "proc");
}

ObjectTree::ObjectTree() :
    mutex(true, "objectTree"),
    root(this)
{
}

bool ObjectTree::Lock()
{
    return mutex.Acquire(3000);
}

void ObjectTree::UnLock()
{
    mutex.Release();
}

ObjectTree::Item *ObjectTree::Get(const char *path)
{
    if(!path || !path[0] || path[0] == '/' && !path[1])
        return &root;
    return root.GetChild(path);
}

ObjectTree::~ObjectTree()
{
    for(Item *it : root.children)
        delete it;
}

void ObjectTree::Item::addChild(ObjectTree::Item *item)
{
    parent = this;
    children.Append(item);
}

ObjectTree::Item *ObjectTree::Item::getChild(const char *name)
{
    // skip leading separators
    while(*name == '/')
        ++name;

    // find next separator or end of string
    size_t partLen = 0;
    while(name[partLen] != '/' && name[partLen])
        ++partLen;

    // copy path part locally
    char *part = (char *)ALLOCA(partLen + 1);
    String::Copy(part, name);

    // do search
    for(Item *child : children)
    {
        if(KeyCheck(part))
            return child; // found a match
    }

    // nothing found
    return nullptr;
}

bool ObjectTree::Item::removeChild(ObjectTree::Item *item)
{
    children.Remove(item, nullptr, false);
    item->parent = nullptr;
    item->tree = nullptr;
}

ObjectTree::Item::Item(ObjectTree *tree) :
    tree(tree), parent(nullptr)
{
}

ObjectTree::Item::Item(ObjectTree::Item *parent) :
    tree(parent->tree), parent(parent)
{
    if(!tree->Lock()) return;
    addChild(this);
    tree->UnLock();
}

ObjectTree::Item *ObjectTree::Item::GetChild(const char *name)
{
    if(!tree->Lock()) return nullptr;
    Item *res = getChild(name);
    tree->UnLock();
    return res;
}

ObjectTree::Item *ObjectTree::Item::GetParent()
{
    return parent;
}

List<ObjectTree::Item *> &ObjectTree::Item::GetChildren()
{
    return children;
}

bool ObjectTree::Item::KeyCheck(const char *name)
{
    return false;
}

bool ObjectTree::Item::GetDisplayName(char *buf, size_t bufSize)
{
    return false;
}

ObjectTree::Item::~Item()
{
    if(parent) parent->removeChild(this);
    for(Item *child : children)
        delete child;
}

ObjectTree::Directory::Directory(ObjectTree::Item *parent, const char *name) :
    Item(parent), name(String::Duplicate(name))
{
}

bool ObjectTree::Directory::KeyCheck(const char *name)
{
    return !String::Compare(this->name, name);
}

ObjectTree::Directory::~Directory()
{
    if(name) delete[] name;
}
