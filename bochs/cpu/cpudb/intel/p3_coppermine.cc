/////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////
//
//   Copyright (c) 2025 frostbite3000
//          Written by frostbite3000
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA B 02110-1301 USA
//
/////////////////////////////////////////////////////////////////////////

#include "bochs.h"
#include "cpu.h"
#include "p3_coppermine.h"

#if BX_CPU_LEVEL >= 6

#define LOG_THIS cpu->

// used Game Cristal's CPUID dump as reference: https://github.com/mamedev/mame/blob/master/src/mame/pc/sis630.cpp#L136-L143

// CPUID 00000000: 00000003-756E6547-6C65746E-49656E69
// CPUID 00000001: 0000068A-00000002-00000000-0387F9FF
// CPUID 00000002: 03020101-00000000-00000000-0C040882
// CPUID 00000003: 00000000-00000000-CA976D2E-000082F6
// CPUID 80000000: 00000000-00000000-CA976D2E-000082F6
// CPUID C0000000: 00000000-00000000-CA976D2E-000082F6

p3_coppermine_t::p3_coppermine_t(BX_CPU_C *cpu): bx_cpuid_t(cpu)
{
  enable_cpu_extension(BX_ISA_X87);
  enable_cpu_extension(BX_ISA_486);
  enable_cpu_extension(BX_ISA_PENTIUM);
  enable_cpu_extension(BX_ISA_MMX);
  enable_cpu_extension(BX_ISA_P6);
  enable_cpu_extension(BX_ISA_SYSENTER_SYSEXIT);
  enable_cpu_extension(BX_ISA_SSE);
  enable_cpu_extension(BX_ISA_DEBUG_EXTENSIONS);
  enable_cpu_extension(BX_ISA_VME);
  enable_cpu_extension(BX_ISA_PSE);
  enable_cpu_extension(BX_ISA_PAE);
  enable_cpu_extension(BX_ISA_PGE);
  enable_cpu_extension(BX_ISA_MTRR);
  enable_cpu_extension(BX_ISA_PAT);
}

void p3_coppermine_t::get_cpuid_leaf(Bit32u function, Bit32u subfunction, cpuid_function_t *leaf) const
{
  switch(function) {
  case 0x00000000:
    get_leaf_0(0x3, "GenuineIntel", leaf);
    return;
  case 0x00000001:
    get_std_cpuid_leaf_1(leaf);
    return;
  case 0x00000002: // CPUID leaf 0x00000002 - Cache and TLB Descriptors
    get_leaf(leaf, 0x03020101, 0x00000000, 0x00000000, 0x0C040882);
    return;
  case 0x00000003: // Processor Serial Number
    get_leaf(leaf, 0x00000000, 0x00000000, 0xCA976D2E, 0x000082F6);
    return;
  case 0x8000000:
    get_leaf(leaf, 0x00000000, 0x00000000, 0xCA976D2E, 0x000082F6);
    return;
  case 0xC000000:
  default:
    get_leaf(leaf, 0x00000000, 0x00000000, 0xCA976D2E, 0x000082F6);
    return;
  }
}

// leaf 0x00000000 //

// leaf 0x00000001 //
void p3_coppermine_t::get_std_cpuid_leaf_1(cpuid_function_t *leaf) const
{
  // EAX:       CPU Version Information
  //   [3:0]   Stepping ID
  //   [7:4]   Model: starts at 1
  //   [11:8]  Family: 4=486, 5=Pentium, 6=PPro, ...
  //   [13:12] Type: 0=OEM, 1=overdrive, 2=dual cpu, 3=reserved
  //   [19:16] Extended Model
  //   [27:20] Extended Family
  leaf->eax = 0x0000068A;

  leaf->ebx = 0x00000002;
  leaf->ecx = 0;

  // EDX: Standard Feature Flags
  // * [0:0]   FPU on chip
  // * [1:1]   VME: Virtual-8086 Mode enhancements
  // * [2:2]   DE: Debug Extensions (I/O breakpoints)
  // * [3:3]   PSE: Page Size Extensions
  // * [4:4]   TSC: Time Stamp Counter
  // * [5:5]   MSR: RDMSR and WRMSR support
  // * [6:6]   PAE: Physical Address Extensions
  // * [7:7]   MCE: Machine Check Exception
  // * [8:8]   CXS: CMPXCHG8B instruction
  //   [9:9]   APIC: APIC on Chip
  //   [10:10] Reserved
  // * [11:11] SYSENTER/SYSEXIT support
  // * [12:12] MTRR: Memory Type Range Reg
  // * [13:13] PGE/PTE Global Bit
  // * [14:14] MCA: Machine Check Architecture
  // * [15:15] CMOV: Cond Mov/Cmp Instructions
  // * [16:16] PAT: Page Attribute Table
  // * [17:17] PSE-36: Physical Address Extensions
  // * [18:18] PSN: Processor Serial Number
  //   [19:19] CLFLUSH: CLFLUSH Instruction support
  //   [20:20] Reserved
  //   [21:21] DS: Debug Store
  //   [22:22] ACPI: Thermal Monitor and Software Controlled Clock Facilities
  // * [23:23] MMX Technology
  // * [24:24] FXSR: FXSAVE/FXRSTOR (also indicates CR4.OSFXSR is available)
  // * [25:25] SSE: SSE Extensions
  //   [26:26] SSE2: SSE2 Extensions
  //   [27:27] Self Snoop
  //   [28:28] Hyper Threading Technology
  //   [29:29] TM: Thermal Monitor
  //   [30:30] Reserved
  //   [31:31] PBE: Pending Break Enable
  leaf->edx = get_std_cpuid_leaf_1_edx(BX_CPUID_STD1_EDX_PROCESSOR_SERIAL_NUMBER);
}

// leaf 0x00000002 - Cache and TLB Descriptors //
// leaf 0x00000003 - Processor Serial Number   //

void p3_coppermine_t::dump_cpuid(void) const
{
  bx_cpuid_t::dump_cpuid(0x3, 0);
}

bx_cpuid_t *create_p3_coppermine_cpuid(BX_CPU_C *cpu) { return new p3_coppermine_t(cpu); }

#endif
