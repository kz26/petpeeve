#!/usr/bin/python
# Reads in a list of generated candidate points and performs k-means clustering
# Prints out the centroids of each cluster

import sys
import numpy as np
import scipy as sp
from scipy.cluster.vq import whiten, kmeans2
from scipy.cluster.hierarchy import linkage

if len(sys.argv) != 2:
    print "Usage: %s input_data_file" % (sys.argv[0])
    sys.exit(-1)

data = np.genfromtxt(sys.argv[1])
#data = whiten(data)

clusters = kmeans2(data, 20, minit='points')
#clusters = linkage(data)

print clusters

