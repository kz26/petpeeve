#!/usr/bin/python
# Python implementation of DBSCAN
# Kevin Zhang
# 08/15/2011

import os, sys, math
import numpy as np

class Point:
    def __init__(self, coords):
        self.point = coords
        self.clid = None
        self.visited = False
    def __repr__(self):
        return " ".join([str(x) for x in self.point])

def readData(fn):
    points = []
    f = open(fn, 'r')
    for line in f:
        points.append(Point([int(x) for x in line.rstrip().split(" ")][:3]))
    return points

def getDistance(point1, point2):
    p1 = np.array(point1.point)
    p2 = np.array(point2.point)
    return math.sqrt(np.sum([math.pow(i, 2) for i in p1 - p2]))
    
def getNeighbors(point, pointset, eps):
    neighbors = []
    for p in pointset:
        if p.point != point.point and getDistance(p, point) <= eps:
            neighbors.append(p)
    return neighbors
    
def expandCluster(p, n, c, eps, minpts):
    #print "in expandCluster"
    c.append(p)
    for point in n:
        if point.visited: continue # point already clustered or marked as noise
        else:
            point.visited = True
        nprime = getNeighbors(point, n, eps)
        if len(nprime) >= minpts:
            expandCluster(point, nprime, c, eps, minpts)
        
def dbscan(data, eps, minpts):
    #print "in dbscan"
    clusters = []
    for p in data:
        if p.visited: continue
        p.visited = True
        n = getNeighbors(p, data, eps)
        if len(n) >= minpts:
            clusters.append([])
            expandCluster(p, n, clusters[-1], eps, minpts)
    return clusters

def getClusterCentroid(pointlist):
    totals = np.zeros(len(pointlist[0].point))
    for p in pointlist:
        totals += p.point
    return list(np.round(totals / len(pointlist)).astype(int))


if len(sys.argv) != 4:
    sys.stderr.write("Usage: %s input_file epsilon min_points\n" % (sys.argv[0]))
    sys.exit(-1)

clusters = dbscan(readData(sys.argv[1]), int(sys.argv[2]), int(sys.argv[3]))
#for cluster in clusters:
#    print " ".join([str(x) for x in getClusterCentroid(cluster)])

# uncomment everything below if doing 2D clustering
flatclusters = []
for cluster in clusters:
    for point in cluster:
        flatclusters.append(Point(point.point)) # reset visited state

clusters3 = dbscan(flatclusters, 10, 2)
for cluster in clusters3:
    #print cluster
    print " ".join([str(x) for x in getClusterCentroid(cluster)])
