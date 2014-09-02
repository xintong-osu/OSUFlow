
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

//#define FILENAME_BUNDLE "D:/Dropbox/streamline/interactive_osuflow/build/renderer/examples/Release/data/15plume3d440_seed300_len500_norm.bdl"
//#define FILENAME_BUNDLE "data/15plume3d440_seed300_len500_norm.bdl"
//#define FILENAME_BUNDLE "data/15plume3d440_seed500_len100_norm.bdl"
//#define FILENAME_BUNDLE "data/15plume3d440_seed500_len250_norm.bdl"
//#define FILENAME_DIST "D:\\Dropbox\\streamline\\interactive_osuflow\\build\\renderer\\examples\\Release\\data\\15plume3d440_seed500_len250_norm_haus.dist"
//#define FILENAME_DIST "D:\\Dropbox\\streamline\\interactive_osuflow\\build\\renderer\\examples\\Release\\data\\15plume3d440_seed500_len250_norm_curvature_torsion.dist"
//#define FILENAME_DIST "D:\\data\\plume\\15plume3d440_seed200_len200_fix_seg.dist"
//#define FILENAME_DIST "D:\\data\\isabel\\UVWf01_step500_seed500_seg.dist"
#if 0
static char* FILENAME_DIST = "data\\UVWf01_step500_seed500_seg.dist";
#elif 0
static char* FILENAME_DIST = "D:\\data\\nek\\nek.d_2_seg_emd.dist";
#else
static char* FILENAME_DIST = "deform_data\\plume\\15plume3d421_seg.dist";
#endif

//#define FILENAME_VEC "D:\\Dropbox\\data\\UVWf01.vec"
//#define FILENAME "data/15plume3d440_seed500_len250_norm.vec.trace"
int frameCount;
int currentTime;
int previousTime;
float fps;

#include <list>
#include "gcb2.h"
#include "OSUFlow.h"
#include "TimeLineRendererInOpenGLDeform.h"
#include "StreamDeform.h"
#include "GL\glui.h"

OSUFlow *osuflow; 
VECTOR4 minLen, maxLen; 
list<vtListTimeSeedTrace*> sl_list; 
CTimeLineRendererInOpenGLDeform cLineRenderer;

int deformWindowId;		//for timer function
//bool _dragLens;
//bool _dragLensEndPt;
VECTOR2 _dragStartPos;
VECTOR2 _dragPrevPos;
//VECTOR2 _dragCurrentPosLine;
int method_type = 0;
int visualMode_type = 0;
int shapeModelRadioOption = 0;
int _showCubeGCB = 1;

set<int> _deviceId;
map<int, vector<VECTOR2>> _touchCoords;

bool firstTime = true;

enum
{
	REDO_DEFORM_ID,
	CUT_LINE_ID,
	DRAW_ELLIPSE_ID,
	X_INCREASE_ID,
	X_DECREASE_ID,
	Y_INCREASE_ID,
	Y_DECREASE_ID,
	Z_INCREASE_ID,
	Z_DECREASE_ID,
	METHOD_TYPE_ID,
	SHAPE_MODEL_ID,
	SHOW_CUBE_ID,
	VISUALIZATION_MODE_ID,
	//LENS_ID,
	//LOCATION_ID,
};


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
	glutPostRedisplay();
}

///////////////////////////////////////////////////////////////////////////////
void
_KeyboardFunc(unsigned char ubKey, int iX, int iY)
{
	DIRECTION dir;

	switch(ubKey)
	{
	case '+':
		cLineRenderer.getDeformLine()->ChangeLensChangeStep(4);
		break;
	case '-':
		cLineRenderer.getDeformLine()->ChangeLensChangeStep(0.25);
		break;
	case 'g':
		cLineRenderer.getDeformLine()->SetDeformOn(false);
		glutPostRedisplay();
		break;
	}
}


//ADD-BY-TONG 02/13/2013-BEGIN

void _ReshapeFunc(int w, int h)
{
	cLineRenderer.getDeformLine()->SetWinSize(w, h);
	cLineRenderer.reshape();
	//cLineRenderer.postRedrawPickingColor();
}


void _MotionFunc(int x, int y)
{
	if(_touchCoords.size() > 1)		//multitouch
		return;

	switch(cLineRenderer.getDeformLine()->GetInteractMode())
	{
	case INTERACT_MODE::MOVE_LENS:
		cLineRenderer.getDeformLine()->MoveLensCenterOnScreen(x - _dragPrevPos[0], y - _dragPrevPos[1]);
		break;
	case INTERACT_MODE::DRAG_LENS_EDGE:
		cLineRenderer.getDeformLine()->MoveLensEndPtOnScreen(x, y);
		break;
	case INTERACT_MODE::CUT_LINE:
		cLineRenderer.SetCutLineCoords(VECTOR2(x, y), _dragStartPos);
		break;
	case INTERACT_MODE::DRAW_ELLIPSE:
		cLineRenderer.getDeformLine()->AddDrawPoints(x, y);
		break;
	}
	_dragPrevPos = VECTOR2(x, y);
}

void _MouseFunc(int button, int state, int x, int y)
{
	//if(button==GLUT_RIGHT_BUTTON)
	//{
	if(cLineRenderer.getDeformLine()->GetInteractMode() == INTERACT_MODE::CUT_LINE)
	{
		if(state==GLUT_DOWN)
		{
			_dragStartPos = VECTOR2(x, y);
		}
		else if(state==GLUT_UP)
		{
			//if(cLineRenderer.GetNewCutLine())
			cLineRenderer.CutLineFinish();
			SetDisableTransformation(false);
		}
	}
	else if(cLineRenderer.getDeformLine()->GetInteractMode() == INTERACT_MODE::DRAW_ELLIPSE)
	{
		if(state==GLUT_UP)
		{
			cLineRenderer.getDeformLine()->DrawEllipse(false);
			SetDisableTransformation(false);
		}
	}
	else if(cLineRenderer.getDeformLine()->GetSourceMode() == SOURCE_MODE::MODE_LENS)
	{
		if(state==GLUT_DOWN)
		{
			_dragStartPos = VECTOR2(x, y);
			cLineRenderer.SetDeformOn(false);

			if(cLineRenderer.getDeformLine()->InsideFirstEllipse(x, y))
			{
				SetDisableTransformation(true);
				if(cLineRenderer.getDeformLine()->AroundLensCenter(x, y))
				{
					cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::MOVE_LENS);
				}
				else if(cLineRenderer.getDeformLine()->OnEllipseEndPoint(x, y))
				{
					cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::DRAG_LENS_EDGE);
					//SetDisableTransformation(true);
				}
				//else
				//	cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::TRANSFORMATION);
			}
			else
				SetDisableTransformation(false);

		}
		else if(state == GLUT_UP)
		{
			cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::TRANSFORMATION);
			cLineRenderer.SetDeformOn(true);
			SetDisableTransformation(false);
			cLineRenderer.getDeformLine()->UpdateLensScreen();
			//cLineRenderer.getDeformLine()->RestoreStreamConnectivity();
			//if(cLineRenderer.getDeformLine()->GetDeformMode() == DEFORM_MODE::MODE_LINE)
			cLineRenderer.getDeformLine()->ProcessAllStream();
			//else
			//	cLineRenderer.getDeformLine()->ProcessAllStream();
		}

	}
				//{
	_dragPrevPos = VECTOR2(x, y);

				//}
				//else if(cLineRenderer.getDeformLine()->GetSourceMode() == SOURCE_MODE::MODE_LENS 
				//	&& )
				//{
				//	
				//	//_dragStartPos = VECTOR2(x, y);
				//}
				//else
					//SetDisableTransformation(false);
			//}
			//else if(state==GLUT_UP)
			//{
				
			//}
		//}
	//}
	//if(/*button == GLUT_LEFT_BUTTON && */state == GLUT_UP)
	//{
	//	cLineRenderer.getDeformLine()->UpdateLensScreen();
	//}
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

void _MultiMotionFunc(int id, int x, int y, float g, map<int, vector<VECTOR2>> touchCoords)
{
	vector<VECTOR2> fingerPos;
	vector<int> fingerCnts;
	for (map<int, vector<VECTOR2>>::iterator it=touchCoords.begin(); it!=touchCoords.end(); ++it)
	{
		if(0 == it->second.size() )	//because it can be e
			return;
		fingerPos.push_back(it->second.back());
		fingerCnts.push_back(it->second.size());
	}

	if(2 == touchCoords.size())
	{
		VECTOR2 finger1 = fingerPos[0];//touchCoords.begin()->second.back();
		VECTOR2 finger2 = fingerPos[1];//touchCoords.back().second.back();
		
		if(INTERACT_MODE::CUT_LINE != cLineRenderer.getDeformLine()->GetInteractMode())		//the first points
		{
			if(cLineRenderer.getDeformLine()->OnEllipseTwoEndPoints(finger1[0], finger1[1], finger2[0], finger2[1]))
			{
				SetDisableTransformation(true);
				cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::DRAG_LENS_TWO_ENDS);
			}
			firstTime = false;
		}

		//	cout<<"id:\t"<<id<<endl;
		switch(cLineRenderer.getDeformLine()->GetInteractMode())
		{
		case INTERACT_MODE::CUT_LINE:
			cLineRenderer.SetCutLineCoords(finger1, finger2);
			break;
		case INTERACT_MODE::DRAG_LENS_TWO_ENDS:
			cLineRenderer.getDeformLine()->MoveLensTwoEndPtOnScreen(finger1[0], finger1[1], finger2[0], finger2[1]);
			break;
		default:
			{
			//	cout<<"g:"<<g<<endl;
				if(abs(g) > 0.01)
					cLineRenderer.getDeformLine()->ChangeLensDepth(g * 0.1);
				break;
			}
		}
	}
	else
		firstTime = true;

	_touchCoords = touchCoords;
//	cout<<"id:\t"<<id<<endl;
//	_touchCoords.insert(id, );
	//SetTouchStatus(_touchCoords.size() > 1 );
	//put a timer here to make it faster;
	//switch(g)
	//{
	//case GESTURE::SPAN:
	//	cout<<"increase depth..."<<endl;
	//	cLineRenderer.getDeformLine()->ChangeLensDepth(1);
	//	break;
	//case GESTURE::SQUEEZE:
	//	cout<<"decrease depth..."<<endl;
	//	cLineRenderer.getDeformLine()->ChangeLensDepth(-1);
	//	break;
	//case GESTURE::NO_GESTURE:
	//	break;
	//}


	//
	//if(cLineRenderer.getDeformLine()->GetInteractMode() == INTERACT_MODE::DRAG_LENS_TWO_ENDS)
	//{
	//	VECTOR2 finger1 = touchCoords.begin()->second.back();
	//	VECTOR2 finger2 = touchCoords.end()->second.back();
	//	//if(cLineRenderer.getDeformLine()->OnEllipseTwoEndPoints(finger1[0], finger1[1], finger2[0], finger2[1]))
	//}


	glutPostRedisplay();
	
	//if(_touchCoords.end() == _touchCoords.find(id))	{
	//	vector<VECTOR2> pts;
	//	pts.push_back(VECTOR2(x, y));
	//	_touchCoords.insert(std::pair<int, vector<VECTOR2>>(id, pts));
	//} else {
	//	_touchCoords.find(id)->second.push_back(VECTOR2(x, y));
	//	if(_touchCoords.size() > 1 && ( 0 == _touchCoords.find(id)->second.size() % 16))	{
	//		map<int, vector<VECTOR2>>::iterator it = _touchCoords.begin();
	//		map<int, vector<VECTOR2>>::iterator it2 = it;
	//		it2++;
	//		int iN = 8;
	//		if(it->second.size() >= iN && it2->second.size() >= iN)
	//		{
	//			float distLast = (it->second.back() - it2->second.back()).GetMag();
	//			float distEarlier = (it->second.at(it->second.size() - iN) - it2->second.at(it2->second.size() - iN)).GetMag();
	//			float diff = distLast - distEarlier;
	//			if( diff > 2)
	//				cLineRenderer.getDeformLine()->ChangeLensDepth(1);
	//			else if(diff < -2)
	//				cLineRenderer.getDeformLine()->ChangeLensDepth(-1);
	//		}
	//	}
	//}
}

void _MultiEntryFunc(int id, int mode, map<int, vector<VECTOR2>> touchCoords)
{
	_touchCoords = touchCoords;
//	cout<<"touchCoords.size():"<<touchCoords.size()<<endl;

	//if(2 == touchCoords.size() )
	//	cLineRenderer.getDeformLine()->SetInteractMode(INTERACT_MODE::);
	//if(mode == GLUT_ENTERED)
	//{
	//	_deviceId.insert(id);
	//	//cout<<"entered"<<endl;
	//}
	//else if(mode == GLUT_LEFT)
	//{
	//	//cout<<"left"<<endl;
	//	_deviceId.erase(id);
	//	_touchCoords.erase(id);
	//}
	//for(set<int>::iterator it=_deviceId.begin(); it!=_deviceId.end(); ++it)
	//	std::cout << ' ' << *it;
	//cout<<endl;
}



void _PassiveMotionFunc(int x, int y)
{
	if(_touchCoords.size() > 1)		//multitouch
		return;
	cLineRenderer._PassiveMotion(x, y);
	//cLineRenderer._getDragSet(x, y);
}

void _MouseWheelFunc( int wheel, int direction, int x, int y )
{
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
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
#if (TEST_PERFORMANCE == 0)
	drawFPS();
#endif

	// NOTE: Call glutSwapBuffers() at the end of your display function
	glutSwapBuffers();
}



void
init()
{
	LOG(printf("Initialize here."));
	glEnable(GL_DEPTH_TEST);
}

void 
quit()
{
	LOG(printf("Clean up here."));
}

void control_cb( int control )
{
	DIRECTION dir;
	
	switch (control)
	{
	case REDO_DEFORM_ID:
		{
			cLineRenderer.getDeformLine()->RedoDeformation();
			break;
		}
	case CUT_LINE_ID:
		{
		//	cLineRenderer.getDeformLine()->SetLensAxis(VECTOR2(0, 0), VECTOR2(0, 0));
			cLineRenderer.SetNewCutLine(true);
			cLineRenderer.getDeformLine()->RedoDeformation();
			SetDisableTransformation(true);
			break;
		}
	case DRAW_ELLIPSE_ID:
		{
			cLineRenderer.getDeformLine()->DrawEllipse(true);
			cLineRenderer.getDeformLine()->RedoDeformation();
			SetDisableTransformation(true);
		}
	case X_INCREASE_ID:
		{
			dir = DIRECTION::DIR_RIGHT;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case X_DECREASE_ID:
		{
			dir = DIRECTION::DIR_LEFT;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case Y_INCREASE_ID:
		{
			dir = DIRECTION::DIR_UP;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case Y_DECREASE_ID:
		{
			dir = DIRECTION::DIR_DOWN;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case Z_INCREASE_ID:
		{
			dir = DIRECTION::DIR_OUT;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case Z_DECREASE_ID:
		{
			dir = DIRECTION::DIR_IN;
			cLineRenderer.getDeformLine()->MovePickBlock(dir);
			glutPostRedisplay();
			break;
		}
	case METHOD_TYPE_ID:
		{
			switch(method_type)
			{
			case 0:
				cLineRenderer.getDeformLine()->SetSourceMode(SOURCE_MODE::MODE_BUNDLE);
				break;
			case 1:
				cLineRenderer.getDeformLine()->SetSourceMode(SOURCE_MODE::MODE_LENS);
				break;
			case 2:
				cLineRenderer.getDeformLine()->SetSourceMode(SOURCE_MODE::MODE_LOCATION);
				break;
			}
			break;
		}
	case SHAPE_MODEL_ID:
		{
			switch(shapeModelRadioOption)
			{
			case 0:
				cLineRenderer.getDeformLine()->SetDeformMode(MODE_ELLIPSE);
				cLineRenderer.getDeformLine()->SetAutoDeformMode(false);

				break;
			case 1:
				cLineRenderer.getDeformLine()->SetDeformMode(MODE_LINE);
				cLineRenderer.getDeformLine()->SetAutoDeformMode(false);

				break;
			case 2:
				cLineRenderer.getDeformLine()->SetDeformMode(MODE_HULL);
				cLineRenderer.getDeformLine()->SetAutoDeformMode(false);

				break;
			case 3:
				cLineRenderer.getDeformLine()->SetAutoDeformMode(true);
				break;
			}

		}
	case SHOW_CUBE_ID:
		{
			cLineRenderer.SetShowCube(_showCubeGCB);
			break;
		}
	case VISUALIZATION_MODE_ID:
		{
			switch(visualMode_type)
			{
			case 0:
				cLineRenderer.getDeformLine()->SetVisualMode(VISUAL_MODE::DEFORM);
				break;
			case 1:
				cLineRenderer.getDeformLine()->SetVisualMode(VISUAL_MODE::TRANSP);
				break;
			}
			break;
		}
	}


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

	if(argc > 1)
	{
		readTraceFile(argv[1]);
		cLineRenderer.getDeformLine()->SetDistFileName(argv[2]);
	}
	else
	{
		readTraceFile(FILENAME);
		cLineRenderer.getDeformLine()->SetDistFileName(FILENAME_DIST);
	}

	// comptue the bounding box of the streamlines 
	VECTOR3 minB, maxB; 
	minB[0] = minLen[0]; minB[1] = minLen[1];  minB[2] = minLen[2];
	maxB[0] = maxLen[0]; maxB[1] = maxLen[1];  maxB[2] = maxLen[2];
	printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n", 
								minLen[0], maxLen[0], minLen[1], maxLen[1], 
								minLen[2], maxLen[2]); 

	///////////////////////////////////////////////////////////////
	cLineRenderer._SetBoundingBox(
		minLen[0], minLen[1], minLen[2], 
		maxLen[0], maxLen[1], maxLen[2]);
	cLineRenderer._SetDataSource(&sl_list);
	cLineRenderer._SetInteger(CLineRenderer::COLOR_SCHEME, CLineRenderer::CColorScheme::COLOR_ALL_WHITE);
//	cLineRenderer.getDeformLine()->SetVectorFieldFilename(FILENAME_VEC);

	///////////////////////////////////////////////////////////////
	glutInitWindowPosition(200, 150);
	glutInitWindowSize(900, 770);
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
	gcbMultiMotionFunc(_MultiMotionFunc);
	gcbMultiEntryFunc(_MultiEntryFunc);
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
	//_dragLens = false;
	//_dragLensEndPt = false;

	//////////GLUI
	GLUI *glui = GLUI_Master.create_glui_subwindow(deformWindowId, GLUI_SUBWINDOW_TOP);

	new GLUI_Button(glui, "Redo Deform", REDO_DEFORM_ID, control_cb );
	
	GLUI_Panel *visualMode_panel = new GLUI_Panel( glui, "Visualization Mode" );
	GLUI_RadioGroup  *radioVisualMode = new GLUI_RadioGroup(visualMode_panel, &visualMode_type, VISUALIZATION_MODE_ID, control_cb);;
	new GLUI_RadioButton( radioVisualMode, "Deformation" );
	new GLUI_RadioButton( radioVisualMode, "Transparency" );
	
	new GLUI_Column( glui, false );
	GLUI_Panel *method_panel = new GLUI_Panel( glui, "Method" );
	GLUI_RadioGroup  *radio = new GLUI_RadioGroup(method_panel, &method_type, METHOD_TYPE_ID, control_cb);;
	new GLUI_RadioButton( radio, "Bundle" );
	new GLUI_RadioButton( radio, "Lens" );
	new GLUI_RadioButton( radio, "Location" );

	new GLUI_Column( glui, false );
	GLUI_Panel *drawLens_panel = new GLUI_Panel( glui, "Draw Lens" );
	new GLUI_Button(drawLens_panel, "Draw Line", CUT_LINE_ID, control_cb );
	new GLUI_Button(drawLens_panel, "Draw Ellipse", DRAW_ELLIPSE_ID, control_cb );

	new GLUI_Column( glui, false );
	GLUI_Panel *shapeModel_panel = new GLUI_Panel( glui, "Focus Region Shape" );
	GLUI_RadioGroup  *radioShapeModel = new GLUI_RadioGroup(shapeModel_panel, &shapeModelRadioOption, SHAPE_MODEL_ID, control_cb);;
	new GLUI_RadioButton( radioShapeModel, "Ellipse" );
	new GLUI_RadioButton( radioShapeModel, "Line" );
	new GLUI_RadioButton( radioShapeModel, "Hull" );
	new GLUI_RadioButton( radioShapeModel, "Auto" );

	new GLUI_Column( glui, false );
	GLUI_Panel *location_panel = new GLUI_Panel( glui, "Cube Location" );
	new GLUI_Button(location_panel, "X --", X_DECREASE_ID, control_cb );
	new GLUI_Button(location_panel, "Y --", Y_DECREASE_ID, control_cb );
	new GLUI_Button(location_panel, "Z --", Z_DECREASE_ID, control_cb );
	new GLUI_Column( location_panel, false );
	new GLUI_Button(location_panel, "X ++", X_INCREASE_ID, control_cb );
	new GLUI_Button(location_panel, "Y ++", Y_INCREASE_ID, control_cb );
	new GLUI_Button(location_panel, "Z ++", Z_INCREASE_ID, control_cb );

	new GLUI_Column( glui, false );
	GLUI_Panel *options_panel = new GLUI_Panel( glui, "Options" );
	new GLUI_Checkbox( options_panel, "Show Cube", &_showCubeGCB, SHOW_CUBE_ID, control_cb );


	///////////////////
	glutMainLoop();
	return 0;
}

/*

[02/10/2012]
1. [1ST] First Time Checkin.

*/
