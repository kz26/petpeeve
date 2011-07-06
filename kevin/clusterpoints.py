#!/usr/bin/python
# Reads in a list of generated candidate points and performs k-means clustering
# Prints out the centroids of each cluster

import sys
import numpy as np
import scipy as sp

if len(sys.argv) != 2:
    print "Usage: %s input_data_file" % (sys.argv[0])
    sys.exit(-1)

data = np.genfromtxt(sys.argv[1])
data = sp.cluster.vq.whiten(data)

clusters = sp.cluster.vq.kmeans(data, 10)

print clusters[0]
