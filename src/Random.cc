#include "Random.h"

#include <chrono>
#include <stdexcept>

// Seeds the generator from the current time for normal play.
Random::Random():
    generator{static_cast<unsigned int>(
        std::chrono::system_clock::now().time_since_epoch().count())} {}

// Seeds the generator from a fixed value for tests/reproducible runs.
Random::Random(unsigned int seed): generator{seed} {}

// Replaces the current generator seed.
void Random::seed(unsigned int seed) {
    generator.seed(seed);
}

// Returns an inclusive random integer and rejects invalid ranges.
int Random::range(int min, int max) {
    if (min > max) {
        throw std::invalid_argument{"Random::range min cannot exceed max"};
    }

    std::uniform_int_distribution<int> distribution{min, max};
    return distribution(generator);
}

// Returns true with probability numerator / denominator.
bool Random::chance(int numerator, int denominator) {
    if (denominator <= 0 || numerator < 0 || numerator > denominator) {
        throw std::invalid_argument{"Random::chance received invalid odds"};
    }

    return range(1, denominator) <= numerator;
}
