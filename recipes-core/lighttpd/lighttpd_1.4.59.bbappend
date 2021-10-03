FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI_append = "file://index.html.lighttpd \
"


do_install_append() {
    install -m 0644 ${WORKDIR}/index.html.lighttpd ${D}/www/pages/index.html
}

