#ifndef COMMAND_LINE_ONLY
#include "ui/GraphicalUI.h"
#endif

#include "RayTracer.h"
#include "ui/CommandLineUI.h"
#include "scene/config.h"
#include <fstream>
#include <iostream>

using namespace std;

RayTracer *theRayTracer;
TraceUI *traceUI;
int TraceUI::m_threads = max(std::thread::hardware_concurrency(), (unsigned)1);
int TraceUI::rayCount[MAX_THREADS];

// usage : ray [option] in.ray out.bmp
// Simply keying in ray will invoke a graphics mode version.
// Use "ray --help" to see the detailed usage.
//
// Graphics mode will be substantially slower than text mode because of
// event handling overhead.
int main(int argc, char **argv)
{
    if (argc != 1) {
        // text mode
        traceUI = new CommandLineUI(argc, argv);
    } else {
#ifdef COMMAND_LINE_ONLY
        // still text mode
        traceUI = new CommandLineUI(argc, argv);
#else
        // graphics mode
        traceUI = new GraphicalUI();
#endif
    }

    int samples = 50;
    if(traceUI->getSamples() != -1) {
        samples = traceUI->getSamples();
    }

    int lightSamples = 20;
    if(traceUI->getLightSamples() != -1) {
        lightSamples = traceUI->getLightSamples();
    }

    Config config(samples, 1.0, lightSamples);

#ifdef PATHTRACING
    cout << "Path Tracing is enabled" << endl;
#endif

    theRayTracer = new RayTracer(config);

    traceUI->setRayTracer(theRayTracer);
    return traceUI->run();
}
