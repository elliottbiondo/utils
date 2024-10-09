Geant4 validation app
=====================

# Dependencies
- Celeritas (with same Geant4 version. See note below)
- Geant4
  - XercesC (`GEANT4_USE_GDML=ON`)
  - Qt5 (optional)
- ROOT (optional)
- HepMC3
- nlohmann/json

## NOTE
**DO NOT** mismatch Geant4 versions between Celeritas and this app. E.g. do not
link an installation of Celeritas that was built using Geant4 10.7.3 to a Geant4
11.0.3 version of this app, as some symbols may be undefined.


# Build
```bash
$ cmake ${src}
$ make
```

## CMake options
- ROOT: `USE_ROOT=[ON/OFF]`
- Multithread: `USE_MT=[ON/OFF]`
- Qt: `USE_QT=[ON/OFF]`


# Run
Usage:
```bash
$ ./g4app input_example.json output.root
$ ./g4app input_example.json
```

If no root output is provided, simulation times are printed on the terminal.

A json input file is used to set up the simulation run. Most of it is
self-explanatory, but here is a short help:  

- `geometry`: GDML input. See `utils/gdml-generator`.
- Leave `hepmc3` field blank to use the `particle_gun`:  
  - `energy` is in **[MeV]**.  
  - `vertex` is in **[cm]**.  
  - `direction` values are always normalized to become a unitary vector.  
- `num_threads` sets the number of worker threads if `USE_MT=ON`.
- `performance_run` minimizes I/O. If `true`, only performance metrics are
produced.  
- `primary_info`, `secondary_info`, `step_info` and `sensdet_info` toggle I/O
for each object. I/O is not thread-safe yet, thus this data is not stored if
`USE_MT=ON`.  
- `random_seed` uses the Unix clock time as seed.
- `verbosity` options are `0`, `1`, or `2`.
- `PrintProgress` is the interval between the event numbers printed to the
terminal.  
- Set `export_celeritas_root` to true to export the input file for the
_Celeritas_ `celer-sim` app.  
- Set `GUI` to true to open Qt5 interface to visualize geometry and events. Not
used if `USE_QT=OFF`.  
  - Edit the visualization macro `vis.mac` to change viewing preferences.  
- Scintillation physics needs `"e_ionization": true`.  
- Polarization is only used for optical photon primaries (`"pdg": -22`).

```json
{
    "geometry": "geometry.gdml",
    "simulation": {
        "hepmc3": "",
        "particle_gun": {
            "events": 1,
            "pdg": 22,
            "energy": 10,
            "vertex": [
                0,
                0,
                0
            ],
            "direction": [
                1,
                0,
                0
            ],
            "polarization": [
                1,
                0,
                0
            ]
        },
        "num_threads": 4,
        "performance_run": false,
        "primary_info": true,
        "secondary_info": true,
        "step_info": true,
        "sensdet_info": true,
        "random_seed": false,
        "spline": true,
        "eloss_fluctuation": false
    },
    "physics": {
        "compton_scattering": true,
        "photoelectric": true,
        "rayleigh_scattering": true,
        "gamma_conversion": true,
        "positron_annihilation": true,
        "bremsstrahlung": true,
        "e_ionization": true,
        "coulomb_scattering": false,
        "multiple_scattering_low": false,
        "multiple_scattering_high": false,
        "scintillation": false,
        "cerenkov": false,
        "optical_rayleigh": false,
        "muon_decay": false

    },
    "verbosity": {
        "RunManager": 0,
        "RunAction": 0,
        "EventAction": 0,
        "PhysicsList": 0,
        "PrintProgress": 100
    },
    "export_celeritas_root": false,
    "GUI": false,
    "vis_macro": "vis.mac"
}
```


# Utils/
A few ROOT macros are included for reading a ROOT output file and generate a
few example histograms. To run them:

```shell
$ root
root[0] .x macro.C("/path/to/out.root")
```


# Development notes
- **Qt 5.15.2** on **macOS 11.4** segfaults if `(x)` window button is clicked.
Session command `exit` terminates the program correctly.  
- Running a full simulation _and_ exporting `celeritas-demo-loop.input.root`
results in `Error in <TObjArray::At>: index N out of bounds`. Nevertheless, both
MC truth and Celeritas exported data are correct.
