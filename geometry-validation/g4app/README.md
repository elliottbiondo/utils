g4app
=====

# Dependencies
- Geant4


# Build
```bash
$ cmake /path/to/src
$ make
```


# Run
```bash
$ ./g4app [geometry enum] [bool is_gdml]
```

Example:

```bash
./g4app 0 1
```

A small script that loops over all possible inputs is also available:  
```shell
$ sh compare_geometries.sh
```

## Geometry enum:
- `0`: Simplistic CMS geometry  
- `1`: Geant4/examples/basic/B1  
- `2`: Celeritas' four-steel-slabs  
- `3`: Geant4/examples/advanced/amsEcal  

## Boolean `is_gdml`
- `0`: Build programmatic geometry  
- `1`: Export programmatic geometry to GDML if gdml does not exist yet and load
GDML using `G4GDMLParser`.


# Notes
Programmatic geometry differs from `G4GDMLParser` when it comes to volume ids. 

Function `G4LogicalVolume::GetInstanceID()` might return different values
depending on where the world volume is defined in the programmatic geometry.
Conversely, the GDML file always stores the world volume last.

Since what really matters is the connection `volume -> material`, different
volume ids do not represent a real issue. Said link between volume and material
is kept, and the list of material ids is identical in both cases.

# g4app outputs

## **simple-cms**

### GDML

| Vol ID | Mat ID | Material    | Volume                |
| ------ | ------ | ----------- | --------------------- | 
| 0      | 1      | G4_Si       | si_tracker_lv         |
| 1      | 2      | G4_Pb       | em_calorimeter_lv     |
| 2      | 3      | G4_C        | had_calorimeter_lv    |
| 3      | 4      | G4_Ti       | sc_solenoid_lv        |
| 4      | 5      | G4_Fe       | iron_muon_chambers_lv |
| 5      | 0      | G4_Galactic | world_lv              |

### Programmatic

| Vol ID | Mat ID | Material    | Volume                |
| ------ | ------ | ----------- | --------------------- | 
| 0      | 0      | G4_Galactic | world_lv              |
| 1      | 1      | G4_Si       | si_tracker_lv         |
| 2      | 2      | G4_Pb       | em_calorimeter_lv     |
| 3      | 3      | G4_C        | had_calorimeter_lv    |
| 4      | 4      | G4_Ti       | sc_solenoid_lv        |
| 5      | 5      | G4_Fe       | iron_muon_chambers_lv |

## **Geant4/examples/basic/B1**

### GDML

| Vol ID | Mat ID | Material             | Volume   |
| ------ | ------ | -------------------- | -------- | 
| 0      | 2      | G4_A-150_TISSUE      | Shape1   |
| 1      | 3      | G4_BONE_COMPACT_ICRU | Shape2   |
| 2      | 1      | G4_WATER             | Envelope |
| 3      | 0      | G4_AIR               | World    |

### Programmatic

| Vol ID | Mat ID | Material             | Volume   |
| ------ | ------ | -------------------- | -------- | 
| 0      | 0      | G4_AIR               | World    |
| 1      | 1      | G4_WATER             | Envelope |
| 2      | 2      | G4_A-150_TISSUE      | Shape1   |
| 3      | 3      | G4_BONE_COMPACT_ICRU | Shape2   |

## **four-steel-slabs**

### GDML

| Vol ID | Mat ID | Material           | Volume     |
| ------ | ------ | ------------------ | ---------- | 
| 0      | 1      | G4_STAINLESS-STEEL | box        |
| 1      | 1      | G4_STAINLESS-STEEL | boxReplica |
| 2      | 1      | G4_STAINLESS-STEEL | boxReplica |
| 3      | 1      | G4_STAINLESS-STEEL | boxReplica |
| 4      | 0      | G4_Galactic        | World      |

### Programmatic

| Vol ID | Mat ID | Material           | Volume     |
| ------ | ------ | ------------------ | ---------- | 
| 0      | 0      | G4_Galactic        | World      |
| 1      | 1      | G4_STAINLESS-STEEL | box        |
| 2      | 1      | G4_STAINLESS-STEEL | boxReplica |
| 3      | 1      | G4_STAINLESS-STEEL | boxReplica |
| 4      | 1      | G4_STAINLESS-STEEL | boxReplica |

## **Geant4/examples/advanced/amsEcal**

### GDML

| Vol ID | Mat ID | Material     | Volume      |
| ------ | ------ | ------------ | ----------- | 
| 0      | 2      | Scintillator | fiber       |
| 1      | 1      | Lead         | layer       |
| 2      | 1      | Lead         | module      |
| 3      | 0      | Galactic     | calorimeter |
| 4      | 0      | Galactic     | world       |

### Programmatic

| Vol ID | Mat ID | Material     | Volume      |
| ------ | ------ | ------------ | ----------- | 
| 0      | 2      | Scintillator | fiber       |
| 1      | 1      | Lead         | layer       |
| 2      | 1      | Lead         | module      |
| 3      | 0      | Galactic     | calorimeter |
| 4      | 0      | Galactic     | world       |