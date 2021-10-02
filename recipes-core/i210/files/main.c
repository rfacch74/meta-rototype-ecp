/*******************************************************************************
 * Copyright 2017-2018 Intel Corporation All Rights Reserved.
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
 ****************************************************************************** */


/*******************************************************************************
**  Includes
*******************************************************************************/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "include/common.h"
#include "include/HelperFunctions.h"
#include "include/PciEeprom.h"
#include "include/helpercode.h"
#include "include/testfuncs.h"
#include <asm/types.h>
#include <linux/kernel.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

extern u16 ExitStatus;
u8 MacAddress[12];


/*******************************************************************************
**
**  Name:           BrandingString
**
**  Description:    String array that indexes silicon info
**
*******************************************************************************/
char * BrandingString[] =
{
	"I210",
	"I211",
	"82574",
	"X550",
	"X553"
};


/*******************************************************************************
**
**  Name:           main()
**
**  Description:    Program entry point
**
*******************************************************************************/
int main(int argc, c8* argv[])
{
	u16 CommandIndex = 0;       // default to fail
	u16 NicId = 0;              // nic identifier
	c8 file[80];                        // filename pointer

	MacAddress[0] = 0xFF; //set this to a bad value to show don't change the MAC
	// if no arguments were given, show adapters in system
	if(1 == argc)
		CommandIndex = SHOWADAPTERS;
	// else, parse the command
	else
		CommandIndex = ParseCommand(argc, argv, file, &NicId);

	// if a valid command was given...
	if(0 < CommandIndex)
	{
		// switch based on the desired functionality determined by parsing
		switch(CommandIndex)
		{
			case HELP:
				ShowHelp();
				break;
			case VERSION:
				ShowVersion();
				break;
			case TEST:
				RunTest(file);
				break;
			case DUMP:
				ShowVersion();
				DumpData(NicId);
				break;
			case PROGRAM:
				ShowVersion();
				//@todo testing here
				WriteToDevice(NicId, file);
				break;
			case SHOWADAPTERS:
				DisplayAdapters();
				break;
			case ERASE:
				ShowVersion();
				EraseDevice(NicId);
				break;
			case READLOCKBITS:
				ReadLockBits(NicId);
				break;
			case WRITELOCKBITS:
				WriteLockBits(NicId, file);
				break;
			case REPROGRAM_MAC:
				ShowVersion();
				ReprogramMacAddress(NicId);
				break;
		}
	}

	return 0;
}


/*******************************************************************************
**
**  Name:           DisplayAdapters()
**
**  Description:    Show Intel Adapters found in the system
**
*******************************************************************************/
void DisplayAdapters()
{
	ShowVersion();
	printf("NIC	BUS	DEV	FUN	Silicon	Memory Type Present\n");
	printf("===	===	===	===	=====	======================\n");

	DisplayAllDevices();

}


/*******************************************************************************
**
**  Name:           ShowHelp()
**
**  Description:    Displays helpful information for the user
**
*******************************************************************************/
u16 ShowHelp()
{
	ShowVersion();
	printf("\n");
	printf("Usage:  %s (optional parameters) -NIC=X  <image file name>\n", TOOLNAME);
	printf("When called without any parameters,  %s will display the I210/I211 devices in the system.\n", TOOLNAME);
	printf("The following commands will execute and terminate the program\n");
	printf("-h\n");
	printf("    Displays command line help\n");
	printf("-dump\n");
	printf("	Dumps the NVM contents without programming the image.  \n");
	printf("-ver\n");
	printf("    Displays build information\n");
	printf("\n");
	printf("The following optional commands will be used to modify the program behavior\n");
	printf("Required parameters for programming:\n");
	printf("\n");
	printf("	-nic=X \n");
	printf("	Where X is the device you want to program.\n\tTo see what the choices are run the tool without any parameters.\n");
	printf("\n");
	printf("	-f=<image filename>\n");
	printf("	The file name of the image to be programmed/written into the NVM.\n\tThe file must be in the same directory as the tool.\n");

	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           ShowVersion()
**
**  Description:    Outlines the version and copyright information
**
*******************************************************************************/
u16 ShowVersion()
{
	printf("\nIntel(R) %s NVM/OTP Programming Example Tool   Version %d.%d.%d\n", TOOLNAME, TOOLVER_MAJOR, TOOLVER_MINOR, TOOLVER_BUILD);
	printf("Provided under the terms of a CNDA.  Do Not Distribute.\n");
	printf("Copyright(C) 2017-2019 by Intel(R) Corporation \n");
	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           RunTest()
**
**  Description:    The new OTP image will be not be written to hardware
**		    but will be displayed on screen
**
*******************************************************************************/
u16 RunTest(c8* fname)
{

	printf("Test run...\nNot Implemented\n");

	return SUCCESS;
}


/*******************************************************************************
**
**  Name:           DumpData()
**
**  Description:    Dumps the OTP contents into a file
**
**  Arguments:      NicId    - A specified device to that we want to dump
**			       the NVM contents of
**
**  Returns:        Status code.
**
*******************************************************************************/
u16 DumpData(u16 NicId)
{
	u32 DumpRow = 0;                            // current dump row
	u32 DumpCol = 0;                            // current dump column
	u16 retval = FAIL;                                  // return value
	u32 i = 0;                                          // index
	c8* FileNamePointer = NULL;                 // file name
	u16* Eeprom_Data = NULL;                    // array for eeprom data
	u32* Otp_Data = NULL;                               // array for otp data
	FILE* FilePointer = NULL;                   // file pointer
	c8 EepromFile[100] = {'\0'};                // eeprom file
	struct  PciDevice Device;                           // device structure


	/* Read the whole EEPROM/NVM/OTP and dump the contents
	 * into a file. Name of the file works like this:
	 * <name>.OTP
	 * <NAME> is the silicon type (I210/I211/82574) followed by
	 * 'NIC' and the NIC number .
	 * For example for an I210 which is NIC 3 it would dump into
	 * the file I210NIC3.OTP.
	 * For an I211 as the only adapter in the system it would
	 * dump into I211NIC1.OTP.
	 * NOTE:  Since the OTP may not be programmed, using other
	 * identifiers like MAC address cannot be used
	 */

	PciDevice(&Device);

	if(0 != NicId)
		Device.NicId = NicId;

	GetDevice(&Device);

	if(!Device.IsMapped)
	{
		DisplayError(13);
		CloseDevice(&Device);
		return FAIL;
	}
	// Determine what type of NVM is present in the device.
	DetermineNvmType(&Device);

	sprintf(EepromFile, "./%sNIC%d.otp", BrandingString[(u8) SiliconName(Device.DeviceId)],NicId);
	FileNamePointer = EepromFile;

	// Make sure we have a valid filename
	if(*FileNamePointer != '\0')
	{
		// open the file for writing
		FilePointer = fopen(EepromFile, "w");

		// make sure we were able to open the file
		if(FilePointer == NULL)
		{
			printf("Could not open file.\n");
		}
		else
		{
			/* begin switching dump method based on the type of
			 * NVM that was found (flash or iNVM) */

			if((1 == Device.NvmPresent) || (1 == Device.NvmPresentX550))
			{
				printf("Found an NVM\n");
				// Get NVM Size
				//Device.wordsize = GenericDetectEepromSize(&Device);

				// Allocate an array for the local buffer
				Eeprom_Data = (u16*) calloc(Device.wordsize,sizeof(u16));

				// make sure we were able to allocate the array
				if(NULL == Eeprom_Data)
				{
					printf("Could not get memory for actions.  Terminating\n");
					CloseDevice(&Device);
					fclose(FilePointer);
					return FAIL;
				}

				// Loop through eeprom, loading it into a local buffer
				for(i = 0; i < Device.wordsize; i += BURST_SIZE)
				{
					Device.nvmops.read(&Device, i, BURST_SIZE, Eeprom_Data + i);
				}

				if(IsX553(&Device))
				{
					u16 checksum = Eeprom_Data[0x3f];
					u16 mac1[4];
					u16 mac2[4];

					memcpy(&mac1, &Eeprom_Data[0x101], sizeof(mac1));
					memcpy(&mac2, &Eeprom_Data[0x109], sizeof(mac2));
					printf("checksum: %#04x\n", checksum);
					printf("mac1: %04x %04x %04x %04x\n", mac1[0], mac1[1], mac1[2], mac1[3]);
					printf("mac2: %04x %04x %04x %04x\n", mac2[0], mac2[1], mac2[2], mac2[3]);
				}

				// Time to dump it pretty style, Flush often to save it to file in case something goes wrong.
				for(DumpRow = 0; DumpRow < (Device.wordsize / 8); DumpRow++)
				{
					for(DumpCol = 0; DumpCol < EEPROM_MAX_COLS; DumpCol++)
					{
						// get the value at the specified coordinate
						u16 val = GET_NVM_VALUE_AT_COORDINATE(Eeprom_Data, DumpRow, DumpCol);

						// Don't put a space after the last word in the row...
						// otherwise, put a space
						if(DumpCol + 1 == EEPROM_MAX_COLS)
						{
							fprintf(FilePointer, "%04X", val);
							fflush(FilePointer);
						}
						else
						{
							fprintf(FilePointer, "%04X ", val);
						}
					} //for DumpCol

					// newline at the end of the row
					if(DumpRow < (Device.wordsize / 8) - 1)
					{
						fprintf(FilePointer, "\n");
					}
				} //for DumpRow

				// free any memory we allocated
				if(NULL != Eeprom_Data)
				{
					free(Eeprom_Data);
					Eeprom_Data = NULL;
				}
				retval = SUCCESS;
			} //if DevicePresent
			else if(1 == Device.InvmPresent)
			//if (INTEL_I211 == Device.DeviceId || INTEL_I211_BLANK == Device.DeviceId)
			{
				Device.wordsize = INVM_SIZE;

				Otp_Data = malloc(Device.wordsize * sizeof(u32));

				if(Otp_Data)
				{
					DumpInvmI211(&Device, &Otp_Data[0]);

					for(DumpRow = 0; DumpRow < (Device.wordsize / 8); DumpRow++)
					{
						for(DumpCol = 0; DumpCol < EEPROM_MAX_COLS; DumpCol++)
						{
							// if we're at the last column, don't put a space at the end of the word
							// else, put a space after each word
							if(DumpCol + 1 == EEPROM_MAX_COLS)
							{
								fprintf(FilePointer, "%08X", GET_NVM_VALUE_AT_COORDINATE(Otp_Data, DumpRow, DumpCol));

								fflush(FilePointer);
							}
							else
							{
								fprintf(FilePointer, "%08X ", GET_NVM_VALUE_AT_COORDINATE(Otp_Data, DumpRow, DumpCol));
							}
						}

						// print a newline at the end of each row
						if(DumpRow < (Device.wordsize / 8) - 1)
						{
							fprintf(FilePointer, "\n");
						}
					} //for

					if((Otp_Data[0] | Otp_Data[1] | Otp_Data[2] | Otp_Data[3]) == 0)
					{
						fprintf(FilePointer, "\nOTP is BLANK!\n");
						printf("Device is BLANK!\n");
					}

					// free the array we allocated
					free(Otp_Data);
					Otp_Data = NULL;
				}
				retval = SUCCESS;

			} //if INVM Present
			fclose(FilePointer);
		} //else file pointer is null
	} //if filepointer


	CloseDevice(&Device);

	return retval;
}


/*******************************************************************************
**
**  Name:           WriteToDevice()
**
**  Description:    Branch based on whether we want to program iNVM (OTP) or NVM
**
**  Arguments:      NicId - A specified device to that we want to program
**		    fname - Name of the file that holds the data we wish use
**			    to program the device
**
**  Returns:        Status code.
**
*******************************************************************************/
u16 WriteToDevice(u16 NicId, c8 * fname)
{
	s16 retval = SUCCESS;                               // return value
	struct PciDevice Device;                                    // Device structure

	PciDevice(&Device);
	// Save Nic ID to the device structure
	if(0 != NicId)
		Device.NicId = NicId;

	// Get the correct device and initialize it
	GetDevice(&Device);

	if(!Device.IsMapped)
	{
		DisplayError(13);
		CloseDevice(&Device);
		return FAIL;
	}
	// Determine what type of NVM is present in the device.
	DetermineNvmType(&Device);

	// Select the right programming method depending on what
	// kind of NVM is present
	if(1 == Device.NvmPresent)
	{
		WriteNvm(&Device, fname);
	}
	else if(1 == Device.InvmPresent)
	{
		WriteOtp(&Device, fname);
	}
	else if(1 == Device.NvmPresentX550)
	{
		WriteNvmX550(&Device, fname);
	}

	// Close down the device
	CloseDevice(&Device);

	return retval;
}

/*******************************************************************************
**
**  Name:           EraseDevice()
**
**  Description:    Initialize device and determine if it has NVM
**
**  Arguments:      NicId - A specified device to that we want to program
**
**  Returns:        Status code.
**
*******************************************************************************/
s8 EraseDevice(u16 NicId)
{
	s8 retval = SUCCESS;                                // return value
	struct PciDevice Device;                                    // Device structure

	PciDevice(&Device);
	// Save Nic ID to the device structure
	if(0 != NicId)
		Device.NicId = NicId;

	// Get the correct device and initialize it
	GetDevice(&Device);
	if(!Device.IsMapped)
	{
		DisplayError(13);
		CloseDevice(&Device);
		return FAIL;
	}

	// Determine what type of NVM is present in the device.
	DetermineNvmType(&Device);

	if(1 == Device.NvmPresent)
	{
		printf("Erasing!\n");
		EraseNvm(&Device);
		printf("done!\n");
	}
	else
	{
		DisplayError(12);
	}

	CloseDevice(&Device);

	return retval;
}

/*******************************************************************************
**
**  Name:           ReadLockBits()
**
**  Description:    Initialize device before reading lock bits
**
**  Arguments:      NicId - A specified device to that we want to program
**
**  Returns:        Status code.
**
*******************************************************************************/
s8 ReadLockBits(u16 NicId)
{
	s8 retval = SUCCESS;                                // return value
	struct PciDevice Device;                                    // Device structure

	PciDevice(&Device);
	// Save Nic ID to the device structure
	if(0 != NicId)
		Device.NicId = NicId;

	// Get the correct device and initialize it
	GetDevice(&Device);
	if(!Device.IsMapped)
	{
		DisplayError(13);
		CloseDevice(&Device);
		return FAIL;
	}
	DetermineNvmType(&Device);
	if(!Device.InvmPresent)
	{
		DisplayError(14);
		CloseDevice(&Device);
		return FAIL;
	}

	ReadInvmLockBits(&Device);

	CloseDevice(&Device);

	return retval;
}

/*******************************************************************************
**
**  Name:           WriteLockBits()
**
**  Description:    Initialize device before writing lock bits
**
**  Arguments:      NicId - A specified device to that we want to program
**		    fname - Name of the file that holds the data we wish use
**			    to program the device
**
**  Returns:        Status code.
**
*******************************************************************************/
s8 WriteLockBits(u16 NicId, c8 * fname)
{
	s8 retval = SUCCESS;                                // return value
	struct PciDevice Device;                                    // Device structure

	PciDevice(&Device);
	// Save Nic ID to the device structure
	if(0 != NicId)
		Device.NicId = NicId;

	// Get the correct device and initialize it
	GetDevice(&Device);
	if(!Device.IsMapped)
	{
		DisplayError(13);
		CloseDevice(&Device);
		return FAIL;
	}
	DetermineNvmType(&Device);
	if(!Device.InvmPresent)
	{
		DisplayError(14);
		CloseDevice(&Device);
		return FAIL;
	}

	WriteInvmLockBits(&Device, fname);

	CloseDevice(&Device);

	return retval;
}

/*******************************************************************************
**
**  Name:           ReprogramMacAddress()
**
**  Description:    Example code to reprogram only the MAC address.
**					Code needs to be modified to fit your architecture,
**					current code is for Denverton
**
**  Arguments:      NicId - A specified device whose MAC address needs to be
**								programmed
**
**  Returns:        Status code.
**
*******************************************************************************/
u16 ReprogramMacAddress(u16 NicId)
{
	u16 ret = FAIL;
	u32 i = 0;
	u16                           * eepromData = NULL;
	u16 port0_mac_addr_loc = 0;
	u16 port1_mac_addr_loc = 0;
	u16 port0_alt_mac_addr_loc = 0;
	u16 port1_alt_mac_addr_loc = 0;
	struct PciDevice Device;

	printf("Sample code to program MAC address in Denverton!\n");

	assert(MacAddress[0] != 0xff);

	PciDevice(&Device);
	if(0 != NicId)
	{
		Device.NicId = NicId;
	}
	GetDevice(&Device);

	if(Device.DeviceId == INTEL_X553_SGMII_BACKPLANE2)
	{
		printf("overriding SGMII_BACKPLANE2 detected device ID\n");
		Device.DeviceId = INTEL_X553_SGMII_BACKPLANE1;
	}
	DetermineNvmType(&Device);

	if(!IsX553(&Device) || Device.NvmPresentX550 != 1)
	{
		printf("Unsupported NIC or platform for MAC programming.\n");
		ret = FAIL;
		goto finish;
	}

	printf("allocating scratch buffer to device wordsize\n");
	eepromData = (u16 *) calloc(Device.wordsize, sizeof(u16));
	if(eepromData == NULL)
	{
		printf("failed to allocate eeprom buffer\n");
		ret = FAIL;
		goto finish;
	}

	printf("reading from nvm\n");
	for(i = 0; i < Device.wordsize; i++) {
		Device.nvmops.read(&Device, i, 1, &eepromData[i]);
	}

	printf("calculating MAC\n");

	u8 newMac[6];

	for(i = 0; i < 12; i += 2) {
		u8 c1 = MacAddress[i], c2 = MacAddress[i + 1];
		newMac[i / 2] = (c1 << 4) | c2;
	}

	printf("rewriting MAC\n");

	bool useFirstMac = NicId % 2 != 0;

#if 0 // Reads Pointers for address location. Inconsistent results. Not sure why, but how it should be done.
	Device.nvmops.read(&Device, X553_LAN_CORE0_PTR_ADDR, 1, &port0_mac_addr_loc);
	Device.nvmops.read(&Device, X553_LAN_CORE1_PTR_ADDR, 1, &port1_mac_addr_loc);

	port0_mac_addr_loc += X553_MAC_ADDR_OFFSET;
	port1_mac_addr_loc += X553_MAC_ADDR_OFFSET;

	u16 macLoc = useFirstMac ? port0_mac_addr_loc : port1_mac_addr_loc;

	Device.nvmops.read(&Device, X553_ALT_MAC_ADDR_PTR_ADDR, 1, &port0_alt_mac_addr_loc);

	port1_alt_mac_addr_loc = port0_alt_mac_addr_loc + X553_ALT_MAC_ADDR_PORT1_OFFSET;

	u16 altMacLoc = useFirstMac ? port0_alt_mac_addr_loc : port1_alt_mac_addr_loc;

	memcpy(&eepromData[macLoc], &newMac, sizeof(newMac));
	memcpy(&eepromData[altMacLoc], &newMac, sizeof(newMac));
#else // Hard Coded Addresses
	port0_mac_addr_loc = X553_PORT0_MAC_ADDR_LOCATION;
	port1_mac_addr_loc = X553_PORT1_MAC_ADDR_LOCATION;

	u16 macLoc = useFirstMac ? port0_mac_addr_loc : port1_mac_addr_loc;

	port0_alt_mac_addr_loc = X553_PORT0_ALT_MAC_ADDR_LOCATION;

	port1_alt_mac_addr_loc = port0_alt_mac_addr_loc + X553_ALT_MAC_ADDR_PORT1_OFFSET;

	u16 altMacLoc = useFirstMac ? port0_alt_mac_addr_loc : port1_alt_mac_addr_loc;

	memcpy(&eepromData[macLoc], &newMac, sizeof(newMac));
	memcpy(&eepromData[altMacLoc], &newMac, sizeof(newMac));
#endif

#if 0
	printf("overwriting user-supplied MAC with hardcoded values for testing\n");
	u8 mac1[6] = {0x98, 0x4f, 0xee, 0x13, 0xb8, 0x2e};
	u8 mac2[6] = {0x98, 0x4f, 0xee, 0x13, 0xb8, 0x2f};
	memcpy(&eepromData[0x101], &mac1, sizeof(mac1));
	memcpy(&eepromData[0x109], &mac2, sizeof(mac2));
#endif

	printf("writing NVM\n");

	for(i = macLoc; i < macLoc + 3; i++) {
		Device.nvmops.write(&Device, i, 1, &eepromData[i]);
	}

	for(i = altMacLoc; i < altMacLoc + 3; i++) {
		Device.nvmops.write(&Device, i, 1, &eepromData[i]);
	}

	printf("updating checksum\n");

	free(eepromData);

	ret = Device.nvmops.update(&Device);
	if(ret != SUCCESS)
	{
		printf("Failed to update NVM\n");
	}

finish:
	CloseDevice(&Device);
	return ret;
}
