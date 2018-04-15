#include <cmath>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include "light.h"

using namespace std;
extern bool debugMode;

double DirectionalLight::distanceAttenuation(const glm::dvec3 &P) const
{
    // distance to light is infinite, so f(di) goes to 0.  Return 1.
    return 1.0;
}

glm::dvec3 DirectionalLight::shadowAttenuation(const ray &r,
                                               const glm::dvec3 &p) const
{
    ray sRay1(p - RAY_EPSILON * r.getDirection(), this->getDirection(p),
              glm::dvec3(1.0), ray::SHADOW);
    isect i1;
    if (!scene->intersect(sRay1, i1)) {
        // No intersection: this ray is not attenuated
        return glm::dvec3(1.0);
    }

    if (!i1.getMaterial().Trans()) {
        // Intersected with opaque geometry. Kill shading.
        return glm::dvec3(0.0);
    }

    glm::dvec3 isectPt = sRay1.getPosition() + i1.getT() * sRay1.getDirection();
    glm::dvec3 isectPtMod = isectPt + 2 * RAY_EPSILON * sRay1.getDirection();

    // The ray collided with translucent geometry. Is this internal or
    // external?
    if (glm::dot(i1.getN(), sRay1.getDirection()) > 0) {
        // This is an internal collision. Assume that we are shading an
        // internal face of translucent geometry. The thickness is the
        // distance from the base of the ray to the i1 collision point.
        double d = i1.getT();
        glm::dvec3 atten = glm::pow(i1.getMaterial().kt(i1), glm::dvec3(d));
        ray sRay2(isectPtMod, sRay1.getDirection(), glm::dvec3(1.0),
                  ray::SHADOW);
        return atten * this->shadowAttenuation(sRay2, isectPtMod);
    } else {
        // This is an external collision. Fire an internal ray to find
        // the thickness of the object, then attenuate and find
        // remainder.
        ray iRay(isectPt + RAY_EPSILON * sRay1.getDirection(),
                 sRay1.getDirection(), glm::dvec3(1.0), ray::SHADOW);
        isect i2;
        if (!scene->intersect(iRay, i2)) {
            // Ray entered object externally but did not exit. This
            // is most likely a glancing blow. Treat as transparent.
            return glm::dvec3(1.0);
        } else {
            // Shade according to thickness and get remainder of
            // attenuation
            double d = i2.getT();
            glm::dvec3 isectPt2 = iRay.getPosition() + d * iRay.getDirection();
            glm::dvec3 isectPt2Mod = 2 * RAY_EPSILON * iRay.getDirection();
            glm::dvec3 atten = glm::pow(i1.getMaterial().kt(i1), glm::dvec3(d));
            ray sRay2(isectPt2Mod, iRay.getDirection(), glm::dvec3(1.0),
                      ray::SHADOW);
            return atten * this->shadowAttenuation(sRay2, isectPt2Mod);
        }
    }

    throw - 3;
}

glm::dvec3 DirectionalLight::getColor() const
{
    return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3 &P) const
{
    return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3 &P) const
{
    // YOUR CODE HERE

    // You'll need to modify this method to attenuate the intensity
    // of the light based on the distance between the source and the
    // point P.  For now, we assume no attenuation and just return 1.0

    double d = glm::l2Norm(P - position);
    double attenFrac =
            1 / (constantTerm + linearTerm * d + quadraticTerm * d * d);

    return min(1.0, attenFrac);
}

glm::dvec3 PointLight::getColor() const
{
    return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3 &P) const
{
    return glm::normalize(position - P);
}

glm::dvec3 PointLight::shadowAttenuation(const ray &r,
                                         const glm::dvec3 &p) const
{
    ray sRay1(p - RAY_EPSILON * r.getDirection(), this->getDirection(p),
              glm::dvec3(1.0), ray::SHADOW);
    isect i1;
    if (!scene->intersect(sRay1, i1)) {
        // No intersection: this ray is not attenuated
        return glm::dvec3(1.0);
    }

    double isectDist = i1.getT();
    double lightDist = glm::l2Norm(this->position - p);
    if (isectDist > lightDist) {
        // Intersection occurs past light source. Light is not
        // attenuated.
        return glm::dvec3(1.0);
    }

    if (!i1.getMaterial().Trans()) {
        // Intersected with opaque geometry. Kill shading.
        return glm::dvec3(0.0);
    }

    glm::dvec3 isectPt = sRay1.getPosition() + i1.getT() * sRay1.getDirection();
    glm::dvec3 isectPtMod = isectPt + 2 * RAY_EPSILON * sRay1.getDirection();

    // The ray collided with translucent geometry. Is this internal or
    // external?
    if (glm::dot(i1.getN(), sRay1.getDirection()) > 0) {
        // This is an internal collision. Assume that we are shading an
        // internal face of translucent geometry. The thickness is the
        // distance from the base of the ray to the i1 collision point.
        if (debugMode) {
            cout << "Internal collision" << endl;
        }
        double d = i1.getT();
        glm::dvec3 atten = glm::pow(i1.getMaterial().kt(i1), glm::dvec3(d));
        ray sRay2(isectPtMod, sRay1.getDirection(), glm::dvec3(1.0),
                  ray::SHADOW);
        return atten * this->shadowAttenuation(sRay2, isectPtMod);
    } else {
        // This is an external collision. Fire an internal ray to find
        // the thickness of the object, then attenuate and find
        // remainder.
        ray iRay(isectPt + RAY_EPSILON * sRay1.getDirection(),
                 sRay1.getDirection(), glm::dvec3(1.0), ray::SHADOW);
        isect i2;
        if (!scene->intersect(iRay, i2)) {
            // Ray entered object externally but did not exit. This
            // is most likely a glancing blow. Treat as transparent.
            return glm::dvec3(1.0);
        } else {
            // Shade according to thickness and get remainder of
            // attenuation
            double d = i2.getT();
            glm::dvec3 isectPt2 = iRay.getPosition() + d * iRay.getDirection();
            glm::dvec3 isectPt2Mod =
                    isectPt2 + 2 * RAY_EPSILON * iRay.getDirection();
            glm::dvec3 atten = glm::pow(i1.getMaterial().kt(i1), glm::dvec3(d));
            ray sRay2(isectPt2Mod, iRay.getDirection(), glm::dvec3(1.0),
                      ray::SHADOW);
            glm::dvec3 remainder = this->shadowAttenuation(sRay2, isectPt2Mod);
            if (debugMode) {
                cout << atten << remainder << endl;
            }
            return atten * remainder;
        }
    }
    throw - 3; // This should be unreachable??
}

#define VERBOSE 0
