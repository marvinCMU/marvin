#include <QtGui>
#include <QtOpenGL>

#include <math.h>
#include "ER_QGLWidget.h"
#include "Renderer.h"

using namespace std;

ER_QGLWidget::ER_QGLWidget(QWidget *parent)
: QGLWidget( parent ), isTrajectoryShown(false), isCameraHistoryShown(false), isCapturing(false)
{
	currentFrame = 0;
	viewX_ = 0.0;
	viewY_ = 0.0;
	viewZ_ = -10.0;
	zoom_ = 1.0;

	trackball(quat_, 0.0, 0.0, 0.0, 0.0);
}

ER_QGLWidget::~ER_QGLWidget()
{
}

QSize ER_QGLWidget::minimumSizeHint() const
{
	return QSize(50, 50);
}
QSize ER_QGLWidget::sizeHint() const
{
	return QSize(800, 800);
}
void ER_QGLWidget::initializeGL()
{
	glClearColor( 0.0, 0.0, 0.0, 0.0 );
	glClearDepth( 1.0f );
	glEnable( GL_DEPTH_TEST );
	glDepthMask( GL_TRUE );

	// anti-aliasing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_BLEND );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	glEnable( GL_LINE_SMOOTH );
	glEnable( GL_POINT_SMOOTH );
}

void ER_QGLWidget::paintGL()
{
	if (!renderer.isBackGroundBlack)
	{
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
		renderer.isBackGroundBlack = true;
	}
	else
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		renderer.isBackGroundBlack = false;
	}
	cout<<"b" << endl;
	glViewport( 0, 0, (double)this->width(), (double)this->height() );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 20, (double)this->width()/(double)this->height(), 0.05, 10000 );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glTranslatef( viewX_ , viewY_, viewZ_ ); 
	build_rotmatrix( rot_, quat_ );
	glMultMatrixf( &rot_[0][0] );	
	glPushMatrix();

	/////////////////////////////////////////////////
	// Rendering 3D
	renderer.UpdateColor();
	renderer.RenderStaticStructure();
	renderer.RenderStaticCamera();
	//renderer.RenderDynamicStructure(currentFrame);
	//
	//if (isTrajectoryShown)
	//	renderer.RenderTrajectory(currentFrame);

	//if (isCameraHistoryShown)
	//	renderer.RenderDynamicCameraHistory(currentFrame);
	//else
	//	renderer.RenderDynamicCamera(currentFrame);
	if (isCapturing)
	{
		CaptureImage();
	}

	glPopMatrix();
	glFlush();
}

void ER_QGLWidget::resizeGL(int width, int height)
{
	glViewport( 0, 0, width, height );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 20, (double)this->width()/(double)this->height(), 0.05, 1000 );
}

void ER_QGLWidget::mousePressEvent( QMouseEvent* e )
{
	prevX_ = e->x();
	prevY_ = e->y();
}

void ER_QGLWidget::mouseMoveEvent( QMouseEvent* e )
{
	int x = e->x();
	int y = e->y();

	// for rotation
	if( e->buttons() & Qt::RightButton ){
		float rot_quat[4];
		trackball( rot_quat, (2.0 * prevX_ - this->width()) / (double)(this->width()), (this->height() - 2.0 * prevY_ ) / (double)(this->width()),
			(2.0 * x - this->width()) / (double)(this->width()), (this->height() - 2.0 * y) / (double)(this->width()) );
		add_quats( rot_quat, quat_, quat_ );

		prevX_ = x;
		prevY_ = y;

		this->glDraw();
	}

	// for translation
	else if( e->buttons() & Qt::LeftButton ){
		double ratio = zoom_ / 5.0;
		viewX_ += (double)( x - prevX_ ) * ratio / (double)(this->width());
		viewY_ += -(double)( y - prevY_ ) * ratio / (double)(this->height());
		prevX_ = x;
		prevY_ = y;
		this->glDraw();
	}
}

void ER_QGLWidget::wheelEvent( QWheelEvent* e )
{
	viewZ_ += ( e->delta() ) * 5 / (double)(this->height());
	this->glDraw();
}

void ER_QGLWidget::UpdateFrame()
{
	//emit FrameFromPlaybackChanged(currentFrame);
	//emit FrameFromPlaybackChanged_Label(GetCurrentLabel());
	this->glDraw();
}

void ER_QGLWidget::SetFrameFromPlayback(int frame)
{
	if (currentFrame != frame)
	{
		currentFrame = frame;
		UpdateFrame();
	}
}

void ER_QGLWidget::SetPlayButton(bool checked)
{
	if (isPlayButtonPressed != checked)
	{
		isPlayButtonPressed = checked;
	}

	if (isPlayButtonPressed)
		emit PlayButtonChanged((int)(1000.0/(double)renderer.renderingFrequency));
	else
		Stop();
}

QString ER_QGLWidget::GetCurrentLabel()
{
	QString currentFrame_str, totalFrame;	
	
	currentFrame_str.setNum((double)currentFrame/(double)renderer.renderingFrequency);
	currentFrame_str += "/";
	totalFrame.setNum((double)renderer.max_nFrames-1);
	currentFrame_str += totalFrame;
	return currentFrame_str;
}

void ER_QGLWidget::Play()
{
	if (currentFrame < renderer.renderingFrequency*(renderer.max_nFrames-1))
	{
		currentFrame++;
		UpdateFrame();
	}
	else
	{
		Stop();
		emit UntoggleButton();
	}
}

void ER_QGLWidget::Stop()
{
	emit StopTimer();
}

void ER_QGLWidget::ShowTrajectory()
{
	isTrajectoryShown = true;
	UpdateFrame();
}

void ER_QGLWidget::HideTrajectory()
{
	isTrajectoryShown = false;
	UpdateFrame();
}

void ER_QGLWidget::ShowCameraHistory()
{
	isCameraHistoryShown = true;
	UpdateFrame();
}

void ER_QGLWidget::HideCameraHistory()
{
	isCameraHistoryShown = false;
	UpdateFrame();
}

void ER_QGLWidget::ChangeStaticPointSize(int plus)
{
	if (renderer.staticPointSize + plus > 0)
		renderer.staticPointSize += plus;
	UpdateFrame();
}

void ER_QGLWidget::ChangeDynamicPointSize(int plus)
{
	if (renderer.dynamicPointSize + plus > 0)
		renderer.dynamicPointSize += plus;
	UpdateFrame();
}

void ER_QGLWidget::ChangeStaticPointColor()
{
	if (renderer.isStaticPointColorMono == false)
		renderer.isStaticPointColorMono = true;
	else
		renderer.isStaticPointColorMono = false;
	UpdateFrame();
}

void ER_QGLWidget::ChangeDynamicPointColor()
{
	if (renderer.isDynamicPointColorMono == false)
		renderer.isDynamicPointColorMono = true;
	else
		renderer.isDynamicPointColorMono = false;
	UpdateFrame();
}

void ER_QGLWidget::CaptureImage()
{
	QImage image = grabFrameBuffer();
	char fname_frame[1024];
	string filename = renderer.filePath + "CapturedImage";
	sprintf( fname_frame, "%s_%07d.bmp", filename.c_str(), renderer.capturedFileIndex);
	grabFrameBuffer().save(fname_frame);
	renderer.capturedFileIndex++;
}
