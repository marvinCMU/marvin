#include "gui.hpp"
#include "SystemParams.h"


/**Main loop for the program
 */
void mainLoop(){
  gRunning = true;

  while( gRunning ){
    render();
    handleEvents();
    addUserInteraction();
    manageState();
    gTime += gDt;
    if(gRunRos) ros::spinOnce();
  }
}

/**Manages the state of the program
 */
void manageState(){
  switch( gState ){
    //waiting for trigger state
  case WAITING_FOR_TRIGGER: {
    Image img;

    // give delay to get frames from the video file
    static int delay_cnt = 0;
    if(gEnableDummyTestVideoIn)
    {
      if(delay_cnt++<5) {
        break;
      }
      delay_cnt = 0;
    }
    //get the frame from the camera
    Image tmpMat = getFrame();
    //resize the frame
    cv::resize( tmpMat, img, cv::Size( gImageWidth, gImageHeight), 0, 0, cv::INTER_CUBIC );
    //update the main texture with the new image- this results
    //in a video stream effect
    glBindTexture( GL_TEXTURE_2D, gMainTex );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0,
		     gImageWidth, gImageHeight, GL_BGR,
		     GL_UNSIGNED_BYTE, img.data );
    break;
  }
  case LISTENING:
    //set the bottom better to be the listening textured image
    gBottomBannerTex = gListenBannerTex;
    break;
  case THINKING:
    //set the bottom banner to be the thinking banner textured image
    gBottomBannerTex = gThinkingBannerTex;
    break;
  case SHOWING_RESULTS:{
    //set the results texture
    updateResultsTexture( );

    //listening for clicks
    if( gWrongClicked ){
      //user has given feedback that the results are wrong
      setState(WAITING_FOR_TRIGGER);
      //if ros is hooked up:
      if(gRunRos){
	//respond with a message to the bayes net 
	std_msgs::String msg;
	msg.data = string("::");
	feedbackPublisher.publish(msg);
      }
    }
    if( gRightClicked ){
      //user has given feedback 
      setState(WAITING_FOR_TRIGGER);
      if(gRunRos){
	//send feedback to the bayes net
	std_msgs::String msg;
	msg.data = string("retail:") + gActiveCommand  + (":") + gActiveObject;
	feedbackPublisher.publish(msg);
      }
    }
    if( gTakeDataClicked ){
      setState( TAKING_DATA );
    }
    break;
  }
  case TAKING_DATA:{
    Image img;
    //get the frame from the camera
    Image tmpMat = getFrame();
    //resize the frame
    cv::resize( tmpMat, img, cv::Size( gImageWidth, gImageHeight), 0, 0, cv::INTER_CUBIC );
    //update the main texture with the new image- this results
    //in a video stream effect
    glBindTexture( GL_TEXTURE_2D, gMainTex );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 
		     gImageWidth, gImageHeight, GL_BGR,
		     GL_UNSIGNED_BYTE, img.data );
    if( facePicsSent < facePicsToSend ){
      facePicsSent++;
      publishFaceImage();
    }
    else{
      setState( WAITING_FOR_TRIGGER );
    }
    
    break;
  }
  }
}

/** Method to call when changing the state of the program
 */
void setState( State state ){
  switch( state ){
  case WAITING_FOR_TRIGGER:{
    //set the bottom banner to the the waiting texture
    gBottomBannerTex = gWaitBannerTex;
    break;
  }
  case LISTENING:{
    if(gRunRos) {
      //publish an image
      publishImage();
    }
    //set the bottom banner
    gBottomBannerTex = gListenBannerTex;
    break;
  }
  case THINKING: {
    //set the bottom banner
    gBottomBannerTex = gThinkingBannerTex;
    break;
  }

  case SHOWING_RESULTS: {
    //set info for the showing results state
    gScrollPos = 0.0f;
    gWrongClicked = false;
    gRightClicked = false;
    gResetClicked = false;
    gTakeDataClicked = false;

    //reset the scroll bar parameters
    gScrollMy= gScrollOmy= 0;
    //set the results info texture
    setResultInfoTexture();
    //set the scroll bar texture based on the results
    setScrollbarTexture( );
    //set the bottom banner
    gBottomBannerTex = gResultBannerTex;

    break;
  }
  case TAKING_DATA:{
    facePicsSent = 0;
    //create a new banner here
    gBottomBannerTex = gTakingDataBannerTex;
    break;
  }
  default: break;
  }
  //set the state to the global state
  gState = state;
}

/** Takes care of all of the on screen drawing
 */
void render(){
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -.5, -.5, 0.0);
  
  glEnable( GL_TEXTURE_2D );
 
  //draw the main textured image
  glBindTexture( GL_TEXTURE_2D, gMainTex );
  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();
   

   glUseProgramObjectARB( gBlendShader );
   glUniform1f( glGetUniformLocationARB( gBlendShader, "blend"), gBannerBlend );
   glUniform1i( glGetUniformLocationARB( gBlendShader, "tex0"), 1);

   //draw the top banner
   glActiveTextureARB( GL_TEXTURE1_ARB );
   glBindTexture( GL_TEXTURE_2D, gTopBannerTex );
   glViewport( 0.0, gImageHeight-gTopBannerHeight, gImageWidth, gTopBannerHeight );
   glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

   glUniform1f( glGetUniformLocationARB( gBlendShader, "blend"), gBannerBlend );
   glUniform1i( glGetUniformLocationARB( gBlendShader, "tex0"), 1);

   glActiveTextureARB( GL_TEXTURE1_ARB );
   glBindTexture( GL_TEXTURE_2D, gBottomBannerTex );

   //draw the bottom banner
   glViewport( 0.0, 0.0, gImageWidth, gBottomBannerHeight );
   glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

   glUseProgramObjectARB( 0 );

   //draw the reset button
  drawResetStrip();

  //disable states
  glViewport( 0, 0, gImageWidth, gImageHeight );
  glDisable( GL_TEXTURE_2D );
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );

  SDL_GL_SwapBuffers();
}

/** Method to take care of all user interaction
 */
void addUserInteraction(){
  //if the scroll bar was clicked, update it
  if( gScrollClicked ){
    //update scroo velocity
    gScrollVelocity = -gScrollOmy + gScrollMy;
  }
  
  //if the scroll position is below the top, spring it back
  if( gScrollPos > gScrollTop ){
    float dx =  gScrollTop - gScrollPos;
    float fs = gScrollSpringK * dx;
    float fc = -gScrollDamping * gScrollVelocity;
    float ftot = fs + fc;
    float accel = ftot / gScrollMass;
    gScrollVelocity = gDt * accel + gScrollVelocity;
  }
  //if the scroll position is above the bottom, spring back
  else if( gScrollPos < gScrollBottom ){
    float dx =  -gScrollPos + gScrollBottom;
    float fs = gScrollSpringK * dx;
    float fc = -gScrollDamping * gScrollVelocity;
    float ftot = fs + fc;
    float accel = ftot / gScrollMass;
    gScrollVelocity = gDt * accel + gScrollVelocity;
  }

  //prevents floating point errors, rests the 
  //scroll position
  if( fabs(gScrollPos - gScrollTop) < EPSILON ) {
    gScrollPos = gScrollTop;
  }
  //prevents floating point errors, reset scroll to the bottom 
  if( fabs(gScrollPos - gScrollBottom) < EPSILON ){
    gScrollPos = gScrollBottom;
  }
  //integrate the position by the velocity
  gScrollPos += gScrollVelocity * gDt;
}

/** Processes all keypresses
 */
void processKeypress(const SDL_keysym * keysym ){
  switch(keysym->sym){
    //these keypresses over ride the system processes
  case SDLK_ESCAPE: gRunning = false; break;
  case SDLK_1: setState(WAITING_FOR_TRIGGER); break;
  case SDLK_2: setState(LISTENING); break;
  case SDLK_3: setState(THINKING); break;
  case SDLK_4:{
    string msg = "retail\nreview price info\nshampoo2 shampoo1 cream";
    //call the callback with the fake message
    bnetCallback(msg); 
    break;
  }
  case SDLK_UP: gScrollPos -=.08; break;
  case SDLK_DOWN: gScrollPos +=.08; break;
    
  default: break;
  }
}

/** SDL process the click up
 */
void processClickUp( const SDL_MouseButtonEvent *button){
 using std::string;
  float x = button->x;
  float y = button->y;
  
  //if we're in the showing result state
  if(gState == SHOWING_RESULTS ){
    //if we're in the bounds of the scrollbar
    if( x >= 0 && x <= gScrollWidth && y >= gTopBannerHeight &&
	y <= gImageHeight - gBottomBannerHeight ){
      //move the scroll bar
      gScrollOmy = gScrollMy = button->y;
      gScrollClicked = button->state == SDL_PRESSED;
      if( !gScrollClicked && fabs(gScrollVelocity) <= 10.0){
	setScrollbarTexture( );
      }
    }
    //set scroll clicked to false
    gScrollClicked = button->state == SDL_PRESSED;
  }
 
}

/** SDL process click down
 */
void processClickDown( const SDL_MouseButtonEvent* button ){
  using std::string;
  float x = button->x;
  float y = button->y;
  std::cout<<"button! ("<<x<<", "<<y<<")"<<std::endl;
  if( gState == SHOWING_RESULTS ){
    //if we're within the bounds of the scroll bar
    if( x >= 0 && x <= gScrollWidth && y >= gTopBannerHeight &&
	y <= gImageHeight - gBottomBannerHeight ){
      //move the scroll bar
      gScrollOmy = gScrollMy = button->y;
      gScrollClicked = button->state == SDL_PRESSED;
      if( !gScrollClicked && fabs(gScrollVelocity) <= 10.0){
	setScrollbarTexture( );
      }
    }

    //if wrog was clicked
    if( x >= gImageWidth - (gFeedbackTileWidth +gTilePadding) && 
	x <= gImageWidth - (gFeedbackTileWidth +gTilePadding) + gFeedbackTileWidth &&
	y >= gTopBannerHeight + gTilePadding &&
	y <= gTopBannerHeight + gTilePadding + gFeedbackTileHeight ){
      gWrongClicked = true;
    }

    //if right was clicked
    if( x >= gImageWidth - (gFeedbackTileWidth +gTilePadding)*2 &&
	x <= gFeedbackTileWidth + gImageWidth - (gFeedbackTileWidth +gTilePadding)*2 &&
	y >=  gTopBannerHeight + gTilePadding &&
	y <= gTopBannerHeight + gTilePadding + gFeedbackTileHeight ){
      gRightClicked = true;
      std::cout<<"Right clicked!"<<std::endl;
    }

    if( x >= gImageWidth - (gFeedbackTileWidth +gTilePadding) && 
	x <= gImageWidth - (gFeedbackTileWidth +gTilePadding) + gFeedbackTileWidth &&
	y >= gTopBannerHeight + gTilePadding +gFeedbackTileWidth + gTilePadding &&
	y <= gTopBannerHeight + gTilePadding + gFeedbackTileHeight + gFeedbackTileWidth + gTilePadding){
      gTakeDataClicked = true;
      std::cout<<"Take data clicked!"<<std::endl;
    }
    //change the positions of the command tiles depending on how many of them there are
    // i.e. (price, info, review, etc.)
    float x0[3];
    float pad = 10;
  switch( gCommands.size() ){
  case 1:
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth ;
    break;
  case 2:
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth - gCmdTileWidth/2.0 - gTilePadding;
    x0[1] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth + gCmdTileWidth/2.0 + gTilePadding;
    break;
  case 3: 
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth - gCmdTileWidth -gTilePadding;
    x0[1] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth;
    x0[2] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth + gCmdTileWidth + gTilePadding;
    break;
  }

  //set the y0 position of the command buttons ( price, review, info, etc. )

    float y0 = ((gCmdBarHeight - gCmdTileHeight)/2.0 + gTopBannerHeight);


    //check all of the buttons for a click
    for(int i = 0; i < gCommands.size(); i++ ){
      //if the button is clicked
      if( x >= x0[i] && x <= x0[i] + gCmdTileWidth && y >= y0 && y <= y0 + gCmdTileHeight ){
	//set the button cliccked to the active button
	gActiveCommand = gCommands[i];
	//update the result info texture based on the button clicked 
	setResultInfoTexture();
      }
    }
  }

  //check if reset was clicked
  int x0 = gImageWidth - (gFeedbackTileWidth +gTilePadding)*2;
  int x1 = gImageWidth - (gFeedbackTileWidth +gTilePadding)*2 + gResetWidth;
  int y0 = gImageHeight - (gTilePadding + gResetHeight);
  int y1 = gImageHeight  - gTilePadding;
    if( x >= x0 &&
	x <= x1 &&
	y >= y0 &&
	y <= y1){ 
      //reset was clicked
      gResetClicked = true;
      std_msgs::String msg;
      msg.data = string("");
      feedbackPublisher.publish(msg);
      setState( WAITING_FOR_TRIGGER );
    }

}

/** process the mouse motion if its clicked 
 */
void processMouseMotion( SDL_MouseMotionEvent* motion ){
  gScrollMy = motion->y;
}


/** handles the SDL events 
 */
void handleEvents(){
  SDL_Event event;
  while( SDL_PollEvent(&event) ) {
    switch( event.type ){
    case SDL_KEYDOWN: processKeypress( &event.key.keysym ); break;
    case SDL_MOUSEMOTION: processMouseMotion( &event.motion ); break;
    case SDL_MOUSEBUTTONUP: processClickUp( &event.button ); break;
    case SDL_MOUSEBUTTONDOWN: processClickDown( &event.button); break;
    case SDL_QUIT: gRunning = false; break;
    }
  }
}

/** updates the results texture based on the current state
 */
void updateResultsTexture( ){

  //render into the framebuffer
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gFboId );
  //render into the main texture
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			     GL_TEXTURE_2D, gMainTex, 0 );
  //clear the main iteixture
  glClear( GL_COLOR_BUFFER_BIT );

  drawBackgroundTexture();
  drawResultInfoTexture();
  drawScrollbar();
  drawCommandTiles();
  drawWrongStrip();
  drawRightStrip();
  //drawTakingDataStrip();

  //unbind the frame buffer and reset the states
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  glViewport( 0.0, 0.0, gImageWidth, gImageHeight );
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );
}

/** Draws the background texture
    For now there is no background texture drawn
 */
void drawBackgroundTexture(){
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -0.5, -0.5, 0.0 );

  glViewport(0, 0, gImageWidth, gImageHeight );
  glEnable( GL_TEXTURE_2D);
  //glBindTexture( GL_TEXTURE_2D, gTartanTex );
  //glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  float tx1 = (float)gImageWidth / (float)gTartanWidth;
  float ty1 = (float)gImageHeight / (float)gTartanHeight;
  
  glColor3f(1.0, 1.0, 1.0 );
  glBegin(GL_QUADS);
  glTexCoord2f( 0.0, 0.0 ); glVertex3f( 0.0, 0.0, 0.0 ); 
  glTexCoord2f( tx1, 0.0 ); glVertex3f( 1.0, 0.0, 0.0 ); 
  glTexCoord2f( tx1, 1.0 ); glVertex3f( 1.0, 1.0, 0.0 ); 
  glTexCoord2f( 0.0, 1.0 ); glVertex3f( 0.0, 1.0, 0.0 ); 
  glEnd();

   glDisable( GL_TEXTURE_2D );
   glActiveTextureARB( GL_TEXTURE0_ARB );
   glBindTexture( GL_TEXTURE_2D, 0 );
   glViewport(0, 0, gImageWidth, gImageHeight );


}

/** Draws the reset button
 */
void drawResetStrip(){

  glViewport( gImageWidth - (gFeedbackTileWidth +gTilePadding)*2, gTilePadding,
	      gResetWidth, gResetHeight );

  glActiveTextureARB( GL_TEXTURE0_ARB);
  glBindTexture( GL_TEXTURE_2D, 0 );
  
  glEnable(GL_TEXTURE_2D);
  glBindTexture( GL_TEXTURE_2D, gResetStripTex );


  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

  glDisable(GL_TEXTURE_2D);
  glActiveTextureARB( GL_TEXTURE0_ARB);
  glBindTexture( GL_TEXTURE_2D, 0 );
  glUseProgramObjectARB( 0 );
}

/** Draws the (correct) user feedback response
 */
void drawRightStrip(){
  glViewport( gImageWidth - (gFeedbackTileWidth +gTilePadding)*2, gTopBannerHeight + gTilePadding,
	      gFeedbackTileWidth, gFeedbackTileHeight );
  
  glUseProgramObjectARB( gMaskShader );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "tex"), 1 );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex" ), 2 );


  glActiveTextureARB( GL_TEXTURE2_ARB);
  glBindTexture( GL_TEXTURE_2D, gFeedbackTileMaskTex );
  glActiveTextureARB( GL_TEXTURE1_ARB);
  glBindTexture( GL_TEXTURE_2D, gRightTileTex );
  
  //std::cout<<"x0, w "<<x0<<", "<<w<<std::endl;
  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

  glActiveTextureARB( GL_TEXTURE0_ARB);
  glBindTexture( GL_TEXTURE_2D, 0 );
  glUseProgramObjectARB( 0 );
}

void drawTakingDataStrip(){
  glViewport( gImageWidth - (gFeedbackTileWidth +gTilePadding),
	      gTopBannerHeight + gTilePadding + gFeedbackTileWidth + gTilePadding,
	      gFeedbackTileWidth, gFeedbackTileHeight );
  
  glUseProgramObjectARB( gMaskShader );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "tex"), 1 );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex" ), 2 );

  glActiveTextureARB( GL_TEXTURE2_ARB);
  glBindTexture( GL_TEXTURE_2D, gFeedbackTileMaskTex );
  glActiveTextureARB( GL_TEXTURE1_ARB );
  glBindTexture( GL_TEXTURE_2D, gWrongTileTex );
  
  
  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

   glUseProgramObjectARB( 0 );


}

/** Draws the (incorrect) user feedback response
 */
void drawWrongStrip(){
  glViewport( gImageWidth - (gFeedbackTileWidth +gTilePadding), gTopBannerHeight + gTilePadding,
	      gFeedbackTileWidth, gFeedbackTileHeight );
  
  glUseProgramObjectARB( gMaskShader );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "tex"), 1 );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex" ), 2 );

  glActiveTextureARB( GL_TEXTURE2_ARB);
  glBindTexture( GL_TEXTURE_2D, gFeedbackTileMaskTex );
  glActiveTextureARB( GL_TEXTURE1_ARB );
  glBindTexture( GL_TEXTURE_2D, gWrongTileTex );
  
  
  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

   glUseProgramObjectARB( 0 );
}

/** Draws the command tiles (price, review info)
 */
void drawCommandTiles(){
  GLuint texId;
  glGenTextures( 1, &texId );
  glBindTexture( GL_TEXTURE_2D, texId );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  //  glMatrixMode( GL_MODELVIEW );
  //glLoadIdentity();
  //glTranslatef( -0.5, -0.5, 0.0 );

  glEnable( GL_TEXTURE_2D );

  //depending on the number of tiles
  //adjust the position
  float x0[3];
  float y0[3];
  bool shadowOn[3];
  float pad = 10;
  switch( gCommands.size() ){
  case 1:
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth ;
    break;
  case 2:
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth - gCmdTileWidth/2.0 - gTilePadding;
    x0[1] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth + gCmdTileWidth/2.0 + gTilePadding;
    break;
  case 3: 
    x0[0] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth - gCmdTileWidth -gTilePadding;
    x0[1] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth;
    x0[2] = (gCmdBarWidth - gCmdTileWidth)/2.0 + gScrollWidth + gCmdTileWidth + gTilePadding;
    break;
  }


  for(int i = 0; i < gCommands.size(); i++){
    y0[i] = (gCmdBarHeight - gCmdTileHeight)/2.0 + gTopBannerHeight;
    shadowOn[i] = false;
    if( gActiveCommand == gCommands[i]){
      y0[i] -= 10;
      x0[i] -= 10;
      shadowOn[i] = true;
    }
  }
  
  int idx = 0;
  string filename;
  //draw the command tiles
  for( StringList::iterator iter = gCommands.begin(); iter != gCommands.end(); iter++){
    filename = (*iter) + string("_tile.jpg" );
    Image tile = loadImage( filename.c_str(), 0, 0 );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef( -0.5, -0.5, 0.0 );

    //draw the tiles with a shadow
    if( shadowOn[idx] ){
      glUseProgramObjectARB( gShadowShader );

      glViewport( x0[idx], y0[idx], gTileShadowWidth, gTileShadowHeight );
      glActiveTextureARB( GL_TEXTURE1_ARB );
      glBindTexture( GL_TEXTURE_2D, gTileShadowTex );
    
      glUniform1i( glGetUniformLocationARB( gShadowShader, "tex"), 1 );
    
      glBegin(GL_QUADS);
      glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
      glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
      glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
      glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
      glEnd();

      glActiveTextureARB( GL_TEXTURE0_ARB );
      glBindTexture( GL_TEXTURE_2D, 0 );
      glUseProgramObjectARB( gMaskShader );
    }else{
      glUseProgramObjectARB( gGreyMaskShader );
    }
      glViewport( x0[idx], y0[idx], gCmdTileWidth, gCmdTileHeight );
    
      glUniform1i( glGetUniformLocationARB( gGreyMaskShader, "tex" ), 2);
      glUniform1i( glGetUniformLocationARB( gGreyMaskShader, "maskTex"), 1);

      glActiveTextureARB( GL_TEXTURE1_ARB );
      glBindTexture( GL_TEXTURE_2D, gTileMaskTex );
      glBegin(GL_QUADS);
      glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
      glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
      glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
      glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
      glEnd();
    
      glActiveTextureARB( GL_TEXTURE2_ARB );
      glBindTexture( GL_TEXTURE_2D, texId );
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		    tile.cols, tile.rows,
		    0, GL_BGR, GL_UNSIGNED_BYTE, tile.data );

      glBegin(GL_QUADS);
      glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
      glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
      glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
      glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
      glEnd();

    idx++;
  }

  glUseProgramObjectARB( 0 );
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );

}

/** Method to draw the scroll bar with the results on it
 */
void drawScrollbar(){
 float height = (float)((float)gScrollHeight*gNumScrollTiles)/gScrollbarHeight;
 //float y0 = -(float)(height / gNumScrollTiles) * 2.0;
 float y0 = 0.0f;
  
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -.5, -.5, 0.0);
  
  //glUseProgramObjectARB( gMaskShader );
  
  //glUniform1i( glGetUniformLocationARB( gMaskShader, "tex" ), 1 );
  //glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex"), 2 );

  //glActiveTextureARB( GL_TEXTURE2_ARB );
  //glBindTexture( GL_TEXTURE_2D, gScrollMaskTex );
  glEnable(GL_TEXTURE_2D);
  //glActiveTextureARB( GL_TEXTURE1_ARB );
  glBindTexture( GL_TEXTURE_2D, gScrollTex );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glViewport(0.0, gTopBannerHeight, gScrollWidth, gScrollbarHeight );
  y0+=gScrollPos;

  glBegin( GL_QUADS );
  glVertex3f( 0.0, y0, 0.0 ); glTexCoord2f(1.0, 1.0 );
  glVertex3f( 1.0, y0, 0.0 ); glTexCoord2f(1.0, 0.0f);
  glVertex3f( 1.0, y0+height, 0.0 ); glTexCoord2f(0.0f, 0.0f);
  glVertex3f( 0.0, y0+height, 0.0 ); glTexCoord2f( 0.0f, 1.0);
  glEnd();
 
  //glUseProgramObjectARB( 0 );
  glDisable( GL_TEXTURE_2D);
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


/** updates the results the screen is displaying
 */
void drawResultInfoTexture(){
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef(-.5, -.5, 0.0 );

  glViewport( gScrollWidth, (gImageHeight - (gTopBannerHeight + gBottomBannerHeight) - gResultInfoHeight),
	      gResultInfoWidth, gResultInfoHeight );

  glEnable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, gResultInfoTex );
  
  glBegin(GL_QUADS);
    glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
   glEnd();

   glActiveTextureARB( GL_TEXTURE0_ARB );
   glBindTexture( GL_TEXTURE_2D, 0 );
   glDisable( GL_TEXTURE_2D );
}

/** updates the results info texture based on the selection the
    user made on the scroll bar
 */
void setResultInfoTexture() {
  //get the filename of the active object in the active command folder
  string filename = (string("info_images/") + string( gActiveCommand ) +
		     string("/") + string(gActiveObject) + string(".jpg") );

  //load the image
  Image infoPic = loadImage( filename.c_str(), 0, 0 );
  GLuint texId;

  //draw the image
  glGenTextures( 1, &texId );
  glBindTexture( GL_TEXTURE_2D, texId );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		infoPic.cols, infoPic.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, infoPic.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gFboId );
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
			     GL_TEXTURE_2D, gResultInfoTex, 0 );
  glClear( GL_COLOR_BUFFER_BIT );

  glViewport( 0, 0, gResultInfoWidth, gResultInfoHeight );
  
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -.5, -.5, 0.0);


  //draw the mask
  glUseProgramObjectARB( gMaskShader );

  glUniform1i( glGetUniformLocationARB( gMaskShader, "tex" ), 1 );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex"), 2 );

  glActiveTextureARB( GL_TEXTURE1_ARB );
  glBindTexture( GL_TEXTURE_2D, texId );
  glActiveTextureARB( GL_TEXTURE2_ARB );
  glBindTexture( GL_TEXTURE_2D, gInfoSurfaceMaskTex );

  glBegin(GL_QUADS);
  glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
  glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
  glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
  glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
  glEnd();

  glUseProgramObjectARB( 0 );
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  glActiveTextureARB( GL_TEXTURE0_ARB);
  glBindTexture( GL_TEXTURE_2D, 0 );

  glViewport( 0.0, 0.0, gImageWidth, gImageHeight );
}


/** draws the results on the 
    scroll bar texture
 */
void setScrollbarTexture( ){

  glBindTexture( GL_TEXTURE_2D, gScrollTex );
  glTexSubImage2D( GL_TEXTURE_2D, 0,
		   0, 0,
		   gScrollBarImage.cols, gScrollBarImage.rows,
		   GL_BGR, GL_UNSIGNED_BYTE, gScrollBarImage.data );
  
  GLuint texId;
  GLuint maskTexId;
  glGenTextures( 1, &texId );
  glBindTexture( GL_TEXTURE_2D, texId );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );

  glGenTextures( 1, &maskTexId );
  glBindTexture( GL_TEXTURE_2D, maskTexId );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );


  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, gFboId );
  glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, 
			    GL_TEXTURE_2D, gScrollTex, 0 );
  

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();
  glTranslatef( -.5, -.5, 0.0);

  //counts up from the bottom
  int tileIndex = gNumScrollTiles - 1;
  int selectedIdx = ((int)(gScrollOmy-gTopBannerHeight + -(gScrollPos * gScrollbarHeight))
		     / gScrollHeight);

  if( selectedIdx >= gObjects.size() ){
    selectedIdx = gObjects.size()-1;
  }

  int selectedTileIndex = gNumScrollTiles - (selectedIdx + 1);
  //3 is the amount of padding from the top
  float x0, y0, w, h;
  float totalHeight = 0.0f;

  //std::cout<<"SelectedTileIndex "<<selectedIdx<<std::endl;
  glEnable(GL_TEXTURE_2D);
  glViewport(0, gScrollHeight * selectedTileIndex, gScrollWidth, gScrollHeight );
  glBindTexture( GL_TEXTURE_2D, gSelectedTileTex );
  glBegin( GL_QUADS );
  glVertex3f( 0.0, 0.0, 0.0 ); glTexCoord2f(1.0, 1.0 );
  glVertex3f( 1.0, 0.0, 0.0 ); glTexCoord2f(1.0, 0.0f);
  glVertex3f( 1.0, 1.0, 0.0 ); glTexCoord2f(0.0f, 0.0f);
  glVertex3f( 0.0, 1.0, 0.0 ); glTexCoord2f( 0.0f, 1.0);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glUseProgramObjectARB( gMaskShader );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "tex"), 1 );
  glUniform1i( glGetUniformLocationARB( gMaskShader, "maskTex"), 2 );

  string filename, maskFilename;
  //for each object load the image and the mask
  for( StringList::iterator iter = gObjects.begin(); iter != gObjects.end(); iter++){
    filename  = string("objects/") + (*iter) + string(".jpg");
    maskFilename = string("objects/") + *(iter) + string("_mask.jpg");

    Image object = loadImage( filename.c_str(),  gScrollWidth-30, gScrollHeight-30);
    Image mask =  loadImage( maskFilename.c_str(), gScrollWidth-30, gScrollHeight-30 );
    //30 is a fudge factor

    glActiveTextureARB( GL_TEXTURE1_ARB );
    glBindTexture( GL_TEXTURE_2D, texId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 
		  object.cols, object.rows,
		  0, GL_BGR, GL_UNSIGNED_BYTE, object.data );

    glActiveTextureARB( GL_TEXTURE2_ARB );
    glBindTexture( GL_TEXTURE_2D, maskTexId );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		  mask.cols, mask.rows,
		  0, GL_BGR, GL_UNSIGNED_BYTE, mask.data );

    glViewport(0, gScrollHeight*tileIndex, gScrollWidth, gScrollHeight );
    
    x0 = ((gScrollWidth - object.cols) / 2.0) / gScrollWidth;
    y0 = ((gScrollHeight - object.rows) / 2.0) / gScrollHeight;
    w = (float)object.cols / gScrollWidth;
    h = (float)object.rows / gScrollHeight;
    
    //draw the objects
    glBegin(GL_QUADS);
    glVertex3f( x0, y0, 0.0 ); glTexCoord2f(1.0, 1.0 );
    glVertex3f( x0+w, y0, 0.0 ); glTexCoord2f(1.0, 0.0f);
    glVertex3f( x0+w, y0+h, 0.0 ); glTexCoord2f(0.0f, 0.0f);
    glVertex3f( x0, y0+h, 0.0 ); glTexCoord2f( 0.0f, 1.0f);
    glEnd();
    tileIndex--;
    totalHeight+=gScrollHeight;
  }

  if( totalHeight > gScrollbarHeight ){
    gScrollBottom = -fabs(gScrollbarHeight - totalHeight)/ gScrollbarHeight;
  }

  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );
  glUseProgramObjectARB( 0 );
  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  glViewport( 0.0, 0.0, gImageWidth, gImageHeight );

  gActiveObject = gObjects[selectedIdx];
  setResultInfoTexture();


}

/** Called when window is resized by SDL
 */
void resizeWindow( int width, int height ){
  Vector3 reference = camera.position + camera.direction;
  glViewport( 0, 0, width, height );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  
  gluPerspective( 45.0f, 1.0f, 0.1f, 100.0f );
  gluLookAt( camera.position.x, camera.position.y, camera.position.z,
	     reference.x, reference.y, reference.z,
	     camera.up.x, camera.up.y, camera.up.z );
  
}

/** call back for the trigger to start the system
    if the gui hears the trigger, then it changes the state to listening
 */
void callbackTrigger(const std_msgs::String::ConstPtr& msg){
  ROS_INFO("%s:%s:%s",REPORT_IN,REPORT_GUI,REPORT_TRIGGER);
  ROS_INFO("Gui trigger heard: %s", msg->data.c_str() );
  if(msg->data.find("marvin") != std::string::npos )
    setState(LISTENING);
}

/** callback for the bayes net results 
    takes in a string that was the message from the bayes net and 
    updates the results accordingly
 */
void bnetCallback(std::string msg){
  //std::cout<<"w, h "<<gResultInfoWidth << ", "<<gResultInfoHeight<<std::endl;
  //if(gState == LISTENING){
  if(gState == THINKING){
    using namespace std; 
    istringstream iss(msg);
    string token;
    deque<string> lines;
    char* temp=NULL;
    char line[255];

    while( getline( iss, token ) ){
      lines.push_back(token);
    }

    lines.pop_front(); //retail

    strcpy(line, lines.front().c_str());
    temp = strtok( line, " "); lines.pop_front(); //commands
    gCommands.clear();
    while( temp!= NULL) {
      gCommands.push_back( string(temp ) );
      temp = strtok(NULL, " ");
    }

    strcpy(line, lines.front().c_str()); //objects
    temp = strtok( line, " "); lines.pop_front();
    gObjects.clear();
    while( temp!=NULL){
      if(gObjects.size() < gNumScrollTiles){
	if( strcmp(temp,"outlet1") != 0 ){
	  gObjects.push_back( string(temp) );
	}
      }
      temp = strtok(NULL, " ");
    }

    gActiveCommand = gCommands[0];
    gActiveObject = gObjects[0];

    setState(SHOWING_RESULTS);
  }
}

//passes the final results message to the bayes net callback
void callbackBnetFinalResult( const std_msgs::String::ConstPtr& msg ){
  bnetCallback( msg->data );
}

//set state to thinking
void speechCommandCallback( const std_msgs::String::ConstPtr& msg ){
  setState(THINKING);
}

void publishFaceImage(){
  std::cout<<"faces sent: "<<facePicsSent<<std::endl;
  /**EDIT HERE IF DESIRED**/
}

/** publishes the image to be consumed by the
    other nodes in the system
 */
void publishImage(){
  //ROS_INFO("%s:%s:%s",REPORT_OUT,REPORT_GUI,REPORT_IMAGE);
  if(gRunRos)
  {
    // dummy test mode
    // gEnableDummyTest : read a image file from disk and pass the file to object recognizer
    // gEnableDummyTestVideoIn : use a video file instead of camera
    if (gEnableDummyTest && !gEnableDummyTestVideoIn || gCloudService)
    {
      std_msgs::String msg;
      msg.data = string("dummy");
      dummyTestPublisher.publish(msg);
    }
    else
    {
      if (gEnableLocalization)
      {
        cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_WIDTH, 640);
        cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_HEIGHT, 480);
        Image hiRes(cvQueryFrame(gCaptor));

        //Publish lowres first:
        cv::Mat smallImage;
        cv::resize(hiRes,smallImage,cv::Size(640,480));
        cv_bridge::CvImage imageMsg( std_msgs::Header(),
                sensor_msgs::image_encodings::BGR8,
             smallImage );
        imagePublisher.publish( imageMsg.toImageMsg () );
        //next publish hi-res
        cv_bridge::CvImage hiResImgMsg( std_msgs::Header(),
                sensor_msgs::image_encodings::BGR8, hiRes);
        imagePublisherHiRes.publish( hiResImgMsg.toImageMsg () );
        cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_WIDTH, 640);
        cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_HEIGHT, 480);
      }
      else
      {
        cv_bridge::CvImage imageMsg( std_msgs::Header(),
                sensor_msgs::image_encodings::BGR8,
                getFrame() );
        imagePublisher.publish( imageMsg.toImageMsg () );
      }
    }
  }
  ROS_INFO("GUI SENT IMAGE");
}

/** Queries the frame from the camera
 */
Image getFrame(int width, int height){
  Image frame(cvQueryFrame(gCaptor));
  return frame;
}

/** loads an image from file
 */
Image loadImage(const char* pictureName, int width, int height, bool maintainAspect ){
  Image frame, temp;
  string path = string(IMAGE_DIR) + string(pictureName);
  ifstream file(path.c_str());
  if( !file.good()){
    string filename = string(IMAGE_DIR) + string("default.jpg");
    ifstream file2(filename.c_str());
    if( !file2.good() ){
      std::cout<<"ERROR!, default image not in location "<<filename.c_str()<<std::endl;
      assert( file2.good() );
    }
    std::cout<<"Warning! Couldn't load image "<<path<<std::endl;
    return cv::imread(filename.c_str());
  }
  frame = cv::imread(path);
  if( width == 0 && height == 0 )
    return frame;
  float ratio = (float)frame.rows / frame.cols;
  
  int newWidth = width, newHeight = height;
  if( maintainAspect ){
    if(frame.rows > frame.cols ){
      newHeight = height;
      newWidth = newHeight / ratio;
    }else{
      newWidth = width;
      newHeight = newWidth * ratio;
    }
  }

  cv::resize( frame, temp, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_AREA );
  frame = temp;
  return frame;
}

/** parses a shader file
 */
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
     //std::cout<<"Compiled shaders successfully" <<std::endl;
   }

   glAttachObjectARB( program, shader );
   free( buffer );
   return true;
 }

/** creates a shader file
 */
 bool create_shader( GLhandleARB program, const char * vert_file, const char* frag_file ){
   bool rv = true;
   //std::cout<< "Loading vertex shader "<< vert_file
   //<<"\nLoading fragment shader "<< frag_file << std::endl;
   rv = rv && load_shader( vert_file, GL_VERTEX_SHADER, program );
   rv = rv && load_shader( frag_file, GL_FRAGMENT_SHADER, program );

   if( !rv )
     return false;

   glLinkProgram( program );
   GLint result;
   glGetProgramiv( program, GL_LINK_STATUS, &result );
   if( result == GL_TRUE ){
     //std::cout << "Succesfully linked shader"<<std::endl;
     return true;
   }else{
     std::cout << "FAILED to link shader" <<std::endl;
     return false;
   }
 }


/** deletes sdl window
 */
void cleanUp(){
  SDL_FreeSurface(gMainWindow);
  SDL_Quit();
}

/** Initializes all shaders
 */
void initShader(){
  gBlendShader = glCreateProgramObjectARB();
  gMaskShader = glCreateProgramObjectARB();
  gMaskAndBlendShader = glCreateProgramObjectARB();
  gShadowShader = glCreateProgramObjectARB();
  gGreyMaskShader = glCreateProgramObjectARB();

  string vertexFilename = string(SHADER_DIR) + string("gui4_shaders/vertex_shader.glsl");
  string fragmentFilename = string(SHADER_DIR) + string("gui4_shaders/blend_frag_shader.glsl");
  create_shader( gBlendShader, vertexFilename.c_str(), fragmentFilename.c_str() ); 

  vertexFilename = string(SHADER_DIR) + string("gui4_shaders/vertex_shader.glsl");
  fragmentFilename = string(SHADER_DIR) + string("gui4_shaders/mask_frag_shader.glsl");
  create_shader( gMaskShader, vertexFilename.c_str(), fragmentFilename.c_str() );

  vertexFilename = string(SHADER_DIR) + string("gui4_shaders/vertex_shader.glsl");
  fragmentFilename = string(SHADER_DIR) + string("gui4_shaders/blend_and_mask_frag.glsl");
  create_shader( gMaskAndBlendShader, vertexFilename.c_str(), fragmentFilename.c_str() );

  vertexFilename = string(SHADER_DIR) + string("gui4_shaders/vertex_shader.glsl");
  fragmentFilename = string(SHADER_DIR) + string("gui4_shaders/shadow_frag.glsl");
  create_shader( gShadowShader, vertexFilename.c_str(), fragmentFilename.c_str() );

  vertexFilename = string(SHADER_DIR) + string("gui4_shaders/vertex_shader.glsl");
  fragmentFilename = string(SHADER_DIR) + string("gui4_shaders/grey_mask_frag.glsl");
  create_shader( gGreyMaskShader, vertexFilename.c_str(), fragmentFilename.c_str() );
}

/** Initializes all textures
 */
void initTexture(){
  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

  glGenTextures(1, &gMainTex);
  glBindTexture( GL_TEXTURE_2D, gMainTex );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		gImageWidth, gImageHeight,
		0, GL_BGR, GL_UNSIGNED_BYTE, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  
  glGenTextures( 1, &gTopBannerTex );
  glBindTexture( GL_TEXTURE_2D, gTopBannerTex );
  Image topBanner = loadImage("topBanner.jpg", 0, 0);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		topBanner.cols, topBanner.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, topBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gScrollTex );
  glBindTexture( GL_TEXTURE_2D, gScrollTex );
  gScrollBarImage = loadImage("scroll_bar2.jpg", gScrollWidth, gScrollHeight * gNumScrollTiles );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		gScrollBarImage.cols, gScrollBarImage.rows,
		0,  GL_BGR, GL_UNSIGNED_BYTE, gScrollBarImage.data  );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gSelectedTileTex );
  glBindTexture( GL_TEXTURE_2D, gSelectedTileTex );
  Image selectedTile = loadImage( "selected_scroll_tile.jpg", gScrollWidth, gScrollHeight );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		selectedTile.cols, selectedTile.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, selectedTile.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gCmdBarTex );
  glBindTexture( GL_TEXTURE_2D, gCmdBarTex );
  Image cmdButtonImg = loadImage( "command_bar.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		cmdButtonImg.cols, cmdButtonImg.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, cmdButtonImg.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gTileMaskTex );
  glBindTexture( GL_TEXTURE_2D, gTileMaskTex );
  Image tileMask = loadImage( "tile_mask.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		tileMask.cols, tileMask.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, tileMask.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gWrongTileTex );
  glBindTexture( GL_TEXTURE_2D, gWrongTileTex );
  Image wrongStrip = loadImage("wrong_tile.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		wrongStrip.cols, wrongStrip.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, wrongStrip.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gRightTileTex );
  glBindTexture( GL_TEXTURE_2D, gRightTileTex );
  Image rightStrip = loadImage( "right_tile.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		rightStrip.cols, rightStrip.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, rightStrip.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gResetStripTex );
  glBindTexture( GL_TEXTURE_2D, gResetStripTex );
  Image resetStrip = loadImage( "reset_button.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		resetStrip.cols, resetStrip.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, resetStrip.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gFeedbackTileMaskTex );
  glBindTexture( GL_TEXTURE_2D, gFeedbackTileMaskTex );
  Image stripMask = loadImage( "feedback_tile_mask.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		stripMask.cols, stripMask.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, stripMask.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gScrollMaskTex );
  glBindTexture( GL_TEXTURE_2D, gScrollMaskTex );
  Image scrollMask = loadImage("scroll_bar_mask.jpg", 0, 0);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		scrollMask.cols, scrollMask.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, scrollMask.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gResultInfoTex );
  glBindTexture( GL_TEXTURE_2D, gResultInfoTex );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
	       gResultInfoWidth, gResultInfoHeight,
	       0, GL_BGR, GL_UNSIGNED_BYTE, NULL );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gTartanTex );
  glBindTexture( GL_TEXTURE_2D, gTartanTex );
  Image tartan = loadImage( "tartan.jpg", 0, 0);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		tartan.rows, tartan.cols,
		0, GL_BGR, GL_UNSIGNED_BYTE, tartan.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gTileShadowTex );
  glBindTexture( GL_TEXTURE_2D, gTileShadowTex );
  Image shadow = loadImage( "tile_shadow.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		shadow.cols, shadow.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, shadow.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gInfoSurfaceMaskTex );
  glBindTexture( GL_TEXTURE_2D, gInfoSurfaceMaskTex );
  cv::Mat infoMask = loadImage( "info_tile_mask.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		infoMask.cols, infoMask.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, infoMask.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gWaitBannerTex );
  glBindTexture( GL_TEXTURE_2D, gWaitBannerTex );
  cv::Mat waitBanner =  loadImage("bottom_banner_await_request.jpg", 0, 0);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		waitBanner.cols, waitBanner.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, waitBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  
  glGenTextures( 1, &gListenBannerTex );
  glBindTexture( GL_TEXTURE_2D, gListenBannerTex );
  cv::Mat listenBanner = loadImage( "bottom_banner_how_service2.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		listenBanner.cols, listenBanner.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, listenBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gResultBannerTex );
  glBindTexture( GL_TEXTURE_2D, gResultBannerTex );
  cv::Mat resultBanner =  loadImage("bottom_banner_found.jpg", 0, 0);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		resultBanner.cols, resultBanner.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, resultBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gThinkingBannerTex );
  glBindTexture( GL_TEXTURE_2D, gThinkingBannerTex );
  Image thinkBanner = loadImage( "bottom_banner_searching.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 
		thinkBanner.cols, thinkBanner.rows, 
		0, GL_BGR, GL_UNSIGNED_BYTE, thinkBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gTakingDataBannerTex );
  glBindTexture( GL_TEXTURE_2D, gTakingDataBannerTex );
  Image takingDataBanner = loadImage( "bottom_banner_taking_data.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		takingDataBanner.cols, takingDataBanner.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, takingDataBanner.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  glGenTextures( 1, &gTakeDataTileTex );
  glBindTexture( GL_TEXTURE_2D, gTakeDataTileTex );
  Image takeDataTile = loadImage("take_data_tile.jpg", 0, 0 );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB,
		takeDataTile.cols, takeDataTile.rows,
		0, GL_BGR, GL_UNSIGNED_BYTE, takeDataTile.data );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

  
  glActiveTextureARB( GL_TEXTURE0_ARB );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


void initCV(){

  // dummy test mode: read a video file from disk
  if(gEnableDummyTestVideoIn)
  {
    string filename;
    filename = string(DUMMY_DIR) + string("video/") + string("test_video.avi");
    gCaptor = cvCaptureFromFile(filename.c_str());
  }
  else
    gCaptor = cvCaptureFromCAM( 0 );
  assert( gCaptor );
  //cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_WIDTH, 1280);
  //cvSetCaptureProperty(gCaptor, CV_CAP_PROP_FRAME_HEIGHT, 960);

  cv::namedWindow("test");
}

void initGL(){
  glClearColor(1.0, 1.0, 1.0, 1.0 );
  glEnable(GL_BLEND);
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  GLenum error = glewInit();
  assert( error == GLEW_OK );
}

bool initSDL(){
  int videoFlags;
  if( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
    std::cout<<"Error starting SDL\n "<<SDL_GetError() << std::endl;
  }
  videoFlags = SDL_OPENGL;
  videoFlags |= SDL_GL_DOUBLEBUFFER;
  videoFlags |= SDL_HWSURFACE;
  videoFlags |= SDL_HWACCEL;
  videoFlags |= SDL_NOFRAME;
  //videoFlags |= SDL_FULLSCREEN;
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  putenv((char*)"SDL_VIDEO_CENTERED=1");
  gMainWindow = SDL_SetVideoMode( gImageWidth, gImageHeight,
				  32, videoFlags );
  if( !gMainWindow ){
    std::cout<<"Video mode failed: "<<SDL_GetError()<<std::endl;
    assert(gMainWindow);
  }
  return true;
}

void initRos(int argc, char** argv){
  ros::init(argc, argv, "guiNode");
  ros::NodeHandle nh;
  image_transport::ImageTransport it(nh);
  imagePublisher = it.advertise("gui/image_raw/original", 1);
  imagePublisherHiRes = it.advertise("gui/image_raw/hi_res", 1);
  feedbackPublisher = nh.advertise< std_msgs::String >("gui/feedback_result", 1000);
  dummyTestPublisher = nh.advertise< std_msgs::String >("dummyTest/sendFake", 1000);
  triggerSubscriber = nh.subscribe("trigger/output", 1000, callbackTrigger);
  bnetResultSubscriber = nh.subscribe("bnet/final_result", 1000, callbackBnetFinalResult);
  commandSubscriber = nh.subscribe("retailCommander/output", 1000, speechCommandCallback);
}


bool init(int argc, char** argv){
  if(gRunRos ) initRos( argc, argv );

  if (ros::param::has("/locationActive"))
    ros::param::get("/locationActive", gEnableLocalization);
  else
    ROS_WARN("Could not find parameter %s.", "/locationActive");

  // dummy test mode: read a test image from disk
  if (ros::param::has("/dummyTestActive"))
    ros::param::get("/dummyTestActive", gEnableDummyTest);
  else
    ROS_WARN("Could not find parameter %s.", "/dummyTestActive");

  // dummy test mode: read a video file from disk
  if (ros::param::has("/dummyTestVideoInEnable"))
    ros::param::get("/dummyTestVideoInEnable", gEnableDummyTestVideoIn);
  else
    ROS_WARN("Could not find parameter %s.", "/dummyTestVideoInEnable");

  // cloud service mode: read a imgae file from cloud server
  if (ros::param::has("/CloudServiceActive"))
      ros::param::get("/CloudServiceActive", gCloudService);
  else
    ROS_WARN("Could not find parameter %s.", "/CloudServiceActive");

  initCV();
  initSDL();
  initGL();
  initTexture();
  initShader();
  glGenFramebuffersEXT( 1, &gFboId );  
  resizeWindow(gImageWidth, gImageHeight);
  setState(WAITING_FOR_TRIGGER);

  return true;
}
int main(int argc, char ** argv) {
  init(argc, argv);
  mainLoop();
  cleanUp();
  return 0;
}
