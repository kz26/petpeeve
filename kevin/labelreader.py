#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# Reads in a series of labeled DICOM images and writes all pixel coordinates for each unique object into a serialized data structure
# Arguments: input_dir output_file

import os, sys, re, dicom, pickle
import numpy as np

if len(sys.argv) != 3:
    print "Usage: %s input_dir output_file" % (sys.argv[0])
    sys.exit(-1)

fnpat = re.compile("Img001_([0-9]{4})")
data = {} # key is object ID; each key corresponds to a list of 3-tuples (x, y, z)
of = open(sys.argv[2], 'w')
os.chdir(sys.argv[1])
files = os.listdir(".")

for f in sorted(files):
    fnmatch = fnpat.match(f)
    if not fnmatch: continue
    print "Reading %s..." % (f)
    ds = dicom.read_file(f)
    for row in range(0, ds.pixel_array.shape[0]):
        for col in range(0, ds.pixel_array.shape[1]):
            label = int(ds.pixel_array[row][col])
            if label not in data.keys():
                data[label] = []
            point = (int(row), int(col), int(fnmatch.group(1)))
            data[label].append(point)

pickle.dump(data, of)
of.close()
