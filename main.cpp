#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <SDL2/SDL.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const Uint32 RENDER_FLAGS = SDL_RENDERER_ACCELERATED;

const int FPS = 30;
const int FRAME_DELAY = 1000 / FPS;

const int RED = 0;
const int GREEN = 0;
const int BLUE = 0;
const int ALPHA = 255;

const int TOTAL_PARTICLES = 100;
const int PARTICLE_LIFETIME = 1000;

struct DesktopWindow {
    SDL_Window* sdl_window;
    Window x11_window;
};

struct Particle {
    double x, y;
    double vx, vy;
    Uint8 r, g ,b;
    Uint8 alpha;
    int size;
    int lifetime;

    Particle() {
        x = rand() % SCREEN_WIDTH;
        y = rand() % SCREEN_HEIGHT;
        double angle = (rand() % 360) * M_PI / 180.0;
        double speed = (rand() % 50) / 10.0 + 1.0;
        vx = speed * cos(angle);
        vy = speed * sin(angle);
        r = rand() % 256;
        g = rand() % 256;
        b = rand() % 256;
        alpha = ALPHA;
        size = rand() % 5 + 1;
        lifetime = rand() % PARTICLE_LIFETIME;
    }

    void update() {
        x += vx;
        if (x - size/2 < 0 || x + size/2 > SCREEN_WIDTH) {
            vx = -vx;
        }
        y += vy;
        if (y - size/2 < 0 || y + size/2 > SCREEN_HEIGHT) {
            vy = -vy;
        }
        --lifetime;
        if (lifetime <= 0) {
            *this = Particle();
        }
    }
};


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

void change_color(int& color_modifier, int& diff) {
    ++diff;
    if (diff >= 510) {
        diff = 0;
        color_modifier = 0;
    } else if (diff >= 255) {
        color_modifier = 510 - diff;
    } else {
        color_modifier = diff;
    }
}

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    Display* display = nullptr;
    Window root;
    get_root_window(display, root);

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    DesktopWindow desktop_window = create_desktop_window(
        display, root,
        XDisplayWidth(display, DefaultScreen(display)),
        XDisplayHeight(display, DefaultScreen(display))
    );
    SDL_Window *win = desktop_window.sdl_window;
    if (win == nullptr){
        std::cout << "SDL_CreateWindowFrom Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, RENDER_FLAGS);
    if (ren == nullptr){
        std::cout << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

    int color_modifier = 0, diff = 0;
    std::vector<Particle> particles;
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        particles.emplace_back(Particle());
    }


    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        int success = SDL_SetRenderDrawColor(
            ren,
            RED + color_modifier,
            GREEN + color_modifier,
            BLUE + color_modifier,
            ALPHA
        );
        if (success != 0) {
            std::cout << "SDL_SetRenderDrawColor Error: " << SDL_GetError() << std::endl;
            return 1;
        }

        if (SDL_RenderClear(ren) != 0) {
            std::cout << "SDL_RenderClear Error: " << SDL_GetError() << std::endl;
            return 1;
        }

        for (auto& p : particles) {
            p.update();
            SDL_SetRenderDrawColor(ren, p.r, p.g, p.b, p.alpha);
            SDL_Rect rect = { static_cast<int>(p.x), static_cast<int>(p.y), p.size, p.size };
            SDL_RenderFillRect(ren, &rect);
        }

        SDL_RenderPresent(ren);

        SDL_Delay(FRAME_DELAY);
        change_color(color_modifier, diff);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    XDestroyWindow(display, desktop_window.x11_window);
    XCloseDisplay(display);
}