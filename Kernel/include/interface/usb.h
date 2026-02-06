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

#ifndef __FENNIX_API_USB_H__
#define __FENNIX_API_USB_H__

#include <types.h>

/**
 * In USB 2.0 spec Table 11-13 at offset 7 the
 * maxmimum number of ports is 255:
 * "Port n (implementation-dependent, up to a maximum of 255 ports)."
 * but I don't think a device can have that many ports.
 */
#define USB_MAX_HUB_PORTS 31

#define USB_HUB_LPSM_MASK 0x03
#define USB_HUB_CPI_MASK 0x04
#define USB_HUB_OP_MASK 0x18
#define USB_HUB_TTTT_MASK 0x60
#define USB_HUB_PIS_MASK 0x80

/* Recipient: Hub */
#define USB_C_HUB_LOCAL_POWER 0
#define USB_C_HUB_OVER_CURRENT 1

/* Recipient: Port */
#define USB_PORT_CONNECTION 0
#define USB_PORT_ENABLE 1
#define USB_PORT_SUSPEND 2
#define USB_PORT_OVER_CURRENT 3
#define USB_PORT_RESET 4
#define USB_PORT_POWER 8
#define USB_PORT_LOW_SPEED 9
#define USB_C_PORT_CONNECTION 16
#define USB_C_PORT_ENABLE 17
#define USB_C_PORT_SUSPEND 18
#define USB_C_PORT_OVER_CURRENT 19
#define USB_C_PORT_RESET 20
#define USB_PORT_TEST 21
#define USB_PORT_INDICATOR 22

/* https://www.usb.org/defined-class-codes */
enum USBClassCodes
{
	/** Use class info in Interface Descriptors */
	USB_CLASS_INTERFACE = 0x00,
	/** Audio */
	USB_CLASS_AUDIO = 0x01,
	/** Communications and CDC Control */
	USB_CLASS_COMMUNICATION = 0x02,
	/** Human Interface Device */
	USB_CLASS_HID = 0x03,
	/** Physical */
	USB_CLASS_PHYSICAL = 0x05,
	/** Image */
	USB_CLASS_STILL_IMAGE = 0x06,
	/** Printer */
	USB_CLASS_PRINTER = 0x07,
	/** Mass Storage */
	USB_CLASS_MASS_STORAGE = 0x08,
	/** Hub (use in Device Descriptor) */
	USB_CLASS_HUB = 0x09,
	/** CDC-Data */
	USB_CLASS_CDC_DATA = 0x0A,
	/** Smart Card */
	USB_CLASS_SMART_CARD = 0x0B,
	/** Content Security */
	USB_CLASS_CONTENT_SECURITY = 0x0D,
	/** Video */
	USB_CLASS_VIDEO = 0x0E,
	/** Personal Healthcare */
	USB_CLASS_PERSONAL_HEALTHCARE = 0x0F,
	/** Audio/Video Devices */
	USB_CLASS_AUDIO_VIDEO = 0x10,
	/** Billboard Device Class (Device Descriptor) */
	USB_CLASS_BILLBOARD = 0x11,
	/** USB Type-C Bridge Class */
	USB_CLASS_TYPE_C_BRIDGE = 0x12,
	/** USB Bulk Display Protocol Device Class */
	USB_CLASS_BULK_DISPLAY = 0x13,
	/** MCTP over USB Protocol Endpoint Device Class */
	USB_CLASS_MCTP_OVER_USB = 0x14,
	/** I3C Device Class */
	USB_CLASS_I3C = 0x3C,
	/** Diagnostic Device (Both) */
	USB_CLASS_DIAGNOSTIC = 0xDC,
	/** Wireless Controller */
	USB_CLASS_WIRELESS_CONTROLLER = 0xE0,
	/** Miscellaneous (Both) */
	USB_CLASS_MISC = 0xEF,
	/** Application Specific (Interface) */
	USB_CLASS_APPLICATION_SPECIFIC = 0xFE,
	/** Vendor Specific (Both) */
	USB_CLASS_VENDOR_SPECIFIC = 0xFF
};

enum USBStandardRequestCodes
{
	USB_GET_STATUS = 0x00,
	USB_CLEAR_FEATURE = 0x01,
	USB_SET_FEATURE = 0x03,
	USB_SET_ADDRESS = 0x05,
	USB_GET_DESCRIPTOR = 0x06,
	USB_SET_DESCRIPTOR = 0x07,
	USB_GET_CONFIGURATION = 0x08,
	USB_SET_CONFIGURATION = 0x09,
	USB_GET_INTERFACE = 0x0A,
	USB_SET_INTERFACE = 0x0B,
	USB_SYNCH_FRAME = 0x0C
};

enum USBStandardDescriptorTypes
{
	USB_DEVICE_DESCRIPTOR = 0x01,
	USB_CONFIGURATION_DESCRIPTOR = 0x02,
	USB_STRING_DESCRIPTOR = 0x03,
	USB_INTERFACE_DESCRIPTOR = 0x04,
	USB_ENDPOINT_DESCRIPTOR = 0x05,
	USB_DEVICE_QUALIFIER_DESCRIPTOR = 0x06,
	USB_OTHER_SPEED_DESCRIPTOR = 0x07,
	USB_INTERFACE_POWER_DESCRIPTOR = 0x08,

	USB_OTG_DESCRIPTOR = 0x09,
	USB_DEBUG_DESCRIPTOR = 0x0A,
	USB_INTERFACE_ASSOCIATION_DESCRIPTOR = 0x0B,

	USB_BOS_DESCRIPTOR = 0x0F,
	USB_DEVICE_CAPABILITY_DESCRIPTOR = 0x10,
	USB_HID_DESCRIPTOR = 0x21,
	USB_REPORT_DESCRIPTOR = 0x22,
	USB_PHYSICAL_DESCRIPTOR = 0x23,
	USB_HUB_DESCRIPTOR = 0x29,
	USB_SUPERSPEED_HUB_DESCRIPTOR = 0x2A,
	USB_SUPERSPEED_ENDPOINT_COMPANION_DESCRIPTOR = 0x30
};

enum USBHIDSubclassCodes
{
	USB_HID_SUBCLASS_NONE = 0x00,
	USB_HID_SUBCLASS_BOOT = 0x01
};

enum USBHIDProtocolCodes
{
	USB_HID_PROTOCOL_NONE = 0x00,
	USB_HID_PROTOCOL_KEYBOARD = 0x01,
	USB_HID_PROTOCOL_MOUSE = 0x02
};

enum HubClassRequestCodes
{
	USB_HUB_GET_STATUS = 0x00,
	USB_HUB_CLEAR_FEATURE = 0x01,
	USB_HUB_SET_FEATURE = 0x03,
	USB_HUB_GET_DESCRIPTOR = 0x06,
	USB_HUB_SET_DESCRIPTOR = 0x07,
	USB_HUB_CLEAR_TT_BUFFER = 0x08,
	USB_HUB_RESET_TT = 0x09,
	USB_HUB_GET_TT_STATE = 0x0A,
	USB_HUB_STOP_TT = 0x0B,
};

struct USBDeviceRequest
{
	union
	{
		struct
		{
			/**
			 * 00000b = Device
			 * 00001b = Interface
			 * 00010b = Endpoint
			 * 00011b = Other
			 *  ...   = Reserved
			 */
			uint8_t Recipient : 5;

			/**
			 * 00b = Standard
			 * 01b = Class
			 * 10b = Vendor
			 * 11b = Reserved
			 */
			uint8_t Type : 2;

			/**
			 * 0b = Host to Device
			 * 1b = Device to Host
			 */
			uint8_t Direction : 1;
		};
		uint8_t raw;
	} bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
} __attribute__((packed));

struct USBEndpointDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
} __attribute__((packed));

struct USBDeviceDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
} __attribute__((packed));

struct USBStringDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wString[];
} __attribute__((packed));

struct USBInterfaceDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
} __attribute__((packed));

struct USBConfigurationDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
} __attribute__((packed));

struct USBHIDDescriptor
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
	struct
	{
		uint8_t bDescriptorType;
		uint16_t wDescriptorLength;
	} Descriptor[];
} __attribute__((packed));

/* USB 2.0 spec Table 11-13 */
struct USBHubDescriptor
{
	/**
	 * Number of bytes in this descriptor.
	 */
	uint8_t bDescLength;

	/**
	 * Descriptor Type
	 *
	 * @note Should be 0x29 (USB_HUB_DESCRIPTOR)
	 */
	uint8_t bDescriptorType;

	/**
	 * Number of downstream ports on the hub.
	 */
	uint8_t bNbrPorts;

	/**
	 * USB Hub Characteristics
	 *
	 * 0000'0000'0 00 0'0 0 00
	 * \       / | |/ \ / | |/       Logical Power Switching Mode
	 *  \     /  | |   |  | +-- 00b = Ganged Power Switching
	 *   \   /   | |   |  | +-- 01b = Individual Port Power Switching
	 *    \ /    | |   |  | +-- 10b = Reserved
	 * Reserved  | |   |  | +-- 11b = Reserved
	 *           | |   |  |
	 *           | |   |  |        Compound Device Identifier
	 *           | |   |  +-- 0b = Hub is not part of a compound device
	 *           | |   |  +-- 1b = Hub is part of a compound device
	 *           | |   |
	 *           | |   |        Overcurrent Protection
	 *           | |   +-- 00b = Global Overcurrent Protection
	 *           | |   +-- 01b = Individual Port Overcurrent Protection
	 *           | |   +-- 10b = No Overcurrent Protection
	 *           | |   +-- 11b = No Overcurrent Protection
	 *           | |
	 *           | |               TT Think Time
	 *           | +-- 00b = TT requires at most 8 FS bit times
	 *           | +-- 01b = TT requires at most 16 FS bit times
	 *           | +-- 10b = TT requires at most 24 FS bit times
	 *           | +-- 11b = TT requires at most 32 FS bit times
	 *           |
	 *           |      Port Indicators Supported
	 *           +-- 0b = Port Indicators not supported
	 *           +-- 1b = Port Indicators supported
	 */
	uint16_t wHubCharacteristics;

	/**
	 * Specifies the time, in 2 ms units, from when the power-on
	 * sequence starts on a port until power is stable on that port.
	 * The USB System Software uses this value to determine the
	 * required wait time before accessing a newly powered port.
	 */
	uint8_t bPwrOn2PwrGood;

	/**
	 * Maximum current requirement of the Hub Controller electronics.
	 *
	 * @note The value is represented in mA.
	 */
	uint8_t bHubContrCurrent;

	union
	{
		struct
		{
			uint8_t DeviceRemovable[(USB_MAX_HUB_PORTS + 1 + 7) / 8];
			uint8_t PortPwrCtrlMask[(USB_MAX_HUB_PORTS + 1 + 7) / 8];
		} __attribute__((packed)) usb20;

		struct
		{
			uint8_t bHubHdrDecLat;
			uint16_t wHubDelay;
			uint16_t DeviceRemovable;
		} __attribute__((packed)) usb30;
	} u;
} __attribute__((packed));

struct USBHubStatus
{
	union
	{
		struct
		{
			uint16_t LPS : 1;
			uint16_t OC : 1;
			uint16_t __reserved : 14;
		};
		uint16_t raw;
	} wHubStatus;

	union
	{
		struct
		{
			uint16_t LPSC : 1;
			uint16_t OCC : 1;
			uint16_t __reserved : 14;
		};
		uint16_t raw;
	} wHubChange;
} __attribute__((packed));

struct USBPortStatus
{
	union
	{
		struct
		{
			uint16_t PORT_CONNECTION : 1;
			uint16_t PORT_ENABLE : 1;
			uint16_t PORT_SUSPEND : 1;
			uint16_t PORT_OVER_CURRENT : 1;
			uint16_t PORT_RESET : 1;
			uint16_t __reserved0 : 3;
			uint16_t PORT_POWER : 1;
			uint16_t PORT_LOW_SPEED : 1;
			uint16_t PORT_HIGH_SPEED : 1;
			uint16_t PORT_TEST : 1;
			uint16_t PORT_INDICATOR : 1;
			uint16_t __reserved1 : 3;
		};
		uint16_t raw;
	} wPortStatus;

	union
	{
		struct
		{
			uint16_t C_PORT_CONNECTION : 1;
			uint16_t C_PORT_ENABLE : 1;
			uint16_t C_PORT_SUSPEND : 1;
			uint16_t C_PORT_OVER_CURRENT : 1;
			uint16_t C_PORT_RESET : 1;
			uint16_t __reserved0 : 11;
		};
		uint16_t raw;
	} wPortChange;
} __attribute__((packed));

/* ================================================ */

enum USBSpeeds
{
	USB_UNKNOWN_SPEED = 0,
	USB_LOW_SPEED,
	USB_FULL_SPEED,
	USB_HIGH_SPEED,
	USB_SUPER_SPEED
};

enum USBRequestStatus
{
	USB_REQ_SUCCESS = 0,
	USB_REQ_STALL,
	USB_REQ_TIMEOUT,
	USB_REQ_DISCONNECT,
	USB_REQ_CRC,
	USB_REQ_ERROR,
	USB_REQ_CANCELED
};

enum USBTransferType
{
	USB_TRANSFER_CONTROL,
	USB_TRANSFER_BULK,
	USB_TRANSFER_INTERRUPT,
	USB_TRANSFER_ISOCHRONOUS
};

struct USBEndpoint
{
	uint8_t Address;		/* Endpoint number + direction */
	uint8_t Type;			/* CONTROL / BULK / INTERRUPT / ISOCHRONOUS */
	uint16_t MaxPacketSize; /* Only 8, 16, 32 and 64 are valid. */
	uint8_t Toggle;			/* DATA0/DATA1 toggle */
};

struct USBRequestBlock
{
	void *Buffer;
	size_t Length;
	size_t ActualLength;

	struct USBDevice *Device;

	struct USBDeviceRequest *Request; /* Control Transfers only */
	uint8_t EndpointAddress;
	enum USBRequestStatus Status;
	enum USBTransferType Type;

	void (*Complete)(struct USBRequestBlock *urb);
	void *CompletionContext;
	void *PrivateData;
};

struct USBController
{
	int (*StartController)(struct USBController *Device);
	int (*StopController)(struct USBController *Device);
	int (*ResetController)(struct USBController *Device);

	int (*SubmitURB)(struct USBDevice *Device, struct USBRequestBlock *urb);
	int (*CancelURB)(struct USBDevice *Device, struct USBRequestBlock *urb);

	void *PrivateData;
};

struct USBDevice
{
	struct USBController *Controller;
	struct USBInterfaceDescriptor Interface;
	struct USBEndpoint Endpoints[16];
	uint8_t NumEndpoints;

	int Port;
	USBSpeeds Speed;

	struct USBDevice *Parent;
	struct USBDevice *Next;

	void *KernelData;
	void *PrivateData;

#ifdef __cplusplus
	int SubmitRequest(struct USBRequestBlock *urb) { return Controller->SubmitURB(this, urb); }
	int CancelRequest(struct USBRequestBlock *urb) { return Controller->CancelURB(this, urb); }
#endif // __cplusplus
};

#ifndef __kernel__
struct USBDevice *CreateUSBDevice();
int DestroyUSBDevice(struct USBDevice *Device);
int InitializeUSBDevice(struct USBDevice *Device);
int AddController(struct USBController *Controller);
int RemoveController(struct USBController *Controller);
#endif // __kernel__

#endif // __FENNIX_API_USB_H__
