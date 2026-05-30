#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <SDL2/SDL.h>

class Particle {

public:

    struct Point {
        double x, y;
    };

    Particle(int screen_width, int screen_height, SDL_Color palette_color, int lifetime);

    void update(
        int screen_width,
        int screen_height,
        int vel_modifier,
        SDL_Color palette_color,
        double wind_speed,
        double wind_direction
    );

    Point get_position() const;
    SDL_Color get_color() const;
    int get_size() const;
    int get_lifetime() const;
    int get_age() const;

    ~Particle() = default;

private:
    double x, y;
    double vx, vy;
    Uint8 r, g ,b;
    Uint8 alpha;
    int size;
    int max_lifetime;
    int lifetime;
    int age = 0;
};

#endif // PARTICLE_HPP
