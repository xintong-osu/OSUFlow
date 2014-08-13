#include "PickPanel.h"
#include "PickWin.h"
#include "math.h"
static std::map<int, PickPanel> instanceMap;

PickWin::PickWin(int argc, char* argv[])
{
	_winWidth = 600;
	_winHeight = 600;
	//glutInit(&argc, argv);
	glutInitWindowPosition(1115, 150);
	glutInitWindowSize(_winWidth, _winHeight);
	_winId = glutCreateWindow("Bundle Picking");
}

void PickWin::LoadData(VECTOR4* vertCoords, vector<int> *pviGlPrimitiveBases, vector<int> *pviGlPrimitiveLengths, 
	vector<vector<int>>* bundle, GLuint vbo_pfCoords, GLuint vbo_tangent, HGLRC deformWinCon, void* deformWin)
{
	_vbo_pfCoords = vbo_pfCoords;
	_vbo_tangent = vbo_tangent;
	_vertCoords = vertCoords;
	_pviGlPrimitiveBases = pviGlPrimitiveBases;
	_pviGlPrimitiveLengths = pviGlPrimitiveLengths;
	_bundle = bundle;
	_nPanel =  _bundle->size();
	_deformWinCon = deformWinCon;
	_deformWin = deformWin;
}

void PickWin::ShowPanels()
{
	int nRow = ceil(sqrt((float)_nPanel));
	//int nCol = ceil((float)_nPanel / nRow);
	int panelWidth = _winWidth / nRow;
	int panelHeight = _winHeight / nRow;
	for(int i = 0; i < _nPanel; i++ )
	{
		PickPanel* pp = new PickPanel((i % nRow) * panelWidth, (i / nRow) * panelHeight, panelWidth, panelHeight, i, _winId);//
		pp->LoadData(_vertCoords, _pviGlPrimitiveBases, _pviGlPrimitiveLengths, &_bundle->at(i), _vbo_pfCoords, _vbo_tangent, _deformWinCon, _deformWin);
	//	instanceMap.insert(std::pair<int, PickPanel>(i,*pp));
	}
	//glutMainLoop();

}