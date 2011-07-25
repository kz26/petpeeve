#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# Reads in a series of labeled DICOM images and writes all pixel coordinates for each unique object into a serialized data structure
# Arguments: input_dir output_file

import os, sys, re, dicom, pickle
import numpy as np

fnpat = re.compile("Img001_([0-9]{4})")

def readImageSeries(imagedir):
    os.chdir(imagedir)
    files = os.listdir(".")
    data = {} # key is label value/object ID; subkey is z-coordinate; final layer is a list of 2-tuples (x, y)
    for f in sorted(files):
        fnmatch = fnpat.match(f)
        if not fnmatch: continue
        sys.stderr.write("Reading %s...\n" % (f)) 
        ds = dicom.read_file(f)
        for row in range(0, ds.pixel_array.shape[0]):
            for col in range(0, ds.pixel_array.shape[1]):
                label = int(ds.pixel_array[row][col])
                if label == 0: continue
                if label not in data.keys():
                    data[label] = {}
                z = int(fnmatch.group(1))
                if z not in data[label].keys():
                    data[label][z] = []
                point = (row, col)
                data[label][z].append(point)
    return data

def readImageSeriesBySlice(imagedir):
    os.chdir(imagedir)
    files = os.listdir(".")
    data = {} # key is z-coordinate; each key cooresponds to a list of 3-tuples(x, y, objid)
    for f in sorted(files):
        fnmatch = fnpat.match(f)
        if not fnmatch: continue
        sys.stderr.write("Reading %s...\n" % (f))
        slicenum = int(fnmatch.group(1))
        data[slicenum] = [] # initialize empty list
        ds = dicom.read_file(f)
        for row in range(0, ds.pixel_array.shape[0]):
            for col in range(0, ds.pixel_array.shape[1]):
                label = int(ds.pixel_array[row][col])
                if label != 0:
                    data[slicenum].append((row, col, label))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: %s input_dir output_file" % (sys.argv[0])
        sys.exit(-1)

    of = open(sys.argv[2], 'w')
    d = readImageSeries(sys.argv[1])
    pickle.dump(d, of)
    of.close()
