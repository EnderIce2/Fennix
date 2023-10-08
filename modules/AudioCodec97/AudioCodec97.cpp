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

#include "ac97.hpp"

#include <debug.h>
#include <pci.hpp>
#include <io.h>

#include "../../mapi.hpp"
#include "../mod.hpp"

using namespace PCI;

namespace AudioCodec97
{
	KernelAPI KAPI;

	/* https://wiki.osdev.org/AC97 */

	PCIDeviceHeader *PCIBaseAddress;
	BARData BAR;
	BufferDescriptorList *DescriptorList = nullptr;

	AudioEncodingValues Encoding = AE_PCMs16le;
	char Channels = 2;
	uint8_t Volume = AV_Maximum;
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
		KAPI = *(KernelAPI *)Data;
		if (KAPI.Version.Major < 0 || KAPI.Version.Minor < 0 || KAPI.Version.Patch < 0)
			return KERNEL_API_VERSION_NOT_SUPPORTED;

		return OK;
	}

	int CallbackHandler(KernelCallback *Data)
	{
		switch (Data->Reason)
		{
		case AcknowledgeReason:
		{
			debug("Kernel acknowledged the driver.");
			break;
		}
		case ConfigurationReason:
		{
			debug("Module received configuration data.");
			PCIBaseAddress = reinterpret_cast<PCIDeviceHeader *>(Data->RawPtr);
			PCIBaseAddress->Command |= PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;

			/* Native Audio Mixer Base Address */
			uint32_t PCIBAR0 = ((PCIHeader0 *)PCIBaseAddress)->BAR0;

			/* Native Audio Bus Master Base Address */
			uint32_t PCIBAR1 = ((PCIHeader0 *)PCIBaseAddress)->BAR1;

			BAR.Type = PCIBAR0 & 1;
			BAR.MixerAddress = (uint16_t)(PCIBAR0 & (~3));
			BAR.BusMasterAddress = PCIBAR1 & (~15);

			if (BAR.Type != 1)
			{
				error("BAR0 is not I/O.");
				return INVALID_PCI_BAR;
			}
			uint16_t OutputPCMTransferControl = (uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl);

			/* DescriptorList address MUST be physical. */
			DescriptorList = (BufferDescriptorList *)KAPI.Memory.RequestPage((sizeof(BufferDescriptorList) * DescriptorListLength) / KAPI.Memory.PageSize + 1);
			memset(DescriptorList, 0, sizeof(BufferDescriptorList) * DescriptorListLength);

			uint16_t DLSampleCount = (uint16_t)(KAPI.Memory.PageSize / SampleSize);
			for (int i = 0; i < DescriptorListLength; i++)
			{
				int DescriptorPages = (int)(sizeof(uint16_t *) / KAPI.Memory.PageSize + 1);
				DescriptorList[i].Address = (uint32_t)(uint64_t)KAPI.Memory.RequestPage(DescriptorPages);
				DescriptorList[i].SampleCount = DLSampleCount;
				DescriptorList[i].Flags = 0;
				debug("DescriptorList[%d] = { Address: 0x%x (%d %s), SampleCount: %d, Flags: 0x%x }",
					  i,
					  DescriptorList[i].Address, DescriptorPages, DescriptorPages == 1 ? "page" : "pages",
					  DescriptorList[i].SampleCount,
					  DescriptorList[i].Flags);
			}

			outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
			outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));

			outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) | TC_TransferReset);
			while (inb(OutputPCMTransferControl) & TC_TransferReset)
				;

			uint32_t GlobalControl = inl((uint16_t)(BAR.BusMasterAddress + NABM_GlobalControl));
			GlobalControl = (GlobalControl & ~((0x3U) << 22)); /* PCM 16-bit mode */
			GlobalControl = (GlobalControl & ~((0x3U) << 20)); /* 2 channels */
			GlobalControl |= GC_GlobalInterruptEnable;
			GlobalControl &= ~GC_ShutDown;

			outl((uint16_t)(BAR.BusMasterAddress + PCMOUT_BufferDescriptorList), (uint32_t)(uint64_t)DescriptorList);
			outl((uint16_t)(BAR.BusMasterAddress + NABM_GlobalControl), GlobalControl);

			uint8_t TransferControl = inb(OutputPCMTransferControl);
			TransferControl |= TC_LastBufferEntryInterruptEnable | TC_IOCInterruptEnable | TC_FifoERRORInterruptEnable;
			outb(OutputPCMTransferControl, TransferControl);

			// Stop DMA
			outb(OutputPCMTransferControl, inb(OutputPCMTransferControl) & ~TC_DMAControllerControl);
			debug("AC'97 configured.");
			break;
		}
		case AdjustReason:
		{
			if (Data->AudioCallback.Adjust._Volume)
			{
				Volume = (uint8_t)(0x3F - (0x3F * Data->AudioCallback.Adjust.Volume / 100));
				outw(BAR.MixerAddress + NAM_MasterVolume, MixerVolume(Volume, Volume, Mute));
				// outw(BAR.MixerAddress + NAM_PCMOutVolume, MixerVolume(Volume, Volume, Mute));
			}
			else if (Data->AudioCallback.Adjust._Encoding)
			{
				fixme("Encoding changing not supported yet.");
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
					error("Invalid sample rate. Defaulting to 16000.");
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
					error("Invalid channel count. Defaulting to 2.");
					break;
				}
				}
			}
			break;
		}
		case QueryReason:
		{
			Data->AudioCallback.Fetch.Volume = (unsigned char)((inw(BAR.MixerAddress + NAM_MasterVolume) & 0x3F) * 100 / 0x3F);
			Data->AudioCallback.Fetch.Encoding = Encoding; /* FIXME */
			// Data->AudioCallback.Fetch.SampleRate = SampleRate; /* FIXME */
			Data->AudioCallback.Fetch.Channels = Channels;
			break;
		}
		case SendReason:
		{
			unsigned char *Buffer = (unsigned char *)Data->AudioCallback.Send.Data;
			unsigned int Length = (unsigned int)Data->AudioCallback.Send.Length;

			if (Buffer == nullptr)
			{
				error("Invalid buffer.");
				return INVALID_DATA;
			}

			if ((Length == 0) || (Length % (SampleSize * Channels)))
			{
				error("Invalid buffer length.");
				return INVALID_DATA;
			}

			int TotalBDLToFill = (int)((Length + KAPI.Memory.PageSize - 1) >> 12);

			while (Length > 0)
			{
				bool ActiveDMA = !(inw((uint16_t)(BAR.BusMasterAddress + PCMOUT_Status)) & TC_DMAControllerControl);

				if (ActiveDMA)
				{
					int RemainingBDL = 0;

					do
					{
						int CurrentBDL = inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_BufferDescriptorEntry));
						int LastBDL = inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_DescriptorEntries));

						RemainingBDL = LastBDL - CurrentBDL;
						if (RemainingBDL < 0)
							RemainingBDL += DescriptorListLength;

						RemainingBDL += 1;

						if (RemainingBDL >= DescriptorListLength - 1)
						{
							long SampleCount = DescriptorList[(CurrentBDL + 1) % DescriptorListLength].SampleCount / Channels;
							if (SampleCount > 0)
								KAPI.Util.Sleep(SampleCount * 1000 / SampleRate); // milliseconds
						}

					} while (RemainingBDL >= DescriptorListLength - 1 && !(inw((uint16_t)(BAR.BusMasterAddress + PCMOUT_Status)) & TC_DMAControllerControl));
				}

				{
					uint8_t CurrentBDL = inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_BufferDescriptorEntry));
					uint8_t LastBDL = inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_DescriptorEntries));
					uint8_t NextBDL = LastBDL % DescriptorListLength;

					ActiveDMA = !(inw((uint16_t)(BAR.BusMasterAddress + PCMOUT_Status)) & TC_DMAControllerControl);
					if (ActiveDMA)
					{
						NextBDL = (uint8_t)((LastBDL + 1) % DescriptorListLength);
						if (NextBDL == CurrentBDL)
							continue;
					}

					do
					{
						size_t Wrote = (KAPI.Memory.PageSize > Length) ? size_t(Length)
																	   : size_t(KAPI.Memory.PageSize);

						if (Wrote == 0)
							break;
						memcpy((void *)((uint64_t)DescriptorList[NextBDL].Address), Buffer, Wrote);
						DescriptorList[NextBDL].Flags = 0;

						Buffer += Wrote;
						Length -= (unsigned int)Wrote;

						DescriptorList[NextBDL].SampleCount = (uint16_t)(Wrote / SampleSize);
						TotalBDLToFill--;
						NextBDL = (uint8_t)((NextBDL + 1) % DescriptorListLength);
					} while (TotalBDLToFill-- && NextBDL != CurrentBDL);

					outb((uint16_t)(BAR.BusMasterAddress + PCMOUT_DescriptorEntries), NextBDL - 1);

					ActiveDMA = !(inw((uint16_t)(BAR.BusMasterAddress + PCMOUT_Status)) & TC_DMAControllerControl);
					if (!ActiveDMA)
					{
						// Start DMA
						outb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl), inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl) | TC_DMAControllerControl));
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
			outb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl), inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl)) & ~TC_DMAControllerControl);

			// Disable interrupts
			uint8_t TransferControl = inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl));
			TransferControl &= ~(TC_LastBufferEntryInterruptEnable | TC_IOCInterruptEnable | TC_FifoERRORInterruptEnable);
			outb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl), TransferControl);

			// Disable global control
			uint32_t GlobalControl = inl((uint16_t)(BAR.BusMasterAddress + NABM_GlobalControl));
			GlobalControl &= ~GC_GlobalInterruptEnable;
			GlobalControl |= GC_ShutDown;
			outl((uint16_t)(BAR.BusMasterAddress + NABM_GlobalControl), GlobalControl);

			debug("Module stopped.");
			break;
		}
		default:
		{
			warn("Unknown reason.");
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
			debug("Interrupt on completion.");
		}
		else if (Status & TC_LastBufferEntryInterruptEnable)
		{
			debug("Last buffer entry.");
			// Stop DMA
			outb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl), inb((uint16_t)(BAR.BusMasterAddress + PCMOUT_TransferControl)) & ~TC_DMAControllerControl);
		}
		else if (Status & TC_FifoERRORInterruptEnable)
		{
			debug("FIFO error.");
		}
		else if (Status != 0x0)
		{
			error("Unknown status: %#lx", Status);
		}

		outw(BAR.MixerAddress + PCMOUT_Status, 0xFFFF);
		return OK;
	}
}
