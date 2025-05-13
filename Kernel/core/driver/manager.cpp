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

#include <driver.hpp>

#include <interface/driver.h>
#include <interface/input.h>
#include <memory.hpp>
#include <stropts.h>
#include <ints.hpp>
#include <task.hpp>
#include <printf.h>
#include <exec.hpp>
#include <rand.hpp>
#include <cwalk.h>
#include <md5.h>

#include "../../kernel.h"

using namespace vfs;

namespace Driver
{
	dev_t Manager::RegisterDevice(dev_t DriverID, DeviceType Type, const InodeOperations *Operations)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers = DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		for (size_t i = 0; i < 128; i++)
		{
			const auto dOps = dop->find(i);
			const auto dOpsEnd = dop->end();
			if (dOps != dOpsEnd)
				continue;

			DeviceType devType = (DeviceType)(Type & DEVICE_TYPE_MASK);
			switch (devType)
			{
			case DEVICE_TYPE_INPUT:
			{
				std::string prefix = "event";
				for (size_t j = 0; j < 128; j++)
				{
					std::string deviceName = prefix + std::to_string(j);
					Node node = fs->Lookup(devInputNode, deviceName);
					if (node)
						continue;

					/* c rwx r-- r-- */
					mode_t mode = S_IRWXU |
								  S_IRGRP |
								  S_IROTH |
								  S_IFCHR;

					node = fs->Create(devInputNode, deviceName, mode);
					node->inode->SetDevice(DriverID, i);

					DriverHandlers dh{};
					dh.Ops = Operations;
					dh.Node = node->inode;
					dh.InputReports = new RingBuffer<InputReport>(16);
					dop->insert({i, std::move(dh)});
					return i;
				}

				ReturnLogError(-EOVERFLOW, "No available slots for device %d", DriverID);
			}
			default:
				ReturnLogError(-EINVAL, "Invalid device type %d", Type);
			}
		}

		ReturnLogError(-EOVERFLOW, "No available slots for device %d", DriverID);
	}

	dev_t Manager::CreateDeviceFile(dev_t DriverID, const char *name, mode_t mode, const InodeOperations *Operations)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers = DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		for (size_t i = 0; i < 128; i++)
		{
			const auto dOps = dop->find(i);
			if (dOps != dop->end())
				continue;

			eNode node = fs->Lookup(devNode, name);
			if (node == true)
				ReturnLogError(-EEXIST, "Device file %s already exists", name);

			node = fs->Create(devNode, name, mode);
			if (node == false)
				ReturnLogError(-node.Error, "Failed to create device file %s; %s", name, node.what());

			node.Value->inode->SetDevice(DriverID, i);

			DriverHandlers dh{};
			dh.Ops = Operations;
			dh.Node = node.Value->inode;
			dh.InputReports = new RingBuffer<InputReport>(16);
			dop->insert({i, std::move(dh)});
			return i;
		}

		ReturnLogError(-EOVERFLOW, "No available slots for device %d", DriverID);
	}

	int Manager::UnregisterDevice(dev_t DriverID, dev_t Device)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers = DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		const auto dOps = dop->find(Device);
		if (dOps == dop->end())
			ReturnLogError(-EINVAL, "Device %d not found", Device);
		dop->erase(dOps);
		fixme("remove eventX from /dev/input");
		fixme("delete InputReports");
		return 0;
	}

	dev_t Manager::RegisterBlockDevice(dev_t DriverID, struct BlockDevice *Device)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers = DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		std::string prefix = Device->Name;
		for (size_t j = 0; j < 128; j++)
		{
			std::string deviceName = prefix + std::to_string(j);
			Node node = fs->Lookup(devBlockNode, deviceName);
			if (node)
				continue;
			debug("Creating \"%s\"", deviceName.c_str());

			/* b rwx --- --- */
			mode_t mode = S_IRWXU |
						  S_IFBLK;

			node = fs->Create(devBlockNode, deviceName, mode);
			node->inode->SetDevice(DriverID, j);
			auto devNode = (Manager::DeviceInode *)node->inode;
			devNode->Size = Device->Size;
			devNode->BlockSize = Device->BlockSize;
			devNode->Blocks = Device->BlockCount;
			devNode->inode.PrivateData = Device->PrivateData;

			DriverHandlers dh{};
			dh.Ops = Device->Ops;
			dh.Node = node->inode;
			dop->insert({j, std::move(dh)});
			debug("dh ops:%#lx node:%#lx %d", dh.Ops, dh.Node, j);
			return j;
		}

		ReturnLogError(-EOVERFLOW, "No available slots for device %d", DriverID);
	}

	int Manager::UnregisterBlockDevice(dev_t DriverID, dev_t DeviceID)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers = DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		const auto dOps = dop->find(DeviceID);
		if (dOps == dop->end())
			ReturnLogError(-EINVAL, "Device %d not found", DeviceID);
		dop->erase(dOps);
		return 0;
	}

	void *Manager::AllocateMemory(dev_t DriverID, size_t Pages)
	{
		auto itr = Drivers.find(DriverID);
		assert(itr != Drivers.end());
		void *ptr = itr->second.vma->RequestPages(Pages);
		memset(ptr, 0, FROM_PAGES(Pages));
		return ptr;
	}

	void Manager::FreeMemory(dev_t DriverID, void *Pointer, size_t Pages)
	{
		auto itr = Drivers.find(DriverID);
		assert(itr != Drivers.end());
		itr->second.vma->FreePages(Pointer, Pages);
	}

	int Manager::ReportInputEvent(dev_t DriverID, InputReport *Report)
	{
		std::unordered_map<dev_t, Driver::DriverObject> &drivers =
			DriverManager->GetDrivers();
		const auto it = drivers.find(DriverID);
		if (it == drivers.end())
			ReturnLogError(-EINVAL, "Driver %d not found", DriverID);
		const Driver::DriverObject *drv = &it->second;

		auto dop = drv->DeviceOperations;
		auto dOps = dop->find(Report->Device);
		if (dOps == dop->end())
			ReturnLogError(-EINVAL, "Device %d not found", Report->Device);

		dOps->second.InputReports->Write(Report, 1);

		switch (Report->Type)
		{
		case INPUT_TYPE_KEYBOARD:
		{
			KeyboardReport *kReport = &Report->Keyboard;
			GlobalKeyboardInputReports.Write(kReport, 1);
			break;
		}
		case INPUT_TYPE_MOUSE:
		{
			MouseReport *mReport = &Report->Mouse;
			GlobalMouseInputReports.Write(mReport, 1);
			break;
		}
		default:
			assert(!"Invalid input type");
		}
		return 0;
	}
}
