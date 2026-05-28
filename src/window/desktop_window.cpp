#include "desktop_window.hpp"

#include <X11/Xatom.h>

#include <iostream>

void get_root_window(Display*& display, Window& root) {
    display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        std::cerr << "Cannot open display" << std::endl;
        exit(1);
    }
    root = DefaultRootWindow(display);
}

void set_window_hints(Display* display, Window window) {
    Atom atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE", false);
    Atom desktop_atom = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DESKTOP", false);
    XChangeProperty(
        display,
        window,
        atom,
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<unsigned char*>(&desktop_atom),
        1
    );
    Atom states[2] = {
        XInternAtom(display, "_NET_WM_STATE_BELOW", false),
        XInternAtom(display, "_NET_WM_STATE_SKIP_TASKBAR", false)
    };
    Atom state_atom = XInternAtom(display, "_NET_WM_STATE", false);
    XChangeProperty(
        display,
        window,
        state_atom,
        XA_ATOM,
        32,
        PropModeReplace,
        reinterpret_cast<unsigned char*>(states),
        2
    );
}

DesktopWindow create_desktop_window(Display* display, Window root, int width, int height) {
    XSetWindowAttributes attrs;
    attrs.override_redirect = False;
    attrs.event_mask = ExposureMask | StructureNotifyMask;
    Window window = XCreateWindow(
        display,
        root,
        0, 0,
        width, height,
        0,
        XDefaultDepth(display, DefaultScreen(display)),
        InputOutput,
        XDefaultVisual(display, DefaultScreen(display)),
        CWOverrideRedirect | CWEventMask,
        &attrs
    );
    XStoreName(display, window, "LiveWallpaper");

    set_window_hints(display, window);

    XMapWindow(display, window);
    XLowerWindow(display, window);

    XEvent event = {};
    event.type = ClientMessage;
    event.xclient.window = window;
    event.xclient.message_type = XInternAtom(display, "_NET_WM_STATE", false);
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1;
    event.xclient.data.l[1] = XInternAtom(display, "_NET_WM_STATE_BELOW", false);
    event.xclient.data.l[2] = 0;

    XSendEvent(display, root, false,
        SubstructureNotifyMask | SubstructureRedirectMask,
        &event);
    XFlush(display);
    return {SDL_CreateWindowFrom((void*)window), window};
}