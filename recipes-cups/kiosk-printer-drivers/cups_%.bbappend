# install CUSTOM VKP80 Driver on CUPS

FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
        file://printers.conf \
        file://cupsd.conf \
        file://CUSTOM_Engineering_VKP80.ppd  \
        file://rastertovkp80 \
"

do_install_append(){
    install -m 644 ${WORKDIR}/cupsd.conf    ${D}${sysconfdir}/cups
    install -m 644 ${WORKDIR}/printers.conf    ${D}${sysconfdir}/cups
    install -d ${D}${sysconfdir}/cups/ppd
   install -m 644 ${WORKDIR}/CUSTOM_Engineering_VKP80.ppd    ${D}${sysconfdir}/cups/ppd
   install -m 755 ${WORKDIR}/rastertovkp80    ${D}${exec_prefix}/libexec/cups/filter
}

