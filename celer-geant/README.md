Celeritas-Geant4 offloading app for validation
==============================================

# Dependencies

- Geant4 v11 or newer with `GEANT4_USE_GDML=ON` and
  `GEANT4_BUILD_MULTITHREADED=ON`
- Celeritas v0.6 or newer with `CELERITAS_USE_Geant4=ON`
- ROOT

# Build and run

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ./celer-geant input.json
```

# Input

See `input-example.json`.  
Keys `"offload_particles"` and `"log_progress"` are optional.

# I/O
`RootIO` is a thread-local singleton that owns a `RootDataStore` object, which
maps all sensitive detector data. Each worker-thread generates its own ROOT
output file with the thread ID appended to the filename. If you prefer to merge
all files into a single output, you can use ROOT's `hadd` command:
```sh
$ hadd [merged-output.root] [output-0.root output-1.root ...]
```
Selecting all thread IDs (`*`) also works:
```sh
$ hadd [merged-output.root] [output-*.root]
```

## Adding new histograms

- Expand JSON with new histogram information.
- `RootDataStore.hh`: Add histogram to `SensDetData` and initialize it in
  `SensDetData::Initialize` using the `SDD_INIT_[TH1D/TH2D]` macros.
  - `TH2D` histograms require `"x"` and `"y"` keys for each axis binning.
- Fill histogram (usually via `SensitiveDetector::ProcessHits`).
- `RootIO.cc`: Write histogram to disk during `RootIO::Finalize` using
  `RIO_HIST_WRITE` macro.
