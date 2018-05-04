Final Project - Pathtracer
=====================
Kevin Song and Aditya Durvasula

# Quick Start

```
  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DPATH_TRACING=ON
  make -jN
  cd ..    # Return to project root
  ./build/bin/ray ./assets/scenes/cbox.ray -r 2
```

# Configuration

There are three config options for pathtracing:

  - Shader samples
    Number of scattered rays to cast at each shading collision. By default
    this is 30
  - Shadow samples
    Number of jittered shadow rays to cast when checking for shadows. By
    default this is 30
  - Pathtracer Depth
    The maximum number of times to recurse on scattered rays. Note that this
    is independently calculated from the normal recursion (which decrements
    on reflection/refraction), but normal recursion needs to be >= 0 to see
    any pathtracing effects due to a quirk in the architecture.
    Default value is 2.

These settings can be edited in `path-config.txt` where they are given
one per line, in the order specified above.

## (Non)GUI

Interactive rendering with the GUI is no longer supported.
