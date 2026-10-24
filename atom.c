//#include <X11/Xlib.h>
//#include <X11/Xatom.h>
//every atoms declare at /usr/include/X11/Xatom.h
#include "atom.h"
Atom net_active_window;
Atom wm_delete_window;
Atom wm_protocols;
Atom net_client_list;
Atom net_supporting_wm_check;
Atom net_wm_name;
Atom utf8_string;
Atom net_close_window;
Atom net_supported;
void init_atoms(Display * dpy){
	net_active_window		= XInternAtom(dpy, "_NET_ACTIVE_WINDOW",False);
	wm_delete_window		= XInternAtom(dpy, "WM_DELETE_WINDOW", False);//WM_DELETE_WINDOW是ICCCM規定 請程式自己優雅關閉
	wm_protocols			= XInternAtom(dpy, "WM_PROTOCOLS", False);
	net_client_list		 	= XInternAtom(dpy, "_NET_CLIENT_LIST",False);
	net_supporting_wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
	net_wm_name             = XInternAtom(dpy, "_NET_WM_NAME", False);
	utf8_string             = XInternAtom(dpy, "UTF8_STRING", False);
	net_close_window		= XInternAtom(dpy, "_NET_CLOSE_WINDOW",False);//_NET_CLOSE_WINDOW是EWMH規定 請WM去關某個視窗
	net_supported			= XInternAtom(dpy, "_NET_SUPPORTED",False);
	Atom net_supported_list[]={
		net_active_window,
		net_client_list,
		net_supporting_wm_check,
		net_wm_name,
		net_close_window,
		net_supported
	};
	int net_supported_list_size=sizeof(net_supported_list)/sizeof(net_supported_list[0]);
	XChangeProperty(dpy,DefaultRootWindow(dpy),net_supported,XA_ATOM,32,PropModeReplace,(unsigned char *)net_supported_list,net_supported_list_size);
}
