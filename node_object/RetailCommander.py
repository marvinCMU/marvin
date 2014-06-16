#!/usr/bin/env python

"""
retailCommander.py is a wrapper for pocketsphinx
used to detect retailCommander defined
  parameters:
    ~lm - filename of language model
    ~dict - filename of dictionary
  publications:
    ~output (std_msgs/String) - text output
  services:
    ~start (std_srvs/Empty) - start speech recognition
    ~stop (std_srvs/Empty) - stop speech recognition
"""

import roslib; roslib.load_manifest('speech')
import rospy

import pygtk
pygtk.require('2.0')
import gtk

import gobject
import pygst
pygst.require('0.10')
gobject.threads_init()
import gst

from std_msgs.msg import String
from std_srvs.srv import *

class RetailCommander(object):
    """ GStreamer based speech recognizer. """

    def __init__(self):
        """ Initialize the speech pipeline components. """
        rospy.init_node('retailCommander')
        rospy.on_shutdown(self.shutdown)

        self.pub = rospy.Publisher('~output',String)

        # services to start/stop recognition
        rospy.Service("~start", Empty, self.start)
        rospy.Service("~stop", Empty, self.stop)

        # configure pipeline
        self.pipeline = gst.parse_launch('gconfaudiosrc ! audioconvert ! audioresample '
                                         + '! vader name=vad auto-threshold=true '
                                         + '! pocketsphinx name=asr ! fakesink')
        asr = self.pipeline.get_by_name('asr')
        asr.connect('partial_result', self.asr_partial_result)
        asr.connect('result', self.asr_result)
        asr.set_property('configured', True)
        asr.set_property('fwdflat', False)
        asr.set_property('bestpath', False)
        asr.set_property('dsratio', 1)
        asr.set_property('maxhmmpf', 1000)
        asr.set_property('maxwpf', 10)

        # parameters for lm and dic
        try:
            lm_ = rospy.get_param('~lm')
        except:
            rospy.logerr('<Speech> RetailCommander: Please specify a language model file')
            return
        try:
            dict_ = rospy.get_param('~dict')
        except:
            rospy.logerr('<Speech> RetailCommander: Please specify a dictionary')
            return
        asr.set_property('lm',lm_)
        asr.set_property('dict',dict_)

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect('message::application', self.application_message)
                # this node was paused initially until triggered
        #self.start(None)
        self.stop(None)
        gtk.main()

    def shutdown(self):
        """ Shutdown the GTK thread. """
        rospy.loginfo("<Speech> RetailCommander node closed")
        gtk.main_quit()

    def start( self, msg ):
        self.pipeline.set_state(gst.STATE_PLAYING)
        print '<Speech> RetailCommander node resumed'
        return EmptyResponse()

    def stop(self, msg):
        self.pipeline.set_state(gst.STATE_PAUSED)
        print '<Speech> RetailCommander node paused'
        return EmptyResponse()

    def asr_partial_result(self, asr, text, uttid):
        """ Forward partial result signals on the bus to the main thread. """
        struct = gst.Structure('partial_result')
        struct.set_value('hyp', text)
        struct.set_value('uttid', uttid)
        asr.post_message(gst.message_new_application(asr, struct))

    def asr_result(self, asr, text, uttid):
        """ Forward result signals on the bus to the main thread. """
        struct = gst.Structure('result')
        struct.set_value('hyp', text)
        struct.set_value('uttid', uttid)
        asr.post_message(gst.message_new_application(asr, struct))

    def application_message(self, bus, msg):
        """ Receive application messages from the bus. """
        msgtype = msg.structure.get_name()
        if msgtype == 'partial_result':
            self.partial_result(msg.structure['hyp'], msg.structure['uttid'])
        if msgtype == 'result':
            self.final_result(msg.structure['hyp'], msg.structure['uttid'])

    def partial_result(self, hyp, uttid):
        """ Delete any previous selection, insert text and select it. """
        print "Partial: " + hyp

    def final_result(self, hyp, uttid):
        """ Insert the final result. """
        msg = String()
        msg.data = str(hyp.lower())
        #print '************************retailCommander node: %s'%msg.data
        rospy.loginfo("<Speech> RetailCommander: %s", msg.data)
        self.pub.publish(msg)
        #self.stop(None)

if __name__=="__main__":
    r = RetailCommander()
