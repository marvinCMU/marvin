#!/usr/bin/python
import fnmatch
import os
import sys


if __name__ == '__main__':
  if len(sys.argv) != 4:
"""
    print("Usage: "+ sys.argv[0] + " <rootdir> <outfile> <extension>\n")
    print("This program outputs a label file to be read by the face reco algorithm.\n")
    print("<rootdir> is a directory structure with images. The last directory is considered \n")
    print("to be the label for each image in that directory. If that directory has an integer\n")
    print("suffix, then we ignore it. Eg 'denver.1' will be assigned the label 'denver'. \n")
    print("The <outfile> is saved relative to <rootdir>.\n")
    print("<extension> is the extension of the images to label (eg, 'jpg').")
"""
  rootfolder = sys.argv[1]
  outputpath = sys.argv[2]
  extension = sys.argv[3]
  
  matches = []
  try:
    for root, dirnames, filenames in os.walk(rootfolder):
      for filename in fnmatch.filter(filenames, '*.'+extension):
          matches.append(os.path.join(os.path.abspath(root), filename))
  except:
    print("Problem accessing root folder " + rootfolder + "?\n")
    
  out = file(rootfolder+'/'+outputpath,'w')
  for fn in matches:
    label = fn.split('/')[-2] #get the parent directory of the image
    label = label.split('.')[0] #ignore integer suffix: denver.1 --> denver
    line = fn+';'+label
    out.write(line+'\n')
    print(line)
  
        
  
    
