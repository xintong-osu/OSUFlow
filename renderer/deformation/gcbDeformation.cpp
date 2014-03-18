
//ADD-BY-TONG 02/15/2013-BEGIN
//#define FILENAME "data/15plume3d440_2.vec.trace"
//#define FILENAME "data/wrfout_d01_2007-10-01_18-00-00.t_0.vec.trace"
//#define FILENAME "data/15plume3d440_seed300_len500_norm.vec.trace"
//#define FILENAME "data/15plume3d440_seed100_len10000.vec.trace"
//#define FILENAME "D:\\data\\plume\\15plume3d440_seed200_len500_fix.vec.trace"
//#define FILENAME "D:\\data\\isabel\\UVWf01_step500_seed500_seg.vec.trace"
#if 0
#define FILENAME "data\\UVWf01_step500_seed500_seg.vec.trace"
#elif 0
#define FILENAME "D:\\data\\nek\\nek.d_2_seg.vec.trace"
#else
#define FILENAME "deform_data\\plume\\15plume3d421_seg.vec.trace"
#endif
//#define FILENAME_VEC "D:\\Dropbox\\data\\UVWf01.vec"
//#define FILENAME "data/15plume3d440_seed500_len250_norm.vec.trace"
int frameCount;
int currentTime;
int previousTime;
float fps;
//#include <GL\glew.h>
//ADD-BY-TONG 02/15/2013-END



#include <list>
#include <iterator>

//#include "gcb.h"
#include "gcb2.h"
#include "OSUFlow.h"
#include "TimeLineRendererInOpenGLDeform.h"
//#include "LineRendererInOpenGLDeform.h"
#include "StreamDeform.h"
//#include <QApplication>


char *szVecFilePath;
OSUFlow *osuflow; 
VECTOR4 minLen, maxLen; 
list<vtListTimeSeedTrace*> sl_list; 
float center[3], len[3]; 
list<VECTOR4> liv4Colors;
int argc = 0;
char **argv;
//QApplication app(argc, argv);
CTimeLineRendererInOpenGLDeform cLineRenderer;

int deformWindowId;
bool _dragLens;
bool _dragLensEndPt;
VECTOR2 _dragStartPos;
VECTOR2 _dragStartPosLine;
VECTOR2 _dragCurrentPosLine;


//
//void InitializeOpenGLExtensions()
//{
//	GLenum err = glewInit();
//	if (GLEW_OK != err)
//	{
//	  /* Problem: glewInit failed, something is seriously wrong. */
//	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
//	  throw "Error initializing GLEW";
//	}
//	if (!GLEW_VERSION_2_1)
//	{
//		throw "Fatal Error: OpenGL 2.1 is required";
//	}
//
//	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
//}

//-------------------------------------------------------------------------
// Calculates the frames per second
//-------------------------------------------------------------------------
void calculateFPS()
{
    //  Increase frame count
    frameCount++;

    //  Get the number of milliseconds since glutInit called
    //  (or first call to glutGet(GLUT ELAPSED TIME)).
    currentTime = glutGet(GLUT_ELAPSED_TIME);

    //  Calculate time passed
    int timeInterval = currentTime - previousTime;

    if(timeInterval > 1000)
    {
		//if(frameCount < 1000)		//otherwise the "frameCount" can get really big
		{
			//  calculate the number of frames per second
			fps = frameCount / (timeInterval / 1000.0f);
		//	cout<<"FPS: "<<fps<<"\tframeCount:"<<frameCount<<"\timeInterval:"<<timeInterval<<endl;
		}
		//  Set time
		previousTime = currentTime;
		//  Reset frame count
		frameCount = 0;
    }
}


////////////////////////////////////////////////////////////////////////////
void assignColors() 
{
	for(int i = 0; i < sl_list.size(); i++)
	{
		VECTOR4 v4Color;
		switch((i/2)%7)
		{
		case 0: v4Color = VECTOR4(1.0f, 0.0f, 0.0f, 1.0f);	break;
		case 1: v4Color = VECTOR4(0.0f, 1.0f, 0.0f, 1.0f);	break;
		case 2: v4Color = VECTOR4(0.0f, 0.0f, 1.0f, 1.0f);	break;
		case 3: v4Color = VECTOR4(1.0f, 1.0f, 0.0f, 1.0f);	break;
		case 4: v4Color = VECTOR4(1.0f, 0.0f, 1.0f, 1.0f);	break;
		case 5: v4Color = VECTOR4(0.0f, 1.0f, 1.0f, 1.0f);	break;
		case 6: v4Color = VECTOR4(1.0f, 1.0f, 1.0f, 1.0f);	break;
		}
		liv4Colors.push_back(v4Color);
	}
	cLineRenderer._Update();
}

void readTraceFile(char* fileName)
{
	FILE *fpPathline;
	fpPathline = fopen(fileName, "rb");
	if (fpPathline==NULL)
	{
		cout<<"cannot open file: "<<fileName<<endl;
	}

	assert(fpPathline);
	float pfMin[4];
	float pfMax[4];
	fread(&pfMin, sizeof(pfMin[0]), 4, fpPathline);
	fread(&pfMax, sizeof(pfMax[0]), 4, fpPathline);
	int iNrOfTimeSteps = 1 + (int)ceilf(pfMax[3] - pfMin[3]);
	minLen[0] = pfMin[0];minLen[1] = pfMin[1];minLen[2] = pfMin[2];minLen[3] = pfMin[3];
	maxLen[0] = pfMax[0];maxLen[1] = pfMax[1];maxLen[2] = pfMax[2];maxLen[3] = pfMax[3];

	cLineRenderer.getDeformLine()->SetDomain(pfMin, pfMax);

	size_t uFOffset = ftell(fpPathline);
	int iNrOfPathlines = 0;
	for(int iDelim = 0; iDelim >= 0; iNrOfPathlines++)
		fread(&iDelim, sizeof(iDelim), 1, fpPathline);
	iNrOfPathlines--;

	size_t uDataStart = ftell(fpPathline);

	fseek(fpPathline, uFOffset, SEEK_SET);

	int* piNrsOfCoords;
	piNrsOfCoords = (int*)malloc(iNrOfPathlines*sizeof(int));
	//cout<<"piNrsOfCoords"<<*piNrsOfCoords<<endl;
	fread(piNrsOfCoords, sizeof(piNrsOfCoords[0]), iNrOfPathlines, fpPathline);

	// skip the delim
	fseek(fpPathline, uDataStart, SEEK_SET);

	int iTotalNrOfPoints = 0;
	for(int s = 0; s < iNrOfPathlines; s++)
	{
		int iNrOfPoints = piNrsOfCoords[s];
		if( 0 == iNrOfPoints )
		{
			LOG("Error");
			continue;
		}

		vtListTimeSeedTrace* vtNewListSeedTrace = new vtListTimeSeedTrace;
		vtNewListSeedTrace->clear();
		VECTOR4* pv4Coords;
		pv4Coords = (VECTOR4*) malloc(iNrOfPoints*sizeof(VECTOR4));
		fread(&pv4Coords[0], sizeof(pv4Coords[0]), iNrOfPoints, fpPathline);

		float T;
		float prevT;
		for(int c = 0; c < iNrOfPoints; c++)
		{
			T = pv4Coords[c][3];
			if(c!=0 && prevT == T)
				LOG(printf("WARNING:Two consequence points in the line list have the same value for time step, you should not use TimeLineRendererInOpenGL"));
			vtNewListSeedTrace->push_back(new VECTOR4(pv4Coords[c]));
			prevT = T;
		}
		//cout<<"iNrOfPoints::"<<iNrOfPoints<<endl;

		//Begin-Add-by-Tong
		free(pv4Coords);
		//End
		sl_list.push_back(vtNewListSeedTrace);
		iTotalNrOfPoints += iNrOfPoints;
	}
	fclose(fpPathline);
}

void draw_streamlines() 
{
	glPushAttrib(
		GL_LIGHTING_BIT |
		0
	);

	cLineRenderer._Draw();

	glPopAttrib();
}

void
_SpecialFunc(int skey, int x, int y)
{
	DIRECTION dir;
	switch(skey)
	{
	case GLUT_KEY_LEFT:
		dir = DIRECTION::DIR_LEFT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	case GLUT_KEY_RIGHT:
		dir = DIRECTION::DIR_RIGHT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	case GLUT_KEY_DOWN:
		dir = DIRECTION::DIR_DOWN;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	case GLUT_KEY_UP:
		dir = DIRECTION::DIR_UP;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	case GLUT_KEY_PAGE_DOWN:
		dir = DIRECTION::DIR_IN;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	case GLUT_KEY_PAGE_UP:
		dir = DIRECTION::DIR_OUT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		break;
	}
	//if(cLineRenderer.getDeformLine()->GetSourceMode() == SOURCE_MODE::MODE_DYNAMIC_TRACE)
	//else if(cLineRenderer.getDeformLine()->GetSourceMode() == SOURCE_MODE::MODE_LENS)
	//	cLineRenderer.getDeformLine()->MoveLensCenterOnScreenByKey(dir);
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
void
_KeyboardFunc(unsigned char ubKey, int iX, int iY)
{
//ADD-BY-TONG 02/27/2013-BEGIN
//	GLenum eModifier = glutGetModifiers();
//ADD-BY-TONG 02/27/2013-END
	DIRECTION dir;

	switch(ubKey)
	{
	case 'h':
		{
			int iHalo;
			cLineRenderer._GetInteger(CLineRenderer::ENABLE_HALO, &iHalo);
			iHalo = !iHalo;
			cLineRenderer._SetInteger(CLineRenderer::ENABLE_HALO, iHalo);
		}
		glutPostRedisplay();
		break;

	case 'l':
		{
			int iLighting;
			cLineRenderer._GetInteger(CLineRenderer::ENABLE_LIGHTING, &iLighting);
			iLighting = !iLighting;
			cLineRenderer._SetInteger(CLineRenderer::ENABLE_LIGHTING, iLighting);
		}

		glutPostRedisplay();
		break;
	case '6':
		cLineRenderer.getDeformLine()->SetDeformMode(MODE_AUTO);
		break;
	case '7':
		cLineRenderer.getDeformLine()->SetDeformMode(MODE_HULL);
		cLineRenderer.getDeformLine()->SetAutoDeformMode(false);
		break;
	case '8':
		cLineRenderer.getDeformLine()->SetDeformMode(MODE_ELLIPSE);
		cLineRenderer.getDeformLine()->SetAutoDeformMode(false);
		break;
	case '9':
		cLineRenderer.getDeformLine()->SetDeformMode(MODE_LINE);
		cLineRenderer.getDeformLine()->SetAutoDeformMode(false);
		break;
	case 't':
		cLineRenderer.getDeformLine()->RedoDeformation();
		break;
	case 'i':
		cLineRenderer.SetNewCutLine(true);
		break;
	case 'o':
		cLineRenderer.getDeformLine()->SetSourceMode(SOURCE_MODE::MODE_LOCATION);
		break;
	case 'p':
		cLineRenderer.getDeformLine()->SetSourceMode(SOURCE_MODE::MODE_LENS);
		break;
	case '+':
		cLineRenderer.getDeformLine()->ChangeLensAngle(1);
		glutPostRedisplay();
		break;
	case '-':
		cLineRenderer.getDeformLine()->ChangeLensAngle(-1);
		glutPostRedisplay();
		break;
	case 'a':
		dir = DIRECTION::DIR_LEFT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;
	case 'd':
		dir = DIRECTION::DIR_RIGHT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;
	case 'w':
		dir = DIRECTION::DIR_UP;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;
	case 's':
		dir = DIRECTION::DIR_DOWN;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;
	case 'q':
		dir = DIRECTION::DIR_IN;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;
	case 'e':
		dir = DIRECTION::DIR_OUT;
		cLineRenderer.getDeformLine()->MovePickBlock(dir);
		glutPostRedisplay();
		break;

	//case '[':
	//	cLineRenderer.getDeformLine()->ChangeLensRatio(1);
	//	glutPostRedisplay();
	//	break;
	//case ']':
	//	cLineRenderer.getDeformLine()->ChangeLensRatio(-1);
	//	glutPostRedisplay();
	//	break;
	}
}


//ADD-BY-TONG 02/13/2013-BEGIN

void _ReshapeFunc(int w, int h)
{
//	cLineRenderer._SetInteger(CLineRendererInOpenGLDeform::WIN_WIDTH,w);
//	cLineRenderer._SetInteger(CLineRendererInOpenGLDeform::WIN_HEIGHT,h);
	cLineRenderer.getDeformLine()->SetWinSize(w, h);
	cLineRenderer.reshape();
	//cLineRenderer.postRedrawPickingColor();
}

void _MouseFunc(int button, int state, int x, int y)
{
	if(button==GLUT_RIGHT_BUTTON)
	{
		if(cLineRenderer.GetNewCutLine() == true)
		{
			if(state==GLUT_DOWN)
				_dragStartPosLine = VECTOR2(x, y);
			else if(state==GLUT_UP)
			{
				if(cLineRenderer.GetNewCutLine())
					cLineRenderer.CutLineFinish();
				cLineRenderer.SetNewCutLine(false);
			}
		}
		else
		{
			if(state==GLUT_DOWN)
			{
				if(cLineRenderer.getDeformLine()->InsideFirstEllipse(x, y))
				{
					_dragLens = true;
					_dragStartPos = VECTOR2(x, y);
				}
				if(cLineRenderer.getDeformLine()->OnEllipseEndPoint(x, y))
				{
					_dragLensEndPt = true;
				}
				cLineRenderer.SetDeformOn(false);
			}
			else if(state==GLUT_UP)
			{
				_dragLens = false;
				_dragLensEndPt = false;
				cLineRenderer.SetDeformOn(true);
				cLineRenderer.getDeformLine()->RedoDeformation();
			}
		}
		//	//cLineRenderer.setDragging(true);
		////	cLineRenderer._getDragSet(x, y);
		//}
		//else if(state==GLUT_UP)
		//	cLineRenderer.setDragging(false);
	}
	else if(button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		cLineRenderer.getDeformLine()->UpdateLensScreen();
	}
	//redraw the picking space only when mouse release
	//there are two cases for mouse release: 
	//1. left mouse release for finishing rotation or transformation; 
	//2. right mouse release for finishing dragging streamlines
	//if(state==GLUT_UP)
		//cLineRenderer.postRedrawPickingColor();
}

//ADD-BY-TONG 03/04/2013-BEGIN
void _IdleFunc()
{
	//cLineRenderer.getDeformLine()->RunCuda();
	calculateFPS();

}

void _TimerFunc(int value)
{
	//only refresh the deformation window, but not the picking window
	glutSetWindow(deformWindowId);
	glutPostRedisplay();
	glutTimerFunc(REFRESH_DELAY, _TimerFunc,0);
}
//ADD-BY-TONG 03/04/2013-END


void _MotionFunc(int x, int y)
{

	if(_dragLens)
		cLineRenderer.getDeformLine()->MoveLensCenterOnScreen(x - _dragStartPos[0], y - _dragStartPos[1]);
	
	if(_dragLensEndPt)
		cLineRenderer.getDeformLine()->MoveLensEndPtOnScreen(x, y);

	_dragStartPos = VECTOR2(x, y);

	if(cLineRenderer.GetNewCutLine() == true)
		cLineRenderer.SetCutLineCoords(_dragStartPosLine ,_dragStartPos);

	//if(button==GLUT_RIGHT_BUTTON)
	//cLineRenderer.dragTo(x, y);
}

void _PassiveMotionFunc(int x, int y)
{
	cLineRenderer._PassiveMotion(x, y);
	//cLineRenderer._getDragSet(x, y);
}

void _MouseWheelFunc( int wheel, int direction, int x, int y )
{
	//if(direction < 0)
	//	cLineRenderer.resizePickWin(true);
	//else
	//	cLineRenderer.resizePickWin(false);
	//cLineRenderer._getDragSet(x, y);
	if(direction > 0)
		cLineRenderer.getDeformLine()->ChangeLensDepth(1);
	else
		cLineRenderer.getDeformLine()->ChangeLensDepth(-1);
	
	glutPostRedisplay();
}

//-------------------------------------------------------------------------
//  Draws a string at the specified coordinates.
//-------------------------------------------------------------------------
GLvoid *font_style = GLUT_BITMAP_TIMES_ROMAN_24;
void printw (float x, float y, float z, char* format, ...)
{
	va_list args;	//  Variable argument list
	int len;		//	String length
	int i;			//  Iterator
	char * text;	//	Text

	//  Initialize a variable argument list
	va_start(args, format);

	//  Return the number of characters in the string referenced the list of arguments.
	//  _vscprintf doesn't count terminating '\0' (that's why +1)
	len = _vscprintf(format, args) + 1; 

	//  Allocate memory for a string of the specified size
	text = (char *)malloc(len * sizeof(char));

	//  Write formatted output using a pointer to the list of arguments
	vsprintf_s(text, len, format, args);

	//  End using variable argument list 
	va_end(args);

	//  Specify the raster position for pixel operations.
	glRasterPos3f (x, y, z);

	//  Draw the characters one by one
    for (i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(font_style, text[i]);

	//  Free the allocated memory for the string
	free(text);
}

//-------------------------------------------------------------------------
//  Draw FPS
//-------------------------------------------------------------------------
void drawFPS()
{
    //  Load the identity matrix so that FPS string being drawn
    //  won't get animates
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();


	//  Print the FPS to the window
	printw (-0.95, -0.95, 0, "FPS: %4.2f", fps);
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

//ADD-BY-TONG 02/13/2013-END

void
_DisplayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// render the scene
    draw_streamlines(); 
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	drawFPS();

	// NOTE: Call glutSwapBuffers() at the end of your display function
	glutSwapBuffers();
}



void
init()
{
	LOG(printf("Initialize here."));
	glEnable(GL_DEPTH_TEST);

	// setup light 0
	static GLfloat pfLightAmbient[4] =	{0.0f, 0.0f, 0.0f, 1.0f};
	static GLfloat pfLightColor[4] =	{0.5f, 0.5f, 0.5f, 1.0f};
	glLightfv(GL_LIGHT0, GL_AMBIENT,		pfLightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE,		pfLightColor);
	glLightfv(GL_LIGHT0, GL_SPECULAR,		pfLightColor);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT,	4.0f);
	cLineRenderer._UpdateLighting();
}

void 
quit()
{
	LOG(printf("Clean up here."));
}

int
main(int argc, char* argv[])
{
	///////////////////////////////////////////////////////////////
	// when use GCB, it is still needed to initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA | GLUT_STENCIL );

	// load the scalar field
	LOG(printf("read file %s\n", argv[1]));

	//ADD-BY-TONG 02/19/2013-BEGIN
//	if(argc <= 1)
		readTraceFile(FILENAME);
//	else
	//ADD-BY-TONG 02/19/2013-END	
	//	readTraceFile(argv[1]);
//	assignColors();

//	szVecFilePath = argv[1];

	// comptue the bounding box of the streamlines 
	VECTOR3 minB, maxB; 
	minB[0] = minLen[0]; minB[1] = minLen[1];  minB[2] = minLen[2];
	maxB[0] = maxLen[0]; maxB[1] = maxLen[1];  maxB[2] = maxLen[2];
	printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n", 
								minLen[0], maxLen[0], minLen[1], maxLen[1], 
								minLen[2], maxLen[2]); 

	center[0] = (minLen[0]+maxLen[0])/2.0; 
	center[1] = (minLen[1]+maxLen[1])/2.0; 
	center[2] = (minLen[2]+maxLen[2])/2.0; 
	printf("center is at %f %f %f \n", center[0], center[1], center[2]); 
	len[0] = maxLen[0]-minLen[0]; 
	len[1] = maxLen[1]-minLen[1]; 
	len[2] = maxLen[2]-minLen[2]; 

	///////////////////////////////////////////////////////////////
	cLineRenderer._SetBoundingBox(
		minLen[0], minLen[1], minLen[2], 
		maxLen[0], maxLen[1], maxLen[2]);
	cLineRenderer._SetDataSource(&sl_list);
	cLineRenderer._SetColorSource(&liv4Colors);
	cLineRenderer._SetInteger(CLineRenderer::COLOR_SCHEME, CLineRenderer::CColorScheme::COLOR_ALL_WHITE);
//	cLineRenderer.getDeformLine()->SetVectorFieldFilename(FILENAME_VEC);

	///////////////////////////////////////////////////////////////
//	glutInitWindowPosition(0, 100);
	glutInitWindowSize(800, 800);
	deformWindowId = glutCreateWindow("Streamline Deformation Visualization");

	// specify the callbacks you really need. Except gcbInit() and gcbDisplayFunc(), other callbacks are optional
	gcbInit(init, quit);
	gcbDisplayFunc(_DisplayFunc);
	gcbKeyboardFunc(_KeyboardFunc);
	//ADD-BY-TONG 02/12/2013-BEGIN
	gcbSpecialFunc(_SpecialFunc);
	gcbReshapeFunc(_ReshapeFunc);
	gcbMouseFunc(_MouseFunc);
	gcbMotionFunc(_MotionFunc);
	gcbPassiveMotionFunc(_PassiveMotionFunc);
	gcbMouseWheelFunc(_MouseWheelFunc);
	gcbIdleFunc(_IdleFunc);
	gcbTimerFunc(_TimerFunc);
	//ADD-BY-TONG 02/12/2013-END
	cLineRenderer._SetFloat(CTimeLineRendererInOpenGLDeform::MIN_TIME_STEP,0.0f);
	cLineRenderer._SetFloat(CTimeLineRendererInOpenGLDeform::MAX_TIME_STEP,1.0f);
	cLineRenderer._SetInteger(CLineRenderer::ENABLE_HALO, true);
	if(argc >= 2)
	{
		cLineRenderer.SetPara((float)atof(argv[1]));
	}
	else
		cLineRenderer.SetPara(0.01);
	cLineRenderer._Update();

	fps = 0;
	frameCount = 0;
	_dragLens = false;
	_dragLensEndPt = false;
	// enter the GLUT loop

	///////////////////
	glutMainLoop();
	//ADD-BY-TONG-BEGIN
	//return app.exec();
	//ADD-BY-TONG-END

	return 0;
}

/*

[02/10/2012]
1. [1ST] First Time Checkin.

*/
