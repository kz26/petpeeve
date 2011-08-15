#!/usr/bin/python
# Reads in a series of labelled DICOM images and prints out the centroids of each object on each slice

import os, sys, dicom, re
import numpy as np

def readImageSeries(dirpath):
    fnpat = re.compile("Img001_([0-9]{4})")
    files = os.listdir(dirpath)
    data = {} # dictionary:dictionary:list -> slice number, obj label, list of points
    for f in sorted(files):
        fnmatch = fnpat.match(f)
        if not fnmatch: continue
        z = int(fnmatch.group(1))
        if z not in data.keys():
            data[z] = {}
        ds = dicom.read_file(os.path.join(dirpath, f))
        for row in range(0, ds.pixel_array.shape[0]):
            for col in range(0, ds.pixel_array.shape[1]):
                label = int(ds.pixel_array[row][col])
                if label == 0: continue
                if label not in data[z].keys():
                    data[z][label] = []
                data[z][label].append((row, col))
        sys.stderr.write("%s: %s objects\n" % (f, len(data[z])))
    return data

def getCentroid(pointlist):
    totals = np.zeros(len(pointlist[0]))
    for p in pointlist:
        totals += p
    return list(np.round(totals / len(pointlist)).astype(int))

def getAllCentroids(d):
    centroids = []
    for slice in sorted(d.keys()):
        for obj in d[slice]:
            centroids.append((getCentroid(d[slice][obj]), slice))
    return centroids

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write("Usage: %s input_dir\n" % (sys.argv[0]))
        sys.exit(-1)

    data = readImageSeries(sys.argv[1])
    centroids = getAllCentroids(data)
    for c in centroids:
        print " ".join([str(x) for x in c[0]]) + " " + str(c[1])

