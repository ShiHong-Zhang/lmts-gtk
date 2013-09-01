#ifndef __ITEMS_H__
#define __ITEMS_H__

int F_audio_register();
int F_audio_crs_zb_register();
int F_battery_register();
int F_burnmac_register();
int F_ckec_register();
int F_ckpmon_register();
int F_ckrom_register();
int F_cmos_register();
int F_cpu_memcap_register();
int F_fanstem_register();
int F_keyboard_register();
int F_lan_register();
int F_lcdwave_register();
int F_ntp_register();
int F_ckntp_register();
int F_scanmbsn_register();
int F_scanpsn_register();
int F_sdcard_register();
int F_touchpad_register();
int F_upload_register();
int F_usb_register();
int F_vgaout_register();
int F_webcam_register();
int F_wireless_register();
int F_wifi_register();
int F_idcard_register();
int F_sata_register();
int F_scrn_prcn_register();
int F_ledlight_register();

// runin
int F_cpu_runin_register();
int F_hd_runin_register();
int F_videoram_runin_register();
int F_control_runin_register();

void RegisterAllFunc();

#endif

