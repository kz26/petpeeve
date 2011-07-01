#!/usr/bin/python
# Removes DCM extension from all DCM files in a directory

import os, sys

if len(sys.argv) != 2:
    print "Usage: %s some_directory" % (sys.argv[0])
    sys.exit(-1)

os.chdir(sys.argv[1])
files = os.listdir('.')
for f in files:
    basefn, ext = os.path.splitext(f)
    if ext.lower() == ".dcm": 
        os.rename(f, basefn)
        print "Renamed %s to %s" % (f, basefn)
