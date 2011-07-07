#!/usr/bin/python
# Writes SGE shell scripts to run pipeline on compute cluster
# Kevin Zhang
# Arguments: PET_data_dir script_output_dir master_script_output.sh
# Run petpeeve
# Run findpoints.py
# Run PointsInTumor.py
# Run CompareSeedPoints.py

import os, sys

if len(sys.argv) != 5:
    print "Usage: %s PET_data_dir PET_output_dir script_output_dir master_script_output.sh" % (sys.argv[0])
    sys.exit(-1)

if not os.path.exists(sys.argv[2]):
    os.mkdir(sys.argv[2])

scriptlist = []

for case in os.listdir(sys.argv[1]):
    casepath = os.path.join(sys.argv[1], case)
    for ab in os.listdir(casepath):
        subcasepath = os.path.join(casepath, ab)
          
        outputpath = os.path.join(sys.argv[2], case, ab) 
        if not os.path.exists(outputpath):
            os.makedirs(outputpath)

        if not os.path.exists(sys.argv[3]):
            os.makedirs(sys.argv[3])

        scriptfn = "%s_%s.sh" % (case, ab)
        scriptlist.append(scriptfn)
        f = open(os.path.join(sys.argv[3], scriptfn), 'w')
        f.write("#!/bin/bash\n")
        f.write("#$ -N %s\n" % (scriptfn))
        f.write("#$ -j y\n")
        f.write("#$ -o %s\n" % (os.path.join(outputpath, "run.log")))
        f.write("#$ -cwd\n")
        f.write("/home/zhangk/bin/petpeeve %s %s %s %s\n" % (os.path.join(subcasepath, "PT"), os.path.join(outputpath, "output"), os.path.join(outputpath, "output_mask"), "1"))
        f.write("/home/zhangk/bin/findpoints.py %s > %s\n" % (os.path.join(outputpath, "output"), os.path.join(outputpath, "found_points.txt")))
        f.write("/home/zhangk/bin/PointsInTumor.py %s %s > %s\n" % (os.path.join(outputpath, "found_points.txt"), os.path.join(subcasepath, "PT_manual_contours_bin"), os.path.join(outputpath, "points_in_tumor.txt")))
        f.write("/home/zhangk/bin/CompareSeedPoints.py %s %s %s > %s\n" % (os.path.join(subcasepath, "seeds-fixed.txt"), os.path.join(outputpath, "found_points.txt"), "5", os.path.join(outputpath, "seed_compare.txt")))
        f.close()

mf = open(sys.argv[4], 'w')
mf.write("#!/bin/bash\n")
for s in scriptlist:
    mf.write("qsub %s\n" % (os.path.abspath(os.path.join(sys.argv[3], s))))
mf.close()

