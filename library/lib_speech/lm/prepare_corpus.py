#!/usr/bin/python
import fnmatch
import os
import sys


if __name__ == '__main__':
  if len(sys.argv) != 3:
    print("Usage: " + sys.argv[0] + " speechsentencefile outputfile\n")
    raise
  
  filedata = []
  outputfile = sys.argv[-1]
  in1 = file(sys.argv[1],'r')
  
  lines1 = in1.readlines()
  
  combinedlines = ['<s> '+ x.lower().strip() +' </s>\n' for x in lines1]
  
  out = file(outputfile,'w')
  out.writelines(combinedlines)
  
    
