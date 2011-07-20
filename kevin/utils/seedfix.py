#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# "Fixes" a text file of coordinates
# Available Fixes:
# Reverses the z coordinate in a text file of points
# (Optional) Changes the z offset (e.g. z = 1 becomes z = 3 if the first image in the reference dir is Img001_0003
# (Optional) Swaps x,y coordinates  
# A valid input file has three or four space-delimited integers on each line (x y z [size_in_pixels])

import os, sys, re, optparse
from optparse import OptionParser

parser = OptionParser(usage='Usage: %prog: [options] coordinates.txt reference_dir')

parser.add_option("--axis", action='store_true', dest='axis', help='switch x,y coordinates')
parser.add_option("--offset", action='store_true', dest='offset', help='change z offset')
parser.add_option("--xyscale", type='float', dest='xyscale', help='Scale X and Y coordinates by FACTOR', metavar='FACTOR')

(options, args) = parser.parse_args()

if len(args) != 2:
    parser.print_help()
    sys.exit(-1)

linepat = re.compile("[0-9]+ [0-9]+ [0-9]+([0-9]+)?")
filepat = re.compile("Img001_([0-9]{4})")
ref_files = sorted(os.listdir(args[1]))
total = len(ref_files)
points = []

offset = 0
if options.offset:
    firstfile = ref_files[0]
    offset = int(filepat.match(firstfile).group(1)) - 1

f = open(args[0], 'r')

for line in f:
    if not linepat.match(line.rstrip()): continue
    l = line.rstrip().split(" ")
    l[2] = abs(int(l[2]) - total)
    if options.offset:
        l[2] += offset
    if options.xyscale:
        l[0] = int(round(int(l[0]) * options.xyscale))
        l[1] = int(round(int(l[1]) * options.xyscale))
    x = int(l[0])
    y = int(l[1])
    if options.axis:
        l[0] = y
        l[1] = x
    point_str = [str(i) for i in l]
    points.append(point_str)
f.close()

points.sort(key=lambda x: int(x[2]))
for p in points:
    print " ".join(p)
