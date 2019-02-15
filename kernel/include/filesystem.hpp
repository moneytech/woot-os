#pragma once

#define FS_DIR  "/dev/fs"

#include <ktypes.h>
#include <list.hpp>
#include <mutex.hpp>
#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>
#include <uuid.hpp>

class DEntry;
class FileSystemType;
class INode;
class Volume;

class FileSystem : public ObjectTree::Item
{
    static Sequencer<int> ids;
    static Mutex mutex;
    static List<DEntry *> dentryCache;
    static List<INode *> inodeCache;
protected:
    int id;
    Volume *volume;
    FileSystemType *type;
    DEntry *root;

    FileSystem(Volume *volume, FileSystemType *type);
public:
    static void Initialize();
    static void Cleanup();
    static bool Lock();
    static void UnLock();
    static int DetectAll();
    static void PutINode(INode *inode);
    static DEntry *GetDEntry(DEntry *parent, const char *name);
    static DEntry *GetDEntry(DEntry *dentry);
    static void PutDEntry(DEntry *dentry);

    INode *GetINode(ino_t number);
    int GetID();
    DEntry *GetRoot();

    virtual bool GetLabel(char *buffer, size_t num);
    virtual UUID GetUUID();
    virtual INode *ReadINode(ino_t number);
    virtual bool WriteINode(INode *inode);
    virtual bool WriteSuperBlock();
    virtual int Synchronize();

    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
