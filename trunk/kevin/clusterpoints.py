#!/usr/bin/python
# Kevin Zhang
# 07/12/2011
# Reads in a list of generated candidate points and performs simple clustering
# Prints out the centroids of each cluster

import sys
import numpy as np
import scipy as sp
from scipy.cluster.hierarchy import fclusterdata

def getCentroids(data, centroid_ids):
    if len(data) != len(centroid_ids):
        raise IndexError("# of data rows does not match size of centroid ID vector")
    centroids = np.zeros((max(centroid_ids), 3)) # actual centroid coordinates
    xyztotals = np.zeros((max(centroid_ids), 3)) # running sum of x,y,z coordinates
    obstotals = np.zeros(max(centroid_ids)) # running count of observations in each cluster
    for i in range(0, len(data)):
        xyztotals[centroid_ids[i] - 1] += data[i]
        print xyztotals
        obstotals[centroid_ids[i] - 1] += 1
        print obstotals
    for i in range(0, len(centroids)):
        centroids[i] = xyztotals[i] / obstotals[i]
    return sorted(np.round(centroids).astype(int), key = lambda x: x[2])

if len(sys.argv) != 2:
    print "Usage: %s input_data_file" % (sys.argv[0])
    sys.exit(-1)

data = np.loadtxt(sys.argv[1], usecols = (0, 1, 2))

clusters = fclusterdata(data, 50, criterion='maxclust')	#clusters data in the matrix data with a threshold of 50 - uses the maxclust method which ensures that the max distance between two clusters is r
#clusters = linkage(data)

centroids = getCentroids(data, clusters)
for c in centroids:
    print "%s %s %s" % (c[0], c[1], c[2])
#print "# of clusters formed: %s" % (np.unique(clusters).size)

