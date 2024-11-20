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

#ifndef __FENNIX_KERNEL_CPU_x86_EXCEPTIONS_H__
#define __FENNIX_KERNEL_CPU_x86_EXCEPTIONS_H__

#include <types.h>

namespace CPU
{
    namespace x86
    {
        enum ISRExceptions
        {
            DivideByZero = 0x0,
            Debug = 0x1,
            NonMaskableInterrupt = 0x2,
            Breakpoint = 0x3,
            Overflow = 0x4,
            BoundRange = 0x5,
            InvalidOpcode = 0x6,
            DeviceNotAvailable = 0x7,
            DoubleFault = 0x8,
            CoprocessorSegmentOverrun = 0x9,
            InvalidTSS = 0xa,
            SegmentNotPresent = 0xb,
            StackSegmentFault = 0xc,
            GeneralProtectionFault = 0xd,
            PageFault = 0xe,
            x87FloatingPoint = 0x10,
            AlignmentCheck = 0x11,
            MachineCheck = 0x12,
            SIMDFloatingPoint = 0x13,
            Virtualization = 0x14,
            Security = 0x1e
        };
    }
}

#endif // !__FENNIX_KERNEL_CPU_x86_EXCEPTIONS_H__
