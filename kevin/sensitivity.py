#!/usr/bin/python
# Kevin Zhang
# Bria Connolly
# 07/25/2011
# Reads in a labeled image mask series that represents tumor regions
# Checks to see if the points in the input file belong to one of the tumor regions
# and calculates overall sensitivity
# Formula: Number of true positives / Number of unique tumors in series
# input file must be a pickled Python file from labelreader.py

import os, sys, re, pickle

if len(sys.argv) != 3:
    print "Usage: %s labeled_img_file your_points.txt" % (sys.argv[0])
    sys.exit(-1)

with open(sys.argv[1], 'r') as f:
    objdata = pickle.load(f)
objdata_count = {}
for k in objdata.keys():
    objdata_count[k] = 0


linepat = re.compile("[0-9 ]+")
points = []
with open(sys.argv[2], 'r') as f:
    for line in f:
        lm = linepat.match(line.rstrip())
        if not lm: continue
        l = [int(x) for x in line.rstrip().split(" ")[:3]]
        points.append(tuple(l))

for p in points:
    for label in objdata.keys():
            if  p in objdata[label]:
                objdata_count[label] += 1

numerator = 0
for k in objdata_count.keys():
    if objdata_count[k] != 0:
        numerator += 1
    else:
        print "Object not found: %s" % (k)
        print "    " + ", ".join(sorted(set([str(x[2]) for x in objdata[k]])))

denominator = len(objdata_count.keys())
sensitivity = round((float(numerator) / denominator) * 100, 2)
print sensitivity


