Final Project - Pathtracer
=====================
Kevin Song and Aditya Durvasula

# eCIS Extra Credit

I (Kevin Song) have submitted an eCIS for this class.
I (Aditya Durvasula) have submitted an eCIS for this class.

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
    this is 50. Set by passing the `-s` flag on the command line.
  - Shadow samples
    Number of jittered shadow rays to cast when checking for shadows. By
    default this is 20. Set by passing the `-l` flag on the command line.

## (Non)GUI

Interactive rendering with the GUI is no longer supported.
