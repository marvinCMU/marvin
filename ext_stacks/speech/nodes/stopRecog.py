#!/usr/bin/env python
import roslib; roslib.load_manifest('speech')

import sys

import rospy
from std_srvs.srv import *

def stopRecognizer():
    print "waiting for service"
    rospy.wait_for_service('trigger/stop')
    print "got service"
    try:
        stopService = rospy.ServiceProxy('trigger/stop', Empty)
        print "got service proxy"
        stopService()
        print "Stop service call done"
    except:
        print "Stop service call failed"

if __name__ == "__main__":
    print "Requesting"
    stopRecognizer()
