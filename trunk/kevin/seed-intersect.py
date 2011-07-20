#!/usr/bin/python
# Kevin Zhang
# 07/19/2011
# Reads in >= 2 coordinate files. Coordinates from first files are used as a reference; for purposes of comparability all coordinates in the other file
# are adjusted to match a coordinate in the first file if their x, y, and z coordinates are all within 1 pixel. A logical AND operation is then performed
# across all coordinate sets.

import sys, re
import numpy as np

def withinThreshold(point1, point2, thres):
    p1 = np.array(point1[0:3])
    p2 = np.array(point2[0:3])
    diff = np.abs(p2 - p1)
    return np.max(diff) <= thres

if len(sys.argv) < 3:
    print "Usage: %s ref_file1 file1 [file2] [file3] [file4] [...]" % (sys.argv[0])
    sys.exit(-1)

linepat = re.compile("[0-9 ]+")

coord_lists = []

for file in sys.argv[1:]:
    f = open(file, 'r')
    coords = []
    for line in f:
        if linepat.match(line.rstrip()):
            l = [int(x) for x in line.rstrip().split(" ")]
            coords.append(tuple(l[0:3]))
    coord_lists.append(coords)
    f.close()
sys.stderr.write("File reading complete\n")

count = 1
for i in range(1, len(coord_lists)):
    for j in range(0, len(coord_lists[i])):
        for ref_coord in coord_lists[0]:
            if coord_lists[i][j] != ref_coord and withinThreshold(ref_coord, coord_lists[i][j], 2):
                sys.stderr.write("Adjusting coordinate (%s) to (%s)\n" % (" ".join([str(x) for x in coord_lists[i][j]]), " ".join([str(x) for x in ref_coord])))
                coord_lists[i][j] = ref_coord[:]
    sys.stderr.write("Finished adjusting coordinates in file %s\n" % (count))
    count += 1
sys.stderr.write("Coordinate adjustment complete\n")


#Method 1: Take intersection of all coordinate sets
coord_sets = [set(x) for x in coord_lists]
finalcoords = set.intersection(*coord_sets)

# Method 2: Keep every coordinate in the first set that is not in any other set
#coord_sets = [set(x) for x in coord_lists[1:]]
#coords_union = set.union(*coord_sets)
#finalcoords = coord_lists[0]
#for coord in coords_union:
#    finalcoords = [x for x in finalcoords if x != coord]

#Method 3: Keep every coordinate that is in at least two sets
#finalcoords = []
#coord_sets = [set(x) for x in coord_lists]
#coord_pool = set.union(*coord_sets)
#for coord in coord_pool:
#    count = 0
#    for coords in coord_lists:
#        if coord in coords:
#            count += 1
#    if count >= 2:
#        finalcoords.append(coord)

#Method 4: Start out with the full reference set, then delete any coordinates that are in only one of the other sets
#finalcoords4 = coord_lists[0]
#coord_sets4 = [set(x) for x in coord_lists[1:]]
#coord_pool4 = set.union(*coord_sets)
#for coord in coord_pool4:
#    count = 0
#    for coords in coord_lists[1:]:
#        if coord in coords:
#            count += 1
#    if count == 1:
#       finalcoords4 = [x for x in finalcoords4 if x != coord]

#finalcoords = set.intersection(set(finalcoords), set(finalcoords4))


sys.stderr.write("Processing complete\n")

for coord in sorted(finalcoords, key=lambda x: x[2]):
    print " ".join([str(x) for x in coord])
