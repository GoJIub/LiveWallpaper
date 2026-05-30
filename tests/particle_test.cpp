#include "particles/particle.hpp"

#include <gtest/gtest.h>

#include <cstdlib>

namespace {

constexpr int kScreenWidth = 800;
constexpr int kScreenHeight = 600;
constexpr int kMaxLifetime = 1000;
constexpr SDL_Color kPaletteColor{120, 160, 220, 255};

Particle create_particle_with_lifetime_greater_than(int minimum_lifetime) {
    Particle particle(kScreenWidth, kScreenHeight, kPaletteColor, kMaxLifetime);
    int attempts = 0;
    while (particle.get_lifetime() <= minimum_lifetime) {
        if (attempts >= 100) {
            ADD_FAILURE() << "Could not create a particle with lifetime greater than " << minimum_lifetime;
            return particle;
        }
        particle = Particle(kScreenWidth, kScreenHeight, kPaletteColor, kMaxLifetime);
        ++attempts;
    }
    return particle;
}

} // namespace

TEST(ParticleTest, InitialStateIsWithinExpectedRanges) {
    std::srand(1);

    Particle particle(kScreenWidth, kScreenHeight, kPaletteColor, kMaxLifetime);
    const auto position = particle.get_position();

    EXPECT_EQ(particle.get_age(), 0);
    EXPECT_GE(position.x, 0.0);
    EXPECT_LT(position.x, kScreenWidth);
    EXPECT_GE(position.y, 0.0);
    EXPECT_LT(position.y, kScreenHeight);
    EXPECT_GE(particle.get_lifetime(), 1);
    EXPECT_LE(particle.get_lifetime(), kMaxLifetime);
}

TEST(ParticleTest, InitialSizeColorAndAlphaAreWithinExpectedRanges) {
    std::srand(3);

    Particle particle(kScreenWidth, kScreenHeight, kPaletteColor, kMaxLifetime);
    const SDL_Color color = particle.get_color();

    EXPECT_GE(particle.get_size(), 1);
    EXPECT_LE(particle.get_size(), 5);
    EXPECT_GE(color.r, kPaletteColor.r - 35);
    EXPECT_LE(color.r, kPaletteColor.r + 35);
    EXPECT_GE(color.g, kPaletteColor.g - 35);
    EXPECT_LE(color.g, kPaletteColor.g + 35);
    EXPECT_GE(color.b, kPaletteColor.b - 35);
    EXPECT_LE(color.b, kPaletteColor.b + 35);
    EXPECT_EQ(color.a, 0);
}

TEST(ParticleTest, ColorVariationIsClampedToByteRange) {
    std::srand(4);

    Particle dark_particle(kScreenWidth, kScreenHeight, SDL_Color{0, 0, 0, 255}, kMaxLifetime);
    Particle light_particle(kScreenWidth, kScreenHeight, SDL_Color{255, 255, 255, 255}, kMaxLifetime);
    const SDL_Color dark_color = dark_particle.get_color();
    const SDL_Color light_color = light_particle.get_color();

    EXPECT_LE(dark_color.r, 35);
    EXPECT_LE(dark_color.g, 35);
    EXPECT_LE(dark_color.b, 35);
    EXPECT_GE(light_color.r, 220);
    EXPECT_GE(light_color.g, 220);
    EXPECT_GE(light_color.b, 220);
}

TEST(ParticleTest, UpdateIncrementsAgeBeforeLifetimeExpires) {
    std::srand(5);

    Particle particle = create_particle_with_lifetime_greater_than(1);
    ASSERT_GT(particle.get_lifetime(), 1);

    particle.update(
        kScreenWidth,
        kScreenHeight,
        0,
        kPaletteColor,
        0.0,
        0.0
    );

    EXPECT_EQ(particle.get_age(), 1);
}

TEST(ParticleTest, LifetimeIsNeverZero) {
    for (int seed = 0; seed < 100; ++seed) {
        std::srand(seed);

        Particle particle(kScreenWidth, kScreenHeight, kPaletteColor, kMaxLifetime);

        EXPECT_GE(particle.get_lifetime(), 1);
        EXPECT_LE(particle.get_lifetime(), kMaxLifetime);
    }
}

TEST(ParticleTest, NonPositiveMaxLifetimeFallsBackToOne) {
    std::srand(6);

    Particle particle(kScreenWidth, kScreenHeight, kPaletteColor, 0);

    EXPECT_EQ(particle.get_lifetime(), 1);
    EXPECT_NO_THROW(particle.update(
        kScreenWidth,
        kScreenHeight,
        0,
        kPaletteColor,
        0.0,
        0.0
    ));
}

TEST(ParticleTest, RecreatesAfterLifetimeKeepingMaxLifetime) {
    std::srand(2);

    Particle particle = create_particle_with_lifetime_greater_than(0);

    const int lifetime = particle.get_lifetime();
    ASSERT_GT(lifetime, 0);
    for (int i = 0; i < lifetime; ++i) {
        particle.update(
            kScreenWidth,
            kScreenHeight,
            0,
            kPaletteColor,
            0.0,
            0.0
        );
    }

    EXPECT_EQ(particle.get_age(), 0);
    EXPECT_GE(particle.get_lifetime(), 1);
    EXPECT_LE(particle.get_lifetime(), kMaxLifetime);
}
