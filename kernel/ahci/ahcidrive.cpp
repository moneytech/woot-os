#include <../ahci/ahcidrive.hpp>
#include <cpu.hpp>
#include <debug.hpp>
#include <errno.h>
#include <ints.hpp>
#include <irqs.hpp>
#include <list.hpp>
#include <memory.hpp>
#include <misc.hpp>
#include <mutex.hpp>
#include <paging.hpp>
#include <pci.hpp>
#include <semaphore.hpp>
#include <stringbuilder.hpp>
#include <sysdefs.h>
#include <time.hpp>

#pragma pack(push, 1)
struct FIS_REG_H2D
{
    uint8_t FISType;        // FIS_TYPE_REG_H2D
    uint8_t PMPort : 4;     // Port multiplier
    uint8_t Reserved0 : 3;  // Reserved
    uint8_t C : 1;          // 1: Command, 0: Control
    uint8_t Command;        // Command register
    uint8_t FeatureL;       // Feature register, 7:0
    uint8_t LBA0;           // LBA low register, 7:0
    uint8_t LBA1;           // LBA mid register, 15:8
    uint8_t LBA2;           // LBA high register, 23:16
    uint8_t Device;         // Device register
    uint8_t LBA3;           // LBA register, 31:24
    uint8_t LBA4;           // LBA register, 39:32
    uint8_t LBA5;           // LBA register, 47:40
    uint8_t FeatureH;       // Feature register, 15:8
    uint8_t CountL;         // Count register, 7:0
    uint8_t CountH;         // Count register, 15:8
    uint8_t ICC;            // Isochronous command completion
    uint8_t Control;        // Control register
    uint8_t Reserved1[4];   // Reserved
};

struct FIS_REG_D2H
{
    uint8_t FISType;        // FIS_TYPE_REG_D2H
    uint8_t PMPort : 4;     // Port multiplier
    uint8_t Reserved0 : 2;  // Reserved
    uint8_t I : 1;          // Interrupt bit
    uint8_t Reserved1 : 1;  // Reserved
    uint8_t Status;         // Status register
    uint8_t Error;          // Error register
    uint8_t LBA0;           // LBA low register, 7:0
    uint8_t LBA1;           // LBA mid register, 15:8
    uint8_t LBA2;           // LBA high register, 23:16
    uint8_t Device;         // Device register
    uint8_t LBA3;           // LBA register, 31:24
    uint8_t LBA4;           // LBA register, 39:32
    uint8_t LBA5;           // LBA register, 47:40
    uint8_t Reserved2;      // Reserved
    uint8_t CountL;         // Count register, 7:0
    uint8_t CountH;         // Count register, 15:8
    uint8_t Reserved3[2];   // Reserved
    uint8_t Reserved4[4];   // Reserved
};

struct FIS_DATA
{
    uint8_t FISType;        // FIS_TYPE_DATA
    uint8_t PMPort : 4;     // Port multiplier
    uint8_t Reserved0 : 4;  // Reserved
    uint8_t Reserved1[2];   // Reserved
};

struct FIS_PIO_SETUP
{
    uint8_t FISType;        // FIS_TYPE_PIO_SETUP
    uint8_t PMPort : 4;     // Port multiplier
    uint8_t Reserved0 : 1;  // Reserved
    uint8_t D : 1;          // Data transfer direction, 1 - device to host
    uint8_t I : 1;          // Interrupt bit
    uint8_t Reserved1 : 1;  // Reserved
    uint8_t Status;         // Status register
    uint8_t Error;          // Error register
    uint8_t LBA0;           // LBA low register, 7:0
    uint8_t LBA1;           // LBA mid register, 15:8
    uint8_t LBA2;           // LBA high register, 23:16
    uint8_t Device;         // Device register
    uint8_t LBA3;           // LBA register, 31:24
    uint8_t LBA4;           // LBA register, 39:32
    uint8_t LBA5;           // LBA register, 47:40
    uint8_t Reserved2;      // Reserved
    uint8_t CountL;         // Count register, 7:0
    uint8_t CountH;         // Count register, 15:8
    uint8_t Reserved3;      // Reserved
    uint8_t EStatus;        // New value of status register
    uint16_t TC;            // Transfer count
    uint8_t Reserved4[2];   // Reserved
};

struct FIS_DMA_SETUP
{
    uint8_t FISType;        // FIS_TYPE_DMA_SETUP
    uint8_t PMPort : 4;     // Port multiplier
    uint8_t Reserved0 : 1;  // Reserved
    uint8_t D : 1;          // Data transfer direction, 1 - device to host
    uint8_t I : 1;          // Interrupt bit
    uint8_t A : 1;          // Auto-activate. Specifies if DMA Activate FIS is needed
    uint8_t Reserved1[2];   // Reserved
    uint64_t DMABufferID;   // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.
    uint32_t Reserved2;     // More reserved
    uint32_t DMABufOffset;  // Byte offset into buffer. First 2 bits must be 0
    uint32_t TransferCount; // Number of bytes to transfer. Bit 0 must be 0
    uint32_t Reserved3;     // Reserved
};

struct HBA_PORT
{
    uint32_t CLB;           // 0x00, command list base address, 1K-byte aligned
    uint32_t CLBU;          // 0x04, command list base address upper 32 bits
    uint32_t FB;            // 0x08, FIS base address, 256-byte aligned
    uint32_t FBU;           // 0x0C, FIS base address upper 32 bits
    uint32_t IS;            // 0x10, interrupt status
    uint32_t IE;            // 0x14, interrupt enable
    uint32_t CMD;           // 0x18, command and status
    uint32_t Reserved0;     // 0x1C, Reserved
    uint32_t TFD;           // 0x20, task file data
    uint32_t SIG;           // 0x24, signature
    uint32_t SSTS;          // 0x28, SATA status (SCR0:SStatus)
    uint32_t SCTL;          // 0x2C, SATA control (SCR2:SControl)
    uint32_t SERR;          // 0x30, SATA error (SCR1:SError)
    uint32_t SACT;          // 0x34, SATA active (SCR3:SActive)
    uint32_t CI;            // 0x38, command issue
    uint32_t SNTF;          // 0x3C, SATA notification (SCR4:SNotification)
    uint32_t FBS;           // 0x40, FIS-based switch control
    uint32_t Reserved1[11]; // 0x44 ~ 0x6F, Reserved
    uint32_t Vendor[4];     // 0x70 ~ 0x7F, vendor specific
};

struct HBA_MEM
{
    uint32_t CAP;                   // 0x00, Host capability
    uint32_t GHC;                   // 0x04, Global host control
    uint32_t IS;                    // 0x08, Interrupt status
    uint32_t PI;                    // 0x0C, Port implemented
    uint32_t VS;                    // 0x10, Version
    uint32_t CCC_CTL;               // 0x14, Command completion coalescing control
    uint32_t CCC_PORTS;             // 0x18, Command completion coalescing ports
    uint32_t EM_LOC;                // 0x1C, Enclosure management location
    uint32_t EM_CTL;                // 0x20, Enclosure management control
    uint32_t CAP2;                  // 0x24, Host capabilities extended
    uint32_t BOHC;                  // 0x28, BIOS/OS handoff control and status
    uint8_t Reserved[0xA0 - 0x2C];  // Reserved
    uint8_t Vendor[0x100 - 0xA0];   // Vendor specific registers
    HBA_PORT Ports[32];             // Port control registers
};

struct HBA_CMD_HEADER
{
    uint8_t CFL : 5;        // Command FIS length in DWORDS, 2 ~ 16
    uint8_t A : 1;          // ATAPI
    uint8_t W : 1;          // Write, 1: H2D, 0: D2H
    uint8_t P : 1;          // Prefetchable
    uint8_t R : 1;          // Reset
    uint8_t B : 1;          // BIST
    uint8_t C : 1;          // Clear busy upon R_OK
    uint8_t Reserved0 : 1;  // Reserved
    uint8_t PMP : 4;        // Port multiplier port
    uint16_t PRDTL;         // Physical region descriptor table length in entries
    uint32_t PRDBC;         // Physical region descriptor byte count transferred
    uint32_t CTBA;          // Command table descriptor base address
    uint32_t CTBAU;         // Command table descriptor base address upper 32 bits
    uint32_t Reserved1[4];  // Reserved
};

struct HBA_PRDT_ENTRY
{
    uint32_t DBA;           // Data base address

    uint32_t DBAU;          // Data base address upper 32 bits

    uint32_t Reserved0;     // Reserved

    uint32_t PRDBC : 22;    // Byte count, 4M max
    uint32_t Reserved1 : 9; // Reserved
    uint32_t I : 1;         // Interrupt on completion
};

struct HBA_CMD_TBL
{
    uint8_t CFIS[64];       // Command FIS
    uint8_t ACMD[16];       // ATAPI command, 12 or 16 bytes
    uint8_t Reserved[48];   // Reserved
    HBA_PRDT_ENTRY PRDT[0]; // PRDT entries
};

typedef uint64_t FIS_DEV_BITS;

struct HBA_FIS
{
    FIS_DMA_SETUP dsfis;    // DMA Setup FIS
    uint8_t pad0[4];
    FIS_PIO_SETUP psfis;    // PIO Setup FIS
    uint8_t pad1[12];
    FIS_REG_D2H rfis;       // Register – Device to Host FIS
    uint8_t pad2[4];
    FIS_DEV_BITS sdbfis;    // Set Device Bit FIS
    uint8_t ufis[64];
    uint8_t rsv[0x100 - 0xA0];
};
#pragma pack(pop)

#define FIS_TYPE_REG_H2D    0x27    // Register FIS - host to device
#define FIS_TYPE_REG_D2H    0x34    // Register FIS - device to host
#define FIS_TYPE_DMA_ACT    0x39    // DMA activate FIS - device to host
#define FIS_TYPE_DMA_SETUP  0x41    // DMA setup FIS - bidirectional
#define FIS_TYPE_DATA       0x46    // Data FIS - bidirectional
#define FIS_TYPE_BIST       0x58    // BIST activate FIS - bidirectional
#define FIS_TYPE_PIO_SETUP  0x5F    // PIO setup FIS - device to host
#define FIS_TYPE_DEV_BITS   0xA1    // Set device bits FIS - device to host


#define HBA_GHC_AE      (1 << 31)
#define HBA_GHC_MRSM    (1 << 2)
#define HBA_GHC_IE      (1 << 1)
#define HBA_GHC_HR      (1 << 0)

#define	PORT_SIG_SATA   0x00000101  // SATA drive
#define	PORT_SIG_SATAPI 0xEB140101  // SATAPI drive
#define	PORT_SIG_SEMB   0xC33C0101  // Enclosure management bridge
#define	PORT_SIG_PM     0x96690101  // Port multiplier

#define PORT_CMD_ICC_IDLE       (0 << 28)
#define PORT_CMD_ICC_ACTIVE     (1 << 28)
#define PORT_CMD_ICC_PARTIAL    (2 << 28)
#define PORT_CMD_ICC_SLUMBER    (6 << 28)
#define PORT_CMD_ICC_DEV_SLEEP  (8 << 28)
#define PORT_CMD_ASP            (1 << 27)
#define PORT_CMD_ALPE           (1 << 26)
#define PORT_CMD_DLAE           (1 << 25)
#define PORT_CMD_ATAPI          (1 << 24)
#define PORT_CMD_APSTE          (1 << 23)
#define PORT_CMD_FBSCP          (1 << 22)
#define PORT_CMD_ESP            (1 << 21)
#define PORT_CMD_CPD            (1 << 20)
#define PORT_CMD_MPSP           (1 << 19)
#define PORT_CMD_HPCP           (1 << 18)
#define PORT_CMD_PMA            (1 << 17)
#define PORT_CMD_CPS            (1 << 16)
#define PORT_CMD_CR             (1 << 15)
#define PORT_CMD_FR             (1 << 14)
#define PORT_CMD_MPSS           (1 << 13)
#define PORT_CMD_CCS            (31 << 8)   // mask
#define PORT_CMD_FRE            (1 << 4)
#define PORT_CMD_CLO            (1 << 3)
#define PORT_CMD_POD            (1 << 2)
#define PORT_CMD_SUD            (1 << 1)
#define PORT_CMD_ST             (1 << 0)

#define PORT_SSTS_IPM_NOT_PRESENT   0
#define PORT_SSTS_IPM_ACTIVE        1
#define PORT_SSTS_IPM_PARTIAL       2
#define PORT_SSTS_IPM_SLUMBER       6
#define PORT_SSTS_IPM_DEV_SLEEP     8

#define PORT_SSTS_SPD_NOT_PRESENT   0
#define PORT_SSTS_SPD_GEN1          1
#define PORT_SSTS_SPD_GEN2          2
#define PORT_SSTS_SPD_GEN3          3

#define PORT_SSTS_DET_NOT_PRESENT       0
#define PORT_SSTS_DET_PRESENT_NOCOMM    1
#define PORT_SSTS_DET_PRESENT_COMM      3
#define PORT_SSTS_DET_OFFLINE           4

static const char *deviceTypeNames[] =
{
    "null device",
    "unknown device",
    "ATA device",
    "ATAPI device",
    "port multiplier",
    "SEMB device"
};

List<AHCIDrive::Controller *> AHCIDrive::controllers;

AHCIDrive::AHCIDrive(Port *port, size_t sectorSize, uint64_t sectorCount, const char *model, const char *serial, int maxBlockTransfer) :
    Drive(sectorSize, port->DevType == DeviceType::ATAPI ? 0x7FFFFFFF : sectorCount, model, serial),
    parent(port), MaxBlockTransfer(maxBlockTransfer)
{
}

char *AHCIDrive::getStringFromID(ATAIdentifyResponse *id, uint offset, uint length)
{
    uint8_t *src = (uint8_t *)id;
    src += offset;
    char *dst = (char *)new char[length + 1];
    Memory::Move(dst, src, length);
    for(uint i = 0; i < length; i += 2)
        swap(char, dst[i], dst[i + 1]);
    for(int i = length; --i;)
    {
        if(dst[i] != ' ')
            break;
        dst[i] = 0;
    }
    dst[length] = 0;

    // skip any leading spaces
    int skip = 0;
    while(dst[skip] == ' ') ++skip;
    if(skip) Memory::Move(dst, dst + skip, (length + 1) - skip);
    return dst;
}

int AHCIDrive::sectorTransfer(bool write, void *buffer, uint64_t start, int64_t count)
{
    //DEBUG("[ahcidrive] sectorTransfer: start: %lu\n", start);

    if(!count) return 0;

    int res = parent->StopCommandEngine();
    if(res) return res;

    parent->Prepare();
    uintptr_t bufPtr = Paging::GetDMAPhysicalAddress(buffer);
    size_t prdtCount = (count + MaxBlockTransfer - 1) / MaxBlockTransfer;
    size_t bytesLeft = count * SectorSize;
    size_t bytesPerPRDT = SectorSize * MaxBlockTransfer;

    if(prdtCount > parent->MaxPRDTs)
    {
        DEBUG("[ahcidrive] prdtCount > parent->MaxPRDTs\n");
        return -ENOMEM;
    }

    // set up command header
    parent->CmdHeader->CFL = sizeof(FIS_REG_H2D) / 4;
    parent->CmdHeader->PRDTL = prdtCount;
    parent->CmdHeader->CTBA = Paging::GetDMAPhysicalAddress((void *)parent->CmdTable);
    parent->CmdHeader->CTBAU = 0;

    // set up PRDTs
    for(int i = 0; i < prdtCount && bytesLeft; ++i)
    {
        size_t byteCount = bytesLeft >= bytesPerPRDT ? bytesPerPRDT : bytesLeft;
        parent->CmdTable->PRDT[i].DBA = (uint32_t)bufPtr;
        parent->CmdTable->PRDT[i].DBAU = 0;
        parent->CmdTable->PRDT[i].PRDBC = byteCount - 1;
        parent->CmdTable->PRDT[i].I = bytesLeft <= bytesPerPRDT;
        bytesLeft -= byteCount;
        bufPtr += byteCount;
    }

    // set up FIS
    if(parent->DevType == DeviceType::ATAPI)
    {   // for SATAPI drive
        FIS_REG_H2D *fis = (FIS_REG_H2D *)parent->CmdTable->CFIS;
        fis->FISType = FIS_TYPE_REG_H2D;
        fis->Command = ATA_CMD_PACKET;
        fis->C = 1;
        fis->FeatureL = 1 | (write ? 0 : 4); // bit 0 DMA bit 2 direction

        // prepare ATAPI packet
        parent->CmdHeader->A = 1;
        Memory::Zero((void *)parent->CmdTable->ACMD, sizeof(parent->CmdTable->ACMD));
        parent->CmdTable->ACMD[0] = write ? 0xAA : 0xA8;
        parent->CmdTable->ACMD[9] = count;
        parent->CmdTable->ACMD[2] = start >> 24;
        parent->CmdTable->ACMD[3] = start >> 16;
        parent->CmdTable->ACMD[4] = start >> 8;
        parent->CmdTable->ACMD[5] = start;
    }
    else
    {   // for SATA drive
        FIS_REG_H2D *fis = (FIS_REG_H2D *)parent->CmdTable->CFIS;
        fis->FISType = FIS_TYPE_REG_H2D;
        fis->Command = write ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT;
        fis->C = 1;
        fis->Device = 0x40; // LBA mode
        fis->LBA0 = start;
        fis->LBA1 = start >> 8;
        fis->LBA2 = start >> 16;
        fis->LBA3 = start >> 24;
        fis->LBA4 = start >> 32;
        fis->LBA5 = start >> 40;
        fis->CountL = count;
        fis->CountH = count >> 8;
    }

    res = parent->StartCommandEngine();
    if(res) return res;

    int retry = 1000;
    while(parent->Registers->TFD & (ATA_DEV_BUSY | ATA_DEV_DRQ) && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;

    // run the command
    parent->Registers->CI = 1;
    if(parent->Interrupt->Wait(5000, false, false) < 0)
        return -EBUSY;

    if(parent->Registers->TFD & ATA_DEV_ERR)
        return -EIO;

    return count;
}

void AHCIDrive::Initialize()
{
    if(!ObjectTree::Objects->Lock()) return;
    ObjectTree::Item *pciDir = ObjectTree::Objects->Get(PCI_BUS_DIR);
    if(!pciDir)
    {
        ObjectTree::Objects->UnLock();
        return;
    }

    for(ObjectTree::Item *item : pciDir->GetChildren())
    {
        PCI::Device *pciDev = (PCI::Device *)item;
        if(pciDev->Class != 0x01 || pciDev->SubClass != 0x06)
            continue;

        DEBUG("[ahcidrive] Found AHCI controller at PCI:%d.%d.%d\n",
              PCI_ADDR_BUS(pciDev->Address),
              PCI_ADDR_DEV(pciDev->Address),
              PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster and MMIO address space accesses
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0006);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        AHCIDrive::Controller *controller = new AHCIDrive::Controller(cfg.Header.Default.BAR[5] & ~7, cfg.Header.Default.InterruptLine);
        controllers.Append(controller);
    }

    ObjectTree::Item *drvDir = ObjectTree::Objects->MakeDir(DRV_DIR);

    for(AHCIDrive::Controller *ctrl : controllers)
    {
        for(int i = 0; i < 32; ++i)
        {
            Port *port = ctrl->Ports[i];
            if(!port) continue;

            DeviceType devType = port->GetDeviceType();

            if(devType == DeviceType::None)
                continue;

            DEBUG("[ahcidrive] Found %s on port %d\n", deviceTypeNames[(int)devType], i);
            int res = port->Rebase();
            if(res)
            {
                DEBUG("[ahcidrive] Couldn't rebase device on port %d (error: %d)\n", i, res);
                continue;
            }

            ATAIdentifyResponse resp;
            res = port->IdentifyDrive(&resp);
            if(res)
            {
                DEBUG("[ahcidrive] Couldn't identify drive on port %d (error: %d)\n", i, res);
                continue;
            }

            AHCIDrive *drive = new AHCIDrive(port, port->DevType == DeviceType::ATAPI ? 2048 : 512,
                                             resp.CommandSetActive.BigLba ? resp.Max48BitLBA : resp.UserAddressableSectors,
                                             nullptr, nullptr, resp.MaximumBlockTransfer ? resp.MaximumBlockTransfer : 1);
            drive->Model = getStringFromID(&resp, offsetof(ATAIdentifyResponse, ModelNumber), 40);
            drive->Serial = getStringFromID(&resp, offsetof(ATAIdentifyResponse, SerialNumber), 20);
            DEBUG("            %s %s %.2f MiB\n", drive->Model, drive->Serial, (double)(drive->SectorCount * drive->SectorSize) / (1 << 20));

            if(drvDir) drvDir->AddChild(drive);
        }
    }
    ObjectTree::Objects->UnLock();
}

void AHCIDrive::Cleanup()
{
    for(AHCIDrive::Controller *ctrl : controllers)
        delete ctrl;
    controllers.Clear();
}

int64_t AHCIDrive::ReadSectors(void *buffer, uint64_t start, int64_t count)
{
    int64_t blocks = align(count, MaxBlockTransfer) / MaxBlockTransfer;
    uint8_t *buf = (uint8_t *)buffer;
    int64_t st = 0;
    for(int64_t i = 0; i < blocks; ++i)
    {
        int64_t str = min(count, MaxBlockTransfer);
        int64_t sr = sectorTransfer(false, buf, start, str);
        if(sr < 0)
            return sr;
        st += sr;
        start += sr;
        count -= sr;
        buf += sr * SectorSize;
        if(!count || sr != str)
            break;
    }
    return st;
}

int64_t AHCIDrive::WriteSectors(const void *buffer, uint64_t start, int64_t count)
{
    int64_t blocks = align(count, MaxBlockTransfer) / MaxBlockTransfer;
    uint8_t *buf = (uint8_t *)buffer;
    int64_t st = 0;
    for(int64_t i = 0; i < blocks; ++i)
    {
        int64_t stw = min(count, MaxBlockTransfer);
        int64_t sw = sectorTransfer(true, buf, start, stw);
        if(sw < 0)
            return sw;
        st += sw;
        start += sw;
        count -= sw;
        buf += sw * SectorSize;
        if(!count || sw != stw)
            break;
    }
    return st;
}

void AHCIDrive::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", ID, Model);
}

AHCIDrive::~AHCIDrive()
{
}

bool AHCIDrive::Controller::interrupt(Ints::State *state, void *context)
{
    Controller *ctrl = (Controller *)context;
    uint32_t IS = ctrl->Registers->IS;
    if(!IS) return false;

    //DEBUG("[ahcidrive] interrupt\n");

    for(int i = 0; i < 32; ++i)
    {
        volatile Port *port = ctrl->Ports[i];
        if(!port || !port->Registers->IS) continue;
        //DEBUG("            on port %d\n", i);
        port->Registers->IS = ~0; // clear all interrupts flags on this port
        port->Interrupt->Signal(state);
    }

    ctrl->Registers->IS = ~0; // clear interrupt flags on the controller
    return true;
}

AHCIDrive::Controller::Controller(uintptr_t base, uint8_t irq) :
    interruptHandler { nullptr, interrupt, this },
    Registers((HBA_MEM *)base), IRQ(irq)
{
    uintptr_t pageAddr = base & PAGE_MASK;
    Paging::MapPage(~0, pageAddr, pageAddr, false, true);

    Memory::Zero(Ports, sizeof(Ports));
    IRQs::RegisterHandler(irq, &interruptHandler);
    IRQs::Enable(irq);

    Enable();
    EnableInterrupts();

    for(uint32_t i = 0, PI = Registers->PI; PI; ++i, PI >>= 1)
    {
        if(!(PI & 1)) continue;
        Ports[i] = new Port(this, i);
    }
}

void AHCIDrive::Controller::Enable()
{
    Registers->GHC |= HBA_GHC_AE;
}

void AHCIDrive::Controller::Disable()
{
    Registers->GHC = 0;
}

void AHCIDrive::Controller::EnableInterrupts()
{
    Registers->GHC |= HBA_GHC_IE;
}

void AHCIDrive::Controller::DisableInterrupts()
{
    Registers->GHC &= ~HBA_GHC_IE;
}

void AHCIDrive::Controller::Reset()
{
    Registers->GHC = 1;
    while(Registers->GHC & 1)
        Time::Sleep(1, false);
}

AHCIDrive::Controller::~Controller()
{
    DisableInterrupts();

    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(IRQ, &interruptHandler);
    IRQs::TryDisable(IRQ);
    cpuRestoreInterrupts(ints);

    for(Port *p : Ports)
        delete p;
}

AHCIDrive::Port::Port(AHCIDrive::Controller *controller, int portNumber) :
    Parent(controller), PortNumber(portNumber),
    Registers(controller->Registers->Ports + portNumber),
    DevType(GetDeviceType()),
    Interrupt(new Semaphore(0, "ahcidriveport"))
{
    Registers->IE = 0x0000000F; // enable needed interrupts
}

AHCIDrive::DeviceType AHCIDrive::Port::GetDeviceType()
{
    uint32_t SSTS = Registers->SSTS;
    uint32_t SIG = Registers->SIG;
    uint8_t IPM = SSTS >> 8 & 0x0F;
    uint8_t DET = SSTS & 0x0F;

    if(DET != PORT_SSTS_DET_PRESENT_COMM || IPM != PORT_SSTS_IPM_ACTIVE)
        return DeviceType::None;

    switch(SIG)
    {
    case PORT_SIG_SATA:
        return DeviceType::ATA;
    case PORT_SIG_SATAPI:
        return DeviceType::ATAPI;
    case PORT_SIG_SEMB:
        return DeviceType::SEMB;
    case PORT_SIG_PM:
        return DeviceType::PM;
    }

    return DeviceType::Unknown;
}

int AHCIDrive::Port::StartCommandEngine()
{
    int retry = 1000;
    while(Registers->CMD & PORT_CMD_CR && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    Registers->CMD |= PORT_CMD_FRE | PORT_CMD_ST;
    return 0;
}

int AHCIDrive::Port::StopCommandEngine()
{
    Registers->CMD &= ~PORT_CMD_ST;
    int retry = 1000;
    while(Registers->CMD & PORT_CMD_CR && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    Registers->CMD &= ~PORT_CMD_FRE;
    return 0;
}

int AHCIDrive::Port::Rebase()
{
    int res = StopCommandEngine();
    if(res) return res;

    // allocate needed structures
    CmdHeader = (HBA_CMD_HEADER *)Paging::AllocDMA(sizeof(HBA_CMD_HEADER));
    FIS = (HBA_FIS *)Paging::AllocDMA(sizeof(HBA_FIS));
    CmdTableSize = sizeof(HBA_CMD_TBL) + MaxPRDTs * sizeof(HBA_PRDT_ENTRY);
    CmdTable = (HBA_CMD_TBL *)Paging::AllocDMA(CmdTableSize);
    Prepare();

    // present it to the hardware
    Registers->CLB = Paging::GetDMAPhysicalAddress((void *)CmdHeader);
    Registers->CLBU = 0;
    Registers->FB = Paging::GetDMAPhysicalAddress((void *)FIS);
    Registers->FBU = 0;
    CmdHeader->CTBA = Paging::GetDMAPhysicalAddress((void *)CmdTable);
    CmdHeader->CTBAU = 0;

    return StartCommandEngine();
}

int AHCIDrive::Port::Prepare()
{
    Memory::Zero((void *)CmdHeader, sizeof(HBA_CMD_HEADER));
    Memory::Zero((void *)FIS, sizeof(HBA_FIS));
    Memory::Zero((void *)CmdTable, CmdTableSize);
    return 0;
}

int AHCIDrive::Port::IdentifyDrive(ATAIdentifyResponse *resp)
{
    int res = StopCommandEngine();
    if(res) return res;

    void *respBuf = Paging::AllocDMA(sizeof(ATAIdentifyResponse));
    Prepare();

    // prepare command header and PRDT
    CmdHeader->CFL = sizeof(FIS_REG_H2D) >> 2;
    CmdHeader->PRDTL = 1;
    CmdHeader->CTBA = Paging::GetDMAPhysicalAddress((void *)CmdTable);
    CmdHeader->CTBAU = 0;
    CmdTable->PRDT[0].DBA = Paging::GetDMAPhysicalAddress(respBuf);
    CmdTable->PRDT[0].DBAU = 0;
    CmdTable->PRDT[0].PRDBC = sizeof(ATAIdentifyResponse) - 1;

    // prepare FIS
    FIS_REG_H2D *fis = (FIS_REG_H2D *)CmdTable->CFIS;
    fis->FISType = FIS_TYPE_REG_H2D;
    fis->Command = DevType == DeviceType::ATAPI ? ATA_CMD_ID_ATAPI : ATA_CMD_ID_ATA;
    fis->Device = 0;
    fis->C = 1;

    res = StartCommandEngine();
    if(res)
    {
        Paging::FreeDMA(respBuf);
        return res;
    }

    int retry = 1000;
    while(Registers->TFD & (ATA_DEV_BUSY | ATA_DEV_DRQ) && --retry)
        Time::Sleep(1, false);
    if(!retry)
    {
        Paging::FreeDMA(respBuf);
        return -EBUSY;
    }

    // run the command
    Registers->CI = 1;
    if(Interrupt->Wait(5000, false, false) < 0)
    {
        Paging::FreeDMA(respBuf);
        return -EBUSY;
    }
    Memory::Move(resp, respBuf, sizeof(ATAIdentifyResponse));
    Paging::FreeDMA(respBuf);
    return 0;
}

AHCIDrive::Port::~Port()
{
    StopCommandEngine();
    if(CmdTable) Paging::FreeDMA((void *)CmdTable);
    if(FIS) Paging::FreeDMA((void *)FIS);
    if(CmdHeader) Paging::FreeDMA((void *)CmdHeader);
    if(Drive) delete Drive;
    if(Interrupt) delete Interrupt;
}
