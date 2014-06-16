#!/usr/bin/env python

"""
talkback.py - Say back what is heard by the command recognizer.
"""

import roslib; roslib.load_manifest('speech')
import rospy

from std_msgs.msg import String
from sound_play.libsoundplay import SoundClient

class TalkBack:

    def __init__(self):
        rospy.init_node('talkback')
        rospy.on_shutdown(self.shutdown)

        # check whether this node activated
        try:
            acti = rospy.get_param("talkbackActive")
        except:
            acti = True
        if acti == False:
            rospy.loginfo("<Speech> Talkback node deactivated")
            return
        rospy.loginfo("<Speech> Talkback node activated")

        self.voice = rospy.get_param("~voice", "voice_don_diphone")
        self.wavepath = rospy.get_param("~wavepath", "")
        
        # Create the sound client object
        self.soundhandle = SoundClient()
        
        rospy.sleep(1)
        self.soundhandle.stopAll()
        
        # Announce that we are ready for input
        self.soundhandle.playWave(self.wavepath)
        rospy.sleep(1)
        #self.soundhandle.say("Ready", self.voice)

        # Subscribe to the recognizer output
        rospy.Subscriber('/retailCommander/output', String, self.talkback)
        rospy.Subscriber('/faceCommander/output', String, self.talkback)
        rospy.spin()

    def shutdown(self):
        rospy.loginfo("<Speech> Talkback node closed")
        
    def talkback(self, msg):
        # Print the recognized words on the screen
        rospy.loginfo(msg.data)
        
        # Speak the recognized words in the selected voice
        self.soundhandle.say(msg.data, self.voice)
        
        # Uncomment to play one of the built-in sounds
        #rospy.sleep(2)
        #self.soundhandle.play(5)
        
        # Uncomment to play a wave file
        #rospy.sleep(2)
        #self.soundhandle.playWave(self.wavepath + "/R2D2a.wav")

if __name__=="__main__":
    t = TalkBack()
