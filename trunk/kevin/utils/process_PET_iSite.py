#!/usr/bin/python
# Preps PET_iSite directory for processing on compute cluster

# Arguments: PET_iSite_dir

import os, sys, shutil
if len(sys.argv) != 2:
    print "Usage: %s PET_iSite_dir" % (sys.argv[0])
    sys.exit(-1)

for case in range(1, 52):
    for subcase in ["before", "after"]:
        rootpath = os.path.join(sys.argv[1], "PET%03d")
        basepath = os.path.join(sys.argv[1], "PET%03d" % (case), subcase)
        if not os.path.exists(basepath): continue
        deletedir = False
        #shutil.rmtree(os.path.join(basepath, "best_threshold_logs"))
        #shutil.rmtree(os.path.join(basepath, "descending_threshold_logs"))
        #shutil.rmtree(os.path.join(basepath, "mrdmtsnm"))
        #shutil.rmtree(os.path.join(basepath, "PT_abras_workaround"))
        #shutil.rmtree(os.path.join(basepath, "PT_manual_contours_bin_labeled"))
        #shutil.rmtree(os.path.join(basepath, "seeds_dat_singles"))
        #shutil.rmtree(os.path.join(basepath, "seeds_xml_singles"))
        #shutil.rmtree(os.path.join(basepath, "stopjump"))
        if not os.path.exists(os.path.join(basepath, "PT")): deletedir = True
        if not os.path.exists(os.path.join(basepath, "PT_manual_contours_bin")): deletedir = True
        if not os.path.exists(os.path.join(basepath, "seeds.dat")): deletedir = True
        if deletedir:
            print "Removing %s..." % ()
            shutil.rmtree(basepath)
            continue
        numslices = len(os.listdir(os.path.join(basepath, "PT")))
        os.system("python ptrename.py %s" % (os.path.join(basepath, "PT_manual_contours_bin")))
        os.system("python seedreverse.py %s %s > %s" % (os.path.join(basepath, "seeds.dat"), numslices, os.path.join(basepath, "seeds-fixed.txt")))

        
