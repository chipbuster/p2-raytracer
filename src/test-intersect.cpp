#ifndef COMMAND_LINE_ONLY
#include "ui/GraphicalUI.h"
#endif

#include "RayTracer.h"
#include "ui/CommandLineUI.h"

#include "SceneObjects/trimesh.h"
#include "scene/scene.h"

#include <cassert>
#include <cstdlib>
#include <glm/gtc/random.hpp>
#include <iostream>

using namespace std;

RayTracer *theRayTracer;
TraceUI *traceUI;
int TraceUI::m_threads = max(std::thread::hardware_concurrency(), (unsigned)1);
int TraceUI::rayCount[MAX_THREADS];

std::ostream &operator<<(std::ostream &os, glm::dvec3 const &m)
{
    return os << m[0] << ", " << m[1] << ", " << m[2];
}

// usage : ray [option] in.ray out.bmp
// Simply keying in ray will invoke a graphics mode version.
// Use "ray --help" to see the detailed usage.
//
// Graphics mode will be substantially slower than text mode because of
// event handling overhead.
int main(int argc, char **argv)
{
    glm::dvec3 b(-3, 0, 3);
    glm::dvec3 a(3, 0, 3);
    glm::dvec3 c(0, 3, 3);

    TransformRoot *trans = new TransformRoot();
    Material *mat = new Material();

    Trimesh *tp = new Trimesh(nullptr, mat, trans);
    tp->addVertex(a);
    tp->addVertex(b);
    tp->addVertex(c);
    bool addedFace = tp->addFace(0, 1, 2);

    assert(addedFace);

    int N = 1000;

    glm::dvec3 side1 = b - a;
    glm::dvec3 side2 = c - a;

    srand(time(NULL));

    // Generate N rays that *definitely* hit the triangle from the origin
    for (int i = 0; i < N; i++) {
        double alpha = (double)rand() / RAND_MAX / 2;
        double beta = (double)rand() / RAND_MAX / 2;
        double gamma = 1 - alpha - beta;

        //        cout << alpha << " , " << beta << " , " << gamma <<
        //        endl;

        glm::dvec3 point = alpha * a + beta * b + gamma * c;

        //        cout << point << endl;

        glm::dvec3 normdir = glm::normalize(point);

        ray r = ray(glm::dvec3(0), normdir, glm::dvec3(0));
        isect is;

        bool intersect = tp->intersectLocal(r, is);

        assert(intersect);
    }

    // Generate N rays that *do not* hit the triangle from the origin.
    int outcounter = 0;
    while (outcounter < N) {
        glm::dvec3 rand = glm::normalize(glm::ballRand(1.0));

        // Not perfect but a decent sanity check
        if ((rand[1] > 0 && rand[0] > 0 && rand[0] + rand[1] < 1) ||
            (rand[1] > 0 && rand[0] < 0 && rand[1] - rand[0] < 1)) {
            continue; // bad rand
        }

        outcounter++;
        ray r = ray(glm::dvec3(0), rand, glm::dvec3(0));
        isect is;

        bool intersect = tp->intersectLocal(r, is);

        assert(!intersect);
    }
    return 0;
}
