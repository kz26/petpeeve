#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# Reads in a series of labeled DICOM images and writes all pixel coordinates for each unique object into a serialized data structure
# Arguments: input_dir output_file

import os, sys, math, re, dicom, pickle
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
                    data[label] = []
                z = int(fnmatch.group(1))
                point = (row, col, z)
                data[label].append(point)
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

def findCentroids(objdata):
    centroids = {}
    for label in objdata.keys():
        if len(objdata[label]) == 1: continue
        xyztotals = []
        xyztotals.append(sum([x[0] for x in objdata[label]]))
        xyztotals.append(sum([x[1] for x in objdata[label]]))
        xyztotals.append(sum([x[2] for x in objdata[label]]))
        centroids[label] = []
        for v in xyztotals:
            centroids[label].append(int(round(float(v) / len(objdata[label]))))
    return centroids

def findDist(point1, point2):
    p1 = np.array(point1)
    p2 = np.array(point2)
    diff = np.abs(p2 - p1)
    diff2 = [math.pow(i, 2) for i in diff]
    return math.sqrt(np.sum(diff2))

def mergeObjects(data):
    centroids = findCentroids(data)
    for label in data.keys():
        if len(data[label]) > 1: continue
        dists = []
        for obj in centroids.keys():
            dists.append((obj, findDist(data[label][0], centroids[obj])))
        dists.sort(key=lambda x: x[1])
        data[dists[0][0]].append(data[label][0])
        del data[label]
    return data


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: %s input_dir output_file" % (sys.argv[0])
        sys.exit(-1)

    of = open(sys.argv[2], 'w')
    d = mergeObjects(readImageSeries(sys.argv[1]))
    pickle.dump(d, of)
    of.close()
