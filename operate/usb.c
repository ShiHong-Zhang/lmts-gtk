#include <gtk/gtk.h>
#include <stdio.h>
#include <usb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <libusb-1.0/libusb.h>

#include "items.h"
#include "../ui.h"
#include "../brace.h"
#include "../types.h"

#define TIME_OUT		5000

//#define USB_SPEC			3	// 3ANB: 1,camera; 2,tf card...
//#define USB_3ANB_HAD		2	// 3ANB: 1,camera; 2,tf card...

#define usb_err(x)	\
	do { if ((x) < 0) { fprintf(stderr, "operate error, line %d\n", __LINE__); continue;}}while(0)

int usb_run(char *arg)
{
	extern lua_State *L;

	struct usb_bus *bus;
	struct usb_device *dev;
	usb_dev_handle *udev;
	struct usb_device_descriptor *desc;
	char buf[16];
	int ret = 0, i = 0, bNbrPorts = 0;
	int count = 0;

	int usb_spec = 0, usb_had = 0;

	usb_spec = getTableNumElement(L, "con", "USB_SPEC");
	usb_had = getTableNumElement(L, "con", "USB_HAD");

	printPrompt("开始USB测试。\n");
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
		
	/* loop all devices */
	/* every bus */
	int bus_count = 1;

	for (bus = usb_busses; bus; bus = bus->next) {
		/* every controller */
		DeBug(printf("bus count: %d\n", bus_count))
		bus_count += 1;

		for (dev = bus->devices; dev; dev = dev->next) {
			desc = &(dev->descriptor);
			if (desc->bDeviceClass == USB_CLASS_HUB) {
				udev = usb_open(dev);
				DeBug(printf("hub :vendor/product ID: %04x:%04x",
					desc->idVendor, desc->idProduct))
				ret = usb_control_msg(udev, 0xa0,
							USB_REQ_GET_DESCRIPTOR,
							USB_DT_HUB << 8,
							0,
							buf, sizeof(buf), TIME_OUT);
				usb_err(ret);

				bNbrPorts = buf[2];
				DeBug(printf(" have %d ports\n",bNbrPorts))

				for (i = 1; i <= bNbrPorts; ++i) {
					ret = usb_control_msg(udev, 0xa3,
								USB_REQ_GET_STATUS,
								0,
								i,
								buf, 4, TIME_OUT);
					usb_err(ret);
					if ((buf[0] & 0x1)) { // if have not device.
					/* if this port have a device, don't set it */
						DeBug(printf("have %04x\n", buf[0]))
						count += 1;
					}
				}
			}
		}
	}
	DeBug(printf("usb device count: %d\n", count))


	int usb_port_count = count - usb_had;

	gchar *text_report = g_strdup_printf("已连接USB外围设备数量：%d个\n", usb_port_count);
	printMsg(g_string_chunk_insert_const(text_chunk, text_report));
	g_free(text_report);
	
	if (usb_port_count != usb_spec) {
		printPrompt("USB外围设备连接数量不够！\n");
		return 1;
	}

#if 0
	if (count != usb_spec) {
		printPrompt("USB外围设备连接数量不够！\n");
		return 1;
	}
#endif

	return 0;

}

int F_usb_register()
{

	F_Test_Run run = usb_run;
	RegisterTestFunc("usb", run);

	return 0;
}
