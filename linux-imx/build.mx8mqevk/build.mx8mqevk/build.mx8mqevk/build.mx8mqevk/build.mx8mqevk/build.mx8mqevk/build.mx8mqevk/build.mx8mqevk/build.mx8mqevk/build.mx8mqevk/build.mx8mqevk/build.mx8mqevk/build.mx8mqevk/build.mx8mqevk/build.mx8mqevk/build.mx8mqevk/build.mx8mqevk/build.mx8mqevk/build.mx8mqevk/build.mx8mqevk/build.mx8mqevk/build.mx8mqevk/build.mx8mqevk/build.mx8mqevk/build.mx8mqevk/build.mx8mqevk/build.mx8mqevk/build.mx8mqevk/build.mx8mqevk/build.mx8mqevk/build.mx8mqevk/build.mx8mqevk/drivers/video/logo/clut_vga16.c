/*
 *  DO NOT EDIT THIS FILE!
 *
 *  It was automatically generated from /home/yin/imx8mqevk/linux-imx/drivers/video/logo/clut_vga16.ppm
 *
 *  Linux logo clut_vga16
 */

#include <linux/linux_logo.h>

static unsigned char clut_vga16_data[] __initdata = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef
};

const struct linux_logo clut_vga16 __initconst = {
	.type		= LINUX_LOGO_VGA16,
	.width		= 16,
	.height		= 1,
	.data		= clut_vga16_data
};
