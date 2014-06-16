#include <cstdio>
#include <fstream>
#include <unistd.h>
#include "sift.h"

void usage(int argc, char * argv[]) {
  fprintf(stderr,
          "\n"
          "%s -i src_%%04d.png -o out_%%04d.key -b BEGIN -e END [options]\n"
          "%s -l list.txt [options]\n"
          "\n"
          "Options:\n"
          "  -s step : Step size (default 1)\n"
          "  -S : Scale the image size (default 1.0. use 2.0 to double, 0.5 to halve the size)\n"
          "  -w : Render the keys on the image\n"
          "  -d filename_fmt : Save the image shown by -w\n"
          "  -m : Save keys in Matlab format\n"
		  "  -M : Use mask image to define ROI (only works in list mode)\n"
          "\n"
          "Example:\n"
          "\n"
          "  $ %s -i src.png -o out.key\n"
          "  => make out.key from src.png\n"
          "\n"
          "  $ %s -i %%04d.png -o %%04d.key -b 0 -e 1000\n"
          "  => make 0000.key from 0000.png, ... 0999.key from 0999.png.\n"
          "\n"
          "  $ %s -i %%04d.png -o %%04d.key -b 0 -e 1000 -s 2\n"
          "  => make 0000.key from 0000.png, 0002.key from 0002.png, ... 0998.key from 0998.png.\n"
          "\n"
          "  $ %s -l list.txt\n"
          "  => Each line of list.txt provides an input image and a key filename separated by white spaces (and lines starting with # will be skipped)\n"
		  " If -M was used, list.txt should contain a mask image on each line as well."
          "\n",
          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0],
          argv[0]
          );
}

void load_list(const char * filename,
               std::vector<std::string> * ifnames,
               std::vector<std::string> * ofnames, 
			   std::vector<std::string> * mfnames = NULL) {
  ifnames->clear();
  ofnames->clear();

  std::ifstream ifs(filename);
  for(std::string line ; std::getline(ifs, line) ; ) {
    if(line.empty() || line[0] == '#') {
      continue;
    }

    std::istringstream iss(line);
    std::string s1, s2, s3;
    iss >> s1 >> s2;
    ifnames->push_back(s1);
    ofnames->push_back(s2);
	if (mfnames != NULL) {
		iss >> s3;
		mfnames->push_back(s3);
	}
  }
  ifs.close();
}

void gen_list(const char * ifname_fmt,
              const char * ofname_fmt,
              int begin,
              int end,
              int step,
               std::vector<std::string> * ifnames,
               std::vector<std::string> * ofnames) {
  ifnames->clear();
  ofnames->clear();

  char ifname[PATH_MAX];
  char ofname[PATH_MAX];

  for(int i=begin ; i<end ; i+=step) {
    snprintf(ifname, sizeof(ifname), ifname_fmt, i);
    snprintf(ofname, sizeof(ofname), ofname_fmt, i);

    ifnames->push_back(ifname);
    ofnames->push_back(ofname);
  }
}


int main(int argc, char ** argv) {
  std::string LIST_FNAME;
  std::string INPUT_FNAME;
  std::string KEY_FNAME;
  std::string DUMP_FNAME;
  int OPT_BEGIN = 0;
  int OPT_END = 1;
  int OPT_STEP = 1;
  double OPT_IMG_SCALE = 1;
  int OPT_DISPLAY_KEYS = 0;
  int OPT_MATLAB= 0;
  int OPT_MASK = 0;

  const static char * getopt_str = "l:i:o:b:e:d:ws:S:mMh?";
  int opt = getopt( argc, argv, getopt_str );
  while( -1 != opt ) {
    switch( opt ) {
    case 'i':
      INPUT_FNAME = optarg;
      break;
    case 'l':
      LIST_FNAME = optarg;
      break;
    case 'o':
      KEY_FNAME = optarg;
      break;
    case 'b':
      OPT_BEGIN = atoi(optarg);
      break;
    case 'e':
      OPT_END = atoi(optarg);
      break;
    case 's':
      OPT_STEP = atoi(optarg);
      break;
    case 'S':
      OPT_IMG_SCALE = atof(optarg);
      break;
    case 'm':
      OPT_MATLAB= 1;
      break;
	case 'M':
	  OPT_MASK = 1;
	  break;
    case 'w':
      OPT_DISPLAY_KEYS = 1;
      break;
    case 'd':
      DUMP_FNAME = optarg;
      break;
    default:
      usage(argc, argv);
      break;
    }
    opt = getopt( argc, argv, getopt_str );
  }

  std::vector<std::string> ifnames, ofnames, mfnames;
  if(LIST_FNAME.empty()) {
	if (OPT_MASK) {
		usage(argc, argv);
		fprintf(stderr, "\nERROR: Can't use mask mode (-M) without list mode (-l)\n\n");
		return 1;
	}
    if(INPUT_FNAME.empty() || KEY_FNAME.empty()) {
      usage(argc, argv);
      fprintf(stderr, "\nERROR: Either -l or '-i and -o' should be given.\n\n");
      return 1;
    } else if(OPT_BEGIN < 0 || OPT_END < 0 || OPT_END <= OPT_BEGIN || OPT_STEP <= 0) {
      usage(argc, argv);
      fprintf(stderr, "\nERROR: The range [%d:%d) with step %d not valid.\n\n", OPT_BEGIN, OPT_END, OPT_STEP);
      return 1;
    }

    gen_list(INPUT_FNAME.c_str(), KEY_FNAME.c_str(), OPT_BEGIN, OPT_END, OPT_STEP, &ifnames, &ofnames);
  } else {
	  if (OPT_MASK) {
		  load_list(LIST_FNAME.c_str(), &ifnames, &ofnames, &mfnames);
	  }
	  else {
		  load_list(LIST_FNAME.c_str(), &ifnames, &ofnames);
	  }

    
  }
	


#ifdef ENABLE_TBB
  // for the Intel TBB
  tbb::task_scheduler_init tbb_init;
#endif

  SIFT::detector_t sd;

  IplImage * gray = NULL;
  IplImage * gray_dbl = NULL;
  IplImage * gray32f = NULL;
  IplImage * mask = NULL;
  IplImage * mask_dbl = NULL;

  std::vector<SIFT::keypoint_t> result;

  bool is_first = true;
  for(unsigned int i=0 ; i<ifnames.size() ; i++) {

    if(gray) {
      cvReleaseImage(&gray);
    }

    fprintf(stderr, "%d %s => %s", i, ifnames[i].c_str(), ofnames[i].c_str());
    gray = cvLoadImage(ifnames[i].c_str(), CV_LOAD_IMAGE_GRAYSCALE);
    assert(NULL != gray);
	  
	if (OPT_MASK) {
		mask = cvLoadImage(mfnames[i].c_str(), CV_LOAD_IMAGE_GRAYSCALE);
		assert(NULL != mask);
	}  
	

    if(is_first) {
      if(OPT_IMG_SCALE != 1) {
        sd.coord_scale = 1.0 / OPT_IMG_SCALE;
        sd.init(gray->width * OPT_IMG_SCALE, gray->height * OPT_IMG_SCALE);
        gray_dbl = cvCreateImage(cvSize(gray->width * OPT_IMG_SCALE,
                                        gray->height * OPT_IMG_SCALE),
                                 IPL_DEPTH_8U, 1);
        gray32f = cvCreateImage(cvSize(gray->width * OPT_IMG_SCALE,
                                       gray->height * OPT_IMG_SCALE),
                                IPL_DEPTH_32F, 1);
		mask_dbl = cvCreateImage(cvSize(gray->width * OPT_IMG_SCALE,
										gray->height * OPT_IMG_SCALE),
								IPL_DEPTH_8U, 1);
      } else {
        gray32f = cvCreateImage(cvSize(gray->width,
                                       gray->height),
                                IPL_DEPTH_32F, 1);
		mask_dbl = cvCreateImage(cvSize(gray->width,
									   gray->height),
								IPL_DEPTH_32F, 1); 
        sd.init(gray->width, gray->height);
      }
      is_first = false;
    }

    if(OPT_IMG_SCALE != 1) {
      if(gray_dbl->width != gray->width * OPT_IMG_SCALE ||
         gray_dbl->height != gray->height * OPT_IMG_SCALE) {
        fprintf(stderr, "%s has a different image size, skip\n", ifnames[i].c_str());
        continue;
      }

      cvResize(gray, gray_dbl);
      cvConvertScale(gray_dbl, gray32f);
	  
    } else {
      if(gray32f->width != gray->width ||
         gray32f->height != gray->height ) {
        fprintf(stderr, "%s has a different image size, skip\n", ifnames[i].c_str());
        continue;
      }

      cvConvertScale(gray, gray32f);
		
	  if (OPT_MASK) {
		cvResize(mask, mask_dbl);
	  }
    }

    result.clear();
	  sd.detect(gray32f, &result, OPT_MASK ? mask_dbl : NULL);

    fprintf(stderr, ", %zd keys found\n", result.size());

    if(OPT_DISPLAY_KEYS || DUMP_FNAME.empty() == false) {
      IplImage * buf = cvCreateImage(cvGetSize(gray), IPL_DEPTH_8U, 3);
      cvCvtColor(gray, buf, CV_GRAY2BGR);
      for(std::vector<SIFT::keypoint_t>::const_iterator itr=result.begin() ;
          itr != result.end() ;
          itr ++) {
        itr->draw(buf, false);
      }
      
      if(OPT_DISPLAY_KEYS) {
        cvNamedWindow("main");
        cvShowImage("main", buf);
        cvWaitKey(0);
        cvDestroyWindow("main");
      }
      
      if(! DUMP_FNAME.empty()) {
        char fname[PATH_MAX];
        snprintf(fname, sizeof(fname), DUMP_FNAME.c_str(), i);
        cvSaveImage(fname, buf);
      }

      cvReleaseImage(&buf);
    }

    std::ofstream ofs(ofnames[i].c_str());
    if(OPT_MATLAB) {
      for(unsigned int i=0 ; i<result.size() ; i++) {
        ofs << result[i].to_matlab();
      }
    } else {
      ofs << result.size() << " " << SIFT::keypoint_t::dim() << "\n";
      for(unsigned int i=0 ; i<result.size() ; i++) {
        ofs << result[i].to_string();
      }
    }
    ofs.close();
  }

  return 0;
}
