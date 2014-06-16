// system library
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <vector>
#include <algorithm>
// GTK Library
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
// OpenCV Library
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
//system include
#include "readObject.h"
#include "systemParams.h"
//ros include
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/CvBridge.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>

// uncomment and rebuild when using the first person camera from Prof. Kanade
// it will rotate the image 180 degree and resize the image to 640x480
//#define FP_CAM

//how many pictures we will publish when gui is listening
#define PICTURES_TO_SEND 3
// (on the tablet) {0 = back, 1 = front}
#define CAMERA_NUMBER 1
//the size ratio of the original image to the video stream size
#define IMAGE_SIZE_RATIO_MULTIPLIER 1.5



//The muliplier for the original video stream size
float image_size_ratio_multiplier = IMAGE_SIZE_RATIO_MULTIPLIER;

//the image capture object
CvCapture *cap;

/**the widgets*/
GtkWidget *window = NULL;
//video stream (left side)
GtkWidget *video_stream_widget = NULL;
//results (right side)
GtkWidget *drawing_area = NULL;
//fixed area, where the left and right side are placed
GtkWidget *fixed_area = NULL;
//text entry, for taking data
GtkWidget *label_entry = NULL;
//widget for a popup
GtkWidget * popup = NULL;


//properties of the image (width, height) to be displayed
int image_height = 0;
int image_width = 0;

//properties of the window
int window_width = 0;
int window_height = 0;

//size of the label for the training data
int label_width = 150, label_height = 50;

//size of the popup label
int popup_width = 800, popup_height = 50;


//image publisher that sends the pictures from the camera
image_transport::Publisher visionPublisher;
//publishes the state of the gui
ros::Publisher guiStatePublisher;
//publishes the label to the face trainer
ros::Publisher training_label_publisher;
//tells the face and reatil reconginzers if we have an answer or not
ros::Publisher got_face_answer;
ros::Publisher got_retail_answer;
ros::Publisher take_data_publisher;
//tells the bnet to change the mode from face detection to retail detection
ros::Publisher change_detection_mode;

//the image that the gui displays when it is "thinking"
cv::Mat analyzed_image;

//keeps track of the amount of time left for the misunderstanding
//message to be displayed on the gui
int misunderstanding_pause_timer = 0;
//boolean to track if the misunderstanding timer has been set
bool  misunderstanding_pause_set = false;
//sets the default misunderstanding pause time
const int default_misunderstanding_pause_time = 30;


//keeps track of the amount of time left for the
//results to be shown on the gui
int results_pause_timer = 0;
//boolean to track if the timer has been set
bool  results_pause_set = false;
//default time to show results
const int default_results_pause_time = 30;

//log info- this is the info that is used to determine
//what is supposed to be displayed on the gui
//current timestamp to store in the log
std::string current_timestamp;
//object that was detected
std::string object;
//the words that the program heard
std::string words_spoken;
//the bounding box to circle
int bounding_box[4] = {0, 0, 0, 0};
//the boundinx box for the training face data
int train_bounding_box[4] = {0, 0, 0, 0};

//the number of the camera to use
#ifdef FP_CAM
int camera_number = 2;
#else
// (on the tablet) {0 = back, 1 = front}
<<<<<<< .merge_file_Y8Xetd
int camera_number = CAMERA_NUMBER;
#endif
=======
int camera_number = 0;
>>>>>>> .merge_file_RMpKBc

//trigger to change the state to LISTENING
std::string trigger = "intelligentsia";

//used to keep track of the number of pictures we still have to send
int pics_to_send = PICTURES_TO_SEND;
//tells us if we should be sending pictures or not
bool send_pics = true;
//tells us if the face detector is ready to recieve a picture
bool face_detector_ready = true;
//handle for the window keypress signal
int window_keypress_handle = 0;
//handle for the popup window keypress
int popup_keypress_handle = 0;
//the label for the training data
std::string training_label;
//tells us if wer're ready to save another face
bool take_data_ready = true;
//tells the gui if a face was found for training purposes
bool found_face = false;
//tells if we are done taking data or not
bool done_taking_data = false;
//confirmed correct id
bool confirmed_correct = false;
//confirmed incorredt id
bool confirmed_incorrect = false;
//mat for the image recogintion result
cv::Mat reco_result;

//enum that defines the possible states of the gui
enum State{
  WAITING_FOR_TRIGGER,
  LISTENING,
  THINKING,
  SHOWING_RESULT,
  MISUNDERSTANDING,
  I_GOT_NOTHING,
  TAKING_DATA,
  SUSPEND,
  CONFIRM_FACE,
} state;

//enum that defines the format of the log string that
//is sent to the gui.  Each item represents a line
//expected to be in the message string sent to the
//gui for the final result
enum Final_Result_Message_Format{
  TIME_STAMP,
  WORDS_SPOKEN,
  OBJECT,
  WORD_IDENTIFIED,
  BB_X1,
  BB_Y1,
  BB_X2,
  BB_Y2
};

enum Face_Feedback{
  INITIAL_FEEDBACK,
  NEW_PERSON,
  KNOWN_PERSON,
} face_feedback_state;

/** Resizes the image
 * This isn't used for the most part, but if need be
 * a key event can be added to resize the image
 */
void resize_image(){
  std::cout<<"Resize called!"<<std::endl;
  image_width = floor((float)image_width * .25);
  image_height = floor((float)image_height * .25);
}

void label_enter_callback( GtkWidget * widget,
			   GtkWidget *entry ){
  const gchar *label_entry_text;
  label_entry_text = gtk_entry_get_text( GTK_ENTRY( entry ) );
  g_signal_handler_unblock( G_OBJECT( window ), window_keypress_handle );
  gtk_widget_hide( GTK_WIDGET( entry ) );
  training_label = label_entry_text;
  std::transform( training_label.begin(),
		   training_label.end(),
		   training_label.begin(),
		   ::tolower );
  gtk_entry_set_text( GTK_ENTRY( entry ), "");
  ROS_INFO("You entered: %s", training_label.c_str() );

  std_msgs::String training_label_msg; 
  training_label_msg.data = training_label;
  //training_label_publisher.publish( training_label_msg );

  state = TAKING_DATA;
}

/**Hanldes any keypresses
 * keys:
 * escape - quit the gui
 * s - resize the image
 * 1 - change the state to "WAITING FOR TRIGGER"
 * 2 - change the state to "LISTENING"
 * 3 - change the state to "THINKING"
 * c - change the camera from front to back (doesn't work yet)
 */
gboolean handle_keypress(GtkWidget *widget, GdkEventKey *event, gpointer data){
  switch( event->keyval ){
  case GDK_KEY_Escape: gtk_main_quit(); break;
  case GDK_KEY_s: resize_image(); break;
  case GDK_KEY_1:
    state =  WAITING_FOR_TRIGGER;
    gtk_widget_queue_draw( GTK_WIDGET( drawing_area ));
    break;
  case GDK_KEY_2: state = LISTENING; break;
  case GDK_KEY_3: state = THINKING; break;
  case GDK_KEY_c:
    if(camera_number == 0 ) camera_number = 1;
    else camera_number = 0;
    break;
  case GDK_KEY_4:
    {
      state = SUSPEND;
      g_signal_handler_block( G_OBJECT( window ), window_keypress_handle );
      gtk_widget_show( label_entry );
      gtk_widget_grab_focus( label_entry );
    }
    break;
  case GDK_KEY_m:
    {
    std_msgs::String change_mode_msg;
    change_detection_mode.publish( change_mode_msg );
    }
    break;
  default: break;
  }
}

gboolean popup_handle_keypress( GtkWidget *widget, GdkEventKey *event, gpointer data ){
  switch( event->keyval ){
  case GDK_KEY_y:{
    ROS_INFO("confrimed correct");
    confirmed_correct = true;
  }
    break;
  case GDK_KEY_n:{
    ROS_INFO("confirmed incorrect");
    confirmed_incorrect = true;
  }
  default: break;
  }
}

/**Function that is called every gtk iteration to
 * stream the video
 */
gboolean stream_video(GtkWidget *widget, GdkEventExpose *event, gpointer data){
  //queue the widget to be drawn again after the function is over
  gtk_widget_queue_draw( GTK_WIDGET( widget ) );

  //cairo object to draw things to
  cairo_t *cr;
  cr = gdk_cairo_create( widget->window );
  //picture buffer to hold the image data from opencv
  GdkPixbuf *pixbuf;
#ifdef FP_CAM
  //flip the image [iljoo]
  IplImage* original_img = cvQueryFrame(cap);
  IplImage* fliped_img = cvCloneImage(original_img);
  cvFlip(original_img, fliped_img, -1);
  //capture the original frame
  cv::Mat frame_org(fliped_img);
#else
  //capture the original frame
  cv::Mat frame_org(cvQueryFrame(cap));
#endif

  //the resized version of the original frame
  cv::Mat frame_resize(image_height,
		       image_width,
		       CV_8U);
  //convert the frame to 8 bit color (only thing accepted by gtk)
  frame_org.convertTo(frame_org, CV_8U);
  //resize the frame to bet the requested size
  cv::resize(frame_org, frame_resize, frame_resize.size(), 0, 0, cv::INTER_CUBIC);
  //the frame we will be showing
  cv::Mat frame;
  //convert the color from RGB to BGR
  cv::cvtColor(frame_resize, frame, CV_RGB2BGR);

  //copy that data into the picture buffer
  pixbuf = gdk_pixbuf_new_from_data((guchar*)frame.data, GDK_COLORSPACE_RGB, FALSE,
    				    8, frame.cols, frame.rows, frame.step, NULL, NULL);

  //set the source to that picture buffer to show the image
  gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
  cairo_paint( cr );
  cairo_destroy( cr );

  //if ros has quit, the exit the gui
  if(!ros::ok()){
    gtk_main_quit();
    return TRUE;
  }

  //spin it
  ros::spinOnce();
}

/**This function determines what to draw on the "state" side of the gui
 * One widget of the gui is used for displaying the video stream, where as
 * this widget is used to display the state/results of the analysis
 */
gboolean annotate_image(GtkWidget *widget, GdkEventExpose *event, gpointer data){

  //cairo object to draw stuff to
  cairo_t *cr;
  cr = gdk_cairo_create( widget->window );

  //pixbuf to hold our image data to display
  GdkPixbuf *pixbuf;
#ifdef FP_CAM
  //flip the image [iljoo]
  IplImage* original_img = cvQueryFrame(cap);
  IplImage* fliped_img = cvCloneImage(original_img);
  cvFlip(original_img, fliped_img, -1);
  //capture the original frame
  cv::Mat frame_org(fliped_img);
#else
  //capture the original frame
  cv::Mat frame_org(cvQueryFrame(cap));
#endif
  //create the resized image container
  cv::Mat frame_resize(image_height,
		       image_width,
		       CV_8U);
  //convert to 8 bit format
  frame_org.convertTo(frame_org, CV_8U);
  //resize the image
  cv::resize(frame_org, frame_resize, frame_resize.size(), 0, 0, cv::INTER_CUBIC);
  //the final image
  cv::Mat frame;
  //copy data to the final image with the correct color format
  cv::cvtColor(frame_resize, frame, CV_RGB2BGR);
  //stuff to write text in cairo
  cairo_text_extents_t extents;
  //the frame to displayed the "blurred" image
  cv::Mat frame_blur;

  //message of the state
  std_msgs::String state_msg;


  
  switch(state){
  case WAITING_FOR_TRIGGER:
    {
      //create the blurred image
      cv::blur( frame, frame_blur, cv::Size( 70,70 ) );

      //send it to the pixbuff and show it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)frame_blur.data, GDK_COLORSPACE_RGB, FALSE,
					8, frame_blur.cols, frame_blur.rows, frame_blur.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the text on the screen
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //make sure the text is in the center
      cairo_text_extents( cr, "Waiting for trigger", &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //show the text
      cairo_show_text( cr, "Waiting for trigger");

      //queue the function for drawing during next iteration
      //this gives us the "stream video" effect
      gtk_widget_queue_draw( GTK_WIDGET( widget ));

      //let everyone know we don't have an answer yet
      state_msg.data = std::string( "waiting" );
      guiStatePublisher.publish( state_msg );

      //reset the pictures to send
      pics_to_send = PICTURES_TO_SEND;
      send_pics = true;


    }
    break;

  case LISTENING:
    {
      //if we still have pictures to send
      if( send_pics )
	{
	  if( face_detector_ready ){
#ifdef FP_CAM
	    //flip and resize the image [iljoo]
	    IplImage* original_img = cvQueryFrame(cap);
	    IplImage* fliped_img = cvCloneImage(original_img);
	    IplImage *resized_img; //Resize image to small size
	    //flip the image
	    cvFlip(original_img, fliped_img, -1);
	    resized_img = cvCreateImage(cvSize(image_width,image_height), 8, 3);
	    //resize the flipped frame
	    cvResize(fliped_img, resized_img);
	    //send the image we got
	    sensor_msgs::ImagePtr msg = sensor_msgs::CvBridge::cvToImgMsg(resized_img, "bgr8");
	    visionPublisher.publish(msg); /**CHANGE THIS**/
#else
	    //get the image
	    IplImage* img = cvQueryFrame(cap);
	    cv::Mat frame_test = img;
	    
	    //send the image we got
	    //sensor_msgs::ImagePtr msg = sensor_msgs::CvBridge::cvToImgMsg(img, "bgr8");
	    cv_bridge::CvImage gui_msg( std_msgs::Header(),
					      sensor_msgs::image_encodings::BGR8,
					      frame_test );
	    visionPublisher.publish( gui_msg.toImageMsg());
#endif
	    ROS_INFO("GUI SENT IMAGE!");

	    //set the analyzed image frame.  This is the frame
	    //we are going to display during the "THINKING" state
	    //just cosmetic
	    analyzed_image = frame;
	    //ROS_INFO( "Sending pic %d", pics_to_send );
	    pics_to_send--;
	    if( pics_to_send <= 0 ){
	      send_pics = false;
	      //state = THINKING;
	      //ROS_INFO( "Sent all the pics" );
	    }
	    face_detector_ready = false;
	  }
	}

      //display the image we got
      pixbuf = gdk_pixbuf_new_from_data((guchar*)frame.data, GDK_COLORSPACE_RGB, FALSE,
					8, frame.cols, frame.rows, frame.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the text
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center it
      cairo_text_extents( cr, "Listening", &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //show the text
      cairo_show_text( cr, "Listening");

      //queue draw for the stream effect
      gtk_widget_queue_draw( GTK_WIDGET( widget ));

    }
    break;

  case THINKING:{
    state_msg.data = std::string( "thinking" );
    guiStatePublisher.publish( state_msg );
    //if we still have pictures to send
      if( send_pics  )
	{
	  if( face_detector_ready ){
	    //get the image
	    IplImage* img = cvQueryFrame(cap);
	    //send the image we got
	    sensor_msgs::ImagePtr msg = sensor_msgs::CvBridge::cvToImgMsg(img, "bgr8");
	    visionPublisher.publish(msg); /**Change this**/
	    ROS_INFO("GUI SENT IMAGE!");
	    //set the analyzed image frame.  This is the frame
	    //we are going to display during the "THINKING" state
	    //just cosmetic
	    analyzed_image = frame;
	    pics_to_send--;
	    if( pics_to_send <= 0 )
	      send_pics = false;
	  } 
	}
      else{
	//state = I_GOT_NOTHING;
	gtk_widget_queue_draw( GTK_WIDGET( widget ));
      }
    
    //show the image in black and white
      cv::Mat frame_bw_org;
      //create the black and white container (note that the image_width
      //has to be multipled by 3, I don't know why but it is too small otherwise
      cv::Mat frame_bw(image_height,
		       image_width*3,
		       CV_8U);
      //copy the data
      cv::cvtColor(analyzed_image, frame_bw_org, CV_BGR2GRAY);
      //resize it
      cv::resize(frame_bw_org, frame_bw, frame_bw.size(), 0, 0, cv::INTER_CUBIC);

      //set up the pixbuff for drawing and draw it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)frame_bw.data, GDK_COLORSPACE_RGB, FALSE,
					8, frame_bw.cols/3, frame_bw.rows, frame_bw.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //create the text
      cairo_set_source_rgb(cr, .6, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center it
      cairo_text_extents( cr, "Thinking", &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //draw the text
      cairo_show_text( cr, "Thinking");
  }
    break;

  case SHOWING_RESULT:
    {

      //get the analyzed image we sent and draw it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)analyzed_image.data, GDK_COLORSPACE_RGB, FALSE,
					8, analyzed_image.cols, analyzed_image.rows, analyzed_image.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the bouding box color
      cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
      //set the bounding box line width
      cairo_set_line_width(cr,6); 
      //create a rectangle with the specified bounding box and draw it on the screen
      cairo_rectangle (cr,
		       bounding_box[0], bounding_box[1],
		       bounding_box[2]-bounding_box[0], bounding_box[3]-bounding_box[1]);
      //draw the rectangle
      cairo_stroke(cr);

      //set the text on the screen to be the identified object
      std::string message = object;
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center the text
      cairo_text_extents( cr, message.c_str(), &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //draw the text
      cairo_show_text( cr, message.c_str());

      //queue draw for the next event
      gtk_widget_queue_draw( GTK_WIDGET( widget ));

      //stay in this state for a certain amount of time, specified
      //by default results pause time variable
      if(results_pause_timer <= 0 && !results_pause_set){
	results_pause_timer = default_results_pause_time;
	results_pause_set = true;
      }
      if(results_pause_timer > 0){
	results_pause_timer-= 1;
      }else{
	state = WAITING_FOR_TRIGGER;
	results_pause_set = false;
	results_pause_timer = 0;
      }

      //reset the pictures to send
      pics_to_send = PICTURES_TO_SEND;
      send_pics = true;
    }
    break;


  case MISUNDERSTANDING:

    //set the pixbuff to the stream data and draw it
    pixbuf = gdk_pixbuf_new_from_data((guchar*)frame.data, GDK_COLORSPACE_RGB, FALSE,
    				    8, frame.cols, frame.rows, frame.step, NULL, NULL);
    gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
    cairo_paint( cr );

    //set the text
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Pursia",
			   CAIRO_FONT_SLANT_ITALIC,
			   CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size( cr, 28 );
    //center the text
    cairo_text_extents( cr, "I didn't understand", &extents);
    cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
    //show the text
    cairo_show_text( cr, "I didn't understand");

    //queue draw for the next frame
    gtk_widget_queue_draw( GTK_WIDGET( widget ));

    //stay in this state for a certain period of time
    //specified by the member variable above
    if(misunderstanding_pause_timer <= 0 && !misunderstanding_pause_set){
      misunderstanding_pause_timer = default_misunderstanding_pause_time;
      misunderstanding_pause_set = true;
    }
    if(misunderstanding_pause_timer > 0){
      misunderstanding_pause_timer-= 1;
    }else{
      state = WAITING_FOR_TRIGGER;
      misunderstanding_pause_set = false;
      misunderstanding_pause_timer = 0;
    }
    break;

    //we didn't find a face or an object
  case I_GOT_NOTHING:
    {
      //set the pixbuff to the stream data and draw it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)frame.data, GDK_COLORSPACE_RGB, FALSE,
					8, frame.cols, frame.rows, frame.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the text
      cairo_set_source_rgb(cr, 0, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center the text
      cairo_text_extents( cr, "I couldn't find any face or object", &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //show the text
      cairo_show_text( cr, "I couldn't find any face or object");

      //queue draw for the next frame
      gtk_widget_queue_draw( GTK_WIDGET( widget ));

      //stay in this state for a certain period of time
      //specified by the member variable above
      if(misunderstanding_pause_timer <= 0 && !misunderstanding_pause_set){
	misunderstanding_pause_timer = default_misunderstanding_pause_time;
	misunderstanding_pause_set = true;
      }
      if(misunderstanding_pause_timer > 0){
	misunderstanding_pause_timer-= 1;
      }else{
	state = WAITING_FOR_TRIGGER;
	misunderstanding_pause_set = false;
	misunderstanding_pause_timer = 0;
      }

      //reset the pictures to send
      pics_to_send = PICTURES_TO_SEND;
      send_pics = true;

    }
    break;
    
    //were meeting someone new
  case TAKING_DATA:
    {

      //get the analyzed image we sent and draw it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)analyzed_image.data, GDK_COLORSPACE_RGB, FALSE,
					8, analyzed_image.cols, analyzed_image.rows, analyzed_image.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the text on the screen to be the identified object
      std::string message = training_label;
      message[0] += 'A'-'a';
      message = std::string("Taking data....");
      cairo_set_source_rgb(cr, .2, .8, .2);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center the text
      cairo_text_extents( cr, message.c_str(), &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //draw the text
      cairo_show_text( cr, message.c_str());

      //set the bouding box color
      cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
      //set the bounding box line width
      cairo_set_line_width(cr,6); 
      //create a rectangle with the specified bounding box and draw it on the screen
      cairo_rectangle (cr,
		       train_bounding_box[0], train_bounding_box[1],
		       train_bounding_box[2]-train_bounding_box[0],
		       train_bounding_box[3]-train_bounding_box[1]);
      //draw the rectangle
      cairo_stroke(cr);
      //reset found face to false

      //if we still have pictures to send
      if( send_pics  )
	{
	  if( face_detector_ready ){
	    //get the image
	    IplImage* img = cvQueryFrame(cap);
	    //send the image we got
	    sensor_msgs::ImagePtr msg = sensor_msgs::CvBridge::cvToImgMsg(img, "bgr8");
	    visionPublisher.publish(msg);
	    ROS_INFO("GUI SENT IMAGE!");
	    //set the analyzed image frame.  This is the frame
	    //we are going to display during the "THINKING" state
	    //just cosmetic
	    analyzed_image = frame;
	    face_detector_ready = false;
	  } 
	}
      //queue draw for the next event
      gtk_widget_queue_draw( GTK_WIDGET( widget ));
    }

    if( done_taking_data ){
      state = WAITING_FOR_TRIGGER;
      pics_to_send = PICTURES_TO_SEND;
      send_pics = true;
      done_taking_data = false;

    }
    break;

  case SUSPEND:
    {
      if(analyzed_image.empty() ){
	analyzed_image = frame;
      }
    //show the image in black and white
      cv::Mat frame_bw_org;
      //create the black and white container (note that the image_width
      //has to be multipled by 3, I don't know why but it is too small otherwise
      cv::Mat frame_bw(image_height,
		       image_width*3,
		       CV_8U);
      //copy the data
      cv::cvtColor(analyzed_image, frame_bw_org, CV_BGR2GRAY);
      //resize it
      cv::resize(frame_bw_org, frame_bw, frame_bw.size(), 0, 0, cv::INTER_CUBIC);

      //set up the pixbuff for drawing and draw it
      pixbuf = gdk_pixbuf_new_from_data((guchar*)frame_bw.data, GDK_COLORSPACE_RGB, FALSE,
					8, frame_bw.cols/3, frame_bw.rows, frame_bw.step, NULL, NULL);
      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //create the text
      cairo_set_source_rgb(cr, .6, 0, 0);
      cairo_select_font_face(cr, "Pursia",
			     CAIRO_FONT_SLANT_ITALIC,
			     CAIRO_FONT_WEIGHT_BOLD);
      cairo_set_font_size( cr, 28 );
      //center it
      cairo_text_extents( cr, "Waiting", &extents);
      cairo_move_to( cr, image_width/2 - extents.width/2, image_height/2 );
      //draw the text
      cairo_show_text( cr, "Waiting");

    }
    break;

  case CONFIRM_FACE:
    {
      std::string text = "NULL";
      std_msgs::String train_msg;
      std_msgs::String training_label_msg;

      switch( face_feedback_state){
      case INITIAL_FEEDBACK:
	text = std::string("(y/n) Is this the right person? ");
	if( confirmed_correct ){
	  face_feedback_state = KNOWN_PERSON;
	  confirmed_correct = false;
	}
	else if( confirmed_incorrect ){
	  face_feedback_state = NEW_PERSON;
	  confirmed_incorrect = false;
	}
	break;
      case NEW_PERSON:
	text = std::string("I don't know this person. Train recognizer for them? (y/n) ");
	if( confirmed_correct ){
	  training_label_msg.data = "new person";
	  training_label_publisher.publish( training_label_msg );
	  take_data_publisher.publish( training_label_msg );
	  confirmed_correct = false;
	  //hide it
	  gtk_widget_hide( GTK_WIDGET( popup ) );
	  g_signal_handler_block( G_OBJECT( popup ), popup_keypress_handle );
	  g_signal_handler_unblock( G_OBJECT( window ), window_keypress_handle );
	  //change the state
	  state = TAKING_DATA;
	  //reset the pictures to send
	  pics_to_send = PICTURES_TO_SEND;
	  send_pics = true;
	}
	else if( confirmed_incorrect ){
	  //do nothing
	  confirmed_incorrect = false;
	  //hide it
	  gtk_widget_hide( GTK_WIDGET( popup ) );
	  g_signal_handler_block( G_OBJECT( popup ), popup_keypress_handle );
	  g_signal_handler_unblock( G_OBJECT( window ), window_keypress_handle );
	  state = WAITING_FOR_TRIGGER;
	}
	break;
      case KNOWN_PERSON:
	text = std::string("Take more data for ") + object + std::string("?(y/n)");
	if( confirmed_correct ){
	  take_data_publisher.publish( training_label_msg );
	  confirmed_correct = false;
	  //hide it
	  gtk_widget_hide( GTK_WIDGET( popup ) );
	  g_signal_handler_block( G_OBJECT( popup ), popup_keypress_handle );
	  g_signal_handler_unblock( G_OBJECT( window ), window_keypress_handle );
	  //change the state
	  state = TAKING_DATA;
	  //reset the pictures to send
	  pics_to_send = PICTURES_TO_SEND;
	  send_pics = true;

	}
	else if( confirmed_incorrect ){
	  //do nothing
	  confirmed_incorrect = false;
	  //hide it
	  gtk_widget_hide( GTK_WIDGET( popup ) );
	  g_signal_handler_block( G_OBJECT( popup ), popup_keypress_handle );
	  g_signal_handler_unblock( G_OBJECT( window ), window_keypress_handle );
	  state = WAITING_FOR_TRIGGER;
	}
	break;
      }

      GtkTextBuffer *buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( popup ) );
      gtk_text_buffer_set_text( buffer, text.c_str(), -1 );

      if(analyzed_image.empty() ){
	analyzed_image = frame;
      }

      if( !reco_result.empty() ){
	pixbuf = gdk_pixbuf_new_from_data((guchar*)reco_result.data, GDK_COLORSPACE_RGB, FALSE,
					  8, reco_result.cols, reco_result.rows,
					  reco_result.step, NULL, NULL);
      }
      
      else{
	  pixbuf = gdk_pixbuf_new_from_data((guchar*)analyzed_image.data, GDK_COLORSPACE_RGB, FALSE,
					    8, analyzed_image.cols, analyzed_image.rows,
					    analyzed_image.step, NULL, NULL);
      }

      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //set the bouding box color
      cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
      //set the bounding box line width
      cairo_set_line_width(cr,6); 
      //create a rectangle with the specified bounding box and draw it on the screen
      cairo_rectangle (cr,
		       bounding_box[0], bounding_box[1],
		       bounding_box[2]-bounding_box[0], bounding_box[3]-bounding_box[1]);
      //draw the rectangle
      cairo_stroke(cr);

      gdk_cairo_set_source_pixbuf( cr, pixbuf, 0, 0 );
      cairo_paint( cr );

      //queue it up 
      gtk_widget_queue_draw( GTK_WIDGET( widget ));

      //reset the send pictures state
      pics_to_send = PICTURES_TO_SEND;
      send_pics = true;

    }
    break;

    
  }//end switch
  //throw out cairo
  cairo_destroy( cr );  
  //we don't want to close, return false
  return FALSE;
}

/** Tells the gui that the face detector is ready
    the gui will only send messages (pictures) if
    the face detector is ready to recieve them.
    For the object detector, only one image is needed
 */
void face_detector_ready_callback( const std_msgs::String::ConstPtr& msg ){
  if( strcmp("yes face", msg->data.c_str() ) == 0 ){
    found_face = true;
  }else{
    found_face = false;
  }
  face_detector_ready = true;
  //ROS_INFO("face detector ready!");
}

void recognition_image_callback( const sensor_msgs::ImageConstPtr& msg ){
  sensor_msgs::CvBridge bridge;
  IplImage* captured  = bridge.imgMsgToCv( msg, "bgr8" );
  //reco_result = captured;
  //cv::Mat test( captured );
  //cv::namedWindow( "display", CV_WINDOW_AUTOSIZE );
  //cv::imshow( "display", test );
  //cv::waitKey(0);
  cv::Mat frame_org(captured);
  cv::Mat frame_resize(image_height,
  		       image_width,
		       CV_8U);
  //convert the frame to 8 bit color (only thing accepted by gtk)
  frame_org.convertTo(frame_org, CV_8U);
  //resize the frame to bet the requested size
  cv::resize(frame_org, frame_resize, frame_resize.size(), 0, 0, cv::INTER_CUBIC);
  //convert the color from RGB to BGR
  cv::cvtColor(frame_resize, reco_result, CV_RGB2BGR);
}

void face_confirmation_callback( const sensor_msgs::ImageConstPtr& msg ){
  if( state != CONFIRM_FACE && state != WAITING_FOR_TRIGGER){
    //change the state
    state = CONFIRM_FACE;
    //ROS_INFO("RECOGNITION IMAGE CALLBACK!");
    sensor_msgs::CvBridge bridge;
    IplImage* captured  = bridge.imgMsgToCv( msg, "bgr8" );
    //reco_result = captured;
    //cv::Mat test( captured );
    //cv::namedWindow( "display", CV_WINDOW_AUTOSIZE );
    //cv::imshow( "display", test );
    //cv::waitKey(0);
    cv::Mat frame_org(captured);
    cv::Mat frame_resize(image_height,
			 image_width,
			 CV_8U);
    //convert the frame to 8 bit color (only thing accepted by gtk)
    frame_org.convertTo(frame_org, CV_8U);
    //resize the frame to bet the requested size
    cv::resize(frame_org, frame_resize, frame_resize.size(), 0, 0, cv::INTER_CUBIC);
    //convert the color from RGB to BGR
    cv::cvtColor(frame_resize, reco_result, CV_RGB2BGR);

    g_signal_handler_unblock( G_OBJECT( popup ), popup_keypress_handle );
    gtk_widget_show( popup );
    g_signal_handler_block( G_OBJECT( window ), window_keypress_handle );
    //std::cout<<"window handle: "<<window_keypress_handle<<std::endl;
    //std::cout<<"popup handle: "<<popup_keypress_handle<<std::endl;
    gtk_widget_grab_focus( popup );

    face_feedback_state = INITIAL_FEEDBACK;
  }
}

void no_face_data_callback( const std_msgs::String::ConstPtr& msg ){
  g_signal_handler_unblock( G_OBJECT( popup ), popup_keypress_handle );
  gtk_widget_show( popup );
  g_signal_handler_block( G_OBJECT( window ), window_keypress_handle );
  //std::cout<<"window handle: "<<window_keypress_handle<<std::endl;
  //std::cout<<"popup handle: "<<popup_keypress_handle<<std::endl;
  gtk_widget_grab_focus( popup );

  face_feedback_state = NEW_PERSON;
  state = CONFIRM_FACE;


}

/** Tells the gui where the bounding box is for a face detection
    This is used to show the box if we are taking data,
    since we won't be going through the bnet to do this
 */
void train_bb_callback( const std_msgs::String::ConstPtr& msg ){
  std::istringstream ss( msg->data.c_str() );
  std::string line;
  std::getline( ss, line ); train_bounding_box[0] = atoi( line.c_str() );
  std::getline( ss, line ); train_bounding_box[1] = atoi( line.c_str() );
  std::getline( ss, line ); train_bounding_box[2] = atoi( line.c_str() ) + train_bounding_box[0];
  std::getline( ss, line ); train_bounding_box[3] = atoi( line.c_str() ) + train_bounding_box[1];

  for(int i = 0; i < 4; i++){
    train_bounding_box[i]*= image_size_ratio_multiplier;
  }

}

/** Tells the gui that it will be training the face recognizer
    The gui will prompt the user for a label
 */
void train_face_callback( const std_msgs::String::ConstPtr& msg ){
  //this is handled by a keypress for now

  //g_signal_handler_disconnect( G_OBJECT( window ), window_keypress_handle );
  //gtk_widget_show( label_entry );
  //gtk_widget_grab_focus( label_entry );
  //std::string str = gtk_entry_get_text( GTK_ENTRY( label_entry ) );
  
}

/**Initialize the camera
 *The default camera is used
 *returns with an error on error
 */
bool init_camera(){
  //set the caputre object
  cap = cvCaptureFromCAM( camera_number );
  if(!cap) return false;
  //get an image to set the image width and height
  cv::Mat frame_org(cvQueryFrame(cap));
  //set the image width/height
  image_height = frame_org.rows * image_size_ratio_multiplier;
  image_width = frame_org.cols * image_size_ratio_multiplier;

#ifdef FP_CAM
  image_width = 640;
  image_height = 480;
  printf("[iljoo:%s:%d] new image_height = %d\n", __func__, __LINE__, image_height);
  printf("[iljoo:%s:%d] new image_width = %d\n", __func__, __LINE__,image_width);
#endif
}

/**
 *Create and initalize the window
 */
bool init_widgets(){
  /**Init window **/
  //create a new window
  window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
  //set the title
  gtk_window_set_title( GTK_WINDOW( window ), "Intellegentsia");
  //set the default position
  gtk_window_set_position( GTK_WINDOW( window ), GTK_WIN_POS_CENTER);
  window_width = image_width * 2;
  window_height = image_height;
  gtk_window_set_default_size( GTK_WINDOW( window ),
			       window_width,
			       window_height);
  //connect signals to the window for the destroy and keypress events
  g_signal_connect( G_OBJECT( window ), "destroy", G_CALLBACK(gtk_main_quit), NULL);
  window_keypress_handle = g_signal_connect( G_OBJECT( window ),
						"key_press_event", G_CALLBACK( handle_keypress ), NULL);
  //add some padding to the boarder of the window
  //gtk_container_set_border_width( GTK_CONTAINER( window ), 10 );

  //create a new fixed area to anchor the widgets too in the window
  fixed_area = gtk_fixed_new();
  gtk_container_add( GTK_CONTAINER( window ), fixed_area );

  /**init stream image window**/
  video_stream_widget = gtk_drawing_area_new();
  gtk_fixed_put( GTK_FIXED( fixed_area ), video_stream_widget, 0, 0 );
  gtk_widget_set_size_request( video_stream_widget, image_width, image_height );
  g_signal_connect( G_OBJECT( video_stream_widget ), "expose_event", G_CALLBACK( stream_video ), NULL );
  gtk_widget_queue_draw( video_stream_widget );
  
  //init the show results/state window
  drawing_area = gtk_drawing_area_new();
  gtk_fixed_put( GTK_FIXED( fixed_area ), drawing_area, image_width, 0 );
  gtk_widget_set_size_request( drawing_area, image_width, image_height );
  g_signal_connect( G_OBJECT( drawing_area ), "expose_event", G_CALLBACK(annotate_image), NULL);
  gtk_widget_set_events( drawing_area, GDK_EXPOSURE_MASK);

  //create the entry widget
  label_entry = gtk_entry_new();
  //gtk_signal_connect( GTK_OBJECT( label_entry ), "activate",
  //		      GTK_SINGAL_FUNC( enter_callback ), 
  //		      label_entry );
  gtk_fixed_put( GTK_FIXED( fixed_area ), label_entry, image_width * (3.0/2.0) - label_width/2,
		 image_height/2 - label_height/2 );
  gtk_widget_set_size_request( label_entry, label_width, label_height );
  g_signal_connect( G_OBJECT( label_entry ), "activate",
		    G_CALLBACK( label_enter_callback ), label_entry );

  popup = gtk_text_view_new();
  gtk_fixed_put( GTK_FIXED( fixed_area ), popup, image_width * (3.0/2.0) - popup_width/2,
		 image_height - popup_height );
  gtk_widget_set_size_request( GTK_WIDGET( popup ), popup_width, popup_height );
  gtk_text_view_set_editable( GTK_TEXT_VIEW( popup ), FALSE );
  gtk_text_view_set_cursor_visible( GTK_TEXT_VIEW( popup ), FALSE );
  popup_keypress_handle = g_signal_connect( G_OBJECT( popup ),
					    "key_press_event", G_CALLBACK( popup_handle_keypress ), NULL);
  g_signal_handler_block( G_OBJECT( popup ), popup_keypress_handle );
  gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( popup ), 
			       GTK_WRAP_WORD );

  //gtk_widget_show( label_entry );
  //gtk_entry_set_editable( GTK_ENTRY( label_entry ), TRUE );
  //gtk_widget_grab_focus( label_entry );

  //show those widgets
  gtk_widget_show_all( window );

  //hide the text entry widget for now
  gtk_widget_hide( GTK_WIDGET( label_entry ) );
  //hide the popup
  gtk_widget_hide( GTK_WIDGET( popup ) );

  
  //all good
  return true;
}


/**this is the callback for the words that come from the dialog manager
 * this is just used to set the THINKING state, it is only cosemetic
 * since the LISTENING state is where we release images
 */
void speech_callback( const std_msgs::String::ConstPtr &msg ){
  //if we get a good signal from the speech, change states
  if(msg->data.compare("good words") == 0){
    if(state != CONFIRM_FACE && state != TAKING_DATA){
      state = THINKING;
    }
  }
  else{
    //right now this wont show up
    state = MISUNDERSTANDING;
  }
}

/**callback for the trigger 
 * if the trigger is heard then the state changes
 * to LISTENING, where the images are released
 */
void trigger_callback(const std_msgs::String::ConstPtr &msg){
  //if we hear the trigger word, change the state
  if(trigger.compare(msg->data) == 0){
    state = LISTENING;
    //queue the drawing of the widget
    gtk_widget_queue_draw( GTK_WIDGET( drawing_area ));
  }
}

/**callback for the faces left function
 * if we still need to take data for the faces
 * we receive this callback and send the message here
 */

void faces_left_callback( const std_msgs::String::ConstPtr &msg ){
  if( strcmp( msg->data.c_str(), "true" ) == 0 ){
    send_pics = true;
  }else{
    send_pics = false;
    done_taking_data = true;
  }
  //take_data_ready = true;
}

/** This callback parses the final result that is sent to the gui
 * from the Bayes Net.  It is expected that the format of the message
 * will be the same as the format specified by the Final_Results_Messsage 
 * enum at the top
 */
void gui_result_parser_callback(const std_msgs::String::ConstPtr &msg){
  
  //read the lines
  std::istringstream iss(msg->data);
  std::string token;
  std::vector<std::string>lines;
  while(getline(iss, token)){
    lines.push_back(token);
  }
  //if we aren't already showing something
  if( state != SHOWING_RESULT ){

    //change the state
    state = SHOWING_RESULT;
    //change the current timestamp
    current_timestamp = lines[TIME_STAMP];
    //change the object string
    object = lines[OBJECT];
    //change the words spoken varible
    words_spoken = lines[WORDS_SPOKEN];
  
    //set the bounding box as specified by the message
    //note that we have to multiply by the image_size_ratio_multiplier
    //so that the bounding box scales, since the analysis is run on an image
    //that is of the original size.  As a result the bounding box coordinates
    //are in those pixel coordinates, so we have to alter that for the
    //bonding box to be in the correct position
    bounding_box[0] = atoi( lines[BB_X1].c_str() ) * image_size_ratio_multiplier;
    bounding_box[1] = atoi( lines[BB_Y1].c_str() ) * image_size_ratio_multiplier;
    bounding_box[2] = atoi( lines[BB_X2].c_str() ) * image_size_ratio_multiplier;
    bounding_box[3] = atoi( lines[BB_Y2].c_str() ) * image_size_ratio_multiplier;
    gtk_widget_queue_draw( GTK_WIDGET( drawing_area ));
  }
}




int main (int argc, char *argv[]) {
  //initalize camera
  assert(init_camera());
  
  //default state
  state = WAITING_FOR_TRIGGER;
  //setup ros
  ros::init(argc, argv, "guiNode");
  ros::NodeHandle nh; 
  image_transport::ImageTransport it(nh);

  //publisher that sends images from the camera
  visionPublisher = it.advertise("gui/image_raw/original", 1);
  //publisher to tell the bnet that we are ready for the results since we've sent all the pics
  guiStatePublisher = nh.advertise< std_msgs::String >( "gui/state", 1000 );
  //publisher that sends the label to the face trainer
  training_label_publisher = nh.advertise< std_msgs::String >( "gui/face_train/label", 1000 );
  //tells the detector that we have to take data
  take_data_publisher = nh.advertise< std_msgs::String >( "gui/take_data", 1000 );
  //tells the bnet to change the detection mode between retail and face
  change_detection_mode = nh.advertise< std_msgs::String >("gui/change_detection_mode", 1000 );
  
  //ros subscribers
  //gets information to display on the gui
  ros::Subscriber guiLogSub = nh.subscribe( "bnet/final_result", 1000, gui_result_parser_callback );
  //gets the face from the bnet to determine if it was correct
  image_transport::Subscriber face_confirm_sub = it.subscribe( "bnet/face/confirmation", 1000,
						   face_confirmation_callback );
  //tells the gui when the trigger has finred
  ros::Subscriber triggerSub = nh.subscribe("trigger/output", 1000, trigger_callback);
  //tell the gui when a command has been issued
  ros::Subscriber speechSubscriber = nh.subscribe( "dialog/command", 1000, speech_callback );
  //tells the gui if it needs to send more messages for taking data of faces
  ros::Subscriber sendFacesToTrainSubscriber = nh.subscribe( "face/face_train/faces_left",
							     1000, faces_left_callback );
  //tells the gui that the face detector is ready to recieve a message
  ros::Subscriber faceDetectorReadySubscriber = nh.subscribe( "face/face_detector/ready",
							      1000, face_detector_ready_callback);
  //tells the gui that it is going to be taking data, so it knows to prompt
  //the user for a label for the data
  ros::Subscriber trainFaceDataSubscriber = nh.subscribe( "bnet/face/take_data",
							  1000, train_face_callback );
  //tells the gui where the bounding box for the face that was detected lies,
  //so that the guy can display it while taking data
  ros::Subscriber trainFaceBBSubscriber = nh.subscribe( "face/face_detection_result/bounding_box",
							1000, train_bb_callback );
  //if we have no data in the face recognizer, we have a special case we need to react to
  ros::Subscriber noFaceDataSubscriber = nh.subscribe( "face/face_detector/no_data", 1000,
						       no_face_data_callback );

  //initalize gtk
  gtk_init(&argc, &argv);
  //initalize window
  assert(init_widgets());
  //the main gtk loop
  gtk_main();  
  return 0;
}
