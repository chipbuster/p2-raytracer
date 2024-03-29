Final Project - Pathtracer
=====================
Kevin Song and Aditya Durvasula

# eCIS Extra Credit

I (Kevin Song) have submitted an eCIS for this class.

I (Aditya Durvasula) have submitted an eCIS for this class.

# Quick Start

```bash
  mkdir build
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DPATH_TRACING=ON
  make -jN
  cd ..    # Return to project root
  ./build/bin/ray -r 1 -s 50 -l 20 demos/cbox.ray output.png
```

# Configuration

There are two new config options for pathtracing in addition to all the
flags in the original raytracer:

  - Shader samples
    Number of scattered rays to cast at each shading collision. By default
    this is 50. Set by passing the `-s` flag on the command line.
  - Shadow samples
    Number of jittered shadow rays to cast when checking for shadows. By
    default this is 20. Set by passing the `-l` flag on the command line.

For example to render cbox.ray with 50 shader samples, 20 shadow samples and
recursion depth 1, run

```bash
./build/bin/ray -r 1 -s 50 -l 20 demos/cbox.ray output.png
```

# Compile time options

- Use the flag -DPATHTRACING=OFF to turn off path tracing. This flag is ON
  by default
- Use the flag -DFORCEAREA=ON to force all point lights to be treated as
  area lights with a radius of 1.0. This flag is ON by default. This flag
  is useful if you want to quickly convert point lights in scenes to area
  lights without having to rewrite or modify the scene

# demos

We have placed a few demo scenes that show off the path tracer and soft
shadows in the folder *demos*.

# generate.py

generate.py is a handy script that runs the path tracer on multiple shader
and shadow samples and creates a matrix of the resulting images. The following
is an example on how to run generate.py and example of a json configuration
file.

```bash
./generate.py --input demos/cbox.ray --json j.json --output output.png
```

```
{
    "samples" : [1, 5, 10, 50],
    "lightSamples" : [1, 5, 10, 20]
}
```

j.json is a simple file with only two keys (samples and lightSamples) each
mapping to a list of values. For example running the above command on the
above j.json yields the following image.

![](matrix.png)

All the images in the left-most column have 1 shader sample, all the images
in the right-most column have 20 shader samples, all the images in the
bottom-most row have 1 soft shadow sample, and all the images in the top-most
row have 50 soft shadow samples.

# Report

The file *report/article.pdf* is our report.

# Notes

## Soft Shadows

Soft shadows are only supported on area lights and point lights using the
-DFORCE_AREA flag. They don't make sense for directional lights

## (Non)GUI

Interactive rendering with the GUI is no longer supported.
