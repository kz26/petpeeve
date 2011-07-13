#!/usr/bin/python
# Kevin Zhang
# 07/12/2011
# Reverses the z coordinate in a text file of points
# A valid input file has three or four space-delimited integers on each line (x y z [size_in_pixels])

import sys, re

if len(sys.argv) != 3 and len(sys.argv) != 4:
    print "Usage: %s seeds_file.txt total_num_slices [-axis]\n-axis flag also flips x,y coordinates to y,x" % (sys.argv[0])
    sys.exit(-1)

switchxy = False
if len(sys.argv) == 4 and sys.argv[3] == "-axis":
    switchxy = True
linepat = re.compile("[0-9]+ [0-9]+ [0-9]+([0-9]+)?")

total = int(sys.argv[2])
points = []
f = open(sys.argv[1], 'r')
f.readline()
for line in f:
    if not linepat.match(line.rstrip()): continue
    l = line.rstrip().split(" ")
    if len(l) == 3: l.append("") # padding
    l[2] = str(abs(int(l[2]) - total))
    if switchxy:
        points.append([l[1], l[0], l[2], l[3]])
    else:
        points.append([l[0], l[1], l[2], l[3]])
f.close()
points.reverse()
for p in points:
    print " ".join(p)
