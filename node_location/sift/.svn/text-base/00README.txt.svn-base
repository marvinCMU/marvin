This is a SIFT feature extraction program based on Lowe's implementation.
Since it is not available in public, please be careful about distributing 
codes here.


Compilation:

  [Short] Just run "make" in the directory. Use "Makefile.gcc" for gcc and
  "Makefile.icc" for icc. For example,

    $ make -f Makefile.gcc

  [Long] Two makefiles are provided in the directory. Makefile.gcc is for
  GCC (g++) v4.4, and Makefile.icc is for Intel C++ compiler (icpc) v12.
  Since icpc generates approximately x2 faster binary, it is strongly 
  recommended to install icpc (it is free for non-commercial use on Linux).
  Visit

  http://software.intel.com/en-us/articles/non-commercial-software-development/

  and get "Intel C++ Composer XE 2011 for Linux."

  As well, you need to install some libraries.
  - libcv-dev (2.0.0-3ubuntu2)
  - libhighgui-dev (2.0.0-3ubuntu2)
  - libtbb-dev (2.2+r009-1) (to enable multi-threading below)

  To enable multi-threading, define ENABLE_TBB and link libtbb.so. For example,
  modify Makefile

    CFLAGS += -DENABLE_TBB
    CXXFLAGS += -DENABLE_TBB
    LDFLAGS += -ltbb

  This is simply done by

    $ make TBB=1 -f Makefile.icc (or Makefile.gcc)
 
  See the Makefiles for detail.


Usage:

  (1) Extract keypoints from a single image

      $ ./img2key -i foo.png -o foo.key

  (2) Extract keypoints from multiple images (0000.png to 0999.png).

      $ ./img2key -i %04d.png -o %04d.key -b 0 -e 1000

  (3) Extract keypoints from multiple images (0000.png, 0010.png, ... 0990.png).

      $ ./img2key -i %04d.png -o %04d.key -b 0 -e 1000 -s 10


Sample code

  All the SIFT-related codes are in "sift.h". The below is a simple main()
  function which describes how to use it.


  #include <iostream>
  #include "sift.h"
  
  int main(int argc, char * argv[]) {
    // init tbb
    tbb::task_scheduler_init tbb_init;
   
    // 32bit floating point grayscale image
    IplImage * gray = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
    IplImage * gray32f = cvCreateImage(cvGetSize(gray), IPL_DEPTH_32F, 1);
    cvConvertScale(gray, gray32f);
    
    // init the detector (you can reuse this for images in the same size)
    SIFT::detector_t sd;
    sd.init(gray32f->width, gray32f->height);
   
    // detect
    std::vector<SIFT::keypoint_t> result;
    sd.detect(gray32f, &result);
   
    // dump
    std::cout << result.size() << " " << SIFT::keypoint_t::dim() << "\n";
    for(unsigned int i=0 ; i<result.size() ; i++) {
      std::cout << result[i].to_string();
    }

    return 0;
  }

