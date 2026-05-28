#ifndef DESKTOP_WINDOW_HPP
#define DESKTOP_WINDOW_HPP

#include <X11/Xlib.h>
#include <SDL2/SDL.h>

struct DesktopWindow {
    SDL_Window* sdl_window;
    Window x11_window;
};

void get_root_window(Display*& display, Window& root);
void set_window_hints(Display* display, Window window);
DesktopWindow create_desktop_window(Display* display, Window root, int width, int height);

#endif // DESKTOP_WINDOW_HPP