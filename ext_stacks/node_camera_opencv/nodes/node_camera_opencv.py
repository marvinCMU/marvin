#!/usr/bin/env python
#This node reads images from the camera and emits the image over ros
import roslib
roslib.load_manifest('node_camera_opencv')
import sys
import rospy
import cv, cv2
from std_msgs.msg import String
from sensor_msgs.msg import Image
from cv_bridge import CvBridge, CvBridgeError

class node_camera_opencv:

  def __init__(self):
    self.image_pub = rospy.Publisher("image_raw",Image)

    cv.NamedWindow("Image window", 1)
    self.bridge = CvBridge()
    if rospy.has_param('camera/device'):
      device = rospy.get_param('camera/device')
      rospy.loginfo('Device %i specified. Trying to use that.' % device)
    else:
      rospy.loginfo('No device specified, using default camera 0')
      device = 0
      
    #TODO: Pass camera index as a parameter:
    rospy.loginfo('Creating capture object...')
    try:
      self.capture = cv2.VideoCapture(device)
      self.capture.set(cv2.cv.CV_CAP_PROP_FRAME_WIDTH,640)
      self.capture.set(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT,480)
    except Exception as e:
      rospy.logerr('Error creating capture object: ' + str(e))
      raise e

  def publish_image(self):
      #    try:
      #      cv_image = self.bridge.imgmsg_to_cv(data, "bgr8")
      #    except CvBridgeError, e:
      #      print e
    try:
      cv_image = self.capture.read()
    except Exception as e:
      rospy.logerr('Error capturing: ' + str(e))

    if (cv_image[0]):
      #print ('.')
      try:
        self.image_pub.publish(self.bridge.cv_to_imgmsg(cv.fromarray(cv_image[1]), "bgr8"))
        #cv2.imshow("Image window", cv_image[1])
        #cv2.waitKey(1)
      except CvBridgeError, e:
        rospy.logerr(e)
    else:
      rospy.logerr('Cannot capture. Camera not plugged in? Run cheese to verify camera working?')
      raise ('Camera not found')

def main(args):
  rospy.loginfo('Starting up node_camera_opencv...')
  ic = node_camera_opencv()
  rospy.init_node('node_camera_opencv', anonymous=True)
  while not rospy.is_shutdown():
    try:
      ic.publish_image()
    except KeyboardInterrupt:
      rospy.loginfo("Shutting down")


if __name__ == '__main__':
  device = rospy.get_param('camera/device')
  rospy.loginfo('device=%i'%
device)
  try:
    main(sys.argv)
  except rospy.ROSInterruptException:
    pass
