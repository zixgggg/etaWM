#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
/*
need to install libx11-dev
header at  /usr/include/X11/Xlib.h
install libx11-doc can see man page
API reference:https://tronche.com/gui/x/xlib/function-index.html
event structures:https://tronche.com/gui/x/xlib/events/structures.html
*/

#define MAX(a, b) ((a) > (b) ? (a) : (b))
Display * dpy;
XColor focus_border_color,exact,unfocus_border_color;
const int BORDER_WIDTH=3;
Window focused_win=None;
Atom net_active_window;
Atom wm_delete_window;
Atom wm_protocols;
void focus_win_func(Window w){
	XSetInputFocus(dpy,w,RevertToParent,CurrentTime);
	XRaiseWindow(dpy,w);
	XSetWindowBorder(dpy,w,focus_border_color.pixel);
	XSetWindowBorderWidth(dpy,w,BORDER_WIDTH);
	if(focused_win!=None && focused_win!=w){
		XSetWindowBorder(dpy,focused_win,unfocus_border_color.pixel);
	}
	focused_win=w;
	XChangeProperty(dpy,DefaultRootWindow(dpy),net_active_window,XA_WINDOW,32,PropModeReplace,(unsigned char *)&w,1);
}
void close_window(Window w){
	XEvent e={0};
	e.xclient.type=ClientMessage;
	e.xclient.window=w;
	e.xclient.message_type=wm_protocols;
	e.xclient.format=32;
	e.xclient.data.l[0]=wm_delete_window;
	e.xclient.data.l[1]=CurrentTime;
	XSendEvent(dpy,w,False,NoEventMask,&e);
}
int main(){
	dpy=XOpenDisplay(NULL);
	Window root=DefaultRootWindow(dpy);
	Atom net_supporting_wm_check = XInternAtom(dpy, "_NET_SUPPORTING_WM_CHECK", False);
    Atom net_wm_name             = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8_string             = XInternAtom(dpy, "UTF8_STRING", False);
    Atom net_close_window		 = XInternAtom(dpy, "_NET_CLOSE_WINDOW",False);//_NET_CLOSE_WINDOW是EWMH規定 請WM去關某個視窗
    wm_delete_window			 = XInternAtom(dpy, "WM_DELETE_WINDOW", False);//WM_DELETE_WINDOW是ICCCM規定 請程式自己優雅關閉
    wm_protocols				 = XInternAtom(dpy, "WM_PROTOCOLS", False);
    net_active_window			 = XInternAtom(dpy, "_NET_ACTIVE_WINDOW",False);
	Window check = XCreateSimpleWindow(dpy,root, 0, 0, 1, 1, 0, 0, 0);
//	XUnmapWindow(dpy,check);
	XChangeProperty(dpy, root, net_supporting_wm_check,
	                XA_WINDOW, 32, PropModeReplace, (unsigned char *)&check, 1);
	XChangeProperty(dpy, check, net_supporting_wm_check,
	                XA_WINDOW, 32, PropModeReplace, (unsigned char *)&check, 1);
	
	const char *name = "etaWM";
	XChangeProperty(dpy, check, net_wm_name,
	                utf8_string, 8, PropModeReplace, (unsigned char *)name, strlen(name));

	Cursor cursor=XCreateFontCursor(dpy,XC_left_ptr);
	XDefineCursor(dpy,DefaultRootWindow(dpy),cursor);
	KeyCode q_code=XKeysymToKeycode(dpy, XStringToKeysym("q"));
	int ignore_key[3]={Mod2Mask,LockMask,Mod2Mask|LockMask};
	for(int i=0;i<3;i++){
		XGrabKey(dpy,q_code,Mod4Mask|ignore_key[i],DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
	    XGrabButton(dpy, 1, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	    XGrabButton(dpy, 3, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    }
//	XChangePointerControl(dpy,True,True,5,1,4);
	XSelectInput(dpy,DefaultRootWindow(dpy),SubstructureRedirectMask|SubstructureNotifyMask|EnterWindowMask);//監聽事件
	int mc_origin_x;//滑鼠原本的xy
	int mc_origin_y;
	int win_origin_x;//視窗原本的xy
	int win_origin_y;
	unsigned int win_origin_width;//視窗原本的寬高，XGetGeometry裡面的width_return,height_return等都是unsigned int
	unsigned int win_origin_height;
	unsigned int border,depth;
	XButtonEvent start;
	start.subwindow=None;
	XAllocNamedColor(dpy,DefaultColormap(dpy,DefaultScreen(dpy)),"blue",&focus_border_color,&exact);//color_name at /usr/share/X11/rgb.txt,or you can see https://en.wikipedia.org/wiki/X11_color_names
	XAllocNamedColor(dpy,DefaultColormap(dpy,DefaultScreen(dpy)),"red",&unfocus_border_color,&exact);
	XEvent ev;
	for(;;){
		XNextEvent(dpy,&ev);
		if(ev.type==MapRequest){
			XSelectInput(dpy, ev.xmaprequest.window, EnterWindowMask);
			XMapWindow(dpy,ev.xmaprequest.window);
			//XSetInputFocus(dpy,ev.xfocus.window,RevertToParent,CurrentTime);
			focus_win_func(ev.xmaprequest.window);
		}
		else if(ev.type==ConfigureRequest){
			XWindowChanges wc;
			wc.x            = ev.xconfigurerequest.x;
		    wc.y            = ev.xconfigurerequest.y;
		    wc.width        = ev.xconfigurerequest.width;
		    wc.height       = ev.xconfigurerequest.height;
		    wc.border_width = BORDER_WIDTH;// 強制使用定義的邊框
		    wc.sibling      = ev.xconfigurerequest.above;
		    wc.stack_mode   = ev.xconfigurerequest.detail;
			XConfigureWindow(dpy,
							 ev.xconfigurerequest.window,
							 ev.xconfigurerequest.value_mask,
							 &wc
			);
		}
		else if(ev.type==KeyPress){
			/*
			if(ev.xkey==q_code){
				XInternAtom(dpy,"WM_DELETE_WINDOW",False);
				
			}
			*/
			if(ev.xkey.keycode==q_code){
				//XKillClient(dpy,ev.xkey.subwindow);
				Window focus = None;
				int revert;
				XGetInputFocus(dpy, &focus, &revert);
				if (focus != None && focus != DefaultRootWindow(dpy)) {
					//XKillClient(dpy, focus);
					close_window(focus);
				}
			}
		}
		else if(ev.type==ButtonPress && ev.xbutton.subwindow!=None){
			start=ev.xbutton;
			XGetGeometry(dpy,ev.xbutton.subwindow,&root,&win_origin_x,&win_origin_y,&win_origin_width,&win_origin_height,&border,&depth);
			mc_origin_x=ev.xbutton.x_root;
			mc_origin_y=ev.xbutton.y_root;
		}
		else if(ev.type==MotionNotify && start.subwindow!=None){
			int moved_x=ev.xmotion.x_root;//滑鼠移動後的xy
			int moved_y=ev.xmotion.y_root;

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
		
		else if(ev.type==EnterNotify && ev.xcrossing.window!=None){
			Window focus_win=ev.xcrossing.window;
			if(focus_win==DefaultRootWindow(dpy)){
				continue;
			}
			focus_win_func(focus_win);

			
		}
		else if (ev.type == DestroyNotify) {
		    if (ev.xdestroywindow.window == focused_win) {
		        focused_win = None;
		    }
		}
		else if(ev.type==ClientMessage){
			Atom message_type=ev.xclient.message_type;
			Window w=ev.xclient.window;
			if(message_type==net_close_window){
				close_window(w);
			}
			else if(message_type==net_active_window){
				focus_win_func(w);
			}
		}
		XFlush(dpy);
	}
	return 0;
}
