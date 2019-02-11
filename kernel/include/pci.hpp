#pragma once

#include <objecttree.hpp>
#include <types.hpp>

#define PCI_BUS_DIR "/dev/bus/pci"

#define PCI_MAKE_ADDR(bus, dev, func, offs) (0x80000000 | (bus) << 16 | (dev) << 11 | (func) << 8 | offs)
#define PCI_ADDR_BUS(addr)  (((addr) >> 16) & 0xFF)
#define PCI_ADDR_DEV(addr)  (((addr) >> 11) & 0x1F)
#define PCI_ADDR_FUNC(addr) (((addr) >> 8) & 0x07)
#define PCI_ADDR_REG(addr)  (((addr) >> 2) & 0x3F)
#define PCI_ADDR_OFFS(addr) ((addr) & 0xFF)

class PCI
{
public:
#pragma pack(push, 1)
    struct DefaultHeader
    {
        uint32_t BAR[6];
        uint32_t CardBusCISPointer;
        uint16_t SubSystemVendorID;
        uint16_t SubSystemID;
        uint32_t ExpansionROMBaseAddress;
        uint8_t CapabilitiesPointer;
        uint8_t Reserved[7];
        uint8_t InterruptLine;
        uint8_t InterruptPIN;
        uint8_t MinGrant;
        uint8_t MaxLatency;
    };

    struct PCI2PCIHeader
    {
        uint32_t BAR[2];
        uint8_t PrimaryBusNumber;
        uint8_t SecondaryBusNumber;
        uint8_t SubordinateBusNumber;
        uint8_t SecondaryLatencyTimer;
        uint8_t IOBase;
        uint8_t IOLimit;
        uint16_t SecondaryStatus;
        uint16_t MemoryBase;
        uint16_t MemoryLimit;
        uint16_t PrefetchableMemoryBase;
        uint16_t PrefetchableMemoryLimit;
        uint16_t PrefetchableMemoryBaseHI;
        uint16_t PrefetchableMemoryLimitHI;
        uint16_t IOBaseHI;
        uint16_t IOLimitHI;
        uint8_t CapabilitiesPointer;
        uint8_t Reserved[3];
        uint32_t ExpansionROMBaseAddress;
        uint8_t InterruptLine;
        uint8_t InterruptPIN;
        uint16_t BridgeControl;
    };

    struct PCI2CardBusHeader
    {
        uint32_t MemoryBaseAddress0;
        uint32_t MemoryLimit0;
        uint32_t MemoryBaseAddress1;
        uint32_t MemoryLimit1;
        uint32_t IOBaseAddress0;
        uint32_t IOLimit0;
        uint32_t IOBaseAddress1;
        uint32_t IOLimit1;
        uint8_t InterruptLine;
        uint8_t InterruptPIN;
        uint16_t BridgeControl;
        uint16_t SubSystemDeviceID;
        uint16_t SubSystemVendorID;
        uint32_t LegacyBaseAddress;
    };

    struct Config
    {
        union
        {
            struct
            {
                uint16_t VendorID;
                uint16_t DeviceID;
                uint16_t Command;
                uint16_t Status;
                uint8_t RevisionID;
                uint8_t ProgIF;
                uint8_t SubClass;
                uint8_t Class;
                uint8_t CacheLineSize;
                uint8_t LatencyTimer;
                uint8_t HeaderType;
                uint8_t BIST;
                union
                {
                    DefaultHeader Default;
                    PCI2PCIHeader PCI2PCI;
                    PCI2CardBusHeader PCI2CardBus;
                } Header;
            };
            uint8_t Data[256];
        };
    };
#pragma pack(pop)
    typedef uint32_t Address;

    class Device : public ObjectTree::Item
    {
    public:
        PCI::Address Address;
        uint16_t VendorID;
        uint16_t DeviceID;
        uint8_t Class;
        uint8_t SubClass;
        uint8_t ProgIF;

        Device(PCI::Address address, uint16_t vid, uint16_t did, uint8_t cls, uint8_t subCls, uint8_t progif);
        virtual bool KeyCheck(const char *name);
        virtual void GetDisplayName(char *buf, size_t bufSize);
    };
private:
    static void Check(ObjectTree::Item *dir);
    static void CheckBus(ObjectTree::Item *dir, uint8_t bus);
    static void CheckDevice(ObjectTree::Item *dir, uint8_t bus, uint8_t device);
    static void CheckFunction(ObjectTree::Item *dir, uint8_t bus, uint8_t device, uint8_t func);
public:
    static void Initialize();
    static uint8_t ReadConfigByte(Address address);
    static void WriteConfigByte(Address address, uint8_t value);
    static uint16_t ReadConfigWord(Address address);
    static void WriteConfigWord(Address address, uint16_t value);
    static uint32_t ReadConfigDWord(Address address);
    static void WriteConfigDWord(Address address, uint32_t value);
    static void ReadConfigData(Config *config, uint32_t Address);
    static void WriteConfigData(Address address, Config *config);
    static void Cleanup();
};
