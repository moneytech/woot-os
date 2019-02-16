#include <cpu.hpp>
#include <debug.hpp>
#include <errno.h>
#include <../ide/idedrive.hpp>
#include <ints.hpp>
#include <irqs.hpp>
#include <memory.hpp>
#include <mutex.hpp>
#include <paging.hpp>
#include <pci.hpp>
#include <semaphore.hpp>
#include <stringbuilder.hpp>
#include <time.hpp>

const uint16_t IDEDrive::defAddrs[4] = { 0x1F0, 0x3F6, 0x170, 0x376 };

bool IDEDrive::interrupt(Ints::State *state, void *context)
{
    class Controller *ctrl = (class Controller *)context;
    uint8_t bmStatus = _inb(ctrl->BM + 2);
    if(!(bmStatus & 0x04))
        return false;
    uint8_t status = _inb(ctrl->Base + 7);
    int drv = (_inb(ctrl->Base + 6) & 0x10) ? 1 : 0;

    if(status & 0x01)
        _outb(ctrl->BM, _inb(ctrl->BM) & ~0x01); // stop bm
    _outb(ctrl->BM + 2, 0x04);

    IDEDrive *drive = ctrl->Drives[drv];
    if(drive) drive->TransferDone->Signal(state);
    return true;
}

IDEDrive::IDEDrive(ATAIdentifyResponse *id, bool atapi, class Controller *ctrl, bool slave) :
    Drive(atapi ? 2048 : 512, // FIXME: Shouldn't be hardcoded
          atapi ? 0x7FFFFFFF : (id->CommandSetActive.BigLba ? id->Max48BitLBA : id->UserAddressableSectors),
          nullptr, nullptr),
    Controller(ctrl), Slave(slave), ATAPI(atapi),
    PRDTsAllocated(8), // some arbitrary value
    PRDTs((PRDTEntry *)Paging::AllocDMA(sizeof(PRDTEntry) * PRDTsAllocated, 64 << 10)),
    TransferDone(new Semaphore(0)),
    MaxBlockTransfer(id->MaximumBlockTransfer ? id->MaximumBlockTransfer : 1)
{
    Memory::Zero(PRDTs, sizeof(PRDTEntry) * PRDTsAllocated);
    Memory::Move(&IDResp, id, sizeof(ATAIdentifyResponse));
    Model = getStringFromID(id, offsetof(ATAIdentifyResponse, ModelNumber), 40);
    Serial = getStringFromID(id, offsetof(ATAIdentifyResponse, SerialNumber), 20);
    Controller->Lock->Acquire(1000); // proceed even if locking fails
    Controller->Drives[Slave ? 1 : 0] = this;
    Controller->Lock->Release();
}

char *IDEDrive::getStringFromID(ATAIdentifyResponse *id, uint offset, uint length)
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

int IDEDrive::sectorTransfer(bool write, void *buffer, uint64_t start, int64_t count)
{
    uintptr_t addressSpace = Paging::GetAddressSpace();
    if(!Controller->Lock->Acquire(5000))
        return -EIO;

    _outb(Controller->Base + 6, Slave ? 0x10 : 0x00);
    if(!Controller->waitFornBSY_nolock(200, nullptr))
    {
        Controller->Lock->Release();
        return -EIO;
    }

    size_t byteCount = count * SectorSize;
    size_t prdtCount = (byteCount + 65535) >> 16;
    if(prdtCount > PRDTsAllocated)
    {
        DEBUG("[idedrive] prdtCount > prdtsAllocated\n");
        Controller->Lock->Release();
        return -ENOMEM;
    }

    size_t bytesLeft = byteCount;
    for(int i = 0; i < prdtCount; ++i, bytesLeft -= (64 << 10))
    {
        bool last = bytesLeft <= (64 << 10);
        PRDTs[i].BufferAddress = Paging::GetPhysicalAddress(addressSpace, (64 << 10) * i + (uintptr_t)buffer);
        PRDTs[i].ByteCount = last ? bytesLeft : 0;
        PRDTs[i].Reserved = 0;
        PRDTs[i].Last = last ? 1 : 0;
    }

    uint32_t prdtAddr = Paging::GetPhysicalAddress(addressSpace, (uintptr_t)PRDTs);
    _outb(Controller->BM, 0); // stop bm
    _outb(Controller->BM, write ? 0x00 : 0x08); // reading/writing
    _outb(Controller->BM + 2, 0x06); // clear error flags
    _outl(Controller->BM + 4, prdtAddr); // set prdt address

    if(ATAPI)
    {
        _outb(Controller->Base + 6, Slave ? 0x10 : 0x00);
        Controller->Wait400ns();
        _outb(Controller->Base + 1, 1 | (write ? 0 : 4)); // dma
        _outb(Controller->Base + 2, 0);
        _outb(Controller->Base + 3, 0);
        _outb(Controller->Base + 4, 0);
        _outb(Controller->Base + 5, 0);
    }
    else
    {
        if(IDResp.CommandSetActive.BigLba)
        {
            _outb(Controller->Base + 6, Slave ? 0x50 : 0x40);
            Controller->Wait400ns();
            _outb(Controller->Base + 1, 0);
            _outb(Controller->Base + 2, count >> 8);
            _outb(Controller->Base + 3, start >> 24);
            _outb(Controller->Base + 4, start >> 32);
            _outb(Controller->Base + 5, start >> 40);
            _outb(Controller->Base + 2, count);
            _outb(Controller->Base + 3, start >> 0);
            _outb(Controller->Base + 4, start >> 8);
            _outb(Controller->Base + 5, start >> 16);
        }
        else
        {
            _outb(Controller->Base + 6, (Slave ? 0xF0 : 0xE0) | ((start >> 24) & 0x0F));
            Controller->Wait400ns();
            _outb(Controller->Base + 1, 0);
            _outb(Controller->Base + 2, count);
            _outb(Controller->Base + 3, start >> 0);
            _outb(Controller->Base + 4, start >> 8);
            _outb(Controller->Base + 5, start >> 16);
        }
    }

    if(!Controller->waitForDRDY_nolock(5000, nullptr))
    {
        Controller->Lock->Release();
        return -EIO;
    }

    if(ATAPI)
    {
        _outb(Controller->Base + 7, ATA_CMD_PACKET);
        uint8_t status = 0;
        if(!Controller->waitFornBSY_nolock(200, &status))
        {
            Controller->Lock->Release();
            return -EIO;
        }

        uint8_t op = write ? 0xAA : 0xA8;
        uint8_t cmd[12] = { op, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        cmd[9] = count;
        cmd[2] = start >> 24;
        cmd[3] = start >> 16;
        cmd[4] = start >> 8;
        cmd[5] = start;
        if(!Controller->waitForDRQorERR_nolock(200, &status) || (status & 1))
        {
            Controller->Lock->Release();
            return -EIO;
        }

        _outsw(cmd, Controller->Base, 6);
    }
    else _outb(Controller->Base + 7, IDResp.CommandSetActive.BigLba ?
                  (write ? ATA_CMD_WRITE_EXT : ATA_CMD_READ_EXT) :
                  (write ? ATA_CMD_WRITE : ATA_CMD_READ));

    _outb(Controller->BM, write ? 0x01 : 0x09); // start reading/writing

    bool ok = TransferDone->Wait(5000, false, false);
    if(!ok)
    {
        //printf("IDE drive read timeout! status: %.2x\n", inb(ctrl->Base + 7));
        Controller->Lock->Release();
        return -EIO;
    }

    uint8_t status = _inb(Controller->Base + 7);
    if(!ok || (status & 1))
    {
        _inb(Controller->BM + 2); // read status
        _outb(Controller->BM, 0); // stop bm
        _outb(Controller->BM + 2, 0x06); // clear error flags
        Controller->Lock->Release();
        return -EIO;
    }

    Controller->Lock->Release();
    return count;
}

void IDEDrive::Initialize()
{
    if(!ObjectTree::Objects->Lock()) return;
    ObjectTree::Item *pciDir = ObjectTree::Objects->Get(PCI_BUS_DIR);
    if(!pciDir)
    {
        ObjectTree::Objects->UnLock();
        return;
    }
    ObjectTree::Item *drvDir = ObjectTree::Objects->MakeDir(DRV_DIR);

    for(ObjectTree::Item *item : pciDir->GetChildren())
    {
        PCI::Device *pciDev = (PCI::Device *)item;
        if(pciDev->Class != 0x01 || pciDev->SubClass != 0x01)
            continue;
        DEBUG("[idedrive] Found PCI IDE controller at PCI:%d.%d.%d\n",
              PCI_ADDR_BUS(pciDev->Address),
              PCI_ADDR_DEV(pciDev->Address),
              PCI_ADDR_FUNC(pciDev->Address));

        // enable PCI BusMaster
        PCI::WriteConfigWord(pciDev->Address | 0x04, PCI::ReadConfigWord(pciDev->Address | 0x04) | 0x0004);

        PCI::Config cfg;
        PCI::ReadConfigData(&cfg, pciDev->Address);

        for(int i = 0; i < 2; ++i)
        {
            uint32_t baseBAR = cfg.Header.Default.BAR[i * 2];
            uint32_t ctrlBAR = cfg.Header.Default.BAR[i * 2 + 1];
            uint16_t base = baseBAR ? baseBAR & 0xFFFFFFFC : defAddrs[i * 2];
            uint16_t ctrl = ctrlBAR ? ctrlBAR & 0xFFFFFFFC : defAddrs[i * 2 + 1];
            uint16_t bm = 8 * i + cfg.Header.Default.BAR[4] & 0xFFFFFFFC;
            uint8_t irq = cfg.Header.Default.InterruptLine;
            if(!irq || irq == 0xFF)
                irq = i ? 15 : 14;

            /*printf("  %s: base: %#.4x ctrl: %#.4x bm: %#.4x irq: %d\n",
                   i ? "sec" : "pri", base, ctrl, bm, irq);*/

            class Controller *controller = new class Controller(base, ctrl, bm, irq);
            IRQs::RegisterHandler(controller->IRQ, &controller->InterruptHandler);
            IRQs::Enable(controller->IRQ);
            controller->SoftReset();

            // detect ATAPI drives
            bool atapi[2] = { false, false };
            _outb(controller->Base + 6, 0x00);
            atapi[0] = _inb(controller->Base + 4) == 0x14 && _inb(controller->Base + 5) == 0xEB;
            _outb(controller->Base + 6, 0x10);
            atapi[1] = _inb(controller->Base + 4) == 0x14 && _inb(controller->Base + 5) == 0xEB;

            // enable IRQ after soft reset
            controller->EnableIRQ();

            ATAIdentifyResponse *id = new ATAIdentifyResponse;
            for(int j = 0; j < 2; ++j)
            {
                if(!controller->Identify(id, j, atapi[j]))
                    continue;
                IDEDrive *drive = new IDEDrive(id, atapi[j], controller, j);
                if(drvDir) drvDir->AddChild(drive);
                int id = drive->ID;
                if(atapi[j]) DEBUG("[idedrive] Found ATAPI drive %s (sn: %s; id: %d)\n", drive->Model, drive->Serial, id);
                else DEBUG("[idedrive] Found %.2fMiB ATA drive %s (sn: %s; id: %d)\n", (double)(drive->SectorCount * drive->SectorSize) / (double)(1 << 20), drive->Model, drive->Serial, id);
            }
            delete id;
        }
    }
    ObjectTree::Objects->UnLock();
}

void IDEDrive::Cleanup()
{
}

int64_t IDEDrive::ReadSectors(void *buffer, uint64_t start, int64_t count)
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

int64_t IDEDrive::WriteSectors(const void *buffer, uint64_t start, int64_t count)
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

void IDEDrive::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", ID, Model);
}

IDEDrive::~IDEDrive()
{
    if(PRDTs) Paging::FreeDMA(PRDTs);
    if(TransferDone) delete TransferDone;
    if(Controller)
    {
        Controller->Lock->Acquire(5000); // proceed even if locking fails
        // unpin this drive object from a controller
        Controller->Drives[Slave ? 1 : 0] = nullptr;

        // delete Controller object if not needed anymore
        if(!Controller->Drives[0] && !Controller->Drives[1])
            delete Controller;
        else Controller->Lock->Release();
    }
}

bool IDEDrive::Controller::waitFornBSYorERR_nolock(int timeout, uint8_t *status)
{
    uint8_t s;
    if(timeout <= 0)
    {
        while(((s = _inb(Base + 7)) & 0x80) && !(s & 0x01));
        if(status) *status = s;
        return true;
    }

    while(--timeout && ((s = _inb(Base + 7)) & 0x80) && !(s & 0x01))
        Time::Sleep(1, false);
    if(status) *status = s;
    return timeout != 0;
}

bool IDEDrive::Controller::waitForDRQorERR_nolock(int timeout, uint8_t *status)
{
    uint8_t s;
    if(timeout <= 0)
    {
        while(!((s = _inb(Base + 7)) & 0x08) && !(s & 0x01));
        if(status) *status = s;
        return true;
    }

    while(--timeout && !((s = _inb(Base + 7)) & 0x08) && !(s & 0x01))
        Time::Sleep(1, false);
    if(status) *status = s;
    return timeout != 0;
}

bool IDEDrive::Controller::waitFornBSY_nolock(int timeout, uint8_t *status)
{
    uint8_t s;
    if(timeout <= 0)
    {
        while((s = _inb(Base + 7)) & 0x80);
        if(status) *status = s;
        return true;
    }

    while(--timeout && ((s = _inb(Base + 7)) & 0x80))
        Time::Sleep(1, false);
    if(status) *status = s;
    return timeout != 0;
}

bool IDEDrive::Controller::waitForDRDY_nolock(int timeout, uint8_t *status)
{
    uint8_t s;
    if(timeout <= 0)
    {
        while(!((s = _inb(Base + 7)) & 0x40));
        if(status) *status = s;
        return true;
    }

    while(--timeout && !((s = _inb(Base + 7)) & 0x40))
        Time::Sleep(1, false);
    if(status) *status = s;
    return timeout != 0;
}

bool IDEDrive::Controller::identify_nolock(ATAIdentifyResponse *id, bool slave, bool atapi)
{
    uint8_t status;
    DisableIRQ();
    _outb(Base + 6, slave ? 0x10 : 0x00);
    Wait400ns();
    _outb(Base + 7, atapi ? ATA_CMD_ID_ATAPI : ATA_CMD_ID_ATA);
    bool ok = waitFornBSYorERR_nolock(100, &status);
    if(!ok || status & 0x01 || status & 0x20)
    {
        EnableIRQ();
        return false;
    }
    ok = waitForDRQorERR_nolock(100, &status);
    if(!ok || status & 0x01 || status & 0x20)
    {
        EnableIRQ();
        return false;
    }
    _insw(id, Base, 256);
    EnableIRQ();
    return true;
}

IDEDrive::Controller::Controller(uint16_t base, uint16_t control, uint16_t bm, uint8_t irq) :
    Lock(new Mutex(false, "ideController")),
    Base(base), Control(control), BM(bm), IRQ(irq),
    InterruptHandler { nullptr, interrupt, this }
{
}

void IDEDrive::Controller::EnableIRQ()
{
    _outb(Control, 0x00);
}

void IDEDrive::Controller::DisableIRQ()
{
    _outb(Control, 0x02);
}

void IDEDrive::Controller::Wait400ns()
{
    for(int i = 0; i < 5; ++i)
        _inb(Control);
}

void IDEDrive::Controller::SoftReset()
{
    Lock->Acquire(5000); // do reset even if locking fails
    _outb(Control, 0x06);
    Wait400ns();
    _outb(Control, 0x02);
    Lock->Release();
}

bool IDEDrive::Controller::WaitFornBSYorERR(int timeout, uint8_t *status)
{
    if(!Lock->Acquire(1000))
        return false;
    bool res = waitFornBSYorERR_nolock(timeout, status);
    Lock->Release();
    return res;
}

bool IDEDrive::Controller::WaitForDRQorERR(int timeout, uint8_t *status)
{
    if(!Lock->Acquire(1000))
        return false;
    bool res = waitForDRQorERR_nolock(timeout, status);
    Lock->Release();
    return res;
}

bool IDEDrive::Controller::WaitFornBSY(int timeout, uint8_t *status)
{
    if(!Lock->Acquire(1000))
        return false;
    bool res = waitFornBSY_nolock(timeout, status);
    Lock->Release();
    return res;
}

bool IDEDrive::Controller::WaitForDRDY(int timeout, uint8_t *status)
{
    if(!Lock->Acquire(1000))
        return false;
    bool res = waitForDRDY_nolock(timeout, status);
    Lock->Release();
    return res;
}

bool IDEDrive::Controller::Identify(ATAIdentifyResponse *id, bool slave, bool atapi)
{
    bool ok = Lock->Acquire(1000);
    if(!ok) return false;
    ok = identify_nolock(id, slave, atapi);
    Lock->Release();
    return ok;
}
