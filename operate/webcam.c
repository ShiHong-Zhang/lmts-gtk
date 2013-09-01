#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <gtk/gtk.h>
#include <linux/videodev.h>
#include <sys/mman.h>
#include <string.h>

#include "color.h"
#include "items.h"
#include "../ui.h"
#include "../brace.h"
#include "../types.h"



#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef struct _buffer {
	void * start;
	size_t length;
} buffer;


char dev_name[128] = "/dev/video0";
int cam_result = 0;

//int width = 320;
//int height = 240;	// higher resolution ratio, more clear
int width = 160;
int height = 120;

int preview_width = 533;
int preview_height = 400;
int FPS = 15;

buffer *buffers = NULL;

int fd = -1;
int reqbuf_count = 4;

GtkWidget *layout = NULL;
GtkWidget *cam_window = NULL;
GtkWidget *image_face = NULL;
GtkWidget *ok_btn = NULL, *ng_btn = NULL;
static GdkPixmap *pixmap = NULL;

unsigned char framebuffer[2048 * 1536 * 3];

guint timeId = 0;


void open_device();
void init_device();
void set_format();
void request_buffer();
void query_buf_and_mmap();
void queue_buffer();
void stream_on();
void read_frame();
static gboolean show_camera(gpointer data);
void stream_off();
void mem_unmap_and_close_dev();
int xioctl(int fd, int request, void* arg);
static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data);
static void ok_btn_cb(GtkWidget *widget, gpointer data);
static void ng_btn_cb(GtkWidget *widget, gpointer data);


int camera_work()
{

	cam_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width (GTK_CONTAINER (cam_window), 0);
	gtk_window_set_title (GTK_WINDOW (cam_window), "Camera");
	gtk_window_set_position(GTK_WINDOW(cam_window), GTK_WIN_POS_CENTER);
	gtk_widget_set_size_request(cam_window, preview_width, preview_height + 40 + 30);
	g_signal_connect (G_OBJECT (cam_window), "delete_event",
				G_CALLBACK (delete_event), NULL);

	layout = gtk_fixed_new ();
	gtk_widget_set_size_request(layout, preview_width, preview_height + 40 + 30);
	gtk_container_add(GTK_CONTAINER(cam_window), layout);

	image_face = gtk_drawing_area_new ();
	gtk_widget_set_size_request(image_face, preview_width, preview_height);
	gtk_fixed_put(GTK_FIXED(layout), image_face, 0, 0);

	ok_btn = gtk_button_new_with_label("OK");
	gtk_widget_set_size_request(ok_btn, 50, 30);
	gtk_fixed_put(GTK_FIXED(layout), ok_btn, preview_width - 20 - 20 - 50 * 2, preview_height + 20);
	g_signal_connect (G_OBJECT (ok_btn), "clicked",
				G_CALLBACK (ok_btn_cb), cam_window);

	ng_btn = gtk_button_new_with_label("NG");
	gtk_widget_set_size_request(ng_btn, 50, 30);
	gtk_fixed_put(GTK_FIXED(layout), ng_btn, preview_width - 50 - 20, preview_height + 20);
	g_signal_connect (G_OBJECT (ng_btn), "clicked",
				G_CALLBACK (ng_btn_cb), cam_window);

	gtk_widget_show_all(cam_window);

	open_device();
	init_device();
	stream_on();
	// defined in color.h, and used to pixel format changing
	initLut();

	timeId = g_timeout_add(1000/FPS, show_camera, NULL);
	gtk_main ();

	freeLut();
	stream_off();
	mem_unmap_and_close_dev();

	return 0;
}

void open_device()
{
	fd = open (dev_name, O_RDWR|O_NONBLOCK, 0);
	if(-1 == fd) {
		/* Error handler */
		fprintf(stderr, "open device error\n");
	}
}

void init_device()
{
	set_format();
	request_buffer();
	query_buf_and_mmap();
	queue_buffer();
}

void set_format()
{
	struct v4l2_format fmt;
	CLEAR (fmt);

	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

	if(-1 == xioctl (fd, VIDIOC_S_FMT, &fmt)) {
		fprintf(stderr, "set format error\n");
		// Error handler
	}
}

void request_buffer()
{
	struct v4l2_requestbuffers req;
	CLEAR (req);

	req.count               = reqbuf_count;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;
	if(-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
		fprintf(stderr, "request buf error\n");
		// Error handler
	}
}

void query_buf_and_mmap()
{
	buffers = (buffer*)calloc(reqbuf_count, sizeof(*buffers));
	if(!buffers) {
		// Error handler
	}

	int i = 0;
	struct v4l2_buffer buf;

	for(i = 0; i < reqbuf_count; ++i) {
		CLEAR (buf);

		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = i;

		if(-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf)) {
			fprintf(stderr, "query buf error\n");
			// Error handler
		}
		DeBug(printf("buffer length: %d\nbuffer offset: %d\n", buf.length, buf.m.offset))

		buffers[i].length = buf.length;
		buffers[i].start = mmap(NULL,
								buf.length,
								PROT_READ|PROT_WRITE,
								MAP_SHARED,
								fd,
								buf.m.offset);

		if(MAP_FAILED == buffers[i].start) {
		// Error handler
		}
	}
}

void queue_buffer()
{
	struct v4l2_buffer buf;
	int i = 0;

	for(i = 0; i <  reqbuf_count; ++i) {
		CLEAR (buf);
		buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index  = i;

		if(-1 == xioctl(fd, VIDIOC_QBUF, &buf)) {
		// Error handler
		}
	}
}

void stream_on()
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == xioctl(fd, VIDIOC_STREAMON, &type)) {
		// Error handler
	}
}

void read_frame()
{
	struct v4l2_buffer buf;
//	unsigned int i;

	CLEAR (buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	if(-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
		// Error handler
	}
//	assert (buf.index  n_buffers0);
	memcpy(framebuffer, buffers[buf.index].start, buf.bytesused);
	if(-1 == xioctl (fd, VIDIOC_QBUF, &buf)) {
		// Error handler
	}
}

static gboolean show_camera(gpointer data)
{
	read_frame();

	if(pixmap) {
		g_object_unref(pixmap); // ref count minus one
	}

	pixmap = gdk_pixmap_new (image_face->window, preview_width, preview_height, -1);

	unsigned char *buf2 = (unsigned char *) malloc (sizeof(unsigned char) * width * height * 3);

	Pyuv422torgb24((unsigned char*)framebuffer, buf2, width, height);
	GdkPixbuf *rgbBuf = gdk_pixbuf_new_from_data(buf2,
												GDK_COLORSPACE_RGB,
												FALSE, 8,
												width, height,
												width * 3,
												NULL, NULL);

	if(rgbBuf != NULL) {
		GdkPixbuf* buf = gdk_pixbuf_scale_simple(rgbBuf,
												preview_width,
												preview_height,
												GDK_INTERP_BILINEAR);

		gdk_draw_pixbuf(pixmap,
						image_face->style->white_gc,
						buf,
						0, 0, 0, 0,
						preview_width, preview_height,
						GDK_RGB_DITHER_NONE, 0, 0);

		gdk_draw_drawable(image_face->window,
						image_face->style->white_gc,
						pixmap,
						0, 0, 0, 0,
						preview_width, preview_height);

		g_object_unref(buf);
		g_object_unref(rgbBuf);
	}

	gtk_widget_show(image_face);

	free(buf2);
	return TRUE;
}

void stream_off(void)
{
	int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(-1 == xioctl(fd, VIDIOC_STREAMOFF, &type)) {
		// Error handler
	}
}

void mem_unmap_and_close_dev()
{
	int i = 0;
	for(i = 0; i < reqbuf_count; ++i) {
		if(-1 == munmap(buffers[i].start, buffers[i].length)){
			// Error hanlder
		}
	}

	free(buffers);
	close(fd);
}

int xioctl(int fd, int request, void* arg)
{
	int r;
	do r = ioctl (fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}

static gboolean delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	g_source_remove(timeId);
	gtk_main_quit ();
	return FALSE;
}

static void ok_btn_cb(GtkWidget *widget, gpointer data)
{
	GtkWidget *window = (GtkWidget *)data;

	cam_result = 0;
	g_source_remove(timeId);
	gtk_widget_destroy(window);
	gtk_main_quit ();

	return;
}

static void ng_btn_cb(GtkWidget *widget, gpointer data)
{
	GtkWidget *window = (GtkWidget *)data;

	cam_result = 1;
	g_source_remove(timeId);
	gtk_widget_destroy(window);
	gtk_main_quit ();
	return;
}

/* *************************************************** */

int webcam_run(char *arg)
{
	printPrompt("进入摄像头测试\n请确认开启摄像头（Fn + F10）\n并根据实际情况选择'OK'或者'NG'\n");

	gdk_threads_enter();
	camera_work();
	gdk_threads_leave();

	if (cam_result) {
		printNG("摄像头测试失败\n");
	} else {
		printOK("摄像头测试成功\n");
	}

	return cam_result;
}

int F_webcam_register()
{

	F_Test_Run run = webcam_run;
	RegisterTestFunc("webcam", run);

	return 0;
}

