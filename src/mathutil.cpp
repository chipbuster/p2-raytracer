#include "mathutil.h"

#include <random>
#include <algorithm>
#include <iterator>

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
FYShuffle::FYShuffle(int size) {
    for(int i = 0; i < size; i++){
        indices.push_back(i);
    }
}

int FYShuffle::next(){
    if(indices.empty()){
        throw -5;
    }

    std::default_random_engine gen;
    std::uniform_int_distribution<int> dist(0,indices.size()-1);

    int j = dist(gen);
    std::swap(indices[j], indices[indices.size() - 1]);

    int val = indices[indices.size() - 1];
    indices.pop_back();
    return val;
}

double randDouble(double limit) {
    return rand() / (RAND_MAX / limit);
}

glm::dvec3 rand3DVector(glm::dvec3 base) {
    glm::dvec3 vec(0.0);
    for(int i = 0; i < 3; i++) {
        vec[i] = randDouble(1.0);
    }
    if(glm::length(glm::cross(vec, base)) == 0.0 or
       glm::length(vec) == 0.0) {
        return rand3DVector(base);
    }
    return glm::normalize(vec);
}

} // namespace MathUtil
