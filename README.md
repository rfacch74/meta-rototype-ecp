meta-rototype-ecp
================

Based on NXP Yocto hardknott 3.3 


```
mkdir imx-yocto-bsp
cd imx-yocto-bsp
repo init -u https://source.codeaurora.org/external/imx/imx-manifest -b imx-linux-hardknott -m imx-5.10.35-2.0.0.xml
repo sync
```

Clone git
--------------

```
cd ~/imx-yocto-bsp/source
git clone git://github.com/rfacch74/meta-rototype-ecp.git;protocol=https
```


SOMs supported
--------------

- imx8mq-ecp

Supported distros
-----------------

- ecp-imx-wayland: Distro for Wayland without X11. This distro includes wayland feature but doesn’t have X11 support.


Images available
----------------

- rototype-evaluation-image

First build
-----------

```
DISTRO=ecp-imx-wayland MACHINE=imx8mq-ecp source imx-setup-release.sh -b build-rototype-ecp
bitbake-layers add-layer ../sources/meta-rototype-ecp
bitbake rototype-evaluation-image
```
