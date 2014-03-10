#ifndef PICK_WIN_H
#define PICK_WIN_H

#include <GL\freeglut.h>
#include <vector>
#include <VectorMatrix.h>

using namespace std;

class PickWin
{
	vector<int> *_pviGlPrimitiveBases;
	vector<int> *_pviGlPrimitiveLengths;
	vector<vector<int> > *_bundle;
	int _winWidth;
	int _winHeight;
	int _nPanel;
	int _winId;
	VECTOR4* _vertCoords;
	GLuint _vbo_tangent;
	GLuint _vbo_pfCoords;
	HGLRC _deformWinCon;
	void* _deformWin;

public:
	PickWin(int argc, char* argv[]);
	void LoadData(VECTOR4* vertCoords, vector<int> *pviGlPrimitiveBases, vector<int> *pviGlPrimitiveLengths, 
	vector<vector<int>>* bundle, GLuint vbo_pfCoords, GLuint vbo_tangent, HGLRC deformWinCon, void* deformWin);
	void ShowPanels();
};

#endif //