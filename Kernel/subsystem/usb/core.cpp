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
#include <errno.h>
#include <auto_page>

#include "../../kernel.h"

static_assert(sizeof(USBPortStatus) == 4, "USBPortStatus must be 4 bytes");

static_assert(sizeof(UniversalSerialBus::Hub) == sizeof(USBDevice), "Hub should be the same size as USBDevice");
static_assert(sizeof(UniversalSerialBus::Function) == sizeof(USBDevice), "Function should be the same size as USBDevice");
static_assert(sizeof(UniversalSerialBus::Compound) == sizeof(USBDevice), "Compound should be the same size as USBDevice");

namespace UniversalSerialBus
{
	USBDevice *Manager::CreateDevice()
	{
		debug("creating new usb device");
		USBDevice *dev = new USBDevice;
		debug("%#lx", dev);
		Devices.push_back(dev);
		debug("there are now %zu usb devices", Devices.size());
		return dev;
	}

	int Manager::RemoveDevice(USBDevice *Device)
	{
		debug("removing usb device");
		Devices.remove(Device);
		delete Device;
		return 0;
	}

	int Manager::InitializeDevice(USBDevice *Device)
	{
		// auto_page<USBDeviceDescriptor> desc;
		// auto_page<USBDeviceRequest> req;
		// auto_page<USBStringDescriptor> dd;
		// auto_page<USBStringDescriptor> sdesc;
		USBDeviceDescriptor *desc = (USBDeviceDescriptor *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBDeviceDescriptor)));
		USBDeviceRequest *req = (USBDeviceRequest *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBDeviceRequest)));
		USBStringDescriptor *dd = (USBStringDescriptor *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBStringDescriptor)));
		USBStringDescriptor *sdesc = (USBStringDescriptor *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBStringDescriptor)));
		Memory::Virtual().Map(desc, desc, sizeof(USBDeviceDescriptor), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		Memory::Virtual().Map(req, req, sizeof(USBDeviceRequest), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		Memory::Virtual().Map(dd, dd, sizeof(USBStringDescriptor), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		Memory::Virtual().Map(sdesc, sdesc, sizeof(USBStringDescriptor), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		req->bmRequestType.Recipient = 0b0;
		req->bmRequestType.Type = 0b0;
		req->bmRequestType.Direction = 0b1;
		req->bRequest = USB_GET_DESCRIPTOR;
		req->wValue = (USB_DEVICE_DESCRIPTOR << 8) | 0;
		req->wIndex = 0;
		req->wLength = 8; // sizeof(USBDeviceDescriptor);
		int result = this->RequestDevice(Device, req, desc);
		if (result != 0)
		{
			debug("failed to get device descriptor: %d", result);
			return result;
		}

		Device->MaxPacketSize = desc->bMaxPacketSize0;
		size_t address = USBAddresses++;
		debug("address: %d", address);

		req->bmRequestType.Recipient = 0b0;
		req->bmRequestType.Type = 0b0;
		req->bmRequestType.Direction = 0b0;
		req->bRequest = USB_SET_ADDRESS;
		req->wValue = address;
		req->wIndex = 0;
		req->wLength = 0;
		result = this->RequestDevice(Device, req, nullptr);
		if (result != 0)
		{
			debug("failed to set device address: %d", result);
			return result;
		}

		Device->Address = address;
		TaskManager->Sleep(10);

		req->bmRequestType.Recipient = 0b0;
		req->bmRequestType.Type = 0b0;
		req->bmRequestType.Direction = 0b1;
		req->bRequest = USB_GET_DESCRIPTOR;
		req->wValue = (USB_DEVICE_DESCRIPTOR << 8) | 0;
		req->wIndex = 0;
		req->wLength = sizeof(USBDeviceDescriptor);
		result = this->RequestDevice(Device, req, desc);
		if (result != 0)
		{
			debug("failed to get device descriptor: %d", result);
			return result;
		}

		debug("ver=%d.%d vendor=%#lx product=%#lx configs=%#lx",
			  desc->bcdUSB >> 8, (desc->bcdUSB >> 4) & 0xF,
			  desc->idVendor, desc->idProduct, desc->bNumConfigurations);

#define USB_STRING_SIZE 127
		uint16_t langs[USB_STRING_SIZE] = {};

		req->bmRequestType.Recipient = 0b0;
		req->bmRequestType.Type = 0b0;
		req->bmRequestType.Direction = 0b1;
		req->bRequest = USB_GET_DESCRIPTOR;
		req->wValue = (USB_STRING_DESCRIPTOR << 8) | 0;
		req->wIndex = 0;
		req->wLength = 1; // sizeof(USBStringDescriptor);
		result = this->RequestDevice(Device, req, sdesc);
		if (result != 0)
		{
			debug("failed to get string descriptor: %d", result);
			return result;
		}

		req->bmRequestType.Recipient = 0b0;
		req->bmRequestType.Type = 0b0;
		req->bmRequestType.Direction = 0b1;
		req->bRequest = USB_GET_DESCRIPTOR;
		req->wValue = (USB_STRING_DESCRIPTOR << 8) | 0;
		req->wIndex = 0;
		req->wLength = sdesc->bLength;
		result = this->RequestDevice(Device, req, sdesc);
		if (result != 0)
		{
			debug("failed to get string descriptor: %d", result);
			return result;
		}

		size_t langsize = (sdesc->bLength - 2) / 2;
		for (size_t i = 0; i < langsize; i++)
			langs[i] = sdesc->wString[i];
		langs[langsize] = 0;

		uint32_t lId = langs[0];
		if (lId)
		{
			char product[USB_STRING_SIZE]{};
			char vendor[USB_STRING_SIZE]{};
			char serial[USB_STRING_SIZE]{};

			auto lambdagetstring = [&](char *buffer, uint32_t langid, uint32_t index)
			{
				req->bmRequestType.Recipient = 0b0;
				req->bmRequestType.Type = 0b0;
				req->bmRequestType.Direction = 0b1;
				req->bRequest = USB_GET_DESCRIPTOR;
				req->wValue = (USB_STRING_DESCRIPTOR << 8) | index;
				req->wIndex = langid;
				req->wLength = 1;
				result = this->RequestDevice(Device, req, dd);
				if (result != 0)
				{
					debug("failed to get string descriptor: %d", result);
					return result;
				}

				req->bmRequestType.Recipient = 0b0;
				req->bmRequestType.Type = 0b0;
				req->bmRequestType.Direction = 0b1;
				req->bRequest = USB_GET_DESCRIPTOR;
				req->wValue = (USB_STRING_DESCRIPTOR << 8) | index;
				req->wIndex = langid;
				req->wLength = dd->bLength;
				result = this->RequestDevice(Device, req, dd);
				if (result != 0)
				{
					debug("failed to get string descriptor: %d", result);
					return result;
				}

				size_t langsize = (dd->bLength - 2) / 2;
				for (size_t i = 0; i < langsize; i++)
					buffer[i] = dd->wString[i];
				buffer[langsize] = 0;
				return 0;
			};

			lambdagetstring(vendor, lId, desc->iManufacturer);
			lambdagetstring(product, lId, desc->iProduct);
			lambdagetstring(serial, lId, desc->iSerialNumber);

			debug("product=\"%s\" vendor=\"%s\" serial=\"%s\"", product, vendor, serial);

			KPrint("USB: v%d.%d %s %s %s", desc->bcdUSB >> 8, (desc->bcdUSB >> 4) & 0xF, vendor, product, serial);
		}

		auto_page<uint8_t> configBuf;
		Memory::Virtual().Map(configBuf, configBuf, sizeof(uint8_t), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		uint8_t pickedConfValue = 0;
		USBInterfaceDescriptor *pickedIntfDesc = nullptr;
		USBEndpointDescriptor *pickedEndpDesc = nullptr;

		for (uint8_t confIndex = 0; confIndex < desc->bNumConfigurations; ++confIndex)
		{
			req->bmRequestType.Recipient = 0b0;
			req->bmRequestType.Type = 0b0;
			req->bmRequestType.Direction = 0b1;
			req->bRequest = USB_GET_DESCRIPTOR;
			req->wValue = (USB_CONFIGURATION_DESCRIPTOR << 8) | confIndex;
			req->wIndex = 0;
			req->wLength = 4;
			result = this->RequestDevice(Device, req, configBuf);
			if (result != 0)
			{
				debug("failed to get configuration descriptor header: %d", result);
				continue;
			}

			USBConfigurationDescriptor *confDesc = configBuf.as<USBConfigurationDescriptor>();
			if (confDesc->wTotalLength > PAGE_SIZE)
			{
				debug("configuration length %d greater than %zu bytes", confDesc->wTotalLength, sizeof(configBuf));
				continue;
			}

			req->wLength = confDesc->wTotalLength;
			result = this->RequestDevice(Device, req, configBuf);
			if (result != 0)
			{
				debug("failed to get full configuration descriptor: %d", result);
				continue;
			}

			KPrint("USB: Configuration %d, value=%d", confIndex, confDesc->bConfigurationValue);

			if (!pickedConfValue)
			{
				pickedConfValue = confDesc->bConfigurationValue;
			}

			uint8_t *data = configBuf + confDesc->bLength;
			uint8_t *end = configBuf + confDesc->wTotalLength;

			while (data < end)
			{
				uint8_t len = data[0];
				uint8_t type = data[1];

				switch (type)
				{
				case USB_INTERFACE_DESCRIPTOR:
				{
					USBInterfaceDescriptor *intfDesc = (USBInterfaceDescriptor *)data;
					if (!pickedIntfDesc)
					{
						pickedIntfDesc = intfDesc;
					}
					break;
				}
				case USB_ENDPOINT_DESCRIPTOR:
				{
					USBEndpointDescriptor *endpDesc = (USBEndpointDescriptor *)data;
					if (!pickedEndpDesc)
					{
						pickedEndpDesc = endpDesc;
					}
					break;
				}
				case USB_HID_DESCRIPTOR:
				{
					USBHIDDescriptor *hidDesc = (USBHIDDescriptor *)data;
					debug("HID descriptor: %d %d %d %d %d",
						  hidDesc->bLength, hidDesc->bDescriptorType, hidDesc->bcdHID,
						  hidDesc->bCountryCode, hidDesc->bNumDescriptors);
					break;
				}
				default:
					warn("unknown descriptor type %d", type);
					break;
				}

				if (len == 0)
					break; // Prevent infinite loop on malformed descriptors
				data += len;
			}
		}

		if (pickedConfValue && pickedIntfDesc && pickedEndpDesc)
		{
			req->bmRequestType.Recipient = 0b0;
			req->bmRequestType.Type = 0b0;
			req->bmRequestType.Direction = 0b0;
			req->bRequest = USB_SET_CONFIGURATION;
			req->wValue = pickedConfValue;
			req->wIndex = 0;
			req->wLength = 0;
			result = this->RequestDevice(Device, req, nullptr);
			if (result != 0)
			{
				debug("failed to set configuration: %d", result);
				return result;
			}

			Device->Endpoint = *pickedEndpDesc;
			Device->Interface = *pickedIntfDesc;

			KPrint("CLASS: %d SUBCLASS: %d PROTOCOL: %d", pickedIntfDesc->bInterfaceClass, pickedIntfDesc->bInterfaceSubClass, pickedIntfDesc->bInterfaceProtocol);

			InitializeHub(Device);
			InitializeMouse(Device);
			InitializeKeyboard(Device);

			// Device->Controller->PollHC(Device->Controller);
			// Device->PortPoll(Device);
		}

		return 0;
	}

	int Manager::RequestDevice(USBDevice *Device, USBDeviceRequest *Request, void *Buffer)
	{
		// auto_page<USBTransfer> transfer;
		USBTransfer *transfer = (USBTransfer *)KernelAllocator.RequestPages(TO_PAGES(sizeof(USBTransfer)));
		Memory::Virtual().Map(transfer, transfer, sizeof(USBTransfer), Memory::P | Memory::RW | Memory::PWT | Memory::PCD);
		transfer->Buffer = Buffer;
		transfer->Length = Request->wLength;
		transfer->Request = Request;
		int result = Device->PortCtl(Device, transfer);
		bool tr = transfer->Success;
		return (result == 0 && tr) ? 0 : (result == 0 ? -EIO : result);
	}

	int Manager::AddController(USBController *Controller)
	{
		debug("adding usb controller");
		Controllers.push_back(Controller);
		return 0;
	}

	int Manager::RemoveController(USBController *Controller)
	{
		debug("removing usb controller");
		Controllers.remove(Controller);
		return 0;
	}

	Manager::Manager()
	{
	}

	Manager::~Manager()
	{
	}
}
