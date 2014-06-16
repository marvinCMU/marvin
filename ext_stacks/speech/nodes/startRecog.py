#!/usr/bin/env python
import roslib; roslib.load_manifest('speech')

import sys

import rospy
from std_srvs.srv import *

def startRecognizer():
    print "waiting for service"
    rospy.wait_for_service('trigger/start')
    print "got service"
    try:
        startService = rospy.ServiceProxy('trigger/start', Empty)
        print "got service proxy"
        startService()
        print "start service call done"
    except:
       print "start service call failed"

if __name__ == "__main__":
    print "Requesting"
    startRecognizer()
