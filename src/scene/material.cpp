#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI *traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"
#include "../mathutil.h"

using namespace std;
extern bool debugMode;

Material::~Material() {}

extern ostream &operator<<(ostream &os, const glm::dvec3 x);

// QS alpha for phong, shadow handling prototype

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray &r, const isect &ist) const
{
    // YOUR CODE HERE

    // Your mission is to fill in this method with the rest of the phong
    // shading model, including the contributions of all the light sources.
    // You will need to call both distanceAttenuation() and
    // shadowAttenuation()
    // somewhere in your code in order to compute shadows and light falloff.
    //	if( debugMode e
    //		std::cout << "Debugging Phong code..." << std::endl;

    // TODO: Add shadow attenuation

    if (debugMode) {
        std::cout << "Entered Phong Shading" << std::endl
                  << std::endl
                  << std::endl;
    }

    isect i(ist);
    // If this is a refraction ray, treat the normals as inverted
    if (r.type() == ray::REFRACTION) {
        i.setN(-ist.getN());
    }

    glm::dvec3 output(0.0);
    glm::dvec3 intersectPt = r.getPosition() + i.getT() * r.getDirection();

    assert(abs(glm::l2Norm(i.getN()) - 1) < 1e-8 && "Normal is not normed");

    // Emissive and ambient components should be calculated even if there
    // are no lights in the scene.
    output += ke(i);
    output += ka(i) * scene->ambient();

    for (const auto &pLight : scene->getAllLights()) {
        glm::dvec3 lIntOrig = pLight->getColor();
        glm::dvec3 lDir = pLight->getDirection(intersectPt);

        glm::dvec3 shadow = pLight->shadowAttenuation(r, intersectPt);
        double attenuation = pLight->distanceAttenuation(intersectPt);

        glm::dvec3 lInt = attenuation * shadow * lIntOrig;
        //        std::cout << "Atten: " << lIntOrig << shadow << lInt
        //        << std::endl;

        // Diffuse
        glm::dvec3 dif = kd(i) * max(glm::dot(lDir, i.getN()), 0.0) * lInt;

        // Specular
        glm::dvec3 outgoing, maxSpecular;
        { // Use local capture block to prevent names from escaping
            glm::dvec3 l = -lDir;
            glm::dvec3 n = i.getN();
            outgoing = l - 2 * glm::dot(n, l) * n;
        }
        maxSpecular = -r.getDirection();
        double alph = shininess(i);
        glm::dvec3 spec = ks(i) *
                          pow(max(glm::dot(maxSpecular, outgoing), 0.0), alph) *
                          lInt;

        if (debugMode) {
            std::cout << "Shadow detected with " << shadow << std::endl;
            std::cout << "Diffuse: " << dif << std::endl;
            std::cout << "Specular: " << spec << std::endl;
        }

        output += dif + spec;
    }

    return output;
}

TextureMap::TextureMap(string filename)
{
    data = readImage(filename.c_str(), width, height);
    if (data.empty()) {
        width = 0;
        height = 0;
        string error("Unable to load texture map '");
        error.append(filename);
        error.append("'.");
        throw TextureMapException(error);
    }
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2 &coord) const
{   
    /* Code to do bilinear interpolation on textures. Input is a 2vec in 
     * [0,1] x [0,1]. Output is value at given position, bilinearly interpolated
     * between neighboring pixels. Coordinate system: x increases left-to-right,
     * y increases top-to-bottom. Top-left is (0,0) */

    double xRaw, yRaw;
    xRaw = coord[0];
    yRaw = coord[1];
    assert(xRaw >= 0 && xRaw <= 1 && yRaw >= 0 && yRaw <= 1);

    double xScaled = xRaw * (width - 1);
    double yScaled = yRaw * (height - 1);

    // Get us the index of the pixel to the upper-left of the 
    size_t i = static_cast<size_t>(xScaled);
    size_t j = static_cast<size_t>(yScaled); 

    // Get fractional part of coordinate.
    double x = xScaled - floor(xScaled);
    double y = yScaled - floor(yScaled);

    assert(x >= 0 && x <= 1 && y >= 0 && y <= 1);
    assert(abs(x + i - xScaled) <= 1e-4);
    assert(abs(y + j - yScaled) <= 1e-4);

    // The four corners surrounding this pixel: upper left, upper right, etc.
    glm::dvec3 ul = this->getPixelAt(i, j);
    glm::dvec3 ur = this->getPixelAt(i + 1, j);
    glm::dvec3 ll = this->getPixelAt(i, j + 1);
    glm::dvec3 lr = this->getPixelAt(i + 1, j + 1);

    // Bilinearly interpolate pixel values
    glm::dvec3 u_interp = (1 - x) * ul + x * ur;
    glm::dvec3 l_interp = (1 - x) * ll + x * lr;
    glm::dvec3 m_interp = (1 - y) * u_interp + y * l_interp;

    return m_interp;
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
    /* Data is a row-major set of tuples in (R,G,B) format.
       (0,0) is the top left corner. Get pixel value at given coord */

    // Overflow handling. Use simplest solution for now and tile.
    size_t xMod = MathUtil::clamp<size_t>(x,0,width-1);
    size_t yMod = MathUtil::clamp<size_t>(y,0,height-1);

    size_t index = (xMod + yMod * width) * 3;

/*
    cout << xMod << "/" << width << " --- " << yMod << "/" << height << endl;
    cout << index << " " << data.size() << endl;
*/

    assert(index < data.size());

    double normFactor = static_cast<double>(std::numeric_limits<uint8_t>::max());

    double R = static_cast<double>(data[index + 0]) / normFactor;
    double G = static_cast<double>(data[index + 1]) / normFactor;
    double B = static_cast<double>(data[index + 2]) / normFactor;

    return glm::dvec3(R, G, B);
}

glm::dvec3 MaterialParameter::value(const isect &is) const
{
    if (0 != _textureMap)
        return _textureMap->getMappedValue(is.getUVCoordinates());
    else
        return _value;
}

double MaterialParameter::intensityValue(const isect &is) const
{
    if (0 != _textureMap) {
        glm::dvec3 value(_textureMap->getMappedValue(is.getUVCoordinates()));
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    } else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}
