#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/cursorfont.h>
/*
header at  /usr/include/X11/Xlib.h
install libx11-doc can see man page
API reference:https://tronche.com/gui/x/xlib/function-index.html
*/

#define MAX(a, b) ((a) > (b) ? (a) : (b))
int main(){
	Display * dpy;
	dpy=XOpenDisplay(NULL);
	XEvent ev;
	Cursor cursor=XCreateFontCursor(dpy,XC_left_ptr);
	KeyCode q_code=XKeysymToKeycode(dpy, XStringToKeysym("q"));
	
	XGrabKey(dpy,q_code,Mod4Mask,DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
    XGrabButton(dpy, 1, Mod4Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod4Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    
	XDefineCursor(dpy,DefaultRootWindow(dpy),cursor);
	
	int mc_origin_x;
	int mc_origin_y;
	int win_origin_x;
	int win_origin_y;
	int win_origin_width;
	int win_origin_height;
	Window root;
	int border,depth;
	XButtonEvent start;
	start.subwindow=None;
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
		else if(ev.type==ButtonPress && ev.xbutton.subwindow!=None){
			start=ev.xbutton;
			XGetGeometry(dpy,ev.xbutton.subwindow,&root,&win_origin_x,&win_origin_y,&win_origin_width,&win_origin_height,&border,&depth);
			/*
			int mc_origin_x=ev.xbutton.x_root;
			int mc_origin_y=ev.xbutton.y_root;
			if(start.button==1){
			}
			else if(start.button==3){
				win_origin_width=ev.xbutton.subwindow.XGetGeometry.width_return;
				win_origin_height=ev.xbutton.subwindow.XGetGeometry.height_return;
			}
			*/
		}
		else if(ev.type==MotionNotify && start!=None){
			int moved_x=ev.xbutton.x_root;
			int moved_y=ev.xbutton.y_root;

			int move_x=moved_x-mc_origin_x;
			int move_y=moved_y-mc_origin_y;

			if(start.button==1){
				XMoveWindow(dpy,start.subwindow,win_origin_x+move_x,win_origin_y+move_y);
			}
			else if(start.subwindow==3){
				const int MIN_WIN_WIDTH=50;
				const int MIN_WIN_HEIGHT=50;
				
				XResizeWindow(dpy,start.subwindow,
					MAX(win_origin_width+move_x,MIN_WIN_WIDTH),
					MAX(win_origin_height+move_y,MIN_WIN_HEIGHT)
				);
			}
		}
		else if(ev.type==ButtonRelease && ev.xbutton.subwindow!=None){
			start=None;
		}
		
		else if(ev.type==EnterNotify && ev.xbutton.subwindow!=None){
			XRaiseWindow(dpy,ev.xbutton.subwindow);
		}
	}
	return 0;
}
