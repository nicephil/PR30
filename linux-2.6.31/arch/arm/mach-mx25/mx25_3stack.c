/*
 * Copyright 2008-2009 Freescale Semiconductor, Inc. All Rights Reserved.
 */

/*
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/nodemask.h>
#include <linux/clk.h>
#include <linux/spi/spi.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
//#include <linux/smsc911x.h>
#if defined(CONFIG_MTD) || defined(CONFIG_MTD_MODULE)
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#include <asm/mach/flash.h>
#endif

#include <mach/common.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/mach/keypad.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/irq.h>
#include <asm/mach/time.h>
#include <mach/memory.h>
#include <mach/gpio.h>
#include <mach/mmc.h>

#include "board-mx25_3stack.h"
#include "crm_regs.h"
#include "iomux.h"
#if 1 /* UPLEONG */
#include <linux/leds.h>
#endif /* UPLEONG */
#if 1 /* UPLEONG */
#include <linux/spi/spi_gpio.h>
#include <linux/delay.h>
#endif /* UPLEONG */

/*!
 * @file mach-mx25/mx25_3stack.c
 *
 * @brief This file contains the board specific initialization routines.
 *
 * @ingroup MSL_MX25
 */

unsigned int mx25_3stack_board_io;

/* working point(wp): 0 - 399MHz; 1 - 266MHz; 2 - 133MHz; */
/* 24MHz input clock table */
static struct cpu_wp cpu_wp_mx25[] = {
	{
	 .pll_rate = 399000000,
	 .cpu_rate = 399000000,
	 .cpu_podf = 0x0,
	 .cpu_voltage = 1450000},
	{
	 .pll_rate = 532000000,
	 .cpu_rate = 266000000,
	 .cpu_podf = 0x1,
	 .cpu_voltage = 1340000},
	{
	 .pll_rate = 532000000,
	 .cpu_rate = 133000000,
	 .cpu_podf = 0x3,
	 .cpu_voltage = 1340000},
};
struct cpu_wp *get_cpu_wp(int *wp)
{
	*wp = 3;
	return cpu_wp_mx25;
}

static void mxc_nop_release(struct device *dev)
{
	/* Nothing */
}

#if defined(CONFIG_KEYBOARD_MXC) || defined(CONFIG_KEYBOARD_MXC_MODULE)
static u16 keymapping[16] = {
	KEY_UP, KEY_DOWN, KEY_VOLUMEDOWN, KEY_HOME,
	KEY_RIGHT, KEY_LEFT, KEY_ENTER, KEY_VOLUMEUP,
	KEY_F6, KEY_F8, KEY_F9, KEY_F10,
	KEY_F1, KEY_F2, KEY_F3, KEY_POWER,
};

static struct resource mxc_kpp_resources[] = {
	[0] = {
	       .start = MXC_INT_KPP,
	       .end = MXC_INT_KPP,
	       .flags = IORESOURCE_IRQ,
	       }
};

static struct keypad_data keypad_plat_data = {
	.rowmax = 4,
	.colmax = 4,
	.irq = MXC_INT_KPP,
	.learning = 0,
	.delay = 2,
	.matrix = keymapping,
};

/* mxc keypad driver */
static struct platform_device mxc_keypad_device = {
	.name = "mxc_keypad",
	.id = 0,
	.num_resources = ARRAY_SIZE(mxc_kpp_resources),
	.resource = mxc_kpp_resources,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &keypad_plat_data,
		},
};

static void mxc_init_keypad(void)
{
	(void)platform_device_register(&mxc_keypad_device);
}
#else

#if 1
#include <linux/input.h>
#include <linux/gpio_keys.h>

static struct gpio_keys_button mxc_gpio_keys_table[] = {
	{BTN_0, IOMUX_TO_GPIO(MX25_PIN_GPIO_E), 1, "gpio-keys: WPS", EV_KEY, 1 },
	{BTN_1, IOMUX_TO_GPIO(MX25_PIN_GPIO_D), 1, "gpio-keys: SW2", EV_KEY, 1 },
	{KEY_POWER, IOMUX_TO_GPIO(MX25_PIN_GPIO_F), 1, "gpio-keys: POWER", EV_PWR, 1 },
//	{KEY_POWER, IOMUX_TO_GPIO(MX25_PIN_A25), 1, "gpio-keys: POWER", EV_PWR, 1 },
};

static struct gpio_keys_platform_data mxc_gpio_keys_data = {
	.buttons        = mxc_gpio_keys_table,
	.nbuttons       = ARRAY_SIZE(mxc_gpio_keys_table),
};

static struct platform_device mxc_device_gpiokeys = {
	.name      = "gpio-keys",
	.dev = {
		.platform_data = &mxc_gpio_keys_data,
	},
};
#endif

static inline void mxc_init_keypad(void)
{
#if 1
	(void)platform_device_register(&mxc_device_gpiokeys);
#endif
}
#endif

#if 1
static struct platform_device otg_switch_device = {
	.name      = "otg-switch",
	.dev = {
		.platform_data = NULL,
	},
};

static inline void mxc_init_otg_switch(void)
{
	(void)platform_device_register(&otg_switch_device);
}
#endif

/* MTD NAND flash */

#if defined(CONFIG_MTD_NAND_MXC_V2) || defined(CONFIG_MTD_NAND_MXC_V2_MODULE)

#define MTD_SIZE_UBOOT		1
#define MTD_SIZE_DATA		1
#define MTD_SIZE_KERNEL		6
#define MTD_SIZE_ROOTFS		10
#define MTD_SIZE_USERDATA	46

static struct mtd_partition mxc_nand_partitions[] = {
#if 0 /* UPLEONG */
	{
	 .name = "nand.bootloader",
	 .offset = 0,
	 .size = 3 * 1024 * 1024},
	{
	 .name = "nand.kernel",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 5 * 1024 * 1024},
	{
	 .name = "nand.rootfs",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 256 * 1024 * 1024},
	{
	 .name = "nand.configure",
	 .offset = MTDPART_OFS_APPEND,
	 .size = 8 * 1024 * 1024},
	{
	 .name = "nand.userfs",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTDPART_SIZ_FULL},
#else
	{
	 .name = "u-boot",
	 .offset = 0,
	 .size = MTD_SIZE_UBOOT * 1024 * 1024},
	{
         .name = "data",
         .offset = MTDPART_OFS_APPEND,
         .size = MTD_SIZE_DATA * 1024 * 1024},
	{
	 .name = "kernel",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTD_SIZE_KERNEL * 1024 * 1024},   // no compression for faster boot up
	{
	 .name = "rootfs",
	 .offset = 0x800000,   // Cannot be MTDPART_OFS_APPEND for ROOTFS_AUTOSPLIT
	 .size = MTD_SIZE_ROOTFS * 1024 * 1024},
	{
	 .name = "openwrt",
	 .offset = 0x200000,
	 .size = (MTD_SIZE_KERNEL + MTD_SIZE_ROOTFS) * 1024 * 1024},
	{
	 .name = "userdata",
	 .offset = MTDPART_OFS_APPEND,
	 .size = MTD_SIZE_USERDATA * 1024 * 1024},
#if 0
        {
         .name = "rescue",
         .offset = MTDPART_OFS_APPEND,
         .size = (4 + 8) * 1024 * 1024},
#endif 
//	{
//	 .name = "nand.full",
//	 .offset = MTDPART_OFS_APPEND,
//	 .size = MTDPART_SIZ_FULL},
#endif /* UPLEONG */
};

static struct flash_platform_data mxc_nand_data = {
	.parts = mxc_nand_partitions,
	.nr_parts = ARRAY_SIZE(mxc_nand_partitions),
	.width = 1,
};

static struct platform_device mxc_nand_mtd_device = {
	.name = "mxc_nandv2_flash",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mxc_nand_data,
		},
};

static void mxc_init_nand_mtd(void)
{
	if (__raw_readl(MXC_CCM_RCSR) & MXC_CCM_RCSR_NF16B)
		mxc_nand_data.width = 2;

	platform_device_register(&mxc_nand_mtd_device);
}
#if 1 /* UPLEONG */
static struct gpio_led pr30_leds[] = {
    { .name = "power:green", .gpio = IOMUX_TO_GPIO(MX25_PIN_CSI_D7), .default_trigger = "heartbeat", },
    { .name = "umts:red", .gpio = 8, .default_trigger = "timer", },
    { .name = "umts:green", .gpio = IOMUX_TO_GPIO(MX25_PIN_CSI_VSYNC), .default_trigger = "timer", },
    { .name = "umts:blue", .gpio = IOMUX_TO_GPIO(MX25_PIN_CSI_HSYNC), .default_trigger = "timer", },
    { .name = "wifi:green", .gpio = IOMUX_TO_GPIO(MX25_PIN_CSI_PIXCLK), .default_trigger = "morse", },
};

static struct gpio_led_platform_data pr30_led_data = {
    .num_leds	= ARRAY_SIZE(pr30_leds),
    .leds		= pr30_leds,
};

static struct platform_device pr30_led_dev = {
	.name		= "leds-gpio",
	.id		= -1,
	.dev		= {
	.platform_data	= &pr30_led_data,
    }
};

static struct platform_device *pr30_devs[] = {
    &pr30_led_dev,
};


static inline void pr30_init_led(void)
{
    //pr_info("******** pr30_init_led...  platform_add_devices...\n");
    platform_add_devices(pr30_devs, ARRAY_SIZE(pr30_devs));
}
#endif /* UPLEONG */
#if 0 /* UPLEONG */
/* LCD */
static struct spi_gpio_platform_data pr30_spi_gpio_data = {
    .sck	= IOMUX_TO_GPIO(MX25_PIN_I2C1_CLK),
    .mosi	= IOMUX_TO_GPIO(MX25_PIN_I2C1_DAT),
    //.sck	= 11,  // test with LED
    //.mosi	= 9,  // test with LED
//    .miso	= IOMUX_TO_GPIO(MX25_PIN_I2C1_DAT),
    .num_chipselect	= 1,
};


static struct platform_device pr30_spi_gpio_dev = {
	.name		= "spi_gpio",
	.id		= 0,
	.dev		= {
	.platform_data	= &pr30_spi_gpio_data,
    }
};

#if 0
static struct platform_device *pr30_spi_devs[] = {
    &pr30_spi_gpio_dev,
};
#endif /* 0 */

static struct spi_board_info pr30_spi_gpio_board_info[] = {
    {
	.modalias = "spidev",
	//.max_speed_hz = 48000000, // 48 Mhz
	//.max_speed_hz = 25000000, // 25 Mhz
	.max_speed_hz = 500000, // 25 Mhz
	.bus_num = 0,
	//.mode = SPI_MODE_1,
	.chip_select = 0,
	.controller_data = (void *) IOMUX_TO_GPIO(MX25_PIN_CSPI1_SS0),
	//.controller_data = (void *) 10,  // test with LED
//	.controller_data = (void *) 16,
    },
};


static inline void pr30_init_lcd(void)
{
    // set gpio

    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 0);	/* OLED_RESET_N */
    udelay(5);		/* spec says 3us min */
    gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_VSTBY_ACK), 1);	/* OLED_RESET_N */


    pr_info("******** pr30_init_lcd...  platform_add_devices...\n");

    spi_register_board_info (pr30_spi_gpio_board_info, ARRAY_SIZE(pr30_spi_gpio_board_info));
    platform_device_register (&pr30_spi_gpio_dev);
//    platform_add_devices(pr30_spi_devs, ARRAY_SIZE(pr30_spi_devs));
}
#endif /* UPLEONG */
#else
static inline void mxc_init_nand_mtd(void)
{
}
#endif

#if defined(CONFIG_FB_MXC_SYNC_PANEL) || \
    defined(CONFIG_FB_MXC_SYNC_PANEL_MODULE)
static const char fb_default_mode[] = "CPT-VGA";

/* mxc lcd driver */
static struct platform_device mxc_fb_device = {
	.name = "mxc_sdc_fb",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &fb_default_mode,
		.coherent_dma_mask = 0xFFFFFFFF,
		},
};

/*
 * Power on/off CPT VGA panel.
 */
void board_power_lcd(int on)
{
	if (on)
		mx2fb_set_brightness(MXC_DEFAULT_INTENSITY);
	else
		mx2fb_set_brightness(MXC_INTENSITY_OFF);
}
EXPORT_SYMBOL_GPL(board_power_lcd);

static void mxc_init_fb(void)
{
	(void)platform_device_register(&mxc_fb_device);
}
#else

#if 1
static const char fb_default_mode[] = "MONO";

/* mxc slcd driver */
static struct platform_device mxc_fb_device = {
	.name = "slcdc",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &fb_default_mode,
		.coherent_dma_mask = 0xFFFFFFFF,
		},
};
#endif

static inline void mxc_init_fb(void)
{
#if 1
	(void)platform_device_register(&mxc_fb_device);
#endif
}
#endif

#if defined(CONFIG_BACKLIGHT_MXC)
static struct platform_device mxcbl_devices[] = {
#if defined(CONFIG_BACKLIGHT_MXC_LCDC) || \
    defined(CONFIG_BACKLIGHT_MXC_LCDC_MODULE)
	{
	 .name = "mxc_lcdc_bl",
	 .id = 0,
	 },
#endif
#if defined(CONFIG_BACKLIGHT_MXC_SLCDC) || \
    defined(CONFIG_BACKLIGHT_MXC_SLCDC_MODULE)
	{
	 .name = "mxc_slcdc_bl",
	 .id = 0,
	 },
#endif
};

static inline void mxc_init_bl(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mxcbl_devices); i++)
		platform_device_register(&mxcbl_devices[i]);
}
#else
static inline void mxc_init_bl(void)
{
}
#endif

#if 1
#else
/*!
 * Power Key interrupt handler.
 */
static irqreturn_t power_key_int(int irq, void *dev_id)
{
	pr_info("on-off key pressed\n");
	return 0;
}

/*!
 * Power Key initialization.
 */
static int __init mxc_init_power_key(void)
{
	/*Set power key as wakeup resource */
	int irq, ret;

	mxc_request_iomux(MX25_PIN_A25, MUX_CONFIG_ALT5);
	mxc_iomux_set_pad(MX25_PIN_A25, PAD_CTL_DRV_NORMAL);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_A25), NULL);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_A25));

	irq = IOMUX_TO_IRQ(MX25_PIN_A25);
	set_irq_type(irq, IRQF_TRIGGER_RISING);
	ret = request_irq(irq, power_key_int, 0, "power_key", 0);
	if (ret)
		pr_info("register on-off key interrupt failed\n");
	else
		enable_irq_wake(irq);

	return ret;
}
late_initcall(mxc_init_power_key);

/*
** Add on-off and WPS button
*/
static irqreturn_t on_off_key_int(int irq, void *dev_id)
{
	pr_info("developemnt on-off key pressed\n");
	return 0;
}

static int __init mxc_init_on_off_key(void)
{
	/*Set on-off key as wakeup resource */
	int irq, ret;

	mxc_request_iomux(MX25_PIN_GPIO_F, MUX_CONFIG_FUNC);
	mxc_iomux_set_pad(MX25_PIN_GPIO_F, PAD_CTL_DRV_NORMAL);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_GPIO_F), NULL);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_F));

	irq = IOMUX_TO_IRQ(MX25_PIN_GPIO_F);
	set_irq_type(irq, IRQF_TRIGGER_RISING);
	ret = request_irq(irq, on_off_key_int, 0, "on_off_key", 0);
	if (ret)
		pr_info("register on-off key interrupt failed\n");
	else
		enable_irq_wake(irq);
	return ret;
}

late_initcall(mxc_init_on_off_key);

static irqreturn_t wps_key_int(int irq, void *dev_id)
{
	pr_info("WPS key pressed\n");
	return 0;
}

static int __init mxc_init_wps_key(void)
{
	/*Set wps key as wakeup resource */
	int irq, ret;

	mxc_request_iomux(MX25_PIN_GPIO_E, MUX_CONFIG_FUNC);
	mxc_iomux_set_pad(MX25_PIN_GPIO_E, PAD_CTL_DRV_NORMAL);
	gpio_request(IOMUX_TO_GPIO(MX25_PIN_GPIO_E), NULL);
	gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_E));

	irq = IOMUX_TO_IRQ(MX25_PIN_GPIO_E);
	set_irq_type(irq, IRQF_TRIGGER_RISING);
	ret = request_irq(irq, wps_key_int, 0, "wps_key", 0);
	if (ret)
		pr_info("register on-off key interrupt failed\n");
	else
		enable_irq_wake(irq);
	return ret;
}

late_initcall(mxc_init_wps_key);

/* ----END */

/*
** Add SW22 button
*/
static irqreturn_t sw2_key_int(int irq, void *dev_id)
{
  pr_info("sw2 key pressed\n");
  return 0;
}

static int __init mxc_init_sw2_key(void)
{
  int irq, ret;

  mxc_request_iomux(MX25_PIN_GPIO_D, MUX_CONFIG_FUNC);
  mxc_iomux_set_pad(MX25_PIN_GPIO_D, PAD_CTL_DRV_NORMAL);
  gpio_request(IOMUX_TO_GPIO(MX25_PIN_GPIO_D), NULL);
  gpio_direction_input(IOMUX_TO_GPIO(MX25_PIN_GPIO_D));

  irq = IOMUX_TO_IRQ(MX25_PIN_GPIO_D);
  set_irq_type(irq, IRQF_TRIGGER_RISING);
  ret = request_irq(irq, sw2_key_int, 0, "sw2_key", 0);
  if (ret)
    pr_info("register sw2 key interrupt failed\n");
  else
    enable_irq_wake(irq);
  return ret;
}

late_initcall(mxc_init_sw2_key);
#endif

/* ----END */


static struct spi_board_info mxc_spi_board_info[] __initdata = {
	{
	 .modalias = "cpld_spi",
	 .max_speed_hz = 18000000,
	 .bus_num = 1,
	 .chip_select = 0,
	 .mode = SPI_MODE_2,
	 },
	{
	 .modalias = "wm8580_spi",
	 .max_speed_hz = 8000000,	/* max spi SCK clock speed in HZ */
	 .bus_num = 1,
	 .chip_select = 1,
	 },
};

static struct mxc_camera_platform_data camera_data = {
	.core_regulator = NULL,
	.io_regulator = NULL,
	.analog_regulator = NULL,
	.gpo_regulator = NULL,
	.mclk = 24000000,
};

static struct i2c_board_info mxc_i2c_board_info[] __initdata = {
	{
	 .type = "sgtl5000-i2c",
	 .addr = 0x0a,
	 },
	{
	 .type = "ak5702-i2c",
	 .addr = 0x13,
	 },
	{
	 .type = "ov2640",
	 .addr = 0x30,
	 .platform_data = (void *)&camera_data,
	 },
};

#if defined(CONFIG_SND_SOC_IMX_3STACK_SGTL5000) \
    || defined(CONFIG_SND_SOC_IMX_3STACK_SGTL5000_MODULE)
static struct mxc_audio_platform_data sgtl5000_data = {
	.ssi_num = 2,
	.src_port = 1,
	.ext_port = 4,
	.hp_irq = IOMUX_TO_IRQ(MX25_PIN_A10),
	.hp_status = headphone_det_status,
	.sysclk = 8300000,
};

static struct platform_device mxc_sgtl5000_device = {
	.name = "imx-3stack-sgtl5000",
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &sgtl5000_data,
		},
};

static void mxc_init_sgtl5000(void)
{
	struct clk *cko1, *parent;
	unsigned long rate;

	/* cko1 clock */
	mxc_request_iomux(MX25_PIN_CLKO, MUX_CONFIG_FUNC);

	cko1 = clk_get(NULL, "clko_clk");
	if (IS_ERR(cko1))
		return;
	parent = clk_get(NULL, "ipg_clk");
	if (IS_ERR(parent))
		return;
	clk_set_parent(cko1, parent);
	rate = clk_round_rate(cko1, 13000000);
	if (rate < 8000000 || rate > 27000000) {
		pr_err("Error: SGTL5000 mclk freq %ld out of range!\n", rate);
		clk_put(parent);
		clk_put(cko1);
		return;
	}
	clk_set_rate(cko1, rate);
	clk_enable(cko1);
	sgtl5000_data.sysclk = rate;
	sgtl5000_enable_amp();
	platform_device_register(&mxc_sgtl5000_device);
}
#else
static inline void mxc_init_sgtl5000(void)
{
}
#endif

#if 1
static void mxc_init_wlan_sleep_clk( void )
{
	struct clk *cko1, *parent;
	unsigned long rate;

	pr_info("%s()\n",__FUNCTION__ );

	cko1 = clk_get(NULL, "clko_clk");
	if (IS_ERR(cko1))
	{
		pr_err("%s():get clko_clk handle failue!\n",__FUNCTION__);
		return;
	}
	clk_disable(cko1);

	parent = clk_get(NULL, "osc32k");
	if (IS_ERR(parent))
	{
		pr_err("%s():get osc32k handle failue!\n",__FUNCTION__);
		return;
	}
	clk_set_parent(cko1, parent);

	rate = clk_round_rate(cko1, 32768);

	clk_set_rate(cko1, rate);

	clk_enable(cko1);
}
#endif

#if defined(CONFIG_SND_SOC_IMX_3STACK_AK5702) \
    || defined(CONFIG_SND_SOC_IMX_3STACK_AK5702_MODULE)
static struct platform_device mxc_ak5702_device = {
	.name = "imx-3stack-ak5702",
	.dev = {
		.release = mxc_nop_release,
		},
};

static void mxc_init_ak5702(void)
{
	platform_device_register(&mxc_ak5702_device);
}
#else
static inline void mxc_init_ak5702(void)
{
}
#endif

#if  defined(CONFIG_SMSC911X) || defined(CONFIG_SMSC911X_MODULE)
static struct resource smsc911x_resources[] = {
	{
	 .start = LAN9217_BASE_ADDR,
	 .end = LAN9217_BASE_ADDR + 255,
	 .flags = IORESOURCE_MEM,
	 },
	{
	 .start = MXC_BOARD_IRQ_START,
	 .flags = IORESOURCE_IRQ,
	 }
};

struct smsc911x_platform_config smsc911x_config = {
        .irq_polarity = SMSC911X_IRQ_POLARITY_ACTIVE_LOW,
        .flags = 0x8000 | SMSC911X_USE_16BIT | SMSC911X_FORCE_INTERNAL_PHY,
};

static struct platform_device smsc_lan9217_device = {
	.name = "smsc911x",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &smsc911x_config,
		},
	.num_resources = ARRAY_SIZE(smsc911x_resources),
	.resource = smsc911x_resources,
};

static int __init mxc_init_enet(void)
{
	(void)platform_device_register(&smsc_lan9217_device);
	return 0;
}
#else
static int __init mxc_init_enet(void)
{
	return 0;
}
#endif

#ifdef CONFIG_TIWLAN_SUPPORT
#include <linux/mmc/sdio_ids.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/host.h>
#include <asm/mach/mmc.h>

static struct sdio_embedded_func wifi_func[] =  {
                {
                .f_class        = SDIO_CLASS_NONE,
                .f_maxblksize   = 512,
                },
                {
                .f_class        = SDIO_CLASS_WLAN,
                .f_maxblksize   = 512,
                },
};

static struct embedded_sdio_data tiwlan_wifi_emb_data = {
        .cis    = {
                .vendor         = 0x104c,
                .device         = 0x9066,
                .blksize        = 512,
                .max_dtr        = 25000000,
        },
        .cccr   = {
                .multi_block    = 1,
                .low_speed      = 0,
                .wide_bus       = 1,
                .high_power     = 0,
                .high_speed     = 0,
        },
        .funcs  = &wifi_func[0],
        .num_funcs = 2,
};
#endif

late_initcall(mxc_init_enet);

#if defined(CONFIG_FEC) || defined(CONFIG_FEC_MODULE)
static struct resource mxc_fec_resources[] = {
	{
		.start	= FEC_BASE_ADDR,
		.end	= FEC_BASE_ADDR + 0xfff,
		.flags	= IORESOURCE_MEM
	}, {
		.start	= MXC_INT_FEC,
		.end	= MXC_INT_FEC,
		.flags	= IORESOURCE_IRQ
	},
};

struct platform_device mxc_fec_device = {
	.name = "fec",
	.id = 0,
	.num_resources = ARRAY_SIZE(mxc_fec_resources),
	.resource = mxc_fec_resources,
};

static __init int mxc_init_fec(void)
{
	return platform_device_register(&mxc_fec_device);
}
#else
static inline int mxc_init_fec(void)
{
	return 0;
}
#endif

#if defined(CONFIG_IMX_SIM) || defined(CONFIG_IMX_SIM_MODULE)
/* Used to configure the SIM bus */
static struct mxc_sim_platform_data sim1_data = {
	.clk_rate = 5000000,
	.clock_sim = "sim1_clk",
	.power_sim = NULL,
	.init = NULL,
	.exit = NULL,
	.detect = 1,
};

/*!
 * Resource definition for the SIM
 */
static struct resource mxc_sim1_resources[] = {
	[0] = {
	       .start = SIM1_BASE_ADDR,
	       .end = SIM1_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = MXC_INT_SIM1,
	       .end = MXC_INT_SIM1,
	       .flags = IORESOURCE_IRQ,
	       },
	[2] = {
	       .start = 0,
	       .end = 0,
	       .flags = IORESOURCE_IRQ,
	       },
};

/*! Device Definition for IMX SIM */
static struct platform_device mxc_sim1_device = {
	.name = "mxc_sim",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &sim1_data,
		},
	.num_resources = ARRAY_SIZE(mxc_sim1_resources),
	.resource = mxc_sim1_resources,
};

static inline void mxc_init_sim(void)
{
	(void)platform_device_register(&mxc_sim1_device);
}
#else
static inline void mxc_init_sim(void)
{
}
#endif

#if defined(CONFIG_MMC_IMX_ESDHCI) || defined(CONFIG_MMC_IMX_ESDHCI_MODULE)
static struct mxc_mmc_platform_data mmc1_data = {
	.ocr_mask = MMC_VDD_29_30 | MMC_VDD_32_33,
	.caps = MMC_CAP_4_BIT_DATA,
	.min_clk = 400000,
	.max_clk = 52000000,
	.card_inserted_state = 1,
	.status = sdhc_get_card_det_status,
	.wp_status = sdhc_write_protect,
	.clock_mmc = "esdhc_clk",
#ifdef CONFIG_TIWLAN_SUPPORT
        .embedded_sdio = &tiwlan_wifi_emb_data,
#endif
};

/*!
 * Resource definition for the SDHC1
 */
static struct resource mxcsdhc1_resources[] = {
	[0] = {
	       .start = MMC_SDHC1_BASE_ADDR,
	       .end = MMC_SDHC1_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = MXC_INT_SDHC1,
	       .end = MXC_INT_SDHC1,
	       .flags = IORESOURCE_IRQ,
	       },
	[2] = {
	       .start = IOMUX_TO_IRQ(MX25_PIN_A15),
	       .end = IOMUX_TO_IRQ(MX25_PIN_A15),
	       .flags = IORESOURCE_IRQ,
	       },
};

/*! Device Definition for MXC SDHC1 */
static struct platform_device mxcsdhc1_device = {
	.name = "mxsdhci",
	.id = 0,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mmc1_data,
		},
	.num_resources = ARRAY_SIZE(mxcsdhc1_resources),
	.resource = mxcsdhc1_resources,
};

#ifdef CONFIG_MMC_IMX_ESDHCI_SELECT2
static struct mxc_mmc_platform_data mmc2_data = {
	.ocr_mask = MMC_VDD_29_30 | MMC_VDD_32_33,
	.caps = MMC_CAP_4_BIT_DATA,
	.min_clk = 400000,
	.max_clk = 52000000,
	.card_fixed = 1,
	.card_inserted_state = 1,
	.status = sdhc_get_card_det_status,
/*
** correct clock name error
*/
#if 1
  	.clock_mmc = "esdhc_clk",
#else //freescale 
//	.clock_mmc = "esdhc2_clk",
#endif
#ifdef CONFIG_TIWLAN_SUPPORT
        .embedded_sdio = NULL,
#endif
/* ----END */
};

/*!
 * Resource definition for the SDHC2
 */
static struct resource mxcsdhc2_resources[] = {
	[0] = {
	       .start = MMC_SDHC2_BASE_ADDR,
	       .end = MMC_SDHC2_BASE_ADDR + SZ_4K - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = MXC_INT_SDHC2,
	       .end = MXC_INT_SDHC2,
	       .flags = IORESOURCE_IRQ,
	       },
/*
** add detect pin
*/
	[2] = {
	       .start = IOMUX_TO_IRQ(MX25_PIN_ECB),
	       .end = IOMUX_TO_IRQ(MX25_PIN_ECB),
	       .flags = IORESOURCE_IRQ,
	       },	       
/* ----END */

};

/*! Device Definition for MXC SDHC2 */
static struct platform_device mxcsdhc2_device = {
	.name = "mxsdhci",
	.id = 1,
	.dev = {
		.release = mxc_nop_release,
		.platform_data = &mmc2_data,
		},
	.num_resources = ARRAY_SIZE(mxcsdhc2_resources),
	.resource = mxcsdhc2_resources,
};
#endif

static inline void mxc_init_mmc(void)
{
	(void)platform_device_register(&mxcsdhc1_device);
#ifdef CONFIG_MMC_IMX_ESDHCI_SELECT2
	(void)platform_device_register(&mxcsdhc2_device);
#endif
}
#else
static inline void mxc_init_mmc(void)
{
}
#endif

static void __init mx25_3stack_timer_init(void)
{
	mx25_clocks_init(24000000);
}

static struct sys_timer mxc_timer = {
	.init = mx25_3stack_timer_init,
};

#if defined(CONFIG_CAN_FLEXCAN) || defined(CONFIG_CAN_FLEXCAN_MODULE)
static void flexcan_xcvr_enable(int id, int en)
{
	static int pwdn;

	if (id != 1)		/* MX25 3-stack uses only CAN2 */
		return;

	if (en) {
		if (!pwdn++)
			gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D14), 0);
	} else {
		if (!--pwdn)
			gpio_set_value(IOMUX_TO_GPIO(MX25_PIN_D14), 1);
	}
}

struct flexcan_platform_data flexcan_data[] = {
	{
	 .core_reg = NULL,
	 .io_reg = NULL,
	 .xcvr_enable = flexcan_xcvr_enable,
	 .active = gpio_can_active,
	 .inactive = gpio_can_inactive,},
	{
	 .core_reg = NULL,
	 .io_reg = NULL,
	 .xcvr_enable = flexcan_xcvr_enable,
	 .active = gpio_can_active,
	 .inactive = gpio_can_inactive,},
};
#endif

/*!
 * Board specific fixup function. It is called by \b setup_arch() in
 * setup.c file very early on during kernel starts. It allows the user to
 * statically fill in the proper values for the passed-in parameters. None of
 * the parameters is used currently.
 *
 * @param  desc         pointer to \b struct \b machine_desc
 * @param  tags         pointer to \b struct \b tag
 * @param  cmdline      pointer to the command line
 * @param  mi           pointer to \b struct \b meminfo
 */
static void __init fixup_mxc_board(struct machine_desc *desc, struct tag *tags,
				   char **cmdline, struct meminfo *mi)
{
	mxc_cpu_init();

#ifdef CONFIG_DISCONTIGMEM
	do {
		int nid;
		mi->nr_banks = MXC_NUMNODES;
		for (nid = 0; nid < mi->nr_banks; nid++)
			SET_NODE(mi, nid);
	} while (0);
#endif
}

/*!
 * Board specific initialization.
 */
static void __init mxc_board_init(void)
{
	pr_info("AIPS1 VA base: 0x%p\n", IO_ADDRESS(AIPS1_BASE_ADDR));
	mxc_cpu_common_init();
	mxc_register_gpios();
	mx25_3stack_gpio_init();
	early_console_setup(saved_command_line);
#ifdef CONFIG_I2C
	i2c_register_board_info(0, mxc_i2c_board_info,
				ARRAY_SIZE(mxc_i2c_board_info));
#endif
	spi_register_board_info(mxc_spi_board_info,
				ARRAY_SIZE(mxc_spi_board_info));
/*
** mark redundant functions and pmic initial function
*/

	mx25_3stack_init_mc34704();
#if 1
	mxc_init_keypad();
#endif
#if 1
	mxc_init_wlan_sleep_clk();
#endif
	mxc_init_fb();
	mxc_init_bl();
	mxc_init_nand_mtd();
//	mxc_init_sgtl5000();  //audio codec
//	mxc_init_ak5702();   //microphone 
	mxc_init_mmc();
//	mxc_init_sim();
//	mxc_init_fec();
#if 1
	//pr_info("******** pr30_init_led...\n");
	pr30_init_led();
#endif
/* ----END */

#if 1
	mxc_init_otg_switch();
#endif
}

/*
 * The following uses standard kernel macros define in arch.h in order to
 * initialize __mach_desc_MX25_3DS data structure.
 */
/* *INDENT-OFF* */
MACHINE_START(MX25_3DS, "Freescale MX25 3-Stack Board")
	/* Maintainer: Freescale Semiconductor, Inc. */
	.phys_io = AIPS1_BASE_ADDR,
	.io_pg_offst = ((AIPS1_BASE_ADDR_VIRT) >> 18) & 0xfffc,
	.boot_params = PHYS_OFFSET + 0x100,
	.fixup = fixup_mxc_board,
	.map_io = mx25_map_io,
	.init_irq = mxc_init_irq,
	.init_machine = mxc_board_init,
	.timer = &mxc_timer,
MACHINE_END
