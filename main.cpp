#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

#include <SDL2/SDL.h>

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

    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *win = SDL_CreateWindow("LiveWallpaper", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (win == nullptr){
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
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
}