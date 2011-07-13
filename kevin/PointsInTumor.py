#!/usr/bin/python
# Kevin Zhang
# 07/12/2011
# Reads in a text file of coordinate locations and calulates how many are true positives using a mask
# Arguments: coordinatesFile dcm_mask_dir

import os, sys, dicom

if len(sys.argv) != 3:
    print "Usage: %s coordinatesFile dcm_mask_dir" % (sys.argv[0])
    sys.exit(-1)

hits = 0
lines = 0

f = open(sys.argv[1], 'r')
for line in f:
    l = line.rstrip().split(" ")
    x = int(round(float(l[0])))
    y = int(round(float(l[1])))
    z = "%04d" % (int(round(float(l[2]))))

    mfp = os.path.join(sys.argv[2], "Img001_%s" % (z))
    if not os.path.exists(mfp):
        print "WARNING: %s not found, skipping" % (mfp)
        continue
    ds = dicom.read_file(mfp)
    if ds.pixel_array[x][y] != 0:
        print line,
        hits += 1
    lines += 1

print "\n# of TPs: %s" % (hits)
print "# of FPs: %s" % (lines - hits)
print "Total # of points: %s" % (lines)
