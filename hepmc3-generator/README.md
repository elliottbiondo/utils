HepMC3 Generator
================

# About
Produce `hepmc3` input files for the Celeritas demo-loop app.

# Dependencies
- HepMC3

# Build
```shell
$ mkdir build; cd build
$ cmake ..
$ make
```

# Run
The `hepmc3-gen` app can produce 3 outputs:  

- Spherical isotropic distribution of primary particles with fixed energy. 
```shell
$ ./hepmc3-gen [isotropic.hepmc3] [num_events] [num_part_per_evt] [pdg] [MeV_energy]
```

- Particle gun with fixed energy, direction, and vertex. Direction and vertex
are expressed in cartesian coordinates. Direction is a unit vector.
```shell
$ ./hepmc3-gen [particle-gun.hepmc3] [num_events] [num_part_per_evt] [pdg] [MeV_energy] [cm_direction] [cm_vertex]
```

- Parse a CMS Pythia HEPEVT ASCII file and produce an equivalent hepmc3 file by
selecting only photon primaries:
```shell
$ ./hepmc3-gen [cms_pythia_hepevt.data] [cms_pythia.hepmc3]
```

## Currently available PDGs for the first 2 options

| PDG | Particle  |
| --- | --------- |
|  22 | photon    |
|  11 | electron  |
| -11 | positrion |
