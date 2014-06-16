/**
 * @file   sift.h
 * @author Shohei NOBUHARA <nob@i.kyoto-u.ac.jp>
 * @date   Fri Feb 11 11:08:55 2011
 * 
 * @brief  This is a modified version of SIFT implemtation by David G. Lowe.
 *
 * @code
 *
 *   // init tbb
 *   tbb::task_scheduler_init tbb_init;
 *
 *   // load 32bit floating point grayscale image
 *   IplImage * gray = cvLoadImage("src.png", CV_LOAD_IMAGE_GRAYSCALE);
 *   IplImage * gray32f = cvCreateImage(cvGetSize(gray), IPL_DEPTH_32F, 1);
 *   cvConvertScale(gray, gray32f);
 *
 *   // init the detector (you can reuse this for images in the same size)
 *   SIFT::detector_t sd;
 *   sd.init(gray32f->width, gray32f->height);
 *
 *   // detect
 *   std::vector<SIFT::keypoint_t> result;
 *   sd.detect(gray32f, &result);
 *
 *   // dump
 *   std::cout << result.size() << " " << SIFT::keypoint_t::dim() << "\n";
 *   for(unsigned int i=0 ; i<result.size() ; i++) {
 *     std::cout << result[i].to_string();
 *   }
 *
 * @endcode
 *
 * @note For the best performance, use icpc (Intel C++ Compiler).
 *
 *
 */

/* Copyright (c) 2000. David G. Lowe, University of British Columbia.
   This code may be used, distributed, or modified only for research
   purposes or under license from the University of British Columbia.
   This notice must be retained in all copies.
*/
/*
 *  Find the SIFT keys in an image.  
 *     Code is by David Lowe, University of British Columbia.
 *     See paper for details at:
 *        http://www.cs.ubc.ca/spider/lowe/papers/iccv99-abs.html
 *     Call routine GetKeypoints(image) to return all keypoints for image.
 */
#ifndef SIFT_H
#define SIFT_H

#include <iostream>
#include <cv.h>
#include <highgui.h>
#include <list>
#include <cstdio>
#include <sys/time.h>
#include <vector>

#ifdef ENABLE_TBB
#  include <tbb/task_scheduler_init.h>
#  include <tbb/blocked_range.h>
#  include <tbb/parallel_for.h>
#endif

namespace SIFT {

  template<int width, int nbins>
  struct keypoint_base_t {
    float x;
    float y;
    float scale;
    float orientation;           // [-M_PI:M_PI]
    int level;
    float descriptor[width*width*nbins];

    void draw(IplImage * buf, int smalltick) const;

    void compute_descriptor(const IplImage * gauss,
                            const IplImage * ori,
                            const float mag);

    static float key_display_size() { return 0.4; }

    static int dim() { return width*width*nbins; }

    std::string to_string() const;
    std::string to_matlab() const;

  };
  typedef keypoint_base_t<4,8> keypoint_t;

#ifdef ENABLE_TBB
  template<typename keypoint_type, int levels>
  struct tbb_keypoint_base_helper_t {
    std::vector<keypoint_type> & array;
    IplImage ** pyr_gauss;
    IplImage ** pyr_ori;
    float mag[levels];

    tbb_keypoint_base_helper_t(std::vector<keypoint_type> & array_,
                               IplImage ** pyr_gauss_,
                               IplImage ** pyr_ori_) :
      array(array_), pyr_gauss(pyr_gauss_), pyr_ori(pyr_ori_) {
      for(int i=0 ; i<levels ; i++) {
        mag[i] = pow(2.0/3.0, i);
      }
    }

    void operator()(const tbb::blocked_range<size_t> & range) const {
      for(size_t i=range.begin() ; i!=range.end() ; i++) {
        const int l = array[i].level-1;
        array[i].compute_descriptor(pyr_gauss[l],
                                    pyr_ori[l],
                                    mag[l]);
      }
    }
  };
#endif

  template<int levels>
  struct detector_base_t {
    /* Set 0.5 if doubled size image was given as the input. */
    float coord_scale;

    /* Gradients are thresholded at this maximum */
    float gradient_threshold;

    /* Normalized peaks must be above this threshold. */
    float normalized_peak_threshold;

    /* The default threshold on ratio by which points in circle of 3 pixels radius
       should dip lower than current point (value should be less than 1.0, but
       can be raised much above 1 to eliminate check).
    */
    float dip;

    /* Number of bins in orientation histogram (10 degree spacing). */
    int orientation_bins;

    /* Size of Gaussian used to select orientations.*/
    float orientation_sigma;

    int min_image_size;

    detector_base_t() {
      for(int i=0 ; i<levels ; i++) {
        pyr_gauss[i] = NULL;
        pyr_dog[i] = NULL;
        pyr_ori[i] = NULL;
      }

      set_default_values();
    }

    ~detector_base_t() {
      release();
    }

    void set_default_values() {
      coord_scale = 1.0;
      gradient_threshold = 0.1;
      normalized_peak_threshold = 0.1;
      dip = 0.95;
      orientation_bins = 36;
      orientation_sigma = 3.0;
      min_image_size = 10;
    }

    void init(int width, int height);

    void release() {
      for(int i=0 ; i<levels ; i++) {
        cvReleaseImage(&(pyr_gauss[i]));
        cvReleaseImage(&(pyr_dog[i]));
        cvReleaseImage(&(pyr_ori[i]));
      }
    }

    template<typename T>
    inline void detect(const IplImage * gray32f, T * result, IplImage * mask = NULL);

    // for findori
    int radius;
    float radiussq;
    float exp_coeff;
    std::vector<float> hist;
    std::vector<float> hist_work;
    std::vector<float> expcache;

  private:
    IplImage * pyr_gauss[levels];
    IplImage * pyr_dog[levels];
    IplImage * pyr_ori[levels];

  };

  typedef detector_base_t<21> detector_t;


  inline int int_sq(int a, int b) {
    return a*a + b*b;
  }

  inline void check(const IplImage * image, int r, int c) {
    assert(r >= 0 && r<image->height);
    assert(c >= 0 && c<image->width);
  }

  inline float * pix_p(IplImage * image, int r) {
    //check(image, r, c);
    return reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r);
  }

  inline const float * pix_p(const IplImage * image, int r) {
    //check(image, r, c);
    return reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r);
  }

  inline float * pix_p(IplImage * image, int r, int c) {
    //check(image, r, c);
    return reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r) + c;
  }

  inline const float * pix_p(const IplImage * image, int r, int c) {
    //check(image, r, c);
    return reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r) + c;
  }

  inline float & pix(IplImage * image, int r, int c) {
    //check(image, r, c);
    return *(reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r) + c);
  }

  inline const float & pix(const IplImage * image, int r, int c) {
    //check(image, r, c);
    return *(reinterpret_cast<float *>((image->imageData) + (image->widthStep) * r) + c);
  }

  /* Display a square marking this location.  The size of the square
     is made 10 times the sigma of the equivalent small DOG filter, which
     gives a size 10 * 1.414 = 14 pixels in pixel sampling for this level.
     Key sampling region is size 16 pixels, for image at level of smaller
     sigma blur. 
     The square is oriented in direction "ori" with a tick mark to show the
     top.  The "smalltick" flag gives a short tick for top-down matches.
  */
  template<int width, int nbins>
  void keypoint_base_t<width,nbins>::draw(IplImage * buf, int smalltick) const {
    CvPoint2D32f pt[5];

    const float cx = this->x;
    const float cy = this->y;

    const float sine = sin(this->orientation);
    const float cosine = cos(this->orientation);
  
    /* Create a vector for one corner (1,1), rotated by ori. */
    float y = -5.0 * key_display_size() * this->scale * (cosine + sine);
    float x =  5.0 * key_display_size() * this->scale * (cosine - sine);

    /* Rotate this vector by increments of 90 degrees to create square. */
    for (int i = 0; i < 5; i++) {
      pt[i].x = cx + x;
      pt[i].y = cy + y;
      float nx = y;
      float ny = - x;
      x = nx;
      y = ny;
    }

    for (int i = 0; i < 4; i++) {
      cvLine(buf, cvPoint(pt[i].x,pt[i].y), cvPoint(pt[i+1].x,pt[i+1].y), CV_RGB(255,0,0));
    }

    /* Place a tick mark half way between first 2 points. */
    const float hx = (pt[0].x + pt[1].x) / 2.0;
    const float hy = (pt[0].y + pt[1].y) / 2.0;
    /* Create line from center to half way point.  If "smalltick" flag is set,
       then remove 0.7 of the tick mark. */
    cvLine(buf, cvPoint(hx, hy), smalltick ?
           cvPoint(cx + 0.7 * (hx - cx), cy + 0.7 * (hy - cy)) :
           cvPoint(cx, cy), CV_RGB(255,0,0));
  }

/* determines the size of a single descriptor orientation histogram */
#define SIFT_DESCR_SCL_FCTR 3.0

/* threshold on magnitude of elements of descriptor vector */
#define SIFT_DESCR_MAG_THR 0.2

/* factor used to convert floating-point descriptor to unsigned char */
#define SIFT_INT_DESCR_FCTR 512.0

  /*
    Interpolates an entry into the array of orientation histograms that form
    the feature descriptor.

    @param hist 2D array of orientation histograms
    @param rbin sub-bin row coordinate of entry
    @param cbin sub-bin column coordinate of entry
    @param obin sub-bin orientation coordinate of entry
    @param mag size of entry
    @param d width of 2D array of orientation histograms
    @param n number of bins per orientation histogram
  */
  static void interp_hist_entry( float* hist, float rbin, float cbin,
                                 float obin, float mag, int d, int n ) {
    float d_r, d_c, d_o, v_r, v_c, v_o;
    float * h;
    int r0, c0, o0, rb, cb, ob, r, c, o;

    r0 = cvFloor( rbin );
    c0 = cvFloor( cbin );
    o0 = cvFloor( obin );
    d_r = rbin - r0;
    d_c = cbin - c0;
    d_o = obin - o0;

    /*
      The entry is distributed into up to 8 bins.  Each entry into a bin
      is multiplied by a weight of 1 - d for each dimension, where d is the
      distance from the center value of the bin measured in bin units.
    */
    for( r = 0; r <= 1; r++ ) {
      rb = r0 + r;
      if( rb >= 0  &&  rb < d ) {
        v_r = mag * ( ( r == 0 )? 1.0 - d_r : d_r );
        //row = hist[rb];
        for( c = 0; c <= 1; c++ ) {
          cb = c0 + c;
          if( cb >= 0  &&  cb < d ) {
            v_c = v_r * ( ( c == 0 )? 1.0 - d_c : d_c );
            //h = row[cb];
            h = hist + rb*d*n + cb*n;
            for( o = 0; o <= 1; o++ ) {
              ob = ( o0 + o ) % n;
              v_o = v_c * ( ( o == 0 )? 1.0 - d_o : d_o );
              h[ob] += v_o;
            }
          }
        }
      }
    }
  }

  static inline void normalize_array( float * array, const int n ) {
    float sum = 0;
    for(int i = 0; i < n; i++ ) {
      sum += array[i]*array[i];
    }
    if(sum == 0) return;

    sum = 1.0 / sqrt(sum);
    for(int i = 0; i < n; i++ ) {
      array[i] *= sum;
    }
  }

  template<int width, int nbins>
  void keypoint_base_t<width,nbins>::compute_descriptor(const IplImage * gauss,
                                                        const IplImage * ori,
                                                        const float mag) {
    /*
    fprintf(stderr, "r=%f, c=%f, ori=%f, scl=%f, d=%d, n=%d, w=%d, h=%d\n",
            y, x, orientation, scale,
            width, nbins, gauss->width, ori->height);
    */

    //const int rval = y * M_SQRT2 / scale - 1.0;
    //const int cval = x * M_SQRT2 / scale - 1.0;

    const int DIM = width*width*nbins;

    for(int i=0 ; i<DIM ; i++) {
      descriptor[i] = 0;
    }

    float grad_mag, grad_ori;

    //const float mag = pow(2.0/3.0,level-1);
    const float cos_t = cos(orientation);
    const float sin_t = sin(orientation);
    const float bins_per_rad = nbins / (M_PI * 2.0);
    const float exp_denom = 1.0 / (width * width * 0.5);
    const float hist_width = SIFT_DESCR_SCL_FCTR * scale;
    const int radius = hist_width * M_SQRT2 * ( width + 1.0 ) * 0.5 + 0.5;
    for(int i = -radius; i <= radius; i++ ) {
      for(int j = -radius; j <= radius; j++ ) {
	/*
	  Calculate sample's histogram array coords rotated relative to ori.
	  Subtract 0.5 so samples that fall e.g. in the center of row 1 (i.e.
	  r_rot = 1.5) have full weight placed in row 1 after interpolation.
	*/
	const float c_rot = ( j * cos_t - i * sin_t ) / hist_width;
	const float r_rot = ( j * sin_t + i * cos_t ) / hist_width;
	const float rbin = r_rot + width / 2 - 0.5;
	const float cbin = c_rot + width / 2 - 0.5;

	if( rbin > -1.0  &&  rbin < width  &&  cbin > -1.0  &&  cbin < width ) {
          int r = (y+i)*mag;
          int c = (x+j)*mag;
          if( r <= 0 || r > gauss->height-1 ||
              c <= 0 || c > gauss->width-1 ) {
            continue;
          }
          grad_mag = pix(gauss,r,c);
          grad_ori = pix(ori,r,c);
          //calc_grad_mag_ori( img, y+i, x+j, &grad_mag, &grad_ori );

          grad_ori -= orientation;
          while( grad_ori < 0.0 )
            grad_ori += (M_PI * 2.0);
          while( grad_ori >= (M_PI * 2.0) )
            grad_ori -= (M_PI * 2.0);

          const float obin = grad_ori * bins_per_rad;
          const float w = exp( -(c_rot * c_rot + r_rot * r_rot) * exp_denom );
          interp_hist_entry( descriptor, rbin, cbin, obin, grad_mag * w, width, nbins );
        }
      }
    }
    // normalize
    normalize_array( descriptor, DIM );
    for(int k=0; k<DIM; k++ ) {
      if( descriptor[k] > SIFT_DESCR_MAG_THR ) {
        descriptor[k] = SIFT_DESCR_MAG_THR;
      }
    }
    normalize_array( descriptor, DIM );

    // convert floating-point descriptor to integer valued descriptor
    for(int k=0; k<DIM ; k++ ) {
      int val = SIFT_INT_DESCR_FCTR * descriptor[k];
      descriptor[k] = MIN( 255, val );
    }
  }

  template<int width, int nbins>
  std::string keypoint_base_t<width,nbins>::to_string() const {
    char buf[2048];
    int idx=0;
    idx = this->snprintf( buf, sizeof(buf), "%f %f %f %f",
                    y, x, scale, orientation );
    for(int i=0; i<width*width*nbins; i++) {
      if(i % 20 == 0) {
        idx += this->snprintf(buf+idx, sizeof(buf)-idx, "\n");
      }
      idx += this->snprintf( buf+idx, sizeof(buf)-idx, " %d", (int)(descriptor[i]) );
    }
    idx += this->snprintf( buf+idx, sizeof(buf)-idx, "\n" );
    assert((unsigned int)idx < sizeof(buf)-1);
    return buf;
  }

  template<int width, int nbins>
  std::string keypoint_base_t<width,nbins>::to_matlab() const {
    char buf[2048];
        int idx=0;
    idx = this->snprintf( buf, sizeof(buf), "%f %f %f %f",
                    y, x, scale, orientation );
    for(int i=0; i<width*width*nbins; i++) {
      idx += this->snprintf( buf+idx, sizeof(buf)-idx, " %d", (int)(descriptor[i]) );
    }
    idx += this->snprintf( buf+idx, sizeof(buf)-idx, "\n" );
    assert((unsigned int)idx < sizeof(buf)-1);
    return buf;
  }

  /* Convolve image with the a 1-D Gaussian kernel vector along image rows.
     The Gaussian has a sigma of sqrt(2), which results in the following kernel:
     (.030 .105 .222 .286 .222 .105 .030)
     Pixels outside the image are set to the value of the closest image pixel.
  */
  static void HorConvSqrt2(const IplImage * image, IplImage * result) {
    const int rows = image->height;
    const int cols = image->width;

    for (int r = 0; r < rows; r++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      float * p = pix_p(result, r, 3);
      const float * prc = pix_p(image, r, 3);
      for (int c = 3; c < cols - 3; c++, p++, prc++) {
        *p = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
          0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (int c = 0; c < 3; c++) {
        const float p1 = (c < 1) ? pix(image,r,0) : pix(image,r,c-1);
        const float * prc = pix_p(image, r, c);
        pix(result,r,c) = 0.135 * pix(image,r,0) + 0.222 * p1 +
          0.286 * prc[0] + 0.222 * prc[1] + 0.105 * prc[2] + 0.030 * prc[3];
      }
      for (int c = cols - 3; c < cols; c++) {
        const float p1 = (c >= cols - 1) ? pix(image,r,cols-1) : pix(image,r,c+1);
        const float * prc = pix_p(image, r, c);
        pix(result,r,c) = 0.030 * prc[-3] + 0.105 * prc[-2] + 0.222 * prc[-1] +
          0.286 * prc[0] + 0.222 * p1 + 0.135 * pix(image,r,cols-1);
      }
    }
  }

  /* Same as HorConvSqrt2, but convolve along vertical direction.
   */
  static void VerConvSqrt2(const IplImage * image, IplImage * result) {
    const int rows = image->height;
    const int cols = image->width;

    for (int c = 0; c < cols; c++) {
      /* Handle easiest case of pixels that do not overlap the boundary. */
      for (int r = 3; r < rows - 3; r++) {
        pix(result,r,c) = 0.030 * pix(image,r-3,c) + 0.105 * pix(image,r-2,c) +
          0.222 * pix(image,r-1,c) + 0.286 * pix(image,r,c) + 0.222 * pix(image,r+1,c) +
          0.105 * pix(image,r+2,c) + 0.030 * pix(image,r+3,c);
      }
      /* For pixels near boundary, use value of boundary pixel. */
      for (int r = 0; r < 3; r++) {
        const float p1 = (r < 1) ? pix(image,0,c) : pix(image,r-1,c);
        pix(result,r,c) = 0.135 * pix(image,0,c) + 0.222 * p1 +
          0.286 * pix(image,r,c) + 0.222 * pix(image,r+1,c) +
          0.105 * pix(image,r+2,c) + 0.030 * pix(image,r+3,c);
      }
      for (int r = rows - 3; r < rows; r++) {
        const float p1 = (r >= rows - 1) ? pix(image,rows-1,c) : pix(image,r+1,c);
        pix(result,r,c) = 0.030 * pix(image,r-3,c) + 0.105 * pix(image,r-2,c) +
          0.222 * pix(image,r-1,c) + 0.286 * pix(image,r,c) + 0.222 * p1 +
          0.135 * pix(image,rows-1,c);
      }
    }
  }

  static IplImage * create_reduced_size_image(const IplImage * image) {
    const int nrows = 2 * (image->height) / 3;
    const int ncols = 2 * (image->width) / 3;
    IplImage * newimage = cvCreateImage(cvSize(ncols,nrows), IPL_DEPTH_32F, 1);
    return newimage;
  }

  /* Reduce the size of the image by resampling with a pixel spacing of
     1.5 times original spacing.  Each block of 9 original pixels is
     replaced by 4 new pixels resampled with bilinear interpolation.
  */
  static void ReduceSize(const IplImage * image, IplImage * newimage) {
    const int nrows = newimage->height;
    const int ncols = newimage->width;

    assert(nrows == 2 * (image->height) / 3);
    assert(ncols == 2 * (image->width) / 3);

    int r1, r2, c1, c2;
    for (int r = 0; r < nrows; r++) {
      for (int c = 0; c < ncols; c++) {
        if (r % 2 == 0) {
          r1 = (r >> 1) * 3;
          r2 = r1 + 1;
        } else {
          r1 = (r >> 1) * 3 + 2;
          r2 = r1 - 1;
        }
        if (c % 2 == 0) {
          c1 = (c >> 1) * 3;
          c2 = c1 + 1;
        } else {
          c1 = (c >> 1) * 3 + 2;
          c2 = c1 - 1;
        }
        pix(newimage,r,c) =
          0.5625 * pix(image,r1,c1) +
          0.1875 * (pix(image,r2,c1) + pix(image,r1,c2)) +
          0.0625 * pix(image,r2,c2);
      }
    }
  }


  /* Given a smoothed image "im", return edge gradients and orientations
     in "im" and "ori".  The gradient is computed from pixel differences,
     so is offset by half a pixel from original image.
     Result is normalized so that threshold value is 1.0, for ease of
     display and to make results less sensitive to change in threshold.
  */
  static void GradOriImages(const detector_t * self, IplImage * im, IplImage * ori) {
    const int rows = im->height;
    const int cols = im->width;

    for (int r = 0; r < rows-1; r++) {
      float * pi = pix_p(im,r);
      float * po = pix_p(ori,r);
      const float * qi = pix_p(im,r+1);
      for (int c = 0; c < cols-1; c++, pi++, po++, qi++) {
        const float xgrad = pi[1] - pi[0];
        const float ygrad = pi[0] - qi[0];
        pi[0] = sqrt(xgrad * xgrad + ygrad * ygrad);
        po[0] = atan2 (ygrad, xgrad);
      }
    }

    /* Threshold all edge magnitudes at GradThresh and scale to 1.0. */
    const float invthresh = 1.0 / (self->gradient_threshold);
    for (int r = 0; r < rows; r++) {
      float * p = pix_p(im,r);
      for (int c = 0; c < cols; c++, p++) {
        if (p[0] > (self->gradient_threshold)) {
          p[0] = 1.0;
        } else {
          p[0] *= invthresh;
        }
      }
    }
  }

  /* Check whether this pixel is a local maximum (positive value) or
     minimum (negative value) compared to the 3x3 neighbourhood that
     is centered at (row,col).  If level is -1 (or 1), then (row,col) must
     be scaled down (or up) a level by factor 1.5.
  */
  static int LocalMaxMin(float val, const IplImage * image, int row, int col, int level) {
    /* Calculate coordinates for image one level down (or up) in pyramid. */
    if (level < 0) {
      row = (3 * row + 1) / 2;
      col = (3 * col + 1) / 2;
    } else if (level > 0) {
      row = (2 * row) / 3;
      col = (2 * col) / 3;
    }

    if (val > 0.0) {
      for (int r = row - 1; r <= row + 1; r++) {
        const float * p = pix_p(image,r);
        for (int c = col - 1; c <= col + 1; c++) {
          if (p[c] > val) {
            return false;
          }
        }
      }
    } else {
      for (int r = row - 1; r <= row + 1; r++) {
        const float * p = pix_p(image,r);
        for (int c = col - 1; c <= col + 1; c++) {
          if (p[c] < val) {
            return false;
          }
        }
      }
    }
    return true;
  }


  /* Check whether the image point closest to (row,col) is below the
     given value.  It would be possible to use bilinear interpolation to
     sample the image more accurately, but this is not necessary as we
     are looking for a worst-case peak value.
  */
  static int CheckDipPoint(float val, const IplImage * image, const int r, const int c) {
    if (r < 0  ||  c < 0  ||  r >= image->height  || c >= image->width)
      return true;
    if (val > 0.0) {
      if (pix(image,r,c) > val)
        return false;
    } else {
      if (pix(image,r,c) < val)
        return false;
    }
    return true;
  }


  /* Check whether 20 points sampled on a ring of radius 3 pixels around
     the center point are all less than Dip * "val".
  */
  static int CheckDip(const detector_t * self, float val, const IplImage * image, int row, int col) {
    float x, y, nx, ny;

    /* Create a vector of length 4 pixels (2*scale was used in orig). */
    x = 3.0;
    y = 0.0;

    /* Rotate this vector around a circle in increments of PI/20 and
       check the dip of each point. */
    const static int N = 20;
    const static float sine = sin(M_PI / N);
    const static float cosine = cos(M_PI / N);
    for (int i = 0; i < N*2; i++) {
      if (! CheckDipPoint((self->dip) * val, image, row+y+1, col+x+1))
        return false;
      nx = cosine * x - sine * y;
      ny = sine * x + cosine * y;
      x = nx;
      y = ny;
    }
    return true;
  }


  /* Return a number in the range [-0.5, 0.5] that represents the location
     of the peak of a parabola passing through the 3 samples.  The center
     value is assumed to be greater than or equal to the other values if
     positive, or less than if negative.
  */
  static float InterpPeak(float a, float b, float c)
  {
    if (b < 0.0) {
      a = -a; b = -b; c = -c;
    }
    assert(b >= a  &&  b >= c);
    return 0.5 * (a - c) / (a - 2.0 * b + c);
  }


  /* Smooth a histogram by using the [0.25, 0.5, 0.25] kernel.  Assume
     the histogram is connected in a circular buffer.

     NOTE: original SmoothHistogram has a bug when i==bins-1 (it uses updated hist[0])
  */
  static void SmoothHistogramTwice(float *hist, const int bins, float *work) {
    work[0] = hist[bins-1]*0.25 + hist[0]*0.5 + hist[1]*0.25;
    for(int i=1 ; i<bins-1 ; i++) {
      work[i] = hist[i-1]*0.25 + hist[i]*0.5 + hist[i+1]*0.25;
    }
    work[bins-1] = hist[bins-2]*0.25 + hist[bins-1]*0.5 + hist[0]*0.25;

    hist[0] = work[bins-1]*0.25 + work[0]*0.5 + work[1]*0.25;
    for(int i=1 ; i<bins-1 ; i++) {
      hist[i] = work[i-1]*0.25 + work[i]*0.5 + work[i+1]*0.25;
    }
    hist[bins-1] = work[bins-2]*0.25 + work[bins-1]*0.5 + work[0]*0.25;
  }

  /* Find a peak in the histogram and return corresponding angle.
   */
  static float FindOriPeaks(const float *hist, int bins) {
    int maxloc = 0;
    float maxval = 0.0;

    /* Find peak in histogram. */
    for (int i = 0; i < bins; i++) {
      if (hist[i] > maxval) {
        maxval = hist[i];
        maxloc = i;
      }
    }

    /* Set angle in range -PI to PI. */
    return (2.0 * M_PI * (maxloc + 0.5) / bins - M_PI);
  }

  /* Assign an orientation to this keypoint.  This is done by
     creating a Gaussian weighted histogram of the gradient directions in
     the region.  The histogram is smoothed and the largest peak selected.
     The results are in the range of -PI to PI.
  */
  static float FindOri(detector_t * self, const IplImage * grad, const IplImage * ori, int row, int col) {
    const int OriBins = self->orientation_bins;
    float * hist = &(self->hist[0]);
    float * work = &(self->hist_work[0]);

    const int rows = grad->height;
    const int cols = grad->width;

    for (int i = 0; i < OriBins; i++) {
      hist[i] = 0;
    }

    /* Look at pixels within 3 sigma around the point and put their
       Gaussian weighted values in the histogram. */
    const int radius = self->radius;
    const float radiussq = self->radiussq;

    /* Do not use last row or column, which are not valid. */
    int rs = row - radius;
    if(rs < 0) rs = 0;

    int re = row + radius;
    if(re >= rows - 2) re = rows - 3;

    int cs = col - radius;
    if(cs < 0) cs = 0;

    int ce = col + radius;
    if(ce >= cols - 2) ce = cols - 3;

    for (int r=rs; r <= re; r++) {
      const float * pg = pix_p(grad,r);
      const float * po = pix_p(ori,r);
      for (int c=cs; c <= ce; c++) {
        const float gval = pg[c];
        if(gval <= 0.0) continue;

        const int distsq = int_sq(r-row, c-col);
        if (distsq >= radiussq) {
          continue;
        }

        const float weight = self->expcache[distsq];

        /* Ori is in range of -PI to PI. */
        const float angle = po[c];
        // assert(angle >= -M_PI  && angle <= M_PI); // this cannot be hold...
        int bin = (int) (OriBins * (angle + (M_PI + 0.001)) * (M_1_PI * 0.5));
        assert(bin >= 0 && bin <= OriBins);
        bin = MIN(bin, OriBins - 1);
        hist[bin] += weight * gval;
      }
    }
    /* Apply smoothing twice. */
    SmoothHistogramTwice(hist, OriBins, work);

    return FindOriPeaks(hist, OriBins);
  }

  /* Find the position of the key point by interpolation in position, and
     create key vector.  Output its location as a square.
     The (grad2,ori2) provide gradient and orientation sampled one octave
     above that of (grad,ori).
  */
  template<int width, int nbins>
  static void InterpKeyPoint(detector_t * self,
                             int level,
                             const IplImage * pyr_dog,
                             const IplImage * grad,
                             const IplImage * ori,
                             const IplImage * grad2,
                             const IplImage * ori2, int r, int c,
                             keypoint_base_t<width,nbins> * key) {
    float center, scale, rval, cval, opeak;

    center = pix(pyr_dog,r,c);
    /* Scale relative to input image.  Represents size of Gaussian
       for smaller of DOG filters used at this scale. */
    scale = 1.414 * pow(1.5, (double) level);

    rval = r + InterpPeak(pix(pyr_dog,r-1,c),
                          center, pix(pyr_dog,r+1,c));
    cval = c + InterpPeak(pix(pyr_dog,r,c-1),
                          center, pix(pyr_dog,r,c+1));

    /* Scale up center location to account for level's scale change.
       A peak at (0,0) in gradient image would be at (0.5,0,5) in
       original image.  Add another 0.5 to place at pixel center.
       Therefore, peak at (0,0) gives location on boundary from 0 to 1. */
    rval = (rval + 1.0) * scale / 1.414;
    cval = (cval + 1.0) * scale / 1.414;

    /* If image was doubled to find keypoints, then row,col must be reduced
       by factor of 2 to put in coordinates of original image. */
    if (self->coord_scale != 1.0) {
      rval *= self->coord_scale;
      cval *= self->coord_scale;
      scale *= self->coord_scale;
    }

    /* Find orientation(s) for this keypoint.*/
    opeak = FindOri(self, grad, ori, r, c);


    key->x = cval;
    key->y = rval;
    key->scale = scale;
    key->orientation = opeak;
    key->level = level;
  }


  /* Find the local maxima and minima of the DOG images in the pyr_dog pyramid.
   */
  template<typename T>
  void FindMaxMin(detector_t * self,
                  IplImage ** pyr_gauss,
                  IplImage ** pyr_dog,
                  IplImage ** pyr_ori,
                  int levels, T * container, IplImage * mask = NULL) {
    const float PeakThresh = self->normalized_peak_threshold;

    /*
    fprintf(stderr, "Searching %d image levels, from size %dx%d\n",
            levels, pyr_dog[0]->height, pyr_dog[0]->width);
    */

    typename T::value_type key;
    /* Search through each scale, leaving 1 scale below and 2 above. */
    for (int i = 1; i < levels - 2; i++) {
      const int rows = pyr_gauss[i]->height;
      const int cols = pyr_gauss[i]->width;

      /* Only find peaks at least 5 pixels from borders. */
		if (mask == NULL) {
			for (int r = 5; r < rows - 5; r++) {
				for (int c = 5; c < cols - 5; c++) {
					const float val = pix(pyr_dog[i],r,c);
					if (fabs(val) > PeakThresh  &&
						LocalMaxMin(val, pyr_dog[i], r, c, 0) &&
						LocalMaxMin(val, pyr_dog[i-1], r, c, -1) &&
						LocalMaxMin(val, pyr_dog[i+1], r, c, 1) &&
						CheckDip(self, val, pyr_dog[i], r, c)) {
						InterpKeyPoint(self, i, pyr_dog[i], pyr_gauss[i], pyr_ori[i],
									   pyr_gauss[i+2], pyr_ori[i+2], r, c, &key);
						container->push_back(key);
					}
				}
			}
		}
		
		else {
			for (int r = 5; r < rows - 5; r++) {
				for (int c = 5; c < cols - 5; c++) {
					const float val = pix(pyr_dog[i],r,c);
					if ( ((uchar *) (mask->imageData + r*mask->widthStep))[c] && // pixel(r,c) is part of the mask.
						fabs(val) > PeakThresh  &&
						LocalMaxMin(val, pyr_dog[i], r, c, 0) &&
						LocalMaxMin(val, pyr_dog[i-1], r, c, -1) &&
						LocalMaxMin(val, pyr_dog[i+1], r, c, 1) &&
						CheckDip(self, val, pyr_dog[i], r, c)) {
						InterpKeyPoint(self, i, pyr_dog[i], pyr_gauss[i], pyr_ori[i],
									   pyr_gauss[i+2], pyr_ori[i+2], r, c, &key);
						container->push_back(key);
					}
				}
			}
		}
		
    }
  }

  template<int levels>
  inline void detector_base_t<levels>::init(int width, int height) {
    int lev = 0;
    IplImage * temp = cvCreateImage(cvSize(width,height), IPL_DEPTH_32F, 1);
    IplImage * image = cvCreateImage(cvSize(width,height), IPL_DEPTH_32F, 1);
    IplImage * smaller;

    while (image->height > min_image_size &&
           image->width > min_image_size && lev < levels-1) {
      IplImage * blur = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_32F, 1);
      pyr_dog[lev] = temp;
      smaller = create_reduced_size_image(blur);
      pyr_gauss[lev] = image;
      pyr_ori[lev] = blur;

      lev++;
      image = smaller;
      temp = cvCreateImage(cvSize(image->width, image->height),
                           IPL_DEPTH_32F, 1);
    }

    // we need +1 gauss at while() loop in detect().
    pyr_gauss[lev] = image;
    // we don't need this
    cvReleaseImage(&temp);

    for(int i=0 ; i<lev ; i++) {
      assert(pyr_gauss[i] != NULL);
      assert(pyr_dog[i] != NULL);
      assert(pyr_ori[i] != NULL);

#if 0
      fprintf(stderr, "%d g:%dx%d, d:%dx%d, o:%dx%d\n",
              i,
              pyr_gauss[i]->width, pyr_gauss[i]->height,
              pyr_dog[i]->width, pyr_dog[i]->height,
              pyr_ori[i]->width, pyr_ori[i]->height);
#endif
    }


    // init cache
    hist.resize(orientation_bins);
    hist_work.resize(orientation_bins);
    // for findori
    radius = (int) (orientation_sigma * 3.0);
    radiussq = radius * radius + 0.5;
    const float exp_coeff = - 1.0 / (2.0 * orientation_sigma * orientation_sigma);
    expcache.resize(radius*radius+1);
    for(unsigned int i=0 ; i<expcache.size() ; i++) {
      expcache[i] = exp(exp_coeff * i);
    }
  }

  static void SmoothImage(const IplImage * src,
                   IplImage * dst,
                   IplImage * work) {
    cvSmooth(src, dst, CV_GAUSSIAN, 7, 7, M_SQRT2);
    return;

    HorConvSqrt2(src, work);
    VerConvSqrt2(work, dst);
  }

  template<int levels>
  template<typename T>
  inline void detector_base_t<levels>::detect(const IplImage * gray, T * result, IplImage* mask) {
    assert(gray->depth == IPL_DEPTH_32F);
    assert(gray->nChannels == 1);

    if(pyr_gauss[0] == NULL) {
      init(gray->width, gray->height);
    }

    int lev = 0, first = true;
    IplImage * blur = NULL;
    IplImage * temp = pyr_dog[lev];
    IplImage * image = pyr_gauss[lev];
    IplImage * smaller = NULL;

    /* Smooth input image with Gaussian of sqrt(2). */
    SmoothImage(gray, image, temp);

    /* Create each level of pyramid.  Keep reducing image by factors of 1.5
       until one dimension is smaller than minimum size.
    */
    while (image->height > min_image_size &&
           image->width > min_image_size && lev < 20) {
      //fprintf(stderr, "lev=%d, r=%d, c=%d\n", lev, image->height, image->width);

      /* Smooth image by sqrt(2) and subtract to get DOG image. */
      blur = pyr_ori[lev];
      SmoothImage(image, blur, temp);

      cvSub(image, blur, temp);
      cvScale(temp, temp, 10);
      pyr_dog[lev] = temp;

      /* Generate next level of pyramid by reducing by factor of 1.5. */
      smaller = pyr_gauss[lev+1];
      ReduceSize(blur, smaller);

      /* Find image gradients and orientations.  Reuse image and blur.
         This does not need to be done for lowest image in pyramid. */
      pyr_gauss[lev] = image;
      pyr_ori[lev] = blur;
      //if (! first) {
        GradOriImages(this, pyr_gauss[lev], pyr_ori[lev]);
        //}
      first = false;

      lev++;
      image = smaller;
      temp = pyr_dog[lev];
    }

    /* Find the keypoints in the pyramid. */
    FindMaxMin(this, pyr_gauss, pyr_dog, pyr_ori, lev, result, mask);

#if ENABLE_TBB
    tbb::parallel_for( tbb::blocked_range<size_t>(0, result->size()),
                       tbb_keypoint_base_helper_t<typename T::value_type,levels>(*result, pyr_gauss, pyr_ori));
#else
	  //std::vector<T> selected;
    for(unsigned int i=0 ; i<result->size() ; i++) {
      assert((*result)[i].level-1 >= 0);
      /*
      if((int)((*result)[i].y) == 276 && (int)((*result)[i].x) == 204) {
        (*result)[i].scale = 2.848562;
        (*result)[i].orientation = 0.091321;
        { __asm__ __volatile__ ("int $03"); }
      }
      */
      //fprintf(stdout, "w=%d, h=%d, scale=%f, level=%d\n", pyr_gauss[(*result)[i].level-1]->width, pyr_gauss[(*result)[i].level-1]->height, (*result)[i].scale, (*result)[i].level-1);

		//if((*result)[i].x > 0 && (*result)[i].y >0) {
			(*result)[i].compute_descriptor(pyr_gauss[(*result)[i].level-1],
											pyr_ori[(*result)[i].level-1],
											pow(2.0/3.0,(*result)[i].level-1));
			//selected.push_back((*result)[i]);			
			//}
    }
	  //result->swap(selected);
	  
#endif
  }
}

#endif
