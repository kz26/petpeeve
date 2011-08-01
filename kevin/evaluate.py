#!/usr/bin/python
# Calculates the number of false positives and true positives
# Input: labeled reference files, labeled output data file (use ImgSeriesReader.py for both)
# Output: line 1 = # of TPs, line 2 = # of FPs

import os, sys, pickle

def mode(mylist):
    count = {}
    for i in mylist:
        if i not in count.keys():
            count[i] = 0
        count[i] += 1
    freq = []
    for k in count.keys():
        freq.append((k, count[k]))
    return sorted(freq, key=lambda x: x[1])[-1][0]

if len(sys.argv) != 3:
    print "Usage: %s labeled_mask_data labeled_output_data" % (sys.argv[0])
    sys.exit(-1)

filepath = os.path.split(os.path.realpath(sys.argv[1]))[0]
print filepath

f = open(sys.argv[1], 'r')
refdata = pickle.load(f)
f.close()

f = open(sys.argv[2], 'r')
inputdata = pickle.load(f)
f.close()


refdata_pxcount = {}
inputdata_pxcount = {}
for label in refdata.keys():
    refdata_pxcount[label] = []

for label in inputdata.keys():
    inputdata_pxcount[label] = []
    for point in inputdata[label]:
        for reflabel in refdata.keys():
            if point in refdata[reflabel]:
                inputdata_pxcount[label].append(reflabel) # tracks all the lesion objects that pixels in this object belong to
                refdata_pxcount[reflabel].append(label) # tracks all the detected objects that fall within this lesion
            
tp_lesions = []
fp_count = 0
for label in inputdata_pxcount.keys():
    if float(len(inputdata_pxcount[label])) / len(inputdata[label]) < 0.25: # minimum overlap threshold
        fp_count += 1
        continue
    for obj in inputdata_pxcount[label]:
        if obj not in tp_lesions:
            tp_lesions.append(obj)
print "TP: %s" % (len(tp_lesions))
print "FP: %s" % (fp_count)
print "Sensitivity: %s" % (float(len(tp_lesions)) / len(refdata.keys()))


