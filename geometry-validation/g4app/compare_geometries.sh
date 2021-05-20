#!/bin/sh

# Clean up
rm *prog.txt
rm *gdml.txt

# Loop over geometry options
for GEOMETRY in 0 1 2 3
do
    # Loop over programmatic and gdml options
    # First loop exports the geometry to gdml in preparation for the second loop
    for GDML in 0 1
    do
        ./g4app $GEOMETRY $GDML
    done
done

# Compare data
diff simple-cms_prog.txt simple-cms_gdml.txt
diff example-b1_prog.txt example-b1_gdml.txt
diff four-steel-slabs_prog.txt four-steel-slabs_gdml.txt
diff ams-ecal_prog.txt ams-ecal_gdml.txt
