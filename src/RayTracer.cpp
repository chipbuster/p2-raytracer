// The main ray tracer.

#pragma warning(disable : 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Parser.h"
#include "parser/Tokenizer.h"

#include <string.h> // for memset
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/component_wise.hpp>
#include <glm/gtx/string_cast.hpp>
#include "ui/TraceUI.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>

#include "mathutil.h"

using namespace std;
using std::cout;
using std::endl;
extern TraceUI *traceUI;
ostream &operator<<(ostream &os, const glm::dvec3 x);

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = true;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates
// (x,y), through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of
// 0.

glm::dvec3 RayTracer::trace(double x, double y)
{

    // DEBUGGING
    //scene->getCamera().setFOV(130);

    // Clear out the ray cache in the scene for debugging purposes,
    if (TraceUI::m_debug)
        scene->intersectCache.clear();

    ray r(glm::dvec3(0, 0, 0), glm::dvec3(0, 0, 0), glm::dvec3(1, 1, 1),
          ray::VISIBILITY);
    scene->getCamera().rayThrough(x, y, r);
    double dummy = -1;
    glm::dvec3 ret =
            traceRay(r, glm::dvec3(1.0, 1.0, 1.0), traceUI->getDepth(), dummy);
    ret = glm::clamp(ret, 0.0, 1.0);
    return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j)
{
    glm::dvec3 col(0, 0, 0);

    if (!sceneLoaded())
        return col;

    double x = double(i) / double(buffer_width);
    double y = double(j) / double(buffer_height);


    unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

    if (traceUI->aaSwitch()) {
        double dx = 1 / (double)(samples * buffer_width);
        double dy = 1 / (double)(samples * buffer_height);


        for (int i = 0; i < samples; i++) {
            for (int j = 0; j < samples; j++) {
                double nx = x + dx * (double)i - (double)(samples / 2) * dx;
                double ny = y + dy * (double)j - (double)(samples / 2) * dy;

#ifdef STOCHSSAA
        double dd = min(dx,dy);
        std::uniform_real_distribution<double> unifunitd(0.0,1.0);
        double bmx1 = unifunitd(this->generator);
        double bmx2 = unifunitd(this->generator);

        // Use box-muller transform to obtain sample from gaussian
        double rx = sqrt(-2 * log(bmx1) ) * cos(2 * pi * bmx2);
        double ry = sqrt(-2 * log(bmx1) ) * sin(2 * pi * bmx2);

        // 2sigma samples should fall within dd of the origin.
        rx *= dd; rx /= 3;
        ry *= dd; ry /= 3;

        // Modify original coordinate points with jitter
        nx += rx;
        ny += ry;
#endif

                col += trace(nx, ny);
            }
        }
        double c = 1 / (double)(samples * samples);
        col *= c;
    } else {
        col = trace(x, y);
    }

    pixel[0] = (int)(255.0 * col[0]);
    pixel[1] = (int)(255.0 * col[1]);
    pixel[2] = (int)(255.0 * col[2]);
    return col;
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray &r, const glm::dvec3 &thresh, int depth,
                               double &t)
{
    isect i;
    glm::dvec3 colorC;

    // Exhausted recursion depth. Use default color here (i.e. black)
    if (depth < 0) {
        return glm::dvec3(0.0);
    }

#if VERBOSE
    std::cerr << "== current depth: " << depth << std::endl;
#endif

    if (scene->intersect(r, i)) {
        // First, add basic color contributions
        const Material &m = i.getMaterial();

        colorC = m.shade(scene.get(), r, i);

        glm::dvec3 isectPt = r.getPosition() + i.getT() * r.getDirection();

        if (r.type() == ray::REFRACTION && !m.Trans()) {
            assert("Refraction ray intersected nontransparent");
        }

#ifdef PATHTRACING

        /* Setup for lambertian contribution to path shading */
        if(r.type() == ray::VISIBILITY){
        glm::dvec3 indirectPathColor(0.0);
        for(int count = 0; count < config.getSamples(); count++){
            // Shoot path rays into the world
            glm::dvec3 newDir = MathUtil::randHemisphere(i.getN());
            glm::dvec3 newPt = isectPt - RAY_EPSILON * r.getDirection();

            ray newRay(newPt, newDir, glm::dvec3(1.0), ray::VISIBILITY);

            glm::dvec3 colorContrib = traceRay(newRay, thresh, depth - 1, t);

            indirectPathColor += glm::dot(i.getN(), newDir) * colorContrib;
        }

        /* A proper pathtracer uses an albedo term to determine the weights. 
           As a hack, take the elementwise-mean of the diffuse terms */
        double albedo = 1.0;
        colorC += indirectPathColor * (1.0 / config.getSamples()) * albedo;
        }
#endif

        // If Recur (i.e. translucent or transparent), add recurrent
        // components
        if (m.Refl() || m.Trans()) {
            glm::dvec3 intersectPt =
                    r.getPosition() + i.getT() * r.getDirection();
            const glm::dvec3 &N = i.getN();
            const glm::dvec3 &R = r.getDirection();
            assert(abs(glm::l2Norm(N) - 1) < 1e-5);
            assert(abs(glm::l2Norm(R) - 1) < 1e-5);

            if (m.Refl()) {
                glm::dvec3 reflDir = R - 2 * glm::dot(N, R) * N;
                ray reflRay(intersectPt - RAY_EPSILON * r.getDirection(),
                            reflDir, glm::dvec3(1.0), ray::REFLECTION);

                colorC += m.kr(i) * traceRay(reflRay, thresh, depth - 1, t);
            }
            if (m.Trans()) {
                const double refrInd = i.getMaterial().index(i);

                /* http://bit.ly/2nxXIL5 */

                /* Is the ray pointed into or out of the shape?
                   NB: this does not tell us where the ray goes,
                   as outbound rays may actually be turned
                   around by TIR. This only describes initial
                   state */
                bool inbound = glm::dot(N, R) < 0;
                const glm::dvec3 Np = inbound ? N : -N; // point against ray
                const double b = inbound ? 1 / refrInd : refrInd;
                const double nn = 1 / refrInd;
                const double cosT = glm::dot(-Np, R);
                glm::dvec3 newDir;

                assert(cosT >= 0 && cosT <= 1);

                double ca = 1 - b * b * (1 - cosT * cosT);

                if (ca < 0) { // Ray is internally reflected
                    newDir = R - 2 * glm::dot(R, N) * -Np;
                } else { // Ray passes through boundary
                    newDir = b * R + (b * cosT - sqrt(ca)) * Np;
                    newDir = glm::normalize(newDir); // Needed?
                }

                // Check that inbound ray, outbound ray, and
                // normal are in a single plane
                assert(abs(MathUtil::scalarTripleProduct(R, newDir, N)) < 1e-3);

                // Is this ray's destination outside of a
                // refractive medium?
                bool escaped = ca > 0 && !inbound;

                // Create a new ray with type based on entering
                // status
                ray newRay(isectPt + RAY_EPSILON * newDir, newDir,
                           glm::dvec3(1.0),
                           escaped ? ray::VISIBILITY : ray::REFRACTION);

                colorC += traceRay(newRay, thresh, depth - 1, t);
            }

            // If this ray was a refraction ray, attenuate intensity
            if (r.type() == ray::REFRACTION) {
                if (debugMode) {
                    cout << "Distance refracted ray "
                            "traveled is "
                         << i.getT() << endl;
                }
                glm::dvec3 atten =
                        glm::pow(i.getMaterial().kt(i), glm::dvec3(i.getT()));
                colorC *= atten;
            }
        }
    } else {
        // No intersection.  This ray travels to infinity, so we color
        // it according to the background color, which in this (simple)
        // case is just black, or a cubemap color
        if (traceUI->cubeMap()) {
            CubeMap *cmap = traceUI->getCubeMap();
            colorC = cmap->getColor(r);
        } else {
            colorC = glm::dvec3(0.0, 0.0, 0.0);
        }
    }
#if VERBOSE
    std::cerr << "== depth: " << depth + 1 << " done, returning: " << colorC
              << std::endl;
#endif

    return colorC;
}

RayTracer::RayTracer(Config conf)
        : scene(nullptr),
          buffer(0),
          thresh(0),
          buffer_width(256),
          buffer_height(256),
          m_bBufferReady(false),
          config(conf)
{
}

RayTracer::~RayTracer() {}

void RayTracer::getBuffer(unsigned char *&buf, int &w, int &h)
{
    buf = buffer.data();
    w = buffer_width;
    h = buffer_height;
}

double RayTracer::aspectRatio()
{
    return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char *fn)
{
    ifstream ifs(fn);
    if (!ifs) {
        string msg("Error: couldn't read scene file ");
        msg.append(fn);
        traceUI->alert(msg);
        return false;
    }

    // Strip off filename, leaving only the path:
    string path(fn);
    if (path.find_last_of("\\/") == string::npos)
        path = ".";
    else
        path = path.substr(0, path.find_last_of("\\/"));

    // Call this with 'true' for debug output from the tokenizer
    Tokenizer tokenizer(ifs, false);
    Parser parser(tokenizer, path);
    try {
        scene.reset(parser.parseScene());
    } catch (SyntaxErrorException &pe) {
        traceUI->alert(pe.formattedMessage());
        return false;
    } catch (ParserException &pe) {
        string msg("Parser: fatal exception ");
        msg.append(pe.message());
        traceUI->alert(msg);
        return false;
    } catch (TextureMapException e) {
        string msg("Texture mapping exception: ");
        msg.append(e.message());
        traceUI->alert(msg);
        return false;
    }

    if (!sceneLoaded())
        return false;

    scene->setConfig(&config);

    return true;
}

void RayTracer::traceSetup(int w, int h)
{
    if (buffer_width != w || buffer_height != h) {
        buffer_width = w;
        buffer_height = h;
        bufferSize = buffer_width * buffer_height * 3;
        buffer.resize(bufferSize);
    }
    std::fill(buffer.begin(), buffer.end(), 0);
    m_bBufferReady = true;

    /*
     * Sync with TraceUI
     */

    threads = traceUI->getThreads();
    block_size = traceUI->getBlockSize();
    thresh = traceUI->getThreshold();
    samples = traceUI->getSuperSamples();
    aaThresh = traceUI->getAaThreshold();

    // YOUR CODE HERE
    if(traceUI->kdSwitch()){
        scene->initKdTree(traceUI->getMaxDepth(), traceUI->getLeafSize());
    }
}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
    // Always call traceSetup before rendering anything.
    traceSetup(w, h);

    // Temporarily disable debugging for initial trace
    debugMode = false;

#ifdef NDEBUG
#pragma omp parallel for schedule(dynamic)
#endif

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            tracePixel(i, j);
            if((i*w+j) % 1000 == 0)
            std::cout << "Pixel " << i * w + j << "/" << w*h << std::endl;
        }
    }
    debugMode = true;
}

int RayTracer::aaImage()
{
    // YOUR CODE HERE
    // FIXME: Implement Anti-aliasing here
    //
    // TIP: samples and aaThresh have been synchronized with TraceUI by
    //      RayTracer::traceSetup() function

    // New width and height
}

bool RayTracer::checkRender()
{
    // YOUR CODE HERE
    // FIXME: Return true if tracing is done.
    //        This is a helper routine for GUI.
    //
    // TIPS: Introduce an array to track the status of each worker thread.
    //       This array is maintained by the worker threads.
    return true; // Blocking implementation
}

void RayTracer::waitRender()
{
    // YOUR CODE HERE
    // FIXME: Wait until the rendering process is done.
    //        This function is essential if you are using an asynchronous
    //        traceImage implementation.
    //
    // TIPS: Join all worker threads here.
}

glm::dvec3 RayTracer::getPixel(int i, int j)
{
    unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;
    return glm::dvec3((double)pixel[0] / 255.0, (double)pixel[1] / 255.0,
                      (double)pixel[2] / 255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
    unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

    pixel[0] = (int)(255.0 * color[0]);
    pixel[1] = (int)(255.0 * color[1]);
    pixel[2] = (int)(255.0 * color[2]);
}
