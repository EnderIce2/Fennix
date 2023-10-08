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

#ifndef __FENNIX_KERNEL_CPU_SIGNATURES_H__
#define __FENNIX_KERNEL_CPU_SIGNATURES_H__

#include <types.h>

#define x86_CPUID_VENDOR_OLDAMD "AMDisbetter!" /* Early engineering samples of AMD K5 processor */
#define x86_CPUID_VENDOR_AMD "AuthenticAMD"
#define x86_CPUID_VENDOR_INTEL "GenuineIntel"
#define x86_CPUID_VENDOR_VIA "CentaurHauls"
#define x86_CPUID_VENDOR_OLDTRANSMETA "TransmetaCPU"
#define x86_CPUID_VENDOR_TRANSMETA "GenuineTMx86"
#define x86_CPUID_VENDOR_CYRIX "CyrixInstead"
#define x86_CPUID_VENDOR_CENTAUR "CentaurHauls"
#define x86_CPUID_VENDOR_NEXGEN "NexGenDriven"
#define x86_CPUID_VENDOR_UMC "UMC UMC UMC "
#define x86_CPUID_VENDOR_SIS "SiS SiS SiS "
#define x86_CPUID_VENDOR_NSC "Geode by NSC"
#define x86_CPUID_VENDOR_RISE "RiseRiseRise"
#define x86_CPUID_VENDOR_VORTEX "Vortex86 SoC"
#define x86_CPUID_VENDOR_VIA2 "VIA VIA VIA "
#define x86_CPUID_VENDOR_HYGON "HygonGenuine"
#define x86_CPUID_VENDOR_E2K "E2K MACHINE"
#define x86_CPUID_VENDOR_MISTER "MiSTer AO486"

/* Vendor-strings from Virtual Machines. */
#define x86_CPUID_VENDOR_VMWARE "VMwareVMware"
#define x86_CPUID_VENDOR_XENHVM "XenVMMXenVMM"
#define x86_CPUID_VENDOR_MICROSOFT_HV "Microsoft Hv"
#define x86_CPUID_VENDOR_MICROSOFT_XTA "MicrosoftXTA"
#define x86_CPUID_VENDOR_PARALLELS " lrpepyh vr"
#define x86_CPUID_VENDOR_KVM "KVMKVMKVM"
#define x86_CPUID_VENDOR_VIRTUALBOX "VBoxVBoxVBox"
#define x86_CPUID_VENDOR_TCG "TCGTCGTCGTCG"
#define x86_CPUID_VENDOR_BHYVE "bhyve bhyve "
#define x86_CPUID_VENDOR_ACRN "ACRNACRNACRN"
#define x86_CPUID_VENDOR_QNX "QNXQVMBSQG"
#define x86_CPUID_VENDOR_APPLE "VirtualApple"

#define x86_CPUID_SIGNATURE_INTEL_b 0x756e6547
#define x86_CPUID_SIGNATURE_INTEL_c 0x6c65746e
#define x86_CPUID_SIGNATURE_INTEL_d 0x49656e69

#define x86_CPUID_SIGNATURE_AMD_b 0x68747541
#define x86_CPUID_SIGNATURE_AMD_c 0x444d4163
#define x86_CPUID_SIGNATURE_AMD_d 0x69746e65

#define x86_CPUID_SIGNATURE_CENTAUR_b 0x746e6543
#define x86_CPUID_SIGNATURE_CENTAUR_c 0x736c7561
#define x86_CPUID_SIGNATURE_CENTAUR_d 0x48727561

#define x86_CPUID_SIGNATURE_CYRIX_b 0x69727943
#define x86_CPUID_SIGNATURE_CYRIX_c 0x64616574
#define x86_CPUID_SIGNATURE_CYRIX_d 0x736e4978

#define x86_CPUID_SIGNATURE_TM1_b 0x6e617254
#define x86_CPUID_SIGNATURE_TM1_c 0x55504361
#define x86_CPUID_SIGNATURE_TM1_d 0x74656d73

#define x86_CPUID_SIGNATURE_TM2_b 0x756e6547
#define x86_CPUID_SIGNATURE_TM2_c 0x3638784d
#define x86_CPUID_SIGNATURE_TM2_d 0x54656e69

#define x86_CPUID_SIGNATURE_NSC_b 0x646f6547
#define x86_CPUID_SIGNATURE_NSC_c 0x43534e20
#define x86_CPUID_SIGNATURE_NSC_d 0x79622065

#define x86_CPUID_SIGNATURE_NEXGEN_b 0x4778654e
#define x86_CPUID_SIGNATURE_NEXGEN_c 0x6e657669
#define x86_CPUID_SIGNATURE_NEXGEN_d 0x72446e65

#define x86_CPUID_SIGNATURE_RISE_b 0x65736952
#define x86_CPUID_SIGNATURE_RISE_c 0x65736952
#define x86_CPUID_SIGNATURE_RISE_d 0x65736952

#define x86_CPUID_SIGNATURE_SIS_b 0x20536953
#define x86_CPUID_SIGNATURE_SIS_c 0x20536953
#define x86_CPUID_SIGNATURE_SIS_d 0x20536953

#define x86_CPUID_SIGNATURE_UMC_b 0x20434d55
#define x86_CPUID_SIGNATURE_UMC_c 0x20434d55
#define x86_CPUID_SIGNATURE_UMC_d 0x20434d55

#define x86_CPUID_SIGNATURE_VIA_b 0x20414956
#define x86_CPUID_SIGNATURE_VIA_c 0x20414956
#define x86_CPUID_SIGNATURE_VIA_d 0x20414956

#define x86_CPUID_SIGNATURE_VORTEX_b 0x74726f56
#define x86_CPUID_SIGNATURE_VORTEX_c 0x436f5320
#define x86_CPUID_SIGNATURE_VORTEX_d 0x36387865

#endif // !__FENNIX_KERNEL_CPU_SIGNATURES_H__
