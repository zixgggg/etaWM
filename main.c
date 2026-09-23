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
	int ignore_key[3]={Mod2Mask,LockMask,Mod2Mask|LockMask};
	for(int i=0;i<4;i++){
		XGrabKey(dpy,q_code,Mod4Mask|ignore_key[i],DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
	    XGrabButton(dpy, 1, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	    XGrabButton(dpy, 3, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    }
	XDefineCursor(dpy,DefaultRootWindow(dpy),cursor);
	
	int mc_origin_x;//滑鼠原本的xy
	int mc_origin_y;
	int win_origin_x;//視窗原本的xy
	int win_origin_y;
	unsigned int win_origin_width;//視窗原本的寬高，XGetGeometry裡面的width_return,height_return等都是unsigned int
	unsigned int win_origin_height;
	Window root;
	unsigned int border,depth;
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
			mc_origin_x=ev.xbutton.x_root;
			mc_origin_y=ev.xbutton.y_root;
		}
		else if(ev.type==MotionNotify && start.subwindow!=None){
			int moved_x=ev.xbutton.x_root;//滑鼠移動後的xy
			int moved_y=ev.xbutton.y_root;

			int move_x=moved_x-mc_origin_x;//滑鼠了xy移動多少
			int move_y=moved_y-mc_origin_y;

			if(start.button==1){
				XMoveWindow(dpy,start.subwindow,win_origin_x+move_x,win_origin_y+move_y);
			}
			else if(start.button==3){
	           //最小視窗寬高常數（最小可設定為1）
				const int MIN_WIN_WIDTH=50;
				const int MIN_WIN_HEIGHT=50;
				
				XResizeWindow(dpy,start.subwindow,
					MAX((int)win_origin_width+move_x,MIN_WIN_WIDTH),//視窗原本的位置+滑鼠移動了多少（也就是要移動多少），unsigned int強制轉型成(int) 因為win_origin_width跟win_origin_height是unsigned int，跟int計算會溢位
					MAX((int)win_origin_height+move_y,MIN_WIN_HEIGHT)
				);
			}
		}
		else if(ev.type==ButtonRelease && ev.xbutton.subwindow!=None){
			start.subwindow=None;
		}
		
		else if(ev.type==EnterNotify && ev.xcrossing.subwindow!=None){
			XRaiseWindow(dpy,ev.xcrossing.subwindow);
		}
		XFlush(dpy);
	}
	return 0;
}
