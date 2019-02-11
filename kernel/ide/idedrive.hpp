#pragma once

#include <ata.hpp>
#include <drive.hpp>
#include <ints.hpp>
#include <types.hpp>

class Mutex;
class Semaphore;

class IDEDrive : public Drive
{
    class Controller
    {
        friend class IDEDrive;

        Mutex *Lock;
        uint16_t Base, Control, BM;
        uint8_t IRQ;
        Ints::Handler InterruptHandler;
        IDEDrive *Drives[2];

        bool waitFornBSYorERR_nolock(int timeout, uint8_t *status);
        bool waitForDRQorERR_nolock(int timeout, uint8_t *status);
        bool waitFornBSY_nolock(int timeout, uint8_t *status);
        bool waitForDRDY_nolock(int timeout, uint8_t *status);
        bool identify_nolock(ATAIdentifyResponse *id, bool slave, bool atapi);

        Controller(uint16_t base, uint16_t control, uint16_t bm, uint8_t irq);
        void EnableIRQ();
        void DisableIRQ();
        void Wait400ns();
        void SoftReset();
        bool WaitFornBSYorERR(int timeout, uint8_t *status);
        bool WaitForDRQorERR(int timeout, uint8_t *status);
        bool WaitFornBSY(int timeout, uint8_t *status);
        bool WaitForDRDY(int timeout, uint8_t *status);
        bool Identify(ATAIdentifyResponse *id, bool slave, bool atapi);
    };

#pragma pack(push, 1)
    typedef struct PRDTEntry
    {
        uint32_t BufferAddress;
        uint16_t ByteCount;
        uint16_t Reserved : 15;
        uint16_t Last : 1;
    } PRDTEntry;
#pragma pack(pop)

    static const uint16_t defAddrs[4];

    Controller *Controller;
    bool Slave, ATAPI;
    ATAIdentifyResponse IDResp;
    size_t PRDTsAllocated;
    PRDTEntry *PRDTs;
    Semaphore *TransferDone;
    int MaxBlockTransfer;

    static bool interrupt(Ints::State *state, void *context);

    IDEDrive(ATAIdentifyResponse *id, bool atapi, class Controller *ctrl, bool slave);
    static char *getStringFromID(ATAIdentifyResponse *id, uint offset, uint length);
    int sectorTransfer(bool write, void *buffer, uint64_t start, int64_t count);
public:
    static void Initialize();
    static void Cleanup();

    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~IDEDrive();
};
