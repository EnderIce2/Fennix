/*
	This file is part of Fennix Kernel.

	Fennix Kernel is free software: you can redistribute it and/or
	modify it under the terms of the GNU General Public License as
	published by the Free Software Foundation, either version 3 of
	the License, or (at your option) any later version.

	Fennix Kernel is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Fennix Kernel. If not, see <https://www.gnu.org/licenses/>.
*/

#if defined(__amd64__) || defined(__i386__)

#include <driver.hpp>
#include <cpu.hpp>
#include <pci.hpp>
#include <io.h>

extern Driver::Manager *DriverManager;
extern PCI::Manager *PCIManager;
namespace Driver::AC97
{
	dev_t DriverID;

#define DescriptorListLength 0x20

	enum AudioVolumeValues
	{
		AV_Maximum = 0x0,
		AV_Minimum = 0x3F,
	};

	enum AudioEncodingValues
	{
		AE_PCMs8,
		AE_PCMu8,

		AE_PCMs16le,
		AE_PCMs20le,
		AE_PCMs24le,
		AE_PCMs32le,

		AE_PCMu16le,
		AE_PCMu20le,
		AE_PCMu24le,
		AE_PCMu32le,

		AE_PCMs16be,
		AE_PCMs20be,
		AE_PCMs24be,
		AE_PCMs32be,

		AE_PCMu16be,
		AE_PCMu20be,
		AE_PCMu24be,
		AE_PCMu32be,
	};

	enum NativeAudioMixerRegisters
	{
		/**
		 * @brief Reset Register
		 * @note Length: word
		 */
		NAM_Reset = 0x00,

		/**
		 * @brief Master Volume Register
		 * @note Length: word
		 */
		NAM_MasterVolume = 0x02,

		/**
		 * @brief Microphone Volume Register
		 * @note Length: word
		 */
		NAM_MicrophoneVolume = 0x0E,

		/**
		 * @brief PCM Out Volume Register
		 * @note Length: word
		 */
		NAM_PCMOutVolume = 0x18,

		/**
		 * @brief Select Record Input Register
		 * @note Length: word
		 */
		NAM_SelectRecordInput = 0x1A,

		/**
		 * @brief Record Gain Register
		 * @note Length: word
		 */
		NAM_RecordGain = 0x1C,

		/**
		 * @brief Record Gain Microphone Register
		 * @note Length: word
		 */
		NAM_RecordGainMicrophone = 0x1E,
	};

	enum NativeAudioBusMasterRegisters
	{
		/**
		 * @brief Register box for PCM IN
		 * @note Length: below
		 */
		NABM_PCMInBox = 0x00,

		/**
		 * @brief Register box for PCM OUT
		 * @note Length: below
		 */
		NABM_PCMOutBox = 0x10,

		/**
		 * @brief Register box for Microphone
		 * @note Length: below
		 */
		NABM_MicrophoneBox = 0x20,

		/**
		 * @brief Global Control Register
		 * @note Length: dword
		 */
		NABM_GlobalControl = 0x2C, /* 0x30 */

		/**
		 * @brief Global Status Register
		 * @note Length: dword
		 */
		NABM_GlobalStatus = 0x30, /* 0x34 */
	};

	enum NativeAudioBusMasterBoxOffsets
	{
		/**
		 * @brief Physical Address of Buffer Descriptor List
		 * @note Length: dword
		 */
		NABMBOFF_BufferDescriptorList = 0x00,

		/**
		 * @brief Number of Actual Processed Buffer Descriptor Entry
		 * @note Length: byte
		 */
		NABMBOFF_BufferDescriptorEntry = 0x04,

		/**
		 * @brief Number of all Descriptor Entries
		 * @note Length: byte
		 */
		NABMBOFF_DescriptorEntries = 0x05,

		/**
		 * @brief Status of transferring Data
		 * @note Length: word
		 */
		NABMBOFF_Status = 0x06,

		/**
		 * @brief Number of transferred Samples in Actual Processed Entry
		 * @note Length: word
		 */
		NABMBOFF_TransferredSamples = 0x08,

		/**
		 * @brief Number of next processed Buffer Entry
		 * @note Length: byte
		 */
		NABMBOFF_NextProcessedBufferEntry = 0x0A,

		/**
		 * @brief Transfer Control
		 * @note Length: byte
		 */
		NABMBOFF_TransferControl = 0x0B,
	};

	enum OutputPulseCodeModulationRegisters
	{
		/**
		 * @brief Physical Address of Buffer Descriptor List
		 * @note Length: dword
		 */
		PCMOUT_BufferDescriptorList = (int)NABM_PCMOutBox + (int)NABMBOFF_BufferDescriptorList,

		/**
		 * @brief Number of Actual Processed Buffer Descriptor Entry
		 * @note Length: byte
		 */
		PCMOUT_BufferDescriptorEntry = (int)NABM_PCMOutBox + (int)NABMBOFF_BufferDescriptorEntry,

		/**
		 * @brief Number of all Descriptor Entries
		 * @note Length: byte
		 */
		PCMOUT_DescriptorEntries = (int)NABM_PCMOutBox + (int)NABMBOFF_DescriptorEntries,

		/**
		 * @brief Status of transferring Data
		 * @note Length: word
		 */
		PCMOUT_Status = (int)NABM_PCMOutBox + (int)NABMBOFF_Status,

		/**
		 * @brief Number of transferred Samples in Actual Processed Entry
		 * @note Length: word
		 */
		PCMOUT_TransferredSamples = (int)NABM_PCMOutBox + (int)NABMBOFF_TransferredSamples,

		/**
		 * @brief Number of next processed Buffer Entry
		 * @note Length: byte
		 */
		PCMOUT_NextProcessedBufferEntry = (int)NABM_PCMOutBox + (int)NABMBOFF_NextProcessedBufferEntry,

		/**
		 * @brief Transfer Control
		 * @note Length: byte
		 */
		PCMOUT_TransferControl = (int)NABM_PCMOutBox + (int)NABMBOFF_TransferControl,
	};

	enum TransferControlRegisters
	{
		/**
		 * @brief DMA controller control
		 *
		 * 0 = Pause transfer
		 * 1 = Transfer sound data
		 */
		TC_DMAControllerControl = 0x01,

		/**
		 * @brief Reset
		 *
		 * 0 = Remove reset condition
		 * 1 = Reset this NABM register box, this bit is cleared by card when is reset complete
		 */
		TC_TransferReset = 0x02,

		/**
		 * @brief Last Buffer Entry Interrupt enable
		 *
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_LastBufferEntryInterruptEnable = 0x04,

		/**
		 * @brief IOC Interrupt enable
		 *
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_IOCInterruptEnable = 0x08,

		/**
		 * @brief Fifo ERROR Interrupt enable
		 *
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_FifoERRORInterruptEnable = 0x10,
	};

	enum GlobalControlRegisters
	{
		/**
		 * @brief Global Interrupt Enable
		 *
		 * 0 = Disable Interrupts
		 * 1 = Enable Interrupts
		 */
		GC_GlobalInterruptEnable = 0x01,

		/**
		 * @brief Cold reset
		 *
		 * 0 = Device is in reset and can not be used
		 * 1 = Resume to operational state
		 */
		GC_ColdReset = 0x02,

		/**
		 * @brief Warm reset
		 */
		GC_WarmReset = 0x04,

		/**
		 * @brief Shut down
		 *
		 * 0 = Device is powered
		 * 1 = Shut down
		 */
		GC_ShutDown = 0x08,

		/**
		 * @brief Channels for PCM Output
		 *
		 * 00 = 2 channels
		 * 01 = 4 channels
		 * 10 = 6 channels
		 * 11 = Reserved
		 */
		GC_ChannelsForPCMOutput = 0x30,

		/**
		 * @brief PCM Output mode
		 *
		 * 00 = 16 bit samples
		 * 01 = 20 bit samples
		 */
		GC_PCMOutputMode = 0xC0,
	};

	struct BufferDescriptorList
	{
		/**
		 * @brief Physical Address to sound data in memory
		 * @note Length: dword
		 */
		uint32_t Address;

		/**
		 * @brief Number of samples in this buffer
		 * @note Length: word
		 */
		uint16_t SampleCount;

		/**
		 * @brief Flags
		 * @note Length: word
		 *
		 * Bit 15 = Interrupt fired when data from this entry is transferred
		 * Bit 14 = Last entry of buffer, stop playing
		 * Other bits = Reserved
		 */
		uint16_t Flags;
	} __attribute__((packed));

	uint16_t MixerVolume(uint8_t Left, uint8_t Right, bool Mute)
	{
		return ((uint16_t)((Right & 0x3F) |
						   ((Left & 0x3F) << 0x8) |
						   (Mute & 1 << 0xF)));
	}

	class AC97Device
	{
	private:
		PCI::PCIHeader0 *Header;
		BufferDescriptorList *DescriptorList = nullptr;

		uint16_t MixerAddress;
		uint16_t BusMasterAddress;

		AudioEncodingValues Encoding = AE_PCMs16le;
		char Channels = 2;
		uint8_t Volume = AV_Maximum;
		bool Mute = false;
		int SampleRate = 48000;
		char SampleSize = 2;

	public:
		size_t write(uint8_t *Buffer, size_t Size)
		{
			if (Buffer == nullptr)
			{
				info("Invalid buffer.");
				return -EINVAL;
			}

			if ((Size == 0) || (Size % (SampleSize * Channels)))
			{
				info("Invalid buffer length.");
				return -EINVAL;
			}

			int TotalBDLToFill = (int)((Size + PAGE_SIZE - 1) >> 12);

			while (Size > 0)
			{
				bool ActiveDMA = !(inw(BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);

				if (ActiveDMA)
				{
					int RemainingBDL = 0;

					do
					{
						int CurrentBDL = inb(BusMasterAddress + PCMOUT_BufferDescriptorEntry);
						int LastBDL = inb(BusMasterAddress + PCMOUT_DescriptorEntries);

						RemainingBDL = LastBDL - CurrentBDL;
						if (RemainingBDL < 0)
							RemainingBDL += DescriptorListLength;

						RemainingBDL += 1;

						if (RemainingBDL >= DescriptorListLength - 1)
						{
							long SampleCount = DescriptorList[(CurrentBDL + 1) % DescriptorListLength].SampleCount / Channels;
							if (SampleCount > 0)
								v0::Sleep(DriverID, SampleCount * 1000 / SampleRate);
						}

					} while (RemainingBDL >= DescriptorListLength - 1 &&
							 !(inw(BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl));
				}

				uint8_t CurrentBDL = inb(BusMasterAddress + PCMOUT_BufferDescriptorEntry);
				uint8_t LastBDL = inb(BusMasterAddress + PCMOUT_DescriptorEntries);
				uint8_t NextBDL = LastBDL % DescriptorListLength;

				ActiveDMA = !(inw(BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);
				if (ActiveDMA)
				{
					NextBDL = (uint8_t)((LastBDL + 1) % DescriptorListLength);
					if (NextBDL == CurrentBDL)
						continue;
				}

				do
				{
					size_t Wrote = (PAGE_SIZE > Size) ? size_t(Size)
													  : size_t(PAGE_SIZE);

					if (Wrote == 0)
					{
						info("Wrote 0 bytes.");
						break;
					}

					memcpy((void *)((uint64_t)DescriptorList[NextBDL].Address), Buffer, Wrote);
					DescriptorList[NextBDL].Flags = 0;

					Buffer += Wrote;
					Size -= (unsigned int)Wrote;

					DescriptorList[NextBDL].SampleCount = uint16_t(Wrote / SampleSize);
					TotalBDLToFill--;
					NextBDL = (uint8_t)((NextBDL + 1) % DescriptorListLength);
				} while (TotalBDLToFill-- && NextBDL != CurrentBDL);

				outb(BusMasterAddress + PCMOUT_DescriptorEntries, NextBDL - 1);

				ActiveDMA = !(inw(BusMasterAddress + PCMOUT_Status) & TC_DMAControllerControl);
				if (!ActiveDMA)
				{
					// Start DMA
					outb(BusMasterAddress + PCMOUT_TransferControl,
						 inb(BusMasterAddress + PCMOUT_TransferControl) | TC_DMAControllerControl);
				}
			}
			return Size;
		}

		int ioctl(AudioIoctl, void *)
		{
			// if (Data->AudioCallback.Adjust._Volume)
			// {
			// 	Volume = (uint8_t)(0x3F - (0x3F * Data->AudioCallback.Adjust.Volume / 100));
			// 	outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
			// 	// outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));
			// }
			// else if (Data->AudioCallback.Adjust._Encoding)
			// {
			// 	fixme("Encoding changing not supported yet.");
			// }
			// else if (Data->AudioCallback.Adjust._SampleRate)
			// {
			// 	switch (Data->AudioCallback.Adjust.SampleRate)
			// 	{
			// 	case 0:
			// 	{
			// 		SampleRate = 8000;
			// 		break;
			// 	}
			// 	case 1:
			// 	{
			// 		SampleRate = 11025;
			// 		break;
			// 	}
			// 	case 2:
			// 	{
			// 		SampleRate = 16000;
			// 		break;
			// 	}
			// 	case 3:
			// 	{
			// 		SampleRate = 22050;
			// 		break;
			// 	}
			// 	case 4:
			// 	{
			// 		SampleRate = 32000;
			// 		break;
			// 	}
			// 	case 5:
			// 	{
			// 		SampleRate = 44100;
			// 		break;
			// 	}
			// 	case 6:
			// 	{
			// 		SampleRate = 48000;
			// 		break;
			// 	}
			// 	case 7:
			// 	{
			// 		SampleRate = 88200;
			// 		break;
			// 	}
			// 	case 8:
			// 	{
			// 		SampleRate = 96000;
			// 		break;
			// 	}
			// 	default:
			// 	{
			// 		SampleRate = 16000;
			// 		error("Invalid sample rate. Defaulting to 16000.");
			// 		break;
			// 	}
			// 	}
			// }
			// else if (Data->AudioCallback.Adjust._Channels)
			// {
			// 	switch (Data->AudioCallback.Adjust.Channels)
			// 	{
			// 	case 0:
			// 	{
			// 		Channels = 1; // Mono
			// 		break;
			// 	}
			// 	case 1:
			// 	{
			// 		Channels = 2; // Stereo
			// 		break;
			// 	}
			// 	default:
			// 	{
			// 		Channels = 2;
			// 		error("Invalid channel count. Defaulting to 2.");
			// 		break;
			// 	}
			// 	}
			// }
			return 0;
		}

		void OnInterruptReceived(CPU::TrapFrame *)
		{
			uint16_t Status = inw(MixerAddress + PCMOUT_Status);
			if (Status & TC_IOCInterruptEnable)
			{
				debug("IOC");
				outw(MixerAddress + PCMOUT_Status, TC_IOCInterruptEnable);
				uint16_t CurrentBDL = inb(BusMasterAddress + PCMOUT_BufferDescriptorEntry);
				uint16_t LastBDL = (CurrentBDL + 2) & (DescriptorListLength - 1);
				outb(BusMasterAddress + PCMOUT_DescriptorEntries, LastBDL);
				info("FIXME: CurrentBDL: %d, LastBDL: %d", CurrentBDL, LastBDL);
			}
			else if (Status & TC_LastBufferEntryInterruptEnable)
			{
				debug("Last buffer entry");
				// Stop DMA
				uint8_t TransferControl = inb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl));
				TransferControl &= ~TC_DMAControllerControl;
				outb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl), TransferControl);

				outw(MixerAddress + PCMOUT_Status, TC_LastBufferEntryInterruptEnable);
			}
			else if (Status & TC_FifoERRORInterruptEnable)
			{
				info("FIFO error");
				outw(MixerAddress + PCMOUT_Status, TC_FifoERRORInterruptEnable);
			}
			else
			{
				debug("Unknown interrupt status %#x", Status);
				outw(MixerAddress + PCMOUT_Status, 0xFFFF);
			}
		}

		void Panic()
		{
			uint8_t TransferControl = inb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl));
			TransferControl &= ~(TC_LastBufferEntryInterruptEnable |
								 TC_IOCInterruptEnable |
								 TC_FifoERRORInterruptEnable);
			outb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl), TransferControl);

			uint32_t GlobalControl = inl((uint16_t)(BusMasterAddress + NABM_GlobalControl));
			GlobalControl &= ~GC_GlobalInterruptEnable;
			GlobalControl |= GC_ShutDown;
			outl((uint16_t)(BusMasterAddress + NABM_GlobalControl), GlobalControl);
		}

		AC97Device(PCI::PCIHeader0 *_Header)
			: Header(_Header)
		{
			/* Native Audio Mixer Base Address */
			uint32_t PCIBAR0 = Header->BAR0;

			/* Native Audio Bus Master Base Address */
			uint32_t PCIBAR1 = Header->BAR1;

			// uint8_t Type = PCIBAR0 & 1;
			MixerAddress = (uint16_t)(PCIBAR0 & (~3));
			BusMasterAddress = PCIBAR1 & (~15);

			uint16_t OutputPCMTransferControl = BusMasterAddress + PCMOUT_TransferControl;

			/* DescriptorList address MUST be physical. */
			DescriptorList = (BufferDescriptorList *)v0::AllocateMemory(DriverID, TO_PAGES(sizeof(BufferDescriptorList) * DescriptorListLength));
			memset(DescriptorList, 0, sizeof(BufferDescriptorList) * DescriptorListLength);

			uint16_t DLSampleCount = (uint16_t)(PAGE_SIZE / SampleSize);
			for (int i = 0; i < DescriptorListLength; i++)
			{
				DescriptorList[i].Address = (uint32_t)(uintptr_t)v0::AllocateMemory(DriverID, TO_PAGES(sizeof(uint16_t *)));
				DescriptorList[i].SampleCount = DLSampleCount;
				DescriptorList[i].Flags = 0;
				debug("DescriptorList[%d] = { Address: %#lx, SampleCount: %d, Flags: %#lx }",
					  i,
					  DescriptorList[i].Address,
					  DescriptorList[i].SampleCount,
					  DescriptorList[i].Flags);
			}

			outw(MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
			outw(MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));

			Volume = 0x3F - (0x3F * /* VOL 50% */ 50 / 100);
			outw(MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));

			outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) | TC_TransferReset);
			while (inb(OutputPCMTransferControl) & TC_TransferReset)
				;

			uint32_t GlobalControl = inl(BusMasterAddress + NABM_GlobalControl);
			GlobalControl = (GlobalControl & ~((0x3U) << 0x16)); /* PCM 16-bit mode */
			GlobalControl = (GlobalControl & ~((0x3U) << 20));	 /* 2 channels */
			GlobalControl |= GC_GlobalInterruptEnable;
			GlobalControl &= ~GC_ShutDown;

			outl(BusMasterAddress + PCMOUT_BufferDescriptorList,
				 (uint32_t)(uint64_t)DescriptorList);

			outl(BusMasterAddress + NABM_GlobalControl, GlobalControl);

			uint8_t TransferControl = inb(OutputPCMTransferControl);
			TransferControl |= TC_IOCInterruptEnable |
							   TC_FifoERRORInterruptEnable;
			outb(OutputPCMTransferControl, TransferControl);

			// Stop DMA
			outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) & ~TC_DMAControllerControl);
		}

		~AC97Device()
		{
			outw(MixerAddress + NAM_MasterVolume, MixerVolume(AV_Maximum, AV_Maximum, true));
			outw(MixerAddress + NAM_PCMOutVolume, MixerVolume(AV_Maximum, AV_Maximum, true));

			// Stop DMA
			outb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl),
				 inb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl)) & ~TC_DMAControllerControl);

			// Disable interrupts
			uint8_t TransferControl = inb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl));

			TransferControl &= ~(TC_LastBufferEntryInterruptEnable |
								 TC_IOCInterruptEnable |
								 TC_FifoERRORInterruptEnable);
			outb((uint16_t)(BusMasterAddress + PCMOUT_TransferControl), TransferControl);

			// Disable global control
			uint32_t GlobalControl = inl((uint16_t)(BusMasterAddress + NABM_GlobalControl));
			GlobalControl &= ~GC_GlobalInterruptEnable;
			GlobalControl |= GC_ShutDown;
			outl((uint16_t)(BusMasterAddress + NABM_GlobalControl), GlobalControl);
		}
	};

	std::unordered_map<dev_t, AC97Device *> Drivers;

	int Open(struct Inode *, int, mode_t)
	{
		return 0;
	}

	int Close(struct Inode *)
	{
		return 0;
	}

	ssize_t Read(struct Inode *, void *, size_t, off_t)
	{
		return 0;
	}

	ssize_t Write(struct Inode *Node, const void *Buffer, size_t Size, off_t)
	{
		return Drivers[Node->GetMinor()]->write((uint8_t *)Buffer, Size);
	}

	int Ioctl(struct Inode *Node, unsigned long Request, void *Argp)
	{
		return Drivers[Node->GetMinor()]->ioctl((AudioIoctl)Request, Argp);
	}

	const struct InodeOperations ops = {
		.Lookup = nullptr,
		.Create = nullptr,
		.Remove = nullptr,
		.Rename = nullptr,
		.Read = Read,
		.Write = Write,
		.Truncate = nullptr,
		.Open = Open,
		.Close = Close,
		.Ioctl = Ioctl,
		.ReadDir = nullptr,
		.MkDir = nullptr,
		.RmDir = nullptr,
		.SymLink = nullptr,
		.ReadLink = nullptr,
		.Seek = nullptr,
		.Stat = nullptr,
	};

	std::list<PCI::PCIDevice> Devices;
	int Entry()
	{
		for (auto &&dev : Devices)
		{
			PCIManager->InitializeDevice(dev, KernelPageTable);
			AC97Device *ac97 = new AC97Device((PCI::PCIHeader0 *)dev.Header);
			dev_t ret = v0::RegisterDevice(DriverID, NETWORK_TYPE_ETHERNET, &ops);
			Drivers[ret] = ac97;
		}

		if (Drivers.empty())
		{
			info("No valid AC'97 device found.");
			return -EINVAL;
		}

		return 0;
	}

	int Final()
	{
		for (auto &&dev : Drivers)
		{
			dev_t ret = dev.first;
			v0::UnregisterDevice(DriverID, ret);
			delete dev.second;
		}
		return 0;
	}

	int Panic()
	{
		for (auto &&i : Drivers)
			i.second->Panic();
		return 0;
	}

	int Probe()
	{
		Devices = PCIManager->FindPCIDevice(
			{
				0x8086, /* Intel */
			},
			{
				0x2415, /* AC'97 */
			});

		for (auto &&i : Devices)
		{
			PCI::PCIHeader0 *hdr0 = (PCI::PCIHeader0 *)i.Header;
			uint8_t type = hdr0->BAR0 & 1;
			if (type != 1)
			{
				warn("Device %x:%x.%d BAR0 is not I/O.",
					 hdr0->Header.VendorID,
					 hdr0->Header.DeviceID,
					 hdr0->Header.ProgIF);
				continue;
			}
		}

		if (Devices.empty())
		{
			info("No AC'97 device found.");
			return -ENODEV;
		}
		return 0;
	}

	REGISTER_BUILTIN_DRIVER(ac97,
							"Audio Codec '97 Driver",
							"enderice2",
							1, 0, 0,
							Entry,
							Final,
							Panic,
							Probe);
}

#endif
