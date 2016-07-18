#ifndef MIPI_OPERAUL_H
#define MIPI_OPERAUL_H

int mipi_operaul_device_register(struct msm_panel_info *pinfo,
        u32 channel, u32 panel);

#define BRI_SETTING_MIN                 30
#define BRI_SETTING_DEF                 143
#define BRI_SETTING_MAX                 255

#endif
