#include <GL\glew.h>
#include "PickPanel.h"
#include "assert.h"
#include <iostream>

map< int , PickPanel*> PickPanel::instanceMap;

void PickPanel::MouseButton(int button, int state, int x, int y)
{
	glutSetWindow(_panelWinId);
	_bButton1Down = (state == GLUT_DOWN) ? true : false;
	if (button == GLUT_LEFT_BUTTON )
	{
		_mouseDownPos = VECTOR2(x, y);
	}
	else if(button == GLUT_RIGHT_BUTTON )
	{
		if(_bButton1Down)
		{
			//int w = WinId2PanelIdMap[glutGetWindow()];
			GLenum eModifier = glutGetModifiers();
			if(eModifier == GLUT_ACTIVE_SHIFT)
			{
				//cout<<"1111111111111111111111111111"<<endl;
				//exit(0);
				_deformWin->AddRemoveBundle(_panelId);
			}
			else
			{
				//cout<<"2222222222222222222222222"<<endl;
				_deformWin->PickBundle(_panelId);
			}
		}
	}

}

void PickPanel::MouseMotion(int x, int y)
{
	// If button1 pressed, zoom in/out if mouse is moved up/down.
	if (_bButton1Down)
	{
		int dx = x - _mouseDownPos[0];
		int dy = y - _mouseDownPos[1];
		xRot = xRot + dy;
		yRot = yRot + dx;
		glutPostRedisplay();
	}
}

void PickPanel::Draw() 
{
	glutSetWindow(_panelWinId);
	// Clear Screen and Depth Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	// int i;

	//for (i = 0; i < 6; i++) {
	//  glBegin(GL_QUADS);
	//  glNormal3fv(&n[i][0]);
	//  glVertex3fv(&v[faces[i][0]][0]);
	//  glVertex3fv(&v[faces[i][1]][0]);
	//  glVertex3fv(&v[faces[i][2]][0]);
	//  glVertex3fv(&v[faces[i][3]][0]);
	//  glEnd();
	//}
	//cout<<"_coordsMid:"<<-_coordsMid[0]<<","<<_coordsMid[1] <<","<<_coordsMid[2]<<endl;

	glTranslatef(_coordsMid[0], _coordsMid[1], _coordsMid[2]);
	glRotatef(xRot / 4.0, 1.0, 0.0, 0.0);
	if(_longerRange == 2)
		glRotatef(yRot / 4.0 + 90, 0.0, 1.0, 0.0);
	else
		glRotatef(yRot / 4.0, 0.0, 1.0, 0.0);

	glRotatef(zRot / 4.0, 0.0, 0.0, 1.0);
	glTranslatef(-_coordsMid[0], -_coordsMid[1], -_coordsMid[2]);


	// Define a viewing transformation
	glBindBuffer(GL_ARRAY_BUFFER, _vbo_pfCoords);
	glVertexPointer(4, GL_FLOAT, 0,  (char *) NULL);

	glBindBuffer(GL_ARRAY_BUFFER, _vbo_tangent);
	glNormalPointer(GL_FLOAT, 0, NULL);

	//	cout<<"_vaoLines:"<<_vaoLines<<endl;
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	//glDisable(GL_DEPTH_TEST);
	glLineWidth(8);
	glColor3f(0.0f, 0.0f, 0.0f);
	for(int il = 0; il < _bundle->size(); il++)
		glDrawArrays(GL_LINE_STRIP, (*_pviGlPrimitiveBases)[(*_bundle)[il]], 
		(*_pviGlPrimitiveLengths)[(*_bundle)[il]]);

	//glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
	glLineWidth(4);
	glColor3f(0.8f, 0.8f, 0.8f);
	//cout<<_vbo_pfCoords<<","<<_vbo_tangent<<endl;
	//VBOs are still shareable, so you just have to create a VAO for each context that binds the shared VBO.
	for(int il = 0; il < _bundle->size(); il++)
		glDrawArrays(GL_LINE_STRIP, (*_pviGlPrimitiveBases)[(*_bundle)[il]], 
		(*_pviGlPrimitiveLengths)[(*_bundle)[il]]);
	//cout<<"bases,length:"<<endl;
	//cout<< (*_pviGlPrimitiveBases)[(*_bundle)[il]]<<","<< 
	//	(*_pviGlPrimitiveLengths)[(*_bundle)[il]];
	/*
	glBegin(GL_TRIANGLES);
	glVertex3f(200,100,0);
	glVertex3f(220,80,0);
	glVertex3f(140,120,0);
	glEnd();*/
	//cout<<"**2"<<endl;
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//	glBindVertexArray(0);
	//	cout<<"render once!"<<endl;


	glutSwapBuffers();
}

//
//void _UpdateLighting()
//{
//	// setup the texture for the lighting
//	const int iTexWidth = 256;
//	const int iTexHeight = 256;
//	float *pfTex;
//	pfTex = (float*)calloc(4 * iTexWidth * iTexHeight, sizeof(pfTex[0]));
//
//	/*
//	const float fKa = 0.1f;
//	const float fKd = 0.6f;
//	const float fKs = 0.3f;
//	const float fN = 4.0f;
//	*/
//	float pfAmbient[4];
//	float pfDiffuse[4];
//	float pfSpecular[4];
//	float fSpotExponent;
//	glGetLightfv(GL_LIGHT0, GL_AMBIENT, pfAmbient);
//	glGetLightfv(GL_LIGHT0, GL_DIFFUSE,	pfDiffuse);
//	glGetLightfv(GL_LIGHT0, GL_SPECULAR, pfSpecular);
//	glGetLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, &fSpotExponent);
//	for(int i = 0,	y = 0; y < iTexHeight;	y++)
//		for(int		x = 0; x < iTexWidth;	x++)
//		{
//			float fT0 = float(x)/float(iTexWidth-1);
//			float fT1 = float(y)/float(iTexHeight-1);
//
//			float fD = 2.0f * fT0 - 1.0f;
//			fD = sqrtf(1.0f - fD * fD);
//			float fS = 2.0f * fT1 - 1.0f;
//			fS = 2.0f * fS * fS - 1.0f;
//			for(int p = 0; p < 4; p++, i++)
//			{
//				float fI = pfAmbient[p] + pfDiffuse[p] * fD + pfSpecular[p] * powf(fS, fSpotExponent);
//				pfTex[i] = ( p < 3 )?fI:1.0f;
//			}
//		}
//
//	if( !t2dLighting )
//	{
//		CREATE_2D_TEXTURE(GL_TEXTURE_2D, t2dLighting, GL_LINEAR, GL_RGBA, iTexWidth, iTexHeight, GL_RGBA, GL_FLOAT, pfTex);
//	}
//	else
//	{
//		glBindTexture(GL_TEXTURE_2D, t2dLighting);	
//		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,	
//			iTexWidth, iTexHeight, 0, GL_RGBA, GL_FLOAT, pfTex);	
//	}
//
//	free(pfTex);
//}


void PickPanel::init()
{
	glutSetWindow(_panelWinId);
	HGLRC subWinCon = wglGetCurrentContext();
	wglShareLists(_deformWinCon, subWinCon);
	// /* Setup cube vertex data. */
	//v[0][0] = v[1][0] = v[2][0] = v[3][0] = -1;
	//v[4][0] = v[5][0] = v[6][0] = v[7][0] = 1;
	//v[0][1] = v[1][1] = v[4][1] = v[5][1] = -1;
	//v[2][1] = v[3][1] = v[6][1] = v[7][1] = 1;
	//v[0][2] = v[3][2] = v[4][2] = v[7][2] = 1;
	//v[1][2] = v[2][2] = v[5][2] = v[6][2] = -1;

	/* Enable a single OpenGL light. */
	/*glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	*//*
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);*/

	///* Use depth buffering for hidden surface elimination. */
	//glEnable(GL_DEPTH_TEST);

	///* Setup the view of the cube. */
	//glMatrixMode(GL_PROJECTION);
	//gluPerspective( /* field of view in degree */ 40.0,
	//  /* aspect ratio */ 1.0,
	//  /* Z near */ 1.0, /* Z far */ 10.0);
	//glMatrixMode(GL_MODELVIEW);
	//gluLookAt(0.0, 0.0, 5.0,  /* eye is at (0,0,5) */
	//  0.0, 0.0, 0.0,      /* center is at (0,0,0) */
	//  0.0, 1.0, 0.);      /* up is in positive Y direction */

	///* Adjust cube position to be asthetic angle. */
	//glTranslatef(0.0, 0.0, -1.0);
	//glRotatef(60, 1.0, 0.0, 0.0);
	//glRotatef(-20, 0.0, 0.0, 1.0);
	VECTOR4 _coordRange = _coordsMax - _coordsMin;
	float border = 0.6;
	VECTOR4 coordsBorder = _coordRange * border; 
	if(_coordRange[0] > _coordRange[1])
	{
		if(_coordRange[0] > _coordRange[2])
			_longerRange = 0;
		else
			_longerRange = 2;
	} 
	else if(_coordRange[1] > _coordRange[2])
		_longerRange = 1;
	else
		_longerRange = 2;

	glMatrixMode(GL_PROJECTION);
	//float orthoArr[6] = {	  _coordsMin[0] - coordsBorder[0],
	//  _coordsMax[0] + coordsBorder[0],
	//  _coordsMin[1] - coordsBorder[1],
	//  _coordsMax[1] + coordsBorder[1],
	//  _coordsMin[2] - coordsBorder[2],
	//  _coordsMax[2] + coordsBorder[2]};
	//cout<<"orthoArr:"<<endl;
	//for(int i = 0; i < 6; i++)
	//	cout<<orthoArr[i]<<endl;
	/*glOrtho(
	_coordsMin[0] - coordsBorder[0], _coordsMax[0] + coordsBorder[0],
	_coordsMin[1] - coordsBorder[1], _coordsMax[1] + coordsBorder[1], 
	-10000, 10000);*/
	glOrtho(
		_coordsMid[0] - coordsBorder[_longerRange], _coordsMid[0] + coordsBorder[_longerRange],
		_coordsMid[1] - coordsBorder[_longerRange], _coordsMid[1] + coordsBorder[_longerRange],
		-10000, 10000);
	/*
	_coordsMin[2] - coordsBorder[2],
	_coordsMax[2] + coordsBorder[2]);*/

	//GLenum err = glewInit();
	//if (GLEW_OK != err)
	//{
	//  /* Problem: glewInit failed, something is seriously wrong. */
	//  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	//  throw "Error initializing GLEW";
	//}
	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 1000.0,  /* eye is at (0,0,5) */
		0.0, 0.0, 0.0,      /* center is at (0,0,0) */
		0.0, 1.0, 0.);      /* up is in positive Y direction */
	//cout<<"**3"<<endl;
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glutPostRedisplay();

}


PickPanel::PickPanel(int x, int y, int width, int height, int panelId, int parentWinId)
{
	_panelWinId = glutCreateSubWindow(parentWinId, x, y, width, height);
	//	cout<<x<<","<<y<<","<<width<<","<<height<<endl;
	glClearColor(0.0,0.0,0.0,0.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	_panelId = panelId;
	instanceMap[_panelWinId] = this;
	//	WinId2PanelIdMap[_panelWinId] = _panelId;
	glutDisplayFunc(drawCallback);						// register Display Function
	glutMouseFunc(MouseButtonCallback);						// register Display Function
	glutMotionFunc(MouseMotionCallback);						// register Display Function
	xRot = 0;
	yRot = 0;
	zRot = 0;
}

PickPanel::~PickPanel()
{
	instanceMap.erase(_panelId);
}

void PickPanel::LoadData(VECTOR4* vertCoords, vector<int> *pviGlPrimitiveBases, vector<int> *pviGlPrimitiveLengths,
	vector<int>* bundle, GLuint vbo_pfCoords, GLuint vbo_tangent, HGLRC deformWinCon, void* deformWin)
{
	_deformWin = (CLineRendererInOpenGLDeform*) deformWin;
	_vbo_pfCoords = vbo_pfCoords;
	_vbo_tangent = vbo_tangent;
	_vertCoords = vertCoords;
	_pviGlPrimitiveBases = pviGlPrimitiveBases;
	_pviGlPrimitiveLengths = pviGlPrimitiveLengths;
	_bundle = bundle;
	_deformWinCon = deformWinCon;

	_coordsMin = VECTOR4(FLT_MAX, FLT_MAX, FLT_MAX, 0);
	_coordsMax = VECTOR4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
	for(int il = 0; il < _bundle->size(); il++)
	{
		int start = (*_pviGlPrimitiveBases)[(*_bundle)[il]];
		int length = (*_pviGlPrimitiveLengths)[(*_bundle)[il]];
		for(int i = start; i < start + length; i++)
		{
			VECTOR4 pt = _vertCoords[i];
			for(int j = 0; j < 3; j++)
			{
				//cout<<pt[j]<<":";
				if(pt[j] > _coordsMax[j])
					_coordsMax[j] = pt[j];
				if(pt[j] < _coordsMin[j])
					_coordsMin[j] = pt[j];
			}
		}
	}
	//cout<<"_coordsMin"<<_coordsMin[0]<<","<<_coordsMin[1]<<","<<_coordsMin[2]<<endl;
	//cout<<"_coordsMax"<<_coordsMax[0]<<","<<_coordsMax[1]<<","<<_coordsMax[2]<<endl;
	_coordsMid = (_coordsMin + _coordsMax) * 0.5;
	init();

}

int PickPanel::GetPanelId()
{
	return _panelId;
}

void PickPanel::drawCallback(void)
{
	//	instanceMap[0].Draw();
	//currentInstance->Draw();
	//instanceMap.find(1).;
	//assert( instanceMap[_panelId] );
	instanceMap[glutGetWindow()]->Draw();
}

void PickPanel::MouseButtonCallback(int button, int state, int x, int y)
{
	instanceMap[glutGetWindow()]->MouseButton(button, state, x, y);
}

void PickPanel::MouseMotionCallback(int x, int y)
{
	instanceMap[glutGetWindow()]->MouseMotion(x, y);
}