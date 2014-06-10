#include "GestureApp.h"
#include "thrust\device_reference.h"
#include "cutil_math.h"
//#include "LeapUtilGL.h"
using namespace Leap;

//inline void drawGridFace(Leap::Vector v0, Leap::Vector v1)
//{
//	//GLAttribScope lightingScope( GL_LIGHTING_BIT );
//	glPushAttrib( GL_LIGHTING_BIT );
//
//	glDisable(GL_LIGHTING);
//
//	glBegin( GL_LINES );
//
//	for ( float x = -fHalfGridSize; x < fHEndStep; x += fHGridStep )
//	{
//		glVertex3f( x, -fHalfGridSize, 0 );
//		glVertex3f( x, fHalfGridSize, 0 );
//	}
//
//	for ( float y = -fHalfGridSize; y < fVEndStep; y += fVGridStep )
//	{
//		glVertex3f( -fHalfGridSize, y, 0 );
//		glVertex3f( fHalfGridSize, y, 0 );
//	}
//	glEnd();
//
//	glPopAttrib();
//}

inline void drawBBox(float3 minLen, float3 maxLen)//float3 v[8])//v0, float3 v1, float3 v2, float3 v3, float3 v4, float3 v5, float3 v6, float3 v7)
{
	static const float s_afCorners[8][3] = {
		// near face - ccw facing origin from face.
		{minLen.x, minLen.y, maxLen.z},
		{maxLen.x, minLen.y, maxLen.z},
		{maxLen.x, maxLen.y, maxLen.z},
		{minLen.x, maxLen.y, maxLen.z},

		// far face - ccw facing origin from face
		{minLen.x, minLen.y, minLen.z},
		{maxLen.x, minLen.y, minLen.z},
		{maxLen.x, maxLen.y, minLen.z},
		{minLen.x, maxLen.y, minLen.z}
	};

	glPushAttrib( GL_LIGHTING_BIT );
	glDisable(GL_LIGHTING);

	glBegin( GL_LINE_LOOP );

	// near
	glNormal3f( 0, 0, 1 );
	glVertex3fv( s_afCorners[0] );
	glVertex3fv( s_afCorners[1] );
	glVertex3fv( s_afCorners[2] );
	glVertex3fv( s_afCorners[3] );

	glEnd();
	glBegin( GL_LINE_LOOP );

	glVertex3fv( s_afCorners[4] );
	glVertex3fv( s_afCorners[5] );
	glVertex3fv( s_afCorners[6] );
	glVertex3fv( s_afCorners[7] );

	glEnd();

	glBegin( GL_LINE_LOOP );

	// far
	glNormal3f( 0, 0, -1 );
	glVertex3fv( s_afCorners[1] );
	glVertex3fv( s_afCorners[5] );
	glVertex3fv( s_afCorners[6] );
	glVertex3fv( s_afCorners[2] );

	glEnd();
	glBegin( GL_LINE_LOOP );

	glVertex3fv( s_afCorners[5] );
	glVertex3fv( s_afCorners[4] );
	glVertex3fv( s_afCorners[7] );
	glVertex3fv( s_afCorners[6] );

	glEnd();

	glBegin( GL_LINE_LOOP );

	// right
	glNormal3f( 1, 0, 0 );
	glVertex3fv( s_afCorners[4] );
	glVertex3fv( s_afCorners[0] );
	glVertex3fv( s_afCorners[3] );
	glVertex3fv( s_afCorners[7] );

	glEnd();
	glBegin( GL_LINE_LOOP );

	glVertex3fv( s_afCorners[3] );
	glVertex3fv( s_afCorners[2] );
	glVertex3fv( s_afCorners[6] );
	glVertex3fv( s_afCorners[7] );

	glEnd();
	glBegin( GL_LINE_LOOP );

	// left
	glNormal3f( -1, 0, 0 );
	glVertex3fv( s_afCorners[4] );
	glVertex3fv( s_afCorners[5] );
	glVertex3fv( s_afCorners[1] );
	glVertex3fv( s_afCorners[0] );

	glEnd();

	glPopAttrib();
}


OpenGLCanvas::	OpenGLCanvas()
	: Component( "OpenGLCanvas" )
{
	m_openGLContext.setRenderer (this);
	m_openGLContext.setComponentPaintingEnabled (true);
	m_openGLContext.attachTo (*this);
	setBounds( 0, 0, 1024, 768 );

	m_fLastUpdateTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());
	m_fLastRenderTimeSeconds = m_fLastUpdateTimeSeconds;

	GestureApp::getController().addListener( *this );

	initColors();

	_dm.initOSUFlow();
	_sc.SetCenter(CoordsData2GL(_dm.GetDataCenter()));
	_DataScale = 2.0f / _dm.GetDomainSize();
	_sc.SetRadiusIn(_dm.GetDomainSize() * _DataScale * 0.03);
	_sc.SetRadiusOut(_sc.GetRadiusIn() * 2);
	_mode = 1;

	//updateShader();
	resetCamera();

	setWantsKeyboardFocus( true );

	m_bPaused = false;

	m_fFrameScale = 0.0075f;
	m_mtxFrameTransform.origin = Leap::Vector( 0.0f, -2.0f, 0.5f );
	m_fPointableRadius = 0.02f;

	m_bShowHelp = false;

	m_strHelp = "ESC - quit\n"
		"h - Toggle help and frame rate display\n"
		"p - Toggle pause\n"
		"Mouse Drag  - Rotate camera\n"
		"Mouse Wheel - Zoom camera\n"
		"Arrow Keys  - Rotate camera\n"
		"Space       - Reset camera";

	m_strPrompt = "Press 'h' for help";
}

OpenGLCanvas::	~OpenGLCanvas()
{
	GestureApp::getController().removeListener( *this );
	m_openGLContext.detach();
}

void OpenGLCanvas:: newOpenGLContextCreated()
{
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_LESS);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glShadeModel(GL_SMOOTH);

	glEnable(GL_LIGHTING);

	m_fixedFont = Font("Courier New", 24, Font::plain );

	_slFingers      = new StreamlineGL (m_openGLContext);
}

bool OpenGLCanvas::keyPressed( const KeyPress& keyPress )
{
	int iKeyCode = toupper(keyPress.getKeyCode());

	if ( iKeyCode == KeyPress::escapeKey )
	{
		JUCEApplication::quit();
		return true;
	}

	if ( iKeyCode == KeyPress::upKey )
	{
		m_camera.RotateOrbit( 0, 0, LeapUtil::kfHalfPi * -0.05f );
		return true;
	}

	if ( iKeyCode == KeyPress::downKey )
	{
		m_camera.RotateOrbit( 0, 0, LeapUtil::kfHalfPi * 0.05f );
		return true;
	}

	if ( iKeyCode == KeyPress::leftKey )
	{
		m_camera.RotateOrbit( 0, LeapUtil::kfHalfPi * -0.05f, 0 );
		return true;
	}

	if ( iKeyCode == KeyPress::rightKey )
	{
		m_camera.RotateOrbit( 0, LeapUtil::kfHalfPi * 0.05f, 0 );
		return true;
	}

	switch( iKeyCode )
	{
	case ' ':
		resetCamera();
		break;
	case 'H':
		m_bShowHelp = !m_bShowHelp;
		break;
	case 'P':
		m_bPaused = !m_bPaused;
		break;
	case 'Q':
		_mode = 1;
		break;
	case 'W':
		_mode = 2;
		break;
	case 'E':
		_mode = 3;
		break;
	case 'R':
		_mode = 4;
		break;
	case 'T':
		_mode = 5;
	case 'C':
		_fingerTrace.clear();
		break;
	default:
		return false;
	}

	return true;
}

void OpenGLCanvas:: renderOpenGL2D() 
{
	LeapUtilGL::GLAttribScope attribScope( GL_ENABLE_BIT );

	// when enabled text draws poorly.
	glDisable(GL_CULL_FACE);

	ScopedPointer<LowLevelGraphicsContext> glRenderer (createOpenGLGraphicsContext (m_openGLContext, getWidth(), getHeight()));

	if (glRenderer != nullptr)
	{
		Graphics g(*glRenderer.get());

		int iMargin   = 10;
		int iFontSize = static_cast<int>(m_fixedFont.getHeight());
		int iLineStep = iFontSize + (iFontSize >> 2);
		int iBaseLine = 20;
		Font origFont = g.getCurrentFont();

		const Rectangle<int>& rectBounds = getBounds();

		if ( m_bShowHelp )
		{
			g.setColour( Colours::seagreen );
			g.setFont( static_cast<float>(iFontSize) );

			if ( !m_bPaused )
			{
				g.drawSingleLineText( m_strUpdateFPS, iMargin, iBaseLine );
			}

			g.drawSingleLineText( m_strRenderFPS, iMargin, iBaseLine + iLineStep );

			g.setFont( m_fixedFont );
			g.setColour( Colours::slateblue );

			g.drawMultiLineText( m_strCoordinates,// m_strHelp,
				iMargin,
				iBaseLine + iLineStep * 3,
				rectBounds.getWidth() - iMargin*2 );
		}

		g.setFont( origFont );
		g.setFont( static_cast<float>(iFontSize) );

		g.setColour( Colours::salmon );
		m_strPrompt.clear();
		m_strPrompt.append(_sc.GetMsg(), 100);
		g.drawMultiLineText(  m_strPrompt,
			iMargin,
			rectBounds.getBottom() - (iFontSize + iFontSize + iLineStep),
			rectBounds.getWidth()/4 );
	}
}

void OpenGLCanvas::update( Leap::Frame frame )
{
	ScopedLock sceneLock(m_renderMutex);

	double curSysTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());

	float deltaTimeSeconds = static_cast<float>(curSysTimeSeconds - m_fLastUpdateTimeSeconds);

	m_fLastUpdateTimeSeconds = curSysTimeSeconds;
	float fUpdateDT = m_avgUpdateDeltaTime.AddSample( deltaTimeSeconds );
	float fUpdateFPS = (fUpdateDT > 0) ? 1.0f/fUpdateDT : 0.0f;
	m_strUpdateFPS = String::formatted( "UpdateFPS: %4.2f", fUpdateFPS );

	_sc.FingerInput(frame);
}

void OpenGLCanvas::setupScene()
{
	OpenGLHelpers::clear (Colours::black.withAlpha (1.0f));

	m_camera.SetAspectRatio( getWidth() / static_cast<float>(getHeight()) );

	m_camera.SetupGLProjection();

	m_camera.ResetGLView();

	// left, high, near - corner light
	LeapUtilGL::GLVector4fv vLight0Position( -3.0f, 3.0f, -3.0f, 1.0f );
	// right, near - side light
	LeapUtilGL::GLVector4fv vLight1Position(  3.0f, 0.0f, -1.5f, 1.0f );
	// near - head light
	LeapUtilGL::GLVector4fv vLight2Position( 0.0f, 0.0f,  -3.0f, 1.0f );

	/// JUCE turns off the depth test every frame when calling paint.
	glEnable(GL_DEPTH_TEST);
	glDepthMask(true);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, GLColor(Colours::darkgrey));

	glLightfv(GL_LIGHT0, GL_POSITION, vLight0Position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, GLColor(Colour(0.5f, 0.40f, 0.40f, 1.0f)));
	glLightfv(GL_LIGHT0, GL_AMBIENT, GLColor(Colours::black));

	glLightfv(GL_LIGHT1, GL_POSITION, vLight1Position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, GLColor(Colour(0.0f, 0.0f, 0.25f, 1.0f)));
	glLightfv(GL_LIGHT1, GL_AMBIENT, GLColor(Colours::black));

	glLightfv(GL_LIGHT2, GL_POSITION, vLight2Position);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, GLColor(Colour(0.15f, 0.15f, 0.15f, 1.0f)));
	glLightfv(GL_LIGHT2, GL_AMBIENT, GLColor(Colours::black));

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	m_camera.SetupGLView();
}

void OpenGLCanvas::updateShader()
{
	if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
	{
		ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (m_openGLContext));
		String statusText;

		if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
			&& newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
			&& newShader->link())
		{
			_slFingers = nullptr;
			_attributes = nullptr;
			_uniforms = nullptr;

			_shader = newShader;
			_shader->use();

			_attributes = new Attributes (m_openGLContext, *_shader);
			_uniforms   = new Uniforms (m_openGLContext, *_shader);

			statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
		}
		else
		{
			statusText = newShader->getLastError();
		}

		// controlsOverlay->statusLabel.setText (statusText, dontSendNotification);


		newVertexShader = String::empty;
		newFragmentShader = String::empty;
	}
}

inline vector<float3> StablizeFingerTips(vector<float3> v)
{
	vector<float3> ret;
	for(int i = 0; i < v.size(); i++)	{
		ret.push_back(make_float3((int)(v[i].x * 20) * 0.05f, (int)(v[i].y * 20) * 0.05f, (int)(v[i].z * 20) * 0.05f));
	}
	return ret;
}

void OpenGLCanvas::renderOpenGL()
{
	{
		MessageManagerLock mm (Thread::getCurrentThread());
		if (! mm.lockWasGained())
			return;
	}

	Leap::Frame frame = m_lastFrame;

	double  curSysTimeSeconds = Time::highResolutionTicksToSeconds(Time::getHighResolutionTicks());
	float   fRenderDT = static_cast<float>(curSysTimeSeconds - m_fLastRenderTimeSeconds);
	fRenderDT = m_avgRenderDeltaTime.AddSample( fRenderDT );
	m_fLastRenderTimeSeconds = curSysTimeSeconds;

	float fRenderFPS = (fRenderDT > 0) ? 1.0f/fRenderDT : 0.0f;

	m_strRenderFPS = String::formatted( "RenderFPS: %4.2f", fRenderFPS );

	LeapUtilGL::GLMatrixScope sceneMatrixScope;

	setupScene();

	vector<float3> rack;
	// draw the grid background
	{
		LeapUtilGL::GLAttribScope colorScope(GL_CURRENT_BIT);

		glColor3f( 0, 0, 1 );

		//{
		//	LeapUtilGL::GLMatrixScope gridMatrixScope;

		//	glTranslatef( 0, 0.0f, -1.5f );

		//	glScalef( 3, 3, 3 );

		//	LeapUtilGL::drawGrid( LeapUtilGL::kPlane_XY, 20, 20 );
		//}

		//{
		//	LeapUtilGL::GLMatrixScope gridMatrixScope;

		//	glTranslatef( 0, -1.5f, 0.0f );

		//	glScalef( 3, 3, 3 );

		//	LeapUtilGL::drawGrid( LeapUtilGL::kPlane_ZX, 20, 20 );
		//}
	}
	switch(_mode)
	{
	case 1:
		{
			//drawStreamlines();
			vector<vector<float3>> sls;
			sls = _dm.compute_streamlines(CoordsGL2DataVector((_fingerTips)));//StablizeFingerTips
			CoordsData2GLVector(sls);
			_slFingers->UpdateData(m_openGLContext, &sls);
			_slFingers->draw(m_openGLContext);//, _attributes);
			break;
		}
	case 2:
		{
			vector<float3> ft = CoordsGL2DataVector((_fingerTips));//StablizeFingerTips
			if(ft.size() < 2)
				return;
			float3 a0 = ft.at(0);
			float3 a1 = ft.at(1);
			int nSteps = 16;
			float3 step = (a1 - a0) / (nSteps);
			for(int i = 0 ; i < nSteps; i ++)	{
				rack.push_back(a0 + i * step);
			}
			vector<vector<float3>> sls;
			sls = _dm.compute_streamlines(rack);

			CoordsData2GLVector(sls);
			_slFingers->UpdateData(m_openGLContext, &sls);
			_slFingers->draw(m_openGLContext);//, _attributes);

			glColor3f(0.8f, 0.2f, 0.1f);
			glPointSize(5);
			glBegin(GL_POINTS);
			for(int i = 0 ; i < rack.size(); i++)	{
				float3 p= CoordsData2GL(rack[i]);
				glVertex3fv(&p.x);
			}
			glEnd();
			glColor3f(1.0f, 1.0f, 1.0f);
			break;
		}
	case 3:
		drawFingerTrace();
		break;
	case 4:
		{
			vector<vector<float3>> sls;
			sls = _dm.compute_streamlines(CoordsGL2DataVector(_fingerTrace));//
			CoordsData2GLVector(sls);
			_slFingers->UpdateData(m_openGLContext, &sls);
			_slFingers->draw(m_openGLContext);//, _attributes);
			break;
		}
	case 5:
		{
			//drawStreamlines();
			vector<vector<float3>> sls;
			Leap::Vector p = _sc.GetCenter();
			vector<float3> pts;
			pts.push_back(make_float3(p.x, p.y, p.z));
			sls = _dm.compute_streamlines(CoordsGL2DataVector(pts));//StablizeFingerTips
			CoordsData2GLVector(sls);
			_slFingers->UpdateData(m_openGLContext, &sls);
			_slFingers->draw(m_openGLContext);//, _attributes);
			break;
		}
	default:
		break;
	}


	{

		LeapUtilGL::GLMatrixScope BBoxScope;
		float3 bb[8];
		float3 minB, maxB;
		float3 centerB;
		//_dm.GetBBox(bb);
		_dm.GetDataDomain(minB, maxB);
		centerB = _dm.GetDataCenter();
		glScalef( _DataScale, _DataScale, _DataScale);

		glRotatef(-90, 1, 0, 0);
		glTranslatef(- centerB.x, - centerB.y, - centerB.z);
		drawBBox(minB, maxB);//float3 v0, float3 v1, float3 v2, float3 v3, float3 v4, float3 v5, float3 v6, float3 v7);

		//LeapUtilGL::GLMatrixScope gridMatrixScope;

		//glTranslatef( minB.x, minB.y, minB.z);

		//glScalef( 3, 3, 3 );

		LeapUtilGL::drawGrid( LeapUtilGL::kPlane_ZX, 20, 20 );
	}


	//_shader->use(); 

	//if (_uniforms->projectionMatrix != nullptr)
	//    _uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

	//if (_uniforms->viewMatrix != nullptr)
	//    _uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

	//if (_uniforms->texture != nullptr)
	//    _uniforms->texture->set ((GLint) 0);

	//if (_uniforms->lightPosition != nullptr)
	//    _uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);

	//if (_uniforms->bouncingNumber != nullptr)
	//    _uniforms->bouncingNumber->set (bouncingNumber.getValue());

	//	_slFingers->draw (m_openGLContext, *_attributes);

	//// Reset the element buffers so child Components draw correctly
	//m_openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
	//m_openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);


	// draw fingers/tools as lines with sphere at the tip.
//	drawPointables( frame );

	_sc.Draw();

	{
		ScopedLock renderLock(m_renderMutex);

		// draw the text overlay
		renderOpenGL2D();
	}
}

float3 OpenGLCanvas::CoordsGL2Data(float3 v)
{
	//centerB = _dm.GetDataCenter();
	//float fScale = 2.0f / _dm.GetDomainSize();
	//glScalef( fScale, fScale, fScale );
	v = v / _DataScale;
	float3 dc = _dm.GetDataCenter();
	Leap::Matrix mRot = Leap::Matrix(Leap::Vector(1, 0, 0), LeapUtil::kfHalfPi);//, Leap::Vector(dc.x, dc.y, dc.z));
	Leap::Vector vv = mRot.transformPoint(Leap::Vector(v.x,v.y, v.z));
	return make_float3(vv.x + dc.x, vv.y  + dc.y, vv.z + dc.z);
	//	glRotatef(-90, 1, 0, 0);
	//	glTranslatef(- centerB.x, - centerB.y, - centerB.z);
}


float3 OpenGLCanvas::CoordsData2GL(float3 v)
{
	//centerB = _dm.GetDataCenter();
	//float fScale = 2.0f / _dm.GetDomainSize();
	//glScalef( fScale, fScale, fScale );
	//v = v / _DataScale;
	//float3 dc = _dm.GetDataCenter();
	//Leap::Matrix mRot = Leap::Matrix(Leap::Vector(1, 0, 0), LeapUtil::kfHalfPi);//, Leap::Vector(dc.x, dc.y, dc.z));
	//Leap::Vector vv = mRot.transformPoint(Leap::Vector(v.x,v.y, v.z));
	float3 dc = _dm.GetDataCenter();
	v -= dc;
	Leap::Matrix mRot = Leap::Matrix(Leap::Vector(1, 0, 0), - LeapUtil::kfHalfPi);//, Leap::Vector(dc.x, dc.y, dc.z));
	Leap::Vector vv = mRot.transformPoint(Leap::Vector(v.x,v.y, v.z));
	vv *= _DataScale;
	return make_float3(vv.x, vv.y, vv.z);
	//	glRotatef(-90, 1, 0, 0);
	//	glTranslatef(- centerB.x, - centerB.y, - centerB.z);
}

vector<float3> OpenGLCanvas::CoordsGL2DataVector(vector<float3> v)
{
	vector<float3> ret;
	for(int i = 0; i < v.size(); i++)	{
		ret.push_back(CoordsGL2Data(v[i]));
	}
	return ret;
}

void OpenGLCanvas::CoordsData2GLVector(vector<vector<float3>> &v)
{
	for(int i = 0; i < v.size(); i++)	{
		for(int j = 0; j < v.at(i).size(); j++)	{
			v[i][j] = CoordsData2GL(v[i][j]);
		}
	}
}

void OpenGLCanvas::drawStreamlines()
{
	_dm.compute_streamlines();
	vector<VECTOR3> line;
	//	for(list<vtListSeedTrace*>::const_iterator
	//		pIter = _dm.GetStreamLines()->begin(); 
	//pIter!=_dm.GetStreamLines()->end(); 
	//pIter++) 
	vector<vector<float3>>* sls = _dm.GetStreamLines();
	for(int i = 0; i < sls->size(); i++)
	{
		glColor3f(1.0f, 1.0f, 0.0f);
		//const vtListSeedTrace *trace = *pIter; 
		//VECTOR3 seedPoint(-1,-1,-1);
		glBegin(GL_LINE_STRIP);
		//bool flag = true;
		/*for(list<VECTOR3*>::const_iterator
		pnIter = trace->begin(); 
		pnIter!= trace->end(); 
		pnIter++) */
		vector<float3>* sl = &sls->at(i);
		for(int j = 0; j < sls->at(i).size(); j++)
		{
			float scale = 0.04f;
			//glVertex3f(((**pnIter)[0] - len[0] * 0.5) / len[0], (((**pnIter)[1]) - len[1] * 0.5) / len[1], ((**pnIter)[2] - len[2] * 0.5) / len[2]);
			float3 p = _dm.ConvertCoordinates(sl->at(j));
			glVertex3f(p.x, p.y, p.z);
			//VECTOR3 p = **pnIter; 
			//line.push_back(); 
			//if(flag)	{
			//	seedPoint = **pnIter;
			//	flag = false;
			//}
		}
		glEnd(); 

		glPointSize(10);
		glColor3f(0.0f, 1.0f, 1.0f);
		//if(seedPoint[0] != -1)	{
		//	glBegin(GL_POINTS);
		//		glVertex3fv(&_dm.ConvertCoordinates(seedPoint)[0]);//(seedPoint[0] - len[0] * 0.5)/ len[0], (seedPoint[1]  - len[1] * 0.5)/ len[1], (seedPoint[2]  - len[2] * 0.5)/ len[2]);
		//	glEnd();
		//}
	}
}

void OpenGLCanvas::drawFingerTrace()
{
	glBegin(GL_LINE_STRIP);
	for(int i = 0; i < _fingerTrace.size(); i++)	{
		float3 p = _fingerTrace[i];////m_mtxFrameTransform.transformPoint(_fingerTrace[i]);// _dm.ConvertCoordinates(_fingerTrace[i]);
		//glVertex3f(p.x * m_fFrameScale, p.y * m_fFrameScale, p.z * m_fFrameScale);
		glVertex3f(p.x , p.y , p.z);
	}
	glEnd(); 
}


void OpenGLCanvas::drawPointables( Leap::Frame frame )
{
	LeapUtilGL::GLAttribScope colorScope( GL_CURRENT_BIT | GL_LINE_BIT );

	const Leap::PointableList& pointables = frame.pointables();

	const float fScale = m_fPointableRadius;

	glLineWidth( 3.0f );

	//_dm.ClearSeeds();
	m_strCoordinates = "";
	_fingerTips.clear();

	const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
	HandList hands = frame.hands();
	for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
		// Get the first hand
		const Hand hand = *hl;

		// Get fingers
		const FingerList fingers = hand.fingers();
		for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
			const Finger finger = *fl;
			Leap::Vector vStartPos   = m_mtxFrameTransform.transformPoint( finger.tipPosition() * m_fFrameScale );

			stringstream ss;
			ss<<"x:"<< finger.bone(Bone::Type::TYPE_DISTAL).center().x;
			ss<<", y:"<< finger.bone(Bone::Type::TYPE_DISTAL).center().y;
			ss<<", z:"<< finger.bone(Bone::Type::TYPE_DISTAL).center().z;
			ss<<"\n";
			m_strCoordinates.append(ss.str(), 100);

			{
				LeapUtilGL::GLMatrixScope matrixScope;

				glTranslatef( vStartPos.x, vStartPos.y, vStartPos.z );
				glScalef( fScale, fScale, fScale );

				LeapUtilGL::drawSphere( LeapUtilGL::kStyle_Solid );
				_fingerTips.push_back(make_float3(vStartPos.x, vStartPos.y, vStartPos.z));
				if(finger.type() == 1)	{
					_fingerTrace.push_back(make_float3(vStartPos.x, vStartPos.y, vStartPos.z));
				}
			}
		}
	}

	//for ( int i = 0, e = pointables.count(); i < e; i++ )
	//{
	//	const Leap::Pointable&  pointable   = pointables[i];
	//	Leap::Vector            vStartPos   = m_mtxFrameTransform.transformPoint( pointable.tipPosition() * m_fFrameScale );
	//	Leap::Vector            vEndPos     = m_mtxFrameTransform.transformDirection( pointable.direction() ) * -0.25f;
	//	const uint32_t          colorIndex  = static_cast<uint32_t>(pointable.id()) % kNumColors;
	//	glColor3fv( m_avColors[colorIndex].toFloatPointer() );

	//	stringstream ss;
	//	ss<<"x:"<< pointable.tipPosition().x;
	//	ss<<", y:"<< pointable.tipPosition().y;
	//	ss<<", z:"<< pointable.tipPosition().z;
	//	ss<<"\n";
	//	m_strCoordinates.append(ss.str(), 100);

	//	{
	//		LeapUtilGL::GLMatrixScope matrixScope;

	//		glTranslatef( vStartPos.x, vStartPos.y, vStartPos.z );
	//		//_dm.InsertSeed(VECTOR3(
	//		//	pointable.tipPosition().x, 
	//		//	pointable.tipPosition().y, 
	//		//	pointable.tipPosition().z));

	//		//glBegin(GL_LINES);

	//		//glVertex3f( 0, 0, 0 ); 
	//		//glVertex3fv( vEndPos.toFloatPointer() );

	//		//glEnd();

	//		glScalef( fScale, fScale, fScale );

	//		LeapUtilGL::drawSphere( LeapUtilGL::kStyle_Solid );
	//		_fingerTips.push_back(make_float3(vStartPos.x, vStartPos.y, vStartPos.z));
	//		if(i == 0)	{
	//			_fingerTrace.push_back(make_float3(vStartPos.x, vStartPos.y, vStartPos.z));//(Leap::Vector(
	//			/*pointable.tipPosition().x, 
	//			pointable.tipPosition().y, 
	//			pointable.tipPosition().z));*/
	//		}
	//	}


	//}
}

void OpenGLCanvas::initColors()
{
	float fMin      = 0.0f;
	float fMax      = 1.0f;
	float fNumSteps = static_cast<float>(pow( kNumColors, 1.0/3.0 ));
	float fStepSize = (fMax - fMin)/fNumSteps;
	float fR = fMin, fG = fMin, fB = fMin;

	for ( uint32_t i = 0; i < kNumColors; i++ )
	{
		m_avColors[i] = Leap::Vector( fR, fG, LeapUtil::Min(fB, fMax) );

		fR += fStepSize;

		if ( fR > fMax )
		{
			fR = fMin;
			fG += fStepSize;

			if ( fG > fMax )
			{
				fG = fMin;
				fB += fStepSize;
			}
		}
	}

	Random rng(0x13491349);

	for ( uint32_t i = 0; i < kNumColors; i++ )
	{
		uint32_t      uiRandIdx    = i + (rng.nextInt() % (kNumColors - i));
		Leap::Vector  vSwapTemp    = m_avColors[i];

		m_avColors[i]          = m_avColors[uiRandIdx];
		m_avColors[uiRandIdx]  = vSwapTemp;
	}
}

void GestureApp::initialise (const String& commandLine)
{
	(void) commandLine;
	// Do your application's initialisation code here..
	m_pMainWindow = new VisualizerWindow();
}