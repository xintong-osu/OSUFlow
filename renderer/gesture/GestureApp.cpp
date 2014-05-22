#include "GestureApp.h"

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
	updateShader();


	resetCamera();

	setWantsKeyboardFocus( true );

	m_bPaused = false;

	m_fFrameScale = 0.0075f;
	m_mtxFrameTransform.origin = Leap::Vector( 0.0f, -2.0f, 0.5f );
	m_fPointableRadius = 0.05f;

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
	glEnable(GL_TEXTURE_2D);

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
			_streamlines = nullptr;
			_attributes = nullptr;
			_uniforms = nullptr;

			_shader = newShader;
			_shader->use();

			_streamlines      = new StreamlineGL (m_openGLContext);
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

	// draw the grid background
	{
		LeapUtilGL::GLAttribScope colorScope(GL_CURRENT_BIT);

		glColor3f( 0, 0, 1 );

		{
			LeapUtilGL::GLMatrixScope gridMatrixScope;

			glTranslatef( 0, 0.0f, -1.5f );

			glScalef( 3, 3, 3 );

			LeapUtilGL::drawGrid( LeapUtilGL::kPlane_XY, 20, 20 );
		}

		{
			LeapUtilGL::GLMatrixScope gridMatrixScope;

			glTranslatef( 0, -1.5f, 0.0f );

			glScalef( 3, 3, 3 );

			LeapUtilGL::drawGrid( LeapUtilGL::kPlane_ZX, 20, 20 );
		}
	}

	switch(_mode)
	{
	case 1:
		drawStreamlines();
		break;
	case 2:
		drawFingerTrace();
		break;
	default:
		break;
	}

	_shader->use(); 

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

	_streamlines->draw (m_openGLContext, *_attributes);

	// Reset the element buffers so child Components draw correctly
	m_openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
	m_openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

	// draw fingers/tools as lines with sphere at the tip.
	drawPointables( frame );

	{
		ScopedLock renderLock(m_renderMutex);

		// draw the text overlay
		renderOpenGL2D();
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
		float3 p = _dm.ConvertCoordinates(_fingerTrace[i]);
		glVertex3f(p.x, p.y, p.z);
	}
	glEnd(); 
}

void OpenGLCanvas::drawPointables( Leap::Frame frame )
{
	LeapUtilGL::GLAttribScope colorScope( GL_CURRENT_BIT | GL_LINE_BIT );

	const Leap::PointableList& pointables = frame.pointables();

	const float fScale = m_fPointableRadius;

	glLineWidth( 3.0f );

	_dm.ClearSeeds();
	m_strCoordinates = "";
	for ( int i = 0, e = pointables.count(); i < e; i++ )
	{
		const Leap::Pointable&  pointable   = pointables[i];
		Leap::Vector            vStartPos   = m_mtxFrameTransform.transformPoint( pointable.tipPosition() * m_fFrameScale );
		Leap::Vector            vEndPos     = m_mtxFrameTransform.transformDirection( pointable.direction() ) * -0.25f;
		const uint32_t          colorIndex  = static_cast<uint32_t>(pointable.id()) % kNumColors;
		glColor3fv( m_avColors[colorIndex].toFloatPointer() );

		stringstream ss;
		ss<<"x:"<< pointable.tipPosition().x;
		ss<<", y:"<< pointable.tipPosition().y;
		ss<<", z:"<< pointable.tipPosition().z;
		ss<<"\n";
		m_strCoordinates.append(ss.str(), 100);

		{
			LeapUtilGL::GLMatrixScope matrixScope;

			glTranslatef( vStartPos.x, vStartPos.y, vStartPos.z );
			_dm.InsertSeed(VECTOR3(
				pointable.tipPosition().x, 
				pointable.tipPosition().y, 
				pointable.tipPosition().z));

			//glBegin(GL_LINES);

			//glVertex3f( 0, 0, 0 ); 
			//glVertex3fv( vEndPos.toFloatPointer() );

			//glEnd();

			glScalef( fScale, fScale, fScale );

			//     LeapUtilGL::drawSphere( LeapUtilGL::kStyle_Solid );
			if(i == 0)	{
				_fingerTrace.push_back(_dm.ConvCoordsLeap2Data(
					pointable.tipPosition().x, 
					pointable.tipPosition().y, 
					pointable.tipPosition().z));
			}
		}


	}
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