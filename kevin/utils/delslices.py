#!/usr/bin/python
# Copies a directory of PET slices to a new directory and deletes the last n slices
# Arguments input_dir, output_dir, num_slices_to_delete
# Assumes all the slice files are named consistently in ascending order

import os, sys, shutil

if len(sys.argv) != 4:
    print "Usage: %s input_dir output_dir num_slices_to_delete" % (sys.argv[0])
    sys.exit(-1)

inputdir = sys.argv[1]
outputdir = sys.argv[2]
numslices = int(sys.argv[3])

if os.path.exists(outputdir):
    shutil.rmtree(outputdir)
shutil.copytree(inputdir, outputdir)
os.chdir(outputdir)
files = os.listdir(".")
for f in sorted(files)[-numslices:]:
    os.remove(f)
    print "Deleted %s" % (f)
