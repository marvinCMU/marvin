#!/usr/bin/env python
import roslib; roslib.load_manifest('speech')
import rospy
import sys

from std_msgs.msg import String
from std_srvs.srv import *
from time import gmtime, strftime

class DialogManager(object):

    def __init__(self):
        rospy.init_node('dialogManager')
        rospy.on_shutdown(self.shutdown)

        self.state = 0
        self.libPath = rospy.get_param('speechLibPath')
        self.retailCmdPub = rospy.Publisher('~retailcmd', String)
        self.retailObjPub = rospy.Publisher('~retailobj', String)
        self.faceCmdPub = rospy.Publisher('~facecmd', String)
        self.faceObjPub = rospy.Publisher('~faceobj', String)
        rospy.Subscriber("trigger/output", String, self.triggerParser)
        rospy.Subscriber("retailCommander/output", String, self.retailParser)
        rospy.Subscriber("faceCommander/output", String, self.faceParser)

        #taylors edits
        self.retailBnetPub = rospy.Publisher( "dialog/retail", String )
        self.faceBnetPub = rospy.Publisher( "dialog/face", String)
        
        #general speech output:
        self.generalPub = rospy.Publisher( "dialog/general", String)

        rospy.Subscriber("bnet/face/speech", String, self.face_bnet_callback) 
        rospy.Subscriber("bnet/retail/speech", String, self.retail_bnet_callback)

        #self.retailGuiPub = rospy.Publisher( "retailGuiSpeech", String )
        #self.faceGuiPub = rospy.Publisher( "faceGuiSpeech", String)
        self.commandPublisher = rospy.Publisher( "dialog/command", String );

          
        # the Commands and Objects are hard coded for now
        self.triggerWord = 'marvin'
        self.retailCommands = ["information", "option", "review"]
        self.retailObjects = ["adapter", "airborne", "boot", "bottle", "cap", "disk", "halls", "powder", "toy", "tumbler", "tupperware", "umbrella"]
        self.faceCommands = ["where", "get_label", "retrain", "take_data"]
        self.faceObjects = ["denver", "fanyi", "qifan"]
        rospy.spin()

    def shutdown(self):
        rospy.loginfo("<Speech> DialogManager node closed")

    # pause and resume speech trigger recognizer
    def stopTrigger(self):
        rospy.wait_for_service('trigger/stop')
        try:
            self.stopTriggerService = rospy.ServiceProxy('trigger/stop', Empty)
            self.stopTriggerService()
            #rospy.loginfo("<Speech> Trigger service paused")
        except:
            rospy.loginfo("<Speech> Trigger service pause failed")
    def startTrigger(self):
        rospy.wait_for_service('trigger/start')
        try:
            self.startTriggerService = rospy.ServiceProxy('trigger/start', Empty)
            self.startTriggerService()
            #rospy.loginfo("<Speech> Trigger service resumed")
        except:
            rospy.loginfo("<Speech> Trigger service resume failed")
            
    # pause and resume speech retail commander
    def stopRetailCommander(self):
        rospy.wait_for_service('retailCommander/stop')
        try:
            self.stopCommandService = rospy.ServiceProxy('retailCommander/stop', Empty)
            self.stopCommandService()
            #rospy.loginfo("<Speech> RetailCommander service paused")
        except:
            rospy.loginfo("<Speech> RetailCommander service pause failed")
    def startRetailCommander(self):
        rospy.wait_for_service('retailCommander/start')
        try:
            self.startCommandService = rospy.ServiceProxy('retailCommander/start', Empty)
            self.startCommandService()
            #rospy.loginfo("<Speech> RetailCommander service resumed")

        except:
            rospy.loginfo("<Speech> RetailCommander service resume failed")

    # pause and resume speech face commander
    def stopFaceCommander(self):
        rospy.wait_for_service('faceCommander/stop')
        try:
            self.stopFaceService = rospy.ServiceProxy('faceCommander/stop', Empty)
            self.stopFaceService()
            #rospy.loginfo("<Speech> FaceCommander service paused")
        except:
            rospy.loginfo("<Speech> FaceCommander service pause failed")
    def startFaceCommander(self):
        rospy.loginfo("starting face command")
        rospy.wait_for_service('faceCommander/start')
        try:
            self.startFaceService = rospy.ServiceProxy('faceCommander/start', Empty)
            self.startFaceService()
            #rospy.loginfo("<Speech> FaceCommander service resumed")
        except:
            rospy.loginfo("<Speech> FaceCommander service resume failed")
            
    # pause and resume face recognizer
    def stopFace(self):
        rospy.wait_for_service('face/stop')
        try:
            self.stopFaceService = rospy.ServiceProxy('face/stop', Empty)
            self.stopFaceService()
        except:
            pass
    def startFace(self):
        rospy.wait_for_service('face/start')
        try:
            self.startFaceService = rospy.ServiceProxy('face/start', Empty)
            self.startFaceService()
        except:
            pass

    # pause and resume location recognizer
    def stopLocation(self):
        rospy.wait_for_service('location/stop')
        try:
            self.stopLocationService = rospy.ServiceProxy('location/stop', Empty)
            self.stopLocationService()
        except:
            pass
    def startLocation(self):
        rospy.wait_for_service('location/start')
        try:
            self.startLocationService = rospy.ServiceProxy('location/start', Empty)
            self.startLocationService()
        except:
            pass

    # parse output from trigger node
    def triggerParser(self, trigger):
        if trigger.data.find(self.triggerWord) > -1:
            self.stopTrigger()
            self.startFaceCommander()
            self.startRetailCommander()
            # trigger visual recognition nodes
            #self.startLocation()   # this will trigger the object recognizer
            #self.startFace()

    # parse output from speech retail commander
    def retailParser(self, command):
        self.retailBnetPub.publish( command.data )
	
	print("[ INFO] [" + str(rospy.get_rostime().to_sec()) + "]: REPORT:OUT:SPEECH:dialog/general:" + str(command.data))
        
        #TODO: Hack: assume retail lm is the same as the general lm:
        self.generalPub.publish(command.data)

        self.stopRetailCommander()
        self.startTrigger()
        
        
    # parse output from speech face commander
    def faceParser(self, command):
        #rospy.loginfo("face parser called ");
        self.faceBnetPub.publish( command.data );
        self.stopFaceCommander()
        self.startTrigger()
   

    
    def face_bnet_callback(self, command):
        self.commandPublisher.publish( "good words" );
 
    def retail_bnet_callback( self, command ):
        self.commandPublisher.publish( "good words" );
        #self.retailGuiPub( command.data );
        


if __name__ == '__main__':
    d = DialogManager()
"""
        rospy.loginfo("CHANGED ! " + str(command.data) ) 
         
        cmdArray = ['0'] * len(self.faceCommands)
        objArray = ['0'] * len(self.faceObjects)
        # command name cases
        for i in range(0, len(self.faceCommands)):
            if (command.data.find(self.faceCommands[i]) > -1):
                cmdArray[i] = '1'

        cmdMessage = ' '.join(cmdArray)
        rospy.loginfo("<Speech> Face command message: " + cmdMessage);
        # object name
        for i in range(0, len(self.faceObjects)):
            if (command.data.find(self.faceObjects[i]) > -1):
                objArray[i] = '1'
        objMessage = ' '.join(objArray)
        rospy.loginfo("<Speech> Face object message: " + objMessage);

        ccc = 'bad words'
        #if (cmdArray[0]=='1'):
         #   ccc = 'good words'
                
        #self.faceCmdPub.publish(cmdMessage)
        self.faceGuiPub.publish( ccc )
        rospy.sleep(1)
        #self.faceObjPub.publish(objMessage)
"""

        
"""
        cmdArray = ['0'] * len(self.retailCommands)
        objArray = ['0'] * len(self.retailObjects)
        # command name cases
        for i in range(0, len(self.retailCommands)):
            if (command.data.find(self.retailCommands[i]) > -1):
                cmdArray[i] = '1'
        # other cases
        if (command.data.find("think") > -1):
            cmdArray[0] = '1'
        if (command.data.find("like") > -1):
            cmdArray[0] = '1'
        if (command.data.find("show") > -1):
            cmdArray[0] = '1'
        if (command.data.find("give") > -1):
            cmdArray[0] = '1'
        cmdMessage = ' '.join(cmdArray)
        rospy.loginfo("<Speech> Retail command message: " + cmdMessage)
        # object name
        for i in range(0, len(self.retailObjects)):
            if (command.data.find(self.retailObjects[i]) > -1):
                objArray[i] = '1'
        objMessage = ' '.join(objArray)
        rospy.loginfo("<Speech> Retail object message: " + objMessage)
        #self.retailCmdPub.publish(cmdMessage)
        #rospy.sleep(1)
        #self.retailObjPub.publish(objMessage)
        # save the result to files
        ccc = ''
        if (cmdArray[0]=='1'):
            ccc = 'information'
        elif (cmdArray[1]=='1'):
            ccc = 'option'
        elif (cmdArray[2]=='1'):
            ccc = 'review'
        ooo = ''
        for i in range(0, len(self.retailObjects)):
            if (objArray[i] == '1'):
                ooo = self.retailObjects[i]
        if (ccc!='' and ooo!=''):
            timestamp = strftime ("s_%H_%M_%S.txt", gmtime())
            speechFile = open("/home/mmfps/mmfps/mmfpspkg/data_tmp/speech/" + timestamp,"w")
            
            speechFile.write(ccc)
            speechFile.write('\n')
            speechFile.write(ooo)
            self.retailCmdPub.publish(ccc)
            self.retailObjPub.publish(ooo)
            speechFile.close()
            rospy.loginfo("<Speech> Retail file saved at: " + timestamp)
            #taylors edits - switch to thinking state here
            word_type = "good words";
        else:
     #taylors edits - publish couldn't understand here
            word_type = "bad words";

        self.retailGuiPub.publish( word_type );
"""

