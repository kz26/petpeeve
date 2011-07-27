#!/usr/bin/python
# Calculates the number of false positives and true positives
# Input: labeled reference files, labeled output data file (use ImgSeriesReader.py for both)
# Output: line 1 = # of TPs, line 2 = # of FPs

import os, sys, pickle
from scipy import stats

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
    inputdata_pxcount[label] = 0
    for point in inputdata[label]:
        for reflabel in refdata.keys():
            if point in refdata[reflabel]:
                inputdata_pxcount[label] += 1
                refdata_pxcount[reflabel].append(label)
            
ref_detected = 0
for label in refdata_pxcount.keys():
    inputlabels_mode = int(stats.mode(refdata_pxcount[label])[0][0])
    input_pct = float(inputdata_pxcount[inputlabels_mode]) / len(inputdata[inputlabels_mode])
    if (float(len(refdata_pxcount[label])) / len(refdata[label]) >= 0.5) or input_pct >= 0.5:
        ref_detected += 1

num_tp = 0
for label in inputdata_pxcount.keys():
    if float(inputdata_pxcount[label]) / len(inputdata[label]) >= 0.5:
        num_tp += 1

print "Sensitivity: %s" % (round(float(ref_detected) / len(refdata.keys()), 2) * 100)
print "TP: %s" % (num_tp)
print "FP: %s" % (len(inputdata.keys()) - num_tp)
