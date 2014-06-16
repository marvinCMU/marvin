#!/usr/bin/python
import fnmatch
import os
import sys


if __name__ == '__main__':
  if len(sys.argv) != 4:
    print("Usage: " + sys.argv[0] + " file1 file2 outputfile\n")
    raise
  
  filedata = []
  outputfile = sys.argv[-1]
  in1 = file(sys.argv[1],'r')
  in2 = file(sys.argv[2],'r')
  
  lines1 = in1.readlines()
  lines2 = in2.readlines()
  
  combinedlines = [x.strip() + " " + y.strip() + '\n' for x in lines1 for y in lines2]
  
  out = file(outputfile,'w')
  out.writelines(combinedlines)
  
    
