#!/usr/bin/python
# Kevin Zhang
# 07/12/2011
# Reads in a set of manual seed points and compares it to a set of generated seed points
# Arguments: points1 points2

import os, sys
import numpy as np

if len(sys.argv) != 4:
    print "Usage: %s points1.txt points2.txt threshold" % (sys.argv[0])
    sys.exit(-1)

#f1 = open(sys.argv[1], 'r')
f1data = np.loadtxt(sys.argv[1], usecols = (0, 1, 2))
f2data = np.loadtxt(sys.argv[2], usecols = (0, 1, 2))
threshold = int(sys.argv[3]) 

counter = np.zeros(f1data.shape[0])
cindex = 0

# Treat 1D arrays as iterable 2D arrays
if len(f1data.shape) == 1:
    f1data = [f1data]
    counter = np.zeros(1)
if len(f2data.shape) == 1:
    f2data = [f2data]

for row1 in f1data:
    print row1
    for row2 in f2data:
        diff = row1 - row2
        if abs(np.min(diff)) <= threshold and abs(np.max(diff)) <= threshold:
            print "    ",
            print row2
            counter[cindex] = 1
    cindex += 1

sens = (float(counter.sum()) / len(counter)) * 100
print "Sensitivity: %s %%" % (sens)