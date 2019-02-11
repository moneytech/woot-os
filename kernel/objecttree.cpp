#include <debug.hpp>
#include <memory.hpp>
#include <objecttree.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>

ObjectTree *ObjectTree::Objects;

void ObjectTree::Initialize()
{
    Objects = new ObjectTree();
    Item *root = Objects->Get(nullptr);
}

ObjectTree::ObjectTree() :
    mutex(true, "objectTree"),
    root("/")
{
    root.tree = this;
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

ObjectTree::Item *ObjectTree::MakeDir(const char *path)
{
    if(!path || !path[0] || path[0] == '/' && !path[1])
        return &root;
    if(!Lock()) return nullptr;
    Item *res = root.getChild(path, true);
    UnLock();
    return res;
}

void ObjectTree::DebugDump()
{
    if(!Lock())
    {
        DEBUG("%s: %s: Lock() failed\n", __FILE__, __PRETTY_FUNCTION__);
        return;
    }
    char buf[64];
    root.debugDump(buf, sizeof(buf), 0);
    UnLock();
}

ObjectTree::~ObjectTree()
{
    for(Item *it : root.children)
        delete it;
}

void ObjectTree::Item::debugDump(char *workBuffer, size_t bufSize, int indent)
{
    workBuffer[0] = 0;
    GetDisplayName(workBuffer, bufSize);
    for(int i = 0; i < indent; ++i)
        DEBUG(" ");
    DEBUG("%s\n", workBuffer);
    for(Item *it : children)
        it->debugDump(workBuffer, bufSize, indent + 1);
}

void ObjectTree::Item::addChild(ObjectTree::Item *item)
{
    item->parent = this;
    item->tree = tree;

    // make sure objects are kept in alphabetical order
    children.InsertBefore(item, [](auto a, auto b) -> bool
    {
        char bufa[64], bufb[64];
        a->GetDisplayName(bufa, sizeof(bufa));
        b->GetDisplayName(bufb, sizeof(bufb));
        return String::Compare(bufa, bufb) > 0;
    });
}

ObjectTree::Item *ObjectTree::Item::getChild(const char *name, bool create)
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
    String::Copy(part, name, partLen);
    part[partLen] = 0;

    // do search
    for(Item *child : children)
    {
        if(child->KeyCheck(part))
        {   // found a match
            return name[partLen] ?
                        child->getChild(name + partLen, create) :
                        child;
        }
    }

    if(create)
    {   // try to create new node
        Directory *dir = new Directory(part);
        addChild(dir);
        return name[partLen] ?
                    dir->getChild(name + partLen, true) :
                    dir;
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

ObjectTree::Item::Item()
{
}

bool ObjectTree::Item::AddChild(ObjectTree::Item *item)
{
    if(!tree->Lock()) return false;
    addChild(item);
    tree->UnLock();
    return true;
}

ObjectTree::Item *ObjectTree::Item::GetChild(const char *name)
{
    if(!tree->Lock()) return nullptr;
    Item *res = getChild(name, false);
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

void ObjectTree::Item::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("@%P", this);
}

ObjectTree::Item::~Item()
{
    if(parent) parent->removeChild(this);
    for(Item *child : children)
        delete child;
}

ObjectTree::Directory::Directory(const char *name) :
    name(String::Duplicate(name))
{
}

bool ObjectTree::Directory::KeyCheck(const char *name)
{
    return !String::Compare(this->name, name);
}

void ObjectTree::Directory::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteStr(name);
}

ObjectTree::Directory::~Directory()
{
    if(name) delete[] name;
}
