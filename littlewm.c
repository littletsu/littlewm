/* TinyWM is written by Nick Welch <nick@incise.org> in 2005 & 2011.
 *
 * This software is in the public domain
 * and is provided AS IS, with NO WARRANTY. */

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <xcb/xproto.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

XSetWindowAttributes setNewWinAttr;
XWindowChanges newWinChanges;

XTextProperty xtextprop;
XWindowAttributes winAttr;

void register_win(Display* dpy, Window win) {
    if(XGetWindowAttributes(dpy, win, &winAttr) && winAttr.map_state == IsViewable) {
        XGetTextProperty(dpy, win, &xtextprop, XCB_ATOM_WM_NAME);
        printf("w: %d h: %d WM_NAME: %s\n", winAttr.width, winAttr.height, xtextprop.value);
        XChangeWindowAttributes(dpy, win, CWEventMask, &setNewWinAttr);
        XConfigureWindow(dpy, win, CWBorderWidth, &newWinChanges);
    }
}

int main(void)
{
    Display * dpy;
    Window root;
    XWindowAttributes rootAttr;
    XSetWindowAttributes setRootAttr;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    setRootAttr.event_mask = SubstructureNotifyMask;
    start.subwindow = None;
    setNewWinAttr.event_mask = EnterWindowMask;
    setNewWinAttr.border_pixel = 0;
    newWinChanges.border_width = 1;

    printf("littlewm\n");
    if(!(dpy = XOpenDisplay(0x0))) return 1;

    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
            ButtonPressMask|ButtonReleaseMask|PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    root = XDefaultRootWindow(dpy);
    XGetWindowAttributes(dpy, root, &rootAttr);
    printf("root w: %d root h: %d\n", rootAttr.width, rootAttr.height);
    XChangeWindowAttributes(dpy, root, CWEventMask, &setRootAttr);

    Window rootReturn, parentReturn, *childrenReturn;
    unsigned int nChild;
    XQueryTree(dpy, root, &rootReturn, &parentReturn, &childrenReturn, &nChild);
    
    while(nChild--) {
        register_win(dpy, childrenReturn[nChild]);
    }

    for(;;)
    {
        XNextEvent(dpy, &ev);
        printf("evtype: %d\n", ev.type);
        if(ev.type == ButtonPress && ev.xbutton.subwindow != None)
        {
            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
        }
        else if(ev.type == MotionNotify && start.subwindow != None)
        {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, start.subwindow,
                attr.x + (start.button==1 ? xdiff : 0),
                attr.y + (start.button==1 ? ydiff : 0),
                MAX(1, attr.width + (start.button==3 ? xdiff : 0)),
                MAX(1, attr.height + (start.button==3 ? ydiff : 0)));
        } else if(ev.type == CreateNotify) {
            register_win(dpy, ev.xcreatewindow.window);
        } else if(ev.type == EnterNotify) {
            XRaiseWindow(dpy, ev.xcrossing.window);
        }
        else if(ev.type == ButtonRelease)
            start.subwindow = None;
    }
}

