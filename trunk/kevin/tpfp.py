#!/usr/bin/python
# Kevin Zhang
# 07/13/2011
# Print the total number of false positives / true positives
# Input is from: find -name points_in_tumor.txt | sort | xargs cat | grep "#"

import sys, re

if len(sys.argv) != 2:
    print "Usage: %s input_file" % (sys.argv[0])
    sys.exit(-1)

tp_pat = re.compile("# of TPs: ([0-9]+)")
fp_pat = re.compile("# of FPs: ([0-9]+)")

tpcount = 0
fpcount = 0
for line in open(sys.argv[1], 'r'):
    tpmatch = tp_pat.match(line)
    fpmatch = fp_pat.match(line)
    if tpmatch:
        tpcount += int(tpmatch.group(1))
    elif fpmatch:
        fpcount += int(fpmatch.group(1))

print "Total TPs: %s" % (tpcount)
print "Total FPs: %s" % (fpcount)
