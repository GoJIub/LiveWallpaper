#include "particles/particle.hpp"
#include "window/desktop_window.hpp"
#include "data/system_data.hpp"
#include "data/weather.hpp"

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

void change_color(int& color_modifier) {
    int current_time = get_current_time();
    if (current_time < 12 * 3600) {
        color_modifier = current_time * 255 / (12 * 3600);
    } else {
        color_modifier = (24 * 3600 - current_time) * 255 / (12 * 3600);
    }
}

SDL_Color get_temperature_palette_color(double temperature) {
    if (temperature < 0.0) {
        return {80, 150, 255, 255};
    }
    if (temperature < 10.0) {
        return {90, 220, 210, 255};
    }
    if (temperature < 20.0) {
        return {130, 230, 140, 255};
    }
    if (temperature < 30.0) {
        return {255, 205, 90, 255};
    }
    return {255, 110, 90, 255};
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

    WeatherClient weather_client;
    WeatherData weather_data = weather_client.current();

    std::vector<Particle> particles;
    SDL_Color palette_color = get_temperature_palette_color(weather_data.temperature);
    for (int i = 0; i < TOTAL_PARTICLES; ++i) {
        particles.emplace_back(
            Particle(
                screen_width,
                screen_height,
                palette_color
            )
        );
    }

    int color_modifier;
    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        change_color(color_modifier);
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

        if (weather_client.update()) {
            weather_data = weather_client.current();
        }

        int cpu_usage = get_cpu_usage();
        palette_color = get_temperature_palette_color(weather_data.temperature);
        for (auto& p : particles) {
            p.update(
                screen_width,
                screen_height,
                cpu_usage,
                palette_color,
                weather_data.wind_speed,
                weather_data.wind_direction
            );
            auto [r, g, b, a] = p.get_color();
            auto [x, y] = p.get_position();
            int size = p.get_size();
            SDL_SetRenderDrawColor(ren, r, g, b, a);
            SDL_Rect rect = { static_cast<int>(x), static_cast<int>(y), size, size };
            SDL_RenderFillRect(ren, &rect);
        }

        SDL_RenderPresent(ren);

        SDL_Delay(FRAME_DELAY);
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    XDestroyWindow(display, desktop_window.x11_window);
    XCloseDisplay(display);
}
