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

#ifndef __FENNIX_KERNEL_AC97_H__
#define __FENNIX_KERNEL_AC97_H__

#include <types.h>
#include "../../mapi.hpp"

namespace AudioCodec97
{
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
		 * 0 = Pause transfer
		 * 1 = Transfer sound data
		 */
		TC_DMAControllerControl = 0x01,

		/**
		 * @brief Reset
		 * 0 = Remove reset condition
		 * 1 = Reset this NABM register box, this bit is cleared by card when is reset complete
		 */
		TC_TransferReset = 0x02,

		/**
		 * @brief Last Buffer Entry Interrupt enable
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_LastBufferEntryInterruptEnable = 0x04,

		/**
		 * @brief IOC Interrupt enable
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_IOCInterruptEnable = 0x08,

		/**
		 * @brief Fifo ERROR Interrupt enable
		 * 0 = Disable interrupt
		 * 1 = Enable interrupt
		 */
		TC_FifoERRORInterruptEnable = 0x10,
	};

	enum GlobalControlRegisters
	{
		/**
		 * @brief Global Interrupt Enable
		 * 0 = Disable Interrupts
		 * 1 = Enable Interrupts
		 */
		GC_GlobalInterruptEnable = 0x01,

		/**
		 * @brief Cold reset
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
		 * 0 = Device is powered
		 * 1 = Shut down
		 */
		GC_ShutDown = 0x08,

		/**
		 * @brief Channels for PCM Output
		 * 00 = 2 channels
		 * 01 = 4 channels
		 * 10 = 6 channels
		 * 11 = Reserved
		 */
		GC_ChannelsForPCMOutput = 0x30,

		/**
		 * @brief PCM Output mode
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

	struct BARData
	{
		uint8_t Type;
		uint16_t MixerAddress;
		uint64_t BusMasterAddress;
	};

	int DriverEntry(void *);
	int CallbackHandler(KernelCallback *);
	int InterruptCallback(CPURegisters *);
}

#endif // !__FENNIX_KERNEL_AC97_H__
