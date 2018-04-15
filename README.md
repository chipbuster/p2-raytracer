Raytracer Milestone 2
=====================
Kevin Song

# Eggstra Kredit

### Jittered Supersampling

This jittered supersampling uses the Box-Muller transform to create a 2D
gaussian random disturbance to the sampling point. It then scales this
disturbance appropriately by the AA-sampling size and applies it to the point.

Jittered supersampling is a compile-time option and can be controlled by turning
on the `STOCHASTIC_SUPERSAMPLING` CMake flag, e.g. with

```
cmake . -DSTOCHASTIC_SUPERSAMPLING=ON
```

The relevant source code can be located in `RayTracer.cpp` by grepping for the
string `#ifdef STOCHSSAA`

### New Scenes

File `periscope.ray`. It's based off of `simple/refract3.ray` but adds a giant 
box to block the scene and then adds four mirrors to bounce light around the box.

In principle, this should give the exact same image as for refract3 but with
additional mirror bounces. In practice, for some reason, my raytracer gives
extra refraction ghosts (i.e. images of an object caused by TIR).

Then again, this scene also appears to break the reference raytracer's shadow
code, so `\_0.o"?_/`.

Both raytracers an issue where colors from the box appear to creep in to the
reflected image. This is apparent even when using antialiasing. I don't know why.

# Known Issues

## Shadows

The translucent shadow issue from the previous milestone is still present. This
affects most refraction scenes and some of the polymesh scenes (e.g. easy3a).
It creates an interesting new issue on the dragon5 scene, where some of the
specular highlights disagree with the reference solution. However, I believe 
this is a manifestation of the shadow issues retained from the previous 
milestones.

## Cube Mapping

To be honest, I didn't even look at the reference solution cubemaps. According
to Piazza, I might be using the wrong set of faces or something...as far as I
can tell, the cubemap is working in a reasonable manner, even if it doesn't
match the reference solution.

## GUI Segfault

For some reason, the GUI will segfault if certain actions are performed (e.g. if
rendered with too many recursions on some scenes, or if debugging windows is
opened before initial render). I suspect these arise from my changing of
the skeleton code for the GUI, since all scenes render fine in CLI.