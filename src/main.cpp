#ifndef COMMAND_LINE_ONLY
#include "ui/GraphicalUI.h"
#endif

#include "RayTracer.h"
#include "ui/CommandLineUI.h"
#include "scene/config.h"
#include <fstream>

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

    // Read config values from file if it exists
    ifstream in("path-config.txt", std::ios::in);
    Config config = nullptr;
    if(in.good()){
        double n1, n2, n3;
        in >> n1;
        in >> n2;
        in >> n3;
        config.set(n1, 1.0, n2, n3);
    }
    else{
        config.set(30.0, 1.0, 30.0, 2);
    }

#ifdef PATHTRACING
    cout << "Path Tracing is enabled" << endl;
#endif

    theRayTracer = new RayTracer(config);

    traceUI->setRayTracer(theRayTracer);
    return traceUI->run();
}
