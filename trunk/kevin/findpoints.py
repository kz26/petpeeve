#!/usr/bin/python
# Kevin Zhang
# 07/05/2011
# A simple detect-the-dots approach that operates on the DCM image output of petpeeve
# Arguments: input_dcm_dir

import os, sys, dicom

if len(sys.argv) != 2:
    print "Usage: %s input_dcm_dir" % (sys.argv[0])
    sys.exit(-1)

os.chdir(sys.argv[1])
files = sorted(os.listdir("."))
count = 1
for f in files:
    ds = dicom.read_file(f)
    for row in range(0, ds.pixel_array.shape[0]):
        for col in range(0, ds.pixel_array.shape[1]):
            if ds.pixel_array[row][col] != 0:
                print "%s %s %s" % (row, col, count)
    count += 1
