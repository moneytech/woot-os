#pragma once

#include <types.hpp>

enum ATA_CMD
{
    ATA_CMD_DEV_RESET               = 0x08, /* ATAPI device reset */
    ATA_CMD_CHK_POWER               = 0xE5, /* check power mode */
    ATA_CMD_STANDBY                 = 0xE2, /* place in standby power mode */
    ATA_CMD_IDLE                    = 0xE3, /* place in idle power mode */
    ATA_CMD_EDD                     = 0x90,	/* execute device diagnostic */
    ATA_CMD_DOWNLOAD_MICRO          = 0x92,
    ATA_CMD_DOWNLOAD_MICRO_DMA      = 0x93,
    ATA_CMD_NOP                     = 0x00,
    ATA_CMD_FLUSH                   = 0xE7,
    ATA_CMD_FLUSH_EXT               = 0xEA,
    ATA_CMD_ID_ATA                  = 0xEC,
    ATA_CMD_ID_ATAPI                = 0xA1,
    ATA_CMD_SERVICE                 = 0xA2,
    ATA_CMD_READ                    = 0xC8,
    ATA_CMD_READ_EXT                = 0x25,
    ATA_CMD_READ_QUEUED             = 0x26,
    ATA_CMD_READ_STREAM_EXT         = 0x2B,
    ATA_CMD_READ_STREAM_DMA_EXT     = 0x2A,
    ATA_CMD_WRITE                   = 0xCA,
    ATA_CMD_WRITE_EXT               = 0x35,
    ATA_CMD_WRITE_QUEUED            = 0x36,
    ATA_CMD_WRITE_STREAM_EXT        = 0x3B,
    ATA_CMD_WRITE_STREAM_DMA_EXT    = 0x3A,
    ATA_CMD_WRITE_FUA_EXT           = 0x3D,
    ATA_CMD_WRITE_QUEUED_FUA_EXT    = 0x3E,
    ATA_CMD_FPDMA_READ              = 0x60,
    ATA_CMD_FPDMA_WRITE             = 0x61,
    ATA_CMD_FPDMA_SEND              = 0x64,
    ATA_CMD_FPDMA_RECV              = 0x65,
    ATA_CMD_PIO_READ                = 0x20,
    ATA_CMD_PIO_READ_EXT            = 0x24,
    ATA_CMD_PIO_WRITE               = 0x30,
    ATA_CMD_PIO_WRITE_EXT           = 0x34,
    ATA_CMD_READ_MULTI              = 0xC4,
    ATA_CMD_READ_MULTI_EXT          = 0x29,
    ATA_CMD_WRITE_MULTI             = 0xC5,
    ATA_CMD_WRITE_MULTI_EXT         = 0x39,
    ATA_CMD_WRITE_MULTI_FUA_EXT     = 0xCE,
    ATA_CMD_SET_FEATURES            = 0xEF,
    ATA_CMD_SET_MULTI               = 0xC6,
    ATA_CMD_PACKET                  = 0xA0,
    ATA_CMD_VERIFY                  = 0x40,
    ATA_CMD_VERIFY_EXT              = 0x42,
    ATA_CMD_WRITE_UNCORR_EXT        = 0x45,
    ATA_CMD_STANDBYNOW1             = 0xE0,
    ATA_CMD_IDLEIMMEDIATE           = 0xE1,
    ATA_CMD_SLEEP                   = 0xE6,
    ATA_CMD_INIT_DEV_PARAMS         = 0x91,
    ATA_CMD_READ_NATIVE_MAX         = 0xF8,
    ATA_CMD_READ_NATIVE_MAX_EXT     = 0x27,
    ATA_CMD_SET_MAX                 = 0xF9,
    ATA_CMD_SET_MAX_EXT             = 0x37,
    ATA_CMD_READ_LOG_EXT            = 0x2F,
    ATA_CMD_WRITE_LOG_EXT           = 0x3F,
    ATA_CMD_READ_LOG_DMA_EXT        = 0x47,
    ATA_CMD_WRITE_LOG_DMA_EXT       = 0x57,
    ATA_CMD_TRUSTED_NONDATA         = 0x5B,
    ATA_CMD_TRUSTED_RCV             = 0x5C,
    ATA_CMD_TRUSTED_RCV_DMA         = 0x5D,
    ATA_CMD_TRUSTED_SND             = 0x5E,
    ATA_CMD_TRUSTED_SND_DMA         = 0x5F,
    ATA_CMD_PMP_READ                = 0xE4,
    ATA_CMD_PMP_READ_DMA            = 0xE9,
    ATA_CMD_PMP_WRITE               = 0xE8,
    ATA_CMD_PMP_WRITE_DMA           = 0xEB,
    ATA_CMD_CONF_OVERLAY            = 0xB1,
    ATA_CMD_SEC_SET_PASS            = 0xF1,
    ATA_CMD_SEC_UNLOCK              = 0xF2,
    ATA_CMD_SEC_ERASE_PREP          = 0xF3,
    ATA_CMD_SEC_ERASE_UNIT          = 0xF4,
    ATA_CMD_SEC_FREEZE_LOCK         = 0xF5,
    ATA_CMD_SEC_DISABLE_PASS        = 0xF6,
    ATA_CMD_CONFIG_STREAM           = 0x51,
    ATA_CMD_SMART                   = 0xB0,
    ATA_CMD_MEDIA_LOCK              = 0xDE,
    ATA_CMD_MEDIA_UNLOCK            = 0xDF,
    ATA_CMD_DSM                     = 0x06,
    ATA_CMD_CHK_MED_CRD_TYP         = 0xD1,
    ATA_CMD_CFA_REQ_EXT_ERR         = 0x03,
    ATA_CMD_CFA_WRITE_NE            = 0x38,
    ATA_CMD_CFA_TRANS_SECT          = 0x87,
    ATA_CMD_CFA_ERASE               = 0xC0,
    ATA_CMD_CFA_WRITE_MULT_NE       = 0xCD,
    ATA_CMD_REQ_SENSE_DATA          = 0x0B,
    ATA_CMD_SANITIZE_DEVICE         = 0xB4
};

#pragma pack(push, 1)
struct ATAIdentifyResponse
{
    struct
    {
        uint16_t Reserved1 : 1;
        uint16_t Retired3 : 1;
        uint16_t ResponseIncomplete : 1;
        uint16_t Retired2 : 3;
        uint16_t FixedDevice : 1;
        uint16_t RemovableMedia : 1;
        uint16_t Retired1 : 7;
        uint16_t DeviceType : 1;
    } GeneralConfiguration;
    uint16_t NumCylinders;
    uint16_t ReservedWord2;
    uint16_t NumHeads;
    uint16_t Retired1[2];
    uint16_t NumSectorsPerTrack;
    uint16_t VendorUnique1[3];
    uint8_t  SerialNumber[20];
    uint16_t Retired2[2];
    uint16_t Obsolete1;
    uint8_t  FirmwareRevision[8];
    uint8_t  ModelNumber[40];
    uint8_t  MaximumBlockTransfer;
    uint8_t  VendorUnique2;
    uint16_t ReservedWord48;
    struct
    {
        uint8_t  ReservedByte49;
        uint8_t  DmaSupported : 1;
        uint8_t  LbaSupported : 1;
        uint8_t  IordyDisable : 1;
        uint8_t  IordySupported : 1;
        uint8_t  Reserved1 : 1;
        uint8_t  StandybyTimerSupport : 1;
        uint8_t  Reserved2 : 2;
        uint16_t ReservedWord50;
    } Capabilities;
    uint16_t ObsoleteWords51[2];
    uint16_t TranslationFieldsValid : 3;
    uint16_t Reserved3 : 13;
    uint16_t NumberOfCurrentCylinders;
    uint16_t NumberOfCurrentHeads;
    uint16_t CurrentSectorsPerTrack;
    uint32_t  CurrentSectorCapacity;
    uint8_t  CurrentMultiSectorSetting;
    uint8_t  MultiSectorSettingValid : 1;
    uint8_t  ReservedByte59 : 7;
    uint32_t  UserAddressableSectors;
    uint16_t ObsoleteWord62;
    uint16_t MultiWordDMASupport : 8;
    uint16_t MultiWordDMAActive : 8;
    uint16_t AdvancedPIOModes : 8;
    uint16_t ReservedByte64 : 8;
    uint16_t MinimumMWXferCycleTime;
    uint16_t RecommendedMWXferCycleTime;
    uint16_t MinimumPIOCycleTime;
    uint16_t MinimumPIOCycleTimeIORDY;
    uint16_t ReservedWords69[6];
    uint16_t QueueDepth : 5;
    uint16_t ReservedWord75 : 11;
    uint16_t ReservedWords76[4];
    uint16_t MajorRevision;
    uint16_t MinorRevision;
    struct
    {
        uint16_t SmartCommands : 1;
        uint16_t SecurityMode : 1;
        uint16_t RemovableMediaFeature : 1;
        uint16_t PowerManagement : 1;
        uint16_t Reserved1 : 1;
        uint16_t WriteCache : 1;
        uint16_t LookAhead : 1;
        uint16_t ReleaseInterrupt : 1;
        uint16_t ServiceInterrupt : 1;
        uint16_t DeviceReset : 1;
        uint16_t HostProtectedArea : 1;
        uint16_t Obsolete1 : 1;
        uint16_t WriteBuffer : 1;
        uint16_t ReadBuffer : 1;
        uint16_t Nop : 1;
        uint16_t Obsolete2 : 1;
        uint16_t DownloadMicrocode : 1;
        uint16_t DmaQueued : 1;
        uint16_t Cfa : 1;
        uint16_t AdvancedPm : 1;
        uint16_t Msn : 1;
        uint16_t PowerUpInStandby : 1;
        uint16_t ManualPowerUp : 1;
        uint16_t Reserved2 : 1;
        uint16_t SetMax : 1;
        uint16_t Acoustics : 1;
        uint16_t BigLba : 1;
        uint16_t DeviceConfigOverlay : 1;
        uint16_t FlushCache : 1;
        uint16_t FlushCacheExt : 1;
        uint16_t Resrved3 : 2;
        uint16_t SmartErrorLog : 1;
        uint16_t SmartSelfTest : 1;
        uint16_t MediaSerialNumber : 1;
        uint16_t MediaCardPassThrough : 1;
        uint16_t StreamingFeature : 1;
        uint16_t GpLogging : 1;
        uint16_t WriteFua : 1;
        uint16_t WriteQueuedFua : 1;
        uint16_t WWN64Bit : 1;
        uint16_t URGReadStream : 1;
        uint16_t URGWriteStream : 1;
        uint16_t ReservedForTechReport : 2;
        uint16_t IdleWithUnloadFeature : 1;
        uint16_t Reserved4 : 2;
    } CommandSetSupport;
    struct
    {
        uint16_t SmartCommands : 1;
        uint16_t SecurityMode : 1;
        uint16_t RemovableMediaFeature : 1;
        uint16_t PowerManagement : 1;
        uint16_t Reserved1 : 1;
        uint16_t WriteCache : 1;
        uint16_t LookAhead : 1;
        uint16_t ReleaseInterrupt : 1;
        uint16_t ServiceInterrupt : 1;
        uint16_t DeviceReset : 1;
        uint16_t HostProtectedArea : 1;
        uint16_t Obsolete1 : 1;
        uint16_t WriteBuffer : 1;
        uint16_t ReadBuffer : 1;
        uint16_t Nop : 1;
        uint16_t Obsolete2 : 1;
        uint16_t DownloadMicrocode : 1;
        uint16_t DmaQueued : 1;
        uint16_t Cfa : 1;
        uint16_t AdvancedPm : 1;
        uint16_t Msn : 1;
        uint16_t PowerUpInStandby : 1;
        uint16_t ManualPowerUp : 1;
        uint16_t Reserved2 : 1;
        uint16_t SetMax : 1;
        uint16_t Acoustics : 1;
        uint16_t BigLba : 1;
        uint16_t DeviceConfigOverlay : 1;
        uint16_t FlushCache : 1;
        uint16_t FlushCacheExt : 1;
        uint16_t Resrved3 : 2;
        uint16_t SmartErrorLog : 1;
        uint16_t SmartSelfTest : 1;
        uint16_t MediaSerialNumber : 1;
        uint16_t MediaCardPassThrough : 1;
        uint16_t StreamingFeature : 1;
        uint16_t GpLogging : 1;
        uint16_t WriteFua : 1;
        uint16_t WriteQueuedFua : 1;
        uint16_t WWN64Bit : 1;
        uint16_t URGReadStream : 1;
        uint16_t URGWriteStream : 1;
        uint16_t ReservedForTechReport : 2;
        uint16_t IdleWithUnloadFeature : 1;
        uint16_t Reserved4 : 2;
    } CommandSetActive;
    uint16_t UltraDMASupport : 8;
    uint16_t UltraDMAActive : 8;
    uint16_t ReservedWord89[4];
    uint16_t HardwareResetResult;
    uint16_t CurrentAcousticValue : 8;
    uint16_t RecommendedAcousticValue : 8;
    uint16_t ReservedWord95[5];
    uint64_t Max48BitLBA;
    uint16_t StreamingTransferTime;
    uint16_t ReservedWord105;
    struct
    {
        uint16_t LogicalSectorsPerPhysicalSector : 4;
        uint16_t Reserved0 : 8;
        uint16_t LogicalSectorLongerThan256Words : 1;
        uint16_t MultipleLogicalSectorsPerPhysicalSector : 1;
        uint16_t Reserved1 : 2;
    } PhysicalLogicalSectorSize;
    uint16_t InterSeekDelay;
    uint16_t WorldWideName[4];
    uint16_t ReservedForWorldWideName128[4];
    uint16_t ReservedForTlcTechnicalReport;
    uint32_t WordsPerLogicalSector;
    struct
    {
        uint16_t ReservedForDrqTechnicalReport : 1;
        uint16_t WriteReadVerifySupported : 1;
        uint16_t Reserved01 : 11;
        uint16_t Reserved1 : 2;
    } CommandSetSupportExt;
    struct
    {
        uint16_t ReservedForDrqTechnicalReport : 1;
        uint16_t WriteReadVerifyEnabled : 1;
        uint16_t Reserved01 : 11;
        uint16_t Reserved1 : 2;
    } CommandSetActiveExt;
    uint16_t ReservedForExpandedSupportandActive[6];
    uint16_t MsnSupport : 2;
    uint16_t ReservedWord1274 : 14;
    struct
    {
        uint16_t SecuritySupported : 1;
        uint16_t SecurityEnabled : 1;
        uint16_t SecurityLocked : 1;
        uint16_t SecurityFrozen : 1;
        uint16_t SecurityCountExpired : 1;
        uint16_t EnhancedSecurityEraseSupported : 1;
        uint16_t Reserved0 : 2;
        uint16_t SecurityLevel : 1;
        uint16_t Reserved1 : 7;
    } SecurityStatus;
    uint16_t ReservedWord129[31];
    struct
    {
        uint16_t MaximumCurrentInMA2 : 12;
        uint16_t CfaPowerMode1Disabled : 1;
        uint16_t CfaPowerMode1Required : 1;
        uint16_t Reserved0 : 1;
        uint16_t Word160Supported : 1;
    } CfaPowerModel;
    uint16_t ReservedForCfaWord161[8];
    struct
    {
        uint16_t SupportsTrim : 1;
        uint16_t Reserved0 : 15;
    } DataSetManagementFeature;
    uint16_t ReservedForCfaWord170[6];
    uint16_t CurrentMediaSerialNumber[30];
    uint16_t ReservedWord206;
    uint16_t ReservedWord207[2];
    struct
    {
        uint16_t AlignmentOfLogicalWithinPhysical : 14;
        uint16_t Word209Supported : 1;
        uint16_t Reserved0 : 1;
    } BlockAlignment;
    uint16_t WriteReadVerifySectorCountMode3Only[2];
    uint16_t WriteReadVerifySectorCountMode2Only[2];
    struct
    {
        uint16_t NVCachePowerModeEnabled : 1;
        uint16_t Reserved0 : 3;
        uint16_t NVCacheFeatureSetEnabled : 1;
        uint16_t Reserved1 : 3;
        uint16_t NVCachePowerModeVersion : 4;
        uint16_t NVCacheFeatureSetVersion : 4;
    } NVCacheCapabilities;
    uint16_t NVCacheSizeLSW;
    uint16_t NVCacheSizeMSW;
    uint16_t NominalMediaRotationRate;
    uint16_t ReservedWord218;
    struct
    {
        uint8_t NVCacheEstimatedTimeToSpinUpInSeconds;
        uint8_t Reserved;
    } NVCacheOptions;
    uint16_t ReservedWord220[35];
    uint16_t Signature : 8;
    uint16_t CheckSum : 8;
};

#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ 0x08
#define ATA_DEV_ERR 0x01

#pragma pack(pop)
