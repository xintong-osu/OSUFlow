
#include "gcb2.h"

//ADD-BY-TONG 02/13/2013-BEGIN
void (*_MouseFunc)(int button, int state, int x, int y);
void (*_MotionFunc)(int x, int y);
void (*_PassiveMotionFunc)(int x, int y);
void (*_MouseWheelFunc)( int wheel, int direction, int x, int y );
void (*_TimerFunc)( int value);
//ADD-BY-TONG 02/13/2013-END


//ADD-BY-TONG 02/13/2013-BEGIN
void
gcbMouseFunc(void (*_MyMouseFunc)(int, int, int, int))
{
	_MouseFunc = _MyMouseFunc;
}

void
gcbMotionFunc(void (*_MyMotionFunc)(int, int))
{
	_MotionFunc = _MyMotionFunc;
}

void
gcbPassiveMotionFunc(void (*_MyPassiveMotionFunc)(int, int))
{
	_PassiveMotionFunc = _MyPassiveMotionFunc;
}

void
gcbMouseWheelFunc(void (*_MyMouseWheelFunc)(int, int, int, int))
{
	_MouseWheelFunc = _MyMouseWheelFunc;
}

void
gcbTimerFunc(void (*_MyTimerFunc)(int))
{
	_TimerFunc = _MyTimerFunc;
}

//ADD-BY-TONG 02/13/2013-END
