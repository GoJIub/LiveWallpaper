#ifndef PARTICLE_HPP
#define PARTICLE_HPP

#include <SDL2/SDL.h>

class Particle {

    struct Point {
        double x, y;
    };

public:

    Particle(int screen_width, int screen_height);

    void update(int screen_width, int screen_height);

    Point get_position() const;
    SDL_Color get_color() const;
    int get_size() const;

    ~Particle() = default;

private:
    double x, y;
    double vx, vy;
    Uint8 r, g ,b;
    Uint8 alpha;
    int size;
    int lifetime;
};

#endif // PARTICLE_HPP