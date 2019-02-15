#pragma once

#include <list.hpp>
#include <types.hpp>

class INode;
class StringBuilder;

class DEntry
{
    static void getPath(DEntry *dentry, StringBuilder &sb);
    void getFSLabelAndID(char *buf, size_t bufSize, int *id);
public:
    DEntry *Parent;
    char *Name;
    ::INode *INode;
    int ReferenceCount;

    DEntry(const char *name, DEntry *parent, class INode *inode);
    size_t GetFullPath(char *buffer, size_t bufferSize);
    ~DEntry();
};
