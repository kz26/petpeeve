#!/usr/bin/python
# Kevin Zhang
# 07/05/2011
# Reverses the z coordinate in a text file of points
# A valid input file has three space-delimited integers on each line

import sys

if len(sys.argv) != 3:
    print "Usage: %s seeds.dat total_num_slices" % (sys.argv[0])
    sys.exit(-1)

total = int(sys.argv[2])
points = []
f = open(sys.argv[1], 'r')
f.readline()
for line in f:
    l = line.rstrip().split(" ")
    if len(l) != 3: continue
    l[2] = str(abs(int(l[2]) - total))
    points.append([l[1], l[0], l[2]])
f.close()
points.reverse()
for p in points:
    print " ".join(p)
