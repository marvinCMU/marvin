//opengl includes
#include <GL/glew.h>
#include <GL/glut.h>
//sdl includes
#include <SDL/SDL.h>
#include <SDL/SDL_getenv.h>
//standard library includes
#include <iostream>
#include <fstream>
#include <iostream>
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
#include "SystemParams.h"
//local includes

#define SCREEN_BPP 32 //bytes per pixel
#define IMG_SIZE_RATIO_MULTIPLIER 2.2
#define AUDIO_FREQ 22050
#define CAP_SIZE 2048

#define PICTURES_TO_SEND 1
#define TIMESTEP 2
#define ZERO 1e-4

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


enum ShaderProgram{
  SHADER_STREAM,
  SHADER_BLUR,
  SHADER_GAUSSIAN_BORDER,
  SHADER_BW,
  SHADER_SOUNDLINE,
  SHADER_FADED,
  SHADER_BORDER,
};

enum State{
  WAITING_FOR_TRIGGER,
  LISTENING,
  THINKING,
  RESULT_OPTIONS,
  FINAL_RESULT,
  SHOWING_COMMAND_OPTIONS,
  SHOWING_INFORMATION,
  RESETTING,
} gState;

enum ResultsConfigurations{
  ONE_RESULT,
  TWO_RESULTS,
  THREE_RESULTS,
  FOUR_RESULTS
};

enum Origin{
  BOTTOM_LEFT,
  BOTTOM_RIGHT,
  TOP_RIGHT,
  TOP_LEFT,
};

struct Audio{
  ALuint helloSource[1];
  ALenum errorCode;
  ALCdevice* audioDevice;
  ALCcontext * audioContext;
  std::list<ALuint> bufferQueue;
  ALCdevice * inputDevice;
} audio;

struct Publishers{
  image_transport::Publisher image;
  ros::Publisher feedback;
} publishers;

struct Subscribers{
  ros::Subscriber trigger;
  ros::Subscriber bnetResult;
} subscribers;

enum Animations{
  CARD_FLIP,
  ZOOM_IN,
  NO_ANIMATION,
  CENTER_FINAL_RESULT,
  OPEN_INFORMATION,
  CLOSE_INFORMATION,
  FALL_AND_FADE,
  MINIMIZE_AND_POPUP_RESULT,
};
struct AnimationParams{
  float flipRate;
  float zoomRate;
  float centerRate;
  float separateRate;
  float togetherRate;
  float backupRate;
  float userApprovalLowerRate;
  float whiteoutRate;
  float whiteoutTime;
  float fallRate;
  float fadeRate;
  float shrinkRate;
  float raiseInfoRate;
  float toCornerRate;

  float separationDecayRate;
  float centeringDecayRate;
  float togetherDecayRate;
  float flipDecayRate;
  float lowerDecayRate;
  float fallAccelRate;
  float toCornerDecayRate;
  float raiseDecayRate;
  float shrinkDecayRate;

  float cardRotation;
  Vector3 path;
  Vector3 pathDir;
  Vector3 centeredPosition;
  Vector3 readyFlipPosition;
  float pathLength;
  Animations animation;
  bool openInfoComplete;
  
} gAnimationParams;

struct CameraParams{
  Vector3 cameraPosition;
  Vector3 cameraDirection;
  Vector3 cameraReference;
  Vector3 cameraUp;
  float FOV;
}gCameraParams;

struct ModelParams{
  Vector3 position;
  
} gModelParams;

struct FinalInformation{
  std::vector<std::string> commandOptions;
  std::vector<std::string> objectOptions;
  std::string command;
  std::string object;
} gFinalInfo;

enum TEXTURES{
  STREAM_TEX,
  RESULT_TEX,
  COMMAND_OPTIONS_TEX,
  USER_APPROVAL_TEX,
  INITIAL_APPROVAL_TEX,
  CMD_OPTION_TEX,
  SELECTED_RESULT_TEX,
  RESULT_INFO_TEX,
  BORDER_TEX,
  //SOUND_TEX,
  //FINAL_RESULT_TEX,
};

enum CommandOptions{
  INFORMATION,
  PRICE,
  REVIEW,
};

enum QuadTypes{
  FRONT_QUAD,
  REAR_QUAD,
  SELECTED_RESULT,
  COMMAND_OPTIONS,  
  USER_APPROVAL,
  INITIAL_APPROVAL,
  CMD_OPTION,
  RESULT_INFO,
};

struct Quad{
  Vector3 position;
  float w, h;
  GLuint texId;
  ShaderProgram shader;
  Origin origin;
  GLvoid *texData;
  //int texWidth, texHeight;
  Quad(float x, float y, float z, float w, float h, Origin origin = BOTTOM_LEFT);
  Quad();
  Quad& operator=(Quad);
};

Quad::Quad(float x, float y, float z, float w, float h, Origin origin ) {
  position = Vector3(x, y, z);
  this->w = w;
  this->h = h;
  this->origin = origin;
  this->shader = SHADER_STREAM;
  this->texData = NULL;
}
Quad::Quad(): position(Vector3()),  w(0.0f), h(0.0f), origin(TOP_LEFT), shader(SHADER_STREAM), texData(NULL){}
Quad& Quad::operator=(Quad other){
  this->position = other.position;
  this->w = other.w;
  this->h = other.h;
  this->origin = other.origin;
  //this->texId = other.texId;
  this->shader = other.shader;
  return *this;
}


typedef std::vector<Quad> ResultConfiguration;
typedef std::vector<cv::Mat> ImageList;
typedef std::vector<Quad> QuadList;

SDL_Surface* gMainWindow;
bool gRunning;
int gScreenWidth;
int gScreenHeight;
CvCapture *gCaptor;
GLhandleARB gPrograms[6];
float gSoundAmplitude;
float gPicsToSend;
int gCameraNumber;
int gTime;
Quad gQuads[8];
std::vector<Quad*> gToRender;
GLuint fboId;
ResultConfiguration gResultsConfig[4];
ResultConfiguration *gActiveResultsConfig;
int gSelectedResultFromConfig;
GLuint gTexIds[9];
bool gUserLikes;
int gTexIndex;
cv::Mat gAnalyzedFrame;


cv::Mat getFrame(float imageRatioMultiplier = IMG_SIZE_RATIO_MULTIPLIER, bool flipped = true);
void render();
void drawQuad(float x, float y, float z, float w, float h, Origin corner= TOP_LEFT);
void drawQuad(Quad, Origin);
void drawQuad(Quad);
void drawQuad(Origin corner = BOTTOM_LEFT);
void drawWireframeQuad( Quad );
void drawWireframeQuad(float x, float y, float z, float w, float h, Origin corner);
void setupShader(ShaderProgram prog);
void setupTexture(GLuint);
bool create_shader( GLhandleARB program, const char * vert_file, const char* frag_file );
bool load_shader( const char* file, GLint type, GLhandleARB program );
void publishImage();
void manageState();
void resetPicsToSend();
void setState( State state );
void processKeypress(const SDL_keysym * keysym );
void handle_events();
void mainLoop();
void resizeWindow( int width, int height );
void captureAudio();
void stopAudioCapture();
void initTextures();
bool initShaders();
void initCV();
void initAL();
void initGL();
bool initSDL();
void initRos(int argc, char** argv);
bool init(int argc, char** argv);
void updateTexture( GLuint texId,
		    int x, int y,
		    int width, int height,
		    const GLvoid * data );
void initFBO();
void setResultsTexture(ResultConfiguration& config, ImageList images, QuadList boundingBoxes);
cv::Mat loadImage(const char* path, bool resize=false);
template<class T>
void _swap(T& a, T& b);
void swap(Quad& a, Quad& b);
void drawSoundLine();
void cleanUp();
void animate();
void resetAnimationRates();
void takeScreenshot();
std::string getTimestamp();
void createTexture( GLuint texId, int width, int height, GLvoid* data );
void setCommandOptionsTexture(ImageList);
void dummy_callback_bnet_final_result( std::string msg );

void mainLoop(){

  gRunning = true;
  while( gRunning ){
    glClear(GL_COLOR_BUFFER_BIT);
    manageState();
    animate();
    handle_events();
    render();
    //resizeWindow(gScreenWidth, gScreenHeight);
    //captureAudio();
    gTime +=TIMESTEP;
    
    ros::spinOnce();
  }
  stopAudioCapture();
}

/**Rendering **/
void manageState(){
  cv::Mat frame = getFrame();

  switch(gState){
  case WAITING_FOR_TRIGGER:{
    updateTexture( gTexIds[STREAM_TEX], 0, 0, frame.cols, frame.rows, frame.data);
    gAnalyzedFrame = frame;
    break;
  }
  case LISTENING:
    if(gPicsToSend-- > 0) publishImage();
    break;
  case THINKING:
     if(gPicsToSend-- > 0 ) publishImage();
    break;
  case RESULT_OPTIONS:{
    break;
  }
  case FINAL_RESULT:{
    gQuads[FRONT_QUAD].shader = SHADER_FADED;
    break;
  }
  }
}

void animate(){
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  switch( gAnimationParams.animation ){
  case CARD_FLIP:{
    if(fabs(gAnimationParams.cardRotation) == 90.0f ){
      gAnimationParams.cardRotation *=-1;
      swap(gQuads[FRONT_QUAD], gQuads[REAR_QUAD]);
    }
    glRotatef(gAnimationParams.cardRotation, 0.0f, 1.0f, 0.0f);
    gAnimationParams.cardRotation+= gAnimationParams.flipRate;
    
    if( gAnimationParams.cardRotation == 0.0f ){
      gAnimationParams.animation = NO_ANIMATION;
      gAnimationParams.cardRotation = 0;
    }
    break;
  }
  case ZOOM_IN:
    glTranslatef( gAnimationParams.path.x * gAnimationParams.zoomRate,
		  gAnimationParams.path.y * gAnimationParams.zoomRate,
		  gAnimationParams.path.z * gAnimationParams.zoomRate );
    break;
  case CENTER_FINAL_RESULT:{
    float delta = 0.0f;
    bool xMoving = gQuads[SELECTED_RESULT].position.x <= gAnimationParams.centeredPosition.x;
    bool yMoving = gQuads[SELECTED_RESULT].position.y <= gAnimationParams.centeredPosition.y;
    bool zMoving = gQuads[SELECTED_RESULT].position.z <= gAnimationParams.centeredPosition.z;
    if( xMoving ){
      gQuads[SELECTED_RESULT].position.x -= gAnimationParams.pathDir.x * gAnimationParams.centerRate;
      delta += fabs(gAnimationParams.pathDir.x * gAnimationParams.centerRate);
    }
    if( yMoving ){
      gQuads[SELECTED_RESULT].position.y -= gAnimationParams.pathDir.y * gAnimationParams.centerRate;
      delta += fabs(gAnimationParams.pathDir.y * gAnimationParams.centerRate);
    }
    if( zMoving ){
      gQuads[SELECTED_RESULT].position.z -= gAnimationParams.pathDir.z * gAnimationParams.centerRate*.1;
      delta += fabs(gAnimationParams.pathDir.z * gAnimationParams.centerRate);
    }
    if((!xMoving && !yMoving && !zMoving) || (delta <= ZERO) ){
      setState(SHOWING_COMMAND_OPTIONS);
    }
    gAnimationParams.centerRate *= gAnimationParams.centeringDecayRate;
    break;
  }
  case OPEN_INFORMATION:{
    bool xMoving = gQuads[SELECTED_RESULT].position.x < 0.5f;
    bool yMoving = gQuads[USER_APPROVAL].position.y < (gQuads[COMMAND_OPTIONS].h +gQuads[COMMAND_OPTIONS].position.y);
    if( xMoving ){
      gQuads[COMMAND_OPTIONS].position.x +=  gAnimationParams.separateRate;
      gQuads[SELECTED_RESULT].position.x += gAnimationParams.separateRate;
      gQuads[USER_APPROVAL].position.x += gAnimationParams.separateRate;
    }
    if(!xMoving || gAnimationParams.separateRate <= ZERO){
      gQuads[COMMAND_OPTIONS].position.x = 0.5f;
      gQuads[SELECTED_RESULT].position.x = 0.5f;
      xMoving = false;
    }

    if(yMoving){
      gQuads[USER_APPROVAL].position.y += gAnimationParams.userApprovalLowerRate;
    }if(!yMoving || gAnimationParams.separateRate <= ZERO ){
      gQuads[USER_APPROVAL].position.y = gQuads[COMMAND_OPTIONS].h + gQuads[COMMAND_OPTIONS].position.y;
      yMoving = false;
    }

    gAnimationParams.separateRate *= gAnimationParams.separationDecayRate;
    gAnimationParams.userApprovalLowerRate *= gAnimationParams.lowerDecayRate;
    break;
  }
  case CLOSE_INFORMATION:{
    bool xMoving = gQuads[SELECTED_RESULT].position.x > 0.25f;
    bool yMoving = gQuads[USER_APPROVAL].position.y > (gQuads[COMMAND_OPTIONS].h +gQuads[COMMAND_OPTIONS].position.y-
						       gQuads[USER_APPROVAL].h);
    float minRate = 1e-3;

    if( xMoving ){
      gQuads[COMMAND_OPTIONS].position.x -=  gAnimationParams.togetherRate;
      gQuads[SELECTED_RESULT].position.x -= gAnimationParams.togetherRate;
      gQuads[USER_APPROVAL].position.x -= gAnimationParams.togetherRate;
    }
    if(!xMoving || (gAnimationParams.togetherRate <= minRate)){
      gQuads[COMMAND_OPTIONS].position.x = 0.25f;
      gQuads[SELECTED_RESULT].position.x = 0.25f;
      xMoving = false;
    }
    if( yMoving ){
      gQuads[USER_APPROVAL].position.y -= gAnimationParams.userApprovalLowerRate;
    }
    if(!yMoving || (gAnimationParams.userApprovalLowerRate <= minRate)){
      gQuads[USER_APPROVAL].position.y = (gQuads[COMMAND_OPTIONS].h +
					  gQuads[COMMAND_OPTIONS].position.y-gQuads[USER_APPROVAL].h);
      yMoving = false;
    }

    if(!xMoving && !yMoving){
      gToRender.pop_back();
      gToRender.pop_back();
      gToRender.pop_back();
      //gToRender.pop_back();
      gToRender.push_back(&gQuads[SELECTED_RESULT]);
      gToRender.push_back(&gQuads[RESULT_INFO]);
      gAnimationParams.animation = MINIMIZE_AND_POPUP_RESULT;
      //gAnimationParams.animation = FALL_AND_FADE;
      
    }

    gAnimationParams.togetherRate *= gAnimationParams.togetherDecayRate;
    gAnimationParams.userApprovalLowerRate *= gAnimationParams.lowerDecayRate;
    break;
  }
  case MINIMIZE_AND_POPUP_RESULT:{
    gQuads[SELECTED_RESULT].origin = TOP_RIGHT;
    bool xResMoving = gQuads[SELECTED_RESULT].position.x > 0.0f;
    bool yResMoving = gQuads[SELECTED_RESULT].position.y >0.0f;
    bool resShrinking = gQuads[SELECTED_RESULT].w > .25;
    bool yInfoMoving = gQuads[RESULT_INFO].position.y > .25;
    float minRate = 1e-3;
    
    //std::cout<<"x,y "<<gQuads[SELECTED_RESULT].position.x <<", "<<gQuads[SELECTED_RESULT].position.y<<std::endl;
    
    if( xResMoving ){
      gQuads[SELECTED_RESULT].position.x -= gAnimationParams.toCornerRate;
    }
    if( yResMoving ){
      gQuads[SELECTED_RESULT].position.y -= gAnimationParams.toCornerRate;
    }
    if( (!xResMoving && !yResMoving) || gAnimationParams.toCornerRate <= minRate ){
      gQuads[SELECTED_RESULT].position.x = 0.0f;
      gQuads[SELECTED_RESULT].position.y = 0.0f;
    }
    
    if( resShrinking ){
      gQuads[SELECTED_RESULT].h -= gAnimationParams.shrinkRate;
      gQuads[SELECTED_RESULT].w -= gAnimationParams.shrinkRate;
    }
    if( gAnimationParams.shrinkRate <= minRate ){
    }

    if( yInfoMoving ){
      gQuads[RESULT_INFO].position.y -= gAnimationParams.raiseInfoRate;
      
    }
    if( !yInfoMoving || gAnimationParams.raiseInfoRate <= minRate ){
      gQuads[RESULT_INFO].position.y = .25f;
    }
    
    gAnimationParams.toCornerRate *= gAnimationParams.toCornerDecayRate;
    gAnimationParams.raiseInfoRate *= gAnimationParams.raiseDecayRate;
    gAnimationParams.shrinkRate *= gAnimationParams.shrinkDecayRate;

    break;
  }
  case FALL_AND_FADE:{
    //std::cout<<"pos: "<<gQuads[SELECTED_RESULT].position.y<<std::endl;
    bool yMoving = gQuads[SELECTED_RESULT].position.y <= 1.0f;
    if( yMoving ){
      gQuads[SELECTED_RESULT].position.y += gAnimationParams.fallRate;
      gQuads[RESULT_INFO].position.y += gAnimationParams.fallRate;

    }
    else if (!yMoving ){
      gAnimationParams.animation = CARD_FLIP;
      setState(WAITING_FOR_TRIGGER);
    }
    gAnimationParams.fallRate *= gAnimationParams.fallAccelRate;
    break;
  }
  case NO_ANIMATION: break;
  }

  glTranslatef(gModelParams.position.x, gModelParams.position.y, gModelParams.position.z);
}

void render(){
  for(int i = 0; i < gToRender.size(); i++){
    setupShader( gToRender[i]->shader );
    setupTexture( gToRender[i]->texId );
    drawQuad( *gToRender[i] );
  }
 
  glDisable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, 0);
  glUseProgramObjectARB( 0 );

  SDL_GL_SwapBuffers();
}

void setupShader( ShaderProgram prog ){
  glUseProgramObjectARB( gPrograms[prog] );
  switch( prog ){
  case SHADER_STREAM:
    break;
  case SHADER_BLUR:
    prog = SHADER_STREAM;
    break;
  case SHADER_GAUSSIAN_BORDER:{
    GLint sigmaLoc = glGetUniformLocationARB( gPrograms[prog], "sigma" );
    glUniform1f( sigmaLoc, gSoundAmplitude );
  }
    break;
  case SHADER_BW:
    break;
  case SHADER_FADED:
    break;
    //case SHADER_SCREENSHOT:
    //float whiteout = gAnimationParams.whiteoutRate * gAnimationParams.whiteoutTime++;
    //GLint whiteoutLoc = glGetUniformLocationARB( gPrograms[prog], "whiteout");
    //glUniform1f( whiteoutLoc, 
    //break;
  }

  GLint borderTexLoc = glGetUniformLocationARB( gPrograms[prog], "border_tex" );
  glUniform1i( borderTexLoc, 1 );

  GLint streamTextureLoc = glGetUniformLocationARB( gPrograms[prog], "stream_tex" );
  glUniform1i( streamTextureLoc, 0 );
}
void setupTexture(GLuint texId){
  glEnable( GL_TEXTURE_2D );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, texId );
}

void updateTexture( GLuint texId,
		    int x, int y,
		    int width, int height,
		    const GLvoid * data ){
  glBindTexture( GL_TEXTURE_2D, texId );
  glTexSubImage2D( GL_TEXTURE_2D, 0, 
		   x, y,
		   width, height,
		   GL_RGB, GL_UNSIGNED_BYTE, data );

}

void setInformationTexture(cv::Mat infoPic){
  createTexture(gQuads[RESULT_INFO].texId, infoPic.cols, infoPic.rows, infoPic.data );
}

void setCommandOptionsTexture(ImageList images){
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboId );
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			     GL_TEXTURE_2D, gQuads[COMMAND_OPTIONS].texId, 0 );
  glClear(GL_COLOR_BUFFER_BIT); 

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(gModelParams.position.x, gModelParams.position.y, gModelParams.position.z);
  float yTemp = 0.0f;

  yTemp = (1.0 - gQuads[CMD_OPTION].h * images.size())/2.0f;

  for(int i = 0; i < images.size(); i++){
    updateTexture( gQuads[CMD_OPTION].texId, 0, 0,
		   images[i].cols, images[i].rows, images[i].data );
    setupShader(SHADER_STREAM );
    setupTexture( gQuads[CMD_OPTION].texId );
    gQuads[CMD_OPTION].position.y = yTemp;
    glColor3f(1.0, 0.0, 0.0);
    
    drawQuad(gQuads[CMD_OPTION]);
    yTemp+= gQuads[CMD_OPTION].h;
  }
  float hOrg = gQuads[COMMAND_OPTIONS].h;
  //gQuads[COMMAND_OPTIONS].h -=.2;
  //gQuads[COMMAND_OPTIONS].h *= images.size()/3;
  //gQuads[COMMAND_OPTIONS].position.y += (gQuads[COMMAND_OPTIONS].h - hOrg)/2.0f;
  
  
  glDisable( GL_TEXTURE_2D );
  glUseProgramObjectARB( 0 );
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  glLoadIdentity();

}
void setResultsTexture(ResultConfiguration &config, ImageList images, QuadList boundingBoxes ){
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboId );
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			     GL_TEXTURE_2D, gTexIds[RESULT_TEX], 0);
  glClear(GL_COLOR_BUFFER_BIT);
  for(int i = 0; i < config.size(); i++){
    createTexture( config[i].texId, images[i].cols, images[i].rows, images[i].data);
    //updateTexture( config[i].texId, 0, 0, images[i].cols, images[i].rows, images[i].data);
    //glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
    //GL_TEXTURE_2D, config[i].texId, 0 );
    config[i].w = images[i].cols / (float)gScreenWidth;
    config[i].h = images[i].rows / (float)gScreenHeight;

    std::cout<<"w,h "<<config[i].w<< ", "<<config[i].h<<std::endl;

    float wSize = 1.0;
    float hSize = .75;
    /*if(config[i].w < wSize){
    //if(config[i].h > hSize){
	float diff = config[i].w- wSize;
	config[i].w -= diff;
	config[i].h -= diff;
	//}
    }
    if(config[i].h > hSize){
      //if(config[i].w < wSize){
      float diff = config[i].h-hSize;
      config[i].w -= diff;
      config[i].h -=diff;
      //}
      }*/
    config[i].position.x = (1.0-config[i].w)/2.0f;
    config[i].position.y = (1.0- config[i].h)/2.0f;
    std::cout<<"w,h "<<config[i].w<< ", "<<config[i].h<<std::endl;


    //boundingBoxes[i].position+= config[i].position;
    //change this
    //boundingBoxes[i].w = .25;
    //boundingBoxes[i].h = .25;
    //boundingBoxes[i].position.x = .25;
    //boundingBoxes[i].position.y = .25;

    glBindTexture( GL_TEXTURE_2D, 0 );
    glUseProgramObjectARB( 0 );
    glColor4f(.95f, .94f, .10f, 1.0f );
    //drawWireframeQuad( boundingBoxes[i] );

    //setupShader( SHADER_STREAM );
    setupTexture( config[i].texId );
    drawQuad( config[i] );


  }
  glBindTexture( GL_TEXTURE_2D, 0 );
  using std::string;
  setupShader( SHADER_STREAM );
  setupTexture( gQuads[INITIAL_APPROVAL].texId );
  drawQuad( gQuads[INITIAL_APPROVAL] );
  
  glDisable( GL_TEXTURE_2D );
  glUseProgramObjectARB( 0 );

  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
}

void drawQuad(Quad quad){
  drawQuad(quad.position.x, quad.position.y, quad.position.z, quad.w, quad.h, quad.origin );
}

void drawQuad(Quad quad, Origin corner){
  drawQuad(quad.position.x, quad.position.y, quad.position.z, quad.w, quad.h, corner);
}

void drawQuad(Origin corner){
  drawQuad(0.0, 0.0, 0.0, 1.0, 1.0, corner);
}
void drawQuad(float x, float y, float z, float w, float h, Origin corner){
  switch(corner){
  case BOTTOM_LEFT: break;
  case TOP_LEFT:  y = 1.0-(h+y); break;
  case BOTTOM_RIGHT: x = 1.0 - (w+x); break;
  case TOP_RIGHT: x = 1.0 - (w+x); y = 1.0-(h+y); break;
  }

  glBegin( GL_QUADS );
  glTexCoord2f(0.0, 0.0);
  glVertex3f( x, y, z );
  glTexCoord2f(1.0, 0.0);
  glVertex3f( x + w, y, z );
  glTexCoord2f(1.0, 1.0);
  glVertex3f( x + w, y + h, z );
  glTexCoord2f(0.0, 1.0);
  glVertex3f( x , y + h, z );
  glEnd();
}

void drawWireframeQuad( Quad quad ){
  drawWireframeQuad(quad.position.x, quad.position.y, quad.position.z, quad.w, quad.h, quad.origin );
}

void drawWireframeQuad( float x, float y, float z, float w, float h, Origin corner ){
  switch(corner){
  case BOTTOM_LEFT: break;
  case TOP_LEFT:  y = 1.0-(h+y); break;
  case BOTTOM_RIGHT: x = 1.0 - (w+x); break;
  case TOP_RIGHT: x = 1.0 - (w+x); y = 1.0-(h+y); break;
  }
  glLineWidth(10);
  glBegin( GL_LINE_LOOP );
  glTexCoord2f(0.0, 0.0);
  glVertex3f( x, y, z );
  glTexCoord2f(1.0, 0.0);
  glVertex3f( x + w, y, z );
  glTexCoord2f(1.0, 1.0);
  glVertex3f( x + w, y + h, z );
  glTexCoord2f(0.0, 1.0);
  glVertex3f( x , y + h, z );
  glEnd();
}

void drawSoundLine(){
  /*  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboId );
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			     GL_TEXTURE_2D, gTexIds[SOUND_TEX], 0 );
  glUseProgramObjectARB(gPrograms[SHADER_SOUNDLINE]);

  GLuint aLoc = glGetUniformLocationARB( gPrograms[SHADER_SOUNDLINE], "A");
  GLuint tLoc = glGetUniformLocationARB( gPrograms[SHADER_SOUNDLINE], "t");
  GLuint lambdaLoc = glGetUniformLocationARB( gPrograms[SHADER_SOUNDLINE], "lambda" );
  GLuint wLoc = glGetUniformLocationARB( gPrograms[SHADER_SOUNDLINE], "w" );

  glUniform1f(aLoc, 0.5f);
  glUniform1f(lambdaLoc, 0.1);
  glUniform1f(wLoc, 10.0f);
  glUniform1f(tLoc, gTime);

  glBegin( GL_LINES );
  glTexCoord2f(0.0, 0.0); glVertex2f(0.0f, 0.25f);
  glTexCoord2f(1.0, 0.0); glVertex2f(1.0f, 0.25f);
  glEnd();

  glUseProgramObjectARB(0);*/
}

std::string getTimestamp(){
  char timestamp[256];
  time_t rawtime;
  struct tm * timeinfo;
  time( &rawtime );
  timeinfo = localtime( &rawtime );
  strftime( timestamp, 256, "%S%M%H%d%C%m", timeinfo );
  return std::string(timestamp);
}
void takeScreenshot(){
  using std::string;
  string timestamp = getTimestamp();
  string filename = string(SCREENSHOT_DIR) + timestamp + string(".jpg");
  std::cout<<"Filename: "<<filename.c_str()<<std::endl;
  cv::Mat rgbFrame = getFrame(1.0, false);
  cv::Mat frame;
  cv::cvtColor( rgbFrame, frame, CV_RGB2BGR);
  cv::imwrite(filename, frame );
}


/**Shader**/

bool load_shader( const char* file, GLint type, GLhandleARB program ){
  std::ifstream infile;
  char* buffer;
  char error_msg[2048];
  GLhandleARB shader;
  
  infile.open( file );
  if( infile.fail() ){
    std::cout<<"Error cannot open file: "<<file <<std::endl;
    infile.close();
    return false;
  }

  infile.seekg( 0, std::ios::end );
  int length = infile.tellg();
  infile.seekg( 0, std::ios::beg );
  buffer = (char*) malloc( (length+1 ) * sizeof *buffer );
  if( !buffer )
    return false;
  infile.getline( buffer, length, '\0');
  infile.close();
  shader = glCreateShaderObjectARB( type );
  const char * src= buffer;
  glShaderSource( shader, 1, &src, NULL );
  glCompileShader( shader );
  GLint result;
  glGetObjectParameterivARB( shader, GL_OBJECT_COMPILE_STATUS_ARB, &result );
  if(result != GL_TRUE ){
    glGetInfoLogARB( shader, sizeof error_msg, NULL, error_msg );
    std::cout<< "GLSL COMPILE ERROR in "<< file <<": "<<error_msg <<std::endl;
    return false;
  }else{
    std::cout<<"Compiled shaders successfully" <<std::endl;
  }

  glAttachObjectARB( program, shader );
  free( buffer );
  return true;
}

bool create_shader( GLhandleARB program, const char * vert_file, const char* frag_file ){
  bool rv = true;
  std::cout<< "Loading vertex shader "<< vert_file
	   <<"\nLoading fragment shader "<< frag_file << std::endl;
  rv = rv && load_shader( vert_file, GL_VERTEX_SHADER, program );
  rv = rv && load_shader( frag_file, GL_FRAGMENT_SHADER, program );
  
  if( !rv )
    return false;
  
  glLinkProgram( program );
  GLint result;
  glGetProgramiv( program, GL_LINK_STATUS, &result );
  if( result == GL_TRUE ){
    std::cout << "Succesfully linked shader"<<std::endl;
    return true;
  }else{
    std::cout << "FAILED to link shader" <<std::endl;
    return false;
  }
}


/** ROS **/
void publishImage(){
  cv_bridge::CvImage imageMsg( std_msgs::Header(),
			       sensor_msgs::image_encodings::RGB8,
			       getFrame(1.0f, false) );
  publishers.image.publish( imageMsg.toImageMsg () );
  ROS_INFO("GUI SENT IMAGE");
}

/** State Management**/
void resetPicsToSend(){gPicsToSend = PICTURES_TO_SEND;}

void setState( State state ){
  gState = state;
  switch( state ){
  case WAITING_FOR_TRIGGER:
    resetAnimationRates();
    resetPicsToSend();
    gQuads[REAR_QUAD].shader = SHADER_BW;
    break;
  case LISTENING:
    gQuads[FRONT_QUAD].shader = SHADER_STREAM;
    //gAnalyzedFrame = getFrame();
    break;
  case THINKING:
    gQuads[FRONT_QUAD].shader = SHADER_STREAM;
    break;
  case RESULT_OPTIONS:{
    /*gQuads[REAR_QUAD].shader = SHADER_STREAM;
    ImageList images;
    images.push_back(loadImage("/home/mmfps/Pictures/Webcam/test1.jpg"));
    images.push_back(loadImage("/home/mmfps/Pictures/Webcam/test2.jpg"));
    images.push_back(loadImage("/home/mmfps/Pictures/Webcam/test3.jpg"));
    images.push_back(loadImage("/home/mmfps/Pictures/Webcam/test4.jpg"));

    setResultsTexture(gResultsConfig[FOUR_RESULTS], images);*/
    gQuads[REAR_QUAD].shader = SHADER_STREAM;
    gAnimationParams.animation = CARD_FLIP;
    break;
  }
  case FINAL_RESULT:{
    cv::Mat frame = getFrame();
    Quad active = (*gActiveResultsConfig)[gSelectedResultFromConfig];
    gQuads[SELECTED_RESULT] = active;
    
    gQuads[SELECTED_RESULT].texId = active.texId;
    gToRender.push_back(&gQuads[SELECTED_RESULT]);
    
    gAnimationParams.path = (gQuads[SELECTED_RESULT].position + gModelParams.position) -
      gCameraParams.cameraPosition;
    gAnimationParams.pathLength = gAnimationParams.path.length();
    gAnimationParams.pathDir = gAnimationParams.path.normalize();
    gAnimationParams.animation = CENTER_FINAL_RESULT;
    break;
  }
  case SHOWING_COMMAND_OPTIONS:{

    gToRender.pop_back();

    resetAnimationRates();

    cv::Mat image = loadImage("/home/mmfps/Pictures/Webcam/test5.jpg");
    gAnimationParams.animation = OPEN_INFORMATION;
    gAnimationParams.openInfoComplete = false;


    gQuads[USER_APPROVAL] = gQuads[SELECTED_RESULT];
    gQuads[USER_APPROVAL].h *= .25;
    gQuads[USER_APPROVAL].origin = TOP_LEFT;
    gQuads[USER_APPROVAL].position.y += gQuads[SELECTED_RESULT].h - gQuads[USER_APPROVAL].h;
    gToRender.push_back(&gQuads[USER_APPROVAL]);

    gQuads[SELECTED_RESULT].origin = TOP_LEFT; 

    gQuads[COMMAND_OPTIONS] = gQuads[SELECTED_RESULT];
    gQuads[COMMAND_OPTIONS].origin = TOP_RIGHT;
    ImageList images;
    using std::string;
    string filename;

    for(int i = 0; i < gFinalInfo.commandOptions.size(); i++){
      if( gFinalInfo.commandOptions[i].compare("info") == 0){
	filename = string(IMAGE_DIR) + string("info_feedback.jpg");
	images.push_back(loadImage(filename.c_str() ) );
      }
      else if( gFinalInfo.commandOptions[i].compare("price") == 0 ){
	filename = string(IMAGE_DIR) + string("price_feedback.jpg");
	images.push_back(loadImage(filename.c_str() ) );
      }
      else if( gFinalInfo.commandOptions[i].compare("review") == 0 ){
	filename = string(IMAGE_DIR) + string("review_feedback.jpg");
	images.push_back(loadImage(filename.c_str() ) );
      }

    }
    setCommandOptionsTexture(images);
    //updateTexture( gQuads[COMMAND_OPTIONS].texId, 0, 200, images[0].cols, images[0].rows-200, images[0].data);
    gToRender.push_back(&gQuads[COMMAND_OPTIONS]);

    gToRender.push_back(&gQuads[SELECTED_RESULT]);

    break;
  }

  case SHOWING_INFORMATION:{

    resetAnimationRates();
    gAnimationParams.animation = CLOSE_INFORMATION;

    gQuads[RESULT_INFO].position.x = .1f;
    gQuads[RESULT_INFO].position.y = 1.0f;
    gQuads[RESULT_INFO].w = .75;
    gQuads[RESULT_INFO].h = .70;
    
    using std::string;
    std::cout<<"Final info: "<<gFinalInfo.command<<", "<<gFinalInfo.object<<std::endl;
    string filename = string(RESULT_INFO_DIR) + string(gFinalInfo.command) +
    "/"+ string(gFinalInfo.object) +string(".jpg");
    std::cout<<"Filename: "<<filename.c_str()<<std::endl;
    //cv::Mat image = loadImage("/home/mmfps/Pictures/Webcam/test5.jpg");
    cv::Mat image = loadImage(filename.c_str());
    setInformationTexture(image);

    break;
  }
  case RESETTING:
    resetAnimationRates();
    gFinalInfo.commandOptions.clear();
    gFinalInfo.objectOptions.clear();
    gAnimationParams.animation = FALL_AND_FADE;
    break;
  }
  gTime = 0;
}


cv::Mat getFrame(float imageRatioMultiplier, bool flipped){
  cv::Mat camFrame(cvQueryFrame(gCaptor));
  std::cout<<"2 init done*******************"<<std::endl;  
  cv::Mat flippedFrame;
  cv::Mat frameResize;
  cv::cvtColor( camFrame, flippedFrame, CV_BGR2RGB);
  if(flipped){
    cv::flip( flippedFrame, frameResize, 0 );
  }else{
    frameResize = flippedFrame;
  }
  int width = frameResize.cols * imageRatioMultiplier;
  int height = frameResize.rows * imageRatioMultiplier;
  cv::Mat frame( height, width, CV_8U );
  cv::resize( frameResize, frame, frame.size(), 0, 0, cv::INTER_CUBIC );
  return frame;
}

cv::Mat loadImage(const char* path, bool resize){
  cv::Mat temp = getFrame();
  cv::Mat loadFrame;
  try{
    loadFrame = cv::imread(path);
  }catch(cv::Exception& e){
    std::cout<<"Couldn't load image"<<std::endl;
    return temp;
  }
  cv::Mat flippedFrame;
  cv::Mat frameResize;
  cv::Mat frame;
  cv::cvtColor( loadFrame, flippedFrame, CV_BGR2RGB);
  cv::flip( flippedFrame, frameResize, 0 );
  if(resize)
    cv::resize( frameResize, frame, temp.size(), 0, 0, cv::INTER_CUBIC );
  else
    frame = frameResize;
  return frame;
}



void processKeypress(const SDL_keysym * keysym ){
  switch( keysym->sym ){
  case SDLK_ESCAPE: gRunning = false; break;
  case SDLK_1: setState(WAITING_FOR_TRIGGER); gQuads[FRONT_QUAD].shader = SHADER_BW; break;
  case SDLK_2: setState(LISTENING); break;
    //case SDLK_3: setState(THINKING); break;
    //case SDLK_4: setState(RESULT_OPTIONS); break; 
    //case SDLK_5: if(gState == RESULT_OPTIONS) setState(FINAL_RESULT);break;
    //case SDLK_6: if(gState == FINAL_RESULT) setState( SHOWING_COMMAND_OPTIONS ); break;
    //case SDLK_6: if(gState == SHOWING_COMMAND_OPTIONS) setState( RESETTING ); break;
  case SDLK_6: if(gState == SHOWING_COMMAND_OPTIONS) setState( SHOWING_INFORMATION ); break;
  case SDLK_7: if(gState == SHOWING_INFORMATION) setState( RESETTING ); break;
  case SDLK_8:{
    if(gState==LISTENING){
      std::string msg ="retail\nprice review price review info\ndentalfloss cleanser\n0 1 2 3\n4 5 6 7\n7 7 7 7\n1 2 3 4";
      dummy_callback_bnet_final_result( msg );
    }
    break;
  }
  case SDLK_c: {
    gCameraNumber++;
    if(gCameraNumber > 1) gCameraNumber = 0;
    cvReleaseCapture(&gCaptor);
    gCaptor = cvCaptureFromCAM( gCameraNumber );
  }
    break;
  case SDLK_RIGHT: gModelParams.position+= Vector3(-.01, 0.0, 0.0); break;
  case SDLK_LEFT: gModelParams.position+= Vector3(.01, 0.0, 0.0); break;
  case SDLK_UP: gModelParams.position+= Vector3(0.0, -.01, 0.0); break;
  case SDLK_DOWN: gModelParams.position+= Vector3(0.0, .01, 0.0); break;

  case SDLK_w: gModelParams.position+= Vector3(0.0, 0.0, .01); break;
  case SDLK_h: gModelParams.position+= Vector3(0.0, 0.0, -.01); break;
    
  case SDLK_s: takeScreenshot(); break;
  case SDLK_u:
    if( gState == RESULT_OPTIONS ){
      if(gActiveResultsConfig->size() > 0){
	gSelectedResultFromConfig = 0;
	gFinalInfo.object = gFinalInfo.objectOptions[0];
	setState(FINAL_RESULT);
      }
    }
    break;
  case SDLK_i:
    if( gState == RESULT_OPTIONS ){
      if(gActiveResultsConfig->size() > 1){
	gSelectedResultFromConfig = 1;
	gFinalInfo.object = gFinalInfo.objectOptions[1];
	setState(FINAL_RESULT);
      }
    }
    break;

  case SDLK_o:
    if( gState == RESULT_OPTIONS ){
      if(gActiveResultsConfig->size() > 2){
	gSelectedResultFromConfig = 2; 
	gFinalInfo.object = gFinalInfo.objectOptions[2];
	setState(FINAL_RESULT);
      }
    }
    break;
  case SDLK_p:
    if( gState == RESULT_OPTIONS ){
      if(gActiveResultsConfig->size() > 3){
	gSelectedResultFromConfig = 3;  
	gFinalInfo.object = gFinalInfo.objectOptions[3];
	setState(FINAL_RESULT);
      }
    }
    break;
  case SDLK_j:
    if( gFinalInfo.commandOptions.size() > 0 ){
      gFinalInfo.command = gFinalInfo.commandOptions[0];
      setState(SHOWING_INFORMATION );
    }
    break;
  case SDLK_k:
    if( gFinalInfo.commandOptions.size() > 1 ){
      gFinalInfo.command = gFinalInfo.commandOptions[1];
      setState(SHOWING_INFORMATION );
    }
    break;
  case SDLK_l:
    if( gFinalInfo.commandOptions.size() > 2 ){
      gFinalInfo.command = gFinalInfo.commandOptions[2];
      setState(SHOWING_INFORMATION );
    }
    break;
  default: break;

  }
}

void processClick( const SDL_MouseButtonEvent* button ){
  if(gState == RESULT_OPTIONS){
    float x = button->x;
    float y = button->y;
    float factor = IMG_SIZE_RATIO_MULTIPLIER / 2.2f;
    std::cout<<"button! ("<<x<<", "<<y<<")"<<std::endl;
    if(x > (463*factor) && x < (651*factor) && y > (968*factor) && y < (1031*factor)){
      std::cout<<"Right! ("<<x<<", "<<y<<")"<<std::endl;
      std_msgs::String msg;
      msg.data = std::string("");
      publishers.feedback.publish(msg);
      setState( RESETTING );
    }
    if( x>(738*factor) && x<(972*factor) && y>(965*factor) && y <(1036*factor)){
      std::cout<<"Wrong! ("<<x<<", "<<y<<")"<<std::endl;
      std_msgs::String msg;
      msg.data = std::string("");
      publishers.feedback.publish(msg);
      setState( RESETTING );
    }
  }

}

void handle_events(){
  SDL_Event event;
  while( SDL_PollEvent(&event) ) {
    switch( event.type ){
    case SDL_ACTIVEEVENT:
      if( event.active.gain == 0 ) {}//could add active here
	break;
    case SDL_KEYDOWN: processKeypress( &event.key.keysym ); break;
    case SDL_MOUSEBUTTONDOWN: processClick( &event.button); break;
    case SDL_QUIT: gRunning = false; break;
    }
  }
}


void resizeWindow( int width, int height ){
  using namespace boost::numeric::ublas;
  std::vector<float> v(3);
  gScreenWidth = width;
  gScreenHeight = height;
  glViewport( 0, 0, gScreenWidth, gScreenHeight );
  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  
  //GLfloat aspectRatio = (GLfloat)gScreenWidth/(GLfloat)gScreenHeight;
  gluPerspective( gCameraParams.FOV, 1.0f, 0.1f, 100.0f );
  gluLookAt( gCameraParams.cameraPosition.x, gCameraParams.cameraPosition.y, gCameraParams.cameraPosition.z,
	     gCameraParams.cameraReference.x, gCameraParams.cameraReference.y, gCameraParams.cameraReference.z,
	     gCameraParams.cameraUp.x, gCameraParams.cameraUp.y, gCameraParams.cameraUp.z );
  
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
}


/**Audio*/
void captureAudio(){

  short buffer[AUDIO_FREQ*2]; //holds captured audio
  ALCint samplesIn = 0;
  ALint availBuffers = 0;
  ALuint myBuff;
  ALuint buffHolder[16];

  alGetSourcei( audio.helloSource[0], AL_BUFFERS_PROCESSED, &availBuffers );
  if( availBuffers > 0 ){
    alSourceUnqueueBuffers( audio.helloSource[0], availBuffers, buffHolder );
    for( int ii=0; ii < availBuffers; ++ii ){
      audio.bufferQueue.push_back( buffHolder[ii] );
    }
  }
  alcGetIntegerv( audio.inputDevice, ALC_CAPTURE_SAMPLES, 1, &samplesIn);
  if( samplesIn > CAP_SIZE ){
    //get the sound
    alcCaptureSamples( audio.inputDevice, buffer, CAP_SIZE );
    float avg = 0.0f;
    for( int ii = 0; ii < AUDIO_FREQ/2; ++ii ){
      avg += buffer[ii];
    }

    if( !audio.bufferQueue.empty() ){
      myBuff = audio.bufferQueue.front(); audio.bufferQueue.pop_front();
      alBufferData( myBuff, AL_FORMAT_MONO16, buffer, CAP_SIZE * sizeof(short), AUDIO_FREQ );
      alSourceQueueBuffers( audio.helloSource[0], 1, &myBuff ); 
      
      ALint sState = 0;
      alGetSourcei( audio.helloSource[0], AL_SOURCE_STATE, &sState );
      if( sState != AL_PLAYING){
	alSourcePlay( audio.helloSource[0] );
      }
    }
  }
}

void stopAudioCapture(){
  alcCaptureStop( audio.inputDevice );
  alcCaptureCloseDevice( audio.inputDevice );
  alSourceStopv( 1, &audio.helloSource[0] );
  for( int ii=0; ii < 1; ++ii){
    alSourcei(audio.helloSource[ii], AL_BUFFER, 0 );
  }
  
  alDeleteSources(1, &audio.helloSource[0]);
  //alDeleteBuffers(16, &helloSourceBuffer[0] );
  //audio.errorCode = alGetError();
  //alDeleteBuffers( 16, &helloBufer[0] );
  //audio.errorCode = alGetError();
  alcMakeContextCurrent( NULL );
  //audio.errorCode = alcGetError();
  alcDestroyContext( audio.audioContext );
  alcCloseDevice( audio.audioDevice );
}

/** Callbacks**/
void callback_trigger(const std_msgs::String::ConstPtr& msg){
  ROS_INFO("Gui trigger heard: %s", msg->data.c_str() );
  if(msg->data.find("marvin") != std::string::npos )
    setState(LISTENING);
}
void dummy_callback_bnet_final_result( std::string msg ){

  using namespace std;
  istringstream iss(msg);
  string token;
  deque<string> lines;
  //vector<string> commands;
  //vector<string> objects;
  vector<float> coords;
  vector<Quad> boundingBoxes;
  char* temp=NULL;
  char line[255];
  while( getline( iss, token ) ){
    lines.push_back(token);
  }

  //don't forget to trim the null delimination
  //add a global scale parameter
  lines.pop_front(); //retail
  strcpy(line, lines.front().c_str());
  temp = strtok( line, " "); lines.pop_front(); //commands
  while( temp!= NULL) {
    gFinalInfo.commandOptions.push_back( string(temp ) );
    //cout<<"cmd: "<<gFinalInfo.commandOptions.back()<<std::endl;
    temp = strtok(NULL, " ");
  }

  strcpy(line, lines.front().c_str());
  temp = strtok( line, " "); lines.pop_front();
  while( temp!=NULL){
    gFinalInfo.objectOptions.push_back( string(temp) ); 
    //cout<<"obj: "<<gFinalInfo.objectOptions.back() <<endl;
    temp = strtok(NULL, " ");
  }
  
  for(int i = 0; i < gFinalInfo.objectOptions.size(); i++){
    strcpy(line, lines.front().c_str());
    temp = strtok( line, " "); lines.pop_front();
    while( temp!=NULL ){
      coords.push_back(atof( temp ));
      temp = strtok(NULL, " ");
    }      
    //std::cout<<"coords: "<<coords[0]<<", "<< coords[1]<<", "<<coords[2]<<", "<<coords[3]<<endl;
    boundingBoxes.push_back(Quad(coords[0], coords[1], 0.0f,
				 coords[0]+coords[2], coords[1] + coords[3], TOP_LEFT ) );
    //cout<<"Quad pos: "<<boundingBoxes[0].position<<endl;
    coords.clear();
  }
  
  if(lines.size() > 0){
    ROS_INFO("THERES FACE DATA!");
  }
  
  ImageList images;
  ResultConfiguration config;
  for(int i = 0; i < 4; i++){
    images.push_back(gAnalyzedFrame);
  }
  cout<<"object size "<<gFinalInfo.objectOptions.size()<<endl;
  /*
  switch( gFinalInfo.objectOptions.size() ){
  case 1: gActiveResultsConfig = &gResultsConfig[ONE_RESULT]; break;
  case 2: gActiveResultsConfig = &gResultsConfig[TWO_RESULTS]; break;
  case 3: gActiveResultsConfig = &gResultsConfig[THREE_RESULTS]; break;
  default: gActiveResultsConfig = &gResultsConfig[FOUR_RESULTS]; break;
  }*/

  gActiveResultsConfig = &gResultsConfig[ONE_RESULT];

  gFinalInfo.command = gFinalInfo.commandOptions[0];
  gFinalInfo.object = gFinalInfo.objectOptions[0];

  std::cout<<"Final info: "<<gFinalInfo.command<<", "<<gFinalInfo.object<<std::endl;

  string filename = string(RESULT_INFO_DIR) + string(gFinalInfo.command) +
    "/"+ string(gFinalInfo.object) +string(".jpg");
  std::cout<<"Filename: "<<filename.c_str()<<std::endl;
  //cv::Mat image = loadImage("/home/mmfps/Pictures/Webcam/test5.jpg");
  cv::Mat image = loadImage(filename.c_str());
  //setInformationTexture(image);
  ImageList imgs;
  imgs.push_back(image);
  QuadList quads;
  quads.push_back(Quad());

  setResultsTexture( *gActiveResultsConfig, imgs, quads );
  setState(RESULT_OPTIONS);

}
void callback_bnet_final_result( const std_msgs::String::ConstPtr& msg ){
  ROS_INFO("Gui bnet heard: %s", msg->data.c_str() );
  if(gState == LISTENING){
    using namespace std;
    istringstream iss(msg->data);
    string token;
    deque<string> lines;
    //vector<string> commands;
    //vector<string> objects;
    vector<float> coords;
    vector<Quad> boundingBoxes;
    char* temp=NULL;
    char line[255];
    while( getline( iss, token ) ){
      lines.push_back(token);
    }

    //don't forget to trim the null delimination
    //add a global scale parameter
    lines.pop_front(); //retail
    strcpy(line, lines.front().c_str());
    temp = strtok( line, " "); lines.pop_front(); //commands
    while( temp!= NULL) {
      gFinalInfo.commandOptions.push_back( string(temp ) );
      cout<<"cmd: "<<gFinalInfo.commandOptions.back()<<std::endl;
      temp = strtok(NULL, " ");
    }

    strcpy(line, lines.front().c_str());
    temp = strtok( line, " "); lines.pop_front();
    while( temp!=NULL){
      gFinalInfo.objectOptions.push_back( string(temp) ); 
      cout<<"obj: "<<gFinalInfo.objectOptions.back() <<endl;
      temp = strtok(NULL, " ");
    }

    cv::Mat frame = getFrame(1.0, false);
  
    for(int i = 0; i < gFinalInfo.objectOptions.size(); i++){
      strcpy(line, lines.front().c_str());
      temp = strtok( line, " "); lines.pop_front();
      while( temp!=NULL ){
	coords.push_back(atof( temp ));
	temp = strtok(NULL, " ");
      }      
      coords[0]/= frame.cols;
      coords[2]/= frame.cols;
      coords[1]/= frame.rows;
      coords[3]/= frame.rows;
      std::cout<<"coords: "<<coords[0]<<", "<< coords[1]<<", "<<coords[2]<<", "<<coords[3]<<endl;
      boundingBoxes.push_back(Quad(coords[0], coords[1], 0.0f,
				   fabs(coords[0]-coords[2]), fabs(coords[1]-coords[3]), TOP_LEFT ) );
      cout<<"Quad pos: "<<boundingBoxes[0].position<<endl;
      coords.clear();
    }
  
    if(lines.size() > 0){
      ROS_INFO("THERES FACE DATA!");
    }
  

  ImageList images;
  ResultConfiguration config;
  for(int i = 0; i < 4; i++){
    images.push_back(gAnalyzedFrame);
  }
  cout<<"object size "<<gFinalInfo.objectOptions.size()<<endl;
  /*
  switch( gFinalInfo.objectOptions.size() ){
  case 1: gActiveResultsConfig = &gResultsConfig[ONE_RESULT]; break;
  case 2: gActiveResultsConfig = &gResultsConfig[TWO_RESULTS]; break;
  case 3: gActiveResultsConfig = &gResultsConfig[THREE_RESULTS]; break;
  default: gActiveResultsConfig = &gResultsConfig[FOUR_RESULTS]; break;
  }*/

  gActiveResultsConfig = &gResultsConfig[ONE_RESULT];

  //gFinalInfo.command = gFinalInfo.commandOptions[0];
  gFinalInfo.command = string("price");
  gFinalInfo.object = gFinalInfo.objectOptions[0];

  std::cout<<"Final info: "<<gFinalInfo.command<<", "<<gFinalInfo.object<<std::endl;

  string filename = string(RESULT_INFO_DIR) + string(gFinalInfo.command) +
    "/"+ string(gFinalInfo.object) +string(".jpg");
  std::cout<<"Filename: "<<filename.c_str()<<std::endl;
  //cv::Mat image = loadImage("/home/mmfps/Pictures/Webcam/test5.jpg");
  cv::Mat image = loadImage(filename.c_str());
  //setInformationTexture(image);
  ImageList imgs;
  imgs.push_back(image);
  QuadList quads;
  quads.push_back(Quad());

  setResultsTexture( *gActiveResultsConfig, imgs, quads );
  setState(RESULT_OPTIONS);
  }
}
/**/

template<class T>
void _swap(T& a, T& b){
  T c = a;
  a = b;
  b = c;
}

void swap(Quad& a, Quad& b){
  swap(a.texId, b.texId);
  swap(a.shader, b.shader);
}

void cleanUp(){
  SDL_FreeSurface(gMainWindow);
  cvReleaseCapture(&gCaptor);
  SDL_Quit();
}


/**Init**/
void initResultsConfigurations(){

  float height = 0.5f;
  float width = 0.5f;

  gResultsConfig[ONE_RESULT].push_back( Quad(0.0 ,0.0, 0.0, 1.0, 1.0, TOP_LEFT) );

  gResultsConfig[TWO_RESULTS].push_back( Quad(0.0, .5-height/2.0, 0.0, width, height, TOP_RIGHT));
  gResultsConfig[TWO_RESULTS].push_back( Quad(0.0, .5-height/2.0, 0.0, width, height, TOP_LEFT));

  gResultsConfig[THREE_RESULTS].push_back(Quad(width - width/2.0, 0.0f, 0.0, width, height, TOP_LEFT));
  gResultsConfig[THREE_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, BOTTOM_LEFT));
  gResultsConfig[THREE_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, BOTTOM_RIGHT));

  gResultsConfig[FOUR_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, TOP_RIGHT));
  gResultsConfig[FOUR_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, BOTTOM_LEFT));
  gResultsConfig[FOUR_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, TOP_LEFT));
  gResultsConfig[FOUR_RESULTS].push_back( Quad(0.0, 0.0, 0.0, width, height, BOTTOM_RIGHT));

}
void initFBO(){
  glGenFramebuffersEXT(1, &fboId );
}

void createTexture( GLuint texId, int width, int height, GLvoid* data ){
    glBindTexture( GL_TEXTURE_2D, texId );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		  width, height,
		  0, GL_RGB, GL_UNSIGNED_BYTE, data );
    //glGenerateMipmap( GL_TEXTURE_2D );

}
void initTextures(){
  cv::Mat frame = getFrame();

  assert(!frame.empty());

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );  
  std::cout<<"w,h "<<frame.cols<<", "<<frame.rows<<std::endl;

  int toCreate = 9;
  gTexIndex+= toCreate;
  glGenTextures( toCreate, gTexIds );
  createTexture(gTexIds[STREAM_TEX], frame.cols, frame.rows, NULL );
  createTexture(gTexIds[RESULT_TEX], frame.cols, frame.rows, NULL );
  createTexture(gTexIds[COMMAND_OPTIONS_TEX], frame.cols, frame.rows, NULL);
  //createTexture(gTexIds[RESULT_OPTION_TEX], RESULT_OPTION_TEX, frame.cols, frame.rows, NULL );
  createTexture(gTexIds[SELECTED_RESULT_TEX], frame.cols, frame.rows, NULL );
  createTexture(gTexIds[RESULT_INFO_TEX], frame.cols, frame.rows, NULL );

  //createTexture(gTexIds[SOUND_TEX], SOUND_TEX, frame.cols, 1 );
  gQuads[FRONT_QUAD].texId = gTexIds[STREAM_TEX];
  gQuads[REAR_QUAD].texId = gTexIds[RESULT_TEX];
  gQuads[COMMAND_OPTIONS].texId = gTexIds[COMMAND_OPTIONS_TEX];
  gQuads[SELECTED_RESULT].texId = gTexIds[SELECTED_RESULT_TEX];
  gQuads[RESULT_INFO].texId = gTexIds[RESULT_INFO_TEX];



  using std::string;
  string filename = string(IMAGE_DIR) + string("user_feedback.jpg");
  cv::Mat UAImg = loadImage(filename.c_str());
  createTexture(gTexIds[USER_APPROVAL_TEX], UAImg.cols, UAImg.rows, UAImg.data );
  gQuads[USER_APPROVAL].texId = gTexIds[USER_APPROVAL_TEX];
  
  filename = string(IMAGE_DIR) + string("review_feedback.jpg");
  UAImg = loadImage(filename.c_str());
  createTexture(gTexIds[CMD_OPTION_TEX], UAImg.cols, UAImg.rows, UAImg.data );
  gQuads[CMD_OPTION].texId = gTexIds[CMD_OPTION_TEX];
  gQuads[CMD_OPTION].w = 2*(640/(float)gScreenWidth);
  gQuads[CMD_OPTION].h = 2*(120/(float)gScreenHeight);

  filename = string(IMAGE_DIR) + string("user_feedback.jpg");
  UAImg = loadImage(filename.c_str());
  createTexture(gTexIds[INITIAL_APPROVAL_TEX], UAImg.cols, UAImg.rows, UAImg.data );
  gQuads[INITIAL_APPROVAL].texId = gTexIds[INITIAL_APPROVAL_TEX];
  gQuads[INITIAL_APPROVAL].w = 640/(float)gScreenWidth;
  gQuads[INITIAL_APPROVAL].h = 120/(float)gScreenHeight;
  gQuads[INITIAL_APPROVAL].position.x = 0.5f - gQuads[INITIAL_APPROVAL].w/2.0f;
  gQuads[INITIAL_APPROVAL].position.y = 1.0f - gQuads[INITIAL_APPROVAL].h;


  for(int j = 0; j < 4; j++ ){
    for(int i = 0; i < gResultsConfig[j].size(); i++ ){
      glGenTextures( 1, &gResultsConfig[j][i].texId );
      //createTexture( gResultsConfig[j][i].texId, frame.cols, frame.rows, NULL );
    }
  }

  glBindTexture( GL_TEXTURE_2D, 0 );
}

bool initShaders(){
  bool rv = true;
  gPrograms[SHADER_STREAM] = glCreateProgramObjectARB();
  gPrograms[SHADER_BLUR] = glCreateProgramObjectARB();
  gPrograms[SHADER_GAUSSIAN_BORDER] = glCreateProgramObjectARB();
  gPrograms[SHADER_BW] = glCreateProgramObjectARB();
  gPrograms[SHADER_FADED] = glCreateProgramObjectARB();
  gPrograms[SHADER_BORDER] = glCreateProgramObjectARB();
  //gPrograms[SHADER_SOUNDLINE] = glCreateProgramObjectARB();
  
  std::string vert_shader, frag_shader;
  vert_shader = std::string(SHADER_DIR) + std::string("vert_shader.glsl");
  frag_shader = std::string(SHADER_DIR) + std::string("stream_frag_shader.glsl");
  rv = rv && create_shader( gPrograms[SHADER_STREAM], vert_shader.c_str(), frag_shader.c_str());
  frag_shader = std::string(SHADER_DIR) + std::string("stream_frag_shader.glsl");
  rv = rv && create_shader( gPrograms[SHADER_BLUR], vert_shader.c_str(), frag_shader.c_str());
  frag_shader = std::string(SHADER_DIR) + std::string("circular_blur_frag_shader.glsl");
  rv = rv && create_shader( gPrograms[SHADER_GAUSSIAN_BORDER], vert_shader.c_str(), frag_shader.c_str());
  frag_shader = std::string(SHADER_DIR) + std::string("bw_frag_shader.glsl");
  rv = rv && create_shader( gPrograms[SHADER_BW], vert_shader.c_str(), frag_shader.c_str());
  frag_shader = std::string(SHADER_DIR) + std::string("faded_frag_shader.glsl");
  rv = rv && create_shader( gPrograms[SHADER_FADED], vert_shader.c_str(), frag_shader.c_str());
  frag_shader = std::string(SHADER_DIR) + std::string("border_frag.glsl");
  rv = rv && create_shader( gPrograms[SHADER_BORDER], vert_shader.c_str(), frag_shader.c_str());


  //frag_shader = std::string(SHADER_DIR) + std::string("calc_sound_frag.glsl");
  //rv = rv && create_shader( gPrograms[SHADER_SOUNDLINE], vert_shader.c_str(), frag_shader.c_str() );

  return rv;
}

void initCV(){
  gCaptor = cvCaptureFromCAM( 0 );
  assert(gCaptor);
  cvNamedWindow("test");
}

void initAL(){
  ALuint helloBuffer[16];
  audio.audioDevice = alcOpenDevice(NULL);
  audio.errorCode = alcGetError( audio.audioDevice );
  audio.audioContext = alcCreateContext( audio.audioDevice, NULL );
  alcMakeContextCurrent( audio.audioContext );
  audio.errorCode = alcGetError( audio.audioDevice );
  audio.inputDevice = alcCaptureOpenDevice( NULL, AUDIO_FREQ, AL_FORMAT_MONO16, AUDIO_FREQ/2);
  audio.errorCode = alcGetError( audio.inputDevice );
  alcCaptureStart( audio.inputDevice );
  audio.errorCode = alcGetError( audio.inputDevice );
  alGenBuffers( 16, &helloBuffer[0] );
  audio.errorCode = alGetError();

  for(int ii=0; ii<16;++ii){
    audio.bufferQueue.push_back(helloBuffer[ii]);
  }

  alGenSources( 1, &audio.helloSource[0] );
  audio.errorCode = alGetError();


}

void initGL(){
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  GLenum error = glewInit();
  assert( error == GLEW_OK );
}

bool initSDL(){
  cv::Mat frame = getFrame();
  gScreenWidth = frame.cols; 
  gScreenHeight = frame.rows;
  int videoFlags;
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
    std::cout<<"Error starting SDL\n "<<SDL_GetError() << std::endl;
  }
  videoFlags = SDL_OPENGL;
  videoFlags |= SDL_GL_DOUBLEBUFFER;
  videoFlags |= SDL_HWSURFACE;
  videoFlags |= SDL_HWACCEL;
  videoFlags |= SDL_NOFRAME;
  videoFlags |= SDL_FULLSCREEN;
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  putenv((char*)"SDL_VIDEO_CENTERED=1");
  gMainWindow = SDL_SetVideoMode( gScreenWidth, gScreenHeight, 
				  SCREEN_BPP, videoFlags );
  if( !gMainWindow ){
    std::cout<<"Video mode failed: "<<SDL_GetError()<<std::endl;
    assert(gMainWindow);
  }
  return true;
}

void resetAnimationRates(){
  gAnimationParams.flipRate = 30.0f;
  assert(90 % (int)gAnimationParams.flipRate == 0); //meh
  gAnimationParams.zoomRate = 0.10f;
  gAnimationParams.centerRate = .22f;
  gAnimationParams.separateRate = .1f;
  gAnimationParams.togetherRate = 0.1f;
  gAnimationParams.userApprovalLowerRate = .05f;
  gAnimationParams.fallRate = .01f;
  gAnimationParams.backupRate = 0.30f;
  gAnimationParams.toCornerRate = .10f;
  gAnimationParams.raiseInfoRate = .3f;
  gAnimationParams.shrinkRate = .1f;
}

void initAnimationParams(){

  gAnimationParams.cardRotation = 0.0f;
  gAnimationParams.animation = NO_ANIMATION;
  gAnimationParams.separationDecayRate = .60f;
  gAnimationParams.togetherDecayRate = .70f;
  gAnimationParams.centeringDecayRate = .714f;
  gAnimationParams.fallAccelRate = 1.75;
  gAnimationParams.lowerDecayRate = .90f;
  gAnimationParams.flipDecayRate = .82;
  gAnimationParams.openInfoComplete = false;
  gAnimationParams.centeredPosition = Vector3(fabs(gModelParams.position.x/2.0f),
					   fabs(gModelParams.position.y/2.0f),
					   0.0f);
  gAnimationParams.readyFlipPosition = Vector3(fabs(gModelParams.position.x/2.0f),
					   fabs(gModelParams.position.y/2.0f),
					       -0.1f);
  gAnimationParams.toCornerDecayRate = .90f;
  gAnimationParams.raiseDecayRate = .60f;
  gAnimationParams.shrinkDecayRate = .90f;

  resetAnimationRates();
}
void initQuads(){
  gQuads[FRONT_QUAD] = Quad(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, BOTTOM_LEFT );
  gQuads[FRONT_QUAD].shader = SHADER_BW;
  //gQuads[FRONT_QUAD].shader = SHADER_BORDER;
  gQuads[REAR_QUAD] =  Quad(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, TOP_RIGHT );
  gQuads[SELECTED_RESULT] = Quad(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, TOP_RIGHT );
  gQuads[COMMAND_OPTIONS] = Quad();
  gQuads[USER_APPROVAL] = Quad();
  gQuads[INITIAL_APPROVAL] = Quad();
  gQuads[CMD_OPTION] = Quad(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, TOP_LEFT);
  gQuads[RESULT_INFO] = Quad();
}

void initGlobalVars(){
  gMainWindow = NULL;
  gRunning=true;
  gScreenWidth = 500;
  gScreenHeight = 500;
  gCaptor = NULL;
  gSoundAmplitude = 1.2f;
  gPicsToSend = PICTURES_TO_SEND;
  gCameraNumber = 0;
  gTime = 0;
  
  initQuads();

  gToRender.push_back(&gQuads[FRONT_QUAD]);

  gModelParams.position = Vector3(-0.5f, -0.5f, 0.0f);

  gCameraParams.cameraPosition = Vector3(0.0, 0.0, 1.20);
  gCameraParams.cameraDirection = Vector3(0.0, 0.0, -1.0);
  gCameraParams.cameraReference = gCameraParams.cameraPosition + gCameraParams.cameraDirection;
  gCameraParams.cameraUp = Vector3(0.0, 1.0, 0.0);
  gCameraParams.FOV = 45.0f;

  initAnimationParams();

  gUserLikes = false;

  gTexIndex = 0;
}

void initRos(int argc, char** argv){
  ros::init(argc, argv, "guiNode");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  publishers.image = it.advertise("gui/image_raw/original", 1);
  publishers.feedback = nh.advertise< std_msgs::String >("gui/feedback_result", 1000);
  subscribers.trigger = nh.subscribe("trigger/output", 1000, callback_trigger);
  subscribers.bnetResult = nh.subscribe("bnet/final_result", 1000, callback_bnet_final_result);
}


bool init(int argc, char** argv){
  initGlobalVars();
  initCV();
  initResultsConfigurations();
  initSDL();
  initGL();
  //initAL();
  initShaders();
  std::cout<<"1 init done*******************"<<std::endl;
  initTextures();

  initFBO();
  initRos(argc, argv);
  resizeWindow(gScreenWidth, gScreenHeight);
  setState(WAITING_FOR_TRIGGER);
  return true;
}
int main(int argc, char ** argv) {
  init(argc, argv);

  mainLoop();
  cleanUp();
  return 0;
}
