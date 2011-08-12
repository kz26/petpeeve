#!/usr/bin/python
# Uses distance-based slice-by-slice hierarchical clustering to eliminate
# superfluous objects

import os, sys, dicom
import numpy as np
from scipy.cluster.hierarchy import fclusterdata

def read_slice(fn):
    ds = dicom.read_file(fn)
    objects = {}
    for i in range(0, ds.pixel_array.shape[0]):
        for j in range(0, ds.pixel_array.shape[1]):
            if ds.pixel_array[i][j] == 0: continue
            if ds.pixel_array[i][j] not in objects.keys():
                objects[ds.pixel_array[i][j]] = []
            objects[ds.pixel_array[i][j]].append((i, j))
    return (objects, ds)

# obj must be a list
def findCentroid(obj):
    dim = len(obj[0])
    totals = np.zeros(2, float)
    for point in obj:
        for i in range(0, len(point)):
            totals[i] += point[i]
    return [int(round(x)) for x in list(totals / len(obj))]

def clusterObs(objs):
    return fclusterdata(objs, 15, criterion='distance')

def findLargestObject(data, obj_ids):
    return sorted([(x, len(data[x])) for x in obj_ids], key=lambda x: x[1])[-1][0]

def writeNewSlice(dcmf, keep_ids, fn, output_dir):
    pxdata = dcmf.pixel_array
    for i in range(0, pxdata.shape[0]):
        for j in range(0, pxdata.shape[1]):
            if pxdata[i][j] not in keep_ids:
                pxdata[i][j] = 0
    dcmf.PixelData = pxdata.tostring()
    dcmf.save_as(os.path.join(output_dir, fn))

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print "Usage: %s input_directory output_directory" % (sys.argv[0])
        sys.exit(-1)

    if not os.path.exists(sys.argv[2]):
        os.mkdir(sys.argv[2])

    for file in sorted(os.listdir(sys.argv[1])):
        data = read_slice(os.path.join(sys.argv[1], file))
        slice = data[0]
        sys.stderr.write(file + "\n")
        sys.stderr.write("    %s objects read\n" % (len(slice.keys())))
        final_obj_ids = []

        if len(slice.keys()) > 1:
            centroids = []
            for object in sorted(slice.keys()):
                centroids.append(findCentroid(slice[object]))
            clusters = clusterObs(centroids)
        
            for clustnum in set(clusters):
                member_ids = [sorted(slice.keys())[x] for x in range(0, len(clusters)) if clusters[x] == clustnum]
                final_obj_ids.append(findLargestObject(slice, member_ids))

        else:
            final_obj_ids.append(slice.keys()[0])
        sys.stderr.write("    %s objects kept\n" % (len(final_obj_ids)))
        writeNewSlice(data[1], final_obj_ids, file, sys.argv[2]) 
