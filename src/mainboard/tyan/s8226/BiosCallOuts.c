/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2011 Advanced Micro Devices, Inc.
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

#include "agesawrapper.h"
#include "amdlib.h"
#include "BiosCallOuts.h"
#include "Ids.h"
#include "OptionsIds.h"
#include "heapManager.h"
#include <northbridge/amd/agesa/family15/dimmSpd.h>
#include <arch/io.h>

#ifdef __PRE_RAM__
/* These defines are used to select the appropriate socket for the SPD read
 * because this is a multi-socket design.
 */
#define PCI_REG_GPIO_48_47_46_37_CNTRL   (0xA6)
#define PCI_REG_GPIO_52_to_49_CNTRL      (0x50)
#define GPIO_OUT_BIT_GPIO48              (BIT3)
#define GPIO_OUT_BIT_GPIO49              (BIT0)
#define GPIO_OUT_ENABLE_BIT_GPIO48       (BIT7)
#define GPIO_OUT_ENABLE_BIT_GPIO49       (BIT4)

static UINT8 select_socket(UINT8 socket_id)
{
	device_t sm_dev       = PCI_DEV(0, 0x14, 0); //SMBUS
	UINT8    value        = 0;
	UINT8    gpio52_to_49 = 0;

	/* Configure GPIO49,48 to select the desired socket
	 * GPIO49,48 control the IDTQS3253 S1,S0
	 *  S1 S0 true table
	 *   0  0   channel 0
	 *   0  1   channel 1
	 *   1  0   channel 2   -  Socket 0
	 *   1  1   channel 3   -  Socket 1
	 * Note: Above is abstracted from Schematic. But actually it seems to be other way.
	 *   1  0   channel 2   -  Socket 1
	 *   1  1   channel 3   -  Socket 0
	 * Note: The DIMMs need to be plugged in from the farthest slot for each channel.
	 */
	gpio52_to_49 = pci_read_config8(sm_dev, PCI_REG_GPIO_52_to_49_CNTRL);
	value  = gpio52_to_49 | GPIO_OUT_BIT_GPIO49; // Output of GPIO49 is always forced to "1"
	value &= ~(GPIO_OUT_ENABLE_BIT_GPIO49); // 0=Output Enabled, 1=Tristate
	pci_write_config8(sm_dev, PCI_REG_GPIO_52_to_49_CNTRL, value);

	value  = pci_read_config8(sm_dev, PCI_REG_GPIO_48_47_46_37_CNTRL);
	value &= ~(GPIO_OUT_BIT_GPIO48);
	value |= (~(socket_id & 1)) << 3;  // Output of GPIO48 is inverse of SocketId
	value &= ~(GPIO_OUT_ENABLE_BIT_GPIO48); // 0=Output Enabled, 1=Tristate
	pci_write_config8(sm_dev, PCI_REG_GPIO_48_47_46_37_CNTRL, value);

	return gpio52_to_49;
}

static void restore_socket(UINT8 original_value)
{
	device_t sm_dev = PCI_DEV(0, 0x14, 0); //SMBUS
	pci_write_config8(sm_dev, PCI_REG_GPIO_52_to_49_CNTRL, original_value);

	// TODO: Restore previous GPIO48 configurations?
	//pci_write_config8(sm_dev, PCI_REG_GPIO_48_47_46_37_CNTRL, gpio48_47_46_37_save);
}
#endif

STATIC BIOS_CALLOUT_STRUCT BiosCallouts[] =
{
	{
		AGESA_ALLOCATE_BUFFER,
		BiosAllocateBuffer
	},

	{
		AGESA_DEALLOCATE_BUFFER,
		BiosDeallocateBuffer
	},

	{
		AGESA_DO_RESET,
		BiosReset
	},

	{
		AGESA_LOCATE_BUFFER,
		BiosLocateBuffer
	},

	{
		AGESA_READ_SPD,
		BiosReadSpd
	},

	{
		AGESA_READ_SPD_RECOVERY,
		BiosDefaultRet
	},

	{
		AGESA_RUNFUNC_ONAP,
		BiosRunFuncOnAp
	},

	{
		AGESA_GET_IDS_INIT_DATA,
		BiosGetIdsInitData
	},

	{
		AGESA_HOOKBEFORE_DQS_TRAINING,
		BiosHookBeforeDQSTraining
	},

	{
		AGESA_HOOKBEFORE_DRAM_INIT,
		BiosHookBeforeDramInit
	},
	{
		AGESA_HOOKBEFORE_EXIT_SELF_REF,
		BiosHookBeforeExitSelfRefresh
	},
};

AGESA_STATUS GetBiosCallout (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	UINTN i;
	AGESA_STATUS CalloutStatus;
	UINTN CallOutCount = sizeof (BiosCallouts) / sizeof (BiosCallouts [0]);

	for (i = 0; i < CallOutCount; i++) {
		if (BiosCallouts[i].CalloutName == Func) {
			break;
		}
	}

	if(i >= CallOutCount) {
		return AGESA_UNSUPPORTED;
	}

	CalloutStatus = BiosCallouts[i].CalloutPtr (Func, Data, ConfigPtr);

	return CalloutStatus;
}


CONST IDS_NV_ITEM IdsData[] =
{
	/*{
	  AGESA_IDS_NV_MAIN_PLL_CON,
	  0x1
	  },
	  {
	  AGESA_IDS_NV_MAIN_PLL_FID_EN,
	  0x1
	  },
	  {
	  AGESA_IDS_NV_MAIN_PLL_FID,
	  0x8
	  },

	  {
	  AGESA_IDS_NV_CUSTOM_NB_PSTATE,
	  },
	  {
	  AGESA_IDS_NV_CUSTOM_NB_P0_DIV_CTRL,
	  },
	  {
	  AGESA_IDS_NV_CUSTOM_NB_P1_DIV_CTRL,
	  },
	  {
	  AGESA_IDS_NV_FORCE_NB_PSTATE,
	  },
	 */
	{
		0xFFFF,
		0xFFFF
	}
};

#define   NUM_IDS_ENTRIES    (sizeof (IdsData) / sizeof (IDS_NV_ITEM))

AGESA_STATUS BiosGetIdsInitData (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	UINTN   i;
	IDS_NV_ITEM *IdsPtr;

	IdsPtr = ((IDS_CALLOUT_STRUCT *) ConfigPtr)->IdsNvPtr;

	if (Data == IDS_CALLOUT_INIT) {
		for (i = 0; i < NUM_IDS_ENTRIES; i++) {
			IdsPtr[i].IdsNvValue = IdsData[i].IdsNvValue;
			IdsPtr[i].IdsNvId = IdsData[i].IdsNvId;
		}
	}
	return AGESA_SUCCESS;
}


AGESA_STATUS BiosReadSpd (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	AGESA_STATUS Status;
#ifdef __PRE_RAM__
	UINT8 original_value = 0;

	if (ConfigPtr == NULL)
		return AGESA_ERROR;

	original_value = select_socket(((AGESA_READ_SPD_PARAMS *)ConfigPtr)->SocketId);

	Status = agesa_ReadSPD (Func, Data, ConfigPtr);

	restore_socket(original_value);
#else
	Status = AGESA_UNSUPPORTED;
#endif

	return Status;
}

/*  Call the host environment interface to provide a user hook opportunity. */
AGESA_STATUS BiosHookBeforeDQSTraining (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	return AGESA_SUCCESS;
}

/*  Call the host environment interface to provide a user hook opportunity. */
AGESA_STATUS BiosHookBeforeDramInit (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	return AGESA_SUCCESS;
}

/*  Call the host environment interface to provide a user hook opportunity. */
AGESA_STATUS BiosHookBeforeExitSelfRefresh (UINT32 Func, UINT32 Data, VOID *ConfigPtr)
{
	return AGESA_SUCCESS;
}

