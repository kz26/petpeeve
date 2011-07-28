#!/usr/bin/python
# Kevin Zhang
# 07/27/2011
# Calculates the total sensitivity and prints the number of false positives / true positives
# Input is from: find -name results.txt | sort | xargs cat > some_file

import sys, re
import numpy as np

if len(sys.argv) != 2:
    print "Usage: %s input_file" % (sys.argv[0])
    sys.exit(-1)

#sens_pat = re.compile("Sensitivity: ([0-9]+)")
#tp_pat = re.compile("TP: ([0-9]+)")
#fp_pat = re.compile("FP: ([0-9]+)")

f = open(sys.argv[1], 'r')
lines = []
for line in f:
    lines.append(line.rstrip())
f.close()

sens = [float(lines[i].split(" ")[1]) for i in range(1, len(lines), 4)]
tp = [int(lines[i].split(" ")[1]) for i in range(2, len(lines), 4)]
fp = [int(lines[i].split(" ")[1]) for i in range(3, len(lines), 4)]

print "Total sensitivity: %s" % (np.mean(sens))
print "Total TPs: %s" % (sum(tp))
print "Total FPs: %s" % (sum(fp))
