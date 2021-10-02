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
#ifndef TEST_FUNC_H_
#define TEST_FUNC_H_

#define OTP_WRITE_PROTECT_REG       0x12324
#define OTP_WRITE_ENABLE            0xABACADA0
#define OTP_WRITE_DISABLE           0xBEE00000
#define OTP_WR_SLEEP_USEC           0x140           // decimal 340
#define OTP_NUM_WORDS               0x40            // decimal 64
#define OTP_LOCKBIT_OFFSET          0x12220
#define OTP_LOCKBIT_MASK            1
#define OTP_LOCKBIT_NUM             32
#define OTP_WAIT_TIMEOUT_USEC       500
#define OTP_WAIT_STEP_USEC          5
#define OTP_BUSY_MASK               0x00000005
#define OTP_BUSY_OKAY               0x00000001
#define WORD_DATA_MASK              0x0000FFFF
#define ETHERNET_ADDRESS_LOW        0x00000019
#define ETHERNET_ADDRESS_MID        0x00000219
#define ETHERNET_ADDRESS_HIGH       0x00000419
#define HIGHBYTE_WORD_DATA_SHIFT    28
#define MID1BYTE_WORD_DATA_SHIFT    24
#define MID2BYTE_WORD_DATA_SHIFT    20
#define LOWBYTE_WORD_DATA_SHIFT     16
#define ONE_DIGIT_SHIFT             4
#define TWO_DIGIT_SHIFT             8
#define THREE_DIGIT_SHIFT           12
#define OTP_HARDWARE_USAGE          3				//The A3 HW uses some words at the end for itself

#define LINE_DATA_LOW               0
#define LINE_DATA_HIGH              1
#define LINE_DATA_DONT_CARE         0xFF
#define SERIAL_FLASH_SCK            0x00000040		/* Serial Flash Clock              */
#define SERIAL_FLASH_CS             0x00000080		/* Serial Flash Chip Select (CS)   */
#define SERIAL_FLASH_SO             0x00040000		/* Serial Flash Serial Data Out    */
#define SERIAL_FLASH_SO_POSITION_I82541  3			/* Bit position for serial data out in I82541*/
#define SERIAL_FLASH_SI             0x00080000		/* Serial Flash Serial Data In     */

/* Flash Access Register */
#define FLA_FL_SCK					0x00000001		/* Serial Flash Clock */
#define FLA_FL_CS					0x00000002		/* Serial Flash Chip Select (CS) */
#define FLA_FL_SI					0x00000004		/* Serial Flash Serial Data In */
#define FLA_FL_SO					0x00000008		/* Serial Flash Serial Data Out */
#define FLA_FL_REQ					0x00000010		/* Request flash access */
#define FLA_FL_GNT					0x00000020		/* Grant flash access */
#define FLA_LOCKED					0x00000040		/* Bit indicating that the NVM is in the Secure mode */
#define FLA_FL_BUSY					0x40000000		/* Flash busy */
#define FLA_FL_ER					0x80000000		/* Flash erase */
#define FLA_FL_SIZE_MASK			0x000E0000
#define I210_FLA                    0x1201C

enum INVM_STATE
{
	INVM_EMPTY,
	INVM_PROGRAMMED,
	INVM_OVERWRITABLE,
	INVM_NONOVERWRITABLE
};

//functions
s8 WriteNvm(struct PciDevice * dev, c8 * fname);

s8 WriteNvmX550(struct PciDevice * dev, c8 * fname);

s8 ReadRawOtpFile(struct PciDevice* Dev, c8* Filename);
s8 WaitForOtp(struct PciDevice* dev);
s8 WriteBufferToOtp(struct PciDevice* Dev, u32* Buffer);
s8 WriteOtp(struct PciDevice * dev, c8 * filename);
s32 EraseNvm(struct PciDevice * dev);
s32 SectorEraseNvm(struct PciDevice * dev, u32 ByteIndex);
s32 BitBangFlashEraseI210(struct PciDevice * dev);
s32 BitBangFlashSectorEraseI210(struct PciDevice * dev, u32 ByteIndex);
//s32 FlswFlashEraseI210(struct PciDevice *dev);
s8 ReadInvmLockBits(struct PciDevice * dev);
s8 WriteInvmLockBits(struct PciDevice * dev, c8 * fname);
s32 BitBangFlashWriteI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
u16 SerialGetFlashIdJedec(struct PciDevice * dev);
u8 SerialWriteFlashFirst8(struct PciDevice * dev, u32 ByteIndex, u8 Data, u8 Flashcmd);
u8 SerialWriteFlashNext8(struct PciDevice * dev, u8 Data);
s32 BitBangFlashOperationDone(struct PciDevice * dev);
s32 SerialFlashWriteStatusRegister(struct PciDevice * dev, bool Enable, bool Generic);

#endif /* TEST_FUNC_H_ */
