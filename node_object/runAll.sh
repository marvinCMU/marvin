
#!/bin/bash

# initialize environment
#./init

# run gui
#./gui &

# run vision system

#matlab -nodesktop -nosplash -r "lazyObjRecognizer" #> /home/mmfps/mmfps/mmfpspkg/data_tmp/vision.log &

#echo $! > matlab_pid 

# run speech recognition system
#java -jar vd.jar


mkdir ../data_tmp/vision
mkdir ../data_tmp/vision_processed
rm ../data_tmp/vision/*
rm ../data_tmp/vision_processed/*

matlab -nodesktop -nosplash -r "simp_object_detect"
