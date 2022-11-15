Verify GMDL
===========

Purpose of this small application is to check the available data from a given
GDML file.

The parsed data is defined in `GeometryData.hh` and filled in
`GeometryStore::loop_volumes()`.

`GeometryStore` overloads `operator<<` to produce an output table such as this one:

| Vol ID     | Copy Num   | Replica    | Mat ID     | Material | Phys. volume          | Log. volume      |
| ---------- | ---------- | ---------- | ---------- | -------- | --------------------- | ---------------- |
| 0          | 0          | 0          | 0          | vacuum   | vacuum_tube_pv        | vacuum_tube      |
| 1          | 0          | 0          | 1          | SiO2     | si_tracker_pv         | si_tracker       |
| 2          | 0          | 0          | 2          | Pb3O4    | em_calorimeter_pv     | em_calorimeter   |
| 3          | 0          | 0          | 3          | C        | had_calorimeter_pv    | had_calorimeter  |
| 4          | 0          | 0          | 4          | Ti       | sc_solenoid_pv        | sc_solenoid      |
| 5          | 0          | 0          | 5          | Fe       | iron_muon_chambers_pv | fe_muon_chambers |
| 6          | 0          | 0          | 0          | vacuum   | world_PV              | world            |
