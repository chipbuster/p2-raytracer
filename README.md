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

The two major configuration options in the pathtracer
are number of samples for shading and shadow calculation.
By default, these are 30 and 30. These can be changed in
`path-config.txt`, with number of shader samples on the
upper line and number of shadow samples on the lower one.

## (Non)GUI

Interactive rendering with the GUI is no longer supported.
