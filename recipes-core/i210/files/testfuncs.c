/***********************************************************************************
* Copyright 2012-2019 Intel Corporation All Rights Reserved.
*
* This EepromAccessTool example code ("Software") is furnished under license
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
#include "include/common.h"
#include "include/helpercode.h"
#include "include/PciEeprom.h"
#include "include/testfuncs.h"
#include "include/HelperFunctions.h"
#include <unistd.h>

#define INVALID_PARAMETER -2
#define INVALID_ADAPTER_HANDLE -3
#define FLASH_DOES_NOT_EXIST -4
#define FLASH_WRITE_FAILED -5

extern u8 MacAddress[12];
extern s32 GetFlswFlashOperationDone(struct PciDevice * dev);
extern u32 ReadReg(struct PciDevice * dev, u32 offset);
extern s32 WriteReg(struct PciDevice * dev, u32 offset, u32 data);
extern s8 SetupWriteMethod(struct PciDevice * dev);
extern void SerialFlashClockOutCommand(struct PciDevice * dev, u8 command);
void SerialFlashWriteEnable(struct PciDevice * dev);
void SerialFlashWriteDisable(struct PciDevice * dev);
void Enable_Serial_Flash_Bit_Interface(struct PciDevice * dev);
void Disable_Serial_Flash_Bit_Interface(struct PciDevice * dev);
void SerialFlashRaiseClock(struct PciDevice * dev);
void SerialFlashLowerClock(struct PciDevice * dev);
void SerialFlashSetCS(struct PciDevice * dev, u32 Value);
void SerialFlashSetSI(struct PciDevice * dev, u8 Value);
u8 SerialFlashReadStatusRegister(struct PciDevice * dev);
u8 SerialFlashGetSO(struct PciDevice * dev);
u8 SerialFlashClockInBit(struct PciDevice * dev);
void SerialFlashClockOutBit(struct PciDevice * dev, u8 Bit);

/***************************************************************************
**
** Name:            WriteOtp()
**
** Description:     Programs the iNVM or OTP.
**
** Arguments:       Dev	     - Device to program.
**                  Filename - Path to OTP Raw configuration file
**
** Returns:         Status Code.
**
****************************************************************************/
s8 WriteOtp(struct PciDevice * dev, c8 * filename)
{
	// if we have a successful return value, return success.
	if(SUCCESS == ReadRawOtpFile(dev, filename))
		return SUCCESS;
	else
		return FAIL;
}

/***************************************************************************
**
** Name:            ReadRawOtpFile()
**
** Description:     Loads new autoload configuration defined in Raw
**                  configuration file.
**
** Arguments:       Dev	     - Device to program.
**                  Filename - Path to OTP Raw configuration file
**
** Returns:         Status Code.
**
**  @todo:  save buffer to device, fix returns comments
****************************************************************************/
s8 ReadRawOtpFile(struct PciDevice * Dev, c8* Filename)
{
	s8 retval = FAIL;       // return value
	u16 i = 0;              // counter
	u16 maxlen = 64;        // max words in the OTP
	FILE * file = NULL;     // file pointer to read in data
	u32 b[8];               // temp buffer for sscanf
	c8 temp[200];           // array to hold a line read in from the file
	u32 buffer[OTP_NUM_WORDS];      // buffer to hold the file image data

	if(Filename)
	{
		file = fopen((char *) Filename, "r");

		// check if the file was actually opened or not
		if(NULL == file)
		{
			printf("Unable to open specified file:  %s\n", Filename);
		}
		else
		{
			// Initialize arrays to zero
			//memset(b, 0x0, sizeof(u32)*8);
			memset(buffer, 0x0, sizeof(u32) * OTP_NUM_WORDS);

			// read a line from the file, ignoring comments and such
			retval = EeReadLineFromEepFile(file, temp, 200);

			/* while we have a successful read and still have words to read
			 * continue looping */
			while((SUCCESS == retval) && (i < maxlen))
			{
				// place the hex numbers from the line read in to the temp buffer
				memset(b, 0x0, sizeof(u32) * 8);
				sscanf((char *) temp, "%08x %08x %08x %08x %08x %08x %08x %08x", \
				       &(b[0]), &(b[1]), &(b[2]), &(b[3]), \
				       &(b[4]), &(b[5]), &(b[6]), &(b[7]));

				// copy the words to the permanent buffer
				buffer[i + 0] = b[0];
				buffer[i + 1] = b[1];
				buffer[i + 2] = b[2];
				buffer[i + 3] = b[3];
				buffer[i + 4] = b[4];
				buffer[i + 5] = b[5];
				buffer[i + 6] = b[6];
				buffer[i + 7] = b[7];

				// increment i by 8 so we can get the next 8 words
				i += 8;

				// read the next line
				retval = EeReadLineFromEepFile(file, temp, 200);

			}
			fclose(file);

			if(MacAddress[0] != 0xFF)
			{
				// Adds new Word Auto-load Structures to the buffer
				// (code is intentionally rolled out for ease of reading and understanding)

				// Lower 16-bits of the MAC address
				for(i = 0; (i < (OTP_NUM_WORDS - 1)) && (buffer[i] != 0); i++); //find first empty space
				buffer[i] = ETHERNET_ADDRESS_LOW;
				buffer[i] |= (MacAddress[2] << HIGHBYTE_WORD_DATA_SHIFT) | (MacAddress[3] << MID1BYTE_WORD_DATA_SHIFT);
				buffer[i] |= (MacAddress[0] << MID2BYTE_WORD_DATA_SHIFT) | (MacAddress[1] << LOWBYTE_WORD_DATA_SHIFT);

				// Medium 16-bits of the MAC address
				for(; (i < (OTP_NUM_WORDS - 1)) && (buffer[i] != 0); i++); //find next empty space
				buffer[i] = ETHERNET_ADDRESS_MID;
				buffer[i] |= (MacAddress[6] << HIGHBYTE_WORD_DATA_SHIFT) | (MacAddress[7] << MID1BYTE_WORD_DATA_SHIFT);
				buffer[i] |= (MacAddress[4] << MID2BYTE_WORD_DATA_SHIFT) | (MacAddress[5] << LOWBYTE_WORD_DATA_SHIFT);

				// Higher 16-bits of the MAC address
				for(; (i < (OTP_NUM_WORDS - 1)) && (buffer[i] != 0); i++); //find next empty space
				buffer[i] = ETHERNET_ADDRESS_HIGH;
				buffer[i] |= (MacAddress[10] << HIGHBYTE_WORD_DATA_SHIFT) | (MacAddress[11] << MID1BYTE_WORD_DATA_SHIFT);
				buffer[i] |= (MacAddress[8] << MID2BYTE_WORD_DATA_SHIFT) | (MacAddress[9] << LOWBYTE_WORD_DATA_SHIFT);

				if(i >= (OTP_NUM_WORDS - 3))      // not enough space to fit whole MAC address
				{
					DisplayError(11);
					return FAIL;
				}
			}

			// program the device
			retval = WriteBufferToOtp(Dev, buffer);
		}
	}

	return retval;
}

/***************************************************************************
**
** Name:            CheckOtpState()
**
** Description:     Checks INVM contents to decide about writing
**
** Arguments:       dev      - Device to program
**                  Buffer   - Pointer to the array which holds the data
**
** Returns:         OTP status
**
****************************************************************************/
u8 CheckOtpState(struct PciDevice* dev, u32* Buffer)
{
	u8 state = INVM_EMPTY;
	u32 value = 0;
	u32 count = 0;        // counts number of words as written
	u8 nonempty_flag = FALSE;
	u8 programmed_flag = TRUE;

	//The adjustment accounts for some HW used words in the OTP
	while(count < (OTP_NUM_WORDS - OTP_HARDWARE_USAGE))
	{
		value = ReadReg(dev, INVM_DATA_REG(count));

		if(value)
		{
			nonempty_flag = TRUE;

			// fail if non-overwritable data is present
			if(value & ~Buffer[count])
			{
				state = INVM_NONOVERWRITABLE;
				break;
			}
		}
		if(value != Buffer[count])
		{
			programmed_flag = FALSE;
		}
		++count;
	}

	if((state != INVM_NONOVERWRITABLE) && nonempty_flag)
	{
		if(programmed_flag)
		{
			state = INVM_PROGRAMMED;
		}
		else
		{
			state = INVM_OVERWRITABLE;
		}
	}

	return state;
}

#define TRIES 10
/***************************************************************************
**
**
** Name:            WriteBufferToOtp()
**
** Description:     Writes data from the given buffer to the OTP
**
** Arguments:       dev      - Device to program
**                  Buffer   - Pointer to the array which holds the data
**
** Returns:         Status Code.
**
****************************************************************************/
s8 WriteBufferToOtp(struct PciDevice* dev, u32* Buffer)
{
	s8 retval = SUCCESS;  // return value
	u32 count = 0;        // counts number of words as written
	u32 i = 0;
	u8 break_flag = FALSE;
	u8 state = INVM_EMPTY;

	state = CheckOtpState(dev, Buffer);
	switch(state)
	{
		case INVM_PROGRAMMED:
			printf("Already programmed with the same data.\n");
			break;

		case INVM_NONOVERWRITABLE:
			DisplayError(18);
			retval = FAIL;
			break;

		case INVM_OVERWRITABLE:
			printf("Overwritable data detected in the INVM. Proceeding with programming...\n");
		//intentional no break

		case INVM_EMPTY:
			// enable writing to the OTP by writing a specific code to the specified register
			WriteReg(dev, OTP_WRITE_PROTECT_REG, (u32) (ReadReg(dev, OTP_WRITE_PROTECT_REG) | OTP_WRITE_ENABLE));

			// loop as long as we have words to write
			//The adjustment accounts for some HW used words in the OTP
			while(count < (OTP_NUM_WORDS - OTP_HARDWARE_USAGE))
			{
				// try up to <TRIES> times to write a double word
				for(i = 0; i < TRIES; i++)
				{
					if(ReadReg(dev, INVM_DATA_REG(count)) == Buffer[count])
					{
						break_flag = TRUE;
						break;
					}

					// wait until busy flag is cleared
					WaitForOtp(dev);

					// write
					WriteReg(dev, INVM_DATA_REG(count), Buffer[count]);

					// wait until busy flag is cleared
					WaitForOtp(dev);
				} //for loop

				// debug print uncomment if needed.
				//printf("offset: %u\ttries: %u\n", count, i);

				// writing failed if break from for loop didn't occur
				if(!break_flag)
				{
					DisplayError(17);
					retval = FAIL;
					break;
				}
				break_flag = FALSE;

				++count;
			} //while loop

			// we're done, so we want to disable OTP writing now to prevent accidental writes
			WriteReg(dev, OTP_WRITE_PROTECT_REG, ReadReg(dev, OTP_WRITE_PROTECT_REG) | OTP_WRITE_DISABLE);

			if(retval == SUCCESS)
			{
				printf("Programming successful!\n");
			}
			break;
	} //switch

	return retval;
}

/***************************************************************************
**
** Name:            WaitForOtp()
**
** Description:     Polls Busy flag in INVM_PROTECT register
**
** Arguments:       dev      - Device to program
**
** Returns:         Status Code.
**
****************************************************************************/
s8 WaitForOtp(struct PciDevice* dev)
{
	s8 retval = FAIL;
	u32 i = 0;
	u32 invm_protect_reg = 0;

	for(i = 0; i < OTP_WAIT_TIMEOUT_USEC; i += OTP_WAIT_STEP_USEC)
	{
		invm_protect_reg = ReadReg(dev, OTP_WRITE_PROTECT_REG);
		if((invm_protect_reg & OTP_BUSY_MASK) == OTP_BUSY_OKAY)
		{
			retval = SUCCESS;
			break;
		}
		usleep(OTP_WAIT_STEP_USEC);
	}

	return retval;
}

/***************************************************************************
**
** Name:            EraseNvm()
**
** Description:     Erases entire FLASH memory using the function ptr
**
** Arguments:       dev      - Device to program
**
** Returns:         Status Code.
**
****************************************************************************/
s32 EraseNvm(struct PciDevice * dev)
{
	if(!dev->nvmops.erase)
	{
		printf("Erase is not implemented for this device.\n");
		return FAIL;
	}

	FlashDetectAndSetWritable(dev);

	return dev->nvmops.erase(dev);
}

/***************************************************************************
**
** Name:            SectorEraseNvm()
**
** Description:     Erases entire FLASH memory using the function ptr
**
** Arguments:       dev      - Device to program
**					ByteIndex - sector offset
**
** Returns:         Status Code.
**
****************************************************************************/
s32 SectorEraseNvm(struct PciDevice * dev, u32 ByteIndex)
{
	if(!dev->nvmops.sector_erase)
	{
		printf("Sector erase is not implemented for this device. Doing full chip erase instead.\n");
		return EraseNvm(dev);
	}

	FlashDetectAndSetWritable(dev);

	return dev->nvmops.sector_erase(dev, ByteIndex);
}

/***************************************************************************
**
** Name:            FlswFlashEraseI210
**
** Description:     Erases entire FLASH memory using the HW register.
** 				While not used anymore, it's left here for reference.
**
** Arguments:       dev      - Device to program
**
** Returns:         Status Code.
**
****************************************************************************/
/*s32 FlswFlashEraseI210(struct PciDevice *dev)
   {
    s32   Status        = FAIL;

    if (dev->nvmops.acquire(dev, 0) != SUCCESS)
    {
        printf("Couldn't get a semaphore for bit bang!\n");
        Status = FAIL;
                return Status;
    }

    Status = GetFlswFlashOperationDone(dev);
    if(Status)
    {
        Status = WriteFlswFlashCommand(dev, FLSWCTL_CMD_DEVICE_ERASE, 0);
        GetFlswFlashOperationDone(dev);
    }

    if(Status != SUCCESS)
        printf("Flash device erase failed.\n");
    else
        printf("Flash device erase succeeded.\n");

        dev->nvmops.release(dev, 0);
    return Status;
   }*/

/***************************************************************************
**
** Name:            BitBangFlashEraseI210
**
** Description:     Erases entire FLASH memory using the NVM commandd
**
** Arguments:       dev      - Device to program
**
** Returns:         Status Code.
**
****************************************************************************/
s32 BitBangFlashEraseI210(struct PciDevice * dev)
{
	s32 Status = SUCCESS;

	if(dev->nvmops.acquire(dev, 0) != SUCCESS)
	{
		printf("Couldn't get a semaphore for bit bang!\n");
		Status = FAIL;
		return Status;
	}

	Enable_Serial_Flash_Bit_Interface(dev);

	Status = BitBangFlashOperationDone(dev);

	if(Status == SUCCESS)
	{
		SerialFlashWriteEnable(dev);
		SerialFlashSetCS(dev, LINE_DATA_LOW);
		SerialFlashClockOutCommand(dev, 0x60);
		SerialFlashSetCS(dev, LINE_DATA_HIGH);

		Status = BitBangFlashOperationDone(dev);

		SerialFlashWriteDisable(dev);
	}
	else printf("BitBangFlash failed\n");

	Disable_Serial_Flash_Bit_Interface(dev);

	/*if(Status != SUCCESS)
	    printf("Flash device erase failed.\n");
	   else
	    printf("Flash device erase succeeded.\n");*/

	dev->nvmops.release(dev, 0);
	return Status;
}

/***************************************************************************
**
** Name:            BitBangFlashSectorEraseI210
**
** Description:     Erases a single sector in FLASH memory using the NVM
**                  command
**
** Arguments:       dev      - Device to program
**                  ByteIndex - Sector index
**
** Returns:         Status Code.
**
****************************************************************************/
s32 BitBangFlashSectorEraseI210(struct PciDevice * dev, u32 ByteIndex)
{
	s32 Status = SUCCESS;
	s8 i;
	u8 Bit;

	if(dev->nvmops.acquire(dev, 0) != SUCCESS)
	{
		printf("Couldn't get a semaphore for bit bang!\n");
		Status = FAIL;
		return Status;
	}

	Enable_Serial_Flash_Bit_Interface(dev);

	Status = BitBangFlashOperationDone(dev);

	if(Status == SUCCESS)
	{
		SerialFlashWriteEnable(dev);
		SerialFlashSetCS(dev, LINE_DATA_LOW);
		SerialFlashClockOutCommand(dev, 0x20);

		ByteIndex &= 0x00FFFFFF;  /* Mask off 24 bits */
		for(i = 23; i >= 0; i--)
		{
			Bit = (u8) ((ByteIndex & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}

		SerialFlashSetCS(dev, LINE_DATA_HIGH);

		Status = BitBangFlashOperationDone(dev);

		SerialFlashWriteDisable(dev);
	}

	Disable_Serial_Flash_Bit_Interface(dev);

/*    if(Status != SUCCESS)
        printf("Flash device erase failed.\n");
    else
        printf("Flash device erase succeeded.\n");*/

	return Status;
}

s32 BitBangFlashOperationDone(struct PciDevice * dev)
{
	u32 i = 0;
	u32 TimesToPoll = 1000;
	s32 Status = FAIL;

	for(i = 0; i < TimesToPoll; i++)
	{
		usleep(50000);
		//printf("Status: %x\n", SerialFlashReadStatusRegister(dev));//DEBUG
		if(!(SerialFlashReadStatusRegister(dev) & 0x01))
		{
			Status = SUCCESS;
			break;
		}
	}
	return Status;
}


/***************************************************************************
**
** Name:            ReadInvmLockBits()
**
** Description:     Reads INVM lock bits and prints them
**
** Arguments:       dev      - Device to program
**
** Returns:         Status Code.
**
****************************************************************************/
s8 ReadInvmLockBits(struct PciDevice * dev)
{
	u8 retval = SUCCESS;        // return value
	u8 i;
	u32 value;

	for(i = 0; i < OTP_LOCKBIT_NUM; i++)
	{
		value = ReadReg(dev, OTP_LOCKBIT_OFFSET + 4 * i);
		if(value != FAIL)
		{
			printf("%d:\t%d\n", i, value & OTP_LOCKBIT_MASK);
		}
		else
		{
			retval = FAIL;
			break;
		}
	}

	return retval;
}

/***************************************************************************
**
** Name:            WriteInvmLockBits()
**
** Description:     Reads lock bit configuration from the input file and
**                  writes it to the INVM
**
** Arguments:       dev	     - Device to program.
**                  fname - Path to lock bit configuration file
**
** Returns:         Status Code.
**
****************************************************************************/
s8 WriteInvmLockBits(struct PciDevice * dev, c8 * fname)
{
	s8 retval = FAIL;           // return value
	FILE * file = NULL;     // file pointer to read in data
	c8 b[50];       // array to hold a line read in from the file
	u8 buffer[OTP_LOCKBIT_NUM];     // buffer to hold the file image data
	u8 i;
	int num, bit, err;

	if(fname)
	{
		file = fopen((char *) fname, "r");

		// check if the file was actually opened or not
		if(NULL == file)
		{
			printf("Unable to open specified file:  %s\n", fname);
		}
		else
		{
			memset(buffer, 0x0, sizeof(u8) * OTP_LOCKBIT_NUM);

			retval = SUCCESS;           // sets correct state before the for loop

			// read the input file and put the data in the buffer
			for(i = 0; i < OTP_LOCKBIT_NUM; i++)
			{
				fgets(b, 50, file);

				if(feof(file))          // stop further read if end of file is reached
				{
					printf("EOF\n");
					break;
				}

				err = sscanf((char *) b, "%d %*c %d", &num, &bit);

				if(err < 2)                     // stop further read if less than 2 integers were recognized
				{
					printf("sscanf\n");
					break;
				}

				// if two values are in range then update the buffer
				if((bit == 1 || bit == 0) && (num >= 0) && (num <= (OTP_LOCKBIT_NUM - 1)))
				{
					buffer[num] = bit;
				}
				else
				{
					retval = FAIL;
					break;
				}
			}

			if(retval != FAIL)
			{
				// enable writing to INVM
				WriteReg(dev, OTP_WRITE_PROTECT_REG, OTP_WRITE_ENABLE);

				// write the buffer to the device
				for(i = 0; i < OTP_LOCKBIT_NUM; i++)
				{
					WriteReg(dev, OTP_LOCKBIT_OFFSET + 4 * i, (u32) buffer[i]);
					usleep(10);
				}

				// disable writing to INVM
				WriteReg(dev, OTP_WRITE_PROTECT_REG, OTP_WRITE_DISABLE);
			}

			fclose(file);
		}
	}
	return retval;
}

/*******************************************************************************
**
**  Name:           isBinFile
**
**  Description:    i210 supports both bin and "Eep" files.  They are processed
**		    very differently.  This function figures out which you have.
**
**  Returns: 	    true if it's a binary file, otherwise false
**
*/
s8 isBinFile(FILE * file)
{
	u16 temp[0x10];
	s8 retval = false;

	fread(temp, sizeof(u16), 0x0F, file);
	if(temp[0x0E] == 0x8086)
	{
		printf("It's a BIN input\n");
		retval = true;
	}
	else
	{
		printf("It's a HEX input\n");
		retval = false;
	}
	rewind(file);
	return retval;
}
void LoadMACAddress(u16 * LineWord)
{
	/*first make sure there is room.  Burst size controls the pointer passed in*/
	if(BURST_SIZE < 3)
		return;
	LineWord[0] = MacAddress[2] << THREE_DIGIT_SHIFT;
	LineWord[0] += MacAddress[3] << TWO_DIGIT_SHIFT;
	LineWord[0] += MacAddress[0] << ONE_DIGIT_SHIFT;
	LineWord[0] += MacAddress[1];

	LineWord[1] = MacAddress[6] << THREE_DIGIT_SHIFT;
	LineWord[1] += MacAddress[7] << TWO_DIGIT_SHIFT;
	LineWord[1] += MacAddress[4] << ONE_DIGIT_SHIFT;
	LineWord[1] += MacAddress[5];

	LineWord[2] = MacAddress[10] << THREE_DIGIT_SHIFT;
	LineWord[2] += MacAddress[11] << TWO_DIGIT_SHIFT;
	LineWord[2] += MacAddress[8] << ONE_DIGIT_SHIFT;
	LineWord[2] += MacAddress[9];
}
/*******************************************************************************
**
**  Name:           WriteNvm()
**
**  Description:    This function will program an NVM that is NOT an OTP (iNVM)
**
**  Arguments:      dev   - Device to program
**                  fname - name of file that holds data to be programmed
**
**  Returns:        Status Code.
**
*******************************************************************************/
s8 WriteNvm(struct PciDevice * dev, c8 * fname)
{

	u8 retval = FAIL;           // return value
//  u16  Idx          = 0;	// word index counter
	u32 i = 0;                  // total word counter
	u32 j = 0;
	u32 maxlen = 0;             // max words
	FILE * file = NULL;         // file pointer
	u16 LineWord[BURST_SIZE];           // temp place to hold words as read from file
	s32 temp_offset = 0;

	// detect the size of the nvm in words
	//maxlen = GetNvmWordSize(dev);
	maxlen = dev->wordsize;
	printf("size is %X\n",maxlen);

	if(SetupWriteMethod(dev) != SUCCESS)
	{
		return FAIL;
	}

	// make sure we have a somewhat valid filename!
	if(fname)
	{
		file = fopen(fname, "r");

		// check if the file was actually opened or not
		if(NULL == file)
		{
			printf("Unable to open specified file:  %s\n", fname);
			retval = FAIL;
		}
		else
		{
			printf("Writing...\n");
			if(isBinFile(file))
			{
				for(j = 0; j < maxlen; j += BURST_SIZE)
				{
					for(i = 0; i < BURST_SIZE; i += 8)
					{
						u8 temp[16];
						retval = fread(temp, sizeof(u8), 16, file);
						if(retval < 16)
							break;
						LineWord[i] = (temp[1] << 8) | temp[0];
						LineWord[i + 1] = (temp[3] << 8) | temp[2];
						LineWord[i + 2] = (temp[5] << 8) | temp[4];
						LineWord[i + 3] = (temp[7] << 8) | temp[6];
						LineWord[i + 4] = (temp[9] << 8) | temp[8];
						LineWord[i + 5] = (temp[11] << 8) | temp[10];
						LineWord[i + 6] = (temp[13] << 8) | temp[12];
						LineWord[i + 7] = (temp[15] << 8) | temp[14];

						//if MAC address was supplied in cmd line, we need to omit writing checksum word now,
						//so it can be recalculated and written later
						if(MacAddress[0] != 0xFF)
						{
							temp_offset = NVM_CHECKSUM_REG - ((j * BURST_SIZE) + i);
							if(temp_offset < 8 && temp_offset >= 0)
								LineWord[i + temp_offset] = 0xFFFF;
						}
					}

					if(feof(file))     //this should cause fail to be returned
						break;

					// MAC address injecting (if supplied in cmd line)
					if((i == 0) && (MacAddress[0] != 0xFF))
					{
						LoadMACAddress(LineWord);
					}

					dev->nvmops.write(dev, j, BURST_SIZE, LineWord);
				}
			}
			else
			{
				for(j = 0; j < maxlen; j += BURST_SIZE)
				{
					for(i = 0; i < BURST_SIZE; i += 8)
					{
						c8 temp[50];
						retval = EeReadLineFromEepFile(file, temp, 50);
						if(retval != SUCCESS)
							break;

						sscanf(temp, "%hx  %hx  %hx  %hx  %hx  %hx  %hx  %hx", \
						       &(LineWord[i]), &(LineWord[i + 1]), \
						       &(LineWord[i + 2]), &(LineWord[i + 3]), \
						       &(LineWord[i + 4]), &(LineWord[i + 5]), \
						       &(LineWord[i + 6]), &(LineWord[i + 7]));

						//if MAC address was supplied in cmd line, we need to omit writing checksum word now,
						//so it can be recalculated and written later
						if(MacAddress[0] != 0xFF)
						{
							temp_offset = NVM_CHECKSUM_REG - ((j * BURST_SIZE) + i);
							if(temp_offset < 8 && temp_offset >= 0)
								LineWord[i + temp_offset] = 0xFFFF;
						}
					}

					if(retval != SUCCESS)     // this should cause FAIL to be returned and some error message to be printed
						break;

					// MAC address injecting (if supplied in cmd line)
					if((j == 0) && (MacAddress[0] != 0xFF))
					{
						LoadMACAddress(LineWord);
					}

					dev->nvmops.write(dev, j, BURST_SIZE, LineWord);
				}
			}

			fclose(file);

			// Update the checksum(s)
			dev->nvmops.update(dev);

		}  //else file null
	} //if(fname)

	return retval;
}


/*******************************************************************************
**
**  Name:           WriteNvmX550()
**
**  Description:    This function will program an NVM
**
**  Arguments:      dev   - Device to program
**                  fname - name of file that holds data to be programmed
**
**  Returns:        Status Code.
**
*******************************************************************************/
s8 WriteNvmX550(struct PciDevice * dev, c8 * fname)
{

	s8 retval = FAIL;           // return value
	u16 Idx = 0;                // word index counter
	u32 i = 0;                  // total word counter
	u32 maxlen = 0;             // max words
	FILE * file = NULL;         // file pointer
	u16 LineWord[8];            // temp place to hold words as read from file
	u8 temp[16];                // temp string to hold line from file

	//printf("Entering the function %s\n",__FUNCTION__);
	// detect the size of the nvm in words
	maxlen = dev->wordsize;
	printf("size is %X\n",maxlen);

	// make sure we have a somewhat valid filename!
	if(fname)
	{
		file = fopen(fname, "rb");

		// check if the file was actually opened or not
		if(NULL == file)
		{
			printf("Unable to open specified file:  %s\n", fname);
			retval = FAIL;
		}
		else
		{
			// read a line from the file
			fread(temp, sizeof(char), 16, file);

			// while we have a line and we have not read all the words...
			while((i < maxlen))         //(SUCCESS == retval) &&
			{

				// the expected file format is 8 words per line
				LineWord[0] = (temp[1] << 8) | temp[0];
				LineWord[1] = (temp[3] << 8) | temp[2];
				LineWord[2] = (temp[5] << 8) | temp[4];
				LineWord[3] = (temp[7] << 8) | temp[6];
				LineWord[4] = (temp[9] << 8) | temp[8];
				LineWord[5] = (temp[11] << 8) | temp[10];
				LineWord[6] = (temp[13] << 8) | temp[12];
				LineWord[7] = (temp[15] << 8) | temp[14];

				// write each of the words to the nvm
				for(Idx = 0; Idx < 8; Idx++)
				{
					dev->nvmops.write(dev, i + Idx, 1, &LineWord[Idx]);
				}

				// increment to reflect we want the next 8 words
				i += 8;
				fread(temp, sizeof(char), 16, file);

			}

			fclose(file);

			// Update the checksum(s)
			dev->nvmops.update(dev);
			retval = SUCCESS;

		}
	}

	return retval;
}

u16 SerialGetFlashIdJedec(struct PciDevice * dev)
{
	u8 Value[3];
	s8 i = 0;
	u16 FlashId = 0;

	memset(Value, 0, sizeof(Value));

	Enable_Serial_Flash_Bit_Interface(dev);
	SerialFlashSetCS(dev, LINE_DATA_LOW);
	SerialFlashClockOutCommand(dev, 0x9F);

	/* Read the first bit - this needs to be read now since reads are done on the
	 * falling edge and the clock has been lowered for the command issued. */
	Value[0] |= (SerialFlashGetSO(dev) << 7);

	/* Time to read rest of the data coming in. There will be 7 more bits. */
	for(i = 6; i >= 0; i--)
	{
		Value[0] |= (SerialFlashClockInBit(dev) << i);
	}
	for(i = 7; i >= 0; i--)
	{
		Value[1] |= (SerialFlashClockInBit(dev) << i);
	}
	for(i = 7; i >= 0; i--)
	{
		Value[2] |= (SerialFlashClockInBit(dev) << i);
	}
	SerialFlashSetCS(dev, LINE_DATA_HIGH);
	Disable_Serial_Flash_Bit_Interface(dev);

	FlashId = MAKE_WORD(Value[0], Value[1]);

	return FlashId;
}

void SerialFlashClockOutCommand(struct PciDevice * dev, u8 command)
{
	s16 i = 0;
	u8 bit = 0;

	for(i = 7; i >= 0; i--)
	{
		bit = ((command & (1 << i)) >> i);
		SerialFlashClockOutBit(dev, bit);
		/*	if(i>7)
		        {
		                 printf("ClockOutCommand wrapped!\n");
		                 break;
		        }*/
	}
}

/***************************************************************************
**
** Name:            SerialWriteFlashFirst8()
**
** Description:     Writes a u8 to the flash at specified location.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  ByteIndex        = Offset in the flash to read.
**                  Data             = Value to write.
**                  Flashcmd         = Flash command to use
**
** Returns:         SUCCESS = Value written successfully.
**                  INVALID_ADAPTER_HANDLE = Adapter is NULL.
**
****************************************************************************/
u8 SerialWriteFlashFirst8(struct PciDevice * dev, u32 ByteIndex, u8 Data, u8 Flashcmd)
{
	u8 Status = INVALID_ADAPTER_HANDLE;
	s8 i = 0;
	u8 Bit = 0;

	if(dev != NULL)
	{
		/* Clock out to the flash write command 0000X010 (2h) */
		SerialFlashClockOutCommand(dev, Flashcmd);

		/* Clock out the address in a 24bit address format */
		ByteIndex &= 0x00FFFFFF;  /* Mask off 24 bits */
		for(i = 23; i >= 0; i--)
		{
			Bit = (u8) ((ByteIndex & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}

		/* Program the byte into flash by clocking it out after the command & address */
		for(i = 7; i >= 0; i--)
		{
			Bit = ((Data & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}

		Status = SUCCESS;
	}

	return Status;
}

/***************************************************************************
**
** Name:            AtmelSerialWriteFlashNext8()
**
** Description:     Writes the next byte to the flash. Used for consecutive writes.
**                  This assumes that the flash is in the state to accept writes byte by
**                  byte. The CS line is set up and the first byte has been clocked in.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  ByteIndex        = Offset in the flash to read.
**                  Data             = Value to write.
**
** Returns:         SUCCESS = Value written successfully.
**                  INVALID_ADAPTER_HANDLE = Adapter is NULL.
**
****************************************************************************/
u8 SerialWriteFlashNext8(struct PciDevice * dev, u8 Data)
{
	u8 Status = INVALID_ADAPTER_HANDLE;
	s8 i = 0;
	u8 Bit = 0;

	if(dev != NULL)
	{
		/* Program the byte into flash by clocking it out after the command & address */
		for(i = 7; i >= 0; i--)
		{
			Bit = ((Data & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}

		Status = SUCCESS;
	}

	return Status;
}

/***************************************************************************
**
** Name:            SerialFlashWriteEnable()
**
** Description:     Sends the WREN (Write Enable) command to the serial flash..
**
** Arguments:       Adapter = Pointer to the adapter structure.
**
** Returns:         Nothing
**
****************************************************************************/
void SerialFlashWriteEnable(struct PciDevice * dev)
{
	SerialFlashSetCS(dev, LINE_DATA_LOW);
	SerialFlashClockOutCommand(dev, 0x06);
	SerialFlashSetCS(dev, LINE_DATA_HIGH);

	/* Issue the status write enable command 06h */
	SerialFlashSetCS(dev, LINE_DATA_LOW);
	SerialFlashClockOutCommand(dev, 0x06);
	SerialFlashSetCS(dev, LINE_DATA_HIGH);
}

/***************************************************************************
**
** Name:            SerialFlashWriteDisable()
**
** Description:     Sends the WRDI (Write Disable) command to the serial flash..
**
** Arguments:       Adapter = Pointer to the adapter structure.
**
** Returns:         Nothing
**
****************************************************************************/
void SerialFlashWriteDisable(struct PciDevice * dev)
{
	/* Set CS low for the duration of the WRDI operation. */
	SerialFlashSetCS(dev, LINE_DATA_LOW);

	/* Issue the WRDI command 00000100 (4h) */
	SerialFlashClockOutCommand(dev, 0x04);

	/* Set CS high, we're done with the operation. */
	SerialFlashSetCS(dev, LINE_DATA_HIGH);
}

/***************************************************************************
**
** Name:            SerialFlashWriteStatusRegister()
**
** Description:     Writes the status register to allow chip erasure.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  Enable = TRUE enables erasure, FALSE disables.
**
** Returns:         STATUS
**
****************************************************************************/
s32 SerialFlashWriteStatusRegister(struct PciDevice * dev, bool Enable, bool Generic)
{
	s32 Status = FAIL;

	//printf("Entering SerialFlashWriteStatusRegister - writing status register\n");
	if(dev != NULL)
	{
		Enable_Serial_Flash_Bit_Interface(dev);

		/* SST requires a write to enable writes to the status register first */
		/* Set CS low, issue write enable command, set CS to high to end command */

		if(Generic == TRUE) //Generic Flash
		{
			SerialFlashSetCS(dev, LINE_DATA_LOW);
			SerialFlashClockOutCommand(dev, 0x06);
			SerialFlashSetCS(dev, LINE_DATA_HIGH);
		}
		else //SST Flash
		{
			SerialFlashSetCS(dev, LINE_DATA_LOW);
			SerialFlashClockOutCommand(dev, 0x50);
			SerialFlashSetCS(dev, LINE_DATA_HIGH);
		}

		/* Set CS low for the duration of the entire operation. */
		SerialFlashSetCS(dev, LINE_DATA_LOW);

		/* Clock out Write Status Register Command (1h) to flash */
		SerialFlashClockOutCommand(dev, 0x01);

		/* Clock out the value to write */
		if(Enable == TRUE)
		{
			/* Clear Block Protect bits 5:2.  Some parts only use 4:2 or 3:2 but we clear 5:2 since
			 * the others are don't care on the write.  Clear bit 7 to allow status register write.
			 * Set Bit 1 to allow write enable */
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
		}
		else
		{
			/* Set Block Protect bits 5:2 to enable protection. Set bit 7 to disable status register write.
			 * Bit 1 is set to allow write enable */
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_HIGH);
			SerialFlashClockOutBit(dev, LINE_DATA_LOW);
		}

		/* Set CS high when done with the operation. */
		SerialFlashSetCS(dev, LINE_DATA_HIGH);
		Disable_Serial_Flash_Bit_Interface(dev);
		Status = SUCCESS;
	}
	return Status;
}


void Enable_Serial_Flash_Bit_Interface(struct PciDevice * dev)
{
	u32 FlaReg;
	int i = 10000;

	FlaReg = ReadReg(dev, I210_FLA);
	FlaReg |= FLA_FL_REQ;
	WriteReg(dev, I210_FLA, FlaReg);
	do
	{
		FlaReg = ReadReg(dev, I210_FLA);
		usleep(100);
	} while ((FlaReg & FLA_FL_GNT) == 0 && (--i));
	if(i == 0)
		printf("Enable Serial Flash Bit Interface timed out.\n");

	/* enable bit interface */
	SerialFlashSetCS(dev, LINE_DATA_HIGH);
	SerialFlashSetSI(dev, LINE_DATA_LOW);
	SerialFlashLowerClock(dev);
}

void Disable_Serial_Flash_Bit_Interface(struct PciDevice * dev)
{
	u32 FlaReg;

	FlaReg = ReadReg(dev, I210_FLA);
	FlaReg &= ~FLA_FL_REQ;
	WriteReg(dev, I210_FLA, FlaReg);
}

#define SST_MAX_BYTE_PROGRAM_TIME 10    // can be found in the datasheet and used to optimize programming time
/***************************************************************************
**
** Name:            SstSerialFlashWriteImage()
**
** Description:     Writes the entire flash image. This will call the function
**                  passed in DisplayPercent to update some sort of display.
**
**                  These are serial flashes and require a 'bit-bang' to write the image.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  Image            = A Pointer to the image buffer to write.
**                  ImageSize        = The size of the image.
**                  RegionOffset     = Optional parameter to specify location in flash to start writing image
**                  DisplayPercent   = pointer to display callback function.
**                                     Set this to NULL if no display is needed.
**
** Returns:         SUCCESS = Flash written successfully.
**                  FLASH_BAD_IMAGE = Image was NULL.
**                  FLASH_BAD_INDEX = Index is out of range.
**                  INVALID_ADAPTER_HANDLE = dev was NULL.
**                  FLASH_WRITE_FAILED = The flash process failed.
**                  FLASH_ID_UNKNOWN = Flash ID unrecognized.
**
****************************************************************************/
s32 SstBitBangFlashWriteI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	u32 i = 0;
	s8 Status = SUCCESS;

	if(dev->nvmops.acquire(dev, 0) != SUCCESS)
	{
		printf("Couldn't get a semaphore for bit bang!\n");
		Status = FAIL;
		return Status;
	}

	if(dev->FlashSize != 0 && Status == SUCCESS)
	{
		/* Enable flash bit interface */
		Enable_Serial_Flash_Bit_Interface(dev);

		SerialFlashWriteEnable(dev);

		/* Set CS low for the duration of the write operation. */
		SerialFlashSetCS(dev, LINE_DATA_LOW);

		// Write the first byte - this also sets the start address
		SerialWriteFlashFirst8(dev, Offset * 2, (u8) * Data, 0xAD);

		// Write second byte
		SerialWriteFlashNext8(dev, (u8) ((*Data) >> 8));

		// SST Flashes require CS to go high and low after each 2 bytes
		SerialFlashSetCS(dev, LINE_DATA_HIGH);
		usleep(SST_MAX_BYTE_PROGRAM_TIME);

		// Write further bytes (when NumWords > 1)
		for(i = 1; i < NumWords; i++)
		{
			Data++;
			SerialFlashSetCS(dev, LINE_DATA_LOW);

			/* SST Flashes require repeating the writing instruction each 2 bytes,
			 * address doesn't have to be repeated (it's auto-incremented) */
			SerialFlashClockOutCommand(dev, 0xAD);
			SerialWriteFlashNext8(dev, (u8) * Data);
			SerialWriteFlashNext8(dev, (u8) ((*Data) >> 8));
			SerialFlashSetCS(dev, LINE_DATA_HIGH);
			usleep(SST_MAX_BYTE_PROGRAM_TIME);
		}

		//printf("StatusReg: %x\n", SerialFlashReadStatusRegister(dev));//DEBUG

		SerialFlashWriteDisable(dev);

		/* we don't poll status register, because we use delay >= max programming time */

		Disable_Serial_Flash_Bit_Interface(dev);
		dev->nvmops.release(dev, 0);
	}
	else
	{
		Status = FLASH_DOES_NOT_EXIST;
		printf("Failed to read flash chip size\n");
	}
	return Status;
}

/***************************************************************************
**
** Name:            GenericSerialFlashWriteImage()
**
** Description:     Writes the entire flash image. This will call the function
**                  passed in DisplayPercent to update some sort of display.
**
**                  These are serial flashes and require a 'bit-bang' to write the image.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  Image            = A Pointer to the image buffer to write.
**                  ImageSize        = The size of the image.
**                  RegionOffset     = Optional parameter to specify location in flash to start writing image
**                  DisplayPercent   = pointer to display callback function.
**                                     Set this to NULL if no display is needed.
**
** Returns:         SUCCESS = Flash written successfully.
**                  FLASH_BAD_IMAGE = Image was NULL.
**                  FLASH_BAD_INDEX = Index is out of range.
**                  INVALID_ADAPTER_HANDLE = dev was NULL.
**                  FLASH_WRITE_FAILED = The flash process failed.
**                  FLASH_ID_UNKNOWN = Flash ID unrecognized.
**
****************************************************************************/
s32 GenericBitBangFlashWriteI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data)
{
	u32 i = 0;
	u32 TimesToPoll = 1000;
	s8 Status = SUCCESS;

	if(dev->nvmops.acquire(dev, 0) != SUCCESS)
	{
		printf("Couldn't get a semaphore for bit bang!\n");
		Status = ~SUCCESS;
	}

	if(dev->FlashSize != 0 && Status == SUCCESS)
	{
		/* Enable flash bit interface */
		Enable_Serial_Flash_Bit_Interface(dev);

		SerialFlashWriteEnable(dev);

		/* Set CS low for the duration of the write operation. */
		SerialFlashSetCS(dev, LINE_DATA_LOW);

		// Write the first byte - this also sets the start address
		SerialWriteFlashFirst8(dev, Offset * 2, (u8) * Data, 0x02);

		// Write second byte
		SerialWriteFlashNext8(dev, (u8) ((*Data) >> 8));

		// Write further bytes (when NumWords > 1)
		for(i = 1; i < NumWords; i++)
		{
			Data++;
			SerialWriteFlashNext8(dev, (u8) (*Data & 0x00FF));
			SerialWriteFlashNext8(dev, (u8) ((*Data) >> 8));
		}

		SerialFlashSetCS(dev, LINE_DATA_HIGH);
		SerialFlashWriteDisable(dev);

		/* Now read the status register and wait until bit 0 is 0 which means the
		 * write operation has completed. */

		for(i = 0; i < TimesToPoll; i++)
		{
			usleep(5);
			if(!(SerialFlashReadStatusRegister(dev) & 0x01))
			{
				break;
			}
		}

		Disable_Serial_Flash_Bit_Interface(dev);
		dev->nvmops.release(dev, 0);

		/* If this is true, the write never finished, quit */
		if(TimesToPoll == i)
		{
			/* Return flash to normal operation */
			Status = FLASH_WRITE_FAILED;
			printf("Flash write failed for offset 0x%x\n", Offset * 2);
			return FAIL;
		}
	}
	else
	{
		Status = FLASH_DOES_NOT_EXIST;
		printf("Failed to read flash chip size\n");
	}
	return Status;
}

/***************************************************************************
**
** Name:            SerialFlashRaiseClock()
**
** Description:     Raises the clock bit on the serial flash by setting SCK
**                  high (SDP6 = 1).
**
** Arguments:       dev       = Pointer to the adapter structure.
**
** Returns:         Nothing
**
****************************************************************************/
void SerialFlashRaiseClock(struct PciDevice * dev)
{
	u32 Reg = 0;

	Reg = ReadReg(dev, I210_FLA);
	Reg |= FLA_FL_SCK;
	WriteReg(dev, I210_FLA, Reg);
//    usleep(1);

//    for(u16 i = 0; i < 2000; i++)
//        asm("nop");
}

/***************************************************************************
**
** Name:            SerialFlashLowerClock()
**
** Description:     Raises the clock bit on the serial flash by setting SCK
**                  low (SDP6 = 0).
**
** Arguments:       dev       = Pointer to the adapter structure.
**
** Returns:         Nothing
**
****************************************************************************/
void SerialFlashLowerClock(struct PciDevice * dev)
{
	u32 Reg = 0;

	Reg = ReadReg(dev, I210_FLA);
	Reg &= ~(FLA_FL_SCK);
	WriteReg(dev, I210_FLA, Reg);
//    usleep(1);
//    for(u16 i = 0; i < 2000; i++)
//        asm("nop");
}

/***************************************************************************
**
** Name:            SerialFlashSetCS()
**
** Description:     Sets the CS line to the value passed in by value. Value can
**                  be either LINE_DATA_HIGH or LINE_DATA_LOW.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  Value      = Value to set CS line to.
**
** Returns:         Nothing
**
** Note:            This should be modified to check CS and if it is low fail.
**
****************************************************************************/
void SerialFlashSetCS(struct PciDevice * dev, u32 Value)
{
	u32 Reg = 0;

	/* Read the control register */
	Reg = ReadReg(dev, I210_FLA);

	/* Set either Hi or Low */
	if(Value == LINE_DATA_HIGH)
	{
		Reg |= FLA_FL_CS;
	}
	else if(Value == LINE_DATA_LOW)
	{
		Reg &= ~(FLA_FL_CS);
	}

	/* Write out the new value to the Control Ext register */
	WriteReg(dev, I210_FLA, Reg);
}

/***************************************************************************
**
** Name:            SerialFlashSetSI()
**
** Description:     This sets the SI line (SDP1) to either 1 or 0 based on the
**                  value of data. Data can be LINE_DATA_HIGH, LINE_DATA_LOW, or
**                  LINE_DATA_DONT_CARE.
**
** Arguments:       dev = Pointer to the adapter structure.
**                  DAta       = Value to set SI line to.
**
** Returns:         Nothing
**
****************************************************************************/
void SerialFlashSetSI(struct PciDevice * dev, u8 Value)
{
	u32 Reg = 0;

	/* Read the control register */
	Reg = ReadReg(dev, I210_FLA);

	/* Set either Hi or Low */
	if(Value == LINE_DATA_HIGH)
	{
		Reg |= FLA_FL_SI;
	}
	else if(Value == LINE_DATA_LOW)
	{
		Reg &= ~(FLA_FL_SI);
	}

	/* Write out the new value to the Control Ext register */
	WriteReg(dev, I210_FLA, Reg);
}

/***************************************************************************
**
** Name:            SerialFlashGetSO()
**
** Description:     This reads the SO line (SDP0) and returns the
**                  value which represents a single BIT clocked from the flash.
**
** Arguments:       dev = Pointer to the adapter structure.
**
** Returns:         Nothing
**
****************************************************************************/
u8 SerialFlashGetSO(struct PciDevice * dev)
{
	u32 Reg = 0;

	/* Read the control register */
	Reg = ReadReg(dev, I210_FLA);

	/* Return the value */
	return (u8) ((Reg & FLA_FL_SO) >> SERIAL_FLASH_SO_POSITION_I82541);
}

/***************************************************************************
**
** Name:            SerialFlashReadStatusRegister()
**
** Description:     Reads and returns the ATMEL serial flash status register.
**
** Arguments:       dev = Pointer to the adapter structure.
**
** Returns:         SUCCESS = Flash Erased Successfully.
**                  INVALID_ADAPTER_HANDLE = dev was NULL.
**                  IO_FAILED = Flash did not erase.
**
****************************************************************************/
u8 SerialFlashReadStatusRegister(struct PciDevice * dev)
{
	int i = 0;                   //signed so >0 will work
	u8 StatusRegister = 0;

	/* Set CS low for the duration of the entire operation. */
	SerialFlashSetCS(dev, LINE_DATA_LOW);

	/* Clock out to the flash read status register 00000101 (5h) */
	SerialFlashClockOutCommand(dev, 0x05);

	/* Read the 8 bits that make up the status register. The first read is done before
	 * touching the clock since the ClockOutBit lowered the clock and the read is done
	 * on the falling edge. Afterwards, each bit is clocked in. */
	StatusRegister |= (SerialFlashGetSO(dev) << 7);
	for(i = 6; i >= 0; i--)
	{
		StatusRegister |= (SerialFlashClockInBit(dev) << i);
	}

	//printf("ReadStatusRegister %x\n",StatusRegister);
	/* Set CS high when done with the operation. */
	SerialFlashSetCS(dev, LINE_DATA_HIGH);

	return StatusRegister;
}

/***************************************************************************
**
** Name:            SerialFlashClockInBit()
**
** Description:     Clocks in a single bit from the SO line on the serial flash.
**
** Arguments:       dev   = Pointer to the current adapter structure.
**
** Returns:         Value of the bit as a u8.
**
****************************************************************************/
u8 SerialFlashClockInBit(struct PciDevice * dev)
{
	u8 Bit = 0;

	/* Atmel serial flashes need to be read on the lower edge of the clock. So, raise,
	 * lower, and the next bit is ready to read */

	SerialFlashRaiseClock(dev);
	SerialFlashLowerClock(dev);
	Bit = SerialFlashGetSO(dev);

	return Bit;
}

/***************************************************************************
**
** Name:            SerialFlashClockOutBit()
**
** Description:     Clocks out a single bit from the SO line on the serial flash.
**
** Arguments:       dev   = Pointer to the current adapter structure.
**                  Bit          = The bit to clock out.
**
** Returns:         Nothing.
**
****************************************************************************/
void SerialFlashClockOutBit(struct PciDevice * dev, u8 Bit)
{
	/* Atmel writes are written on the rising edge of the clock. So put the bit to
	 * write out on the wire, raise clock, lower clock */
	SerialFlashSetSI(dev, Bit);
	SerialFlashRaiseClock(dev);
	SerialFlashLowerClock(dev);
}

/*********************************************************************************************************//**
**
** Name:         SerialReadFlash8
**
** details         Returns a UINT8 from the flash at specified location. This access trough flash serial
**                  interface only if supported.
**
** param[in]       Handle           Handle to the adapter returned from NalInitializeAdapter with
**                                   NAL_INIT_ENABLE_FLASH flag
** param[in]       ByteIndex        Address in bytes from where data should be read
** param[out]      Value            Pointer to memory where read value is stored
**
** returns         NAL_SUCCESS if operation finishes with success, otherwise see the NAL Status Codes.
**
*************************************************************************************************************/
s8 SerialReadFlash8(struct PciDevice * dev,u32 ByteIndex, u8* Value)
{
	s8 Status = FAIL;
	int i = 0;
	u8 Bit = 0;

	if(Value == NULL)
	{
		printf("Value was passed a bogus value\n");
	}
	else
	{
		*Value = 0;

		/* Enable flash bit interface */
		Enable_Serial_Flash_Bit_Interface(dev);

		/* Set CS low for the duration of the entire operation. */
		SerialFlashSetCS(dev, LINE_DATA_LOW);

		/* Clock out to the flash read command 00000011 (3h) */
		SerialFlashClockOutCommand(dev, 0x03);

		/* Clock out the address in a 24bit address format */
		ByteIndex &= 0x00FFFFFF;  /* Mask off 24 bits */
		for(i = 23; i >= 0; i--)
		{
			Bit = (u8) ((ByteIndex & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}

		/* Read the first bit - this needs to be read now since reads are done on the
		 * falling edge and the clock has been lowered for the command issued. */
		*Value |= (SerialFlashGetSO(dev) << 7);

		/* Time to read rest of the data coming in. There will be 7 more bits. */
		for(i = 6; i >= 0; i--)
		{
			*Value |= (SerialFlashClockInBit(dev) << i);
		}

		/* Set CS high to signal the end of the operation. */
		SerialFlashSetCS(dev, LINE_DATA_HIGH);

		/* Disable flash bit interface */
		Disable_Serial_Flash_Bit_Interface(dev);
		Status = SUCCESS;
	}

	return Status;
}

s32 SerialReadFlashFirst8(struct PciDevice * dev, u32 ByteIndex, u8 * Value)
{
	s32 Status;
	s8 i = 0;
	u8 Bit = 0;

	if(Value == NULL)
	{
		Status = FAIL;
	}
	else
	{
		SerialFlashClockOutCommand(dev, 0x03);
		ByteIndex &= 0x00FFFFFF;          /* Mask off 24 bits */
		for(i = 23; i >= 0; i--)
		{
			Bit = (u8) ((ByteIndex & (1 << i)) >> i);
			SerialFlashClockOutBit(dev, Bit);
		}
		*Value |= (SerialFlashGetSO(dev) << 7);

		for(i = 6; i >= 0; i--)
		{
			*Value |= (SerialFlashClockInBit(dev) << i);
		}
		Status = SUCCESS;
	}
	return Status;
}

s32 SerialReadFlashNext8(struct PciDevice * dev, u8 * Value)
{
	s32 Status = FAIL;
	s8 j = 0;

	if(Value)
	{
		for(j = 7; j >= 0; j--)
		{
			*Value |= (SerialFlashClockInBit(dev) << j);
		}
		Status = SUCCESS;
	}
	return Status;
}

s32 GenericBitBangFlashReadI210(struct PciDevice * dev, u32 ByteIndex, u16 NumWords, u16 * Value)
{
	s32 i;
	u8 HiByte = 0;
	u8 LowByte = 0;
	s8 Status = SUCCESS;

	if(dev->nvmops.acquire(dev, 0) != SUCCESS)
	{
		printf("Couldn't get a semaphore for bit bang!\n");
		Status = FAIL;
		return Status;
	}
	if(dev->FlashSize != 0 && Status == SUCCESS)
	{
		Enable_Serial_Flash_Bit_Interface(dev);
		SerialFlashSetCS(dev, LINE_DATA_LOW);

		Status = SerialReadFlashFirst8(dev, 2 * ByteIndex, &LowByte);
		Status = SerialReadFlashNext8(dev, &HiByte);
		Value[0] = MAKE_WORD(HiByte, LowByte);

		for(i = 1; i < NumWords; i++)
		{
			HiByte = 0;
			LowByte = 0;
			Status = SerialReadFlashNext8(dev, &LowByte);
			Status = SerialReadFlashNext8(dev, &HiByte);
			Value[i] = MAKE_WORD(HiByte, LowByte);
		}
		SerialFlashSetCS(dev, LINE_DATA_HIGH);
		Disable_Serial_Flash_Bit_Interface(dev);
		dev->nvmops.release(dev, 0);
		Status = SUCCESS;
	}
	else
	{
		Status = FLASH_DOES_NOT_EXIST;
		printf("Failed to read flash chip size\n");
	}
	return Status;
}
