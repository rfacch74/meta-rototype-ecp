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

#ifndef PCIE_EEPROM_H_
#define PCIE_EEPROM_H_

// EEWR offset for ID 1533
#define EEWR_OFFSET    0x12018
#define WRITE_DONE     0x2
#define ZERO_EEWR_DATA 0x0000E003
#define EEWR_DATA      0x10
#define EEWR_ADDRESS   0x2

#define TRUE  1
#define FALSE 0

// Defines for X553
#define X553_MAC_ADDR_OFFSET             (1)
#define X553_ALT_MAC_ADDR_PORT1_OFFSET   (3)
#define X553_LAN_CORE0_PTR_ADDR          (0x9)
#define X553_LAN_CORE1_PTR_ADDR          (0xA)
#define X553_ALT_MAC_ADDR_PTR_ADDR       (0x37)
#define X553_PORT0_MAC_ADDR_LOCATION     (0x101)
#define X553_PORT1_MAC_ADDR_LOCATION     (0x109)
#define X553_PORT0_ALT_MAC_ADDR_LOCATION (0x144)
#define X553_PORT1_ALT_MAC_ADDR_LOCATION (0x147)

//Defines for X550
#define COMMAND_TIMEOUT         500
#define MAX_BUFFER_SIZE         1024
#define NVM_LAST_WORD           0x41
#define MAX_BLOCK_BYTE_LENGTH   1792
/*#define X550_CPU_TO_BE16(_x) cpu_to_be16(_x)
 #define X550_BE16_TO_CPU(_x) be16_to_cpu(_x)
 #define X550_CPU_TO_BE32(_x) cpu_to_be32(_x)
 #define X550_BE32_TO_CPU(_x) be32_to_cpu(_x)
 #define X550_CPU_TO_LE32(_i) cpu_to_le32(_i)
 #define X550_LE32_TO_CPUS(_i) le32_to_cpus(_i)
   typedef int bool;
 #define true 1
 #define false 0*/
#define _Bool char

#ifndef bool
#define bool _Bool
#define true 1
#define false 0
#endif

// Defines for Mem Types
#define NVM_ONLY  0
#define INVM_ONLY 1
#define NVM_INVM  2
#define NVM_X550  3

// EEWR offset for ID 1533
#define EERD_OFFSET            0x12014
#define EERD_DATA_MASK         0xFFFF0000
#define READ_DONE              0x2
#define EERD_DATA              0x10
#define EERD_ADDRESS           0x2
#define MDIO_OWNERSHIP_TIMEOUT 10

// Semaphore for ID 1533
#define SEM_REG 0x5B50
#define SW_SEM  0x02

// Token to make sure the temp buffer didn't overflow
#define OVERFLOW_TOKEN                          0xD1044552

#define EXTCNF_CTRL                             0x00F00
#define EXTCNF_CTRL_MDIO_SW_OWNERSHIP			0x00000020

#define EECD                                    0x00010
#define EERD                                    0x00014    // Read EEPROM Register offset
#define EEWR                                    0x0102C    // Write EEPROM Register offset
#define FLSWCTL                                 0x12048
#define FLSWDATA                                0x1204C
#define FLSWCNT                                 0x12050

#define SRWR                                    0x12018
#define SWSM                                    0x05B50
#define SWSM_SWESMBI                            0x00000002
#define SWSM_SMBI                               0x00000001
#define SWFW_EEP_SM                             0x01

#define SW_FW_SYNC                              0x05B5C   // Sync register for SW and FW

#define INVM_SIZE                               64        // Size in 32bit words of the iNVM
#define NVM_POLL_WRITE                          1         // Write complete flag
#define NVM_POLL_READ                           0         // Read complete flag
#define NVM_CHECKSUM_REG                        0x003F    // Offset of the checksum register
#define NVM_RW_REG_DONE                         2         // Read/Write done bit offset
#define NVM_RW_REG_START                        1
#define NVM_RW_REG_DATA                         16        // Data storage bit location in NVM
#define NVM_RW_ADDR_SHIFT                       2         // Bits to shift to get to the address
#define NVM_DATA_OFFSET                         3

#define PCIE_GENERAL_SIZE                       0x24
#define PCIE_CONFIG_SIZE                        0x08
#define OPTION_ROM_PTR                          0x05
#define START_PTR                               0x03
#define LAST_PTR                                0x0F

#define PCIE_GENERAL_PTR                        0x06
#define PCIE_CONFIG0_PTR                        0x07
#define PCIE_CONFIG1_PTR                        0x08
#define PHY_PTR                                 0x04

#define EERD_EEWR_MAX_COUNT                     512
#define FLUDONE_ATTEMPTS                        20000
#define NVM_SUM                                 0xBABA		// Checksum value that should be in the checksum reg

#define NVM_WORD_SIZE_BASE_SHIFT				6
#define EECD_FLUDONE_I210                       0x04000000  // Flash update complete
#define EECD_SIZE_EX_MASK                       0x00007800  // Mask to extract NVM size
#define EECD_SIZE_EX_SHIFT                      11          // Number of bits to shift to get the size

#define FLASH_4K_SECTOR_SIZE            0x1000
#define FLSW_MAX_FLASH_DWORDS_TO_READ   ((FLASH_4K_SECTOR_SIZE - 1) >> 2)
#define FLSW_MAX_FLASH_DWORDS_TO_WRITE  (256 >> 2)
#define FLASH_ERASE_POLL                        100000
#define FLSWCTL_GL_DONE                 (1 << 31)
#define FLSWCTL_DONE                    (1 << 30)
#define FLSWCTL_BUSY                    (1 << 29)
#define FLSWCTL_CMDV                    (1 << 28)

#define FLSWCTL_ADDRESS_MASK          (0x00FFFFFF)
#define FLSWCTL_CMD_MASK              (0x0F000000)
#define FLSWCTL_CMD_READ              (0x00000000)
#define FLSWCTL_CMD_WRITE             (0x01000000)
#define FLSWCTL_CMD_SECTOR_ERASE      (0x02000000)
#define FLSWCTL_CMD_DEVICE_ERASE      (0x03000000)
#define FLSWCTL_CMD_WRITE_ENABLE      (0x06000000)


/* For checksumming, the sum of all words in the NVM should equal 0xBABA. */
#define INVM_DATA_REG(_n)				(0x12120 + 4 * (_n))   // Represent register offsets using this equation
#define EECD_FLUPD_I210                 0x00800000             // Update Flash

/*Register definitions for X550*/
#define EEC_X550                        0x10010
#define SWSM_X550                       0x10140
#define SW_FW_SYNC_X550                 0x10160   // Sync register for SW and FW
#define FLEX_MNG                        0x15800  /*0x15800 - 0x15EFC : Ram Space*/
#define HICR                            0x15F00
#define FWSTS                           0x15F0C

/* Register definitions for X553 */
#define SWSM_X553                       0x15f70
#define SW_FW_SYNC_X553                 0x15f78
#define STATUS_X553                     0x00008
#define GSSR_EEP_SM_X553                0x0001
#define GSSR_PHY0_SM_X553               0x0002
#define GSSR_PHY1_SM_X553               0x0004
#define GSSR_MAC_CSR_SM_X553			0x0008
#define GSSR_I2C_MASK_X553              0x1800

#define SWSM_SMBI_X550                  0x00000001
#define SWFW_EEP_SM_X550				0x0001
#define SWFW_FLASH_SM_X550				0x0010
#define SWFW_NVM_PHY_MASK_X550			0xF
#define SWFW_SW_MNG_SM_X550				0x0400
#define SWFW_REGSMP_X550                0x80000000
#define HICR_EN                         0x01  /* Enable bit - RO */
#define HICR_C							0x02
#define HICR_SV                         0x04  /* Status Validity */
#define FWSTS_FWRI                      0x00000200 /* Firmware Reset Indication */

/*Commands to write/read buffer for X550*/
#define FW_WRITE_SHADOW_RAM_CMD         0x33
#define FW_WRITE_SHADOW_RAM_LEN         0xA
#define FW_READ_SHADOW_RAM_CMD          0x31
#define FW_READ_SHADOW_RAM_LEN          0x6
#define FW_SHADOW_RAM_DUMP_CMD          0x36
#define FW_SHADOW_RAM_DUMP_LEN          0
#define FW_SHADOW_RAM_ERASE_CMD         0x35
#define FW_SHADOW_RAM_ERASE_LEN         0x5
#define FW_DEFAULT_CHECKSUM             0xFF

#define BURST_SIZE						128		// safe values: 8, 16, 32, 64, 128


struct PciDevice;

struct NvmCommand;

struct HicCommand;

struct Request;

struct Response;

union UpdateCommand;

enum eeprom_type {
	eeprom_uninitialized = 0,
	eeprom_spi,
	flash,
	eeprom_none     /* No NVM support */
};
struct nvmops_t {
	//Function pointers for getting the semaphores for NVM actions
	s32 (* acquire)(struct PciDevice *, u32 mask);
	void (* release)(struct PciDevice *, u32 mask);

	//Function pointers for doing NVM actions.
	s32 (* read)(struct PciDevice *, u32, u16, u16 *);
	s32 (* write)(struct PciDevice *, u32, u16, u16 *);
	s32 (* erase)(struct PciDevice *);
	s32 (* sector_erase)(struct PciDevice *, u32);

	//Function pointers for checksum work
	s32 (* update)(struct PciDevice *);
	s32 (* validate)(struct PciDevice *);
	s32 (* calculate)(struct PciDevice *, u16 *, u32);
};

struct OtpOps {
	//Function pointers for getting the semaphores for NVM actions
	s32 (* acquire)(struct PciDevice *);
	void (* release)(struct PciDevice *);

	//Function pointers for doing NVM actions.
	s32 (* read)(struct PciDevice *, u16, u16, u16 *);
	s32 (* write)(struct PciDevice *, u16, u16, u16 *);

	//Function pointers for checksum work
	s32 (* update)(struct PciDevice *);
	s32 (* validate)(struct PciDevice *);
};

struct OtpData {
	u32 OtpContents[64];
	u32 RawFileContents[64];
};

// typedefs
struct PciDevice
{
	u8 InvmPresent;
	u8 NvmPresent;
	u8 NvmPresentX550;
	s16 FileDescriptor;
	u16 MemType;
	u16 NicId;
	u16 IsMapped;
	u16 DeviceId;
	u16 VendorId;
	u16 SubDeviceId;
	u16 SubVendorId;
	u16 LocationUnparsed;
	u16 BusNum;
	u16 DeviceNum;
	u16 FunctionNum;
	u32 FlashSize;
	u16 Unused;
	u32 wordsize;
	u64 Memory1Address;
	u64 Memory2Address;
	u32 SemaphoreDelay;
	void* Mapped1Address;
	void* Mapped2Address;
	u16 MemoryIOAddress;
	u16 AccessMode;
	u64 BaseRegAddress[6];
	s8 FileLocation[100];
	struct nvmops_t nvmops;
	struct OtpOps InvmOps;
	struct OtpData InvmData;
	enum eeprom_type type;

};

#pragma pack()

struct Request {
	u8 cmd;
	u8 buf_lenh;
	u8 buf_lenl;
	u8 checksum;
};

struct Response {
	u8 cmd;
	u8 buf_lenl;
	u8 buf_lenh_status;         /* 7-5: high bits of buf_len, 4-0: status */
	u8 checksum;
};

union UpdateCommand {
	struct Request req;
	struct Response rsp;
};

struct NvmCommand {
	union UpdateCommand hdr;
	//u8 Command;
	//u8 BufferLengthHigh;
	//u8 BufferLengthLow;
	//u8 CheckSum;
	u32 Address;
	u16 Length;
	u16 Pad2;
	u16 Data;
	u16 Pad3;
};

struct HicCommand {
	u8 Command;
	u8 BufferLength;
	union {
		u8 CommandRequest;
		u8 ResponseStatus;
	} CmdOrResp;
	u8 Checksum;
};
// functions
void ReleaseNvmI210(struct PciDevice * dev, u32 mask);
s32  GetHwSemaphoreI210(struct PciDevice * dev);
void PutHwSemaphoreI210(struct PciDevice * dev);
s32  WriteNvmSrwr(struct PciDevice * dev, u32 offset, u16 words, u16 * data);
s32  PollFlashUpdateDoneI210(struct PciDevice * dev);

void* MapAddress(struct PciDevice* Dev, u32 Address);
void  UnmapDevice(struct PciDevice* Dev);
s32   AquireDeviceLock(struct PciDevice* Dev);
void  TestBarWrite(struct PciDevice * dev, u32 offset, u32 word);
void  ReadPciData(void* StartPosition, u32 Offset, u32* Data);
u32   ReadReg(struct PciDevice * Dev, u32 Offset);
s32   WriteReg(struct PciDevice * Dev, u32 Offset, u32 Data);
void* MapPciDevice(struct PciDevice* Dev);
void  GetDevice(struct PciDevice* Device);
void  DisplayAllDevices();
void  PciDevice(struct PciDevice * dev);
void  DetermineNvmType(struct PciDevice * dev);

s32 AcquireNvmX550(struct PciDevice * dev, u32 mask);
void ReleaseNvmX550(struct PciDevice * dev, u32 mask);
s32 GetHwSemaphoreX550(struct PciDevice * dev);
void PutHwSemaphoreX550(struct PciDevice * dev);
s32 ReadNvmBufferX550(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
s32 WriteNvmBuffer550(struct PciDevice * dev, u32 offset, u16 words, u16 * data);
s32 WriteNvmData550(struct PciDevice * dev, u32 Offset, u16 Data);
s32 ValidateNvmChecksumX550(struct PciDevice * dev);
s32 UpdateNvmChecksumX550(struct PciDevice * dev);
s32 UpdateFlashX550(struct PciDevice * dev);
s32 CalculateChecksumX550(struct PciDevice * dev, u16 * buffer, u32 size);
s32 HostInterfaceCommandX550(struct PciDevice * dev, u32 * buffer, u32 length, u32 timeout, bool value);
bool IsX553(struct PciDevice * dev);
s32 AcquireNvmX553(struct PciDevice * dev, u32 mask);
void ReleaseNvmX553(struct PciDevice * dev, u32 mask);
s32 GetHwSemaphoreX553(struct PciDevice * dev);
void PutHwSemaphoreX553(struct PciDevice * dev);
s32 UpdateNvmChecksumX553(struct PciDevice * dev);
s32 CalculateChecksumX553(struct PciDevice * dev, u16 * buffer, u32 size);
void PutHwSemaphore82574(struct PciDevice * dev, u32 mask);
s32  GetHwSemaphore82574(struct PciDevice * dev, u32 mask);
s32  UpdateFlashI210(struct PciDevice * dev);
s32  ReadNvmEerd(struct PciDevice * dev, u32 offset, u16 words, u16 * data);
s32  DumpInvmI211(struct PciDevice * dev, u32 * buffer);
s32  NullWriteNvm(struct PciDevice * dev, u32 a, u16 b, u16 * c);
s32  NullOpsGeneric(struct PciDevice * dev);
s32  WriteNvmEewr82574(struct PciDevice * dev, u32 offset, u16 words, u16 * data);
s32  UpdateNvmChecksum82574(struct PciDevice * dev);
s32  ValidateNvmChecksum82574(struct PciDevice * dev);
s32 PollEerdEewrDone(struct PciDevice * dev, u16 ee_reg);
s32  AcquireNvmI210(struct PciDevice* dev, u32 mask);
s32 ReadNvmSrrdI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
s32 WriteNvmSrwrI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
s32  InitNvmParams82574(struct PciDevice * DeviceInfo);
s32  InitNvmParamsI210(struct PciDevice * DeviceInfo);
s32  InitNvmParamsI211(struct PciDevice * DeviceInfo);
s32  InitNvmParamsX550(struct PciDevice * DeviceInfo);
s32 InitNvmParamsX553(struct PciDevice * DeviceInfo);
s32 InitNvmParamsNull(struct PciDevice * dev);

s32  AcquireFlswI210(struct PciDevice* dev, u32 mask);
void ReleaseFlswI210(struct PciDevice * dev, u32 mask);
s32 ReadNvmFlswI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
s32 WriteNvmFlswI210(struct PciDevice * dev, u32 Offset, u16 NumWords, u16 * Data);
u32  GetNvmWordSize(struct PciDevice * Dev);
u32  GetNvmWordSizeX550(struct PciDevice * Dev);
void CloseDevice(struct PciDevice* DeviceInfo);
s32 WriteFlswFlashCommand(struct PciDevice * dev, u32 Opcode, u32 Address);
u8 FlashDetectAndSetWritable(struct PciDevice * dev);
s8 SetupWriteMethod(struct PciDevice * dev);
#endif /* PCIE_EEPROM_H_ */
