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

#ifndef HELPER_CODE_H_
#define HELPER_CODE_H_

/******** Defines ********/
// Registers
#define HWSW_SEM_REG      0x5B50    /* HW/SW access semaphore */
#define HWSW_SYNC         0x5B5C    /* Synchronize */
#define FL_ACCESS_REG     0x001C    /* Flash access register */
#define FLASH_BLOCK_BASE  0x12100   /* Flash block base */
#define EECTRL_OFFSET     0x10
#define PROC_DEVICES      "/proc/bus/pci/devices"

// IDs
#define INTEL_ID                0x8086

// Masks
#define MAP_SIZE                0x1000
#define MAP_MASK                MAP_SIZE - 0x1
#define BITMASK                 0xFFFF
#define EECTRL_FLASH_PRESENT    0x80000

// functions
u16 ShowHelp();
u16 ShowVersion();
u16 RunTest(c8* fname);
u16 DumpData(u16 NicId);
u16 WriteToDevice(u16 NicId, c8* fname);
s8 ReadLockBits(u16 NicId);
s8 WriteLockBits(u16 NicId, c8 * fname);
s8 EraseDevice(u16 NicId);
void DisplayAdapters();
u16 ReprogramMacAddress(u16 NicId);
#endif /* HELPER_CODE_H_ */
