/*

Created by 
Abon Chaudhuri and Teng-Yok Lee (The Ohio State University)
May, 2010

*/

#include "SurfaceRendererBasic.h"
#include "opengl.h"

VECTOR3 crossProduct(VECTOR3 v2,VECTOR3 v1)
{
	VECTOR3 vec;
	float val;
	vec[0]=v2[1]*v1[2]-v2[2]*v1[1];
	vec[1]=v2[2]*v1[0]-v2[0]*v1[2];
	vec[2]=v2[0]*v1[1]-v2[1]*v1[0];
	val=fabs(sqrt(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]));
	if(val==0.0f)
	  val=1.0f;
	vec[0]=vec[0]/val;
	vec[1]=vec[1]/val;
	vec[2]=vec[2]/val;	

	return vec;
}

void calculateNormals(VECTOR3 *pts,VECTOR3 * norms)
{
	VECTOR3 vec1,vec2;
	vec1[0]=pts[3][0]-pts[0][0];
	vec1[1]=pts[3][1]-pts[0][1];
	vec1[2]=pts[3][2]-pts[0][2];

	vec2[0]=pts[1][0]-pts[0][0];
	vec2[1]=pts[1][1]-pts[0][1];
	vec2[2]=pts[1][2]-pts[0][2];

	norms[0]= crossProduct(vec2,vec1);

	vec1[0]=pts[0][0]-pts[1][0];
	vec1[1]=pts[0][1]-pts[1][1];
	vec1[2]=pts[0][2]-pts[1][2];

	vec2[0]=pts[2][0]-pts[1][0];
	vec2[1]=pts[2][1]-pts[1][1];
	vec2[2]=pts[2][2]-pts[1][2];

	norms[1]= crossProduct(vec2,vec1);

	vec1[0]=pts[1][0]-pts[2][0];
	vec1[1]=pts[1][1]-pts[2][1];
	vec1[2]=pts[1][2]-pts[2][2];

	vec2[0]=pts[3][0]-pts[2][0];
	vec2[1]=pts[3][1]-pts[2][1];
	vec2[2]=pts[3][2]-pts[2][2];

	norms[2]= crossProduct(vec2,vec1);

	vec1[0]=pts[2][0]-pts[3][0];
	vec1[1]=pts[2][1]-pts[3][1];
	vec1[2]=pts[2][2]-pts[3][2];

	vec2[0]=pts[0][0]-pts[3][0];
	vec2[1]=pts[0][1]-pts[3][1];
	vec2[2]=pts[0][2]-pts[3][2];

	norms[3]= crossProduct(vec2,vec1);

}

void draw_mesh(list<VECTOR3>mylist)
{
	list<VECTOR3>::iterator sIdIter;
	VECTOR3 points[4],normals[4],color[4];
    bool flag_solid = true;
	if(flag_solid)
  	{
    	 glLineWidth(1.0);
  	}
  	else 
  	{
   	  glLineWidth(0.1);
   	}

	float s0=0.0f,t0=0.0f,s1=1.0f,t1=1.0f,del=0;
	float db=1.0;
	s0=0.0f;t0=0.0f;s1=1.0f;t1=1.0f;
	
	db=1.0;
	if(0)//(animate)
	{
		s1+=0.2;
		if(s1>1)
		s1=0.2;
	}
	else s1=1.0;
	for(sIdIter=mylist.begin();sIdIter!=mylist.end();++sIdIter)
	{
	/*	if(animate)
		{
			if(counter>max_count)
			glDisable(GL_TEXTURE_2D);
			else glEnable(GL_TEXTURE_2D);

			counter++;
		}
	*/	

		glColor3f(0.0f,0.0f,db);				// Set The Color To Blue One Time Only
		float mcolor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mcolor);
		glBegin(GL_QUADS);						// Start Drawing Quads
		points[0]=*sIdIter;
		++sIdIter;
		color[0]=*sIdIter;
		++sIdIter;
		points[1]=*sIdIter;
		++sIdIter;
		color[1]=*sIdIter;
		++sIdIter;
		points[2]=*sIdIter;
		++sIdIter;
		color[2]=*sIdIter;
		++sIdIter;
		points[3]=*sIdIter;
		++sIdIter;
		color[3]=*sIdIter;

	//	glMaterial(GL_FRONT_AND_BACK);	
		calculateNormals(points,normals);
		glNormal3f(normals[0][0],normals[0][1],normals[0][2]);
		glTexCoord2f(s0,t1);
		//glColor4f(color[0][0],color[0][1],color[0][2],interpolate(points[0]));
		glColor4f(color[0][0],color[0][1],color[0][2],1);
		glVertex3f(points[0][0], points[0][1],points[0][2]);		// Left And Up 1 Unit (Top Left)
		//cout<<"The point location is "<<(*sIdIter)[0]<<" "<<(*sIdIter)[1]<<" "<<(*sIdIter)[2]<<endl;
		
		glNormal3f(normals[1][0],normals[1][1],normals[1][2]);
		glTexCoord2f(s0,t0);
		//glColor4f(color[1][0],color[1][1],color[1][2],interpolate(points[1]));
		glColor4f(color[1][0],color[1][1],color[1][2],1);
		glVertex3f(points[1][0], points[1][1],points[1][2]);		// Right And Up 1 Unit (Top Right)

		glNormal3f(normals[2][0],normals[2][1],normals[2][2]);
		glTexCoord2f(s1,t0);
//		glColor4f(color[2][0],color[2][1],color[2][2],interpolate(points[2]));
		glColor4f(color[2][0],color[2][1],color[2][2],1);
		glVertex3f(points[2][0], points[2][1],points[2][2]);		// Right And Down One Unit (Bottom Right)

		glNormal3f(normals[3][0],normals[3][1],normals[3][2]);
		glTexCoord2f(s1,t1);
		//glColor4f(color[3][0],color[3][1],color[3][2],interpolate(points[3]));
		glColor4f(color[3][0],color[3][1],color[3][2],1);
		glVertex3f(points[3][0], points[3][1],points[3][2]);		// Left And Down One Unit (Bottom Left)
		glEnd();
	}
	glDisable(GL_TEXTURE_2D);

}

// ADD-BY-LEETEN 10/01/2010-BEGIN
void 
CSurfaceRendererBasic::_TurnLightingOff()
{
	glPushAttrib(GL_TRANSFORM_BIT);
	glMatrixMode(GL_TEXTURE);
	glPopMatrix();
	glPopAttrib();	// glPushAttrib(GL_TRANSFORM_BIT);
}
// ADD-BY-LEETEN 10/01/2010-END

// ADD-BY-LEETEN 04/14/2010-BEGIN
void 
CSurfaceRendererBasic::_TurnLightingOn()
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
CSurfaceRendererBasic::_UpdateLighting()
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
CSurfaceRendererBasic::_TraverseLinesBegin(int iNrOfTraces)
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
CSurfaceRendererBasic::_TraverseLinesEnd()
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
CSurfaceRendererBasic::_TraverseTraceBegin(int iTraceIndex, int iNrOfPoints)
{
	// ADD-BY-LEETEN 01/18/2011-BEGIN
	pviGlPrimitiveBases[iTraceIndex] = pv4Coords.size();
	// ADD-BY-LEETEN 01/18/2011-END
}

void 
// MOD-By-LEETEN 01/20/2011-FROM:
	// CSurfaceRendererBasic::_TraverseTraceEnd()
// TO:
CSurfaceRendererBasic::_TraverseTraceEnd(int iTraceIndex)
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
CSurfaceRendererBasic::_TraversePoint(int iPointIndex, int iTraceIndex, float fX, float fY, float fZ, float fT)
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

void 
CSurfaceRendererBasic::_Draw()
{
	if( 0 == uLid )
		return;

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

	//if( bIsHaloEnabled )
	//{
	//	// pass 1: draw the halo

	//	glPushAttrib(
	//		GL_LINE_BIT |
	//		0
	//		);

	//	glLineWidth(cHalo.fWidth);
	//	glColor4fv(&cHalo.v4Color[0]);
	//	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	//	glVertexPointer(4, GL_FLOAT, 0, cVertexArray.pfCoords);
	//	glTexCoordPointer(4, GL_FLOAT, 0, cVertexArray.pfTexCoords);
	//	glEnableClientState(GL_VERTEX_ARRAY);
	//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	//	for(int iT = 0; iT < this->pviGlPrimitiveBases.size(); iT++)
	//	{
	//		bool bIsDrawing;
	//		_CheckTrace(iT, bIsDrawing);
	//		if( !bIsDrawing )
	//			continue;
	//		// MOD-BY-LEETEN 02/03/2012-FROM:
	//			// glDrawArrays(GL_LINES, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
	//		// TO:
	//		glDrawArrays(GL_LINE_STRIP, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
	//		// MOD-BY-LEETEN 02/03/2012-END
	//	}
	//	glPopClientAttrib();	// glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	//	glPopAttrib();	
	//}

	// pass 2: draw the lines
	glPushAttrib(
		GL_LINE_BIT |
		GL_TEXTURE_BIT |
		GL_DEPTH_BUFFER_BIT |
		0
		);

	if( bIsLightingEnabled )
	{
		_TurnLightingOn();
	}

	if( bIsHaloEnabled )
	{
		glDepthFunc(GL_LEQUAL);
		glDepthMask(GL_FALSE);
	}
	glLineWidth(cLine.fWidth);

	glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glColorPointer(4, GL_FLOAT, 0, cVertexArray.pfColors);
	glVertexPointer(4, GL_FLOAT, 0, cVertexArray.pfCoords);
	glTexCoordPointer(4, GL_FLOAT, 0, cVertexArray.pfTexCoords);
	// DEL-BY-LEETEN 01/19/2011-BEGIN
		// glEnableClientState(GL_COLOR_ARRAY);
	// DEL-BY-LEETEN 01/19/2011-END
	// ADD-BY-LEETEN 01/20/2011-BEGIN
	if(CColorScheme::COLOR_ON_THE_FLY != this->cColorScheme.iScheme )
		glEnableClientState(GL_COLOR_ARRAY);
	// ADD-BY-LEETEN 01/20/2011-END
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	// MOD-BY-LEETEN 01/18/2011-FROM:
		// glDrawArrays(GL_LINES, 0, cVertexArray.iNrOfVertices);
	// TO:
	for(int iT = 0; iT < this->pviGlPrimitiveBases.size(); iT++)
	{
		// ADD-BY-LEETEN 01/20/2011-BEGIN
		if(CColorScheme::COLOR_ON_THE_FLY != this->cColorScheme.iScheme )
		{
			// MOD-BY-LEETEN 02/03/2012-FROM:
				// glDrawArrays(GL_LINES, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
			// TO:
			glDrawArrays(GL_LINE_STRIP, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
			// MOD-BY-LEETEN 02/03/2012-END
			continue;
		}
		// ADD-BY-LEETEN 01/20/2011-END
		bool bIsDrawing;
		_CheckTrace(iT, bIsDrawing);
		if( !bIsDrawing )
			continue;
		float fR, fG, fB, fA;
		_GetTraceColor(iT, fR, fG, fB, fA);
		glColor4f(fR, fG, fB, fA);
		// MOD-BY-LEETEN 02/03/2012-FROM:
			// glDrawArrays(GL_LINES, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
		// TO:
		draw_mesh();
	//	glDrawArrays(GL_LINE_STRIP, pviGlPrimitiveBases[iT], pviGlPrimitiveLengths[iT]);
		// MOD-BY-LEETEN 02/03/2012-END
	}
	// MOD-BY-LEETEN 01/18/2011-END
	glPopClientAttrib();	// glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
	glPopAttrib();	// glPushAttrib(GL_LINE_BIT);
	glPopMatrix();

	// ADD-BY-LEETEN 10/01/2010-BEGIN
	if( bIsLightingEnabled )
	{
		_TurnLightingOff();
	}
	// ADD-BY-LEETEN 10/01/2010-END
}

void 
CSurfaceRendererBasic::_Update()
{
	if( NULL == pDataSource )
		return;

	if( 0 ==uLid )
		uLid = glGenLists(1);

	glNewList(uLid, GL_COMPILE);

	_TraverseLines();

	glEndList();
}

CSurfaceRendererBasic::CSurfaceRendererBasic(void)
{
}

CSurfaceRendererBasic::~CSurfaceRendererBasic(void)
{
}

/*

$Log: SurfaceRendererBasic.cpp,v $
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
