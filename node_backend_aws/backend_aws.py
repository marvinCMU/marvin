#!/usr/bin/env python
import roslib; roslib.load_manifest('speech')
import rospy
import sys

from std_msgs.msg import String
from std_srvs.srv import *
from time import gmtime, strftime

# Import the AWS SDK
#import simplejson, boto, uuid
#from boto.sqs.message import Message
#import boto.dynamodb
import boto.sqs
import boto.s3
from boto.s3.connection import S3Connection
from boto.s3.key import Key
from boto.sqs.message import Message
from boto.sqs.message import RawMessage
import boto.dynamodb
import boto.dynamodb2
from boto.dynamodb2.table import Table
import boto

class BackendAWSManager(object):

    def __init__(self):
        rospy.init_node('backend_aws')
        rospy.on_shutdown(self.shutdown)
	
	# publisher
        self.queueMsgPub = rospy.Publisher("backend/queueMsg", String)
        self.dbMsgPub = rospy.Publisher("backend/dbMsg", String)
        self.fileMsgPub = rospy.Publisher("backend/fileMsg", String)

	# subcriber
        rospy.Subscriber("backend/getQueue", String, self.getQueue)
	rospy.Subscriber("backend/setQueue", String, self.setQueue)
	rospy.Subscriber("backend/getDB", String, self.getDB)
	rospy.Subscriber("backend/setDB", String, self.setDB)
	rospy.Subscriber("backend/getFile", String, self.getFile)

	# aws connection
	self.s3 = boto.connect_s3() # connection to s3
	self.sqs = boto.sqs.connect_to_region('us-west-2') # connection to SQS
	self.dynamodb = boto.dynamodb.connect_to_region('us-west-2') # connection to dynamoDB

        rospy.spin()

    def shutdown(self):
        rospy.loginfo("<AWS> backend_aws node closed")

    def getQueue(self, message):
	# set queue for test
	rospy.loginfo("<AWS> getQueue:" + str(message.data));
	q = self.sqs.get_queue('MarvinToDo')
	m = q.read()
	if m is not None:
		queueMsg = str(m.get_body())
		q.delete_message(m)
		self.queueMsgPub.publish(queueMsg)
		rospy.loginfo("\n" + queueMsg)
		rospy.loginfo("<AWS> getQueue: Done ================\n\n")
	else :
		rospy.loginfo("<AWS> getQueue: empty : Done ================\n\n")

    def setQueue(self, message):
	rospy.loginfo("<AWS> setQueue:\n" + str(message.data));
	# parse message
	parse_msg = str(message.data);
	parse_msg = parse_msg.split('#')
	#print parse_msg
	#print parse_msg[0]
	if parse_msg[0] == 'test' :
		rospy.loginfo("<AWS> test message :\n" + parse_msg[1]);
		q = self.sqs.create_queue('MarvinToDo')
		m = Message()
		m.set_body(parse_msg[1])
		#m.set_body('2L_10011.bmp\n2L_10011\ngive me review for shampoo bottle')
		status = q.write(m)		
	else :
		rospy.loginfo("<AWS> creat user queue for final result : " + parse_msg[0]);
		rospy.loginfo("<AWS> message :\n" + parse_msg[1]);
		q = self.sqs.create_queue(parse_msg[0])
		m = Message()
		m.set_body(parse_msg[1])
		status = q.write(m)
	rospy.loginfo("<AWS> setQueue: Done ================\n\n");
        
    def getDB(self, message):
	rospy.loginfo("<AWS> getDB: " + str(message.data))

	print self.dynamodb.list_tables()

	table = self.dynamodb.get_table('marvin_cloud')
	item = table.get_item(hash_key='2L_10001')
	print item
	
	dbMsg = str(message.data) + ' ' + str("2L_10011.bmp")
	self.dbMsgPub.publish(dbMsg)
	rospy.loginfo("<AWS> getDB: Done : " + dbMsg + " ================\n\n");

    def createDB(self, message):
	message_table_schema = self.dynamodb.create_schema(
		hash_key_name='User ID',
		hash_key_proto_value=str)

	table = self.dynamodb.create_table(
	name='marvin_cloud',
	schema=message_table_schema,
	read_units=10,
	write_units=10)

	print self.dynamodb.list_tables()    


    def setDB(self, message):
	rospy.loginfo("<AWS> setDB: " + str(message.data))

	table = self.dynamodb.get_table('marvin_cloud')
	item_data = {
	        'Image Name': '2L_10001.bmp',
        	'Resolution': '640x480',
		'Speech Command' : 'give review for shampoo bottle',
		'Detected Result' : 'detergent, wipe, lock',
		'User feedback' : 'wipe'}
	item = table.new_item(
	        hash_key='2L_10001',
	        attrs=item_data)
	item.put()

    def getFile(self, message):
	rospy.loginfo("<AWS> getFile: " + str(message.data))
	key = self.s3.get_bucket('marvinsbucket').get_key(str(message.data))
	key.get_contents_to_filename('/home/mmfps/mmfps/mmfpspkg/node_backend_aws/img/' + str(message.data))
	self.fileMsgPub.publish(str(message.data))
	rospy.loginfo("<AWS> getFile: Done : " + str(message.data) + " ================\n\n")


if __name__ == '__main__':
    d = BackendAWSManager()

