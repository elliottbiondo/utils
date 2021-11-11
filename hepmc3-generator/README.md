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

- Spherical isotropic distribution of primary particles with fixed energy,
provided the output filename, the number of events, the number of primaries per
event, the primary's PDG id and energy (**MeV**). Currently available PDGs:
  - `22`: photon
  - `11` (`-11`): electron (positron)
```shell
$ ./hepmc3-gen [isotropic.hepmc3] [num_events] [num_part_per_evt] [pdg_id] [MeV_energy]
```

- Parse a CMS Pythia HEPEVT ASCII file and produce an equivalent hepmc3 file by
selecting only photon primaries:
```shell
$ ./hepmc3-gen [cms_pythia_hepevt.data] [cms_pythia.hepmc3]
```
