#!/usr/bin/python

# Renames PET files in PT_manual_contours_bin in reverse order
# Input file name format: ####
# Output file name format: Img001_XXXX (where XXXX is # files - ####)

import os, sys, re

if len(sys.argv) != 2:
    print "Usage: %s some_directory" % (sys.argv[0])
    sys.exit(-1)

os.chdir(sys.argv[1])

files = os.listdir(".")

pat_orig = re.compile(r"[0-9]{4}")
for f in files:
    if pat_orig.match(f):
        newnum = abs(len(files) - int(f)) + 1 
        newfn = "Img001_%04d" % (newnum)
        os.rename(f, newfn)
        print "Renamed %s to %s" % (f, newfn)
