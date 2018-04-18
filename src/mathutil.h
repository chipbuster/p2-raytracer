#ifndef MATHUTIL_H
#define MATHUTIL_H

#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/random.hpp>

#include <vector>

using vec3 = glm::dvec3;

constexpr double pi = 3.14159265358979323846;

namespace MathUtil {

// Calculate a `dot` b `cross` c
double scalarTripleProduct(vec3 a, vec3 b, vec3 c);
double rescaleTrig(double x);

template <typename T>
double clamp(T x, T l, T h)
{
    if (x > h)
        return h;
    if (x < l)
        return l;
    return x;
}

struct FYShuffle{
    FYShuffle(int size);

    int next();

    std::vector<int> indices;
};


// random unit vector that is not parallel to base
glm::dvec3 randHemisphere(const glm::dvec3& normal);

} // namespace MathUtil
#endif
