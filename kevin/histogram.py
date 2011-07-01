#!/usr/bin/python
# Kevin Zhang
# Computes histogram and basic statistics about lesion and normal region pixel values

from numpy import *
import matplotlib.pyplot as plt
import os, sys, dicom

if len(sys.argv) != 6:
    print "Usage: %s DCM_input_dir DCM_lesion_mask_dir DCM_body_mask_dir lesion_output.png normal_output.png" % (sys.argv[0])
    sys.exit(-1)

lesion_pixvals = []
normal_pixvals = []

files = os.listdir(sys.argv[1])
for f in files:
    inputfn = os.path.join(sys.argv[1], f) # original input DCM file
    lesionmaskfn = os.path.join(sys.argv[2], f) # lesion mask file
    bodymaskfn = os.path.join(sys.argv[3], f) # body mask file
    inputdcm = dicom.read_file(inputfn)
    lesionmaskdcm = dicom.read_file(lesionmaskfn)
    bodymaskdcm = dicom.read_file(bodymaskfn)

    input_arr = inputdcm.pixel_array
    lesionmask_arr = lesionmaskdcm.pixel_array
    bodymask_arr = bodymaskdcm.pixel_array

    rows = input_arr.shape[0]
    cols = input_arr.shape[1]
    if rows != lesionmask_arr.shape[0] or rows != lesionmask_arr.shape[1]:
        print "Skipping %s, dimensions of input image and mask do not match" % (f)
        continue

    for r in range(0, rows):
        for c in range(0, cols):
            if lesionmask_arr[r][c] != 0: lesion_pixvals.append(input_arr[r][c]) # is pixel part of lesion?
            elif bodymask_arr[r][c] == 0: normal_pixvals.append(input_arr[r][c]) # is pixel part of body?
    print "Processed %s" % (f)

print ""

plt.figure()
n1, bins1, patches1 = plt.hist(lesion_pixvals, bins=10, facecolor='green')
plt.xlabel("Pixel intensity")
plt.ylabel("# of pixels")
plt.title("Lesion")
plt.savefig(sys.argv[4])

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
plt.savefig(sys.argv[5])
print "Normal - min: %s" % (min(normal_pixvals))
print "Normal - max: %s" % (max(normal_pixvals))
print "Normal - mean: %s" % (mean(normal_pixvals))
print "Normal - median: %s" % (median(normal_pixvals))


