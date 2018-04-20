#include "mathutil.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <limits>
#include <random>

namespace MathUtil {

// Calculate a `dot` b `cross` c
double scalarTripleProduct(vec3 a, vec3 b, vec3 c)
{
    glm::dot(a, glm::cross(b, c));
}

// Rescale a value for inverse trig functions
double rescaleTrig(double x)
{
    while (x > 1) {
        x -= pi;
    }
    while (x < -1) {
        x += pi;
    }
    return x;
}

// Fisher-Yates shuffle to get indices.
FYShuffle::FYShuffle(int size)
{
    for (int i = 0; i < size; i++) {
        indices.push_back(i);
    }
}

int FYShuffle::next()
{
    if (indices.empty()) {
        throw - 5;
    }

    std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(0, indices.size() - 1);

    int j = dist(gen);
    std::swap(indices[j], indices[indices.size() - 1]);

    int val = indices[indices.size() - 1];
    indices.pop_back();
    return val;
}


glm::dvec3 uniformSampleHemisphere(const float &r1, const float &r2)
{
    double theta = 2 * M_PI * r1;
    double phi = acos(1 - 2 * r2);
    double x = sin(phi) * cos(theta);
    double y = sin(phi) * sin(theta);
    double z = cos(phi);

    return glm::dvec3(x, y, z);
}

thread_local static uint32_t s_RndState = 0;
thread_local static bool s_RndInit = false;
constexpr float invFltMax = 1.0 / (std::numeric_limits<uint32_t>::max());
static uint32_t XorShift32()
{
    if (!s_RndInit) {
        s_RndState = rand();
        s_RndInit = true;
    }

    uint32_t x = s_RndState;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 15;
    s_RndState = x;
    return x;
}

double randDouble(const double limit) {
    return XorShift32() / (RAND_MAX / limit);
}

glm::dvec3 randHemisphere(const glm::dvec3 &normal)
{
    float r1 = static_cast<float>(XorShift32()) * invFltMax;
    float r2 = static_cast<float>(XorShift32()) * invFltMax;
    glm::dvec3 rand = uniformSampleHemisphere(r1, r2);

    // If not in hemisphere, apply householder reflector
    if (glm::dot(rand, normal) < 0) {
        rand -= 2 * glm::dot(rand, normal) * normal;
    }

    assert(glm::length(rand) > 0.99 && glm::length(rand) <= 1.01);

    return rand;
}

} // namespace MathUtil
