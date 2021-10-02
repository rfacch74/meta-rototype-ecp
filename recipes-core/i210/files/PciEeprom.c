/***********************************************************************************
* Copyright 2017-2019 Intel Corporation All Rights Reserved.
*
* This EEPROM Access Tool example code ("Software") is furnished under license
* and may only be used or copied in accordance with the terms of that license.
* No license, express or implied, by estoppel or otherwise, to any
* intellectual property rights is granted by this document. The Software
* is subject to change without notice, and should not be construed as a
* commitment by Intel Corporation to market, license, sell or support
* any product or technology. Unless otherwise provided for in the license
* under which this Software is provided, the Software is provided AS IS,
* with no warranties of any kind, express or implied. Except as expressly
* permitted by the Software license, neither Intel Corporation nor its
* suppliers assumes any responsibility or liability for any errors or
* inaccuracies that may appear herein. Except as expressly permitted
* by the Software license, no part of the Software may be reproduced,
* stored in a retrieval system, transmitted in any form, or distributed
* by any means without the express written consent of Intel Corporation.
*
* The source code, information and material ("Material") contained herein
* is owned by Intel Corporation or its suppliers or licensors, and
* title to such Material remains with Intel Corporation or its suppliers
* or licensors. The Material contains proprietary information of Intel or
* its suppliers and licensors. The Material is protected by worldwide copyright
* laws and treaty provisions. No part of the Material may be used, copied,
* reproduced, modified, published, uploaded, posted, transmitted, distributed
* or disclosed in any way without Intel's prior express written permission.
* No license under any patent, copyright or other intellectual property rights
* in the Material is granted to or conferred upon you, either expressly, by
* implication, inducement, estoppel or otherwise. Any license under such
* intellectual property rights must be express and approved by Intel in writing.
*
* Third Party trademarks are the property of their respective owners.
*
* Unless otherwise agreed by Intel in writing, you may not remove or alter this
* notice or any other notice embedded in Materials by Intel or Intel’s
* suppliers or licensors in any way.”
********************************************************************************* */


/*******************************************************************************
**  Includes
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>
#include "include/common.h"
#include "include/helpercode.h"
#include "include/HelperFunctions.h"
#include "include/PciEeprom.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#ifdef USE_IO_MODE
#include <sys/io.h>
#endif
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/if_ether.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <bits/sockaddr.h>
#include <linux/in.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <endian.h>
#include <stdbool.h>

#define _Bool char

#ifndef bool
#define bool _Bool
#define true 1
#define false 0
#endif

#define X550_CPU_TO_LE32(_i) cpu_to_le32(_i)
//#define EAT_CPU_TO_LE32(_i) cpu_to_le32(_i)
#define EAT_CPU_TO_LE32(_i) (_i)
#define MEMORY_ACCESS  1
#define IO_ACCESS      0
#define DEVICE_LOCATION  "/proc/bus/pci"
#define COMMAND_STATUS  1
#define BUS_MASTER 0x0004
#define MEM_SPACE  0x0002
#define IO_SPACE   0x0001

static u64 BAR_ADDR_MASK = ~0xF;
extern u16 ExitStatus;
extern u8 MacAddress[12];

void CheckCommand(u16 LocationUnparsed);

extern s32 BitBangFlashEraseI210(struct PciDevice * dev);
extern s32 BitBangFlashSectorEraseI210(struct PciDevice * dev, u32 ByteIndex);
extern s32 EraseNvm(struct PciDevice * dev);
extern s32 SectorEraseNvm(struct PciDevice * dev, u32 ByteIndex);
extern s32 FlswFlashEraseI210(struct PciDevice * dev);
extern s32 GenericBitBangFlashWriteI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
extern s32 SstBitBangFlashWriteI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
extern s32 GenericBitBangFlashReadI210(struct PciDevice * dev, u32 ByteIndex, u16 NumWords, u16 * Value);
extern u16 SerialGetFlashIdJedec(struct PciDevice * dev);
extern s32 SerialFlashWriteStatusRegister(struct PciDevice * dev, bool Enable, bool Generic);
/*******************************************************************************
**
**  Name:           BrandingString2
**
**  Description:    String array that indexes silicon info
**
*******************************************************************************/
char * MemTypes[] =
{
	"FLASH",
	"INVM",
	"INVM+FLASH",
	"UNKNOWN"
};
#define MEMTYPE_UNKNOWN (sizeof(MemTypes) / sizeof(MemTypes[0]))


/*******************************************************************************
**
**  Name:           BrandingString2
**
**  Description:    String array that indexes silicon info
**
*******************************************************************************/
char * BrandingString2[] =
{
	"I210",
	"I211",
	"82574",
	"X550",
	"X553"
};


/*******************************************************************************
**
**  Name:           GetDevice()
**
**  Description:    Finds the PCI device to examine
**
**  Arguments:      DeviceInfo -  IN:  Device to populate
**                                OUT: Populated device
**
*******************************************************************************/
void GetDevice(struct PciDevice* DeviceInfo)
{
	/* NOTE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	 * Use caution when variables to this list!  The OverflowCheck
	 * token is placed immediately after the temp[] to detect overflow.  It
	 * would be a good idea to place additional variables at the end of
	 * this list so as to not disturb this overflow detection method
	 * */

	FILE * devices = NULL;                                      // file pointer for PCI devices
	c8 temp[512];                                                               // temp array
	u32 OverflowCheck = OVERFLOW_TOKEN;
	s16 FileSize = sizeof(temp) - 1;                            // size of PCI device file
	u16 DeviceCount = 0;
	u16 DeviceFound = 0;                                                // flag to be set when we find the device
	u32 VenDevInfo = 0;

	// check for overflow
	if(snprintf(temp, sizeof(temp), "%s", PROC_DEVICES) == sizeof(temp))
	{
		DisplayError(9);
	}

	// open the file that lists the devices connected to the PCI bus
	devices = fopen(temp, "r");

	// check if the file was actually opened or not
	if(NULL == devices)
	{
		DisplayError(10);
		return;
	}

	// iterate through the device list until we find our device of interest
	while(devices && (NULL != fgets(temp, FileSize, devices))
	      && (0 == DeviceFound))
	{
		sscanf(temp, "%hx %x %hx %llx %llx %llx %llx %llx %llx", \
		       &(DeviceInfo->LocationUnparsed), &VenDevInfo, \
		       &(DeviceInfo->Unused),&(DeviceInfo->BaseRegAddress[0]), \
		       &(DeviceInfo->BaseRegAddress[1]),&(DeviceInfo->BaseRegAddress[2]), \
		       &(DeviceInfo->BaseRegAddress[3]),&(DeviceInfo->BaseRegAddress[4]), \
		       &(DeviceInfo->BaseRegAddress[5]));

		// extract vendor and device info from the VenDevInfo word
		DeviceInfo->DeviceId = VenDevInfo & BITMASK;
		DeviceInfo->VendorId = VenDevInfo >> 16U;

		// extract bus and device info from the Location word
		DeviceInfo->BusNum = DeviceInfo->LocationUnparsed >> 8U;
		DeviceInfo->DeviceNum = DeviceInfo->LocationUnparsed & DEV_BITS;
		DeviceInfo->FunctionNum = DeviceInfo->LocationUnparsed & FUN_BITS;

		DeviceInfo->Memory1Address = ((DeviceInfo->BaseRegAddress[0]) & BAR_ADDR_MASK);
		if(DeviceInfo->BaseRegAddress[3] != 0)
			DeviceInfo->Memory2Address = ((DeviceInfo->BaseRegAddress[3]) & BAR_ADDR_MASK);
		else
			DeviceInfo->Memory2Address = ((DeviceInfo->BaseRegAddress[4]) & BAR_ADDR_MASK);

		// uncomment the line below to see all devices as they are discovered
		//printf("Device ID %4x DeviceInfo:  %llx\n", DeviceInfo->DeviceId, DeviceInfo->Memory1Address);

		// if we've found the device with our vendor ID and the
		// corresponding device ID
		if((INTEL_ID == DeviceInfo->VendorId))
		{

			switch(DeviceInfo->DeviceId)
			{
				case INTEL_82574:
				case INTEL_82574_GIG:
				case INTEL_82574_BLANK:
					++DeviceCount;
					break;

				case INTEL_I210_AT_OEM:
				case INTEL_I210_AT_EMP:
				case INTEL_I210_BLANK:
				case INTEL_I210_AS_FIB:
				case INTEL_I210_AS_1:
				case INTEL_I210_AS_2:
				case INTEL_I210_AT_RET:
				case INTEL_I210_AS_OEM:
				case INTEL_I210_SGMII_FLASHLESS:
					++DeviceCount;
					break;

				case INTEL_I211:
					++DeviceCount;
					break;

				case INTEL_I211_BLANK:
					++DeviceCount;
					break;

				case INTEL_X550_OEM:
				case INTEL_X550_VF1:
				case INTEL_X550_VF2:
				case INTEL_X550_T1:
					++DeviceCount;
					break;

				case INTEL_X553_SGMII_BACKPLANE1:
				case INTEL_X553_SGMII_BACKPLANE2:
				case INTEL_X553_KX:
				case INTEL_X553_SGMII_1:
				case INTEL_X553_SGMII_2:
					++DeviceCount;
					break;
			}

			if((DeviceCount != 0) && (DeviceCount == DeviceInfo->NicId))
			{
				// map the device into memory
				CheckCommand(DeviceInfo->LocationUnparsed);
				DeviceFound = 1;
				MapPciDevice(DeviceInfo);
			}

		}

		InitNvmParamsNull(DeviceInfo);
		//Setup the function pointers for the rest of the code to work.
		switch(DeviceInfo->DeviceId)
		{
			case INTEL_82574:
			case INTEL_82574_GIG:
			case INTEL_82574_BLANK:
				InitNvmParams82574(DeviceInfo);
				break;

			case INTEL_I210_AT_OEM:
			case INTEL_I210_AT_EMP:
			case INTEL_I210_BLANK:
			case INTEL_I210_AS_FIB:
			case INTEL_I210_AS_1:
			case INTEL_I210_AS_2:
			case INTEL_I210_AT_RET:
			case INTEL_I210_AS_OEM:
			case INTEL_I210_SGMII_FLASHLESS:
				InitNvmParamsI210(DeviceInfo);
				break;

			case INTEL_I211_BLANK:
			case INTEL_I211:
				InitNvmParamsI211(DeviceInfo);
				break;

			case INTEL_X550_OEM:
			case INTEL_X550_VF1:
			case INTEL_X550_VF2:
			case INTEL_X550_T1:
				InitNvmParamsX550(DeviceInfo);
				break;

			case INTEL_X553_SGMII_BACKPLANE1:
			case INTEL_X553_SGMII_BACKPLANE2:
			case INTEL_X553_KX:
			case INTEL_X553_SGMII_1:
			case INTEL_X553_SGMII_2:
				InitNvmParamsX553(DeviceInfo);
				break;
		}
	}

	// See if we overflowed our temp buffer
	if(OVERFLOW_TOKEN != OverflowCheck)
	{
		printf("A temporary buffer overflowed.\n");
	}

	fclose(devices);
}
/*******************************************************************************
**
**  Name:           CheckCommand()
**
**  Description:    Checks the PCI Command register to make sure it's set okay.
**
**  Arguments:      LocationUnparsed -  IN:  Device location to check
**
*******************************************************************************/

void CheckCommand(u16 LocationUnparsed)
{
	FILE * devices = NULL;                                      // file pointer for PCI devices
	c8 temp[256];                                                       // temp array
	u32 OverflowCheck = OVERFLOW_TOKEN;    //this always has to follow the temp buffer for checking
	u32 Config[17];
	s16 FileSize = sizeof(Config) - 1;                  // size of PCI device file
	u8 bus;
	u8 device;
	u8 function;
	u16 iomode = 0;

	Config[16] = '\0';
	// configuration space seems to be at PROC/DEVICES/bus#/slot.function
	// so /PROC_DEVICE/07/00.0 would be bus 7 device 0 func 0 device
	// /proc/bus/pci/07/00.0 should be where we end up.

	bus = (LocationUnparsed >> 8);
	device = ((LocationUnparsed & 0x0f0) >> 4);
	function = (LocationUnparsed & 0x00f);

	// open the file that lists the devices connected to the PCI bus
	// some systems may have it in a different place, so check both
	if(snprintf(temp, (size_t) sizeof(temp), "%s/0000:%.2x/%.2x.%x", DEVICE_LOCATION,bus,device, function) == sizeof(temp))
	{
		printf("Check Command error %s\n",temp);
	}
	else
	{
		//test if the first one worked, if not use the second method and let the rest of the code see if that works
		devices = fopen(temp, "r+");
		if(NULL == devices)
		{
			if(snprintf(temp, (size_t) sizeof(temp), "%s/%.2x/%.2x.%x", DEVICE_LOCATION,bus,device, function) == sizeof(temp))
			{
				printf("check command error second check\n");
			}
			else
			{
				devices = fopen(temp, "r+");
			}
		}
	}

	// check if the file was actually opened or not
	if(NULL == devices)
	{
		printf("Check Command open to write error\n");
	}
	else
	{
		// load up the PCI ocnfiguration space and check it out.
		if(NULL != fgets((char *) &Config[0], FileSize, devices))
		{
			if(Config[4] || Config[5])
			{
				//debug prints removed but this is a good place if your having troubles.
			}
			else
			{
				printf("No Memeory BAR resources supplied by OS/BIOS.\nTool will not work correctly without it.\n");
				//put use the IO mode stuff here if you want to use it.
			}

			if(Config[6] & IO_SPACE)
			{
				//Debug prints removed but this is a good place to put one.
				iomode = 1;
			}
			if((Config[COMMAND_STATUS] & BUS_MASTER) && (Config[COMMAND_STATUS] & MEM_SPACE))
			{
				//Debug code goes here.
				//printf("busmaster and memory set on \n");
			}
			else
			{
				printf("Updating command word...");
				rewind(devices);
				Config[COMMAND_STATUS] = Config[COMMAND_STATUS] | 0x06 | iomode;
				fputs((char *) &Config[0], devices);
				printf(".. Done!\n");
			}
		}
		fclose(devices);
	}
	// See if we overflowed our temp buffer
	if(OVERFLOW_TOKEN != OverflowCheck)
	{
		printf("A temporary buffer overflowed in CheckCommand.\n");
	}
}

/*******************************************************************************
**
**  Name:           DisplayAllDevices()
**
**  Description:    Displays supported INTEL adapters found on the system
**
*******************************************************************************/
void DisplayAllDevices()
{
	FILE * devices = NULL;                                      // file pointer for PCI devices
	c8 temp[512];                                                       // temp array
	u32 OverflowCheck = OVERFLOW_TOKEN;
	u16 i = 1;
	s16 FileSize = sizeof(temp) - 1;            // size of PCI device file
	u16 LocationUnparsed = 0;
	u16 Unused;
	u16 DeviceId;
	u16 VendorId;
	u16 BusNum;
	u16 DeviceNum;
	u16 FunctionNum;
	u32 VenDevInfo = 0;
	unsigned long long int BaseRegAddress[6];
	struct PciDevice dev;
	PciDevice(&dev);            // to check if this works for other than DNV

	// check for overflow
	if(snprintf(temp, (size_t) sizeof(temp), "%s", PROC_DEVICES) == sizeof(temp))
	{
		DisplayError(9);
	}

	// open the file that lists the devices connected to the PCI bus
	devices = fopen(temp, "r");

	// check if the file was actually opened or not
	if(NULL == devices)
	{
		DisplayError(10);
	}
	else
	{
		// iterate through the device list until we find our device of interest
		while(devices && (NULL != fgets(temp, FileSize, devices)))
		{
			sscanf(temp, "%hx %x %hx %llx %llx %llx %llx %llx %llx", \
			       &LocationUnparsed, &VenDevInfo, \
			       &Unused,&(BaseRegAddress[0]), \
			       &(BaseRegAddress[1]),&(BaseRegAddress[2]), \
			       &(BaseRegAddress[3]),&(BaseRegAddress[4]), \
			       &(BaseRegAddress[5]));

			// extract vendor and device info from the VenDevInfo word
			DeviceId = VenDevInfo & BITMASK;
			VendorId = VenDevInfo >> 16U;

			// extract bus and device info from the Location word
			BusNum = LocationUnparsed >> 8U;
			DeviceNum = LocationUnparsed & DEV_BITS;
			FunctionNum = LocationUnparsed & FUN_BITS;

			dev.Memory1Address = BaseRegAddress[0] & BAR_ADDR_MASK;
			if(BaseRegAddress[3] != 0)
				dev.Memory2Address = BaseRegAddress[3] & BAR_ADDR_MASK;
			else
				dev.Memory2Address = BaseRegAddress[4] & BAR_ADDR_MASK;
			dev.DeviceId = DeviceId;
			//dev.NicId = i;

			// print info to the screen
			if(INTEL_ID == VendorId)
			{
				// there can be numerous device ID's that describe the same hardware
				switch(DeviceId)
				{
					case INTEL_82574:
					case INTEL_82574_GIG:
					case INTEL_82574_BLANK:
					case INTEL_I210_AT_OEM:
					case INTEL_I210_AT_EMP:
					case INTEL_I210_BLANK:
					case INTEL_I210_AS_FIB:
					case INTEL_I210_AS_1:
					case INTEL_I210_AS_2:
					case INTEL_I210_AT_RET:
					case INTEL_I210_AS_OEM:
					case INTEL_I210_SGMII_FLASHLESS:
					case INTEL_I211_BLANK:
					case INTEL_I211:
					case INTEL_X550_OEM:
					case INTEL_X550_VF1:
					case INTEL_X550_VF2:
					case INTEL_X550_T1:
					case INTEL_X553_SGMII_BACKPLANE1:
					case INTEL_X553_SGMII_BACKPLANE2:
					case INTEL_X553_KX:
					case INTEL_X553_SGMII_1:
					case INTEL_X553_SGMII_2:
						CheckCommand(LocationUnparsed);
						MapPciDevice(&dev);
						DetermineNvmType(&dev);
						printf(" %d	%d	 %d	 %d	%s	     %s\n", i, BusNum, DeviceNum, FunctionNum, BrandingString2[(u8) (u8) SiliconName(DeviceId)], MemTypes[(u8) (dev.MemType)]);
						++i;

						if(1 == dev.IsMapped)
						{
							UnmapDevice(&dev);
						}
						break;
				}
			}
		}
		fclose(devices);
	}

	// See if we overflowed our temp buffer
	if(OVERFLOW_TOKEN != OverflowCheck)
	{
		printf("A temporary buffer overflowed.\n");
	}

}


/*******************************************************************************
**
**  Name:           DetermineNvmType()
**
**  Description:    Determines whether an iNVM or an NVM is present in the
**		    system.
**
**  Arguments:      dev -  IN:  Device to determine device type of
**
*******************************************************************************/
void DetermineNvmType(struct PciDevice * dev)
{

	u32 FlashPresent = 0;

	if(dev && dev->IsMapped != 0)
	{

		switch(dev->DeviceId)
		{
			case INTEL_82574:
			case INTEL_82574_GIG:
			case INTEL_82574_BLANK:
				dev->NvmPresent = 1;
				dev->InvmPresent = 0;
				dev->MemType = 0x0;
				break;

			case INTEL_I210_AT_OEM:
			case INTEL_I210_AT_EMP:
			case INTEL_I210_BLANK:
			case INTEL_I210_AS_FIB:
			case INTEL_I210_AS_1:
			case INTEL_I210_AS_2:
			case INTEL_I210_AT_RET:
			case INTEL_I210_AS_OEM:
			case INTEL_I210_SGMII_FLASHLESS:
			case INTEL_I211_BLANK:
			case INTEL_I211:
				FlashPresent = (ReadReg(dev, EECD) & EECTRL_FLASH_PRESENT);

				if(FlashPresent == 0)
				{
					dev->InvmPresent = 1;
					dev->NvmPresent = 0;
					dev->MemType = 0x1;
				}
				else
				{
					dev->NvmPresent = 1;
					dev->InvmPresent = 1; // there IS an iNVM present
					dev->MemType = 0x2;
				}
				break;

			case INTEL_X550_OEM:
			case INTEL_X550_VF1:
			case INTEL_X550_VF2:
			case INTEL_X550_T1:
				dev->NvmPresentX550 = 1;
				dev->InvmPresent = 0;
				dev->NvmPresent = 0;
				dev->MemType = 0x0;
				break;

			case INTEL_X553_SGMII_BACKPLANE1:
			case INTEL_X553_SGMII_BACKPLANE2:
			case INTEL_X553_KX:
			case INTEL_X553_SGMII_1:
			case INTEL_X553_SGMII_2:
				dev->NvmPresentX550 = 1;
				dev->InvmPresent = 0;
				dev->NvmPresent = 0;
				dev->MemType = 0x0;
				// FlashPresent = 2;
				break;
		}
	}
}


/*******************************************************************************
**
**  Name:           PciDevice()
**
**  Description:    Initializes member values for the given device
**
**  Arguments:      dev -  IN:  Device to initialize
**
*******************************************************************************/
void PciDevice(struct PciDevice * dev)
{
	dev->NicId = 0;
	dev->IsMapped = 0;
	dev->InvmPresent = 0;
	dev->NvmPresent = 0;
	dev->MemType = MEMTYPE_UNKNOWN;
}


/*******************************************************************************
**
**  Name:           CloseDevice()
**
**  Description:    Shuts down the PCI device
**
**  Arguments:      DeviceInfo -  IN:  Device to close
**
*******************************************************************************/
void CloseDevice(struct PciDevice* DeviceInfo)
{
	// make sure the device was mapped
	if(DeviceInfo)
	{
		// unmap the device from userspace when we're done with it
		if(1 == DeviceInfo->IsMapped)
		{
			UnmapDevice(DeviceInfo);
		}
	}
}


/*******************************************************************************
**
**  Name:           MapPciDevice()
**
**  Description:    Maps a hardware device into memory
**
**  Arguments:      Dev    - Device to map to memory
**
**  Returns:        Address of mapped device
**
*******************************************************************************/
void* MapPciDevice(struct PciDevice* Dev)
{
	off_t WordLocation = 0;                         // used for parsing
	s16 FileDescriptor = -1;                    // default to error condition
	void* Mapped1Address = NULL;                // mapped bars
	void* Mapped2Address = NULL;
	c8 FileLocation[75] = "/dev/mem";           // where we map to
	struct stat sb;

	// try open the PCI resource file for reading/writing
	if(access(FileLocation, F_OK) == -1)
	{
		printf("Run as a root user !! Error at line %d in file %s (errno: %d, description:  %s)\n", \
		       __LINE__, __FILE__, errno, strerror(errno));
	}
	else
	{
		// open the file
		FileDescriptor = open(FileLocation, O_RDWR | O_SYNC);

		// check for errors opening file
		if(-1 == FileDescriptor)
		{
			printf("Open file failed.  Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
		}
		else
		{
			WordLocation = (off_t) (Dev->Memory1Address);

			WordLocation &= ~(sysconf(_SC_PAGESIZE) - 1);

			// check to make sure the offset isn't past the end of the file
			if(-1 == fstat(FileDescriptor, &sb))
			{
				printf("fstat error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
			}

			/* create a memory mapping so we can map the device resource file
			 * into user space memory
			 * multiply by pagesize to get the amount of mapped memory in units of pagesize */
			Mapped1Address = mmap(NULL, 4096 * 32, PROT_WRITE | PROT_READ, MAP_SHARED, FileDescriptor, (off_t) WordLocation);
			//printf("M1A = %llX\n",(u64) Mapped1Address);

			// make sure the above call to mmap didn't return an error
			if(((void *) -1) == Mapped1Address)
			{
				printf("Make sure the current driver is unloaded !!!\n");
				Mapped1Address = NULL;
				Mapped2Address = NULL;
			}
			else
			{
				// set device is mapped flag to true
				Dev->IsMapped = 1;
				Dev->AccessMode = MEMORY_ACCESS;
			}
			//Now to map the Flash for devices that require it.
			WordLocation = (off_t) (Dev->Memory2Address);

			WordLocation &= ~(sysconf(_SC_PAGESIZE) - 1);

			// check to make sure the offset isn't past the end of the file
			if(-1 == fstat(FileDescriptor, &sb))
			{
				printf("fstat error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
			}

			/* create a memory mapping so we can map the device resource file
			 * into user space memory
			 * multiply by pagesize to get the amount of mapped memory in units of pagesize */
			Mapped2Address = mmap(NULL, 4096 * 32, PROT_WRITE | PROT_READ, MAP_SHARED, FileDescriptor, (off_t) WordLocation);

			//printf("M2A %llx = %llX\n", Dev->Memory2Address, (u64) Mapped2Address);
			// make sure the above call to mmap didn't return an error
			if(((void *) -1) == Mapped2Address)
			{
				printf("Memory 2 could not be mapped\n");
				//printf(Error at line %d, file %s (%d) [%s]\n", __LINE__, __FILE__, errno, strerror(errno));
				Mapped2Address = NULL;
			}
		}

		// set other device parameters
		Dev->FileDescriptor = FileDescriptor;
		if(FileDescriptor != -1)
			close(FileDescriptor);
		Dev->Mapped1Address = Mapped1Address;
		Dev->Mapped2Address = Mapped2Address;
	}

	return Mapped1Address;
}


/*******************************************************************************
**
**  Name:           WriteReg()
**
**  Description:    Writes a given value to a given register
**
**  Arguments:      Dev    - Device to write to
**		    Offset - Offset to write to
**		    Data   - Data to write
**
**  Returns:        Pass or fail flag
**
*******************************************************************************/
s32 WriteReg(struct PciDevice * Dev, u32 Offset, u32 Data)
{
	void* Reg;

	if(Dev)
	{
		if(Dev->Mapped1Address)
		{
			Reg = (void*) (Dev->Mapped1Address + Offset);
			*((u32 *) Reg) = Data;
			return SUCCESS;
		}

#ifdef USE_IO_MODE
		if(Dev->MemoryIOAddress)
		{
			outl(Dev->MemoryIOAddress, Offset);
			outl(Dev->MemoryIOAddress + 4, Data);
			return SUCCESS;
		}
#endif
	}

	printf("Error writing to device memory\n");
	return FAIL;
}


/*******************************************************************************
**
**  Name:           ReadReg()
**
**  Description:    Reads a given value from a given register
**
**  Arguments:      Dev    - Device to read from
**		    Offset - Offset to read from
**
**  Returns:        Data or fail flag, will print error if failure
**
*******************************************************************************/
u32 ReadReg(struct PciDevice * Dev, u32 Offset)
{
	void* Reg;

	if(Dev)
	{
		if(Dev->Mapped1Address)
		{
			Reg = (void*) (Dev->Mapped1Address + Offset);
			return ((u32) * ((u32 *) Reg));
		}
#ifdef USE_IO_MODE
		if(Dev->MemoryIOAddress)
		{
			outl(Dev->MemoryIOAddress, Offset);
			return(inl(Dev->MemoryIOAddress + 4));
		}
#endif
	}

	printf("Error reading from device memory.  Programming will not happen.\n");
	return FAIL;
}


/*******************************************************************************
**
**  Name:           UnmapDevice()
**
**  Description:    Unmaps hardware device from memory
**
**  Arguments:      Dev - Device to unmap
**
*******************************************************************************/
void UnmapDevice(struct PciDevice* Dev)
{
	s32 Ret = FAIL;

	//Make sure we don't have a null pointer
	if(NULL != Dev)
	{
		// unmap the address provided
		Ret = munmap(Dev->Mapped1Address, MAP_SIZE);
		Ret |= munmap(Dev->Mapped2Address, MAP_SIZE);

		// make sure the above call to munmap didn't return an error
		// Any value other than 0 is a fail value
		if(Ret)
		{
			printf("unmap memory failed.  Error at line %d, file %s (%d) [%s]\n", \
			       __LINE__, __FILE__, errno, strerror(errno));
		}
		else
		{
			Dev->Mapped1Address = NULL;
			Dev->Mapped2Address = NULL;
			Dev->IsMapped = 0;
		}
	}
}


/*******************************************************************************
**
**  Name:           AcquireNvmX550()
**
**  Description:    Acquires the HW semaphore
**
**  Arguments:      dev    - Device to get the semaphore for
**
**  Returns:        SUCCESS if we got the semaphore, FAIL otherwise
**
*******************************************************************************/
s32 AcquireNvmX550(struct PciDevice * dev, u32 mask)
{
	u32 SyncVal = 0;
	u32 SwCtrlMask = SWFW_EEP_SM_X550 & SWFW_NVM_PHY_MASK_X550;
	u32 FwCtrlMask = SwCtrlMask << 5;
	u32 HwCtrlMask = 0;
	s32 retval = SUCCESS;
	u32 i = 0;
	u32 MaxAttempts = 200;

	//printf("Entering the function %s\n",__FUNCTION__);

	if(SwCtrlMask & SWFW_EEP_SM_X550)
		HwCtrlMask |= SWFW_FLASH_SM_X550;

	/* SW only mask doesn't have FW bit pair */
	if(mask & SWFW_SW_MNG_SM_X550)
		SwCtrlMask |= SWFW_SW_MNG_SM_X550;

	// Try to get the FW semaphore.
	// staying within the timeout boundary
	for(i = 0; i < MaxAttempts; i++)
	{
		/* SW NVM semaphore bit is used for access to all
		 * SW_FW_SYNC bits (not just NVM)
		 */
		if(GetHwSemaphoreX550(dev))
		{
			retval = FAIL;
			return retval;
		}

		// read the sync value from the register
		SyncVal = ReadReg(dev, SW_FW_SYNC_X550);

		if(!(SyncVal & (FwCtrlMask | SwCtrlMask | HwCtrlMask)))
		{
			SyncVal |= SwCtrlMask;
			WriteReg(dev, SW_FW_SYNC_X550, SyncVal);

			PutHwSemaphoreX550(dev);
			usleep(50);
			return retval;
		}
		/* Firmware currently using resource (FwCtrlMask), hardware
		 * currently using resource (HwCtrlMask), or other software
		 * thread currently using resource (SwCtrlMask)
		 */
		PutHwSemaphoreX550(dev);
		usleep(50);
	}
	// check to see if we timed out, if so print an error and return
	if(i == MaxAttempts)
	{
		retval = FAIL;
		return retval;
	}

	PutHwSemaphoreX550(dev);

	return retval;
}

/*******************************************************************************
**
**  Name:           Isx553()
**
**  Description:    Use the PCI device to figure out if it is x553 or not.
**
**  Arguments:      DeviceInfo -  IN:  Device to populate
**
*******************************************************************************/
bool IsX553(struct PciDevice * dev)
{
	return ((dev != NULL) && ((dev->DeviceId == INTEL_X553_SGMII_BACKPLANE1) ||
	                          (dev->DeviceId == INTEL_X553_SGMII_BACKPLANE2) ||
	                          (dev->DeviceId == INTEL_X553_KX) ||
	                          (dev->DeviceId == INTEL_X553_SGMII_1) ||
	                          (dev->DeviceId == INTEL_X553_SGMII_2)));
}

/*******************************************************************************
**
**  Name:           AcquireNvmX553()
**
**  Description:    Acquires the HW semaphore
**
**  Arguments:      dev    - Device to get the semaphore for
**                  mask   -
**
**  Returns:        SUCCESS if we got the semaphore, FAIL otherwise
**
*******************************************************************************/
s32 AcquireNvmX553(struct PciDevice * dev, u32 mask)
{
	u32 SyncVal = 0;
	u32 SwCtrlMask = mask & SWFW_NVM_PHY_MASK_X550;
	u32 FwCtrlMask = SwCtrlMask << 5;
	u32 HwCtrlMask = 0;
	s32 retval = SUCCESS;
	u32 i = 0;
	u32 MaxAttempts = 1000;
	u32 rmask = 0;
	u32 swi2c_mask = mask & GSSR_I2C_MASK_X553;

	// printf("Entering the function %s\n",__FUNCTION__);

	if(SwCtrlMask & SWFW_EEP_SM_X550)
		HwCtrlMask |= SWFW_FLASH_SM_X550;

	/* SW only mask doesn't have FW bit pair */
	if(mask & SWFW_SW_MNG_SM_X550)
		SwCtrlMask |= SWFW_SW_MNG_SM_X550;

	SwCtrlMask |= swi2c_mask;
	FwCtrlMask |= swi2c_mask << 2;

	// Try to get the FW semaphore.
	// staying within the timeout boundary
	for(i = 0; i < MaxAttempts; i++)
	{
		/* SW NVM semaphore bit is used for access to all
		 * SW_FW_SYNC bits (not just NVM)
		 */
		if(GetHwSemaphoreX553(dev))
		{
			retval = FAIL;
			return retval;
		}

		// read the sync value from the register
		SyncVal = ReadReg(dev, SW_FW_SYNC_X553);

		if(!(SyncVal & (FwCtrlMask | SwCtrlMask | HwCtrlMask)))
		{
			SyncVal |= SwCtrlMask;
			WriteReg(dev, SW_FW_SYNC_X553, SyncVal);

			PutHwSemaphoreX553(dev);
			usleep(50);
			return retval;
		}
		/* Firmware currently using resource (FwCtrlMask), hardware
		 * currently using resource (HwCtrlMask), or other software
		 * thread currently using resource (SwCtrlMask)
		 */
		PutHwSemaphoreX553(dev);
		usleep(50);
	}

	/* Failed to get SW only Semaphore */
	if(SwCtrlMask == SWFW_SW_MNG_SM_X550)
	{
		retval = FAIL;
		return retval;
	}

	/* If the resource is not released by the FW/HW the SW can assume that the
	 * FW/HW malfunctions. In that case the SW should set the SW bit(s) of the
	 * requested resources(s) while ignoring the corresponding FW/HW bits in the
	 * SW_FW_SYNC register.*/
	if(GetHwSemaphoreX553(dev))
	{
		retval = FAIL;
		return retval;
	}

	// read the sync value from the register
	SyncVal = ReadReg(dev, SW_FW_SYNC_X553);

	if(SyncVal & (FwCtrlMask | HwCtrlMask))
	{
		SyncVal |= SwCtrlMask;
		WriteReg(dev, SW_FW_SYNC_X553, SyncVal);
		PutHwSemaphoreX553(dev);
		usleep(50);
		return retval;
	}

	/* If the resource is not released by other SW the SW can assume that the
	 * other SW malfunctions. In that case the SW should clear all SW flags that
	 * it does not own and then repeat the whole process once again. */
	if(SyncVal & SwCtrlMask)
	{
		rmask = GSSR_EEP_SM_X553 | GSSR_PHY0_SM_X553 | GSSR_PHY1_SM_X553 |
		        GSSR_MAC_CSR_SM_X553;

		if(swi2c_mask)
		{
			rmask |= GSSR_I2C_MASK_X553;
		}

		PutHwSemaphoreX553(dev);
		// Supposed to release 540 sem as well. Not in this code. Put here. KT
		retval = FAIL;
		return retval;
	}

	PutHwSemaphoreX553(dev);

	return retval;
}


/*******************************************************************************
**
**  Name:           ReleaseNvmX550()
**
**  Description:    Clear the request bit for the eeprom, and release
**		    any semaphore that may have been aquired
**
**  Arguments:      dev - Device to release
**
*******************************************************************************/
void ReleaseNvmX550(struct PciDevice * dev, u32 mask)
{
	u32 SyncVal = 0; // a place to hold register data

	//printf("Entering the function %s\n",__FUNCTION__);
	while(GetHwSemaphoreX550(dev) != SUCCESS)
		; /* Empty */

	// Read the sync reg
	SyncVal = ReadReg(dev, SW_FW_SYNC_X550);

	// Clear the request bit
	SyncVal &= ~SWFW_EEP_SM_X550;

	// write that value back
	WriteReg(dev, SW_FW_SYNC_X550, SyncVal);

	// put hw semaphore
	PutHwSemaphoreX550(dev);
}


/*******************************************************************************
**
**  Name:           ReleaseNvmX553()
**
**  Description:    Clear the request bit for the eeprom, and release
**		    any semaphore that may have been aquired
**
**  Arguments:      dev  - Device to release
**                  mask -
**
*******************************************************************************/
void ReleaseNvmX553(struct PciDevice * dev, u32 mask)
{

	u32 swmask = mask & (SWFW_NVM_PHY_MASK_X550 | SWFW_SW_MNG_SM_X550);
	u32 SyncVal = 0; // a place to hold register data

	// printf("Entering the function %s\n",__FUNCTION__);
	while(GetHwSemaphoreX553(dev) != SUCCESS)
		; /* Empty */

	// Read the sync reg
	SyncVal = ReadReg(dev, SW_FW_SYNC_X553);

	// Clear the request bit
	SyncVal &= ~swmask;

	// write that value back
	WriteReg(dev, SW_FW_SYNC_X553, SyncVal);

	// put hw semaphore
	PutHwSemaphoreX553(dev);
}


/*******************************************************************************
**
**  Name:           GetHwSemaphoreX550()
**
**  Description:    Acquires the HW semaphore
**
**  Arguments:      dev    - Device to get the semaphore for
**
**  Returns:        SUCCESS if we got the semaphore, FAIL otherwise
**
*******************************************************************************/
s32 GetHwSemaphoreX550(struct PciDevice * dev)
{
	u32 SemVal = 0;                        // semaphore value
	s32 retval = SUCCESS;                  // return value
	s32 MaxAttempts = 2000;   // timeout
	s32 i = 0;                             // counter

	//printf("Entering the function %s\n",__FUNCTION__);

	/* Get SMBI software semaphore between device drivers first */
	for(i = 0; i < MaxAttempts; i++)
	{
		/*
		 * If the SMBI bit is 0 when we read it, then the bit will be
		 * set and we have the semaphore
		 */
		SemVal = ReadReg(dev, SWSM_X550);

		if(!(SemVal & SWSM_SMBI_X550))
		{
			break;
		}
		usleep(50);
	}

	/* Now get the semaphore between SW/FW through the REGSMP bit */
	for(i = 0; i < MaxAttempts; i++)
	{
		SemVal = ReadReg(dev, SWSM_X550);

		if(!(SemVal & SWFW_REGSMP_X550))
			break;

		usleep(50);
	}

	// check to see if we've timed out
	if(i == MaxAttempts)
	{
		// Release semaphores
		PutHwSemaphoreX550(dev);

		retval = FAIL;
		return retval;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           GetHwSemaphoreX553()
**
**  Description:    Acquires the HW semaphore
**
**  Arguments:      dev    - Device to get the semaphore for
**
**  Returns:        SUCCESS if we got the semaphore, FAIL otherwise
**
*******************************************************************************/
s32 GetHwSemaphoreX553(struct PciDevice * dev)
{
	u32 SemVal = 0;       // semaphore value
	s32 retval = SUCCESS; // return value
	s32 MaxAttempts = 2000; // timeout
	s32 i = 0;            // counter

	// printf("Entering the function %s\n",__FUNCTION__);

	/* Get SMBI software semaphore between device drivers first */
	for(i = 0; i < MaxAttempts; i++)
	{
		/*
		 * If the SMBI bit is 0 when we read it, then the bit will be
		 * set and we have the semaphore
		 */
		SemVal = ReadReg(dev, SWSM_X553);

		if(!(SemVal & SWSM_SMBI_X550))
		{
			break;
		}
		usleep(50);
	}

	/* Now get the semaphore between SW/FW through the REGSMP bit */
	for(i = 0; i < MaxAttempts; i++)
	{
		SemVal = ReadReg(dev, SW_FW_SYNC_X553);

		if(!(SemVal & SWFW_REGSMP_X550))
			break;

		usleep(50);
	}

	// check to see if we've timed out
	if(i >= MaxAttempts)
	{
		// Release semaphores
		PutHwSemaphoreX553(dev);

		retval = FAIL;
		return retval;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           PutHwSemaphoreX550()
**
**  Description:    Puts the HW semaphore
**
**  Arguments:      dev    - Device to put semaphore for
**
*******************************************************************************/
void PutHwSemaphoreX550(struct PciDevice * dev)
{
	u32 SemVal = 0;     // semaphore value

	//printf("Entering the function %s\n",__FUNCTION__);

	// Read the sync reg
	SemVal = ReadReg(dev, SW_FW_SYNC_X550);

	// write the bit
	SemVal &= ~SWFW_REGSMP_X550;

	// write the register
	WriteReg(dev, SW_FW_SYNC_X550, SemVal);

	// get the software semaphore value
	SemVal = ReadReg(dev, SWSM_X550);

	// write the bit
	SemVal &= ~SWSM_SMBI_X550;

	// write the register
	WriteReg(dev, SWSM_X550, SemVal);
}


/*******************************************************************************
**
**  Name:           PutHwSemaphoreX553()
**
**  Description:    Puts the HW semaphore
**
**  Arguments:      dev    - Device to put semaphore for
**
*******************************************************************************/
void PutHwSemaphoreX553(struct PciDevice * dev)
{
	u32 SemVal = 0; // semaphore value

	// printf("Entering the function %s\n",__FUNCTION__);

	// Read the sync reg
	SemVal = ReadReg(dev, SW_FW_SYNC_X553);

	// write the bit
	SemVal &= ~SWFW_REGSMP_X550;

	// write the register
	WriteReg(dev, SW_FW_SYNC_X553, SemVal);

	// get the software semaphore value
	SemVal = ReadReg(dev, SWSM_X553);

	// write the bit
	SemVal &= ~SWSM_SMBI_X550;

	// write the register
	WriteReg(dev, SWSM_X553, SemVal);
	// Need to do flush. Write Flush per driver KT
	SemVal = ReadReg(dev, STATUS_X553);
}


/*******************************************************************************
**
**  Name:           ReadNvmBufferX550()
**
**  Description:    Reads shadow ram register
**
**  Arguments:      dev       - Device
**		    Offset    - offset to read from
**		    NumWords  - number of words to read
**		    Data      - OUT: data read from register
**
**  Returns:        SUCCESS if we were able to read the shadow ram
**
*******************************************************************************/
s32 ReadNvmBufferX550(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	s32 retval;                          // return value
	u32 i = 0;                                   // counter
	u16 count = 0;                               // word count
	u32 value = 0;                               // current word
	struct NvmCommand buffer;

	//printf("Entering the function %s\n",__FUNCTION__);

	/* Take semaphore for the entire operation. */
	retval = dev->nvmops.acquire(dev, SWFW_EEP_SM_X550);
	if(retval != SUCCESS)
	{
		printf("acquire failed in read buffer\n");
		retval = FAIL;
		return retval;
	}

	while(NumWords)
	{
		if(NumWords > MAX_BUFFER_SIZE / 2)
			count = MAX_BUFFER_SIZE / 2;
		else
			count = NumWords;

		buffer.hdr.req.cmd = FW_READ_SHADOW_RAM_CMD;
		buffer.hdr.req.buf_lenh = 0;
		buffer.hdr.req.buf_lenl = FW_READ_SHADOW_RAM_LEN;
		buffer.hdr.req.checksum = FW_DEFAULT_CHECKSUM;

		/* convert offset from words to bytes */
		buffer.Address = htobe32((Offset + value) * 2);
		buffer.Length = htobe16(count * 2);

		retval = HostInterfaceCommandX550(dev, (u32 *) &buffer, (sizeof(buffer) / 2), COMMAND_TIMEOUT, false);

		if(retval != SUCCESS)
		{
			dev->nvmops.release(dev, SWFW_EEP_SM_X550);
			return retval;
		}

		for(i = 0; i < count; i++)
		{
			u32 reg = FLEX_MNG + (NVM_DATA_OFFSET << 2) + 2 * i;
			u32 ValueRead = ReadReg(dev, reg);

			Data[value] = (u16) (ValueRead & 0xffff);
			value++;
			i++;
			if(i < count)
			{
				ValueRead >>= 16;
				Data[value] = (u16) (ValueRead & 0xffff);
				value++;
			}
		}
		NumWords -= count;
	}
	dev->nvmops.release(dev, SWFW_EEP_SM_X550);
	return retval;
}


/*******************************************************************************
**
**  Name:           WriteNvmBuffer550()
**
**  Description:    Writes value to shadow ram with the given data to the
**		    specified offset.  Update checksum should be done after
**		    a call here.  Also, it is possible that if an error
**		    condition is returned, data has been partially written.
**
**  Arguments:      dev       - Device
**		    Offset    - offset to write to
**		    NumWords  - number of words to write
**		    Data      - data to write, can be a buffer
**
**  Returns:        SUCCESS on a successful shadow ram write.
**
*******************************************************************************/
s32 WriteNvmBuffer550(struct PciDevice * dev, u32 offset, u16 words, u16 * data)
{
	s32 retval = SUCCESS;
	s32 i = 0;

	/* Take semaphore for the entire operation. */
	retval = dev->nvmops.acquire(dev, SWFW_EEP_SM_X550);
	if(retval != SUCCESS)
	{
		retval = FAIL;
		return retval;
	}

	for(i = 0; i < words; i++)
	{
		retval = WriteNvmData550(dev, offset + i, data[i]);

		if(retval != SUCCESS)
		{
			printf("Eeprom buffered write failed\n");
			break;
		}
	}

	dev->nvmops.release(dev, SWFW_EEP_SM_X550);
	return retval;
}


/*******************************************************************************
**
**  Name:           WriteNvmData550()
**
**  Description:    Writes value to shadow ram with the given data to the
**		    specified offset.  Update checksum should be done after
**		    a call here.  Also, it is possible that if an error
**		    condition is returned, data has been partially written.
**
**  Arguments:      dev       - Device
**		    Offset    - offset to write to
**		    NumWords  - number of words to write
**		    Data      - data to write, can be a buffer
**
**  Returns:        SUCCESS on a successful shadow ram write.
**
*******************************************************************************/
s32 WriteNvmData550(struct PciDevice * dev, u32 Offset, u16 data)
{
	s32 retval;        // status code
	struct NvmCommand buffer;

	//printf("Entering the function %s\n",__FUNCTION__);

	buffer.hdr.req.cmd = FW_WRITE_SHADOW_RAM_CMD;
	buffer.hdr.req.buf_lenh = 0;
	buffer.hdr.req.buf_lenl = FW_WRITE_SHADOW_RAM_LEN;
	buffer.hdr.req.checksum = FW_DEFAULT_CHECKSUM;

	/* one word */
	buffer.Length = htobe16(sizeof(u16));
	buffer.Data = data;
	buffer.Address = htobe32(Offset * 2);

	retval = HostInterfaceCommandX550(dev, (u32 *) &buffer, sizeof(buffer), COMMAND_TIMEOUT, false);
	return retval;
}


/*******************************************************************************
**
**  Name:           ValidateNvmChecksumX550()
**
**  Description:    Verifies checksum after an update.
**
**  Arguments:      dev    - Device to update checksum for
**
**  Returns:        SUCCESS if the checksum is valid.
**
*******************************************************************************/
s32 ValidateNvmChecksumX550(struct PciDevice * dev)
{
	s32 retval;                                      // return value
	u16 checksum;
	u16 ReadChecksum = 0;

	//printf("Entering the function %s\n",__FUNCTION__);

	/* Read the first word from the EEPROM. If this times out or fails, do
	 * not continue or we could be in for a very long wait while every
	 * EEPROM read fails
	 */
	retval = dev->nvmops.read(dev, 0, 1, &checksum);
	if(retval)
	{
		printf("EEPROM read failed\n");
		retval = FAIL;
		return retval;
	}

	retval = dev->nvmops.calculate(dev, NULL, 0);
	if(0 < retval)
	{
		return retval;
	}

	checksum = (u16) (retval & 0xffff);

	retval = ReadNvmBufferX550(dev, NVM_CHECKSUM_REG, 1, &ReadChecksum);
	if(retval != SUCCESS)
	{
		retval = FAIL;
		return retval;
	}

	/* Verify read checksum from EEPROM is the same as
	 * calculated checksum
	 */
	if(ReadChecksum != checksum)
	{
		retval = FAIL;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           UpdateNvmChecksumX553()
**
**  Description:    Calculates checksum after an update and updates the
**		    checksum register.
**
**  Arguments:      dev    - Device to update checksum for
**
**  Returns:        SUCCESS on a successful checksum update.
**
*******************************************************************************/
s32 UpdateNvmChecksumX553(struct PciDevice * dev)
{
	s32 retval = SUCCESS; // return value
	u16 Checksum = 0;   // checksum accumulator
	u16 i = 0;

	// printf("Entering the function %s\n", __FUNCTION__);

	/* Read the first word from the EEPROM. If this times out or fails, do
	 * not continue or we could be in for a very long wait while every
	 * EEPROM read fails
	 */
	retval = ReadNvmBufferX550(dev, 0, 1, &Checksum);
	if(retval != SUCCESS)
	{
		retval = FAIL;
		return retval;
	}

	u16 * eepromData = (u16 *) calloc(dev->wordsize, sizeof(u16));
	if(eepromData == NULL)
	{
		retval = FAIL;
		return retval;
	}
	for(i = 0; i < dev->wordsize; i++)
	{
		dev->nvmops.read(dev, i, 1, &eepromData[i]);
	}

	retval = CalculateChecksumX553(dev, eepromData, dev->wordsize);

	Checksum = (u16) (retval & 0xffff);

	retval = WriteNvmBuffer550(dev, NVM_CHECKSUM_REG, 1, &Checksum);
	if(retval != SUCCESS)
	{
		free(eepromData);
		retval = FAIL;
		printf("NVM Write Error while updating checksum.\n");
		return retval;
	}
	free(eepromData);

	retval = UpdateFlashX550(dev);
	if(retval != SUCCESS)
	{
		printf("NVM Write Error while updating checksum.\n");
	}

	retval = UpdateFlashX550(dev);

	if(retval != SUCCESS)
	{
		printf("NVM Write Error while updating checksum.\n");
	}

	retval = UpdateFlashX550(dev);
	return retval;
}


/*******************************************************************************
**
**  Name:           UpdateNvmChecksumX550()
**
**  Description:    Calculates checksum after an update and updates the
**		    checksum register.
**
**  Arguments:      dev    - Device to update checksum for
**
**  Returns:        SUCCESS on a successful checksum update.
**
*******************************************************************************/
s32 UpdateNvmChecksumX550(struct PciDevice * dev)
{
	s32 retval = SUCCESS;        // return value
	u16 Checksum = 0;              // checksum accumulator

	//printf("Entering the function %s\n",__FUNCTION__);

	/* Read the first word from the EEPROM. If this times out or fails, do
	 * not continue or we could be in for a very long wait while every
	 * EEPROM read fails
	 */
	retval = ReadNvmBufferX550(dev, 0, 1, &Checksum);
	if(retval != SUCCESS)
	{
		retval = FAIL;
		return retval;
	}

	retval = CalculateChecksumX550(dev, NULL, 0);
	if(0 > retval)
	{
		retval = FAIL;
		return retval;
	}

	Checksum = (u16) (retval & 0xffff);

	retval = WriteNvmBuffer550(dev, NVM_CHECKSUM_REG, 1, &Checksum);
	if(retval != SUCCESS)
	{
		retval = FAIL;
		printf("NVM Write Error while updating checksum.\n");
		return retval;
	}

	retval = UpdateFlashX550(dev);

	return retval;
}


/*******************************************************************************
**
**  Name:           UpdateFlashX550()
**
**  Description:    Sets flash update on I210
**
**  Arguments:      dev - Pci device structure
**
**  Returns:        Value indicating whether or not the flash was updated
**
*******************************************************************************/
s32 UpdateFlashX550(struct PciDevice * dev)
{
	s32 retval = SUCCESS;       // return value
	union UpdateCommand buffer;

	buffer.req.cmd = FW_SHADOW_RAM_DUMP_CMD;
	buffer.req.buf_lenh = 0;
	buffer.req.buf_lenl = FW_SHADOW_RAM_DUMP_LEN;
	buffer.req.checksum = FW_DEFAULT_CHECKSUM;

	retval = HostInterfaceCommandX550(dev, (u32 *) &buffer, sizeof(buffer), COMMAND_TIMEOUT, FALSE);

	if(retval == SUCCESS)
		printf("Flash update complete\n");
	else
		printf("Flash update time out\n");

	return retval;
}


/*******************************************************************************
**
**  Name:           CalculateChecksumX550()
**
**  Description:    Calculates checksum during an update
**
**  Arguments:      dev    - Pci device structure
**					buffer - pointer to buffer
**					size   - size of buffer
**
**  Returns:        16-bit Checksum value
**
*******************************************************************************/
s32 CalculateChecksumX550(struct PciDevice * dev, u16 * buffer, u32 size)
{
	u16 NvmPtr[NVM_LAST_WORD + 1];
	s32 retval = SUCCESS;  // return value
	u16 Checksum = 0;      // checksum accumulator
	u16 i = 0;             // counter
	u16 * RegData; // place to hold local data

	//printf("Entering the function %s\n",__FUNCTION__);

	if(!buffer)
	{
		/* Read pointer area */
		retval = ReadNvmBufferX550(dev, 0, NVM_LAST_WORD + 1, NvmPtr);
		if(retval != SUCCESS)
		{
			retval = FAIL;
			return retval;
		}
		RegData = NvmPtr;
	}
	else
	{
		if(size < NVM_LAST_WORD)
		{
			retval = FAIL;
			return retval;
		}
		RegData = buffer;
	}

	/*
	 * For X550 hardware include 0x0-0x41 in the checksum, skip the
	 * checksum word itself
	 */
	for(i = 0; i <= NVM_LAST_WORD; i++)
		if(i != NVM_CHECKSUM_REG)
			Checksum += RegData[i];

	Checksum = (u16) NVM_SUM - Checksum;

	return (s32) Checksum;
}


/*******************************************************************************
**
**  Name:           CalculateChecksumX553()
**
**  Description:    Calculates checksum during an update
**
**  Arguments:      dev    - Pci device structure
**                  buffer - pointer to buffer
**                  size   - size of buffer
**
**  Returns:        16-bit Checksum value
**
*******************************************************************************/
s32 CalculateChecksumX553(struct PciDevice * dev, u16 * buffer, u32 size)
{
	u16 checksum = 0; // checksum accumulator
	u16 length = 0;
	u16 offset = 0;
	u16 i = 0;
	u16 j = 0;

	/*
	 * For X553 hardware include 0x0-0x41 in the checksum, skip the
	 * checksum word itself
	 **/
	for(i = 0; i <= NVM_LAST_WORD; i++)
	{
		if(i != NVM_CHECKSUM_REG)
			checksum += buffer[i];
	}

	for(i = START_PTR; i < LAST_PTR; i++)
	{
		if((i == PHY_PTR) || (i == OPTION_ROM_PTR))
			continue;

		if((i == PCIE_GENERAL_PTR) || (i == PCIE_CONFIG0_PTR) ||
		   (i == PCIE_CONFIG1_PTR))
		{

			length = i == PCIE_GENERAL_PTR ? PCIE_GENERAL_SIZE
			                               : PCIE_CONFIG_SIZE;
			offset = buffer[i];
			for(j = offset; j < offset + length; j++)
			{
				checksum += buffer[j];
			}
		}
		else {
			offset = buffer[i];
			if(offset == 0xffff || offset == 0)
			{
				// skip an invalid section
				continue;
			}

			length = buffer[offset];
			for(j = offset + 1; j <= offset + length; j++)
			{
				checksum += buffer[j];
			}
		}
	}

	checksum = (u16) (NVM_SUM - checksum);
	return checksum;
}


/*******************************************************************************
**
**  Name:           HostInterfaceCommandX550()
**
**  Description:    Issues command to manageability block
**
**  Arguments:      dev     - Pci device structure
**					buffer  - contains command to write and also
**							return status
**					length  - length of buffer, multiple of 4 bytes
**					timeout - time in ms to wait for completion
**					value   - read and return data from buffer (true)
**								or not (false)
**
**  Returns:        SUCCESS on a successful communication with
**						manageability block
**
*******************************************************************************/
s32 HostInterfaceCommandX550(struct PciDevice * dev, u32 * buffer, u32 length, u32 timeout, bool value)
{
	s32 retval = SUCCESS;
	u32 RegVal;
	u32 Val;
	u32 fwsts;
	u32 i = 0;
	u32 j = 0;
	u32 size = sizeof(struct HicCommand);
	u16 BufferLength;
	u16 WordLength;

	//printf("Entering the function %s\n",__FUNCTION__);

	retval = dev->nvmops.acquire(dev, SWFW_SW_MNG_SM_X550);
	if(retval != SUCCESS)
	{
		printf("acquire failed in host interface command\n");
		return retval;
	}

	if(length == 0 || length > MAX_BLOCK_BYTE_LENGTH)
	{
		printf("Buffer length failure\n");
		retval = FAIL;
		return retval;
	}

	/* Set bit 9 of FWSTS clearing FW reset indication */
	fwsts = ReadReg(dev, FWSTS);

	WriteReg(dev, FWSTS, fwsts | FWSTS_FWRI);

	/* Check that the host interface is enabled. */
	RegVal = ReadReg(dev, HICR);

	if((RegVal & HICR_EN) == 0)
	{
		retval = FAIL;
		printf("Host Enable bit disabled\n");
		goto rel_out;
	}

	/* Calculate length in DWORDs. We must be DWORD aligned */
	if((length % (sizeof(u32))) != 0)
	{
		retval = FAIL;
		printf("Buffer length failure, not aligned to dword\n");
		goto rel_out;
	}

	WordLength = length >> 2;

	/* The device driver writes the relevant command block
	 * into the ram area.
	 */
	for(i = 0; i < WordLength; i++)
	{
		WriteReg(dev, (FLEX_MNG + (i << 2)), htole32(buffer[i]));
	}

	/* Setting this bit tells the ARC that a new command is pending. */
	Val = RegVal | HICR_C;
	WriteReg(dev, HICR, Val);

	for(i = 0; i < timeout; i++)
	{
		RegVal = ReadReg(dev, HICR);
		if(!(RegVal & HICR_C))
			break;
		usleep(10);
	}

	/* Check command completion */
	if((timeout != 0 && i == timeout) || !(ReadReg(dev, HICR) & HICR_SV))
	{
		retval = FAIL;
		printf("Status is not valid\n");
		goto rel_out;
	}

	if(!value)
	{
		goto rel_out;
	}

	/* Calculate length in DWORDs */
	WordLength = size >> 2;

	/* first pull in the header so we know the buffer length */
	for(j = 0; j < WordLength; j++)
	{
		buffer[j] = ReadReg(dev, (FLEX_MNG) + ((j) << 2));         // Read reg array
		buffer[j] = le32toh(buffer[j]);
	}

	/* If there is any thing in data position pull it in */
	BufferLength = ((struct HicCommand *) buffer)->BufferLength;
	if(BufferLength == 0)
	{
		retval = SUCCESS;
		goto rel_out;
	}

	if(length < BufferLength + size)
	{
		retval = FAIL;
		printf("Buffer not large enough for reply\n");
		goto rel_out;
	}

	/* Calculate length in DWORDs, add 3 for odd lengths */
	WordLength = (BufferLength + 3) >> 2;

	/* Pull in the rest of the buffer (bi is where we left off) */
	for(; j <= WordLength; j++)
	{
		buffer[j] = ReadReg(dev, (FLEX_MNG) + ((j) << 2));
		buffer[j] = le32toh(buffer[j]);
	}

	retval = SUCCESS;

rel_out:
	dev->nvmops.release(dev, SWFW_SW_MNG_SM_X550);

	return retval;
}


/*******************************************************************************
**
**  Name:           AcquireNvmI210()
**
**  Description:    Request semaphores for NVM access.
**
**  Arguments:      dev    - Device we need semaphores for
**
**  Returns:        Success on acquisition of the semaphore, fail otherwise
**
*******************************************************************************/
s32 AcquireNvmI210(struct PciDevice* dev, u32 mask)
{
	u32 SyncVal = 0;
	u32 SwCtrlMask = SWFW_EEP_SM;
	u32 FwCtrlMask = SWFW_EEP_SM << 16;
	s32 retval = SUCCESS;
	s32 i = 0;
	s32 MaxAttempts = 200;

	/* we only try to aquire the semaphore for a certain amount of time
	 * to prevent infinite polling */
	while(i < MaxAttempts)
	{
		// Check for the hardware semaphore
		if(GetHwSemaphoreI210(dev))
		{
			retval = FAIL;
			printf("GetHwSemaphoreI210 failed\n");
			return retval;
		}

		// read the sync value from the register
		SyncVal = ReadReg(dev, SW_FW_SYNC);

		// if the firmware semaphore is not set, break
		if(!(SyncVal & FwCtrlMask))
			break;

		// Firmware currently using resource (fwmask)
		PutHwSemaphoreI210(dev);

		// delay before trying again
		usleep(5);
		i++;
	}

	// check to see if we timed out, if so print an error and return
	if(i == MaxAttempts)
	{
		retval = FAIL;
		printf("GetHwSemaphoreI210 MaxAttempts failed\n");
		return retval;
	}

	// set the sw semaphore bit using swmask
	SyncVal |= SwCtrlMask;

	// write that value to the sync register
	WriteReg(dev, SW_FW_SYNC, SyncVal);

	// put hw semaphore
	PutHwSemaphoreI210(dev);

	return retval;
}


/*******************************************************************************
**
**  Name:           ReleaseNvmI210()
**
**  Description:    Clear the request bit for the eeprom, and release
**		    any semaphore that may have been aquired
**
**  Arguments:      dev - Device to release
**
*******************************************************************************/
void ReleaseNvmI210(struct PciDevice * dev, u32 mask)
{
	u32 SyncVal = 0; // a place to hold register data

	while(GetHwSemaphoreI210(dev) != SUCCESS)
		; /* Empty */

	// Read the sync reg
	SyncVal = ReadReg(dev, SW_FW_SYNC);

	// Clear the request bit
	SyncVal &= ~SWFW_EEP_SM;

	// write that value back
	WriteReg(dev, SW_FW_SYNC, SyncVal);

	// put hw semaphore
	PutHwSemaphoreI210(dev);
}


/*******************************************************************************
**
**  Name:           GetHwSemaphoreI210()
**
**  Description:    Acquires the HW semaphore
**
**  Arguments:      dev    - Device to get the semaphore for
**
**  Returns:        SUCCESS if we got the semaphore, FAIL otherwise
**
*******************************************************************************/
s32 GetHwSemaphoreI210(struct PciDevice * dev)
{
	u32 SemVal = 0;                        // semaphore value
	s32 retval = SUCCESS;                  // return value
	s32 MaxAttempts = dev->wordsize + 1;   // timeout
	s32 i = 0;                             // counter

	// Try to get the FW semaphore.
	// staying within the timeout boundary
	for(i = 0; i < MaxAttempts; i++)
	{
		// read the semaphore value
		SemVal = ReadReg(dev, SWSM);

		// set the hw semaphore bit
		WriteReg(dev, SWSM, SemVal | SWSM_SWESMBI);

		// Semaphore acquired if bit latched
		if(ReadReg(dev, SWSM) & SWSM_SWESMBI)
			break;

		// delay before trying again
		usleep(50);
	}

	// check to see if we've timed out
	if(i == MaxAttempts)
	{
		// Release semaphores
		PutHwSemaphore82574(dev, 0);

		retval = FAIL;
		return retval;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           PutHwSemaphoreI210()
**
**  Description:    Puts the HW semaphore
**
**  Arguments:      dev    - Device to put semaphore for
**
*******************************************************************************/
void PutHwSemaphoreI210(struct PciDevice * dev)
{
	u32 SemVal = 0;     // semaphore value

	// get the software semaphore value
	SemVal = ReadReg(dev, SWSM);

	// write the bit
	SemVal &= ~SWSM_SWESMBI;

	// write the register
	WriteReg(dev, SWSM, SemVal);
}


/*******************************************************************************
**
**  Name:           ReadNvmSrrdI210()
**
**  Description:    Reads shadow ram register
**
**  Arguments:      dev       - Device
**		    Offset    - offset to read from
**		    NumWords  - number of words to read
**		    Data      - OUT: data read from register
**
**  Returns:        SUCCESS if we were able to read the shadow ram
**
*******************************************************************************/
s32 ReadNvmSrrdI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	s32 retval = SUCCESS; // return value
	u16 i = 0;           // counter
	u16 count = 0;       // word count

	/* Read words in small bursts due to the fact that firmware can
	 * forcefully take control of the semaphore after a timeout period */
	for(i = 0; i < NumWords; i += EERD_EEWR_MAX_COUNT)
	{
		count = (NumWords - i) / EERD_EEWR_MAX_COUNT > 0 ? EERD_EEWR_MAX_COUNT : (NumWords - i);

		if(dev->nvmops.acquire(dev, 0) == SUCCESS)
		{
			retval = ReadNvmEerd(dev, Offset, count, Data + i);
			dev->nvmops.release(dev, 0);
		}
		else
		{
			retval = FAIL;
		}

		if(retval != SUCCESS)
			break;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           WriteNvmSrwrI210()
**
**  Description:    Writes value to shadow ram with the given data to the
**		    specified offset.  Update checksum should be done after
**		    a call here.  Also, it is possible that if an error
**		    condition is returned, data has been partially written.
**
**  Arguments:      dev       - Device
**		    Offset    - offset to write to
**		    NumWords  - number of words to write
**		    Data      - data to write, can be a buffer
**
**  Returns:        SUCCESS on a successful shadow ram write.
**
*******************************************************************************/
s32 WriteNvmSrwrI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	s32 retval = SUCCESS;  // status code
	u16 i = 0;             // count index
	u16 count = 0;         // count index


	/* Write words in small bursts due to the fact that firmware can
	 * forcefully take control of the semaphore after a timeout period */
	for(i = 0; i < NumWords; i += EERD_EEWR_MAX_COUNT)
	{
		count = (NumWords - i) / EERD_EEWR_MAX_COUNT > 0 ? EERD_EEWR_MAX_COUNT : (NumWords - i);

		if(dev->nvmops.acquire(dev, 0) == SUCCESS)
		{
			retval = WriteNvmSrwr(dev, Offset, count, Data);
			dev->nvmops.release(dev, 0);
		}
		else
		{
			retval = FAIL;
		}

		if(retval != SUCCESS)
			break;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           WriteNvmSrwr()
**
**  Description:    Writes value to shadow ram with the given data to the
**		    specified offset.  Update checksum should be done after
**		    a call here.  Also, it is possible that if an error
**		    condition is returned, data has been partially written.
**
**  Arguments:      dev       - Device
**		    Offset    - offset to write to
**		    NumWords  - number of words to write
**		    Data      - data to write, can be a buffer
**
**  Returns:        SUCCESS if we read words, FAIL otherwise
**
*******************************************************************************/
s32 WriteNvmSrwr(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	u32 i = 0;                  // counter
	u32 k = 0;                  // poll attempt counter
	u32 RegVal = 0;             // value to write to eewr
	u32 MaxAttempts = 100000;   // max attempts to try
	s32 retval = SUCCESS;       // return value

	/* A check for invalid values:  offset too large, too many words,
	 * too many words for the offset, and not enough words. */
	if((Offset >= dev->wordsize) || (NumWords > (dev->wordsize - Offset)) || (NumWords == 0))
	{
		retval = FAIL;
		return retval;
	}

	// loop through the number of words specified
	for(i = 0; i < NumWords; i++)
	{
		// calculate the value to write to the register
		RegVal = ((Offset + i) << NVM_RW_ADDR_SHIFT) | (Data[i] << NVM_RW_REG_DATA) | NVM_RW_REG_START;

		// write the value to the register
		WriteReg(dev, SRWR, RegVal);

		/* poll to see if the value was written
		 * if so break out of the loop */
		for(k = 0; k < MaxAttempts; k++)
		{
			if(NVM_RW_REG_DONE & ReadReg(dev, SRWR))
			{
				retval = SUCCESS;
				break;
			}

			// delay for a bit before trying again
			usleep(5);
		}

		if(retval != SUCCESS)
		{
			break;
		}
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           DumpInvmI211()
**
**  Description:    Dumps contents of iNVM into the buffe, as 32 bit words
**
**  Arguments:      dev     - PCI device structure
**		    buffer  - Buffer to dump into
**
**  Returns:        Status code.
**
*******************************************************************************/
s32 DumpInvmI211(struct PciDevice * dev, u32 * buffer)
{
	s32 retval = SUCCESS; // return value
	u32 dword = 0;        // a place to read the word into
	u16 i = 0;            // counter

	// read each word in from the iNVM and copy it to the buffer
	for(i = 0; i < INVM_SIZE; i++)
	{
		dword = ReadReg(dev, INVM_DATA_REG(i));
		*buffer = dword;
		++buffer;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           ValidateNvmChecksumI210()
**
**  Description:    Verifies checksum after an update.
**
**  Arguments:      dev    - Device to update checksum for
**
**  Returns:        SUCCESS if the checksum is valid.
**
*******************************************************************************/
s32 ValidateNvmChecksumI210(struct PciDevice * dev)
{
	s32 retval = SUCCESS;                                    // return value
	s32 (* read_op_ptr)(struct PciDevice *, u32, u16, u16 *); // function ptr

	// attempt to get the semaphore
	if(dev->nvmops.acquire(dev, 0) == SUCCESS)
	{
		/* We already have the semaphore so we can just read the register
		 * at this point.
		 */
		read_op_ptr = dev->nvmops.read;
		dev->nvmops.read = ReadNvmEerd;

		// reuse checksum validation function, it will be the same
		retval = ValidateNvmChecksum82574(dev);

		// Revert original read operation.
		dev->nvmops.read = read_op_ptr;

		// release the semaphore
		dev->nvmops.release(dev, 0);
	}
	else
	{
		retval = FAIL;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           UpdateNvmChecksumI210()
**
**  Description:    Calculates checksum after an update and updates the
**		    checksum register.
**
**  Arguments:      dev    - Device to update checksum for
**
**  Returns:        SUCCESS on a successful checksum update.
**
*******************************************************************************/
s32 UpdateNvmChecksumI210(struct PciDevice * dev)
{
	s32 retval = SUCCESS;        // return value
	u16 Checksum = 0;              // checksum accumulator
	u16 i = 0;                     // counter
	u16 RegData[2];        // place to hold data

	if(MacAddress[0] == 0xFF)
	{
		printf("Checksum already written.\n");
		return SUCCESS;
	}

	printf("Updating checksum...\n");

	// check to see if we have the semaphore
	if(dev->nvmops.acquire(dev, 0) == SUCCESS)
	{
		// loop through regs up to the checksum reg
		for(i = 0; i < NVM_CHECKSUM_REG; i += 2)
		{
			memset(RegData, 0, sizeof(RegData));
			retval = dev->nvmops.read(dev, i, 2, RegData);

			if(retval)
			{
				dev->nvmops.release(dev, 0);
				printf("NVM Read Error while updating checksum.\n");
				return FAIL;
			}
			Checksum += RegData[0];
			if(i < NVM_CHECKSUM_REG - 1)
				Checksum += RegData[1];
		}

		// calculate the checksum value and write it
		Checksum = (u16) NVM_SUM - Checksum;

		//printf("Calculated checksum: %04x\n", Checksum);

		retval = dev->nvmops.write(dev, NVM_CHECKSUM_REG, 1, &Checksum);

		if(retval != SUCCESS)
			printf("NVM Write Error while updating checksum.\n");

		dev->nvmops.release(dev, 0);
	}
	else
	{
		retval = FAIL;
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           UpdateFlashI210()
**
**  Description:    Sets flash update on I210
**
**  Arguments:      dev - Pci device structure
**
**  Returns:        Value indicating whether or not the flash was updated
**
*******************************************************************************/
s32 UpdateFlashI210(struct PciDevice * dev)
{
	s32 retval = SUCCESS;       // return value
	u32 UpdateStatus = 0;       // register data
	u32 RegVal = 0;

	// poll for a flash update
	retval = PollFlashUpdateDoneI210(dev);

	// a timeout occured, so return fail
	if(retval == FAIL)
	{
		return retval;
	}

	// read what's currently in the register and set the flash update bit
	RegVal = ReadReg(dev, EECD);
	UpdateStatus = RegVal | EECD_FLUPD_I210;

	// write that value
	WriteReg(dev, EECD, UpdateStatus);

	// look for update complete...
	retval = PollFlashUpdateDoneI210(dev);
	retval = PollFlashUpdateDoneI210(dev);

	if(retval == SUCCESS)
		printf("Flash update complete\n");
	else
		printf("Flash update time out retval is %X\n", retval);

	return retval;
}


/*******************************************************************************
**
**  Name:           PollFlashUpdateDoneI210()
**
**  Description:    Poll the update register, we want a write complete signal
**
**  Arguments:      dev - Pci device structure
**
**  Returns:        Value indicating whether or not we have received
**		    an update complete signal (SUCCESS or FAIL)
**
*******************************************************************************/
s32 PollFlashUpdateDoneI210(struct PciDevice * dev)
{
	s32 retval = FAIL; // return value
	u32 i = 0;        // counter
	u32 RegVal = 0;   // holds register data

	// Try for the specified number of attempts
	for(i = 0; i < FLUDONE_ATTEMPTS; i++)
	{
		// get the value in EECD
		RegVal = ReadReg(dev, EECD);

		// check if the flash update done bit is set.  if so, return success and break
		if(RegVal & EECD_FLUDONE_I210)
		{
			retval = SUCCESS;
			break;
		}

		// wait a little bit before trying again
		usleep(5);
	}

	return retval;
}

/*******************************************************************************
**
**  Name:           GetFlswFlashOperationDone()
**
**  Description:    Checks to see if the I210 NVM when blank is in a neutral state (ready to program)
**
**  Arguments:      dev    - Device we need to check
**
**  Returns:        Success on it being in done mode, fail otherwise
**
*******************************************************************************/
s32 GetFlswFlashOperationDone(struct PciDevice * dev)
{
	u32 Flswctl = 0;
	u32 Timeout = FLASH_ERASE_POLL * 100;                   /* 100 second */
	s32 Status = FAIL;

	do
	{
		/* Read FLSWCNT register */
		Flswctl = ReadReg(dev, FLSWCTL);

		/* Check if done bit is set */
		if((Flswctl & (FLSWCTL_BUSY | FLSWCTL_DONE)) == FLSWCTL_DONE)
		{
			Status = SUCCESS;
			break;
		}

		/* Wait for 1 microsecond */
		usleep(1);
		Timeout--;
	} while (Timeout);
	return Status;
}

/***************************************************************************
**
** Name:            WriteFlswFlashCommand()
**
** Description:     Sets FLSW Command and Address in FLSWCTL register and checks
**                  if the command is valid
**
** Arguments:       dev - pointer to the adapter structure.
**                  Opcode - opcode of requested command
**                  Address - Address in bytes
**
** Returns:         SUCCESS - Operation ended successfully.
**                  FAIL- Bad parameters passed or command valid bit was not set.
**
****************************************************************************/
s32 WriteFlswFlashCommand(struct PciDevice * dev, u32 Opcode, u32 Address)
{
	u32 Flswctl = 0;
	s32 Status = SUCCESS;

	/* Validate input data */
	if((Address != (Address & FLSWCTL_ADDRESS_MASK)) || (Opcode != (Opcode & FLSWCTL_CMD_MASK)))
	{
		printf("WriteFlswFlashCommand was passed bogus inputs\n");
		Status = FAIL;
	}
	else
	{
		/* Build and issue command */
		Flswctl = Opcode | Address;
		WriteReg(dev, FLSWCTL, Flswctl);

		/* Check if issued command is valid */
		Flswctl = ReadReg(dev, FLSWCTL);
		if(!(Flswctl & FLSWCTL_CMDV))
		{
			Status = FAIL;
			printf("Hardware rejected opcode=%x, addr=%x\n", Opcode, Address);
		}
	}
	return Status;
}

/*******************************************************************************
**
**  Name:           ReadNvmFlswI210()
**
**  Description:    Reads NVM when in FLSW mode
**
**  Arguments:      dev       - Device
**		    Offset    - offset to read from
**		    NumWords  - number of words to read
**		    Data      - OUT: data read from register
**
**  Returns:        SUCCESS if we were able to read the NVM
**
*******************************************************************************/
s32 ReadNvmFlswI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	s32 Status = SUCCESS;
	u32 FlashSize = 0;
	s32 i;
	u32 help;
	u8 SemTaken = FALSE;

	//printf("Entering ReadNvmFlswI210 offset %x NumWords %x\n", Offset, NumWords);

	/* Get flash size */
	FlashSize = dev->FlashSize;
	/* Check if DataSize is DWORD aligned and we don't try to read beyond the Flash Size */
	if(Offset + NumWords > FlashSize)
	{
		printf("ReadNVMFlsw was passed a bogus input\n");
		Status = FAIL;
	}
	do
	{
		if(NumWords < 2 || NumWords > 128)
		{
			printf("error: NumWords out of range\n");
			break;
		}
		/* Make sure that FLSW interface is ready */
		Status = GetFlswFlashOperationDone(dev);
		if(Status != SUCCESS)
		{
			printf("interface not ready\n");
			break;
		}

		/* Acquire Flash ownership */
		//dev->nvmops.acquire(dev);
		Status = AcquireNvmI210(dev, 0);
		if(Status != SUCCESS)
		{
			printf("Could not get Semaphores\n");
			break;
		}
		SemTaken = TRUE;
		/* Set number of bytes to write */
		WriteReg(dev, FLSWCNT, NumWords * 2);

		/* Wait till operation has finished */
		Status = GetFlswFlashOperationDone(dev);
		if(Status != SUCCESS)
		{
			printf("Could not clear last operation\n");
			break;
		}

		/* Write command to command register */
		Status = WriteFlswFlashCommand(dev, FLSWCTL_CMD_READ, Offset * 2);
		if(Status != SUCCESS)
		{
			printf("WriteFlswFlashCommand in ReadNvmFlswI210 failed\n");
			break;
		}

		/* Wait till operation has finished */
		Status = GetFlswFlashOperationDone(dev);
		if(Status != SUCCESS)
		{
			printf("Could not clear last operation after write\n");
			break;
		}

		/* Read data to data register */
		for(i = 0; i < NumWords / 2; i++)
		{
			help = ReadReg(dev, FLSWDATA);
			Status = GetFlswFlashOperationDone(dev);
			*(Data + 2 * i) = (u16) help;
			*(Data + 2 * i + 1) = (u16) (help >> 16);
		}
	} while (0);

	/* Release Flash ownership */
	if(SemTaken == TRUE)
	{
		dev->nvmops.release(dev, 0);
	}

	return Status;
}

/*******************************************************************************
**
**  Name:           WriteNvmFlswI210()
**
**  Description:    Writes value to NVM with the given data to the
**		    specified offset.  Update checksum should be done after
**		    a call here.  Also, it is possible that if an error
**		    condition is returned, data has been partially written.
**
**  Arguments:      dev       - Device
**		    Offset    - offset to write to
**		    NumWords  - number of words to write this only does WORDS.
**		    Data      - data to write, can be a buffer
**
**  Returns:        SUCCESS on a successful NVM write.
**
*******************************************************************************/
s32 WriteNvmFlswI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	s32 Status = FAIL;
	u32 FlashSize = dev->FlashSize;
	u32 outdata = 0;
	u8 i = 0;
	u8 SemTaken = FALSE;

	do
	{
		//printf("Entering WriteNVMFlswI210 offset %x Numwords %x FlashSize %x data%x\n", Offset, NumWords,FlashSize, *Data);
		/* Validate parameters */
		if((Offset & 0x0FF) == 0x0FF)
		{
			//can't cross a page boundary
			Status = SUCCESS;
			//break;
		}

		if(Offset + NumWords > FlashSize)
		{
			printf("Error: Trying to write beyond Flash Size\n");
			Status = FAIL;
			break;
		}
		if((NumWords < 1) || (NumWords > 128))
		{
			Status = FAIL;
			printf("NumWords is out of range!\n");
			break;
		}

		/* Make sure that FLSW interface is ready */
		Status = GetFlswFlashOperationDone(dev);
		if(Status != SUCCESS)
		{
			break;
		}

		/* Acquire Flash ownership */
		Status = AcquireNvmI210(dev, 0);
		if(Status != SUCCESS)
		{
			printf("Could not get Semaphores\n");
			break;
		}

		SemTaken = TRUE;
		/* Set number of bytes to write */
		WriteReg(dev, FLSWCNT, NumWords * 2);           //write a NumWords words

		/* Write command to command register */
		Status = WriteFlswFlashCommand(dev, FLSWCTL_CMD_WRITE, Offset * 2);
		if(Status != SUCCESS)
		{
			printf("WriteFlswFlashCommand in WriteNvmFlswI210 failed\n");
			break;
		}

		/* Wait till operation has finished */
		Status = GetFlswFlashOperationDone(dev);
		do
		{
			outdata = *Data;
			Data++;
			outdata |= ((u32) * Data) << 16;
			Data++;
			//printf("writing: %08x\n", outdata);
			Status = WriteReg(dev, FLSWDATA, outdata);
			if(Status != SUCCESS)
			{
				printf("WriteReg in WriteNvmFlswI210 failed\n");
				break;
			}

			// Wait till operation has finished
			Status = GetFlswFlashOperationDone(dev);
			if(Status != SUCCESS)
			{
				printf("GetFlswFlashOperationDone in WriteNvmFlswI210 failed\n");
				break;
			}
			i++;
		} while (i <= NumWords);

	} while (0);

	/* Release Flash ownership */
	if(SemTaken == TRUE)
	{
		dev->nvmops.release(dev, 0);
	}

	return Status;
}

/*******************************************************************************
**
**  Name:           GetHwSemaphore82574()
**
**  Description:    request the semaphore
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        Value indicating whether or not we have acquired the
**		    semaphore (SUCCESS or FAIL)
**
*******************************************************************************/
s32 GetHwSemaphore82574(struct PciDevice * dev, u32 mask)
{
	u32 ExtConfigCtrl = 0;  // config control readings will go here
	s32 i = 0;              // counter

	// read extended config control register
	ExtConfigCtrl = ReadReg(dev, EXTCNF_CTRL);

	// set the bit requesting software ownership
	ExtConfigCtrl |= EXTCNF_CTRL_MDIO_SW_OWNERSHIP;

	// try at least once to get ownership, repeat until timeout
	do
	{
		// write the software request to the control register
		WriteReg(dev, EXTCNF_CTRL, ExtConfigCtrl);

		// read the value back from the control register
		ExtConfigCtrl = ReadReg(dev, EXTCNF_CTRL);

		// check to see if ownership was granted, if yes, break
		if(ExtConfigCtrl & EXTCNF_CTRL_MDIO_SW_OWNERSHIP)
			break;

		// set the sw ownership bit so we can request again
		ExtConfigCtrl |= EXTCNF_CTRL_MDIO_SW_OWNERSHIP;

		// delay for a little bit
		sleep(2);

		i++;
	} while (i < MDIO_OWNERSHIP_TIMEOUT);

	// if our counter reached the max attempts, sw didn't get ownership
	if(i == MDIO_OWNERSHIP_TIMEOUT)
	{
		// Release semaphores
		PutHwSemaphore82574(dev, 0);

		return FAIL;
	}

	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           PutHwSemaphore82574()
**
**  Description:    Release the semaphore
**
**  Arguments:      dev - Pci Device structure
**
*******************************************************************************/
void PutHwSemaphore82574(struct PciDevice * dev, u32 mask)
{
	u32 ExtConfigCtrl = 0;      // somewhere to put register data

	// read the extended configuration control register
	ExtConfigCtrl = ReadReg(dev, EXTCNF_CTRL);

	// clear the software ownership bit
	ExtConfigCtrl &= ~EXTCNF_CTRL_MDIO_SW_OWNERSHIP;

	// write that value back to the register
	WriteReg(dev, EXTCNF_CTRL, ExtConfigCtrl);
}


/*******************************************************************************
**
**  Name:           ReadNvmEerd()
**
**  Description:    Reads some word(s) from the NVM
**
**  Arguments:      dev      - Pci device structure
**		    Offset   - NVM offset to read
**		    NumWords - Number of words to read
**		    Data     - a place to put the read value
**
**  Returns:        SUCCESS if any words were read, FAIL if invalid data was
**		    given.
**
*******************************************************************************/
s32 ReadNvmEerd(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	u32 i = 0;                  // counter
	u32 RegVal = 0;             // a place to put register data
	s32 retval = SUCCESS;       // return value

	/* A check for invalid values:  offset too large, too many words,
	 * too many words for the offset, and not enough words. */
	if((Offset >= dev->wordsize) || (NumWords > (dev->wordsize - Offset)) || (NumWords == 0))
	{
		printf("nvm parameter(s) out of bounds offset %x wordsize %x numwords %x\n", Offset, dev->wordsize, NumWords);
		return FAIL;
	}

	// Loop reading one word at a time until we reach the number of words requested
	for(i = 0; i < NumWords; i++)
	{
		// calculate the address we want to read and put it in the correct bit positions
		RegVal = ((Offset + i) << NVM_RW_ADDR_SHIFT) + NVM_RW_REG_START;

		// write the value to the EERD register
		WriteReg(dev, EERD, RegVal);

		// wait until the operation is complete
		retval = PollEerdEewrDone(dev, NVM_POLL_READ);

		if(retval)
			break;

		// copy the register value into the array
		Data[i] = (ReadReg(dev, EERD) >> NVM_RW_REG_DATA);
	}

	return retval;
}


/*******************************************************************************
**
**  Name:           InitNvmParamsX550
**
**  Description:    Initialize function pointers
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        SUCCESS
**
*******************************************************************************/
s32 InitNvmParamsX550(struct PciDevice * dev)
{
	//printf("Entering the function %s\n",__FUNCTION__);
	dev->nvmops.acquire = AcquireNvmX550;
	dev->nvmops.release = ReleaseNvmX550;
	dev->nvmops.read = ReadNvmBufferX550;
	dev->nvmops.write = WriteNvmBuffer550;
	dev->nvmops.validate = ValidateNvmChecksumX550;
	dev->nvmops.update = UpdateNvmChecksumX550;
	dev->nvmops.calculate = CalculateChecksumX550;

	dev->wordsize = NVMSIZE_X550;

	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           InitNvmParamsX553
**
**  Description:    Initialize function pointers
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        SUCCESS
**
*******************************************************************************/
s32 InitNvmParamsX553(struct PciDevice * dev)
{
	// printf("Entering the function %s\n",__FUNCTION__);
	dev->nvmops.acquire = AcquireNvmX553;
	dev->nvmops.release = ReleaseNvmX553;
	dev->nvmops.read = ReadNvmBufferX550;
	dev->nvmops.write = WriteNvmBuffer550;
	dev->nvmops.validate = ValidateNvmChecksumX550;
	dev->nvmops.update = UpdateNvmChecksumX553;
	dev->nvmops.calculate = CalculateChecksumX553;

	dev->wordsize = NVMSIZE_X550;
	return SUCCESS;
}

u8 FlashDetectAndSetWritable(struct PciDevice * dev)
{
	static u8 ManufacturerId = 0;
	u16 FlashId;

	if(!ManufacturerId)         // first function call
	{
		FlashId = SerialGetFlashIdJedec(dev);
		printf("Detected FlashId: %04x\n", FlashId);
		ManufacturerId = FlashId >> 8;

		switch(ManufacturerId)
		{
			case 0xBF:
				printf("Using SST method\n");
				SerialFlashWriteStatusRegister(dev, TRUE, FALSE);
				break;
			default:
				printf("Using generic method\n");
				SerialFlashWriteStatusRegister(dev, TRUE, TRUE);
		}
	}

	return ManufacturerId;
}
s8 SetupWriteMethod(struct PciDevice * dev)
{
	u16 TestPattern = 0xBABA;
	u16 Data[2];
	u8 ManufacturerId = 0;
	u8 i = 0;
	s8 Status = FAIL;

	// Flash ID detection and  writing to the status register, to allow writing and erasing
	ManufacturerId = FlashDetectAndSetWritable(dev);

#ifdef FAST
	if((dev->DeviceId != INTEL_I210_BLANK) && (ManufacturerId == 0xBF))
	{
		dev->nvmops.write = SstBitBangFlashWriteI210;
	}
	return SUCCESS;
#endif
	// Full chip erase
	printf("Erasing Full NVM...\n");
	EraseNvm(dev);

	// Testing writing and reading methods
	for(i = 0; i < 2; i++)
	{
		memset(Data, 0, sizeof(Data));
		dev->nvmops.write(dev, 0, 1, &TestPattern);
		dev->nvmops.read(dev, 0, 2, Data);

		//printf("Write: %x, Read: %x\n", TestPattern, Data[0]);

		if(Data[0] == TestPattern)
		{
			printf("Test writing worked.\n");
			Status = SUCCESS;
			break;
		}
		else if(i == 1) // writing failed for both iterations
		{
			printf("error: No writing method worked.\n");
			Status = FAIL;
			break;
		}

		printf("Default writing method failed, trying bit banging...\n");
		switch(ManufacturerId)
		{
			case 0xBF:
				dev->nvmops.write = SstBitBangFlashWriteI210;
				break;
			default:
				dev->nvmops.write = GenericBitBangFlashWriteI210;
		}
		dev->nvmops.read = GenericBitBangFlashReadI210;
	}

	// Erasing first sector after test writing
	printf("Erasing sector to cleanup test write...\n");
	SectorEraseNvm(dev, 0);
	return Status;
}

/*******************************************************************************
**
**  Name:           InitNvmParamsI210
**
**  Description:    Initialize function pointers
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        SUCCESS
**
*******************************************************************************/
s32 InitNvmParamsI210(struct PciDevice * dev)
{
	//Code path is different for a blank NVM I210 for access.
	if(dev->DeviceId == INTEL_I210_BLANK)
	{
		dev->nvmops.read = ReadNvmFlswI210;
		dev->nvmops.write = WriteNvmFlswI210;
	}
	else
	{
		dev->nvmops.read = GenericBitBangFlashReadI210;
		dev->nvmops.write = GenericBitBangFlashWriteI210;
	}

	dev->FlashSize = 0x40000;
	dev->wordsize = dev->FlashSize;

	//these are shared across Blank and unblank.
	dev->nvmops.erase = BitBangFlashEraseI210;
	dev->nvmops.sector_erase = BitBangFlashSectorEraseI210;
	dev->nvmops.validate = ValidateNvmChecksumI210;
	dev->nvmops.update = UpdateNvmChecksumI210;
	dev->nvmops.acquire = AcquireNvmI210;
	dev->nvmops.release = ReleaseNvmI210;
	return SUCCESS;
}

s32 InitNvmParamsNull(struct PciDevice * dev)
{
	dev->nvmops.read = NULL;
	dev->nvmops.write = NULL;
	dev->nvmops.erase = NULL;
	dev->nvmops.sector_erase = NULL;
	dev->nvmops.validate = NULL;
	dev->nvmops.update = NULL;
	dev->nvmops.acquire = NULL;
	dev->nvmops.release = NULL;

	return SUCCESS;
}

/*******************************************************************************
**
**  Name:           InitNvmParamsI211
**
**  Description:    Initialize the NVM parameters and function pointers for I211.
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        SUCCESS
**
*******************************************************************************/
s32 InitNvmParamsI211(struct PciDevice * dev)
{
	dev->nvmops.acquire = AcquireNvmI210;
	dev->nvmops.release = ReleaseNvmI210;
	dev->nvmops.read = NullWriteNvm;    // @todo: change this later?
	dev->nvmops.write = NullWriteNvm;
	dev->nvmops.validate = NullOpsGeneric;
	dev->nvmops.update = NullOpsGeneric;

	dev->wordsize = NVMSIZE_I211;
	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           InitNvmParams82574()
**
**  Description:    Initialize function pointers
**
**  Arguments:      dev - Pci Device structure
**
**  Returns:        SUCCESS
**
*******************************************************************************/
s32 InitNvmParams82574(struct PciDevice * dev)
{
	dev->nvmops.acquire = GetHwSemaphore82574;
	dev->nvmops.release = PutHwSemaphore82574;
	dev->nvmops.read = ReadNvmEerd;
	dev->nvmops.write = WriteNvmEewr82574;
	dev->nvmops.update = UpdateNvmChecksum82574;
	dev->nvmops.validate = ValidateNvmChecksum82574;

	dev->wordsize = NVMSIZE_82574;
	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           GetNvmWordSize()
**
**  Description:    Calculates the wordsize of the NVM
**
**  Arguments:      dev - Pci Device Structure
**
**  Returns:        Wordsize of the NVM
**
*******************************************************************************/
#define DEFAULT_WORDSIZE  2048
#define DEFAULT_EECD      0x006A3E218

u32 GetNvmWordSize(struct PciDevice * dev)
{
	u32 RegVal = 0;  // value read from register
	u16 Size = 0;    // placeholder to manipulate bits for size

	// read the register
	RegVal = ReadReg(dev, EECD);

	// bit shifting ops to extract size
	Size = (u16) ((RegVal & EECD_SIZE_EX_MASK) >> EECD_SIZE_EX_SHIFT);

	printf("Flash wordsize reg val:  %x register %x\n", Size,RegVal);

	if(Size == 0)
	{
		/*This typically means it is an unprogrammed NVM.  Load up some defaults
		 * to help bootstrap the programming process.  If it doesn't work with
		 * your blank NVM, try changing the DEFAULT_WORDSIZE define and try again
		 * DEFAULT_WORDSIZE and DEFAULT_EECD are immediately above this function */
		printf("Using EECD defaults, is this an unprogrammed NVM?\n");
		if(dev->DeviceId == 0x1531)
			dev->wordsize = 0x100000;
		else dev->wordsize = DEFAULT_WORDSIZE;

		RegVal = WriteReg(dev, EECD, DEFAULT_EECD);
		return(dev->wordsize);
	}

	// offset size
	Size += NVM_WORD_SIZE_BASE_SHIFT;

	// if something weird happened, use the largest shift value
	if(Size > 14)
		Size = 14;

	dev->wordsize = 1 << Size;

	return (dev->wordsize);
}


/*******************************************************************************
**
**  Name:           GetNvmWordSizeX550()
**
**  Description:    Calculates the wordsize of the NVM
**
**  Arguments:      dev - Pci Device Structure
**
**  Returns:        Wordsize of the NVM
**
*******************************************************************************/
u32 GetNvmWordSizeX550(struct PciDevice * dev)
{
	u32 RegVal = 0;  // value read from register
	u16 Size = 0;    // placeholder to manipulate bits for size

	//printf("Entering the function %s\n",__FUNCTION__);

	// read the register
	RegVal = ReadReg(dev, EEC_X550);

	// bit shifting ops to extract size
	Size = (u16) ((RegVal & EECD_SIZE_EX_MASK) >> EECD_SIZE_EX_SHIFT);
	printf("Flash wordsize reg val:  %u register %x\n", Size,RegVal);

	// offset size
	Size += NVM_WORD_SIZE_BASE_SHIFT;

	// if something weird happened, use the largest shift value
	if(Size > 14)
		Size = 14;

	dev->wordsize = 1 << Size;

	return (dev->wordsize);
}

/*******************************************************************************
**
**  Name:           WriteNvmEewr82574()
**
**  Description:    Makes sure previous writes/commands are complete before trying
**		    to write another word.  Checksum needs to be updated after a
**		    call to this function.
**
**  Arguments:      dev    - Device hardware structure
**		    offset - NVM offset to be written to
**		    words  - number of words to write
**		    data   - data buffer containing the words to write
**
**  Returns:        SUCCESS if we completed a write, otherwise a value
**		    indicating failure.
**
*******************************************************************************/
s32 WriteNvmEewr82574(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	u16 i = 0;                  // counter
	u32 RegVal = 0;             // register value
	s32 retval = SUCCESS;       // return value

	/* A check for invalid values:  offset too large, too many words,
	 * and not enough words. */
	if((Offset >= dev->wordsize) || (NumWords > (dev->wordsize - Offset)) || (NumWords == 0))
	{
		printf("nvm parameter(s) out of bounds offset %x wordsize %x numwords %x\n", Offset, dev->wordsize, NumWords);
		return FAIL;
	}

	// loop through the number of words
	for(i = 0; i < NumWords; i++)
	{
		// calculate value to write
		RegVal = (Data[i] << NVM_RW_REG_DATA) | ((Offset + i) << NVM_RW_ADDR_SHIFT) | NVM_RW_REG_START;

		// make sure a write isn't in progress
		retval = PollEerdEewrDone(dev, NVM_POLL_WRITE);

		// if it is, break
		if(retval)
			break;

		// write the value to the register
		WriteReg(dev, EEWR, RegVal);

		// poll until done
		retval = PollEerdEewrDone(dev, NVM_POLL_WRITE);

		if(retval)
			break;

	}

	return retval;
}


/*******************************************************************************
**
**  Name:           ValidateNvmChecksum82574()
**
**  Description:    Verifies device checksum
**
**  Arguments:      dev - Device to update the checksum for
**
**  Returns:        SUCCESS if the checksum is valid, FAIL otherwise
**
*******************************************************************************/
s32 ValidateNvmChecksum82574(struct PciDevice * dev)
{
	s32 retval;         // return value
	u16 Checksum = 0;   // checksum accumulator
	u16 i;                      // counter
	u16 RegData;        // data from nvm

	// loop through all the registers up to the checksum register
	for(i = 0; i < (NVM_CHECKSUM_REG + 1); i++)
	{
		retval = dev->nvmops.read(dev, i, 1, &RegData);

		// check to see if we had an error, if so, return that error
		if(retval)
		{
			printf("NVM Read Error\n");
			return retval;
		}

		// update accumulator
		Checksum += RegData;
	}

	// check to see if we have a valid checksum
	if(Checksum != (u16) NVM_SUM)
	{
		printf("NVM Checksum Invalid\n");
		return FAIL;
	}

	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           UpdateNvmChecksum82574()
**
**  Description:    Updates device checksum
**
**  Arguments:      dev - Device to update the checksum for
**
**  Returns:        SUCCESS upon successful update, status code otherwise
**
*******************************************************************************/
s32 UpdateNvmChecksum82574(struct PciDevice * dev)
{
	s32 retval;         // return value
	u32 Checksum = 0;   // checksum accumulator
	u16 i;                      // counter
	u16 RegData;        // data from nvm

	printf("Updating the checksum.  Seriously!\n");
	// loop through all the registers up to the checksum register
	for(i = 0; i < NVM_CHECKSUM_REG; i++)
	{
		retval = dev->nvmops.read(dev, i, 1, &RegData);

		// check to see if we had an error, if so, return that error
		if(retval)
		{
			printf("NVM Read Error while updating checksum.\n");
			return retval;
		}

		// update accumulator
		Checksum += RegData;
	}

	// calculate the checksum
	Checksum &= 0x00000FFff;
	i = (u16) NVM_SUM - Checksum;

	// write new checksum to checksum register
	retval = dev->nvmops.write(dev, NVM_CHECKSUM_REG, 1, &i);

	printf("checksum was %x\n",i);
	// print an error if we error'ed out
	if(retval)
		printf("NVM Write Error while updating checksum.\n");

	return retval;
}


/*******************************************************************************
**
**  Name:           PollEerdEewrDone()
**
**  Description:    Polls status bit for read/write completion
**
**  Arguments:      dev     - device structure
**		    ee_reg - flag for polling
**
**  Returns:        SUCCESS on read/write complete, FAIL otherwise
**
*******************************************************************************/
s32 PollEerdEewrDone(struct PciDevice * dev, u16 Reg)
{
	u32 MaxAttempts = 100000;   // max poll attempts
	u32 i = 0;                  // counter
	u32 Val = 0;                // register value

	// Loop until we reach the max number of attempts, or until the op is successful
	for(i = 0; i < MaxAttempts; i++)
	{
		// Determine whether the register passed is a read register or write register
		if(Reg == NVM_POLL_READ)
			Val = ReadReg(dev, EERD);
		else
			Val = ReadReg(dev, EEWR);

		// if the done bit is set, return success
		if(Val & NVM_RW_REG_DONE)
			return SUCCESS;

		// delay for a little bit before trying again
		usleep(5);
	}

	return FAIL;
}


/*******************************************************************************
**
**  Name:           NullWriteNvm()
**
**  Description:    Generic NoOp function used as a placeholder
**		    in the nvmOps function pointer table where
**		    certain functions are not valid for specific
**		    devices.
**
**  Arguments:      dev - PciDevice
**
**  Returns:        Null value - placeholder.
**
*******************************************************************************/
s32 NullWriteNvm(struct PciDevice * dev, u32 a, u16 b, u16 * c)
{
	return 0;
}

/*******************************************************************************
**
**  Name:           NullOpsGeneric()
**
**  Description:    Generic NoOp function used as a placeholder
**		    in the nvmOps function pointer table where
**		    certain functions are not valid for specific
**		    devices.
**
**  Arguments:      dev - PciDevice
**
**  Returns:        Null value - placeholder.
**
*******************************************************************************/
s32 NullOpsGeneric(struct PciDevice * dev)
{
	return 0;
}
