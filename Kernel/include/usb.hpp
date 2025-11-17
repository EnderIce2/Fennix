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

#pragma once

#include <interface/usb.h>
#include <list>

static_assert(sizeof(USBDeviceRequest) == 0x8);

namespace UniversalSerialBus
{
	class Hub : public USBDevice
	{
	public:
		int ClearHubFeature(uint16_t FeatureSelector);
		int ClearPortFeature(uint16_t FeatureSelector, uint8_t Selector, uint8_t Port);
		int ClearTTBuffer(uint16_t DeviceAddressOrEndpointNumber, uint16_t TT_port);
		int GetHubDescriptor(uint16_t DescriptorTypeAndDescriptorIndex, uint16_t LanguageID, uint16_t DescriptorLength, void *Descriptor);
		int GetHubStatus(USBPortStatus *HubStatusAndChangeStatus);
		int GetPortStatus(uint16_t Port, USBPortStatus *PortStatusAndChangeStatus);
		int Get_TT_State(uint16_t TT_Flags, uint16_t TT_Port, uint16_t TTStateLength, void *TTState);
		int ResetTT(uint16_t TT_Port);
		int SetHubDescriptor(uint16_t DescriptorTypeAndDescriptorIndex, uint16_t LanguageID, uint16_t DescriptorLength, void *Descriptor);
		int SetHubFeature(uint16_t FeatureSelector);
		int SetPortFeature(uint16_t FeatureSelector, uint8_t Selector, uint8_t Port);
		int GetTTState(uint16_t TT_Flags, uint16_t TT_Port, uint16_t TTStateLength, void *TTState);
		int StopTT(uint16_t TT_Port);

		Hub();
		~Hub();
	};

	class Function : public USBDevice
	{
	public:
		Function();
		~Function();
	};

	class Compound : public USBDevice
	{
	public:
		Compound();
		~Compound();
	};

	class Scheduler : public USBScheduler
	{
	private:
		struct SchedPoolInternal : USBSchedulerPool
		{
			fnx::spinlock_t Lock;
			size_t SizeOfDescriptor;
			size_t Count;
			off_t Breadth;
			off_t Depth;
			off_t Software;

			void *GetBreadth(off_t Index);
			void *GetDepth(off_t Index);
			void *GetSoftware(off_t Index);
		};

		std::list<SchedPoolInternal> Pools;

	public:
		USBSchedulerPool *CreateNewPool(size_t SizeOfDescriptor, int RequredAlignment, size_t Count, off_t Breadth, off_t Depth, off_t Software);
		int GetCurrentFrame();

		int GetPoolElement(USBSchedulerPool *Pool, size_t Index, void **Element);
		int GetPoolElement(size_t PoolIndex, size_t Index, void **Element);

		bool Initialize(uint32_t FrameListData);
		Scheduler(size_t NumOfFrames, size_t NumOfSubFrames, size_t MaxBandwidth);
		~Scheduler();
	};

	class Manager
	{
	private:
		std::list<USBController *> Controllers;
		std::list<USBDevice *> Devices;
		size_t USBAddresses = 1;

	public:
		USBDevice *CreateDevice();
		int RemoveDevice(USBDevice *Device);
		int InitializeDevice(USBDevice *Device);
		int RequestDevice(USBDevice *Device, USBDeviceRequest *Request, void *Buffer);

		int AddController(USBController *Controller);
		int RemoveController(USBController *Controller);

		Manager();
		~Manager();
	};

	int InitializeHub(USBDevice *Device);
	int InitializeMouse(USBDevice *Device);
	int InitializeKeyboard(USBDevice *Device);
}
