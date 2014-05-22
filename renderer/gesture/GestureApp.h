#ifndef GESTURE_APP_H
#define GESTURE_APP_H
#include "JuceLibraryCode\JuceHeader.h"
#include "Leap.h"
#include "LeapUtilGL.h"
#include "DataMgr.h"
#include "StreamlineGL.h"
//#include <cctype>


// intermediate class for convenient conversion from JUCE color
// to float vector argument passed to GL functions
struct GLColor 
{
	GLColor() : r(1), g(1), b(1), a(1) {}

	GLColor( float _r, float _g, float _b, float _a=1 )
		: r(_r), g(_g), b(_b), a(_a)
	{}

	explicit GLColor( const Colour& juceColor ) 
		: r(juceColor.getFloatRed()), 
		g(juceColor.getFloatGreen()),
		b(juceColor.getFloatBlue()),
		a(juceColor.getFloatAlpha())
	{}

	operator const GLfloat*() const { return &r; }

	GLfloat r, g, b, a; 
};
//==============================================================================
class OpenGLCanvas  : public Component,
	public OpenGLRenderer,
	Leap::Listener
{
public:
	OpenGLCanvas();

	~OpenGLCanvas();

	void newOpenGLContextCreated();

	void openGLContextClosing()
	{
	}

	bool keyPressed( const KeyPress& keyPress );

	void mouseDown (const MouseEvent& e)
	{
		m_camera.OnMouseDown( LeapUtil::FromVector2( e.getPosition() ) );
	}

	void mouseDrag (const MouseEvent& e)
	{
		m_camera.OnMouseMoveOrbit( LeapUtil::FromVector2( e.getPosition() ) );
		m_openGLContext.triggerRepaint();
	}

	void mouseWheelMove ( const MouseEvent& e,
		const MouseWheelDetails& wheel )
	{
		(void)e;
		m_camera.OnMouseWheel( wheel.deltaY );
		m_openGLContext.triggerRepaint();
	}

	void resized()
	{
	}

	void paint(Graphics&)
	{
	}

	void renderOpenGL2D() ;

	//
	// calculations that should only be done once per leap data frame but may be drawn many times should go here.
	//   
	void update( Leap::Frame frame );

	/// affects model view matrix.  needs to be inside a glPush/glPop matrix block!
	void setupScene();

	void updateShader();

	// data should be drawn here but no heavy calculations done.
	// any major calculations that only need to be updated per leap data frame
	// should be handled in update and cached in members.
	void renderOpenGL();

	void drawStreamlines();

	void drawFingerTrace();

	void drawPointables( Leap::Frame frame );

	virtual void onInit(const Leap::Controller&) 
	{
	}

	virtual void onConnect(const Leap::Controller&) 
	{
	}

	virtual void onDisconnect(const Leap::Controller&) 
	{
	}

	virtual void onFrame(const Leap::Controller& controller)
	{
		if ( !m_bPaused )
		{
			Leap::Frame frame = controller.frame();
			update( frame );
			m_lastFrame = frame;
			m_openGLContext.triggerRepaint();
		}
	}

	void resetCamera()
	{
		m_camera.SetOrbitTarget( Leap::Vector::zero() );
		m_camera.SetPOVLookAt( Leap::Vector( 0, 2, 4 ), m_camera.GetOrbitTarget() );
	}

	void initColors();

private:
	OpenGLContext               m_openGLContext;
	LeapUtilGL::CameraGL        m_camera;
	Leap::Frame                 m_lastFrame;
	double                      m_fLastUpdateTimeSeconds;
	double                      m_fLastRenderTimeSeconds;
	Leap::Matrix                m_mtxFrameTransform;
	float                       m_fFrameScale;
	float                       m_fPointableRadius;
	LeapUtil::RollingAverage<>  m_avgUpdateDeltaTime;
	LeapUtil::RollingAverage<>  m_avgRenderDeltaTime;
	String                      m_strUpdateFPS;
	String                      m_strRenderFPS;
	String                      m_strPrompt;
	String                      m_strHelp;
	String						m_strCoordinates;
	Font                        m_fixedFont;
	CriticalSection             m_renderMutex;
	bool                        m_bShowHelp;
	bool                        m_bPaused;

	enum  { kNumColors = 256 };
	Leap::Vector            m_avColors[kNumColors];

	ScopedPointer<OpenGLShaderProgram> _shader;
    ScopedPointer<StreamlineGL> _streamlines;
    ScopedPointer<Attributes> _attributes;
    ScopedPointer<Uniforms> _uniforms;
       
	//ScopedPointer<DemoControlsOverlay> controlsOverlay;
       
	String newVertexShader, newFragmentShader;
	DataMgr _dm;
	int _mode;
	vector<VECTOR3> _fingerTrace;

	//==============================================================================
    
};


//==============================================================================
/**
This is the top-level window that we'll pop up. Inside it, we'll create and
show a component from the MainComponent.cpp file (you can open this file using
the Jucer to edit it).
*/
class VisualizerWindow  : public DocumentWindow
{
public:
	//==============================================================================
	VisualizerWindow()
		: DocumentWindow ("Leap Finger Visualizer",
		Colours::lightgrey,
		DocumentWindow::allButtons,
		true)
	{
		setContentOwned (new OpenGLCanvas(), true);

		// Centre the window on the screen
		centreWithSize (getWidth(), getHeight());

		// And show it!
		setVisible (true);

		getChildComponent(0)->grabKeyboardFocus();
	}

	~VisualizerWindow()
	{
		// (the content component will be deleted automatically, so no need to do it here)
	}

	//==============================================================================
	void closeButtonPressed()
	{
		// When the user presses the close button, we'll tell the app to quit. This
		JUCEApplication::quit();
	}
};

//==============================================================================
class GestureApp  : public JUCEApplication
{
public:
	//==============================================================================
	GestureApp()
	{
	}

	~GestureApp()
	{
	}

	//==============================================================================
	void initialise (const String& commandLine);

	void shutdown()
	{
		// Do your application's shutdown code here..

	}

	//==============================================================================
	void systemRequestedQuit()
	{
		quit();
	}

	//==============================================================================
	const String getApplicationName()
	{
		return "Gesture-based Flow Visualization";
	}

	const String getApplicationVersion()
	{
		return ProjectInfo::versionString;
	}

	bool moreThanOneInstanceAllowed()
	{
		return true;
	}

	void anotherInstanceStarted (const String& commandLine)
	{
		(void)commandLine;        
	}

	static Leap::Controller& getController() 
	{
		static Leap::Controller s_controller;

		return  s_controller;
	}

private:
	ScopedPointer<VisualizerWindow>  m_pMainWindow; 
};


#endif