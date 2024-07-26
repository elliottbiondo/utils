# Geant4 Raytracer test app

## Dependencies

- Geant4 with `GEANT4_USE_RAYTRACER_X11=ON`.
- XQuartz (macOS)

## Build & run

```shell
$ mkdir build; cd build;
$ cmake ..
$ make
$ ./main input.gdml
```

Use the `vis.mac` macro to change ray tracing options at run time.
