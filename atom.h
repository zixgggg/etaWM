#include <X11/Xlib.h>
#include <X11/Xatom.h>
void init_atoms(Display * dpy);//函式預設宣告就是extern
extern Atom net_active_window;
extern Atom wm_delete_window;
extern Atom wm_protocols;
extern Atom net_client_list;
extern Atom net_supporting_wm_check;
extern Atom net_wm_name;
extern Atom utf8_string;
extern Atom net_close_window;
extern Atom net_supported;
