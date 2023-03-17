vgapp
=====

# Dependencies
- VecGeom
- XercesC

# Build
```bash
$ cmake /path/to/src
$ make
```

# Run
```bash
$ ./g4app [input.gdml]
```

# Notes
The thin `vgdml::Frontend` does not provide material id information, which
limits the comparison with `g4app` tables.
The link between volume id, volume, and material do match with the outputs
produced by the `g4app`.

# vgapp outputs

## **simple-cms**

| Vol ID | Material                  | Volume                              |
| ------ | ------------------------- | ----------------------------------- |
| 0      | G4_Si0x7f828b4434b0       | si_tracker_lv0x7f828b446eb0         |
| 1      | G4_Pb0x7f828b443c90       | em_calorimeter_lv0x7f828b446f80     |
| 2      | G4_C0x7f828b444820        | had_calorimeter_lv0x7f828b447050    |
| 3      | G4_Ti0x7f828b4450d0       | sc_solenoid_lv0x7f828b447120        |
| 4      | G4_Fe0x7f828b445930       | iron_muon_chambers_lv0x7f828b4471f0 |
| 5      | G4_Galactic0x7f828b442940 | world_lv0x7f828b446af0              |

## **Geant4/examples/basic/B1**

| Vol ID | Material                           | Volume                 |
| ------ | ---------------------------------- | ---------------------- |
| 0      | G4_A-150_TISSUE0x7f8f5fc4b480      | Shape10x7f8f5fc4bef0   |
| 1      | G4_BONE_COMPACT_ICRU0x7f8f5fc4bfd0 | Shape20x7f8f5fc4de80   |
| 2      | G4_WATER0x7f8f5fc486f0             | Envelope0x7f8f5fc4b350 |
| 3      | G4_AIR0x7f8f5fc49680               | World0x7f8f5fc49fe0    |

## **four-steel-slabs**

| Vol ID | Material                         | Volume                   |
| ------ | -------------------------------- | ------------------------ | 
| 0      | G4_STAINLESS-STEEL0x7fd917c1d150 | box0x7fd917c1ef80        |
| 1      | G4_STAINLESS-STEEL0x7fd917c1d150 | boxReplica0x7fd917c1f0e0 |
| 2      | G4_STAINLESS-STEEL0x7fd917c1d150 | boxReplica0x7fd917c1f260 |
| 3      | G4_STAINLESS-STEEL0x7fd917c1d150 | boxReplica0x7fd917c1f430 |
| 4      | G4_Galactic0x7fd917c1c5e0        | World0x7fd917c1e7e0      |

## **Geant4/examples/advanced/amsEcal**

| Vol ID | Material                   | Volume                    |
| ------ | -------------------------- | ------------------------- |
| 0      | Scintillator0x7ff1e8f59dd0 | fiber0x7ff1e8f5b900       |
| 1      | Lead0x7ff1e8f58fe0         | layer0x7ff1e8f5bd30       |
| 2      | Lead0x7ff1e8f58fe0         | module0x7ff1e8f694c0      |
| 3      | Galactic0x7ff1e8f5ac90     | calorimeter0x7ff1e8f69ab0 |
| 4      | Galactic0x7ff1e8f5ac90     | world0x7ff1e8f6a1a0       |
