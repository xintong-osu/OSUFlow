/*

Created by 
Abon Chaudhuri and Teng-Yok Lee (The Ohio State University)
May, 2010

*/
//#define GLEW_STATIC 
#include <GL\glew.h>
#include "LineRendererInOpenGLDeform.h"
#include "shaderprogram.h"
#include "PickWin.h"
#include <fstream>
#include <cuda_gl_interop.h>
//#include "ControlPanel.h"

#define DEFORM 1
//static ControlPanel* _CtrlPnl;
clock_t t_last;
//#define FILENAME_GRAPH "D:\\data\\isabel\\UVWf01_step500_seed500_seg_graph.txt"
//#define FILENAME_GRAPH "data\\UVWf01_step500_seed500_seg_graph.txt"
//#define FILENAME_VEC "D:/data/isabelUVWf01.vec"

////////////////////////////////////////////////////////////////////////////
//! Check for OpenGL error
//! @return CUTTrue if no GL error has been encountered, otherwise 0
//! @param file  __FILE__ macro
//! @param line  __LINE__ macro
//! @note The GL error is listed on stderr
//! @note This function should be used via the CHECK_ERROR_GL() macro
////////////////////////////////////////////////////////////////////////////
inline bool
sdkCheckErrorGL( const char* file, const int line) 
{
	bool ret_val = true;

	// check for error
	GLenum gl_error = glGetError();
	if (gl_error != GL_NO_ERROR) 
	{
#ifdef _WIN32
		char tmpStr[512];
		// NOTE: "%s(%i) : " allows Visual Studio to directly jump to the file at the right line
		// when the user double clicks on the error line in the Output pane. Like any compile error.
		sprintf_s(tmpStr, 255, "\n%s(%i) : GL Error : %s\n\n", file, line, gluErrorString(gl_error));
		cout <<tmpStr;
#endif
		fprintf(stderr, "GL Error in file '%s' in line %d :\n", file, line);
		fprintf(stderr, "%s\n", gluErrorString(gl_error));
		ret_val = false;
	}
	return ret_val;
}

#define SDK_CHECK_ERROR_GL()                                              \
	    if( false == sdkCheckErrorGL( __FILE__, __LINE__)) {                  \
	        exit(EXIT_FAILURE);                                               \
	    }

template<typename T>
void WriteFileMatrix(vector<vector<T>> m, char *filename)
{
	ofstream ofs;
	ofs.open(filename);
	if (!ofs.is_open())
	{
		cout<<"cannot open file: "<<filename<<endl;
	}
	for(int i = 0; i < m.size(); i++)
	{
		for(int j = 0; j < m[i].size(); j++)
		{
			ofs << m[i][j]<< "\t";
		}
		ofs << endl;
	}
	ofs.close();
}

void InitializeOpenGLExtensions()
{
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
	  /* Problem: glewInit failed, something is seriously wrong. */
	  fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	  throw "Error initializing GLEW";
	}
	if (!GLEW_VERSION_2_1)
	{
		throw "Fatal Error: OpenGL 2.1 is required";
	}

	fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
}

// ADD-BY-LEETEN 10/01/2010-BEGIN
void 
CLineRendererInOpenGLDeform::_TurnLightingOff()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glPopAttrib();	// glPushAttrib(GL_TRANSFORM_BIT);
}
// ADD-BY-LEETEN 10/01/2010-END

// ADD-BY-LEETEN 04/14/2010-BEGIN
void 
CLineRendererInOpenGLDeform::_TurnLightingOn()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, this->t2dLighting);

	double pdModelviewMatrix[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, pdModelviewMatrix);
	// set the translation to 0;
	/*
	pdModelviewMatrix[12] = 
		pdModelviewMatrix[13] = 
		pdModelviewMatrix[14] = 0.0;
	*/
	MATRIX4 m4ModelViewMatrix;
	for(int p = 0,	j = 0; j < 4; j++)
		for(int		i = 0; i < 4; i++, p++)
			m4ModelViewMatrix[j][i] = float(pdModelviewMatrix[p]);
	m4ModelViewMatrix[0].Normalize();
	m4ModelViewMatrix[1].Normalize();
	m4ModelViewMatrix[2].Normalize();
	m4ModelViewMatrix[3][0] = m4ModelViewMatrix[3][1] = m4ModelViewMatrix[3][2] = 0.0;
	for(int p = 0,	j = 0; j < 4; j++)
		for(int		i = 0; i < 4; i++, p++)
			pdModelviewMatrix[p] = double(m4ModelViewMatrix[j][i]);

	// setup a mtrix to mimic the lighting 
	// assume the light is a head light
	double pdLightMatrix[16];
	memset(pdLightMatrix, 0, sizeof(pdLightMatrix));
	pdLightMatrix[8] =  1.0;
	pdLightMatrix[9] =  1.0;
	pdLightMatrix[15] = 1.0;

	#if	0	// MOD-BY-LEETEN 10/01/2010-FROM:
		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glScalef(0.5f, 0.5f, 1.0f);
		glTranslatef(1.0f, 1.0f, 0.0f);
		glMultMatrixd(pdLightMatrix);
		glMultMatrixd(pdModelviewMatrix);

		glMatrixMode(GL_MODELVIEW);
	#else	// MOD-BY-LEETEN 10/01/2010-TO:
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_TEXTURE);
	glPushMatrix();
	glLoadIdentity();
	glScalef(0.5f, 0.5f, 1.0f);
	glTranslatef(1.0f, 1.0f, 0.0f);
	glMultMatrixd(pdLightMatrix);
	glMultMatrixd(pdModelviewMatrix);
	glPopAttrib();	// glPushAttrib(GL_TRANSFORM_BIT);
	#endif	// MOD-BY-LEETEN 10/01/2010-END
}
// ADD-BY-LEETEN 04/14/2010-END

void
CLineRendererInOpenGLDeform::_UpdateLighting()
{
	// setup the texture for the lighting
	const int iTexWidth = 256;
	const int iTexHeight = 256;
	float *pfTex;
	pfTex = (float*)calloc(4 * iTexWidth * iTexHeight, sizeof(pfTex[0]));

	/*
	const float fKa = 0.1f;
	const float fKd = 0.6f;
	const float fKs = 0.3f;
	const float fN = 4.0f;
	*/
	float pfAmbient[4];
	float pfDiffuse[4];
	float pfSpecular[4];
	float fSpotExponent;
	glGetLightfv(GL_LIGHT0, GL_AMBIENT, pfAmbient);
	glGetLightfv(GL_LIGHT0, GL_DIFFUSE,	pfDiffuse);
	glGetLightfv(GL_LIGHT0, GL_SPECULAR, pfSpecular);
	glGetLightfv(GL_LIGHT0, GL_SPOT_EXPONENT, &fSpotExponent);
	for(int i = 0,	y = 0; y < iTexHeight;	y++)
		for(int		x = 0; x < iTexWidth;	x++)
		{
			float fT0 = float(x)/float(iTexWidth-1);
			float fT1 = float(y)/float(iTexHeight-1);

			float fD = 2.0f * fT0 - 1.0f;
			fD = sqrtf(1.0f - fD * fD);
			float fS = 2.0f * fT1 - 1.0f;
			fS = 2.0f * fS * fS - 1.0f;
			for(int p = 0; p < 4; p++, i++)
			{
				float fI = pfAmbient[p] + pfDiffuse[p] * fD + pfSpecular[p] * powf(fS, fSpotExponent);
				pfTex[i] = ( p < 3 )?fI:1.0f;
			}
		}

	if( !t2dLighting )
	{
		CREATE_2D_TEXTURE(GL_TEXTURE_2D, t2dLighting, GL_LINEAR, GL_RGBA, iTexWidth, iTexHeight, GL_RGBA, GL_FLOAT, pfTex);
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, t2dLighting);	
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,	
			iTexWidth, iTexHeight, 0, GL_RGBA, GL_FLOAT, pfTex);	
	}

	free(pfTex);
}

void
CLineRendererInOpenGLDeform::_TraverseLinesBegin(int iNrOfTraces)
{
	// ADD-BY-LEETEN 07/07/2010-BEGIN
	cColorScheme._Reset();
	// ADD-BY-LEETEN 07/07/2010-END

	// ADD-BY-LEETEN 01/18/2011-BEGIN
	pviGlPrimitiveBases.resize(iNrOfTraces);
	pviGlPrimitiveLengths.resize(iNrOfTraces);
	// ADD-BY-LEETEN 01/18/2011-END
}

void
CLineRendererInOpenGLDeform::_TraverseLinesEnd()
{
	// ADD-BY-LEETEN 08/26/2010-BEGIN
	cVertexArray.iNrOfVertices = pv4Coords.size();
	// ADD-BY-LEETEN 08/26/2010-END

	if( cVertexArray.pfCoords )
	{
		free(cVertexArray.pfCoords);
		cVertexArray.pfCoords = NULL;
	}
	cVertexArray.pfCoords = (float*)calloc(sizeof(cVertexArray.pfCoords[0]) * 4, pv4Coords.size());
	for(int		p  =0,	v = 0; v < pv4Coords.size(); v++)
		for(int			i = 0; i < 4; i++, p++)
			cVertexArray.pfCoords[p] = pv4Coords[v][i];
	glVertexPointer(4, GL_FLOAT, 0, cVertexArray.pfCoords);

	if( cVertexArray.pfTexCoords )
	{
		free(cVertexArray.pfTexCoords );
		cVertexArray.pfTexCoords = NULL;
	}
	cVertexArray.pfTexCoords = (float*)calloc(sizeof(cVertexArray.pfTexCoords[0]) * 4, pv4TexCoords.size());
	for(int		p  = 0,	v = 0; v < pv4TexCoords.size(); v++)
		for(int			i = 0; i < 4; i++, p++)
			cVertexArray.pfTexCoords[p] = pv4TexCoords[v][i];
	glTexCoordPointer(4, GL_FLOAT, 0, cVertexArray.pfTexCoords);

	// ADD-BY-LEETEN 07/07/2010-BEGIN
	if( cVertexArray.pfColors )
	{
		free(cVertexArray.pfColors );
		cVertexArray.pfColors = NULL;
	}
	cVertexArray.pfColors = (float*)calloc(sizeof(cVertexArray.pfColors[0]) * 4, pv4Colors.size());
	for(int		p  = 0,	v = 0; v < pv4Colors.size(); v++)
		for(int			i = 0; i < 4; i++, p++)
			cVertexArray.pfColors[p] = pv4Colors[v][i];
	glColorPointer(4, GL_FLOAT, 0, cVertexArray.pfColors);
	// ADD-BY-LEETEN 07/07/2010-END

	glPushMatrix(); 
	glPopMatrix(); 

	#if	0	// DEL-BY-LEETEN 01/20/2011-BEGIN
		// ADD-BY-LEETEN 01/18/2011-BEGIN
		for(int iT = 0; iT < pviGlPrimitiveBases.size(); iT++)
			if( iT < pviGlPrimitiveBases.size() - 1) 
				pviGlPrimitiveLengths[iT] = pviGlPrimitiveBases[iT+1] - pviGlPrimitiveBases[iT];
			else
				pviGlPrimitiveLengths[iT] = pv4Coords.size() - pviGlPrimitiveBases[iT];
		// ADD-BY-LEETEN 01/18/2011-END
	#endif	// DEL-BY-LEETEN 01/20/2011-END
	pv4Coords.clear();
	pv4TexCoords.clear();
	// ADD-BY-LEETEN 07/07/2010-BEGIN
	pv4Colors.clear();
	// ADD-BY-LEETEN 07/07/2010-END

}

void 
CLineRendererInOpenGLDeform::_TraverseTraceBegin(int iTraceIndex, int iNrOfPoints)
{
	// ADD-BY-LEETEN 01/18/2011-BEGIN
	pviGlPrimitiveBases[iTraceIndex] = pv4Coords.size();
	// ADD-BY-LEETEN 01/18/2011-END
}

void 
// MOD-By-LEETEN 01/20/2011-FROM:
	// CLineRendererInOpenGLDeform::_TraverseTraceEnd()
// TO:
CLineRendererInOpenGLDeform::_TraverseTraceEnd(int iTraceIndex)
// MOD-By-LEETEN 01/20/2011-END
{
	// ADD-BY-LEETEN 01/20/2011-BEGIN
	pviGlPrimitiveLengths[iTraceIndex] = pv4Coords.size() - pviGlPrimitiveBases[iTraceIndex];
	// ADD-BY-LEETEN 01/20/2011-END

	// ADD-BY-LEETEN 07/07/2010-BEGIN
	cColorScheme._MoveToNextTrace();
	// ADD-BY-LEETEN 07/07/2010-END
}

void 
CLineRendererInOpenGLDeform::_TraversePoint(int iPointIndex, int iTraceIndex, float fX, float fY, float fZ, float fT)
{
	static list<int>::iterator liiParticleIterator;
	static VECTOR3 v3PrevPoint;
	static VECTOR3 v3PrevTangent;
	// ADD-BY-LEETEN 07/07/2010-BEGIN
	static VECTOR4 v4PrevColor;
	VECTOR4 v4Color = cColorScheme.V4GetColor();
	// ADD-BY-LEETEN 07/07/2010-END

	VECTOR3 v3Point(fX, fY, fZ);

	#if	0	// MOD-BY-LEETEN 02/03/2012-FROM:
		VECTOR3 v3Tangent = v3Point - v3PrevPoint;
		v3Tangent.Normalize();

		if( iPointIndex > 0 )
		{
			if( 1 == iPointIndex )
				pv4TexCoords.push_back(VECTOR4(v3Tangent[0], v3Tangent[1], v3Tangent[2], 1.0));
			else
				pv4TexCoords.push_back(VECTOR4(v3PrevTangent[0], v3PrevTangent[1], v3PrevTangent[2], 1.0));
			pv4Coords.push_back(VECTOR4(v3PrevPoint[0], v3PrevPoint[1], v3PrevPoint[2], 1.0));
			// ADD-BY-LEETEN 07/07/2010-BEGIN
			pv4Colors.push_back(v4PrevColor);
			// ADD-BY-LEETEN 07/07/2010-END

			pv4TexCoords.push_back(VECTOR4(v3Tangent[0], v3Tangent[1], v3Tangent[2], 1.0));
			pv4Coords.push_back(VECTOR4(v3Point[0], v3Point[1], v3Point[2], 1.0));
			// ADD-BY-LEETEN 07/07/2010-BEGIN
			pv4Colors.push_back(v4Color);
			// ADD-BY-LEETEN 07/07/2010-END
		}
	#else	// MOD-BY-LEETEN 02/03/2012-TO:
	VECTOR3 v3Tangent = v3Point - v3PrevPoint;
	v3Tangent.Normalize();

	if( iPointIndex > 0 )
	{
		pv4TexCoords.push_back(VECTOR4(v3Tangent[0], v3Tangent[1], v3Tangent[2], 1.0));
		pv4Coords.push_back(VECTOR4(v3Point[0], v3Point[1], v3Point[2], 1.0));
		pv4Colors.push_back(v4Color);
	}
	pv4TexCoords.push_back(VECTOR4(v3Tangent[0], v3Tangent[1], v3Tangent[2], 1.0));
	pv4Coords.push_back(VECTOR4(v3Point[0], v3Point[1], v3Point[2], 1.0));
	pv4Colors.push_back(v4Color);
	#endif	// MOD-BY-LEETEN 02/03/2012-END
	iNrOfRenderedParticles++;

	v3PrevPoint = v3Point;
	v3PrevTangent = v3Tangent;

	// ADD-BY-LEETEN 07/07/2010-BEGIN
	v4PrevColor = v4Color;
	cColorScheme._MoveToNextPoint();
	// ADD-BY-LEETEN 07/07/2010-END
}

void CLineRendererInOpenGLDeform::SetPara(float para)
{
	_para = para;
	_deformLine.SetPara(para);
}

void Draw3DArray(vector<VECTOR3> v)
{
	// activate and specify pointer to vertex array
	for(int i = 0;i < v.size(); i++)
	{
		glBegin(GL_POINTS);
		glVertex3f(v[i][0], v[i][1], v[i][2]);
		//glVertex3f(10,10,10);
		glEnd();
	}
}

inline void DrawCircle(VECTOR2 v, float r)
{
	glBegin( GL_TRIANGLE_FAN );
        glVertex2f( v[0], v[1] );
        for( float i = 0; i <= 2 * M_PI + 0.1; i += 0.1 )
            glVertex2f( v[0] + sin( i ) * r, v[1] + cos( i ) * r );
    glEnd();
}

void Draw3DArray(vector<VECTOR4> v)
{
	// activate and specify pointer to vertex array
	for(int i = 0;i < v.size(); i++)
	{
		glBegin(GL_POINTS);
		glVertex3f(v[i][0], v[i][1], v[i][2]);
		//glVertex3f(10,10,10);
		glEnd();
	}
}

void Draw3DPoint(float* v)
{
	// activate and specify pointer to vertex array
	glPointSize(3.0);
	
	glBegin(GL_POINTS);
		glVertex3f(v[0], v[1], v[2]);
	glEnd();
	
	glPointSize(1.0);
}

inline void Draw2DLine(VECTOR2 startPoint, VECTOR2 endPoint)
{
	glBegin(GL_LINES);
		glVertex2f(startPoint[0], startPoint[1]);
		glVertex2f(endPoint[0], endPoint[1]);
	glEnd();
}

void Draw3DPolyline(vector<VECTOR4> v)
{
	// activate and specify pointer to vertex array
	glBegin(GL_LINE_STRIP);
	for(int i = 0;i < v.size(); i++)
	{
		glVertex3f(v[i][0], v[i][1], v[i][2]);
		//printf("v:%f,%f,%f\n", v[i][0], v[i][1], v[i][2]);
	}
	glEnd();
}

inline void Draw3DCube(VECTOR3 minCube, VECTOR3 maxCube)
{
	//Multi-colored side - FRONT
	glBegin(GL_LINE_LOOP);
 
	glVertex3f(  maxCube[0], minCube[1], minCube[2] );      // P1 is red
	glVertex3f(  maxCube[0],  maxCube[1], minCube[2] );      // P2 is green
	glVertex3f( minCube[0],  maxCube[1], minCube[2] );      // P3 is blue
	glVertex3f( minCube[0], minCube[1], minCube[2] );      // P4 is purple
 
	glEnd();

	// White side - BACK
	glBegin(GL_LINE_LOOP);
	
	glVertex3f(  maxCube[0], minCube[1], maxCube[2] );
	glVertex3f(  maxCube[0],  maxCube[1], maxCube[2] );
	glVertex3f( minCube[0],  maxCube[1], maxCube[2] );
	glVertex3f( minCube[0], minCube[1], maxCube[2] );
	glEnd();
 
	// Purple side - RIGHT
	glBegin(GL_LINE_LOOP);
	
	glVertex3f( maxCube[0], minCube[1], minCube[2] );
	glVertex3f( maxCube[0],  maxCube[1], minCube[2] );
	glVertex3f( maxCube[0],  maxCube[1],  maxCube[2]);
	glVertex3f( maxCube[0], minCube[1],  maxCube[2] );
	glEnd();
 
	// Green side - LEFT
	glBegin(GL_LINE_LOOP);
	
	glVertex3f( minCube[0], minCube[1],  maxCube[2] );
	glVertex3f( minCube[0],  maxCube[1],  maxCube[2] );
	glVertex3f( minCube[0],  maxCube[1], minCube[2] );
	glVertex3f( minCube[0], minCube[1], minCube[2] );
	glEnd();
 
	// Blue side - TOP
	glBegin(GL_LINE_LOOP);
	
	glVertex3f(  maxCube[0],  maxCube[1],  maxCube[2] );
	glVertex3f(  maxCube[0],  maxCube[1], minCube[2] );
	glVertex3f( minCube[0],  maxCube[1], minCube[2] );
	glVertex3f( minCube[0],  maxCube[1],  maxCube[2] );
	glEnd();
 
	// Red side - BOTTOM
	glBegin(GL_LINE_LOOP);
	
	glVertex3f(  maxCube[0], minCube[1], minCube[2] );
	glVertex3f(  maxCube[0], minCube[1],  maxCube[2] );
	glVertex3f( minCube[0], minCube[1],  maxCube[2]);
	glVertex3f( minCube[0], minCube[1], minCube[2] );
	glEnd();
}

inline void TestPerf(char* text)
{
	clock_t t = clock();
	clock_t compute_time = (t - t_last) * 1000 / CLOCKS_PER_SEC;
	t_last = t;
	cout<<"Spent " << (float)compute_time * 0.001 << " sec on " << text << endl;
}

inline VECTOR2 GetEllipsePoint(ellipse e, float t)
{
	return VECTOR2(e.x + e.a * cos(t) * cos(e.angle) - e.b * sin(t) * sin(e.angle),
			e.y + e.a * cos(t) * sin(e.angle) + e.b * sin(t) * cos(e.angle));
}

void 
CLineRendererInOpenGLDeform::_Draw()
{
	if( 0 == uLid )
		return;
	

	glClearColor(1.0,1.0,1.0,1.0);
	glClearDepth(1.0);

	glPushMatrix();

	// ADD-BY-LEETEN 04/15/2010-BEGIN
	// ADD-BY-LEETEN 08/23/2012-BEGIN
	if( iIsWithBoundingBox )
	{
		// ADD-BY-LEETEN 08/23/2012-END
		float fMaxDim = 0;
		for(int i = 1; i < 3; i++)
			fMaxDim = max(fMaxDim, 
				(cBoundingBox.pv3Corners[1][i] - cBoundingBox.pv3Corners[0][i]) );

		glScalef(
			(cBoundingBox.pv3Corners[1][0] - cBoundingBox.pv3Corners[0][0])/fMaxDim, 
			(cBoundingBox.pv3Corners[1][1] - cBoundingBox.pv3Corners[0][1])/fMaxDim, 
			(cBoundingBox.pv3Corners[1][2] - cBoundingBox.pv3Corners[0][2])/fMaxDim);
		// ADD-BY-LEETEN 04/15/2010-END

		glTranslatef(-1.0f, -1.0f, -1.0f); 
		glScalef(
			2.0f/(cBoundingBox.pv3Corners[1][0] - cBoundingBox.pv3Corners[0][0]), 
			2.0f/(cBoundingBox.pv3Corners[1][1] - cBoundingBox.pv3Corners[0][1]), 
			2.0f/(cBoundingBox.pv3Corners[1][2] - cBoundingBox.pv3Corners[0][2]));
		glTranslatef(-cBoundingBox.pv3Corners[0][0], -cBoundingBox.pv3Corners[0][1], -cBoundingBox.pv3Corners[0][2]); 
	}	// ADD-BY-LEETEN 08/23/2012
	//this line has to be right after the translation
	getMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	_deformLine.RunCuda();

	clock_t t0 = clock();

	glUseProgram(line_programID);

	float tModelViewMatrixf[16];
	float tProjectionMatrixf[16];
	Bouble2FloatMatrix(tModelViewMatrix, tModelViewMatrixf);
	Bouble2FloatMatrix(tModelViewMatrix, tProjectionMatrixf);
	glUniformMatrix4fv(g_loc_modelView, 1, GL_FALSE, tModelViewMatrixf);
    glUniformMatrix4fv(g_loc_projection, 1, GL_FALSE, tProjectionMatrixf);
	
	glBindBuffer(GL_ARRAY_BUFFER, g_vbo_clipCoords);
	glVertexAttribPointer(g_loc_pfCoords, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, g_vbo_vertexColor);
	glVertexAttribPointer(g_loc_vertexColor, 4, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, g_vbo_tangent);
	glVertexAttribPointer(g_loc_tangent, 3, GL_FLOAT, GL_FALSE, 0, 0);
	VECTOR4 cyan(0.0f, 1.0f, 1.0f, 1.0f);
	
	vector<int> streamOffsetsRender = _deformLine.GetPrimitiveOffsets();
	vector<int> streamLengthsRender = _deformLine.GetPrimitiveLengths();
	if(bIsHaloEnabled )
	{
		// pass 1: draw the halo
		glLineWidth(cHalo.fWidth);
		for(int il = 0; il < streamLengthsRender.size(); il++)
		{
			VECTOR4 haloColor;
			if(_deformLine.GetLinePicked(il))
				haloColor = VECTOR4(cyan[0], cyan[1], cyan[2], 0.5f);
			else
				haloColor = VECTOR4(cHalo.v4Color[0], cHalo.v4Color[1], cHalo.v4Color[2], 0.5f);
			
			glUniform4fv(g_loc_color, 1, &haloColor[0]);
			glDrawArrays(GL_LINE_STRIP, pviGlPrimitiveBases[0] + streamOffsetsRender[il], 
				streamLengthsRender[il]);
		}
	}
	if( bIsHaloEnabled )
	{
		glDepthFunc(GL_LEQUAL);
	}

	glLineWidth(cLine.fWidth);
	
	for(int il = 0; il < streamLengthsRender.size(); il++)
	{
		glUniform4f(g_loc_color, 0.5f, 0.5f, 0.5f, 1.0f);
		glDrawArrays(GL_LINE_STRIP, pviGlPrimitiveBases[0] + streamOffsetsRender[il], 
			streamLengthsRender[il]);
	}



	//draw other features 
	//draw lens center
	glUseProgram(0);

	if(_deformLine.GetSourceMode() == SOURCE_MODE::MODE_LOCATION)
	{
		VECTOR3 cubeMin, cubeMax;
		_deformLine.GetPickCube(cubeMin, cubeMax);
		glColor4f(0.0f, 1.0f, 0.145f, 0.5f);
		Draw3DCube(cubeMin, cubeMax);
	}

	if(_deformLine.GetSourceMode() == SOURCE_MODE::MODE_LENS)
	{
		glColor4f(0.0f, 1.0f, 0.145f, 0.5f);
		Draw3DPoint(_deformLine.GetLensCenter());
	}

	glPopMatrix();

	//load 2D coordinate system
	glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, piViewport[2] - 1, 0.0, piViewport[3] - 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();



	if(_deformLine.GetSourceMode() == SOURCE_MODE::MODE_LENS)
	{
		//cout<<"**1"<<endl;
		switch(_deformLine.GetDeformMode())
		{
		case DEFORM_MODE::MODE_HULL:
			//draw hull
			glColor4f(0.0f, 1.0f, 0.145f, 0.5f);
			DrawHull(*_deformLine.GetHull());
			break;
		case DEFORM_MODE::MODE_ELLIPSE:
			//cout<<"**2"<<endl;
			//draw ellipse
			glColor4f(0.0f, 1.0f, 0.145f, 0.5f);
			DrawEllipse();
			break;
		case DEFORM_MODE::MODE_LINE:
			glColor4f(0.0f, 1.0f, 0.145f, 0.5f);
			DrawBlinds();
			break;
		}
		vector<ellipse> ellipseSet = _deformLine.GetEllipse();
		if(ellipseSet.size() > 0)
		{
			//cout<<"**3"<<endl;
			DrawCircle(GetEllipsePoint(ellipseSet.front(), 0), 4);
			DrawCircle(GetEllipsePoint(ellipseSet.front(), M_PI * 0.5), 4);
			DrawCircle(GetEllipsePoint(ellipseSet.front(), M_PI), 4);
			DrawCircle(GetEllipsePoint(ellipseSet.front(), M_PI * 1.5), 4);
			//VECTOR2 lens_center_screen = VECTOR2(ellipseSet.front().x, ellipseSet.front().y);
			//DrawCircle(lens_center_screen, 2);
		}
	}

	if(_newCutLine && 
		!(_cutLine[0][0] == _cutLine[1][0] && _cutLine[0][0] == _cutLine[1][0]))
		Draw2DLine(_cutLine[0], _cutLine[1]);

	//restore the original 3D coordinate system
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPopMatrix();

    SDK_CHECK_ERROR_GL();
#if (TEST_PERFORMANCE == 5)
	PrintElapsedTime(t0, "rendering");
#endif
}

void CLineRendererInOpenGLDeform::DrawHull(std::vector<hull_type> hull)
{
	//glBegin(GL_LINE_LOOP);
	glPointSize(4);
	glBegin(GL_LINE_LOOP);
	for(int i = 0; i < hull.size(); i++)
	{
		for(int j = 0; j < hull[i].nv; j++)
		{
			glVertex2f(hull[i].v[j].x, hull[i].v[j].y);

		}
	}	
/*
	for(int i = 0; i < contour.size(); i++)
			glVertex2f(contour[i][0], contour[i][1]);*/
	glEnd();

	
}

void CLineRendererInOpenGLDeform::SetNewCutLine(bool b)
{
	_newCutLine = b;
	_cutLine[0] = VECTOR2(0, 0);
	_cutLine[1] = VECTOR2(0, 0);
}

bool CLineRendererInOpenGLDeform::GetNewCutLine()
{
	return _newCutLine;
}


void CLineRendererInOpenGLDeform::DrawEllipse()
{
	//float center_x;
	//float center_y;
	//float width;
	//float height;
	//float angle;
	vector<ellipse> ellipseSet = _deformLine.GetEllipse();
	//cout<<"width:"<<width<<" height:"<<height<<endl; 
	//float angle = _ellipse_angle;
	//cout<<"num of ellipses:"<<ellipseSet.size()<<endl;
	for(int i = 0; i < ellipseSet.size(); i++)
	{
		//cout<<"**4"<<endl;
		float a = ellipseSet[i].a;
		float b = ellipseSet[i].b;
		float angle = ellipseSet[i].angle;
		vector<VECTOR2> ellipseVertices;
		//cout<<ellipseSet[i].x <<"," << ellipseSet[i].y <<":"<<a<<","<<b<<".."<<angle<<endl;
		for(float t = 0; t < M_PI * 2; t += (M_PI * 0.01))
		{
			float x = ellipseSet[i].x + ellipseSet[i].a * cos(t) * cos(angle) - ellipseSet[i].b * sin(t) * sin(angle);
			float y = ellipseSet[i].y + ellipseSet[i].a * cos(t) * sin(angle) + ellipseSet[i].b * sin(t) * cos(angle);
			VECTOR2 a(x,y);
			ellipseVertices.push_back(a);
		}
		DrawLineLoop(ellipseVertices);
	}
}



inline float BladesWidth(float a, float b, float dist2Center)
{
	return b * ( tanh(- 10.0 * dist2Center/ a + 8.0) + 1) * 0.5;
}

void CLineRendererInOpenGLDeform::DrawBlinds()
{
	vector<ellipse> ellipseSet = _deformLine.GetEllipse();
	const int num = 64;
	for(int i = 0; i < ellipseSet.size(); i++)
	{
		ellipse e = ellipseSet[i];
		vector<VECTOR2> shapeVertices;
		
		VECTOR2 end0 = GetEllipsePoint(e, 0);
		VECTOR2 end1 = GetEllipsePoint(e, M_PI);
		VECTOR2 center(e.x, e.y);
		VECTOR2 step = (end1 - center) * ( 1.0 / num);
		VECTOR2 dir = end0 - end1;
		VECTOR2 normDir(dir[1], - dir[0]);
		normDir.Normalize();
		for(int i = 0; i < num; i++)
		{
			VECTOR2 pBase = center + step * i;
			float radius = BladesWidth(e.a, e.b, (pBase - center).GetMag());
			shapeVertices.push_back(pBase + radius * normDir);
		}
		for(int i = num; i > 0; i--)
		{
			VECTOR2 pBase = center + step * i;
			float radius = BladesWidth(e.a, e.b, (pBase - center).GetMag());
			shapeVertices.push_back(pBase - radius * normDir);
		}
		for(int i = 0; i < num; i++)
		{
			VECTOR2 pBase = center - step * i;
			float radius = BladesWidth(e.a, e.b, (pBase - center).GetMag());
			shapeVertices.push_back(pBase - radius * normDir);
		}
		for(int i = num; i > 0; i--)
		{
			VECTOR2 pBase = center - step * i;
			float radius = BladesWidth(e.a, e.b, (pBase - center).GetMag());
			shapeVertices.push_back(pBase + radius * normDir);
		}

		DrawLineLoop(shapeVertices);
	}
}


void CLineRendererInOpenGLDeform::DrawLineLoop(vector<VECTOR2> contour)
{


	//glBegin(GL_LINE_LOOP);
	glPointSize(4);
	glBegin(GL_LINE_LOOP);
		for(int i = 0; i < contour.size(); i++)
			glVertex2f(contour[i][0], contour[i][1]);
	glEnd();

}

void CLineRendererInOpenGLDeform::DrawPixel(VECTOR2 pixel)
{
	//load 2D coordinate system
	glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, piViewport[2] - 1, 0.0, piViewport[3] - 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	//glBegin(GL_LINE_LOOP);
	glPointSize(4);
	glBegin(GL_POINTS);
		glVertex2f(pixel[0], pixel[1]);
	glEnd();

	//restore the original 3D coordinate system
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPopMatrix();
}

//ADD-BY-TONG 02/12/2013-BEGIN

void CLineRendererInOpenGLDeform::reshape()
{
	getMatrix();
//	_deformLine.SetWinSize(_winWidth, _winHeight);
}

void CLineRendererInOpenGLDeform::SetWinSize(int w, int h)
{

}

//
//there will be problem when resizing the window, because the size of the renderbuffer changes.
void CLineRendererInOpenGLDeform::InitGL()
{
	//// initiate VAO VBO
	glGenBuffers(1, &g_vbo_pfCoords_static);
	glGenBuffers(1, &g_vbo_vertexColor);
	glGenBuffers(1, &g_vbo_clipCoords);
	glGenBuffers(1, &g_vbo_tangent);

	/////////////////shaders////////////////
	ShaderProgram line_program;
    line_programID = line_program.createShader("deform_data/shaders/lines_pass_thru.vert","deform_data/shaders/pass_thru.frag");//,"shaders/pass_thru.geom");

    g_loc_pfCoords      = glGetAttribLocation(line_programID,"vertex");
	g_loc_vertexColor	= glGetAttribLocation(line_programID,"vertexObjectColor");
    g_loc_tangent      = glGetAttribLocation(line_programID,"tangent");
	g_loc_projection = glGetUniformLocation(line_programID, "projectionMatrix");
    g_loc_modelView = glGetUniformLocation(line_programID, "modelViewMatrix");
    g_loc_color = glGetUniformLocation(line_programID,"line_color");
}

StreamDeform* CLineRendererInOpenGLDeform::getDeformLine()
{
	return &_deformLine;
}


void CLineRendererInOpenGLDeform::_PassiveMotion(int x, int y)
{

}

void CLineRendererInOpenGLDeform::getMatrix()
{
	glGetDoublev(GL_MODELVIEW_MATRIX, tModelViewMatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, tProjectionMatrix);

	glGetIntegerv(GL_VIEWPORT, piViewport);
}

void CLineRendererInOpenGLDeform::PickBundle(int i)
{
	//_deformLine.resetOrigPos();
	_deformLine.PickBundle(i);
}

void CLineRendererInOpenGLDeform::AddRemoveBundle(int i)
{
	//_deformLine.resetOrigPos();
	_deformLine.AddRemoveBundle(i);
}

void 
CLineRendererInOpenGLDeform::_Update()
{
	if( NULL == pDataSource )
		return;

	if( 0 ==uLid )
		uLid = glGenLists(1);
//#if 1

	glNewList(uLid, GL_COMPILE);

	//cout<<"** before _TraverseLines()"<<endl;
	//cout<<"*1"<<endl;
	_TraverseLines();
    //SDK_CHECK_ERROR_GL();

	glEndList();
	//cout<<"*2"<<endl;

	//ADD-BY-TONG 02/12/2013-BEGIN
	InitializeOpenGLExtensions();
	//ADD-BY-TONG 02/12/2013-END

	//initiate CUDA
	cudaGLSetGLDevice(0);// gpuGetMaxGflopsDeviceId() );
    //SDK_CHECK_ERROR_GL();

	//ADD-BY-TONG 02/12/2013-BEGIN
	//cout<<"cVertexArray.iNrOfVertices"<<cVertexArray.iNrOfVertices<<endl;
	_deformLine.setData(cVertexArray.pfCoords, cVertexArray.iNrOfVertices, pviGlPrimitiveBases, pviGlPrimitiveLengths);
	_bundle = _deformLine.getBundle();

	InitGL();
	AllocGLBuffer();
	//ADD-BY-TONG 02/12/2013-END
	//cout<<"*3"<<endl;


	_deformLine.PickBundle(-1); //11
	_deformLine.setMatrix(tModelViewMatrix, tProjectionMatrix, piViewport);
	//cout<<"*4"<<endl;

	//_deformLine.UpdateVertexLineIndex();
	//_deformLine.SetVectorFieldFilename(FILENAME_VEC);
	//_deformLine.InitVecField();
	

	HGLRC currc = wglGetCurrentContext();
	
	//picking window 
	PickWin pw(NULL, NULL);
	pw.LoadData(_deformLine.GetPrimitiveBases()->at(0), &pviGlPrimitiveBases, &pviGlPrimitiveLengths, _bundle,
		g_vbo_pfCoords_static, g_vbo_tangent, currc, this);
	pw.ShowPanels();
//#endif
	//cout<<"*5"<<endl;
}
//
//void CLineRendererInOpenGLDeform::UpdateBundle()
//{
//	_deformLine.UpdateBundle();
//	_deformLine.resetOrigPos();
//}

CLineRendererInOpenGLDeform::CLineRendererInOpenGLDeform(void)
{

	_newCutLine = false;
	_cutLine[0] = VECTOR2(0, 0);
	_cutLine[1] = VECTOR2(0, 0);
//	_CtrlPnl = new ControlPanel;
//	_CtrlPnl->show();
}

CLineRendererInOpenGLDeform::~CLineRendererInOpenGLDeform(void)
{

}

void CLineRendererInOpenGLDeform::SetCutLineCoords(VECTOR2 startPoint, VECTOR2 endPoint)
{
	_cutLine[0] = startPoint;
	_cutLine[1] = endPoint;
}

void CLineRendererInOpenGLDeform::CutLineFinish()
{
	_deformLine.SetLensAxis(_cutLine[0], _cutLine[1]);
}

void CLineRendererInOpenGLDeform::SetDeformOn(bool b)
{
	_deformLine.SetDeformOn(b);
}

//Used only when new data loaded
void CLineRendererInOpenGLDeform::AllocGLBuffer()
{
	//feed data to picking window
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo_pfCoords_static);
	glVertexPointer(4, GL_FLOAT, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, 4*cVertexArray.iNrOfVertices*sizeof(GLfloat), 
		_deformLine.GetPrimitiveBases()->at(0),  GL_STATIC_DRAW);



	//give data to CUDA program
	_deformLine.Init();

    glBindBuffer(GL_ARRAY_BUFFER, g_vbo_vertexColor);
	glEnableVertexAttribArray(g_loc_vertexColor);
//	glVertexPointer(4, GL_FLOAT, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, 4*cVertexArray.iNrOfVertices*sizeof(GLfloat), 
		&(_deformLine.GetPrimitiveColors()->at(0)[0]),  GL_STATIC_DRAW);

	//initiate VBO for clip coordinates
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo_clipCoords);
	glEnableVertexAttribArray(g_loc_pfCoords);
    glBufferData(GL_ARRAY_BUFFER, 4*cVertexArray.iNrOfVertices*sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);//GL_DYNAMIC_COPY );//could be GL_STATIC_DRAW
	check_cuda_errors(__FILE__, __LINE__);

	//initiate VBO for tangent
	glBindBuffer(GL_ARRAY_BUFFER, g_vbo_tangent);
	glEnableVertexAttribArray(g_loc_tangent);
    glBufferData(GL_ARRAY_BUFFER, 3*cVertexArray.iNrOfVertices*sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);//GL_DYNAMIC_COPY );//could be GL_STATIC_DRAW
	check_cuda_errors(__FILE__, __LINE__);

	cudaGraphicsGLRegisterBuffer(&cuda_vbo_clip_resource, 
		g_vbo_clipCoords, cudaGraphicsMapFlagsWriteDiscard);
	check_cuda_errors(__FILE__, __LINE__);

	cudaGraphicsGLRegisterBuffer(&cuda_vbo_tangent_resource, 
		g_vbo_tangent, cudaGraphicsMapFlagsWriteDiscard);
	//cout<<"***0"<<endl;
	//cout<<"g_vbo_clipCoords:"<<g_vbo_clipCoords<<endl;

	_deformLine.SetCudaResourceClip(cuda_vbo_clip_resource);
	_deformLine.SetCudaResourceTangent(cuda_vbo_tangent_resource);

    SDK_CHECK_ERROR_GL();
}

/*

$Log: LineRendererInOpenGL.cpp,v $
Revision 1.12  2011-02-07 02:56:21  leeten

[02/06/2011]
1. [ADD] Change the lines from line segments to line strips.

Revision 1.11  2011/01/20 17:12:37  leeten

[01/19/2010]
1. [DEL] Remove old deleted code segments.
2. [MOD] Compute the #points per streamlines in the new _TraverseTraceEnd().
3. [MOD] Enable the color array when CColorScheme::COLOR_ON_THE_FLY != cColorScheme.iScheme.

Revision 1.10  2011/01/19 19:25:21  leeten

[01/19/2010]
1. [ADD] Add two vectors pviGlPrimitiveBases and pviGlPrimitiveLengths to store the base and length of each streamline.

Revision 1.9  2010/10/01 20:36:09  leeten

[10/01/2010]
1. Checkin the merged version from r.186.

Revision 1.7  2010/08/26 20:43:46  leeten

[08/26/2010]
1. [ADD] Add a field iNrOfVertices to the struct CVertexArray to record the #points for OpenGL (not the #particles).

Revision 1.6  2010/08/15 12:34:31  leeten

[08/14/2010]
1. [ADD] Add the authorship to the beginning of the source codes.

Revision 1.5  2010/07/07 17:34:27  leeten
[07/07/2010]
1. [ADD] Get the user-specified color through the methods of CColorScheme.
2. [ADD] Use vertex array other than display list to render the lines.

Revision 1.4  2010/07/05 14:27:19  leeten

[07/05/2010]
1. [ADD] Add structures and code segments to supprt the use of vertex array.

Revision 1.3  2010/04/16 17:26:23  leeten

[04/16/2010]
1. [MOD] Render the lines as GL_LINES other than GL_LINE_STRIPS.
2. [ADD] Scale the object coordinate to keep the aspect ratio.

Revision 1.2  2010/04/15 03:58:51  leeten

[04/12/2010]
1. [MOD] Move the OpenGL-related macros to the file opengl.h
2. [ADD] Defien a new method CLineRenderInOpenGL::_TurnLightingOn() to setup the texture for illumination.

Revision 1.1.1.1  2010/04/12 21:31:38  leeten

[04/12/2010]
1. [1ST] First Time Checkin.


*/
