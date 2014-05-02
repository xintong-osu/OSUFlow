/*

Created by 
Abon Chaudhuri and Teng-Yok Lee (The Ohio State University)
May, 2010

*/

#include "SurfaceRenderer.h"

// ADD-BY-LEETEN 01/19/2011-BEGIN
void 
CSurfaceRenderer::_CheckTrace
(
	int iTrace,
	bool& bIsDrawingTrace
)
{
	bIsDrawingTrace = true;
}

void 
CSurfaceRenderer::_GetTraceColor
(
	int iTrace, 
	float& fR, 
	float& fG, 
	float& fB, 
	float& fA
)
{
	if( 0 == iTrace )
		this->cColorScheme._Reset();
	VECTOR4 v4Color = this->cColorScheme.V4GetColor();
	fR = v4Color[0];
	fG = v4Color[1];
	fB = v4Color[2];
	fA = v4Color[3];
	this->cColorScheme._MoveToNextTrace();
}
// ADD-BY-LEETEN 01/19/2011-END

void 
CSurfaceRenderer::_SetInteger(int iParameter,	int iValue)
{
	switch(iParameter)
	{
	// ADD-BY-LEETEN 07/07/2010-BEGIN
	case COLOR_SCHEME:
		this->cColorScheme.iScheme = iValue;
		break;
	// ADD-BY-LEETEN 07/07/2010-END

	case ENABLE_LIGHTING:
		this->bIsLightingEnabled = (bool)iValue;
		break;

	case ENABLE_HALO:
		this->bIsHaloEnabled = (bool)iValue;
		break;
	}
	CRenderer::_SetInteger(iParameter, iValue);
}

void 
CSurfaceRenderer::_GetInteger(int iParameter,	int* piValue)
{
	switch(iParameter)
	{
	case ENABLE_LIGHTING:
		*piValue = int(this->bIsLightingEnabled);
		break;

	case ENABLE_HALO:
		*piValue = int(this->bIsHaloEnabled);
		break;

	default:
		CRenderer::_GetInteger(iParameter, piValue);
	}
}

void 
CSurfaceRenderer::_SetFloat(int iParameter,		float fValue)
{
	switch(iParameter)
	{
	case LINE_WIDTH:
		this->cLine.fWidth = fValue;
		break;

	case HALO_WIDTH:
		this->cHalo.fWidth = fValue;
		break;
	}
	CRenderer::_SetFloat(iParameter, fValue);
}

void 
CSurfaceRenderer::_GetFloat(int iParameter,		float* pfValue)
{
	switch(iParameter)
	{
	case LINE_WIDTH:
		*pfValue = this->cLine.fWidth;
		break;

	case HALO_WIDTH:
		*pfValue = this->cHalo.fWidth;
		break;

	default:
		CRenderer::_GetFloat(iParameter, pfValue);
	}
}


void 
CSurfaceRenderer::_SetIntegerv(int iParameter,	int iNrOfValues, int piValues[])
{
	switch(iParameter)
	{
	default:
		CRenderer::_SetIntegerv(iParameter, iNrOfValues, piValues);
	}
}

void 
CSurfaceRenderer::_GetIntegerv(int iParameter,	int iNrOfValues, int piValues[])
{
	switch(iParameter)
	{
	default:
		CRenderer::_GetIntegerv(iParameter, iNrOfValues, piValues);
	}
}


void 
CSurfaceRenderer::_SetFloatv(int iParameter,		int iNrOfValues, float pfValues[])
{
	switch(iParameter)
	{
	case LINE_COLOR:
		switch(iNrOfValues)
		{
		case 3:
		case 4:	
			for(int i = 0; i < 4; i++)
				if( i < 3 )
					this->cLine.v4Color[i] = pfValues[i];
				else
					this->cLine.v4Color[i] = ( 4 == iNrOfValues)?pfValues[i]:1.0;
			// ADD-BY-LEETEN 02/03/2012-BEGIN
			this->cColorScheme._SetColor(this->cLine.v4Color);
			// ADD-BY-LEETEN 02/03/2012-END

			break;
		default:
			perror("Invalid parameter");
			exit(EXIT_FAILURE);
		}
		break;

	case HALO_COLOR:
		switch(iNrOfValues)
		{
		case 3:	
		case 4:	
			for(int i = 0; i < 4; i++)
				if( i < 3 )
					this->cHalo.v4Color[i] = pfValues[i];
				else
					this->cHalo.v4Color[i] = ( 4 == iNrOfValues)?pfValues[i]:1.0;

			break;

		default:
			assert("Invalid parameter");
			exit(EXIT_FAILURE);
		}
		break;
	}
	CRenderer::_SetFloatv(iParameter, iNrOfValues, pfValues);
}

void 
CSurfaceRenderer::_GetFloatv(int iParameter,		int iNrOfValues, float pfValues[])
{
	switch(iParameter)
	{
	case LINE_COLOR:
		for(int i = 0; i < 4; i++)
			pfValues[i] = this->cLine.v4Color[i];
		break;

	case HALO_COLOR:
		for(int i = 0; i < 4; i++)
			pfValues[i] = this->cHalo.v4Color[i];
		break;

	default:
		CRenderer::_GetFloatv(iParameter, iNrOfValues, pfValues);
	}
}

void 
CSurfaceRenderer::_TraverseLines()
{
	// ADD-BY-LEETEN 04/15/2010-BEGIN
	iNrOfRenderedParticles = 0;
	// ADD-BY-LEETEN 04/15/2010-END

	const list<vtListSeedTrace*>* sl_list = (const list<vtListSeedTrace*>*)this->pDataSource;

	_TraverseLinesBegin(sl_list->size());

	int iT = 0;
	for(list<vtListSeedTrace*>::const_iterator
			pIter = sl_list->begin(); 
		pIter!=sl_list->end(); 
		pIter++, iT++) 
	{
	    const vtListSeedTrace *trace = *pIter; 

		_TraverseTraceBegin(iT, trace->size());

		int iP = 0;
		for(list<VECTOR3*>::const_iterator
				pnIter = trace->begin(); 
			pnIter!= trace->end(); 
			pnIter++, iP++) 
		{
			VECTOR3 p = **pnIter; 
			_TraversePoint(iP, iT, p[0], p[1], p[2], 0.0f); 
		}
		// MOD-By-LEETEN 01/20/2011-FROM:
			// _TraverseTraceEnd();
		// TO:
		_TraverseTraceEnd(iT);
		// MOD-By-LEETEN 01/20/2011-END
	}

	_TraverseLinesEnd();
}

void CSurfaceRenderer::_TraverseSurface()
{

}

// ADD-BY-LEETEN 07/07/2010-BEGIN
void 
CSurfaceRenderer::_SetColorSource(const void *pColorSource)
{
	CRenderer::_SetColorSource(pColorSource);
	cColorScheme.plv4Colors = (const list<VECTOR4>*)this->pColorSource;
}
// ADD-BY-LEETEN 07/07/2010-END

CSurfaceRenderer::CSurfaceRenderer(void)
{
	cLine = CLine(1.0f, 1.0f, 1.0f, 1.0f, 2.0f);
	cHalo = CLine(0.0f, 0.0f, 0.0f, 1.0f, 4.0f);
}

CSurfaceRenderer::~CSurfaceRenderer(void)
{
}


void
CSurfaceRenderer::CColorScheme::_SetColor(const VECTOR4& v4Color)
{
	this->v4Color = v4Color;
}
// ADD-BY-LEETEN 02/03/2012-END

VECTOR4 
CSurfaceRenderer::CColorScheme::V4GetColor()
{
	VECTOR4 v4Color;
	switch(iScheme )
	{
	case COLOR_PER_POINT:
	case COLOR_PER_TRACE:
		if( NULL != plv4Colors )
		{
			v4Color = *liv4Colors;
			break;
		}
		else
		{
			// warning
		}
		// ADD-BY-LEETEN 02/03/2012-BEGIN
		break;
		// ADD-BY-LEETEN 02/03/2012-END

	case COLOR_ALL_WHITE:
		// MOD-BY-LEETEN 02/03/2012-FROM:
			// v4Color = VECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		// TO:
		v4Color = this->v4Color;
		// MOD-BY-LEETEN 02/03/2012-END
		break;
	}
	return v4Color;
}

void
CSurfaceRenderer::CColorScheme::_Reset()
{
	switch(iScheme)
	{
	case COLOR_PER_TRACE:
	case COLOR_PER_POINT:
		if( NULL == plv4Colors )
		{
			// warning
			return;
		}
	
		liv4Colors = plv4Colors->begin();
		break;
	}
}

void
CSurfaceRenderer::CColorScheme::_MoveToNextPoint()
{
	switch(iScheme)
	{
	case COLOR_PER_POINT:
		if( NULL == plv4Colors )
		{
			// warning
			return;
		}

		if( liv4Colors == plv4Colors->end() )
		{
			// warning
			return;
		}

		liv4Colors++;
		break;
	}
}

void
CSurfaceRenderer::CColorScheme::_MoveToNextTrace()
{
	switch(iScheme)
	{
	case COLOR_PER_TRACE:
		if( NULL == plv4Colors )
		{
			// warning
			return;
		}

		if( liv4Colors == plv4Colors->end() )
		{
			// warning
			return;
		}

		liv4Colors++;
		break;
	}
}

CSurfaceRenderer::CColorScheme::CColorScheme()
{
	iScheme = COLOR_ALL_WHITE;
	plv4Colors = NULL;
	// ADD-BY-LEETEN 02/03/2012-BEGIN
	v4Color.Set(1.0f, 1.0f, 1.0f, 1.0f);
	// ADD-BY-LEETEN 02/03/2012-END
}