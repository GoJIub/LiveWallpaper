#include "particle.hpp"

#include <cmath>

const int PARTICLE_LIFETIME = 1000;

static const double pi = std::acos(-1.0);

Particle::Particle(int screen_width, int screen_height) {
    x = rand() % screen_width;
    y = rand() % screen_height;
    double angle = (rand() % 360) * pi / 180.0;
    double speed = (rand() % 50) / 10.0 + 1.0;
    vx = speed * cos(angle);
    vy = speed * sin(angle);
    r = rand() % 256;
    g = rand() % 256;
    b = rand() % 256;
    alpha = 255;
    size = rand() % 5 + 1;
    lifetime = rand() % PARTICLE_LIFETIME;
}

void Particle::update(int screen_width, int screen_height) {
    x += vx;
    if (x - size/2 < 0 || x + size/2 > screen_width) {
        vx = -vx;
    }
    y += vy;
    if (y - size/2 < 0 || y + size/2 > screen_height) {
        vy = -vy;
    }
    --lifetime;
    if (lifetime <= 0) {
        *this = Particle(screen_width, screen_height);
    }
}

Particle::Point Particle::get_position() const {
    return {x, y};
}

SDL_Color Particle::get_color() const {
    return {r, g, b, alpha};
}

int Particle::get_size() const {
    return size;
}