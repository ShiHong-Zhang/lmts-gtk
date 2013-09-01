#include <stdio.h>
#include "../brace.h"
#include "../types.h"
#include "items.h"

void RegisterAllFunc()
{
	F_audio_register();
	F_audio_crs_zb_register();
	F_battery_register();
	F_burnmac_register();
	F_ckec_register();
	F_ckntp_register();
	F_ckpmon_register();
	F_ckrom_register();
	F_cmos_register();
	F_cpu_memcap_register();
	F_fanstem_register();
	F_keyboard_register();
	F_lan_register();
	F_lcdwave_register();
	F_ntp_register();
	F_scanmbsn_register();
	F_scanpsn_register();
	F_sdcard_register();
	F_touchpad_register();
	F_upload_register();
	F_usb_register();
	F_vgaout_register();
	F_webcam_register();
	F_wifi_register();
	F_idcard_register();
	F_sata_register();
	F_scrn_prcn_register();
	F_ledlight_register();

	// runin
	F_cpu_runin_register();
	F_hd_runin_register();
	F_videoram_runin_register();
	F_control_runin_register();
}
