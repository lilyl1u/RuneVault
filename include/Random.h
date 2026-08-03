#ifndef RANDOM_H
#define RANDOM_H

#include <random>

// Small wrapper around the standard random generator.
// Keeping this in one class makes seeded runs consistent across the project.
class Random {
    std::mt19937 generator;

  public:
    // Default constructor seeds from the current time.
    Random();

    // Seeded constructor is useful for reproducible tests and demos.
    explicit Random(unsigned int seed);

    void seed(unsigned int seed);

    // Returns an integer in the inclusive range [min, max].
    int range(int min, int max);

    // Returns true with probability numerator / denominator.
    // Example: chance(1, 2) is a 50% chance.
    bool chance(int numerator, int denominator);
};

#endif
