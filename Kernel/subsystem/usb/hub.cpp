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

#include <usb.hpp>
#include <auto_page>

#include "../../kernel.h"

namespace UniversalSerialBus
{
#define req ((USBDeviceRequest *)KernelData)

	int Hub::ClearHubFeature(uint16_t FeatureSelector)
	{
		req->bmRequestType.raw = 0b00100000;
		req->bRequest = USB_HUB_CLEAR_FEATURE;
		req->wValue = FeatureSelector;
		req->wIndex = 0;
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::ClearPortFeature(uint16_t FeatureSelector, uint8_t Selector, uint8_t Port)
	{
		req->bmRequestType.raw = 0b00100011;
		req->bRequest = USB_HUB_CLEAR_FEATURE;
		req->wValue = FeatureSelector;
		req->wIndex = ((uint16_t)(Selector & 0xFF) << 8) | (uint16_t)(Port & 0xFF);
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::ClearTTBuffer(uint16_t DeviceAddressOrEndpointNumber, uint16_t TT_port)
	{
		req->bmRequestType.raw = 0b00100011;
		req->bRequest = USB_HUB_CLEAR_TT_BUFFER;
		req->wValue = DeviceAddressOrEndpointNumber;
		req->wIndex = TT_port;
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::GetHubDescriptor(uint16_t DescriptorTypeAndDescriptorIndex, uint16_t LanguageID, uint16_t DescriptorLength, void *Descriptor)
	{
		req->bmRequestType.raw = 0b10100000;
		req->bRequest = USB_HUB_GET_DESCRIPTOR;
		req->wValue = DescriptorTypeAndDescriptorIndex;
		req->wIndex = LanguageID;
		req->wLength = DescriptorLength;
		int result = usb->RequestDevice(this, req, Descriptor);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::GetHubStatus(USBPortStatus *HubStatusAndChangeStatus)
	{
		req->bmRequestType.raw = 0b10100000;
		req->bRequest = USB_HUB_GET_STATUS;
		req->wValue = 0;
		req->wIndex = 0;
		req->wLength = sizeof(USBPortStatus);
		int result = usb->RequestDevice(this, req, HubStatusAndChangeStatus);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::GetPortStatus(uint16_t Port, USBPortStatus *PortStatusAndChangeStatus)
	{
		req->bmRequestType.raw = 0b10100011;
		req->bRequest = USB_HUB_GET_STATUS;
		req->wValue = 0;
		req->wIndex = Port;
		req->wLength = sizeof(USBPortStatus);
		int result = usb->RequestDevice(this, req, PortStatusAndChangeStatus);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::Get_TT_State(uint16_t TT_Flags, uint16_t TT_Port, uint16_t TTStateLength, void *TTState)
	{
		req->bmRequestType.raw = 0b10100011;
		req->bRequest = USB_HUB_GET_TT_STATE;
		req->wValue = TT_Flags;
		req->wIndex = TT_Port;
		req->wLength = TTStateLength;
		int result = usb->RequestDevice(this, req, TTState);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::ResetTT(uint16_t TT_Port)
	{
		req->bmRequestType.raw = 0b00100011;
		req->bRequest = USB_HUB_RESET_TT;
		req->wValue = 0;
		req->wIndex = TT_Port;
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::SetHubDescriptor(uint16_t DescriptorTypeAndDescriptorIndex, uint16_t LanguageID, uint16_t DescriptorLength, void *Descriptor)
	{
		req->bmRequestType.raw = 0b00100000;
		req->bRequest = USB_HUB_SET_DESCRIPTOR;
		req->wValue = DescriptorTypeAndDescriptorIndex;
		req->wIndex = LanguageID;
		req->wLength = DescriptorLength;
		int result = usb->RequestDevice(this, req, Descriptor);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::SetHubFeature(uint16_t FeatureSelector)
	{
		req->bmRequestType.raw = 0b00100000;
		req->bRequest = USB_HUB_SET_FEATURE;
		req->wValue = FeatureSelector;
		req->wIndex = 0;
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::SetPortFeature(uint16_t FeatureSelector, uint8_t Selector, uint8_t Port)
	{
		req->bmRequestType.raw = 0b00100011;
		req->bRequest = USB_HUB_SET_FEATURE;
		req->wValue = FeatureSelector;
		req->wIndex = ((uint16_t)(Selector & 0xFF) << 8) | (uint16_t)(Port & 0xFF);
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::GetTTState(uint16_t TT_Flags, uint16_t TT_Port, uint16_t TTStateLength, void *TTState)
	{
		req->bmRequestType.raw = 0b10100011;
		req->bRequest = USB_HUB_GET_TT_STATE;
		req->wValue = TT_Flags;
		req->wIndex = TT_Port;
		req->wLength = TTStateLength;
		int result = usb->RequestDevice(this, req, TTState);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	int Hub::StopTT(uint16_t TT_Port)
	{
		req->bmRequestType.raw = 0b00100011;
		req->bRequest = USB_HUB_STOP_TT;
		req->wValue = 0;
		req->wIndex = TT_Port;
		req->wLength = 0;
		int result = usb->RequestDevice(this, req, 0);
		if (result != 0)
			warn("Operation failed: %d", result);
		return result;
	}

	Hub::Hub()
	{
		KernelData = KernelAllocator.RequestPages(TO_PAGES(sizeof(USBDeviceRequest)));
	}

	Hub::~Hub()
	{
		KernelAllocator.FreePages(KernelData, TO_PAGES(sizeof(USBDeviceRequest)));
	}

#undef req

	int HubPollStub(USBDevice *Device)
	{
		return 0;
	}

	int HubProbe(Hub *hub, USBHubDescriptor *Descriptor)
	{
		if ((Descriptor->wHubCharacteristics & USB_HUB_LPSM_MASK) == 0b01)
		{
			for (uint8_t port = 0; port < Descriptor->bNbrPorts; port++)
			{
				int result = hub->SetPortFeature(USB_PORT_POWER, 0, port + 1);
				if (result != 0)
				{
					debug("failed to power port: %d", result);
					return result;
				}
				TimeManager->Sleep(Time::FromMilliseconds(Descriptor->bPwrOn2PwrGood * 2));
				debug("port %d powered", port);

				result = hub->SetPortFeature(USB_PORT_RESET, 0, port + 1);
				if (result != 0)
				{
					debug("failed to reset port: %d", result);
					return result;
				}
			}
		}

		for (uint8_t port = 0; port < Descriptor->bNbrPorts; port++)
		{
			auto_page<USBPortStatus> status;
			Memory::Virtual().Map(status, status, sizeof(USBPortStatus), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
			// USBPortStatus *status = (USBPortStatus *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBPortStatus)));
			for (size_t i = 0; i < 16; i++)
			{
				TimeManager->Sleep(Time::FromMilliseconds(100));

				int result = hub->GetPortStatus(port + 1, status);
				if (result != 0)
				{
					debug("failed to get status: %d", result);
					return result;
				}

				// uint32_t *buf = status;
				// uint32_t sta = *buf;
				// if (~sta & (1 << 0))
				if (status->wPortStatus.PORT_CONNECTION == 0)
				{
					debug("device is NOT present on port %d", port);
					break;
				}

				// if (sta & (1 << 1))
				if (status->wPortStatus.PORT_ENABLE)
				{
					debug("port %d enabled", port);
					break;
				}
			}

			if (status->wPortStatus.PORT_ENABLE == 0)
			{
				debug("port %d not enabled", port);
				continue;
			}

			// uint32_t *__buf = status;
			// uint32_t sta = *__buf;

			// uint32_t portspeed = (sta & (3 << 9)) >> 9;
			USBSpeeds portspeed = status->wPortStatus.PORT_LOW_SPEED == 1 ? USB_LOW_SPEED : USB_FULL_SPEED;
			if (portspeed == USB_FULL_SPEED)
				portspeed = status->wPortStatus.PORT_HIGH_SPEED == 1 ? USB_HIGH_SPEED : USB_FULL_SPEED;

			USBDevice *dev = usb->CreateDevice();
			*dev = *hub;
			dev->DataToggle = 0;
			dev->Parent = hub;
			dev->KernelData = 0;
			dev->PrivateData = 0;

			dev->Port = port;
			dev->Speed = portspeed;
			dev->MaxPacketSize = 8;
			dev->Address = 0;
			dev->PortPoll = HubPollStub;

			usb->InitializeDevice(dev);
		}

		return 0;
	}

	int InitializeHub(USBDevice *Device)
	{
		if (Device->Interface.bInterfaceClass != USB_CLASS_HUB)
			return -ENODEV;

		Hub *hub = (Hub *)Device;
		new (hub) Hub;

		auto_page<USBHubDescriptor> desc;
		Memory::Virtual().Map(desc, desc, sizeof(USBHubDescriptor), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		int result = hub->GetHubDescriptor((USB_HUB_DESCRIPTOR << 8) | 0, 0, sizeof(USBHubDescriptor), desc);
		if (result != 0)
		{
			debug("failed to get hub descriptor: %d", result);
			return result;
		}

		debug("bDescLength:%d bDescriptorType:%d bNbrPorts:%d wHubCharacteristics:%d bPwrOn2PwrGood:%d", desc->bDescLength, desc->bDescriptorType, desc->bNbrPorts, desc->wHubCharacteristics, desc->bPwrOn2PwrGood);

		return HubProbe(hub, desc);
	}
}
