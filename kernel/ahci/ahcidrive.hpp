#pragma once

#include <ata.hpp>
#include <drive.hpp>
#include <ints.hpp>
#include <list.hpp>
#include <types.hpp>

struct HBA_MEM;
struct HBA_PORT;
struct HBA_CMD_HEADER;
struct HBA_FIS;
struct HBA_CMD_TBL;

class Semaphore;

class AHCIDrive : public Drive
{
    class Port;

    enum class DeviceType
    {
        None = 0,
        Unknown,
        ATA,
        ATAPI,
        PM,
        SEMB
    };

    class Controller
    {
        static bool interrupt(Ints::State *state, void *context);

        Ints::Handler interruptHandler;
    public:
        volatile HBA_MEM *Registers;
        uint8_t IRQ;
        Port *Ports[32];

        Controller(uintptr_t base, uint8_t irq);
        void Enable();
        void Disable();
        void EnableInterrupts();
        void DisableInterrupts();
        void Reset();
        ~Controller();
    };

    class Port
    {
        friend class AHCIDrive;

        Controller *Parent;
        int PortNumber;
        volatile HBA_PORT *Registers;
        volatile HBA_CMD_HEADER *CmdHeader;
        volatile HBA_FIS *FIS;
        volatile HBA_CMD_TBL *CmdTable;
        size_t MaxPRDTs = 256;
        size_t CmdTableSize;
        DeviceType DevType;
        AHCIDrive *Drive;
        Semaphore *Interrupt;
    public:
        Port(Controller *controller, int portNumber);
        DeviceType GetDeviceType();
        int StartCommandEngine();
        int StopCommandEngine();
        int Rebase();
        int Prepare();
        int IdentifyDrive(ATAIdentifyResponse *resp);
        ~Port();
    };

    static List<AHCIDrive::Controller *> controllers;

    Port *parent;
    int MaxBlockTransfer;

    AHCIDrive(Port *port, size_t sectorSize, uint64_t sectorCount, const char *model, const char *serial, int maxBlockTransfer);
    static char *getStringFromID(ATAIdentifyResponse *id, uint offset, uint length);
    int sectorTransfer(bool write, void *buffer, uint64_t start, int64_t count);
public:
    static void Initialize();
    static void Cleanup();

    virtual int64_t ReadSectors(void *buffer, uint64_t start, int64_t count);
    virtual int64_t WriteSectors(const void *buffer, uint64_t start, int64_t count);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    ~AHCIDrive();
};
