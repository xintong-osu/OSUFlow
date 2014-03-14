#ifndef PICK_PANEL_H
#define PICK_PANEL_H

#include <GL\freeglut.h>
#include <vector>
#include <map>
#include <VectorMatrix.h>
#include <LineRendererInOpenGLDeform.h>

using namespace std;
//struct PickPanel;


class PickPanel
{
	static map<int, PickPanel*> instanceMap;
//	static map< int , int> WinId2PanelIdMap;
protected:
	static PickPanel* currentInstance;

private:
	vector<int> *_pviGlPrimitiveBases;
	vector<int> *_pviGlPrimitiveLengths;
	vector<int>* _bundle;

	int xRot, yRot, zRot;

	void Draw();

	
	//map< int , PickPanel> PickPanel::instanceMap;

	static void drawCallback(void);
	

	void MouseButton(int button, int state, int x, int y);
	static void MouseButtonCallback(int button, int state, int x, int y);
	void MouseMotion(int x, int y);
	static void MouseMotionCallback(int x, int y);


	int _panelId;
	int _panelWinId;

	int GetPanelId();

	void init();

	VECTOR4 *_vertCoords;
	VECTOR4 _coordsMin;
	VECTOR4 _coordsMax;
	VECTOR4 _coordsMid;

	GLuint _vbo_pfCoords;
	GLuint _vbo_tangent;
	HGLRC _deformWinCon;

	VECTOR2 _mouseDownPos;
	bool _bButton1Down ;
	CLineRendererInOpenGLDeform* _deformWin;
	static int _currentPanel;
	int _longerRange;

public:
	PickPanel(int x, int y, int width, int height, int panelId, int parentWinId);
	PickPanel(){};
	~PickPanel();
	void LoadData(VECTOR4* vertCoords, vector<int> *pviGlPrimitiveBases, vector<int> *pviGlPrimitiveLengths, vector<int>* bundle, GLuint vbo_pfCoords, GLuint vbo_tangent, HGLRC deformWinCon, void* deformWin);
};


#endif //PICK_PANEL_H