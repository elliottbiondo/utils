Verify GMDL
===========

Purpose of this small application is to check the available data from a given
GDML file. Continuity in the logical volume IDs is tested.

The code produces a markdown output file with the following gdml information:

| Log vol ID  | Phys vol ID | Copy Num    | Replica     | Mat ID      | Material | Phys volume                         | Log volume                     |
| ----------- | ----------- | ----------- | ----------- | ----------- | -------- | ----------------------------------- | ------------------------------ | 
| 0           | 0           | 0           | 0           | 0           | vacuum   | vacuum_tube_pv0x7fc71996d240        | vacuum_tube0x7fc71996c9f0      |
| 1           | 1           | 0           | 0           | 1           | SiO2     | si_tracker_pv0x7fc71996d290         | si_tracker0x7fc71996cac0       |
| 2           | 2           | 0           | 0           | 2           | Pb3O4    | em_calorimeter_pv0x7fc71996d300     | em_calorimeter0x7fc71996cb90   |
| 3           | 3           | 0           | 0           | 3           | C        | had_calorimeter_pv0x7fc71996d390    | had_calorimeter0x7fc71996cc60  |
| 4           | 4           | 0           | 0           | 4           | Ti       | sc_solenoid_pv0x7fc71996d400        | sc_solenoid0x7fc71996cd30      |
| 5           | 5           | 0           | 0           | 5           | Fe       | iron_muon_chambers_pv0x7fc71996d4b0 | fe_muon_chambers0x7fc71996ce00 |
| 6           | 6           | 0           | 0           | 0           | vacuum   | world0x7fc71996c630_PV              | world0x7fc71996c630            |
