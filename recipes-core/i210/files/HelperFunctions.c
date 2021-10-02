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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include "include/common.h"
#include "include/HelperFunctions.h"
#include "include/PciEeprom.h"
#include "include/helpercode.h"

extern u8 MacAddress[12];


/*******************************************************************************
**
**  Name:           ErrorDefinitionsTable
**
**  Description:    Pairs error codes with a description that gives more detail
**		    about the error(s) encountered at runtime.
**
*******************************************************************************/
ErrorDefinition ErrorDefinitionsTable[] =
{
	{0, "Program terminated successfully"},
	{1, "Unknown error encountered."},
	{2, "Unknown parameter used."},
	{3, "Invalid NIC specified."},
	{4, "Invalid filename length specified."},
	{5, "Failure to allocate space for filename."},
	{6, "Required parameter missing - make sure a filename and nic are both specified."},
	{7, "Unknown parameter encountered, exiting now."},
	{8, "Filename cannot begin with a '-'."},
	{9, "Device proc location too long, could not parse device list."},
	{10, "Unable to read devices, could not open devices file."},
	{11, "Not enough space to add MAC address."},
	{12, "NVM not detected."},
	{13, "Device is not mapped."},
	{14, "Device does not have INVM."},
	{15, "Invalid MAC address length given, must be 12 characters!"},
	{16, "Invalid MAC address value!"},
	{17, "Programming failed!"},
	{18, "Non-overwritable data detected in the iNVM!  No changes made.  Programming failed!"}
};


/*******************************************************************************
**
**  Name:           ValidParametersTable
**
**  Description:    Helps define valid parameters and how they should be
**		    used, especially in combination with one another.
**
**		    The format is as follows:
**		    {paramater, Combo ID, singular, paramlength, function}
**
*******************************************************************************/
ValidParam ValidParametersTable[] =
{
	{"-H",                  1,    1, 2,   HELP},
	{"-VER",                2,    1, 4,   VERSION},
	{"-TEST",               4,    0, 5,   TEST},
	{"-DUMP",               8,    0, 5,   DUMP},
	{"-NIC=",              16,    0, 5,   PROGRAM},
	{"-F=",                32,    0, 3,   PROGRAM},
	{"-MAC=",              64,    0, 5,   PROGRAM},
	{"-ERASE",            128,    0, 6,   ERASE},
	{"-READLOCKBITS",     256,    0, 13,  READLOCKBITS},
	{"-WRITELOCKBITS",    512,    0, 14,  WRITELOCKBITS}
};


/*******************************************************************************
**
**  Name:           DisplayError()
**
**  Description:    Displays information about errors encountered at runtime.
**
**  Arguments:      ErrorStatus  - numeric descriptor of errors
**
*******************************************************************************/
void DisplayError(u16 ErrorStatus)
{
	printf("%s\n", ErrorDefinitionsTable[ErrorStatus].name);
}

/*******************************************************************************
**
**  Name:           Convert2Hex()
**
**  Description:    Converts input char to hex value.  so 'A' becomes 0x0A
**
**  Arguments:      InputChar - value to convert.
**
*******************************************************************************/
u8 Convert2Hex(u8 InputChar)
{
	if((InputChar >= '0') && (InputChar <= '9'))
		return (InputChar - '0');
	if((InputChar >= 'A') && (InputChar <= 'F'))
		return ((InputChar - 'A') + 10);
	return(0xFF);
}

/*******************************************************************************
**
**  Name:           ParseCommand()
**
**  Description:    Translates parameters obtained from command line program
**		    invocation to tool functionality.
**
**  Arguments:      argc   - Image Combi to convert to string
**                  argv[] - Output string formatted like: pxe+efi+iscsi
**                  file   -  IN:  Placeholder char* for parsed file name
**                            OUT: Parsed file name as given on command line,
**				   returned as a string
**		    nic    -  IN:  Placeholder var for parsed nic ID
**                            OUT: Parsed nic ID as given on command line
**
**  Returns:        u16 CommandIndex - tool function as parsed
**				       from the command line invocation
**
*******************************************************************************/
u16 ParseCommand(u16 argc, c8 * argv[], c8 * file, u16* nic)
{
	u16 CommandIndex = 0;                       // default to fail
	u16 Param = 1;                              // param 0 is the program invocation itself
	u16 ActionsPresent = 0;
	u16 NicId = 0;                              // temp var for nic id parsing
	u16 SingularParamFound = 0;         // flag to set if a singular command parameter is found
	u16 Exit = 0;                                       // used as a loop flag to break on high value
	u16 i = 0;                                          // loop variable
	u16 idx = 0;                                        // loop variable
	u16 FilenameLen = 0;
	s16 ArgLen = 0;
	c8 * TempString = NULL;
	c8 * InputString = 0;                       // temp pointer used for parsing

	/* while the current parameter we're looking at is less than the number of
	   parameters supplied && break flag is low value */
	while((Param < argc) && (0 == Exit))
	{
		// convert the parameter to uppercase
		TempString = (c8 *) malloc((strlen(argv[Param]) + 1));
		if(!TempString)
		{
			printf("malloc error\n");
			Exit = 1;
			break;
		}

		TempString = strcpy(TempString, argv[Param]);

		Strupr(argv[Param]);

		i = 0;

		// while loop used to loop through each of the known possible parameters
		while(i < KNOWN_PARAMS)
		{
			/* if we have a match between the string argument supplied, and the
			   parameter string in the valid parameters table */
			if(strcmp(argv[Param], ValidParametersTable[i].Param) == 0)
			{
				// check to see if the parameter is a singular parameter
				if(1 == ValidParametersTable[i].Singular)
				{
					/* if a singular parameter is found, set the commandIndex
					   to perform the action associated with that parameter.
					   Set SingularParamFound to true, set the exit status to true
					   and end command line parsing by setting the Exit flag.
					   Finally, break from the loop. */
					CommandIndex = ValidParametersTable[i].Function;
					SingularParamFound = 1;
					Exit = 1;
					break;
				}
				else if(TEST_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= ValidParametersTable[i].ParamId;
				}
				else if(DUMP_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= DUMP_ID;
				}
				else if(ERASE_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= ERASE_ID;
				}
				else if(READLOCKBITS_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= READLOCKBITS_ID;
				}
				else if(WRITELOCKBITS_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= WRITELOCKBITS_ID;
				}
			}
			/* if we have a match between the string argument supplied, and the
			   parameter string in the valid parameters table taking the length into account */
			else if(strncmp(argv[Param], ValidParametersTable[i].Param,
			                ValidParametersTable[i].ParamLength) == 0)
			{
				// if the parameter is describing the NIC id
				if(NIC_ID == ValidParametersTable[i].ParamId)
				{
					// temp pointer used to parse the argument
					InputString = argv[Param];

					// make sure there's data being pointed to!
					if(InputString && InputString[ValidParametersTable[i].ParamLength])
					{
						/* atoi expects a pointer to convert to an integer, so we use the
						   index that corresponds to where the nicID should be, and convert
						   that to a pointer so atoi is happy.  */
						NicId = atoi(&InputString[ValidParametersTable[i].ParamLength]);
						//printf("NIC id given:  %u\n", NicId);      /*uncomment for debugging*/
					}
					// check to ensure an erroneous nicId was not given
					if((0 >= NicId) || (MAX_NICS < NicId) || (NicId > SupportedNicsFound()))
					{
						DisplayError(3);
						Exit = 1;
						break;
					}
					else
					{
						/* flag that a valid nic ID was given as part of the command line
						    invocation, then set the variable passed to this function so it
						   contains the nic id */
						ActionsPresent |= ValidParametersTable[i].ParamId;
						//printf("nic ok:  actions present = %x\n", ActionsPresent);
						*nic = NicId;
					}
				}
				// if the parameter is describing the file to use
				else if(FILE_ID == ValidParametersTable[i].ParamId)
				{
					// temp pointer used to parse the argument
					InputString = argv[Param];

					/* the filename is a a subset of the total argument given,
					   so here we calculate that length */
					FilenameLen = strlen(argv[Param]) -
					              (ValidParametersTable[i].ParamLength);

					// make sure the filename is of an appropriate length
					if((FilenameLen < 1) || (MAX_FILELEN < FilenameLen))
					{
						DisplayError(4);
						Exit = 1;
						break;
					}
					// make sure there's data being pointed to!
					else if(InputString && InputString[ValidParametersTable[i].ParamLength])
					{
						// point InputString to the start of where the filename begins
						InputString = TempString + ValidParametersTable[i].ParamLength;

						// check for an empty buffer
						if(!file)
						{
							DisplayError(5);
							Exit = 1;
							break;
						}
						// make sure the filename doesn't begin with a '-'
						else if('-' == InputString[0])
						{
							DisplayError(8);
							Exit = 1;
							break;
						}
						// if we reached this else statement, the filename is okay
						else
						{
							for(idx = 0; idx < FilenameLen + 1; ++idx)
							{
								file[idx] = *InputString;
								++InputString;
							}

							/* flag that a valid filename was given as
							 * part of the command line invocation */
							ActionsPresent |= FILE_ID;
						}
					}
				}
				else if(MAC_ID == ValidParametersTable[i].ParamId)
				{
					// temp pointer used to parse the argument
					InputString = argv[Param];

					// the filename is a a subset of the total argument given,
					//  so here we calculate that length
					ArgLen = strlen(argv[Param]) -
					         (ValidParametersTable[i].ParamLength);

					// make sure the filename is of an appropriate length
					if(ArgLen != 12)
					{
						DisplayError(15);
						Exit = 1;
						break;
					}
					// make sure there's data being pointed to!
					else if(InputString &&
					        InputString[ValidParametersTable[i].ParamLength])
					{
						// point InputString to the start of where the filename begins
						InputString = TempString + ValidParametersTable[i].ParamLength;

						Strupr(InputString);

						if(strncmp(InputString, ZERO_MAC, MAC_LEN) == 0 || strncmp(InputString, FF_MAC, MAC_LEN) == 0)
						{
							DisplayError(16);
							Exit = 1;
							break;
						}

						for(idx = 0; idx < ArgLen; ++idx)
						{
							MacAddress[idx] = Convert2Hex(InputString[idx]);
							//printf("Input %C %x\n", InputString[idx], MacAddress[idx]);
						}

						ActionsPresent |= MAC_ID;
					}
				}
				// else if the parameter indicates to run the test functionality
				else if(TEST_ID == ValidParametersTable[i].ParamId)
					/* flag the actions present to indicate test run */
					ActionsPresent |= ValidParametersTable[i].ParamId;
				// else if dump parameter was supplied
				else if(DUMP_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= DUMP_ID;
					//printf(" ok:  actions present = %x\n", ActionsPresent);
				}
				// else if erase parameter was supplied
				else if(ERASE_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= ERASE_ID;
				}
				// else if readlockbits parameter was supplied
				else if(READLOCKBITS_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= READLOCKBITS_ID;
				}
				// else if writelockbits parameter was supplied
				else if(WRITELOCKBITS_ID == ValidParametersTable[i].ParamId)
				{
					ActionsPresent |= WRITELOCKBITS_ID;
				}
				// else, unknown command
				else
				{
					DisplayError(7);
					Exit = 1;
				}
			}

			/* increment i so we can try to find a match for the
			 * command in the list of known commands */
			++i;
		}
		if(TempString)
		{
			free(TempString);
			TempString = NULL;
		}
		// increment Param so we can parse the next command in argv
		++Param;
	}

	// uncomment for debugging
	//printf("ActionsPresent: %d\n\n", ActionsPresent);

	// if we did not encounter a single param during parsing...
	if((0 == SingularParamFound) && (!Exit))
	{
		/* check to see if RUN_TEST was present, if so, set CommandIndex to indicate
		   indicate function */
		if(RUN_TEST == ActionsPresent)
		{
			CommandIndex = TEST;
		}
		// else if, check if writing to the device functionality has been invoked
		else if(WRITE_TO_DEVICE == ActionsPresent || WRITE_TO_DEVICE_MAC == ActionsPresent)
		{
			CommandIndex = PROGRAM;
		}
		else if(DUMP_DEVICE == ActionsPresent)
		{
			CommandIndex = DUMP;
		}
		else if(ERASE_DEVICE == ActionsPresent)
		{
			CommandIndex = ERASE;
		}
		else if(READLOCKBITS_MASK == ActionsPresent)
		{
			CommandIndex = READLOCKBITS;
		}
		else if(WRITELOCKBITS_MASK == ActionsPresent)
		{
			CommandIndex = WRITELOCKBITS;
		}
		else if(REPROGRAM_MAC_MASK == ActionsPresent)
		{
			CommandIndex = REPROGRAM_MAC;
		}
		/* else if, we didn't get both parameters necessary for device programming,
		   indicate error */
		else if((NIC_ID == ActionsPresent) || (FILE_ID == ActionsPresent))
		{
			DisplayError(6);
		}
	}

	// return command function as parsed from the command line invocation
	return CommandIndex;
}

/*******************************************************************************
**
**
**  Name:           SupportedNicsFound()
**
**  Description:    Counts the number of supported INTEL devices found
**
**  Returns  :      Number of supported Intel devices in the system
**
**
*******************************************************************************/
u16 SupportedNicsFound()
{
	FILE * devices = NULL;                                      // file pointer for PCI devices
	c8 temp[512];                                               // temp array
	u32 OverflowCheck = OVERFLOW_TOKEN;
	u16 FileSize = sizeof(temp) - 1;                            // size of PCI device file
	u16 DeviceCount = 0;                                        // number of supported Intel devices found
	u32 VenDevInfo = 0;                                         // word that contains vendor/device id info
	u16 VendorId = 0;                                           // parsed vendor ID
	u16 DeviceId = 0;                                           // parsed device ID
	u16 Data = 0;                                               // Temp var

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
	}

	// iterate through the device list until we find our device of interest
	while(devices && (NULL != fgets(temp, FileSize, devices)))
	{
		sscanf(temp, "%hx %x", &Data, &VenDevInfo);

		// extract vendor and device info from the VenDevInfo word
		DeviceId = VenDevInfo & BITMASK;
		VendorId = VenDevInfo >> 16U;

		// For each intel device found, increment the supported device count
		if((INTEL_ID == VendorId))
		{
			switch(DeviceId)
			{
				case INTEL_82574:
				case INTEL_I210_AT_EMP:
				case INTEL_I210_AT_OEM:
				case INTEL_I210_BLANK:
				case INTEL_I210_AS_FIB:
				case INTEL_I210_AS_1:
				case INTEL_I210_AS_2:
				case INTEL_I210_AT_RET:
				case INTEL_I210_AS_OEM:
				case INTEL_I210_SGMII_FLASHLESS:
				case INTEL_I211:
				case INTEL_I211_BLANK:
				case INTEL_X550_OEM:
				case INTEL_X550_VF1:
				case INTEL_X550_VF2:
				case INTEL_X550_T1:
					++DeviceCount;
					break;
			}//switch
		}//if
	} //while

	if(devices)
		fclose(devices);

	if(OVERFLOW_TOKEN != OverflowCheck)
		printf("Overflow occured...\n");

	return DeviceCount;
}


/*******************************************************************************
**
**  Name:           Strupr()
**
**  Description:    Converts a string to all uppercase
**
**  Arguments:      string1  - pointer to string to be converted to uppercase
**
*******************************************************************************/
void Strupr (c8 * string1)
{
	u16 i = 0;

	while((string1[i] != '\0') && (i < 0xFFFF))
	{
		if(string1[i] > 0x60 && string1[i] < 0x7B)
			string1[i] -= 0x20;
		i++;
	}
}


/*******************************************************************************
**
**  Name:           SiliconName()
**
**  Description:    Displays information
**
**  Arguments:      DeviceId 	- the device ID of the hardware in question
**
**  Returns:        char RetVal - a value corresponding to the silicon for
**				  the given DeviceId
**
*******************************************************************************/
u8 SiliconName(u16 DeviceId)
{
	u8 RetVal = -1;

	switch(DeviceId)
	{
		case  INTEL_82574:
			RetVal = 2;
			break;

		case INTEL_I210_BLANK:
		case INTEL_I210_AT_OEM:
		case INTEL_I210_AT_EMP:
		case INTEL_I210_AS_FIB:
		case INTEL_I210_AS_1:
		case INTEL_I210_AS_2:
		case INTEL_I210_AT_RET:
		case INTEL_I210_AS_OEM:
		case INTEL_I210_SGMII_FLASHLESS:
			RetVal = 0;
			break;

		case  INTEL_I211_BLANK:
		case  INTEL_I211:
			RetVal = 1;
			break;

		case INTEL_X550_OEM:
		case INTEL_X550_VF1:
		case INTEL_X550_VF2:
		case INTEL_X550_T1:
			RetVal = 3;
			break;

		case INTEL_X553_SGMII_BACKPLANE1:
		case INTEL_X553_SGMII_BACKPLANE2:
		case INTEL_X553_KX:
		case INTEL_X553_SGMII_1:
		case INTEL_X553_SGMII_2:
			RetVal = 4;
			break;
	}

	return RetVal;
}

/******************************************************************************
**
**  Name:           EeReadLineFromEepFile()
**
**  Description:    This function reads one line from
**		    the file and strips comments.  This method does not
**		    check the correctness of arguments supplied.
**
**  Arguments:      DataFile    - Opened file to read from
**                  Buffer      - IN:  Allocated buffer for the line
**				       to be stored into
**                                OUT: Line from .eep file
**                  BufferSize  - Size of the input buffer
**
**  Returns:        Status Code: SUCCESS - When line was successfully read
**                               FAIL    - There was a problem reading the line
**
******************************************************************************/
u16 EeReadLineFromEepFile(FILE* DataFile, c8 * Buffer, u32 BufferSize)
{
	u32 Sign = 0;
	u16 StringIndex = 0;
	u16 EeStatus = SUCCESS;
	boolean EndOfLine = FALSE;
	boolean Comment = FALSE;

	do
	{
		/* Read line from the file. If the line is longer
		 * it will be serviced below */
		if(fgets(Buffer, BufferSize, DataFile) == NULL)
		{
			EeStatus = FAIL;
			break;
		}

		/* Look for:
		 * a semicolon in the string indicating a comment
		 * a EOL just for detection if this is a full line
		 * read to the buffer */
		for(StringIndex = 0; StringIndex < BufferSize; StringIndex++)
		{
			/* Break the loop if string has ended */
			if(Buffer[StringIndex + 1] == '\0')
			{
				break;
			}

			/* Detect end of line and comments */
			if(Buffer[StringIndex + 1] == '\n' ||
			   Buffer[StringIndex + 1] == '\r' ||
			   Buffer[StringIndex + 1] == ';')
			{
				/* Determine which we encountered...EOL or
				 * comment...add terminating NULL as needed */
				if(Buffer[StringIndex + 1] == ';')
				{
					Comment = TRUE;
					Buffer[StringIndex + 1] = '\0';
				}
				else
				{
					EndOfLine = TRUE;
				}
			}
		}

		/* If the end of line was not detected - read the rest of
		 * line and omit it if it's a comment */
		if(EndOfLine == FALSE)
		{
			if(Comment == TRUE)
			{
				while((Sign = getc(DataFile)) != EOF)
				{
					if(Sign == (u32) '\n' || Sign == (u32) '\r')
					{
						break;
					}
				}
			}
			/* If data in buffer does not contain the whole line
			 * (it does not contain comment)
			 * then it will be read in next step */
		}

		/* If this line is empty, clear local flags */
		if(*Buffer == '\0')
		{
			EndOfLine = FALSE;
			Comment = FALSE;
		}
	} while (*Buffer == '\0');

	return EeStatus;
}


/*******************************************************************************
**
**  Name:           GenericDetectEepromSize()
**
**  Description:    Implements an algorithm to detect the size
**	            of an EEPROM specified by the input.
**
**  Arguments:      Dev - the device ID of the hardware in question
**
**  Returns:        SizeInWords - a value corresponding to the number
**				  of words fouund in the hardware's
**				  EEPROM
**
*******************************************************************************/
u16 GenericDetectEepromSize(struct PciDevice * Dev)
{
	u32 i = 0;                           // counter
	u32 TestingLimit = 0;                // max value
	u16 OldValue = 0;                    // save previous values
	u16 WordValue = 0;                   // place to read to
	u32 SizeInWords = 0;                 // eeprom size in words
	boolean SizeDetected = FALSE;        // stopping point for our loop

	/* Prior to entering this function, the shared code
	 * EEPROM word size value should be set to
	 * BIGGEST_POSSIBLE_EEPROM in order for the
	 * test writes/reads not fail based on offset range */
	TestingLimit = BIGGEST_POSSIBLE_EEPROM;

	do
	{
		SizeInWords = TestingLimit;

		/* Read the last EEPROM word at the biggest EEPROM offset.
		 * If this is beyond our EEPROM, it will read the last
		 * word anyway because read/writes wrap. All EEPROMs are
		 * in sizes of power of 2's so this will always be
		 * the last word unless the EEPROM size
		 * BIGGEST_POSSIBLE_EEPROM is not big enough. */
		Dev->nvmops.read(Dev, TestingLimit - 1, 1, &OldValue);

		/* Write the signature value to the word we suspect
		 * is the last */
		WordValue = EEPROM_SIGNATURE;
		Dev->nvmops.write(Dev, TestingLimit - 1, 1, &WordValue);

		/* Now loop through and check every possible last word.
		 * When we don't get a dupe of this word, wrap-around
		 * did not occur and the next size up
		 * is our EEPROM size */
		for(i = TestingLimit; i > 31; i /= 2)
		{
			WordValue = 0;
			//WordValue = ReadEERD(Dev, i-1);
			Dev->nvmops.read(Dev, i - 1, 1, &WordValue);

			if(WordValue != EEPROM_SIGNATURE)
			{
				SizeInWords = i * 2;
				/*If you think your having size problems uncomment this next line*/
				//printf("Size is %d word\n", SizeInWords);
				break;
			}
		}

		//WriteEEWR(Dev, SizeInWords - 1, OldValue);
		Dev->nvmops.write(Dev, SizeInWords - 1, 1, &OldValue);

		/* If the EEPROM doesn't wrap for some reason,
		 * this will lower the max and repeat the
		 * entire process. This will find the limit
		 * in the case of non-wrapping EEPROMs */
		if(SizeInWords > TestingLimit)
		{
			TestingLimit = TestingLimit / 2;
		}
		else
		{
			SizeDetected = TRUE;
		}
	} while ((SizeDetected == FALSE) && (TestingLimit > MAX_TESTS));

	/* Return the size in words of EEPROM */
	return SizeInWords;
}
