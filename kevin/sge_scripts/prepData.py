#!/usr/bin/python
# Preps the original dataset for processing
# Generates labeled object data for each case
# This script must be run from the directory containing the supporting programs
# Kevin Zhang

import os, sys, shutil

if len(sys.argv) != 4:
    print "Usage: %s PET_data_dir script_output_dir master_script_output.sh" % (sys.argv[0])
    sys.exit(-1)

scriptlist = [] #list that holds script filenames
bindir = os.getcwd()

if not os.path.exists(sys.argv[2]):
    os.makedirs(sys.argv[2])

for case in os.listdir(sys.argv[1]): # loop through PET directories
    casepath = os.path.join(sys.argv[1], case)
    if len(os.listdir(casepath)) == 0:
            shutil.rmtree(casepath)
            continue
    for ab in os.listdir(casepath): # loop through before/after
        subcasepath = os.path.join(casepath, ab)
        DeleteSubcase = False
        if not os.path.exists(os.path.join(subcasepath, "PT")): DeleteSubcase = True
        if not os.path.exists(os.path.join(subcasepath, "PT_manual_contours_bin")): DeleteSubcase = True
        if not os.path.exists(os.path.join(subcasepath, "PT_manual_contours_bin_labeled")): DeleteSubcase = True
        if not os.path.exists(os.path.join(subcasepath, "seeds.dat")): DeleteSubcase = True
        if DeleteSubcase:
            shutil.rmtree(subcasepath)
            continue
        scriptfn = "%s_%s.sh" % (case, ab)
        scriptlist.append(scriptfn)
        f = open(os.path.join(sys.argv[2], scriptfn), 'w')
        f.write("#!/bin/bash\n")
        f.write("#$ -N %s\n" % (scriptfn))
        f.write("#$ -j y\n")
        f.write("#$ -cwd\n")
        f.write("%s/utils/ptrename.py %s\n" % (bindir, os.path.join(subcasepath, "PT_manual_contours_bin")))
        f.write("%s/utils/ptrename.py %s\n" % (bindir, os.path.join(subcasepath, "PT_manual_contours_bin_labeled")))
        f.write("%s/utils/seedfix.py --axis %s %s > %s\n" % (bindir, os.path.join(subcasepath, "seeds.dat"), os.path.join(subcasepath, "PT"), os.path.join(subcasepath, "seeds-fixed.txt")))
        f.write("%s/ImgSeriesReader.py %s %s\n" % (bindir, os.path.join(subcasepath, "PT_manual_contours_bin_labeled"), os.path.join(subcasepath, "objdata")))
        f.close()

mf = open(sys.argv[3], 'w')
mf.write("#!/bin/bash\n")
for s in scriptlist:
    mf.write("qsub %s\n" % (os.path.abspath(os.path.join(sys.argv[2], s))))
mf.close()
os.system("chmod +x %s" % (sys.argv[3]))

