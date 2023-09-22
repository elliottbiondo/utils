# GDML Generator
Export programmatic geometries to GDML, including secondary production cuts.


# Dependencies
Geant4 with `GEANT4_USE_GDML=ON` .


# Build
```shell
$ mkdir build; cd build;
$ cmake ..; make
```


# Run
```shell
$ ./gdml-gen [geometry_enum]
```
For the segmented simple cms, the number of segments are also needed:
```shell
$ ./gdml-gen [3 or 4] [num_segments_r] [num_segments_theta] [num_segments_z]
```

The available geometries are:

| Enum | Geometry description |
| ---- | -------------------- |
| 0    | `G4_Pb` cube of 500 m side ("infinite") |
| 1    | Simple CMS with simple materials |
| 2    | Simple CMS with composite materials |
| 3    | Segmented simple CMS with simple materials |
| 4    | Segmented simple CMS with composite materials |
| 5    | [TestEm3][testem3] with simple materials |
| 6    | [TestEm3][testem3] with composite materials |
| 7    | Flattened [TestEm3][testem3] with simple materials (for ORANGE only) |
| 8    | Flattened [TestEm3][testem3] with simple materials (for ORANGE only) |

[testem3]: https://github.com/apt-sim/AdePT/tree/master/examples/TestEm3

Secondary production cut thresholds are set equally for protons, electrons,
positrons, and gammas and are are set to 0.7 **[mm]**.


# Adding new geometries
To make the addition of new geometries easier, all the basic mandatory classes
are stored in `/src/core`, leaving the `/src` directory only with the different
geometries created by different implementations of
`G4VUserDetectorConstruction`.

The addition of new geometries only need:
- A new concrete `GeometryName` class implementation of
  `G4VUserDetectorConstruction`.  
- Two updates in `gdml-gen.cc`:  
  - Addition of said geometry in the `GeometryID` enum.  
  - Addition of said geometry in the `switch` statement:  
    ```cpp
    switch (geometry_id) {
        // (...)
        case GeometryID::geometry_name:
            run_manager->SetUserInitialization(new GeometryName());
            gdml_filename = "geometry_name.gdml";
            break;
        // (...)
    }
    ```


# Implemented geometries

## Simple CMS

| Volume                       | Composition      | Dimensions [cm]          |
| ---------------------------- | ---------------- | ------------------------ |
| world volume                 | H (G4_Galactic)  | [1000, 1000, 2000]       |
| vacuum tube                  | H (G4_Galactic)  | [0, 30 - gap, 1400]      |
| silicon tracker              | Si or SiO2       | [30, 125 - gap, 1400]    |
| electromagnetic calorimeter  | Pb or Pb3O4      | [125, 175 - gap, 1400]   |
| hadron calorimeter           | C                | [175, 275 - gap, 1400]   |
| superconducting solenoid     | Ti               | [275, 375 - gap, 1400]   |
| muon chambers                | Fe               | [375, 700, 1400]         |

- The `World volume` is a box, and its dimensions are expressed in cartesian
coordinates `[x, y, z]`.  
- **All** other volumes are concentric cylinders, and their dimensions are
expressed as `[inner radius, outer radius, length]`
- Different gaps between volumes are set so that geometry navigation can be
tested. These values are defined in `SimpleCMSDetector::volume_gaps_`. Current
configuration uses:

  | Volume                       | Gap type   |
  | ---------------------------- | ---------- |
  | world                        | N/A        |
  | vacuum tube                  | tolerance  |
  | silicon tracker              | tolerance  |
  | electromagnetic calorimeter  | overlap    |
  | hadron calorimeter           | overlap    |
  | superconducting solenoid     | millimeter |
  | muon chambers                | N/A        |

### Coordinate system
The origin of the coordinate system is at the center of the geometry, which was
designed to have the same dimensions in every axis, for simplicity. The detector
limits are located at:

| Axis | Limits [cm] |
| ---- | ----------- |
| x    | [-700, 700] |
| y    | [-700, 700] |
| z    | [-700, 700] |

Values are in `mm` in the figure.

<img src="simple_cms_evd.png" width="800"/>

## TestEm3
The TestEm3 consists of a world volume with a calorimeter. The calorimeter has
50 daughter volumes, each being a layer containing 2 volumes: a slab
representing a gap (light blue) and an absorber (red).

| Volume   | Material    | Size (x, y, z) [cm] |
| -------- | ----------- | ------------------- |
| World    | G4_Galactic | (44, 44, 44) |
| Gap      | Pb or PbWO4 | (40, 40, 0.23) |
| Absorber | LAr         | (40, 40, 0.57) |


Note: The materials assigned to the gap and absorber appear to be mistakenly
inverted in AdePT's implementation. This choice is replicated here for
consistency. Figure axes are in `mm`.

<img src="testem3_evd.png" width="800"/>
