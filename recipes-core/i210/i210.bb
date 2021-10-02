#
# This file is the i210 programming recipe.
#

SUMMARY = "Intel I210 eepromaccesstool application"
SECTION = "ROTOTYPE/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://main.c \
           file://testfuncs.c \
           file://PciEeprom.c \
           file://HelperFunctions.c \
           file://include/common.h \
           file://include/helpercode.h \
           file://include/HelperFunctions.h \
           file://include/PciEeprom.h \
           file://include/testfuncs.h \
	   file://Makefile \
  	   "

S = "${WORKDIR}"

do_compile() {
	     oe_runmake
}

do_install() {
	     install -d ${D}${bindir}
	     install -m 0755 EepromAccessTool ${D}${bindir}
}


