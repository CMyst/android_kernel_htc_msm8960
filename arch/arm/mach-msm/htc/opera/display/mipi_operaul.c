#include <mach/panel_id.h>
#include "../../../drivers/video/msm/msm_fb.h"
#include "../../../drivers/video/msm/mipi_dsi.h"
#include <linux/leds.h>
#include <mach/board.h>
#include "mipi_operaul.h"

static struct mipi_dsi_panel_platform_data *mipi_operaul_pdata;

static struct dsi_cmd_desc *init_on_cmds = NULL;
static struct dsi_cmd_desc *display_on_cmds = NULL;
static struct dsi_cmd_desc *display_off_cmds = NULL;
static struct dsi_cmd_desc *backlight_cmds = NULL;
static int init_on_cmds_count = 0;
static int display_on_cmds_count = 0;
static int display_off_cmds_count = 0;
static int backlight_cmds_count = 0;
static int wled_trigger_initialized;
static atomic_t lcd_power_state;

static char enter_sleep[2] = {0x10, 0x00}; 
static char exit_sleep[2] = {0x11, 0x00}; 
static char display_off[2] = {0x28, 0x00}; 
static char display_on[2] = {0x29, 0x00}; 

static char led_pwm1[] = {0x51, 0xff}; 
static char led_pwm2[] = {0x53, 0x24}; 
static char pwm_off[]  = {0x51, 0x00};

static char himax_b9[] = {0xB9, 0xFF, 0x83, 0x92}; 
static char himax_d4[] = {0xD4, 0x00}; 
static char himax_ba[] = {0xBA, 0x12, 0x83, 0x00, 0xD6, 0xC6, 0x00, 0x0A}; 
static char himax_c0[] = {0xC0, 0x01, 0x94}; 
static char himax_c6[] = {0xC6, 0x35, 0x00, 0x00, 0x04}; 
static char himax_d5[] = {0xD5, 0x00, 0x00, 0x02}; 
static char himax_bf[] = {0xBF, 0x05, 0x60, 0x02}; 
static char himax_b2[] = {0xB2, 0x0F, 0xC8, 0x04, 0x0C, 0x04}; 
static char himax_35[] = {0x35, 0x00}; 
static char himax_c2[] = {0xC2, 0x08}; 
static char himax_36[] = {0x36, 0x03};
static char himax_55[] = {0x55, 0x03};
static char cabc_UI[] = {
	0xCA, 0x2D, 0x27, 0x26,
	0x25, 0x25, 0x25, 0x21,
	0x20, 0x20};
#ifdef CONFIG_MSM_CABC_VIDEO_ENHANCE
static char cabc_moving[] = {
	0xCA, 0x40, 0x3C, 0x38,
	0x34, 0x33, 0x32, 0x2D,
	0x24, 0x20};
#endif
static char himax_e3[] = {0xE3, 0x01};
static char himax_e5[] = {
	0xE5, 0x00, 0x04, 0x0B,
	0x05, 0x05, 0x00, 0x80,
	0x20, 0x80, 0x10, 0x00,
	0x07, 0x07, 0x07, 0x07,
	0x07, 0x80, 0x0A};
static char himax_c9[] = {0xC9, 0x1F, 0x00, 0x1E, 0x3F, 0x00, 0x80}; 

static char nt_disp_mode[] = {0xC2, 0x08};
static char nt_enable_TE[] = {0x35, 0x00};
static char nt_page_EE[] = {0xFF, 0xEE};
static char nt_random_dot_12[] = {0x12, 0x50};
static char nt_random_dot_13[] = {0x13, 0x02};
static char nt_random_dot_6A[] = {0x6A, 0x60};
static char nt_page_0[] = {0xFF, 0x00};
static char nt_invert[] = {0x36, 0xD4};
static char nt_mipi_lane[] = {0xBA, 0x02};

static struct dsi_cmd_desc sharp_hx_cmd_on_rotation_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_b9), himax_b9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_d4), himax_d4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_ba), himax_ba},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c0), himax_c0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c6), himax_c6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_d5), himax_d5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_bf), himax_bf},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_b2), himax_b2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_e3), himax_e3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_e5), himax_e5},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_35), himax_35},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_c2), himax_c2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_36), himax_36},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(led_pwm2), led_pwm2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_55), himax_55},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(cabc_UI), cabc_UI},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c9), himax_c9},
};

static struct dsi_cmd_desc sharp_hx_cmd_on_cmds[] = {
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_b9), himax_b9},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_d4), himax_d4},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_ba), himax_ba},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c0), himax_c0},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c6), himax_c6},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_d5), himax_d5},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_bf), himax_bf},

	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_b2), himax_b2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_e3), himax_e3},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_e5), himax_e5},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_35), himax_35},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_c2), himax_c2},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(led_pwm2), led_pwm2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(himax_55), himax_55},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(cabc_UI), cabc_UI},
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(himax_c9), himax_c9},
};

static struct dsi_cmd_desc sharp_nt_cmd_on_rotation_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_disp_mode), nt_disp_mode},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_enable_TE), nt_enable_TE},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_page_EE), nt_page_EE},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_12), nt_random_dot_12},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_13), nt_random_dot_13},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_6A), nt_random_dot_6A},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_page_0), nt_page_0},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_invert), nt_invert},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(led_pwm2), led_pwm2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_mipi_lane), nt_mipi_lane},
};

static struct dsi_cmd_desc sharp_nt_cmd_on_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_disp_mode), nt_disp_mode},
	{DTYPE_DCS_WRITE, 1, 0, 0, 120, sizeof(exit_sleep), exit_sleep},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_enable_TE), nt_enable_TE},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_page_EE), nt_page_EE},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_12), nt_random_dot_12},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_13), nt_random_dot_13},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_random_dot_6A), nt_random_dot_6A},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_page_0), nt_page_0},

	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,  sizeof(led_pwm2), led_pwm2},
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,  sizeof(nt_mipi_lane), nt_mipi_lane},
};

static struct dsi_cmd_desc sharp_display_off_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 1,
		sizeof(pwm_off), pwm_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 1,
		sizeof(display_off), display_off},
	{DTYPE_DCS_WRITE, 1, 0, 0, 130,
		sizeof(enter_sleep), enter_sleep},
};
static struct dsi_cmd_desc sharp_display_on_cmds[] = {  
	{DTYPE_DCS_WRITE, 1, 0, 0, 40, sizeof(display_on), display_on},
};

static struct dsi_cmd_desc sharp_hx_cmd_backlight_cmds[] = {
	{DTYPE_DCS_WRITE1, 1, 0, 0, 0,
		sizeof(led_pwm1), led_pwm1},
};

#ifdef CONFIG_MSM_CABC_VIDEO_ENHANCE
static struct dsi_cmd_desc cabc_UI_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(cabc_UI), cabc_UI},
};
static struct dsi_cmd_desc cabc_moving_cmds[] = {
	{DTYPE_DCS_LWRITE, 1, 0, 0, 1,  sizeof(cabc_moving), cabc_moving},
};
#endif

static int operaul_send_display_cmds(struct dsi_cmd_desc *cmd, int cnt, bool clk_ctrl)
{
	int ret = 0;
	struct dcs_cmd_req cmdreq;

	cmdreq.cmds = cmd;
	cmdreq.cmds_cnt = cnt;
	cmdreq.flags = CMD_REQ_COMMIT;
	if(clk_ctrl)
		cmdreq.flags |= CMD_CLK_CTRL;
	cmdreq.rlen = 0;
	cmdreq.cb = NULL;

	ret = mipi_dsi_cmdlist_put(&cmdreq);
	if (ret < 0)
		pr_err("%s failed (%d)\n", __func__, ret);
	return ret;
}

static int operaul_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	struct mipi_panel_info *mipi;
	struct msm_panel_info *pinfo;

	mfd = platform_get_drvdata(pdev);
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	pinfo = &mfd->panel_info;
	mipi  = &mfd->panel_info.mipi;

	operaul_send_display_cmds(init_on_cmds, init_on_cmds_count, false);

	atomic_set(&lcd_power_state, 1);

	pr_debug("Display On \n");

	return 0;
}

static int operaul_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	return 0;
}

static int operaul_display_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	bool clk_ctrl;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	mipi_dsi_op_mode_config(DSI_CMD_MODE);

	clk_ctrl = (mfd->panel_info.type == MIPI_CMD_PANEL);
	operaul_send_display_cmds(display_on_cmds, display_on_cmds_count,clk_ctrl);

	pr_debug("%s\n", __func__);
	return 0;
}

DEFINE_LED_TRIGGER(bkl_led_trigger);

static int operaul_display_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	bool clk_ctrl;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	clk_ctrl = (mfd->panel_info.type == MIPI_CMD_PANEL);
	operaul_send_display_cmds(display_off_cmds, display_off_cmds_count,clk_ctrl);

	if (wled_trigger_initialized)
		led_trigger_event(bkl_led_trigger, 0);

	else
		pr_err("%s: wled trigger is not initialized!\n", __func__);

	atomic_set(&lcd_power_state, 0);

	pr_debug("%s\n", __func__);

	return 0;
}

#ifdef CONFIG_MSM_CABC_VIDEO_ENHANCE
static void operaul_set_cabc(struct msm_fb_data_type *mfd, int mode)
{
	struct msm_fb_data_type *mfd;
	struct dsi_cmd_desc *cabc_cmds;
	int cabc_cmds_size = 0;

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	pr_debug("%s: mode=%d\n",  __FUNCTION__, mode);

	if (mode == 0) {
		cabc_cmds = cabc_UI_cmds;
		cabc_cmds_size = ARRAY_SIZE(cabc_UI_cmds);
	} else {
		cabc_cmds = cabc_moving_cmds;
		cabc_cmds_size = ARRAY_SIZE(cabc_moving_cmds);
	}
	operaul_send_display_cmds(cabc_cmds, cabc_cmds_size);
}
#endif

static unsigned char operaul_shrink_pwm(int val)
{
	unsigned int pwm_min, pwm_default, pwm_max;
	unsigned char shrink_br = BRI_SETTING_MAX;

	pwm_min = 12;
	pwm_default = 82;
	pwm_max = 255;


	if (val <= 0) {
		shrink_br = 0;
	} else if (val > 0 && (val < BRI_SETTING_MIN)) {
			shrink_br = pwm_min;
	} else if ((val >= BRI_SETTING_MIN) && (val <= BRI_SETTING_DEF)) {
			shrink_br = (val - BRI_SETTING_MIN) * (pwm_default - pwm_min) /
		(BRI_SETTING_DEF - BRI_SETTING_MIN) + pwm_min;
	} else if (val > BRI_SETTING_DEF && val <= BRI_SETTING_MAX) {
			shrink_br = (val - BRI_SETTING_DEF) * (pwm_max - pwm_default) /
		(BRI_SETTING_MAX - BRI_SETTING_DEF) + pwm_default;
	} else if (val > BRI_SETTING_MAX)
			shrink_br = pwm_max;

	pr_debug("brightness orig=%d, transformed=%d\n", val, shrink_br);

	return shrink_br;
}

static void operaul_set_backlight(struct msm_fb_data_type *mfd)
{
	struct mipi_panel_info *mipi;
	bool clk_ctrl = (mfd && mfd->panel_info.type == MIPI_CMD_PANEL);

	led_pwm1[1] = operaul_shrink_pwm((unsigned char)(mfd->bl_level));

	if (mipi_operaul_pdata && (mipi_operaul_pdata->enable_wled_bl_ctrl)
	    && (wled_trigger_initialized)) {
		led_trigger_event(bkl_led_trigger, led_pwm1[1]);
		return;
	}
	mipi  = &mfd->panel_info.mipi;
	pr_debug("%s+:bl=%d \n", __func__, mfd->bl_level);

	if (atomic_read(&lcd_power_state) == 0) {
		pr_debug("%s: LCD is off. Skip backlight setting\n", __func__);
		return;
	}

	if (mipi->mode == DSI_VIDEO_MODE) {
		return;
	}
	

	if (mipi->mode == DSI_CMD_MODE) {
		mipi_dsi_op_mode_config(DSI_CMD_MODE);
	}

	operaul_send_display_cmds(backlight_cmds, backlight_cmds_count,clk_ctrl);

#ifdef CONFIG_BACKLIGHT_WLED_CABC
	if (wled_trigger_initialized) {
		led_trigger_event(bkl_led_trigger, mfd->bl_level);
	}
#endif
	return;
}

static int __devinit mipi_operaul_lcd_probe(struct platform_device *pdev)
{
	if (panel_type == PANEL_ID_OPA_SHARP_NT_ROTA) {
		init_on_cmds = sharp_nt_cmd_on_rotation_cmds;
		init_on_cmds_count = ARRAY_SIZE(sharp_nt_cmd_on_rotation_cmds);
	} else if (panel_type == PANEL_ID_OPA_SHARP_NT){
		init_on_cmds = sharp_nt_cmd_on_cmds;
		init_on_cmds_count = ARRAY_SIZE(sharp_nt_cmd_on_cmds);
	} else if(panel_type == PANEL_ID_OPA_SHARP_HX_ROTA) {
		init_on_cmds = sharp_hx_cmd_on_rotation_cmds;
		init_on_cmds_count = ARRAY_SIZE(sharp_hx_cmd_on_rotation_cmds);
	} else {
		init_on_cmds = sharp_hx_cmd_on_cmds;
		init_on_cmds_count = ARRAY_SIZE(sharp_hx_cmd_on_cmds);
	}
	display_on_cmds = sharp_display_on_cmds;
	display_on_cmds_count = ARRAY_SIZE(sharp_display_on_cmds);
	display_off_cmds = sharp_display_off_cmds;
	display_off_cmds_count = ARRAY_SIZE(sharp_display_off_cmds);
	backlight_cmds = sharp_hx_cmd_backlight_cmds;
	backlight_cmds_count = ARRAY_SIZE(sharp_hx_cmd_backlight_cmds);

	if (pdev->id == 0)
		mipi_operaul_pdata = pdev->dev.platform_data;
	
	msm_fb_add_device(pdev);

	return 0;
}

static struct platform_driver this_driver = {
	.probe  = mipi_operaul_lcd_probe,
	.driver = {
		.name   = "mipi_operaul",
	},
};

static struct msm_fb_panel_data operaul_panel_data = {
	.on		= operaul_lcd_on,
	.off		= operaul_lcd_off,
	.set_backlight  = operaul_set_backlight,
	.late_init = operaul_display_on,
	.early_off = operaul_display_off,
#ifdef CONFIG_MSM_CABC_VIDEO_ENHANCE
	.set_cabc	= operaul_set_cabc,
#endif
};

static int ch_used[3];

int mipi_operaul_device_register(struct msm_panel_info *pinfo,
					u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	led_trigger_register_simple("bkl_trigger", &bkl_led_trigger);
	pr_info("%s: SUCCESS (WLED TRIGGER)\n", __func__);
	wled_trigger_initialized = 1;
	atomic_set(&lcd_power_state, 1);

	ret = platform_driver_register(&this_driver);
	if (ret) {
		pr_err("platform_driver_register() failed with ret %u\n", ret);
		return ret;
	}

	pdev = platform_device_alloc("mipi_operaul", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	operaul_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &operaul_panel_data,
		sizeof(operaul_panel_data));
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		printk(KERN_ERR
		  "%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}

	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}
