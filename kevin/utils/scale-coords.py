#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# Scales the x, y values in a list of coordinates by the specified value
# Arguments: input_file scale-factor

import sys
if len(sys.argv) != 3:
    print "Usage: %s input_file scale_factor" % (sys.argv[0])
    sys.exit(-1)

f = open(sys.argv[1], 'r')
scalefactor = float(sys.argv[2])

for line in f:
    l = line.rstrip().split(" ")
    l[0] = str(int(round(float(l[0]) * scalefactor)))
    l[1] = str(int(round(float(l[1]) * scalefactor)))
    print " ".join(l)
