/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2012 Advanced Micro Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <stdint.h>
#include <string.h>
#include <device/pci_def.h>
#include <device/pci_ids.h>
#include <arch/acpi.h>
#include <arch/io.h>
#include <arch/stages.h>
#include <device/pnp_def.h>
#include <arch/cpu.h>
#include <cpu/x86/lapic.h>
#include <console/console.h>
#include <console/loglevel.h>
#include "cpu/amd/car.h"
#include "agesawrapper.h"
#include <northbridge/amd/agesa/agesawrapper_call.h>
#include "cpu/x86/bist.h"
#include "cpu/x86/lapic.h"
#include "southbridge/amd/agesa/hudson/hudson.h"
#include "src/superio/smsc/lpc47n217/early_serial.c"
#include "cpu/amd/agesa/s3_resume.h"
#include "cbmem.h"

#define SERIAL_DEV PNP_DEV(0x2e, LPC47N217_SP1)


void cache_as_ram_main(unsigned long bist, unsigned long cpu_init_detectedx)
{
	u32 val;
	u8 byte;
	device_t dev;
	AGESAWRAPPER_PRE_CONSOLE(amdinitmmio);

	hudson_lpc_port80();
	//__asm__ volatile ("1: jmp 1b");
	/* TODO: */
	dev = PCI_DEV(0, 0x14, 3);//pci_locate_device(PCI_ID(0x1002, 0x439D), 0);
	byte = pci_read_config8(dev, 0x48);
	byte |= 3;		/* 2e, 2f */
	pci_write_config8(dev, 0x48, byte);

	if (!cpu_init_detectedx && boot_cpu()) {
		post_code(0x30);

		post_code(0x31);
		lpc47n217_enable_serial(SERIAL_DEV, CONFIG_TTYS0_BASE);
		outb(0x24, 0xcd6);
		outb(0x1, 0xcd7);
		outb(0xea, 0xcd6);
		outb(0x1, 0xcd7);
		*(u8 *)0xfed80101 = 0x98;
		console_init();
	}

	/* Halt if there was a built in self test failure */
	post_code(0x34);
	report_bist_failure(bist);

	/* Load MPB */
	val = cpuid_eax(1);
	printk(BIOS_DEBUG, "BSP Family_Model: %08x\n", val);
	printk(BIOS_DEBUG, "cpu_init_detectedx = %08lx\n", cpu_init_detectedx);

	post_code(0x37);
	AGESAWRAPPER(amdinitreset);
	post_code(0x39);

	AGESAWRAPPER(amdinitearly);
	int s3resume = acpi_is_wakeup_early() && acpi_s3_resume_allowed();
	if (!s3resume) {
		post_code(0x40);
		AGESAWRAPPER(amdinitpost);
		post_code(0x41);
		AGESAWRAPPER(amdinitenv);
		disable_cache_as_ram();
	} else {		/* S3 detect */
		printk(BIOS_INFO, "S3 detected\n");

		post_code(0x60);
		AGESAWRAPPER(amdinitresume);

		AGESAWRAPPER(amds3laterestore);

		post_code(0x61);
		prepare_for_resume();
	}

	post_code(0x50);
	copy_and_run();

	post_code(0x54);  /* Should never see this post code. */
}
