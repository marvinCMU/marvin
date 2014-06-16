/* 
 * Project: Multi Modal First Person Sensing (MMFPS)
 * Author: Qifan Hu (qhu@andrew.cmu.edu)
 * Affiliation: Carnegie Mellon University, ISTC-EC
 * Date: 11/5/2012
 */

// check whether the camera node is working
rostopic list
// show the image, open a new terminal
rosrun image_view image_view image:=/camera/image_raw
