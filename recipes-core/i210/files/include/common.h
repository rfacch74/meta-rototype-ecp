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

#ifndef COMMON_H_
#define COMMON_H_
// Data types
#define u8         unsigned char
#define u16        unsigned short
#define u32        unsigned int
#define u64        unsigned long long

#define s8         signed char
#define c8         char
#define s16        signed short
#define s32        signed int
#define s64        signed long long
#define boolean    unsigned char

#define DEV_BITS       0xF0
#define FUN_BITS       0x0F
#define SUCCESS        0
#define FAIL           -1
#define TRUE           1
#define FALSE		   0
#define TOOLNAME	   "EEPROM Access Tool"
#define TOOLVER_MAJOR  0
#define TOOLVER_MINOR  7
#define TOOLVER_BUILD  9

#define BAR_MASK            0xFFFFE003

// Intel 82574 Device Ids
#define INTEL_82574         0x10D3
#define INTEL_82574_GIG     0x10F6
#define INTEL_82574_BLANK   0x10D4

// Intel I211 Device Ids
#define INTEL_I211_BLANK    0x1532
#define INTEL_I211          0x1539

// Intel I210 Device Ids
#define INTEL_I210_BLANK			0x1531
#define INTEL_I210_AT_OEM			0x1533
#define INTEL_I210_AT_EMP			0x1534
#define INTEL_I210_AS_FIB			0x1536
#define INTEL_I210_AS_1				0x1537
#define INTEL_I210_AS_2				0x1538
#define INTEL_I210_AT_RET			0x157B
#define INTEL_I210_AS_OEM			0x157C
#define INTEL_I210_SGMII_FLASHLESS  0x15f6

// Intel X550 Device Ids
#define INTEL_X550_OEM          0x1563
#define INTEL_X550_VF1          0x1564
#define INTEL_X550_VF2          0x1565
#define INTEL_X550_T1           0x15D1

// Denverton X553 Device Ids
#define INTEL_X553_SGMII_BACKPLANE1 0x15c6
#define INTEL_X553_SGMII_BACKPLANE2 0x15c7
#define INTEL_X553_KX               0x15c3
#define INTEL_X553_SGMII_1          0x15E4
#define INTEL_X553_SGMII_2          0x15E5

#define NVMSIZE_X550        2048
#define NVMSIZE_I210        2048
#define NVMSIZE_I211        2048
#define NVMSIZE_82574       2048

#define DELAY_USEC(a)       (a)
#define DELAY_MSEC(a)       (a)
#define DELAY_MSEC_IRQ(a)   (a)

#define GET_NVM_VALUE_AT_COORDINATE(DataArray, Row, Column) \
	(DataArray[ (Row) * EEPROM_MAX_COLS + (Column) ])
#define SET_NVM_VALUE_AT_COORDINATE(DataArray, Row, Column, Value) \
	(DataArray[ (Row) * EEPROM_MAX_COLS + (Column) ]) = (Value)

#define MAKE_WORD(hi, low)                  ((u16) ((((u16) (hi)) << 8) | (low)))

#endif /* COMMON_H_ */
