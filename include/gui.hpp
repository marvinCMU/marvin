#ifndef _TJS_GUI4_
#define _TJS_GUI4_

//opengl includes
#include <GL/glew.h>
#include <GL/glut.h>
//sdl includes
#include <SDL/SDL.h>
#include <SDL/SDL_getenv.h>
//standard library includes
#include <iostream>
#include <fstream>
#include <list>
#include <stdexcept>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctime>
#include <deque>
//boost
#include<boost/tokenizer.hpp>
//opencv includes
#include <cv.h>
#include <opencv/highgui.h>
//openal stuff
#include <AL/al.h>
#include <AL/alc.h>
//ros stuff
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/CvBridge.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv/cvwimage.h>
#include <opencv/highgui.h>
//boost
#include <boost/numeric/ublas/vector.hpp>


struct Vector3{
  float x, y, z;
  Vector3();
  Vector3(float, float, float);
  float lengthSq();
  float length();
  void operator-=(Vector3);
  void operator+=(Vector3);
  Vector3 operator+(Vector3);
  Vector3 operator-(Vector3);
  Vector3& operator=(Vector3);
  Vector3 operator*(float);
  Vector3 normalize();
};

Vector3::Vector3(){  this->x=0; this->y=0; this->z=0;}
Vector3::Vector3(float x, float y, float z){
  this->x = x; this->y=y; this->z=z;
}

Vector3& Vector3::operator=(Vector3 other){
  this->x = other.x;
  this->y = other.y;
  this->z = other.z;
  return *this;
}
void Vector3::operator-=(Vector3 other){
  this->x-=other.x; this->y-=other.y; this->z-=other.z;
}
void Vector3::operator+=(Vector3 other){
  this->x+=other.x; this->y+=other.y; this->z+=other.z;
}
Vector3 Vector3::operator+(Vector3 b){
  Vector3 c=(*this);  c+=b;  return c;
}
Vector3 Vector3::operator-(Vector3 b){
  Vector3 c=(*this); c-=b; return c;
}
Vector3 Vector3::operator*(float scale){
  return Vector3(this->x*scale, this->y*scale, this->z*scale);
}
float Vector3::length(){
  return sqrt( pow(this->x,2) + pow(this->y,2) + pow(this->z,2) );
}
Vector3 Vector3::normalize(){
  float length = this->length();
  return Vector3(this->x/length,this->y/length,this->z/length);
}
std::ostream& operator<<(std::ostream& str, const Vector3& v){
  str<< "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

struct Camera{
  Vector3 position;
  Vector3 direction;
  Vector3 up;
  float FOV;
  Camera();
};

Camera::Camera(){
  this->position = Vector3(0.0, 0.0, 1.2071);
  this->direction = Vector3(0.0, 0.0, -1.0);
  this->up = Vector3(0.0, 1.0, 0.0);
  this->FOV = 45;
}

enum State{
  WAITING_FOR_TRIGGER,
  LISTENING,
  THINKING,
  SHOWING_RESULTS,
  TAKING_DATA,
};

typedef std::vector< cv::Mat > ImageList;
typedef cv::Mat Image;
typedef std::deque<std::string> StringList;

void processKeypress( const SDL_keysym * keysym );
void processClickUp( const SDL_MouseButtonEvent* button );
void processClickDown( const SDL_MouseButtonEvent* button );
void handleEvents();
void render();
void mainLoop();
void resizeWindow( int width, int height );
void cleanUp();
void initGL();
bool initSDL();
bool init( int argc, char** argv );
void initTexture();
void manageState();
cv::Mat loadImage( const char* path, int width, int height, bool maintainAspect = true);
cv::Mat getFrame(int width=640, int height=480);
bool create_shader( GLhandleARB program, const char * vert_file, const char* frag_file );
bool load_shader( const char* file, GLint type, GLhandleARB program );
void updateResultsTexture( );
void setScrollbarTexture( );
void setState( State state );
void addUserInteraction();
void drawScrollbar();
void drawCommandTiles();
void  drawWrongStrip();
void drawRightStrip();
void drawResetStrip();
void drawResultInfoTexture();
void drawBackgroundTexture();
void initRos(int argc, char** argv);
void publishImage();
void callbackBnetFinalResult( const std_msgs::String::ConstPtr& msg );
void callbackTrigger(const std_msgs::String::ConstPtr& msg);
void speechCommandCallback(const std_msgs::String::ConstPtr& msg);
void setResultInfoTexture();
void bnetCallback(std::string );
void drawTakingDataStrip();
void publishFaceImage();

Camera camera;
SDL_Surface* gMainWindow;
State gState = WAITING_FOR_TRIGGER;
bool gRunning;
const int gImageWidth = 1280, gImageHeight = 960;
const int gTopBannerHeight = 78, gBottomBannerHeight = 78;
//const int gScrollWidth = 260, gScrollHeight = 320, gNumScrollTiles = 12;
const int gScrollWidth = 160, gScrollHeight = 200, gNumScrollTiles = 12;
const int gScrollbarHeight = gImageHeight - (gTopBannerHeight + gBottomBannerHeight );
const float gScrollSpringK = 1.0e6,  gScrollMass = 1;
const float gScrollDamping = 2 * sqrt(gScrollSpringK * gScrollMass);
const int gCmdBarWidth = 1020, gCmdBarHeight = 100;
const int gCmdButtonHeight = 140, gCmdButtonWidth = 140;
//const int gCmdTileWidth = 110, gCmdTileHeight = 110;
const int gCmdTileWidth = 80, gCmdTileHeight = 80;
const int gFeedbackTileWidth = 80, gFeedbackTileHeight = 80, gStripTabWidth = 80;
const int gTilePadding = 8;
const int gResultInfoWidth = gImageWidth - gScrollWidth;
const int gResultInfoHeight = 600;
const int gTartanWidth = 120, gTartanHeight = 120;
const int gTileShadowWidth = 100, gTileShadowHeight = 100;
const float gMaxStripPos = -.1;
const float gDt = .0002;
const float EPSILON = 1e-5;
const float gBannerBlend = .82, gBannerFadeRate = .01;
const int gResetWidth = 100, gResetHeight = 50;
int gTime;
GLuint gMainTex;
GLuint gTopBannerTex;
GLuint gBottomBannerTex;
GLuint gScrollTex;
GLuint gSelectedTileTex;
GLuint gCmdBarTex;
GLuint gTileMaskTex;
GLuint gFeedbackTileMaskTex;
GLuint gScrollMaskTex;
GLuint gWrongTileTex;
GLuint gRightTileTex;
GLuint gTakeDataTileTex;
GLuint gResetStripTex;
GLuint gResultInfoTex;
GLuint gTartanTex;
GLuint gTileShadowTex;
GLuint gInfoSurfaceMaskTex; 
GLuint gWaitBannerTex;
GLuint gListenBannerTex;
GLuint gResultBannerTex;
GLuint gThinkingBannerTex;
GLuint gTakingDataBannerTex;
GLhandleARB gBlendShader;
GLhandleARB gMaskShader;
GLhandleARB gMaskAndBlendShader;
GLhandleARB gShadowShader;
GLhandleARB gGreyMaskShader;

CvCapture *gCaptor;
GLuint gFboId;
float gScrollPos, gScrollVelocity;
float gScrollBottom = 0.0, gScrollTop = 0.0;
bool gWrongClicked = false, gResetClicked = false, gRightClicked = false;
float gResetPos, gResetVelocity;
int gSelectedTileIndex;

StringList gObjects;
StringList gCommands;
std::string gActiveObject;
std::string gActiveCommand;

float gScrollMy, gScrollOmy;
bool gScrollClicked;

Image gScrollBarImage;

image_transport::Publisher imagePublisher;
image_transport::Publisher imagePublisherHiRes;
ros::Publisher feedbackPublisher;
ros::Publisher dummyTestPublisher;

ros::Subscriber triggerSubscriber;
ros::Subscriber bnetResultSubscriber;
ros::Subscriber commandSubscriber;

const bool gRunRos = true;
bool gEnableLocalization = false;
bool gEnableDummyTest = false;
bool gEnableDummyTestVideoIn = false;
bool gCloudService = false;

const int facePicsToSend = 30;
int facePicsSent = 0;
bool gTakeDataClicked = false;

#endif
