#include "particles/particle.hpp"
#include "window/desktop_window.hpp"

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <SDL2/SDL.h>

const Uint32 RENDER_FLAGS = SDL_RENDERER_ACCELERATED;

const int FPS = 30;
const int FRAME_DELAY = 1000 / FPS;

const int RED = 0;
const int GREEN = 0;
const int BLUE = 0;
const int ALPHA = 255;

const int TOTAL_PARTICLES = 100;


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

    int screen_width = XDisplayWidth(display, DefaultScreen(display));
    int screen_height = XDisplayHeight(display, DefaultScreen(display));

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    DesktopWindow desktop_window = create_desktop_window(
        display, root,
        screen_width,
        screen_height
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
        particles.emplace_back(Particle(screen_width, screen_height));
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
            p.update(screen_width, screen_height);
            auto [r, g, b, a] = p.get_color();
            auto [x, y] = p.get_position();
            int size = p.get_size();
            SDL_SetRenderDrawColor(ren, r, g, b, a);
            SDL_Rect rect = { static_cast<int>(x), static_cast<int>(y), size, size };
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