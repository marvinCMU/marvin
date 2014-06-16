#include <cstdio>
#include <fstream>
#include <unistd.h>
#include "sift.h"

void load_keys(const char * filename, std::vector<SIFT::keypoint_t> * key) {
  std::ifstream ifs(filename);
  for(std::string line ; std::getline(ifs, line) ; ) {
    std::istringstream iss(line);
    SIFT::keypoint_t k;
    iss >> k.y >> k.x >> k.scale >> k.orientation;
    for(int i=0 ; i<SIFT::keypoint_t::dim() ; i++) {
      iss >> k.descriptor[i];
    }
    key->push_back(k);
  }
  ifs.close();
}

int main(int argc, char ** argv) {
  if(argc != 3) {
    fprintf(stderr, 
            "\n"
            "%s img key\n"
            "\n"
            "  img : image file\n"
            "  key : key file in Matlab format (given by img2key -m)\n"
            "\n"
            , argv[0]);
    return 1;
  }

  // load image
  IplImage * gray = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE);
  IplImage * bgr = cvCreateImage(cvGetSize(gray), IPL_DEPTH_8U, 3);
  cvCvtColor(gray, bgr, CV_GRAY2BGR);

  // load keys
  std::vector<SIFT::keypoint_t> key;
  load_keys(argv[2], &key);

  // render
  for(unsigned int i=0 ; i<key.size() ; i++) {
    key[i].draw(bgr, false);
  }

  // show
  cvNamedWindow("main");
  cvShowImage("main", bgr);
  cvWaitKey(0);
  cvDestroyWindow("main");

  return 0;
}
