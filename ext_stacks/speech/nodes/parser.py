#!/usr/bin/env python
import roslib; roslib.load_manifest('speech')
import rospy
import sys

from std_msgs.msg import String
from std_srvs.srv import *

class parser(object):

    def __init__(self):
        rospy.init_node('parser')
        self.retailpub = rospy.Publisher('~retailCmd', String)
        rospy.Subscriber("trigger/output", String, self.triggerParser)
        rospy.Subscriber("command/output", String, self.commandParser)
        rospy.spin()

    def stopTrigger(self):
        rospy.wait_for_service('trigger/stop')
        try:
            self.stopTriggerService = rospy.ServiceProxy('trigger/stop', Empty)
            self.stopTriggerService()
            rospy.loginfo("trigger service paused")
        except:
            rospy.loginfo("trigger service pause failed")

    def startTrigger(self):
        rospy.wait_for_service('trigger/start')
        try:
            self.startTriggerService = rospy.ServiceProxy('trigger/start', Empty)
            self.startTriggerService()
            rospy.loginfo("trigger service resumed")
        except:
            rospy.loginfo("trigger service resume failed")

    def stopCommand(self):
        rospy.wait_for_service('command/stop')
        try:
            self.stopCommandService = rospy.ServiceProxy('command/stop', Empty)
            self.stopCommandService()
            rospy.loginfo("command service paused")
        except:
            rospy.loginfo("command service pause failed")

    def startCommand(self):
        rospy.wait_for_service('command/start')
        try:
            self.startCommandService = rospy.ServiceProxy('command/start', Empty)
            self.startCommandService()
            rospy.loginfo("command service resumed")
        except:
            rospy.loginfo("command service resume failed")

    def triggerParser(self, trigger):
        rospy.loginfo(rospy.get_name()+"From Trigger: %s", trigger.data)
        if trigger.data.find("wrong") > -1:
            self.stopTrigger()
            self.startCommand()
        else:
            rospy.loginfo(rospy.get_name()+" can't find the trigger word")

    def commandParser(self, command):
        rospy.loginfo(rospy.get_name()+"From Command: %s", command.data)
        # command
        if command.data.find("information") > -1:    
            self.msg = 'information'
        elif command.data.find("option") > -1:
            self.msg = 'option'
        elif command.data.find("review") > -1:
            self.msg = 'review'
        # object name
        #if command.data.find("adapter") > -1:    
        #    self.retailpub.publish(self.msg + "adapter")
        #elif if command.data.find("airborne") > -1:    
        #    self.retailpub.publish(self.msg + "airborne")
        #elif if command.data.find("boot") > -1:    
        #    self.retailpub.publish(self.msg + "boot")
        #elif if command.data.find("bottle") > -1:    
        #    self.retailpub.publish(self.msg + "bottle")
        #elif if command.data.find("cap") > -1:    
        #    self.retailpub.publish(self.msg + "cap")
        #elif if command.data.find("disk") > -1:    
        #    self.retailpub.publish(self.msg + "disk")
        #else:
        #    self.retailpub.publish(command.data)

        self.retailpub.publish(command.data)
        self.stopCommand()
        self.startTrigger()

if __name__ == '__main__':
    p = parser()
