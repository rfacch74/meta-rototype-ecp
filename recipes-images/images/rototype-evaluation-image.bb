# Copyright (C) 2015 Freescale Semiconductor
# Copyright 2017-2019 NXP
# Released under the MIT license (see COPYING.MIT for the terms)

DESCRIPTION = "Rototype evaluation image"
LICENSE = "MIT"

inherit core-image
#inherit populate_sdk_qt5

### WARNING: This image is NOT suitable for production use and is intended
###          to provide a way for users to reproduce the image used during
###          the validation process of Engicam SOM

## Select Image Features
IMAGE_FEATURES += " \
	debug-tweaks \
	tools-profile \
	tools-debug \
	tools-testapps \
	package-management \
	splash \
	nfs-server \
	ssh-server-dropbear \
	hwcodecs \
	${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'weston','', d)} \
"

SDKIMAGE_FEATURES_append = " \
    staticdev-pkgs \
"


ERPC_COMPS ?= ""

ISP_PKGS = ""


# Install fonts
QT5_FONTS = "ttf-dejavu-common ttf-dejavu-sans \
	     ttf-dejavu-sans-mono ttf-dejavu-serif "

# Install qtquick3d
QT5_QTQUICK3D = "qtquick3d"

QT5_IMAGE = " \
    ${QT5_QTQUICK3D} \
    ${QT5_FONTS} \
    ${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'qtwayland qtwayland-plugins', '', d)}\
    qtdeclarative \
    qtquickcontrols2 \
    qtquickcontrols-qmlplugins \
    qtmqtt \
    qtmultimedia \
    qtserialport \
    qtserialbus \
    qtwebsockets \
    qtwebengine \
    qt-kiosk-browser \
    qtwayland \
"

CLINFO ?= ""
CLINFO_imxgpu = "clinfo"
CLINFO_mx8mm = ""

PKG_DEBUG = "\
	devmem2 \
	dosfstools \
	e2fsprogs \
	ethtool \
	evtest \
	i2c-tools \
	iproute2 \
	minicom \
	parted \
	e2fsprogs-resize2fs \
        e2fsprogs-mke2fs  \
        util-linux \
	tslib \
	tslib-calibrate \
	tslib-conf \
	tslib-tests \
	mtd-utils \
	mtd-utils-misc \ 
	usbutils \
	linux-firmware \
	nvme-cli \
	ldd \
"

PKG_ECP = "\
	i210 \
	rototypescript \
	lighttpd \
	libexif  \
	curl \
	cups  cups-doc  cups-filters  cups-libimage \
	ghostscript \
        hplip  \
"
IMAGE_INSTALL += " \
	packagegroup-core-full-cmdline \
	packagegroup-fsl-tools-audio \
	packagegroup-fsl-gstreamer1.0 \
	packagegroup-qt5-qtcreator-debug \
	packagegroup-core-buildessential \ 
	${@bb.utils.contains('DISTRO_FEATURES', 'wayland', 'weston-init', '', d)} \
	${ERPC_COMPS} \
	${ISP_PKGS} \
	${QT5_IMAGE} \
	${PKG_DEBUG} \
        ${PKG_ECP} \
	${CLINFO} \
"

