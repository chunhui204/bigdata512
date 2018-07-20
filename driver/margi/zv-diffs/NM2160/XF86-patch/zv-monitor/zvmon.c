#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#define GUID_YUV12_PLANAR 0x32315659

int main (int argc, char* argv[])
{
  Display *dpy;
  int screen;
  XVisualInfo vinfo;
  XSetWindowAttributes xswa;
  unsigned long mask;
  unsigned int p_version, p_release, p_request_base, p_event_base, p_error_base;
  int p_num_adaptors;
  XvAdaptorInfo *ai;
  int xv_port = -1;
  int ret, x, y, w, h, b, d;
  Window window, root;
  GC gc;
  XEvent event;

  if ((dpy = XOpenDisplay(NULL)) == NULL){
    fprintf(stderr, "Cannot open Display.\n");
    exit(-1);
  }
  screen = DefaultScreen(dpy);
  if (!XMatchVisualInfo(dpy, screen, 16, TrueColor, &vinfo)
     && !XMatchVisualInfo(dpy, screen, 15, TrueColor, &vinfo)){
    fprintf(stderr, "Cannot find the visual (15bpp or 16bpp).\n");
    exit(-1);
  }
  
  xswa.colormap =  XCreateColormap(dpy, DefaultRootWindow(dpy), vinfo.visual, AllocNone);
  xswa.event_mask = StructureNotifyMask | ExposureMask;
  xswa.background_pixel = 0;
  xswa.border_pixel = 0;
  mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
  
  window = XCreateWindow(dpy, DefaultRootWindow(dpy),
			 0, 0, 640, 480, 0, vinfo.depth,
			 InputOutput, vinfo.visual, mask, &xswa);
  gc = XCreateGC(dpy, window, 0, 0);
  XStoreName(dpy, window, "ZV");
  XSetIconName(dpy, window, "ZV");
  XSelectInput(dpy, window, StructureNotifyMask);
  XMapWindow(dpy, window);
  XSync(dpy, 1);
  
  if ((ret = XvQueryExtension(dpy, &p_version, &p_release, &p_request_base, &p_event_base, &p_error_base)) != Success){
      fprintf(stderr, "Cannot find XV Extention.\n");
      exit(-1);
  }
  if ((ret = XvQueryAdaptors(dpy, DefaultRootWindow(dpy), &p_num_adaptors, &ai)) != Success){
      fprintf(stderr, "Cannot find XV adaptor.\n");
      exit(-1);
  }
  if (p_num_adaptors == 0){
      fprintf(stderr, "Cannot find XV adaptor.\n");
      exit(-1);
  }
  xv_port = ai[0].base_id;
    
  XGetGeometry(dpy, window, &root, &x, &y, &w, &h, &b, &d);
  if (XvPutVideo(dpy, xv_port, window, gc, 0, 0, 720, 480, x, y, w, h) != Success){
    printf("XvPutVideo failed.\n");
    exit(-1);
  }
  XFlush(dpy);

  getchar();
}
