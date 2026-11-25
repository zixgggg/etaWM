#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <X11/Xutil.h>
#include "atom.h"
#include <stdbool.h>
/*
need to install libx11-dev
header at /usr/include/X11/Xlib.h
install libx11-doc can see man page
API reference:https://tronche.com/gui/x/xlib/function-index.html
event structures:https://tronche.com/gui/x/xlib/events/structures.html
*/

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX_WIN_AMOUNT 256//xorg預設最多可以跟256的客戶端連線
Display * dpy;
const int BORDER_WIDTH=3;
const char *name_of_WM = "etaWM";
const int MIN_WIN_WIDTH=50;//最小視窗寬高常數（最小可設定為1）
const int MIN_WIN_HEIGHT=50;
//int workspace[10]={};
typedef struct
{
	Window window_id;
	bool full_screen_status;
	int origin_x;
	int origin_y;
	unsigned int origin_width;
	unsigned int origin_height;
}Window_data;
Window_data window_list[MAX_WIN_AMOUNT]={0};
XColor focus_border_color,exact,unfocus_border_color;
Window focused_win=None;//目前聚焦視窗
void focus_win_func(Window w){
	if(w==None||w==DefaultRootWindow(dpy)||w==focused_win){
		return;
	}
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
Window_data * find_window_data(Window w){//宣告一個函式，他會回傳該視窗的陣列元素的記憶體位置，那個記憶體位置裝的資料形態是Window_data
	for(int i=0;i<MAX_WIN_AMOUNT;i++){//尋找該視窗
		if(window_list[i].window_id==w){
			return &window_list[i];
		}
	}
	return NULL;//如果沒找到視窗
}
void full_screen(Window w){
	Window_data	*data=find_window_data(w);//data是指標
	Window root_return;
	unsigned int border_width_return;
	unsigned int depth_return;
	XGetGeometry(dpy,w,&root_return,//儲存全螢幕前的視窗位子、大小等等
				 &data->origin_x,//data->相當於是(*data).
				 &data->origin_y,
				 &data->origin_width,
				 &data->origin_height,
				 &border_width_return,
				 &depth_return
				 );
	XSetWindowBorderWidth(dpy,w,0);
	XMoveResizeWindow(dpy,w,0,0, DisplayWidth(dpy,DefaultScreen(dpy)), DisplayHeight(dpy,DefaultScreen(dpy)));
	data->full_screen_status=true;
}
void unfull_screen(Window w){
	Window_data	*data=find_window_data(w);//data是指標
	XMoveResizeWindow(dpy,w,data->origin_x,data->origin_y, data->origin_width,data->origin_height);
	XSetWindowBorderWidth(dpy,w,BORDER_WIDTH);
	data->full_screen_status=false;
}
void toggle_full_screen(Window w){
	Window_data	*data=find_window_data(w);//data是指標
	if(data==NULL){
		return;
	}
	if(data->full_screen_status==false){
		full_screen(w);
	}
	else{//如果這個視窗現在是全螢幕
		unfull_screen(w);
	}
}
int main(){
	dpy=XOpenDisplay(NULL);
	Window root=DefaultRootWindow(dpy);
	init_atoms(dpy);
	Window check = XCreateSimpleWindow(dpy,root, 0, 0, 1, 1, 0, 0, 0);
//	XUnmapWindow(dpy,check);
	XChangeProperty(dpy, root, net_supporting_wm_check,
	                XA_WINDOW, 32, PropModeReplace, (unsigned char *)&check, 1);
	XChangeProperty(dpy, check, net_supporting_wm_check,
	                XA_WINDOW, 32, PropModeReplace, (unsigned char *)&check, 1);
	
	XChangeProperty(dpy, check, net_wm_name,
	                utf8_string, 8, PropModeReplace, (unsigned char *)name_of_WM, strlen(name_of_WM));
	Cursor cursor=XCreateFontCursor(dpy,XC_left_ptr);
	XDefineCursor(dpy,DefaultRootWindow(dpy),cursor);
	KeyCode q_code=XKeysymToKeycode(dpy, XStringToKeysym("q"));
	KeyCode f_code=XKeysymToKeycode(dpy, XStringToKeysym("f"));
	int ignore_key[4]={Mod2Mask,LockMask,Mod2Mask|LockMask,0};
	for(int i=0;i<4;i++){
		XGrabKey(dpy,q_code,Mod4Mask|ignore_key[i],DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
	    XGrabButton(dpy, 1, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
	    XGrabButton(dpy, 3, Mod4Mask|ignore_key[i], DefaultRootWindow(dpy), True,
	            	ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
		XGrabKey(dpy,f_code,Mod4Mask|ignore_key[i],DefaultRootWindow(dpy),True,GrabModeAsync,GrabModeAsync);
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
			Window w=ev.xmaprequest.window;
			XSelectInput(dpy,w, EnterWindowMask);
			XMapWindow(dpy,w);
			//XSetInputFocus(dpy,ev.xfocus.window,RevertToParent,CurrentTime);
			focus_win_func(w);
			for(int i=0;i<MAX_WIN_AMOUNT;i++){
	            if(window_list[i].window_id==w){//確保視窗不重複
                    break;
	            }
	            else if(window_list[i].window_id==0){
                    window_list[i].window_id=w;//把視窗id放進陣列
                    XChangeProperty(dpy,root,net_client_list,XA_WINDOW,32,PropModeAppend,(unsigned char *)&w,1);//用PropModeAppend模式把視窗ID加到屬性陣列最後面
                    break;
	            }
            }
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
			if(ev.xkey.keycode==f_code){
				if(focused_win==None){
					continue;
				}
				else{
					toggle_full_screen(focused_win);
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
			Window_data*data=find_window_data(start.subwindow);
			if (data == NULL) {
			    continue;
			}
			if(start.button==1&&data->full_screen_status==false){
				XMoveWindow(dpy,start.subwindow,win_origin_x+move_x,win_origin_y+move_y);
			}
			else if(start.button==3&&data->full_screen_status==false){
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
			Window destroy_win=ev.xdestroywindow.window;
			int destroy_win_index=-1;
;
			for(int i=0;i<MAX_WIN_AMOUNT;i++){
				if(window_list[i].window_id==destroy_win){
					destroy_win_index=i;
					break;
				}
			}
			if(destroy_win_index!=-1){
				for(int i=destroy_win_index;i<MAX_WIN_AMOUNT-1;i++){//把關掉的視窗ID移出陣列，從關掉的視窗ID那格索引開始把後面的元素覆蓋前面的，因為最後一個元素後面沒元素覆蓋它，所以i<MAX_WIN_AMOUNT-1跑到倒數第二個就好。
					window_list[i]=window_list[i+1];
				}
				window_list[MAX_WIN_AMOUNT-1].window_id=0;//覆蓋最後一個元素
				
				int window_amount=0;//記錄在window_list裡面的視窗數量
				Window net_client_list_for_XChangeProperty[MAX_WIN_AMOUNT]={0};//因為window_list現在是Window_data形態，而_NET_CLIENT_LIST只接受Window；所以要另外建立純 Window ID 陣列再傳入XChangeProperty。
				for(int i=0;i<MAX_WIN_AMOUNT;i++){
					if(window_list[i].window_id!=0){
						window_amount=window_amount+1;
						net_client_list_for_XChangeProperty[i]=window_list[i].window_id;
					}
				}
				
				XChangeProperty(dpy,root,net_client_list,XA_WINDOW,32,PropModeReplace,(unsigned char *)net_client_list_for_XChangeProperty,window_amount);//視窗關掉時更新root的_NET_CLIENT_LIST
			}
			if(destroy_win==focused_win){//如果關掉的視窗是當前聚焦的視窗
				focused_win=None;
				if(destroy_win_index-1>=0){//如果下一個要聚焦的視窗有上一個元素
					focus_win_func(window_list[destroy_win_index-1].window_id);
				}
				else{//不是destroy_win_index-1>=0就是0，也就是關掉的視窗是陣列的第一個元素，因為前面有把關掉的視窗ID移出陣列並遞補，所以就聚焦遞補上來第一個索引的元素。
					if(window_list[destroy_win_index].window_id!=0){//如果第一個元素不是0（空的索引）的話，聚焦剛剛遞補的元素，也就是從第二個遞補到第一個的元素
						focus_win_func(window_list[destroy_win_index].window_id);
					}
					else{
						continue;
					}
				}
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
