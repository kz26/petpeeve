#!/usr/bin/python
#Bria Connolly
#07/19/2011
#Removes x,y coordinates from a list based on x,y value criteria
#Arguments: input_file

import sys
if len(sys.argv) != 2:
  print "Usage: %s input_file" % (sys.argv[0])
  sys.exit(-1)

f = open(sys.argv[1], 'r')

for line in f:
  l = [int(x) for x in line.rstrip().split(" ")]
  lstr = line.rstrip().split(" ")
  if l[0] in range(24, 113) and l[1] in range(35,86) and l[2] in range(5,351):
    print " ".join(lstr)