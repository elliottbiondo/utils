HepMC3 Generator
================

# About
Produce `hepmc3` input files for the Celeritas demo-loop app.

# Dependencies
- HepMC3

# Build
```shell
$ cd build
$ cmake ..
$ make
```

# Run
The `hepmc3-gen` app can produce 2 outputs:  
- Spherical isotropic distribution of primary photons with fixed energy,
provided the output filename, the number of events, the number of particles per
event, and the energy of the primary photons in **MeV**:
```shell
$ ./hepmc3-gen [isotropic.hepmc3] [num_events] [num_part_per_evt] [MeV_energy]
```
- Parse a CMS Pythia HEPEVT ASCII file and produce an equivalent hepmc3 file 
containing only the photon primaries:
```shell
$ ./hepmc3-gen-app [cms_pythia_hepevt.data] [cms_pythia.hepmc3]
```
