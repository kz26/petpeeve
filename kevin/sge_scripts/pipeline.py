#!/usr/bin/python
# Writes SGE shell scripts to run pipeline on compute cluster
# Kevin Zhang
# Arguments: PET_data_dir script_output_dir master_script_output.sh
# Run petpeeve
# Run findpoints.py
# Run PointsInTumor.py
# Run CompareSeedPoints.py

import os, sys, shutil

if len(sys.argv) != 5:
    print "Usage: %s PET_data_dir PET_output_dir script_output_dir master_script_output.sh" % (sys.argv[0])
    sys.exit(-1)

if not os.path.exists(sys.argv[2]):
    os.mkdir(sys.argv[2])

scriptlist = [] #list that holds script filenames
bindir = os.getcwd()

if os.path.exists(sys.argv[2]): # clear output directory
    shutil.rmtree(sys.argv[2])

if not os.path.exists(sys.argv[3]):
    os.makedirs(sys.argv[3])


for case in os.listdir(sys.argv[1]): # loop through PET directories
    casepath = os.path.join(sys.argv[1], case)
    for ab in os.listdir(casepath): # loop through before/after
        subcasepath = os.path.join(casepath, ab)
          
        outputpath = os.path.join(sys.argv[2], case, ab) 
        if not os.path.exists(outputpath):
            os.makedirs(outputpath)

        if not os.path.exists(sys.argv[3]):
            os.makedirs(sys.argv[3])

        scriptfn = "%s_%s.sh" % (case, ab)
        scriptlist.append(scriptfn)
        numofslices = len(os.listdir(os.path.join(subcasepath, "PT")))
        f = open(os.path.join(sys.argv[3], scriptfn), 'w')
        f.write("#!/bin/bash\n")
        f.write("#$ -N %s\n" % (scriptfn))
        f.write("#$ -j y\n")
        f.write("#$ -o %s\n" % (os.path.join(outputpath, "run.log")))
        f.write("#$ -cwd\n")
        f.write("%s/utils/delslices.py %s %s 2\n" % (bindir, os.path.join(subcasepath, "PT"), os.path.join(outputpath, "PT_trimmed")))
        f.write("%s/petpeeve %s %s %s %s %s > %s\n" % (bindir, os.path.join(outputpath, "PT_trimmed"), os.path.join(outputpath, "output"), os.path.join(outputpath, "output_raw"), "5", "2", os.path.join(outputpath, "centroids.txt")))
        f.write("%s/utils/seedfix.py --offset %s %s > %s\n" % (bindir, os.path.join(outputpath, "centroids.txt"), os.path.join(outputpath, "PT_trimmed"), os.path.join(outputpath, "centroids-fixed.txt")))
        f.write("%s/sensitivity.py %s %s > %s\n" % (bindir, os.path.join(subcasepath, "objdata"), os.path.join(outputpath, "centroids-fixed.txt"), os.path.join(outputpath, "sensitivity-region.txt")))
        f.write("%s/sensitivity2.py %s %s > %s\n" % (bindir, os.path.join(subcasepath, "objdata"), os.path.join(outputpath, "output"), os.path.join(outputpath, "sensitivity-overlap.txt")))
        #f.write("%s/PointsInTumor.py %s %s > %s\n" % (bindir, os.path.join(outputpath, "centroids-fixed.txt"), os.path.join(subcasepath, "PT_manual_contours_bin"), os.path.join(outputpath, "points_in_tumor.txt")))
        #f.write("%s/CompareSeedPoints.py %s %s %s > %s\n" % (bindir, os.path.join(subcasepath, "seeds-fixed.txt"), os.path.join(outputpath, "centroids-fixed.txt"), "8", os.path.join(outputpath, "seed_compare.txt")))
        f.close()

mf = open(sys.argv[4], 'w')
mf.write("#!/bin/bash\n")
for s in scriptlist:
    mf.write("qsub %s\n" % (os.path.abspath(os.path.join(sys.argv[3], s))))
mf.close()
os.system("chmod +x %s" % (sys.argv[4]))

