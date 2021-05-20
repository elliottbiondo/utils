Geant4 / VecGeom geometry validation
====================================

# About

These tests cover issue
[#223](https://github.com/celeritas-project/celeritas/issues/223), where there
might be discrepancies between Geant4 and VecGeom when it comes to loading
information from GDML files. To test that, two small apps are used:
- `g4app`: Geant4 app that constructs both programmatic and gdml geometries and
produce text outputs with volume and material information for both cases. 
- `vgapp`: Use VecGeom to parse gdml files and produce text outputs with volume
and material information.

For more information on the results and test apps, see the aforementioned
Github issue and the `README.md` of each app.
