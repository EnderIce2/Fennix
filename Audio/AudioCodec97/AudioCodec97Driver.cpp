#include "ac97.hpp"
#include "../../../Kernel/DAPI.hpp"
#include "../../../Kernel/Fex.hpp"

extern "C" int DriverEntry(void *Data);
int CallbackHandler(KernelCallback *Data);
int InterruptCallback(CPURegisters *Registers);

HEAD(FexFormatType_Driver, FexOSType_Fennix, DriverEntry);

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

__attribute__((section(".extended"))) FexExtended ExtendedHeader = {
    .Driver = {
        .Name = "Audio Codec '97 Driver",
        .Type = FexDriverType_Audio,
        .TypeFlags = FexDriverInputTypes_None,
        .OverrideOnConflict = false,
        .Callback = CallbackHandler,
        .InterruptCallback = InterruptCallback,
        .Bind = {
            .Type = BIND_PCI,
            .PCI = {
                .VendorID = {0x8086},
                .DeviceID = {0x2415},
                .Class = 0x4,
                .SubClass = 0x3,
                .ProgIF = 0x0,
            }}}};

KernelAPI *KAPI;

#define print(msg) KAPI->Util.DebugPrint((char *)(msg), KAPI->Info.DriverUID)

/* --------------------------------------------------------------------------------------------------------- */

/* https://wiki.osdev.org/AC97 */

PCIDeviceHeader *PCIBaseAddress;
BARData BAR;
BufferDescriptorList *DescriptorList = nullptr;

AudioEncodingValues Encoding = AE_PCMs16le;
char Channels = 2;
char Volume = AV_Maximum;
bool Mute = false;
int SampleRate = 48000;
char SampleSize = 2;

uint16_t MixerVolume(uint8_t Left, uint8_t Right, bool Mute)
{
    return ((uint16_t)((Right & 0x3F) |
                       ((Left & 0x3F) << 0x8) |
                       (Mute & 1 << 0xF)));
}

int DriverEntry(void *Data)
{
    if (!Data)
        return INVALID_KERNEL_API;
    KAPI = (KernelAPI *)Data;
    if (KAPI->Version.Major < 0 || KAPI->Version.Minor < 0 || KAPI->Version.Patch < 0)
        return KERNEL_API_VERSION_NOT_SUPPORTED;

    return OK;
}

int CallbackHandler(KernelCallback *Data)
{
    switch (Data->Reason)
    {
    case AcknowledgeReason:
    {
        print("Kernel acknowledged the driver.");
        break;
    }
    case ConfigurationReason:
    {
        print("Driver received configuration data.");
        PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
        PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;

        /* Native Audio Mixer Base Address */
        uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;

        /* Native Audio Bus Master Base Address */
        uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

        BAR.Type = PCIBAR0 & 1;
        BAR.MixerAddress = PCIBAR0 & (~3);
        BAR.BusMasterAddress = PCIBAR1 & (~15);

        if (BAR.Type != 1)
        {
            print("BAR0 is not I/O.");
            return INVALID_PCI_BAR;
        }
        uint16_t OutputPCMTransferControl = BAR.BusMasterAddress + PCMOUT_TransferControl;

        /* DescriptorList address MUST be physical. */
        DescriptorList = (BufferDescriptorList *)KAPI->Memory.RequestPage((sizeof(BufferDescriptorList) * DescriptorListLength) / KAPI->Memory.PageSize + 1);
        KAPI->Util.memset(DescriptorList, 0, sizeof(BufferDescriptorList) * DescriptorListLength);

        uint16_t DLSampleCount = KAPI->Memory.PageSize / SampleSize;
        char DLLogBuffer[128];
        for (int i = 0; i < DescriptorListLength; i++)
        {
            int DescriptorPages = sizeof(uint16_t *) / KAPI->Memory.PageSize + 1;
            DescriptorList[i].Address = (uint32_t)(uint64_t)KAPI->Memory.RequestPage(DescriptorPages);
            DescriptorList[i].SampleCount = DLSampleCount;
            DescriptorList[i].Flags = 0;
            KAPI->Util.sprintf(DLLogBuffer, "DescriptorList[%d] = { Address: 0x%x (%d %s), SampleCount: %d, Flags: 0x%x }",
                               i,
                               DescriptorList[i].Address, DescriptorPages, DescriptorPages == 1 ? "page" : "pages",
                               DescriptorList[i].SampleCount,
                               DescriptorList[i].Flags);
            print(DLLogBuffer);
        }

        outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
        outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));

        outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) | TC_TransferReset);
        while (inb(OutputPCMTransferControl) & TC_TransferReset)
            ;

        uint32_t GlobalControl = inl(BAR.BusMasterAddress + NABM_GlobalControl);
        GlobalControl = (GlobalControl & ~((0x3U) << 22)); /* PCM 16-bit mode */
        GlobalControl = (GlobalControl & ~((0x3U) << 20)); /* 2 channels */
        GlobalControl |= GC_GlobalInterruptEnable;
        GlobalControl &= ~GC_ShutDown;

        outl(BAR.BusMasterAddress + PCMOUT_BufferDescriptorList, (uint32_t)(uint64_t)DescriptorList);
        outl(BAR.BusMasterAddress + NABM_GlobalControl, GlobalControl);

        uint8_t TransferControl = inb(OutputPCMTransferControl);
        TransferControl |= TC_LastBufferEntryInterruptEnable | TC_IOCInterruptEnable | TC_FifoERRORInterruptEnable;
        outb(OutputPCMTransferControl, TransferControl);

        // Stop DMA
        outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) & ~TC_DMAControllerControl);
        print("AC'97 configured.");
        break;
    }
    case AdjustReason:
    {
        if (Data->AudioCallback.Adjust._Volume)
        {
            Volume = 0x3F - (0x3F * Data->AudioCallback.Adjust.Volume / 100);
            outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
            // outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));
        }
        else if (Data->AudioCallback.Adjust._Encoding)
        {
            print("Encoding changing not supported yet.");
        }
        else if (Data->AudioCallback.Adjust._SampleRate)
        {
            switch (Data->AudioCallback.Adjust.SampleRate)
            {
            case 0:
            {
                SampleRate = 8000;
                break;
            }
            case 1:
            {
                SampleRate = 11025;
                break;
            }
            case 2:
            {
                SampleRate = 16000;
                break;
            }
            case 3:
            {
                SampleRate = 22050;
                break;
            }
            case 4:
            {
                SampleRate = 32000;
                break;
            }
            case 5:
            {
                SampleRate = 44100;
                break;
            }
            case 6:
            {
                SampleRate = 48000;
                break;
            }
            case 7:
            {
                SampleRate = 88200;
                break;
            }
            case 8:
            {
                SampleRate = 96000;
                break;
            }
            default:
            {
                SampleRate = 16000;
                print("Invalid sample rate. Defaulting to 16000.");
                break;
            }
            }
        }
        else if (Data->AudioCallback.Adjust._Channels)
        {
            switch (Data->AudioCallback.Adjust.Channels)
            {
            case 0:
            {
                Channels = 1; // Mono
                break;
            }
            case 1:
            {
                Channels = 2; // Stereo
                break;
            }
            default:
            {
                Channels = 2;
                print("Invalid channel count. Defaulting to 2.");
                break;
            }
            }
        }
        break;
    }
    case FetchReason:
    {
        Data->AudioCallback.Fetch.Volume = (inw(BAR.MixerAddress + NAM_MasterVolume) & 0x3F) * 100 / 0x3F;
        Data->AudioCallback.Fetch.Encoding = Encoding; /* FIXME */
        Data->AudioCallback.Fetch.SampleRate = SampleRate;
        Data->AudioCallback.Fetch.Channels = Channels;
        break;
    }
    case SendReason:
    {
        unsigned char *Buffer = (unsigned char *)Data->AudioCallback.Send.Data;
        unsigned int Length = Data->AudioCallback.Send.Length;

        if (Buffer == nullptr)
        {
            print("Invalid buffer.");
            return INVALID_DATA;
        }

        if ((Length == 0) || (Length % (SampleSize * Channels)))
        {
            print("Invalid buffer length.");
            return INVALID_DATA;
        }

        int TotalBDLToFill = (Length + KAPI->Memory.PageSize - 1) >> 12;

        while (Length > 0)
        {
            bool ActiveDMA = !(inw(BAR.BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);

            if (ActiveDMA)
            {
                int RemainingBDL = 0;

                do
                {
                    int CurrentBDL = inb(BAR.BusMasterAddress + PCMOUT_BufferDescriptorEntry);
                    int LastBDL = inb(BAR.BusMasterAddress + PCMOUT_DescriptorEntries);

                    RemainingBDL = LastBDL - CurrentBDL;
                    if (RemainingBDL < 0)
                        RemainingBDL += DescriptorListLength;

                    RemainingBDL += 1;

                    if (RemainingBDL >= DescriptorListLength - 1)
                    {
                        long SampleCount = DescriptorList[(CurrentBDL + 1) % DescriptorListLength].SampleCount / Channels;
                        if (SampleCount > 0)
                            KAPI->Util.Sleep(SampleCount * 1000 / SampleRate); // milliseconds
                    }

                } while (RemainingBDL >= DescriptorListLength - 1 && !(inw(BAR.BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl));
            }

            {
                int CurrentBDL = inb(BAR.BusMasterAddress + PCMOUT_BufferDescriptorEntry);
                int LastBDL = inb(BAR.BusMasterAddress + PCMOUT_DescriptorEntries);
                int NextBDL = LastBDL % DescriptorListLength;

                ActiveDMA = !(inw(BAR.BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);
                if (ActiveDMA)
                {
                    NextBDL = (LastBDL + 1) % DescriptorListLength;
                    if (NextBDL == CurrentBDL)
                        continue;
                }

                do
                {
                    int Wrote = (KAPI->Memory.PageSize > Length) ? Length : KAPI->Memory.PageSize;

                    if (Wrote == 0)
                        break;
                    KAPI->Util.memcpy((void *)((uint64_t)DescriptorList[NextBDL].Address), Buffer, Wrote);
                    DescriptorList[NextBDL].Flags = 0;

                    Buffer += Wrote;
                    Length -= Wrote;

                    DescriptorList[NextBDL].SampleCount = Wrote / SampleSize;
                    TotalBDLToFill--;
                    NextBDL = (NextBDL + 1) % DescriptorListLength;
                } while (TotalBDLToFill-- && NextBDL != CurrentBDL);

                outb(BAR.BusMasterAddress + PCMOUT_DescriptorEntries, NextBDL - 1);

                ActiveDMA = !(inw(BAR.BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);
                if (!ActiveDMA)
                {
                    // Start DMA
                    outb(BAR.BusMasterAddress + PCMOUT_TransferControl, inb(BAR.BusMasterAddress + PCMOUT_TransferControl) | TC_DMAControllerControl);
                }
            }
        }
        break;
    }
    case StopReason:
    {
        outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(AV_Maximum, AV_Maximum, true));
        outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(AV_Maximum, AV_Maximum, true));

        // Stop DMA
        outb(BAR.BusMasterAddress + PCMOUT_TransferControl, inb(BAR.BusMasterAddress + PCMOUT_TransferControl) & ~TC_DMAControllerControl);

        // Disable interrupts
        uint8_t TransferControl = inb(BAR.BusMasterAddress + PCMOUT_TransferControl);
        TransferControl &= ~(TC_LastBufferEntryInterruptEnable | TC_IOCInterruptEnable | TC_FifoERRORInterruptEnable);
        outb(BAR.BusMasterAddress + PCMOUT_TransferControl, TransferControl);

        // Disable global control
        uint32_t GlobalControl = inl(BAR.BusMasterAddress + NABM_GlobalControl);
        GlobalControl &= ~GC_GlobalInterruptEnable;
        GlobalControl |= GC_ShutDown;
        outl(BAR.BusMasterAddress + NABM_GlobalControl, GlobalControl);

        print("Driver stopped.");
        break;
    }
    default:
    {
        print("Unknown reason.");
        break;
    }
    }
    return OK;
}

int InterruptCallback(CPURegisters *)
{
    uint16_t Status = inw(BAR.MixerAddress + PCMOUT_Status);

    if (Status & TC_IOCInterruptEnable)
    {
        print("Interrupt on completion.");
    }
    else if (Status & TC_LastBufferEntryInterruptEnable)
    {
        print("Last buffer entry.");
        // Stop DMA
        outb(BAR.BusMasterAddress + PCMOUT_TransferControl, inb(BAR.BusMasterAddress + PCMOUT_TransferControl) & ~TC_DMAControllerControl);
    }
    else if (Status & TC_FifoERRORInterruptEnable)
    {
        print("FIFO error.");
    }
    else if (Status != 0x0)
    {
        char UnknownStatusText[64];
        KAPI->Util.sprintf(UnknownStatusText, "Unknown status: %#lx", Status);
        print(UnknownStatusText);
    }

    outw(BAR.MixerAddress + PCMOUT_Status, 0xFFFF);
    return OK;
}
