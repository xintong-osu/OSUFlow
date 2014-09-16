#ifndef MODES_H
#define MODES_H

#include "stdlib.h"

enum DEFORM_MODE{
	MODE_ELLIPSE,
	MODE_LINE,
	MODE_HULL,
	MODE_DIRECT_TRANSLATE,
	MODE_AUTO,
};

enum SOURCE_MODE{
	MODE_BUNDLE,
	MODE_LENS,
	MODE_LOCATION,
};

enum INTERACT_MODE{
	DRAG_LENS_EDGE,
	DRAG_LENS_TWO_ENDS,
	MOVE_LENS,
	TRANSFORMATION,
	CUT_LINE,
	DRAW_ELLIPSE,
};

enum DIRECTION
{
	DIR_LEFT,
	DIR_RIGHT,
	DIR_UP,
	DIR_DOWN,
	DIR_IN,
	DIR_OUT,
};

enum VISUAL_MODE
{
	DEFORM,
	TRANSP,
};

#endif
