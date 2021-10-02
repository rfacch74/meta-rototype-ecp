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
#ifndef HELPER_FUNCTION_H_
#define HELPER_FUNCTION_H_

#define IN
#define OUT
#define MAX_TESTS                       64
#define MAX_ALLOWED_PARAMS              3
#define KNOWN_PARAMS                    10
#define RUN_TEST                        52
#define WRITE_TO_DEVICE                 48
#define WRITE_TO_DEVICE_MAC				112
#define DUMP_DEVICE                     24
#define ERASE_DEVICE                    144
#define REPROGRAM_MAC_MASK				80
#define READLOCKBITS_MASK               272
#define WRITELOCKBITS_MASK              560
#define MIN_NICS                        1
#define MAX_NICS                        32
#define MAX_FILELEN                     80
#define NIC_ID                          16
#define FILE_ID                         32
#define TEST_ID                         4
#define DUMP_ID                         8
#define MAC_ID                          64
#define ERASE_ID                        128
#define READLOCKBITS_ID                 256
#define WRITELOCKBITS_ID                512
#define ZERO_MAC                        "000000000000"
#define FF_MAC                          "FFFFFFFFFFFF"
#define MAC_LEN							12

#define EEPROM_SIGNATURE                0x0D10
#define BIGGEST_POSSIBLE_EEPROM         65535
#define EEPROM_MAX_COLS                 8
#define EEPROM_DISPLAY_ROWS             8
#define EEPROM_MAX_DISPLAY_WORDS        64

// functions
void DisplayError(u16 ErrorStatus);
u16 ParseCommand(u16 argc, c8 * argv[], c8 * filename, u16* nic);
u16  SupportedNicsFound();
u8 SiliconName(u16 DeviceId);
u16 EeReadLineFromEepFile(FILE* DataFile, c8 * Buffer, u32 BufferSize);
void Strupr (c8 * string1);

// structures
typedef struct _ErrorDefinition
{
	s16 ErrorCode;
	c8  * name;
} ErrorDefinition;

typedef struct _ValidParam
{
	c8 * Param;
	u16 ParamId;
	u16 Singular;
	u16 ParamLength;
	u16 Function;
} ValidParam;


// enums
enum NVMFUNCTIONS
{
	INVALID,
	HELP,
	VERSION,
	TEST,
	DUMP,
	PROGRAM,
	SHOWADAPTERS,
	ERASE,
	READLOCKBITS,
	WRITELOCKBITS,
	REPROGRAM_MAC
};

#endif /* HELPER_FUNCTION_H_ */

