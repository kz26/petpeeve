#!/usr/bin/python

from numpy import *
import matplotlib.pyplot as plt
import os, sys, dicom

if len(sys.argv) != 3:
    print "Usage: %s DCM_input_directory DCM_binmask_input_directory lesion_output_filename.png normal_output_filename.png" % (sys.argv[0])
    sys.exit(-1)

lesion_pixvals = []
normal_pixvals = []

files = os.listdir(sys.argv[1])
for f in files:
    inputfn = os.path.join(sys.argv[1], f)
    maskfn = os.path.join(sys.argv[2], f)
    inputdcm = dicom.read_file(inputfn)
    maskdcm = dicom.read_file(maskfn)

    input_arr = inputdcm.pixel_array
    mask_arr = maskdcm.pixel_array

    rows = input_arr.shape[0]
    cols = input_arr.shape[1]
    if rows != mask_arr.shape[0] or rows != mask_arr.shape[1]:
        print "Skipping %s, dimensions of input image and mask do not match" % (f)
        continue

    for r in range(0, rows):
        for c in range(0, cols):
            if mask_arr[r][c] != 0: lesion_pixvals.append(input_arr[r][c])
            elif input_arr[r][c] > 800: normal_pixvals.append(input_arr[r][c])
            #else: normal_pixvals.append(input_arr[r][c]) 
    print "Processed %s" % (f)

print ""

plt.figure()
n1, bins1, patches1 = plt.hist(lesion_pixvals, bins=10, facecolor='green')
plt.xlabel("Pixel intensity")
plt.ylabel("# of pixels")
plt.title("Lesion")
plt.savefig("lesion.png")

print "Lesion - min: %s" % (min(lesion_pixvals))
print "Lesion - max: %s" % (max(lesion_pixvals))
print "Lesion - mean: %s" % (mean(lesion_pixvals))
print "Lesion - median: %s" % (median(lesion_pixvals))

print ""

plt.figure()
n1, bins1, patches1 = plt.hist(normal_pixvals, bins=10, range=[0,max(lesion_pixvals)])
plt.xlabel("Pixel intensity")
plt.ylabel("# of pixels")
plt.title("Normal")
plt.savefig("normal.png")
print "Normal - min: %s" % (min(normal_pixvals))
print "Normal - max: %s" % (max(normal_pixvals))
print "Normal - mean: %s" % (mean(normal_pixvals))
print "Normal - median: %s" % (median(normal_pixvals))


