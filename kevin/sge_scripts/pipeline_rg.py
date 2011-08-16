#!/usr/bin/python
# Writes SGE shell scripts to run RG-baseline pipeline on compute cluster
# Kevin Zhang
# 08/16/2011

import os, sys, shutil

if len(sys.argv) != 7:
    print "Usage: %s PET_data_dir PET_output_dir script_output_dir master_script_output.sh sigma convex_height" % (sys.argv[0])
    sys.exit(-1)

sigma = int(sys.argv[-2])
convex_height = int(sys.argv[-1])

if not os.path.exists(sys.argv[2]):
    os.mkdir(sys.argv[2])

scriptlist = [] #list that holds script filenames
bindir = os.getcwd()

if os.path.exists(sys.argv[2]): # clear output directory
    shutil.rmtree(sys.argv[2])

if not os.path.exists(sys.argv[3]):
    os.makedirs(sys.argv[3])


for case in sorted(os.listdir(sys.argv[1])): # loop through PET directories
    casepath = os.path.join(sys.argv[1], case)
    for ab in sorted(os.listdir(casepath)): # loop through before/after
        subcasepath = os.path.join(casepath, ab)
          
        outputpath = os.path.join(sys.argv[2], case, ab) 
        if not os.path.exists(outputpath):
            os.makedirs(outputpath)

        if not os.path.exists(sys.argv[3]):
            os.makedirs(sys.argv[3])

        scriptfn = "%s_%s.sh" % (case, ab)
        scriptlist.append(scriptfn)
        #numofslices = len(os.listdir(os.path.join(subcasepath, "PT")))
        f = open(os.path.join(sys.argv[3], scriptfn), 'w')
        f.write("#!/bin/bash\n")
        f.write("#$ -N %s\n" % (scriptfn))
        f.write("#$ -j y\n")
        f.write("#$ -o %s\n" % (os.path.join(outputpath, "run.log")))
        f.write("#$ -cwd\n")
        f.write("%s/utils/delslices.py %s %s 4\n" % (bindir, os.path.join(subcasepath, "PT"), os.path.join(outputpath, "PT_trimmed")))
        f.write("%s/petpeeve %s %s %s %s %s %s %s > %s\n" % (bindir, os.path.join(outputpath, "PT_trimmed"), os.path.join(outputpath, "output"), os.path.join(outputpath, "output_raw1"), os.path.join(outputpath, "output_raw2"), str(sigma), str(convex_height), "2", os.path.join(outputpath, "centroids.txt")))
        f.write("%s/utils/seedfix.py --offset %s %s > %s\n" % (bindir, os.path.join(outputpath, "centroids.txt"), os.path.join(outputpath, "PT_trimmed"), os.path.join(outputpath, "centroids-fixed.txt")))
        #f.write("%s/sensitivity.py %s %s > %s\n" % (bindir, os.path.join(subcasepath, "objdata"), os.path.join(outputpath, "centroids-fixed.txt"), os.path.join(outputpath, "sensitivity-region.txt")))
        #f.write("%s/sensitivity2.py %s %s > %s\n" % (bindir, os.path.join(subcasepath, "objdata"), os.path.join(outputpath, "output"), os.path.join(outputpath, "sensitivity-overlap.txt")))
        f.write("%s/ImgSeriesReader.py %s %s\n" % (bindir, os.path.join(outputpath, "output"), os.path.join(outputpath, "output_data")))
        f.write("%s/evaluate.py %s %s > %s\n" % (bindir, os.path.join(subcasepath, "objdata"), os.path.join(outputpath, "output_data"), os.path.join(outputpath, "results.txt")))
        #f.write("%s/PointsInTumor.py %s %s > %s\n" % (bindir, os.path.join(outputpath, "centroids-fixed.txt"), os.path.join(subcasepath, "PT_manual_contours_bin"), os.path.join(outputpath, "points_in_tumor.txt")))
        #f.write("%s/CompareSeedPoints.py %s %s %s > %s\n" % (bindir, os.path.join(subcasepath, "seeds-fixed.txt"), os.path.join(outputpath, "centroids-fixed.txt"), "8", os.path.join(outputpath, "seed_compare.txt")))
        f.close()

mf = open(sys.argv[4], 'w')
mf.write("#!/bin/bash\n")
for s in scriptlist:
    mf.write("qsub %s\n" % (os.path.abspath(os.path.join(sys.argv[3], s))))
    mf.write("sleep 0.2\n")
mf.close()
os.system("chmod +x %s" % (sys.argv[4]))

