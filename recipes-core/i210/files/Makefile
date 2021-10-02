#/***********************************************************************************
# * Copyright 2017-2018 Intel Corporation All Rights Reserved.
# *
# * This EEPROM Access Tool example code ("Software") is furnished under license
# * and may only be used or copied in accordance with the terms of that license.
# * No license, express or implied, by estoppel or otherwise, to any
# * intellectual property rights is granted by this document. The Software
# * is subject to change without notice, and should not be construed as a
# * commitment by Intel Corporation to market, license, sell or support
# * any product or technology. Unless otherwise provided for in the license
# * under which this Software is provided, the Software is provided AS IS,
# * with no warranties of any kind, express or implied. Except as expressly
# * permitted by the Software license, neither Intel Corporation nor its
# * suppliers assumes any responsibility or liability for any errors or
# * inaccuracies that may appear herein. Except as expressly permitted
# * by the Software license, no part of the Software may be reproduced,
# * stored in a retrieval system, transmitted in any form, or distributed
# * by any means without the express written consent of Intel Corporation.
# *
# * The source code, information and material ("Material") contained herein
# * is owned by Intel Corporation or its suppliers or licensors, and
# * title to such Material remains with Intel Corporation or its suppliers
# * or licensors. The Material contains proprietary information of Intel or
# * its suppliers and licensors. The Material is protected by worldwide copyright
# * laws and treaty provisions. No part of the Material may be used, copied,
# * reproduced, modified, published, uploaded, posted, transmitted, distributed
# * or disclosed in any way without Intel's prior express written permission.
# * No license under any patent, copyright or other intellectual property rights
# * in the Material is granted to or conferred upon you, either expressly, by
# * implication, inducement, estoppel or otherwise. Any license under such
# * intellectual property rights must be express and approved by Intel in writing.
# *
# * Third Party trademarks are the property of their respective owners.
# *
# * Unless otherwise agreed by Intel in writing, you may not remove or alter this
# * notice or any other notice embedded in Materials by Intel or Intel's
# * suppliers or licensors in any way.
# ********************************************************************************* */

APP = EepromAccessTool

# Add any other object files to this list below
APP_OBJS = testfuncs.o PciEeprom.o HelperFunctions.o main.o

all: build

build: $(APP)

$(APP): $(APP_OBJS)
	$(CC) -o $@ $(APP_OBJS) $(DEFINES) $(CFLAGS) $(LDFLAGS) -DCONFIG_GENERIC_IOMAP
clean:
	rm -f $(APP) *.o

