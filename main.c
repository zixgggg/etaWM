#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
/*
header at  /usr/include/X11/Xlib.h
install libx11-doc can see man page
API reference:https://tronche.com/gui/x/xlib/function-index.html
*/
int main(){
	Display * dpy;
	dpy=XOpenDisplay(NULL);
	XEvent ev;
	Cursor cursor=XCreateFontCursor(dpy,XC_left_ptr);
	KeyCode q_code=XKeysymToKeycode(dpy, XStringToKeysym("q"));
	
	XGrabKey(dpy,q_code,Mod1Mask,DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
    XGrabButton(dpy, 1, Mod4Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod4Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    
	XDefineCursor(dpy,DefaultRootWindow(dpy),cursor);
	while(True){
		XNextEvent(dpy,&ev);
		if(ev.type==KeyPress){
			/*
			if(ev.xkey==q_code){
				XInternAtom(dpy,"WM_DELETE_WINDOW",False);
				
			}
			*/
			if(ev.xkey.keycode==q_code && ev.xkey.subwindow!=None){
				XKillClient(dpy,ev.xkey.subwindow);
			}
		}
		/*
		else if(ev.type==ButtonPress){
			int mc_origin_x=ev.xbutton.x_root;
			int mc_origin_y=ev.xbutton.y_root;
			if(ev.xbutton.button==1){
				int win_origin_x=ev.xbutton.subwindow.XGetGeometry.x_return;
			}
			
		}
		*/
		else if(ev.type==EnterNotify && ev.xbutton.subwindow!=None){
			XRaiseWindow(dpy,ev.xbutton.subwindow);
		}
	}
	return 0;
}
