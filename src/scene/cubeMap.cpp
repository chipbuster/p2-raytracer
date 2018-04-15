#include "cubeMap.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include "../scene/material.h"
#include "../ui/TraceUI.h"
#include "glm/gtx/norm.hpp"
#include "ray.h"

#include <iostream>
extern TraceUI *traceUI;
extern bool debugMode;
std::ostream &operator<<(std::ostream &os, const glm::dvec3 x);

glm::dvec3 CubeMap::getColor(ray r) const
{
    const glm::dvec3 &R = r.getDirection();
    assert(std::abs(glm::l2Norm(R) - 1.0) < 1e-4);

    glm::dvec3 x(1, 0, 0);
    glm::dvec3 y(0, 1, 0);
    glm::dvec3 z(0, 0, 1);

    // Which face does this ray point into?
    std::vector<double> dot_prods(6);
    
    dot_prods[0] = glm::dot(-x, R);
    dot_prods[1] = glm::dot(x, R);
    dot_prods[2] = glm::dot(y, R);
    dot_prods[3] = glm::dot(-y, R);
    dot_prods[4] = glm::dot(z, R);
    dot_prods[5] = glm::dot(-z, R);

    int max =
            std::distance(dot_prods.begin(),
                          std::max_element(dot_prods.begin(), dot_prods.end()));

    /* NB: for whatever reason, the cubemap defines a left-handed coordinate
     * system. We deal with this by switching the mapping for the x-axis (in the
     * tMap structure, 0, is +x and 1 is -x, we switch them here).*/

    /* Compute positions on this face by extending the vector so that it hits
     * a square face that is 1 unit away from the camera. The two remaining
     * elements of the vector will be the position on the face--which elements
     * is determined by which face was hit. */

    double lr; // Left-to-right position, should be in [-1,1]
    double tb; // Top-to-bottom position, should be in [-1,1]
    glm::dvec3 rescale;
    switch (max) {
        case 0:
            rescale = R / -R[0];
            lr = -rescale[2];
            tb = rescale[1];
            break;
        case 1:
            rescale = R / R[0];
            lr = rescale[2];
            tb = rescale[1];
            break;
        case 2:
            rescale = R / R[1];
            lr = -rescale[0];
            tb = -rescale[2];
            break;
        case 3:
            rescale = R / -R[1];
            lr = -rescale[0];
            tb = rescale[2];
            break;
        case 4:
            rescale = R / R[2];
            lr = -rescale[0];
            tb = rescale[1];
            break;
        case 5:
            rescale = R / -R[2];
            lr = rescale[0];
            tb = rescale[1];
            break;
    }

    assert(lr >= -1 && lr <= 1 && tb >= -1 && tb <= 1);
    // Rescale into [0,1]
    lr = lr / 2.0 + 0.5;
    tb = tb / 2.0 + 0.5;

/*
    if(debugMode){
        std::cout << "ray is " << R << ", with max " << max << std::endl;
        std::cout << lr <<  "," << tb << std::endl;
    }
*/

    return tMap[max]->getMappedValue(glm::dvec2(lr, tb));
}

CubeMap::CubeMap() {}

CubeMap::~CubeMap() {}

void CubeMap::setNthMap(int n, TextureMap *m)
{
    if (m != tMap[n].get())
        tMap[n].reset(m);
}
