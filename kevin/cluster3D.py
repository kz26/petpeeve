#!/usr/bin/python
# Kevin Zhang
# 08/10/2011
# Clusters labelled 3D objects in a volume using a distance-based hierarchical method

import sys
import pickle
import numpy as np
from scipy.cluster.hierarchy import fclusterdata

# obj must be a list
def findCentroid(obj):
    dim = len(obj[0])
    totals = np.zeros(dim, float)
    for point in obj:
        for i in range(0, len(point)):
            totals[i] += point[i]
    return [int(round(x)) for x in list(totals / len(obj))]

def findLargestObject(data, obj_ids):
    return sorted([(x, len(data[x])) for x in obj_ids], key=lambda x: x[1])[-1][0]

if len(sys.argv) != 2:
    print "Usage: %s input_data_file" % (sys.argv[0])
    sys.exit(-1)

f = open(sys.argv[1], 'r')
data = pickle.load(f)
f.close()

centroids = []
for object in sorted(data.keys()):
    centroids.append(findCentroid(data[object]))

clusters = fclusterdata(centroids, 5, criterion='distance')

for clustnum in set(clusters):
    member_ids = [sorted(data.keys())[x] for x in range(0, len(clusters)) if clusters[x] == clustnum]
    biggest_member = findLargestObject(data, member_ids)
    centroid = [str(x) for x in findCentroid(data[biggest_member])]
    print " ".join(centroid)

