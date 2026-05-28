#include "particle.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>

const int PARTICLE_LIFETIME = 1000;

static const double pi = std::acos(-1.0);

static Uint8 vary_color(Uint8 value) {
    int color = static_cast<int>(value) + rand() % 71 - 35;
    return static_cast<Uint8>(std::clamp(color, 0, 255));
}

Particle::Particle(int screen_width, int screen_height, SDL_Color palette_color) {
    x = rand() % screen_width;
    y = rand() % screen_height;
    double angle = (rand() % 360) * pi / 180.0;
    double speed = (rand() % 50) / 10.0 + 1.0;
    vx = speed * std::cos(angle);
    vy = speed * std::sin(angle);
    r = vary_color(palette_color.r);
    g = vary_color(palette_color.g);
    b = vary_color(palette_color.b);
    alpha = 0;
    size = rand() % 5 + 1;
    lifetime = rand() % PARTICLE_LIFETIME;
}

void Particle::update(
    int screen_width,
    int screen_height,
    int vel_modifier,
    SDL_Color palette_color,
    double wind_speed,
    double wind_direction
) {
    ++age;
    if (age < lifetime / 4) {
        alpha = static_cast<Uint8>(age * 4 * 255 / lifetime);
    } else if (age > lifetime * 3 / 4) {
        alpha = static_cast<Uint8>((lifetime - age) * 4 * 255 / lifetime);
    } else {
        alpha = 255;
    }

    double speed_multiplier = 1.0 + vel_modifier / 100.0;
    double wind_angle = wind_direction * pi / 180.0;
    double wind_force = wind_speed / 60.0;

    x += vx * speed_multiplier + std::sin(wind_angle) * wind_force;
    if (x - size/2 < 0 || x + size/2 > screen_width) {
        vx = -vx;
    }
    y += vy * speed_multiplier - std::cos(wind_angle) * wind_force;
    if (y - size/2 < 0 || y + size/2 > screen_height) {
        vy = -vy;
    }
    if (age >= lifetime) {
        *this = Particle(screen_width, screen_height, palette_color);
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
