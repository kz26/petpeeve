#!/usr/bin/python
# Kevin Zhang
# Bria Connolly
# 07/25/2011
# Alternate sensitivity calculation routine
# Reads in a reference series of labeled tumor region masks and an input series of
# binary masks
# For each uniquely labeled object in the reference series, check if any of its points
# overlap with points in the input series
# Formula: Number of true positives / Number of unique tumors in series
# labeled input must be a pickled Python file from labelreader.py

import os, sys, re, pickle, dicom

if len(sys.argv) != 3:
    print "Usage: %s labeled_ref_file input_dir" % (sys.argv[0])
    sys.exit(-1)

with open(sys.argv[1], 'r') as f:
    objdata = pickle.load(f)
objdata_count = {}
for k in objdata.keys():
    objdata_count[k] = False

currentslice = 0
for k in objdata.keys():
    for point in sorted(objdata[k], key=lambda x: x[2]):
        if point[2] != currentslice:
            currentslice = point[2]
            fn = "Img001_%04d.dcm" % (point[2])
            ds = dicom.read_file(os.path.join(sys.argv[2], fn))
        if ds.pixel_array[point[0]][point[1]] != 0:
            objdata_count[k] = True
            break

numerator = 0
for k in objdata_count.keys():
    if objdata_count[k]:
        numerator += 1
    else:
        sys.stdout.write("Object not found: %s\n" % (k))
        sys.stdout.write("    " + ", ".join(sorted(set([str(x[2]) for x in objdata[k]]))) + "\n")

denominator = len(objdata_count.keys())
sensitivity = round((float(numerator) / denominator) * 100, 2)
print sensitivity


