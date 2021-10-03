# Example of recipe-core/psplash/psplash_git.bbappend
# (insert ecp-psplash.png image in recipe-core/psplash/files folder)

FILESEXTRAPATHS_prepend := "${THISDIR}/files:"
SPLASH_IMAGES = "file://ecp-psplash.png;outsuffix=default"

