// Draw colored 3D pyramid

#include <iostream>
#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream> 
#include <iomanip>
#include "OSUFlow.h"

#include <list>
#include <iterator>
//#include <GeometricDerivations.h>

#define PI 3.1416
#define ALPHA 0.5


using namespace std;
//streamsurface variables
VECTOR3 minLen, maxLen; 
OSUFlow *osuflow= new OSUFlow();
float from[3], to[3]; 
int nSeeds; 
VECTOR3* seeds;
int i=0; 
list<VECTOR3> mylist;
float rake_length;
float ribbon_length;
float db=1.0;
TRACE_DIR direction=FORWARD_DIR;
// Callback functions
void display();                                         // Display callback
void keyboardCallback(unsigned char key, int x, int y); // Keyboard callback
void spKeyboardCallback(int key, int x, int y); // Keyboard callback
void mouseCallback(int btn, int state, int x, int y);   // Mouse callback
void MouseMotion(int x,int y);

GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};  /* White diffuse light. */
GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f,1.0 }; // white specular tlight
GLfloat light_position[] = {250.0, 250.0, 1200.0, 0.0};  /* Infinite light*/
GLfloat material_diffuse[] = { 0, 0, 1, 1 };
GLfloat material_specular[] = { .3, .3, .3, 1 };
GLfloat material_shininess[] = { 15 };
GLubyte texture[128][128][3]={0};

static float lmodel_twoside[] ={GL_TRUE};

bool triangle_flag = false;
bool is_flag_set = false;
bool first_point=true;
bool inMotion=0;
int animate=0;
int texture_enable=0;
int circle=0,step=0;
//float x1,y1,x2,y2;
int WindowHeight;
int WindowWidth;
int drawLine=0,drawSquare=0;
int translating=0,rotating=0;
float dx=0.0,dy=0.0,dz=0.0,tdx=3.0,tdy=3.0,tdz=3.0;
int tran_posX=0,tran_posY=0,tran_posZ=0;
int line_startX=-50,line_startY=0,line_startZ=0,line_endX=-50,line_endY=50,line_endZ=0;
float arr[16]={0};
float arr2[4]={0,0,0,1};
float arr3[16]={0};
float box[16]={0};
float oldLinePos[16]={0};
float s0=0.0f,t0=0.0f,s1=1.0f,t1=1.0f,del=0;
int xAxis=0,yAxis=0,zAxis=0;
float angle=0,tAngle=5.0;
float zoom_factor=1.0,view_angle=60.0;
float startX,startY,startZ,endX,endY,endZ;
bool showSur=0;
int counter=0;
int max_count=0;
static int g_bButton1Down = 0;
int currentMouseX=0,currentMouseY=0,prevMouseX=0,prevMouseY=0;
// Subroutines
void init_opengl();
void print_instructions();

bool showEntropy=0;
const GLfloat ROTATION_ANGLE = 5;
bool flag_solid = true;

static double vol[126][126][512]={0.0};
float entropy=0,old_entropy=0;
int old_size=0;

void read_entropy()
{
    ifstream infile;
    char *inname = "plume_en.vol";
    int xdim,ydim,zdim;
    unsigned i;
    float x,max=0.0,min=9999999.0;
    unsigned char *cXpoint = (unsigned char *)&x;

    infile.open(inname, ios::binary);
    if (!infile) {
        cout << "There was a problem opening file " << inname
             << " for reading." << endl;
    }
    cout << "Opened " << inname << " for reading." << endl;
    infile.read((char *)&x, 4);
    infile.read((char *)&x, 4);
    infile.read((char *)&x, 4);
    for (zdim=0;zdim<512;zdim++)
        for (ydim=0;ydim<126;ydim++)
            for (xdim=0;xdim<126;xdim++)
            {
                infile.read((char *)&x, 4);
                if (!infile) {
                    cout << "There was a problem reading " << sizeof(float)
                         << " bytes from " << inname << endl;
                }
        //        cout << "Successfully read a float from the file." << endl << endl;
        //        cout << "The bytes of x in memory : ";
        /*        for (i = 0; i < sizeof(float); i++) {
                    cout << "0x"<< hex << setfill('0') << setw(2) << (int)cXpoint[i] << " ";
                }
                cout << endl;*/

    //            cout << "x = " << fixed << setprecision(6) << x << endl;
                if(x>max)
                    max=x;
                vol[xdim][ydim][zdim]=x;
                //getch();
            }
}

float interpolate(VECTOR3 vec)
{
	double x,y,z;
	int x0,y0,z0,x1,y1,z1;
	double fz,fy,fx;
	double d000, d001, d010, d011, d100, d101, d110, d111;
	double value;
	x=vec[0];
	y=vec[1];
	z=vec[2];

	z0 = int(floor(z));
	z1 = z0 + 1;
	fz = z - z0;
	
	y0 = int(floor(y));
	y1 = y0 + 1;
	fy = y - y0;
	
	x0 = int(floor(x));
	x1 = x0 + 1;
	fx = x - x0;

	if((x0>=0) && (x1<126) && (y0>=0) && (y1<126) && (z0>=0) && (z1<512))
	{

	d000 = vol[x0][y0][z0];
	d001 = vol[x1][y0][z0];
	d010 = vol[x0][y1][z0];
	d011 = vol[x1][y1][z0];
	
	d100 = vol[x0][y0][z1];
	d101 = vol[x1][y0][z1];
	d110 = vol[x0][y1][z1];
	d111 = vol[x1][y1][z1];
	
	value =(((d000*(1-fx)+d001*fx)*(1-fy)+(d010*(1-fx)+d011*fx )*fy)*(1-fz)+((d100*(1-fx)+d101*fx)*(1-fy)+(d110*(1-fx)+d111*fx)*fy)*fz);
		
	} 
	else value=0.0;

	return value;
}

void setTube(VECTOR3 *seedSet, VECTOR3 center, int points, float radius)
{
        float theta=2*(float)PI/points;
        float angle=0;
        int i=0;
        for(;angle<=2*PI+0.0001;angle+=theta,i++)
        {
                seedSet[i][0]=center[0]+radius*cos(angle);
                seedSet[i][1]=center[1]+radius*sin(angle);
                seedSet[i][2]=center[2];

        }
}

void init_texture()
{
	ifstream myfile;
	myfile.open("new_arrow.ppm");
	
	int i,j,k;
	string line;
  
  if (myfile.is_open())
  {
    for(i=0;i<4;i++)
	{
	  getline (myfile,line);
      //cout << line << endl;
	}
	for(i=127;i>=0;i--)
		for(j=0;j<128;j++)
		{
			if ( myfile.good() )
			{
			  for(k=0;k<3;k++)
			  {	
				  getline (myfile,line);
				  texture[i][j][k]=atoi(line.c_str());
			  }
			  //cout << line << endl;
			}
		}
    myfile.close();
 }
}

void applyTransformations()
{
	glPushMatrix();
	 
	  glLoadMatrixf(arr);
	  if(translating)
	  {
		  glTranslatef(dx,dy,dz);		  
	  }
	  if(rotating)
	  {
		glTranslatef((line_startX+line_endX)/2,(line_startY+line_endY)/2,(line_startZ+line_endZ)/2);
		glRotatef(angle,xAxis,yAxis,zAxis);
		glTranslatef(-(line_startX+line_endX)/2,-(line_startY+line_endY)/2,-(line_startZ+line_endZ)/2);
	  }
	glGetFloatv(GL_MODELVIEW_MATRIX ,arr);

	glLoadMatrixf(arr3);
	  if(translating)
	  {
		  glTranslatef(dx,dy,dz);		  
	  }
	  if(rotating)
	  {
		glTranslatef((line_startX+line_endX)/2,(line_startY+line_endY)/2,(line_startZ+line_endZ)/2);
		glRotatef(angle,xAxis,yAxis,zAxis);
		glTranslatef(-(line_startX+line_endX)/2,-(line_startY+line_endY)/2,-(line_startZ+line_endZ)/2);
	  }
	glGetFloatv(GL_MODELVIEW_MATRIX ,arr3);
	glPopMatrix();
}

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
	entropy=0;
	if(flag_solid)
  	{
    	 glLineWidth(1.0);
  	}
  	else 
  	{
   	  glLineWidth(0.1);
   	}
	//cout<<"drawing mesh"<<endl;
	if(texture_enable)	
	glEnable(GL_TEXTURE_2D);
	s0=0.0f;t0=0.0f;s1=1.0f;t1=1.0f;
	if(0)//(animate)
	{
		db=db+0.01f;
		if(db>1.0f)
		db-=1.0f;
		if(db<0.2)
		db=0.2;
	}
	else db=1.0;
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
		if(showEntropy)
		{
			entropy+=interpolate(points[0]);
			entropy+=interpolate(points[1]);
			entropy+=interpolate(points[2]);
			entropy+=interpolate(points[3]);
		}

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
	if(showEntropy)
	{
		//entropy=entropy-old_entropy;		
		cout<<"The entropy for this surface is "<<(entropy-old_entropy)/((mylist.size()-old_size)/2)<<endl;
		showEntropy=0;
		old_entropy=entropy;
		old_size=mylist.size();
	}

/*	max_count++;
	if(max_count>mylist.size())
		max_count=0;*/
}

void read_file_streamlines()
{
	FILE * fp = fopen("/home/biswas/osu_flow_jan24_13/renderer/examples/streamlines.out","rb");
	
	unsigned int uNrOfPoints;

	fread(&uNrOfPoints, sizeof(uNrOfPoints), 1, fp);
		
	osuflow->SetRandomSeedPoints(from, to, uNrOfPoints);  
	seeds = osuflow->GetSeeds(nSeeds);

	if(nSeeds!=uNrOfPoints)
	cout<<"Fatal Error. Mismatch in streamlines"<<endl; 

	VECTOR3 pv3;

	for (i=0; i<nSeeds; i++)
	{
		fread(&pv3,sizeof(VECTOR3), 1, fp);
		seeds[i][0]=pv3[0];
		seeds[i][1]=pv3[1];
		seeds[i][2]=pv3[2];
		cout<<" pos is "<<pv3[0]<<" "<<pv3[1]<<" "<<pv3[2]<<endl;
	}
	fclose(fp);
}


void generateSur()
{  

float dx,dy,dz,ds;
float split_num=1.0/1.0;
  dx=(endX-startX)/(rake_length*split_num);
  dy=(endY-startY)/(rake_length*split_num);
  dz=(endZ-startZ)/(rake_length*split_num);

from[0] = minLen[0];   from[1] = minLen[1];   from[2] = minLen[2]; 
  to[0] = maxLen[0];   to[1] = maxLen[1];   to[2] = maxLen[2];

if(1)//(line_startX > 0)
{ 
	osuflow->SetRandomSeedPoints(from, to, (rake_length*split_num)+1);  
	seeds = osuflow->GetSeeds(nSeeds); 

	if(step)
	ds=0.5;
	else ds=1;

	for (i=0; i<nSeeds; i++)
	{
	seeds[i][0]=startX+i*dx;
	seeds[i][1]=startY+i*dy;
	seeds[i][2]=startZ+i*dz;
	}
}
//else read_file_streamlines();
////////////////////////////////////////////////////////////
//this is for streamsurface validation
//writing infinite(??!!) streamlines into a file
/*
int ay_num_seeds=40000;
  dx=(endX-startX)/ay_num_seeds;
  dy=(endY-startY)/ay_num_seeds;
  dz=(endZ-startZ)/ay_num_seeds;
ofstream ay_seed_file ("streamsur4.txt");
ay_seed_file<<ay_num_seeds+1<<"\n";
for (i=0; i<=ay_num_seeds; i++)
    {
	ay_seed_file<<startX+i*dx<<" "<<startY+i*dy<<" "<<startZ+i*dz<<"\n";
    }
ay_seed_file.close();
cout<<"Done writing to the seeds file"<<endl; */
//getchar();
////////////////////////////////////////////////////////////

if(circle)
setTube(seeds,seeds[nSeeds/2],rake_length-1,rake_length);

list<VECTOR3> mylocallist; 
/*for (int i=0; i<nSeeds; i++) 
    printf(" seed no. %d : [%f %f %f]\n", i, seeds[i][0], 
	   seeds[i][1], seeds[i][2]);   */
  osuflow->SetIntegrationParams(1, 5); 
  osuflow->GenStreamSurface(mylist , direction, ribbon_length, 0); 
  printf(" done integrations\n"); 
  //mylist=mylocallist;
  printf("list size = %d\n", (int)mylist.size()); 
}

//draw bounding box of the data
void drawDataBoundary()
{
	GLfloat x0=minLen[0],y0=minLen[1],z0=minLen[2],x1=maxLen[0],y1=maxLen[1],z1=maxLen[2];
	glLineWidth(4.0);
	glBegin(GL_QUADS);
	glVertex3f(x0,y0,z0);
	glVertex3f(x1,y0,z0);
	glVertex3f(x1,y1,z0);
	glVertex3f(x0,y1,z0);
	
	glVertex3f(x0,y1,z0);
	glVertex3f(x0,y1,z1);
	glVertex3f(x1,y1,z1);
	glVertex3f(x1,y1,z0);

	glVertex3f(x0,y0,z0);
	glVertex3f(x0,y1,z0);
	glVertex3f(x0,y1,z1);
	glVertex3f(x0,y0,z1);

	glVertex3f(x0,y1,z1);
	glVertex3f(x0,y0,z1);
	glVertex3f(x1,y0,z1);
	glVertex3f(x1,y1,z1);

	glVertex3f(x0,y0,z0);
	glVertex3f(x1,y0,z0);
	glVertex3f(x1,y0,z1);
	glVertex3f(x0,y0,z1);

	
	glVertex3f(x1,y1,z1);
	glVertex3f(x1,y0,z1);
	glVertex3f(x1,y0,z0);
	glVertex3f(x1,y1,z0);

	glEnd();
}

void draw_line(const GLfloat x1,const GLfloat y1,const GLfloat x2,const GLfloat y2)
{
  glBegin(GL_LINES);
  glVertex2f(x1,y1);
  glVertex2f(x2,y2);
  glEnd();
}

void resizeWindow(int w, int h)
{
	WindowHeight = (h>1) ? h : 2;
	WindowWidth = (w>1) ? w : 2;
	//glViewport(0, 0, (GLsizei) w, (GLsizei) h);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);  // Always view [0,1]x[0,1].
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}

void demo_menu(int id)
{
	switch(id)
	{
	case 1: exit(0);
		break;
	case 2: cout<<"Drawing line"<<endl;
		drawLine=1;
		translating=0;
		rotating=0;
		drawSquare=0;
		break;
	case 3: cout<<"Generating Surface..."<<endl;
		generateSur();	
		showSur=1;
		showEntropy=1;	
		drawSquare=1;
		drawLine=0;
		break;
	}
	glutPostRedisplay();
}

void rotate_menu(int id)
{
	translating=0;
	rotating=1;
	
	switch(id)
	{
	case 4: cout<<"Rotating in X"<<endl;
		tran_posX=1;
		tran_posY=0;
		tran_posZ=0;
		break;
	case 5: cout<<"Rotating in Y"<<endl;
		tran_posX=0;
		tran_posY=1;
		tran_posZ=0;
		break;
	case 6: cout<<"Rotating in Z"<<endl;
		tran_posX=0;
		tran_posY=0;
		tran_posZ=1;
		break;
	}
	//glutPostRedisplay();
}

void translate_menu(int id)
{
	rotating=0;
	translating=1;
	switch(id)
	{
	case 7: cout<<"Translating in X"<<endl;
		tran_posX=1;
		tran_posY=0;
		tran_posZ=0;
		break;
	case 8: cout<<"Translating in Y"<<endl;
		tran_posX=0;
		tran_posY=1;
		tran_posZ=0;
		break;
	case 9: cout<<"Translating in Z"<<endl;
		tran_posX=0;
		tran_posY=0;
		tran_posZ=1;
		break;
	}
	//glutPostRedisplay();
}

void create_menu()
{
	int submenu,submenu1;
	submenu=glutCreateMenu(rotate_menu);
	glutAddMenuEntry("Rotate in X direction",4);
	glutAddMenuEntry("Rotate in Y direction",5);
	glutAddMenuEntry("Rotate in Z direction",6);
	
	submenu1=glutCreateMenu(translate_menu);
	glutAddMenuEntry("Translate in X direction",7);
	glutAddMenuEntry("Translate in Y direction",8);
	glutAddMenuEntry("Translate in Z direction",9);
	
	glutCreateMenu(demo_menu);
	glutAddMenuEntry("Quit",1);
	glutAddMenuEntry("Draw New Line",2);
	glutAddSubMenu("Rotate",submenu);
	glutAddSubMenu("Translate",submenu1);
	glutAddMenuEntry("Generate Surface",3);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);

}

static void Timer(int value){
    if(inMotion||animate)
    {
	applyTransformations();
	glutPostRedisplay();
     }
    // 40 milliseconds
    glutTimerFunc(40, Timer, 0);
}

void read_off(char* off_file, int index)
{
	int vertex_count,face_count,edge_count;
	char *str;//[80];
	int nbytes = 100;
	char * pch;
	string line;
	cout<<"Need to open file "<<off_file<<endl;
	ifstream myfile (off_file);
	FILE *fp = fopen(off_file,"r");
	float val[20];
	int val_count=0;
	VECTOR3 pt,col;
	
	if (myfile.is_open())
	{
		getline (myfile,line);//read COFF
		
		getline (myfile,line);
			//getline(&str,&nbytes, fp);
			str=(char *)(line.c_str());
			//cout << line << endl;
			pch = strtok (str," ,.-");
			printf ("%s\n",pch);
			vertex_count=atoi(pch);
			//getchar();
			
			pch = strtok (NULL, " ,.-");
			printf ("%s\n",pch);
			face_count=atoi(pch);
			//getchar();
			pch = strtok (NULL, " ,.-");
			printf ("%s\n",pch);
			edge_count=atoi(pch);
			//getchar();

			cout<<vertex_count<<" "<<face_count<<" "<<edge_count<<endl;
						
		//fscanf(myfile,"%d %d %d", &vertex_count, &face_count, &edge_count);
		//while ( myfile.good() )
		for(int i=0; i<vertex_count;)
		{
			val_count=0;
			getline (myfile,line);
			//getline(&str,&nbytes, fp);
			str=(char *)(line.c_str());
			if(str[0]=='#')
			continue;
			//cout << line << endl;
			pch = strtok (str," ,");
			while (pch != NULL)
			{
			//printf ("%s\n",pch);
			val[val_count]=atof(pch);
			val_count++;
			//getchar();
			pch = strtok (NULL, " ,");
			}
			i++;

		/*	for(int j=0; j<val_count; j++)
			cout<<val[j]<<" ";
			cout<<endl;*/
	
			pt[0]=val[0]; pt[1]=val[1]; pt[2]=val[2];
			if(index==1)
			{
				col[0]=1.0; col[1]=0.0; col[2]=0.0;
			}
			else
			{
				col[0]=val[3]; col[1]=val[4]; col[2]=val[5];
			}

			mylist.push_back(pt);
			mylist.push_back(col);
			
		}
		myfile.close();
	}
	else cout << "Unable to open file"; 

}

void readlist(char* listname)
{
	char str [80];
	float f;
	FILE *pFile;
	int xdim,ydim,zdim;
	int offfiles;

	pFile = fopen (listname,"r");
	//open the list file
	fscanf(pFile,"%d %d %d",&xdim,&ydim,&zdim);
	cout<<xdim<<" "<<ydim<<" "<<zdim<<endl;
	minLen[0]=0; maxLen[0]=xdim;
	minLen[1]=0; maxLen[1]=ydim;
	minLen[2]=0; maxLen[2]=zdim;

	fscanf(pFile,"%d",&offfiles);

	for(int i=0; i<offfiles; i++)
	{
		fscanf(pFile,"%s",str);
		cout<<"Opening file "<<str<<endl;
		//read off file into mylist for display
		read_off(str, i);
	}
}

int main(int argc, char** argv)
{
  glutInit(&argc,argv); 

             
//char fileName[]="/home/ayan/isabel/isabel1.vec";
  printf("read file %s\n", argv[1]);
  cout<<"argc is "<<argc<<endl;
  //readlist(argv[1]);
  if(argc>=3)
   rake_length=atof(argv[2]);
  else rake_length=50;
  cout<<"rake lenth is "<<rake_length<<endl;
  if(argc>=4)
   ribbon_length=atof(argv[3]);
  else ribbon_length=100;
  
  if(argc>=5)
   line_startX=atof(argv[4]);
  
  if(argc>=6)
   line_startY=atof(argv[5]);
  
  if(argc>=7)
   line_startZ=atof(argv[6]);
  printf("Line stat values %f %f %f",line_startX,line_startY,line_startZ);
  
  line_endX=line_startX;
  line_endY=line_startY+rake_length;
  line_endZ=line_startZ;
  //read_entropy();
  osuflow->LoadData((const char*)argv[1], true); //true: a steady flow field 
  osuflow->Boundary(minLen, maxLen); 
  printf(" volume boundary X: [%f %f] Y: [%f %f] Z: [%f %f]\n", 
                                minLen[0], maxLen[0], minLen[1], maxLen[1], 
                                minLen[2], maxLen[2]); 
 // float from[3], to[3]; 
  

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  glutInitWindowSize(700,700);         
  glutCreateWindow("GUI for StreamSurface");  
  glEnable (GL_BLEND); 
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.0,0.0,0.0,0.0);
  //glEnable(GL_DEPTH_TEST);
  glutDisplayFunc(display);          
  glutReshapeFunc(resizeWindow);
  glutKeyboardFunc(keyboardCallback);  
  glutSpecialFunc(spKeyboardCallback);  
  glutMouseFunc(mouseCallback);  
  glutMotionFunc(MouseMotion);
  create_menu();

  init_opengl();
  Timer(0);                        
  print_instructions();                
  
  glutMainLoop();                      // Enter event loop.
  return(0);
}

void set_eye_location_and_view_direction()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetFloatv(GL_MODELVIEW_MATRIX ,arr3);
  gluLookAt(250.0, 250.0, 1200.0,       // *** CAMERA IS AT (3.0, 2.0, 3.0) ***
	    0.0, 0.0, 0.0,       // *** CENTER OF INTEREST IS (0,0,0) ***
	    0.0, 1.0, 0.0);      // *** UP DIRECTION IS POSITIVE Y ***
}

void set_projection_matrix()
{
  // Set projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // Set field of view, aspect ratio, near and far clipping planes
  gluPerspective(view_angle/zoom_factor, 1, 1.0, 10000.0);
}


void init_opengl()
{
  glClearColor(0.4, 0.4, 0.4, 1.0);    // grey background
  
  /* Enable a single OpenGL light. */
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
  glShadeModel (GL_SMOOTH);  
  //glColorMaterial ( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE ) ;
  glMaterialfv(GL_FRONT, GL_DIFFUSE, material_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, material_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, material_shininess);
  glEnable ( GL_COLOR_MATERIAL ) ;
  glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);
  glEnable(GL_DEPTH_TEST);             // Enable depth test
  // Set projection matrix
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0,           // *** FIELD OF VIEW ANGLE IN Y DIRECTION ***
		 1,              // *** ASPECT RATIO ***
		 1.0, 10000.0);    // *** NEAR & FAR CLIPPING PLANES ***

  set_eye_location_and_view_direction();
  glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
  glGetFloatv(GL_MODELVIEW_MATRIX ,oldLinePos);
//  gluOrtho2D(-40.0,160.0,-40.0,160.0);

  init_texture();  
glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 128,128,0, GL_RGB, GL_UNSIGNED_BYTE, texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glLineWidth(4.0);
}



void display()
{
  
//	float arr2[16]={0};	
	// Reset color buffer and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//glClear(GL_COLOR_BUFFER_BIT);

//glEnable(GL_DEPTH_TEST);

  

  if(drawLine)
  {
	  glLineWidth(4.0);
	  glMatrixMode(GL_MODELVIEW);
	  //glLoadIdentity();
	  glPushMatrix();
	 // glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
	  glLoadMatrixf(arr);

	  glBegin(GL_LINES);
	  glColor3f(1.0,0.0,0.0);
	  glVertex3f(line_startX,line_startY,line_startZ);
	  glVertex3f(line_endX,line_endY,line_endZ);
	  glEnd();
//	  glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
	  /************For getting world coordinates ************/
	  glLoadMatrixf(arr3);

	  arr2[0]=line_startX;
	  arr2[1]=line_startY;
	  arr2[2]=line_startZ;
	  arr2[3]=1;
//	  glGetFloatv(GL_MODELVIEW_MATRIX ,arr3);
	  glMultMatrixf(arr2);
	  glGetFloatv(GL_MODELVIEW_MATRIX ,arr2);
	  startX=arr2[0];
	  startY=arr2[1];
	  startZ=arr2[2];
	 // cout<<"Line starts at "<<startX<<" "<<startY<<" "<<startZ<<endl;
	  arr2[0]=line_endX;
	  arr2[1]=line_endY;
	  arr2[2]=line_endZ;
	  arr2[3]=1;
	  glLoadMatrixf(arr3);
	  glMultMatrixf(arr2);
	  glGetFloatv(GL_MODELVIEW_MATRIX ,arr2);
	  endX=arr2[0];
	  endY=arr2[1];
	  endZ=arr2[2];
	 // cout<<"Line ends at "<<endX<<" "<<endY<<" "<<endZ<<endl;
	  glPopMatrix();
  }

  //glMatrixMode(GL_MODELVIEW);
  //glLoadIdentity();
	
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glColor3f(0.0,0.0,1.0);
  drawDataBoundary();
  if(flag_solid)
  {
     glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else 
  {
     glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  //if(showSur)
  draw_mesh(mylist);
  //cout<<"SIze of this display list is "<<mylist.size()<<endl; 
glutSwapBuffers();
  //glFlush();
}

void spKeyboardCallback(int key, int x, int y)
{
   glMatrixMode(GL_MODELVIEW);
   int xx,yy,zz;

   if(key==GLUT_KEY_UP)
   {
    glMatrixMode(GL_MODELVIEW);
    xx=0;
    yy=tdy;
    zz=0;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
   else if(key==GLUT_KEY_DOWN)
   {
	glMatrixMode(GL_MODELVIEW);
    xx=0;
    yy=-tdy;
    zz=0;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
   else if(key==GLUT_KEY_LEFT)
   {
	glMatrixMode(GL_MODELVIEW);
    xx=-tdx;
    yy=0;
    zz=0;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
   else if(key==GLUT_KEY_RIGHT)
   {
	glMatrixMode(GL_MODELVIEW);
    xx=tdx;
    yy=0;
    zz=0;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
   else if(key==GLUT_KEY_PAGE_UP)
   {
	glMatrixMode(GL_MODELVIEW);
    xx=0;
    yy=0;
    zz=tdz;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
   else if(key==GLUT_KEY_PAGE_DOWN)
   {
	glMatrixMode(GL_MODELVIEW);
    xx=0;
    yy=0;
    zz=-tdz;
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glPushMatrix();
    glLoadMatrixf(arr);
    glTranslatef(xx,yy,zz);     // rotate around x-axis
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
   }
}

// Keyboard callback function
void keyboardCallback(unsigned char key, int x, int y)
{
  glMatrixMode(GL_MODELVIEW);
  if (key == 'x') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);     // rotate around x-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  } 
  else if (key == 'X') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(360-ROTATION_ANGLE, 1.0, 0.0, 0.0);     // rotate around x-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  } 
  else if (key == 'y') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(ROTATION_ANGLE, 0.0, 1.0, 0.0);     // rotate around y-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    //glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);
    //glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  }  
  else if (key == 'Y') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(360-ROTATION_ANGLE, 0.0, 1.0, 0.0);     // rotate around y-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  }
  else if (key == 'z') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(ROTATION_ANGLE, 0.0, 0.0, 1.0);     // rotate around z-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    glRotatef(ROTATION_ANGLE, 1.0, 0.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  }  
  else if (key == 'Z') {
    glMatrixMode(GL_MODELVIEW);
    glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
    glRotatef(360-ROTATION_ANGLE, 0.0, 0.0, 1.0);     // rotate around z-axis
    glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
    glPushMatrix();
    glLoadMatrixf(arr);
    glRotatef(360-ROTATION_ANGLE, 1.0, 0.0, 0.0);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  }
else if (key == 'A') {
    zoom_factor = zoom_factor + 0.2;                // zoom in
    set_projection_matrix();                      // reset projection matrix
    glutPostRedisplay();
  }
else if (key == 'a') {
    zoom_factor = zoom_factor - 0.2;                // zoom out
    set_projection_matrix();                      // reset projection matrix
    glutPostRedisplay();
  }
  else if (key == 'R') {
    set_eye_location_and_view_direction();        // reset to original view
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadMatrixf(oldLinePos);
    glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
    glPopMatrix();
    glutPostRedisplay();
  }  
  else if (key == 's') {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    // solid/filled polygons
    flag_solid = true;
    glutPostRedisplay();
  }
  else if (key == 'w') {                          // wireframe
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0, 1.0, 1.0);                     // color wireframe white
    flag_solid = false;
    glutPostRedisplay();
  }
  else if (key == 'l') {                          // animate
	animate=(animate+1)%2;
  }
 else if (key == 'c') {                          // circular rake
	circle=(circle+1)%2;
  }
else if (key == 'e') {                          // finer step in rake
	step=(step+1)%2;
  }
  else if (key == 't') {                          // texture
	texture_enable=(texture_enable+1)%2;
	glutPostRedisplay();
	
    }
  else if (key == 'D') {                          // texture
	//delete mylist :: list of seed traces
	mylist.clear();
	old_entropy=0;
	old_size=0;
	glutPostRedisplay();
	
    }
  else if(key== 'f')
  {
	direction=FORWARD_DIR;
  }
  else if(key== 'b')
  {
	direction=BACKWARD_DIR;
  }
  else if (key == 'q' || key == 27) {   // 27 is ascii for ESC
    exit(0);                            // quit
  }
  else {  cerr << "Unassigned key " << key << endl; } // Error
}

void mouseCallback(int btn, int state, int x, int y)
{
  
   currentMouseX=x;
   currentMouseY=y;  

   if (btn == GLUT_LEFT_BUTTON)
     {
      	if(state == GLUT_DOWN)
	  g_bButton1Down = 1;
	else g_bButton1Down = 0;
	
      }


   if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {

    //cout << "PRESSED LEFT MOUSE BUTTON" << endl;
    if(translating)
	{
		if(tran_posX)
		dx=-tdx;
		else dx=0;
	
		if(tran_posY)
			dy=-tdy;
		else dy=0;
		
		if(tran_posZ)
			dz=-tdz;
		else dz=0;
	}
	if(rotating)
	{
		angle=-tAngle;
		if(tran_posX)
			xAxis=1.0;
		else xAxis=0;
	
		if(tran_posY)
			yAxis=1.0;
		else yAxis=0;
		
		if(tran_posZ)
			zAxis=1.0;
		else zAxis=0;
	}
	applyTransformations();


    glutPostRedisplay();                // redisplay the picture
  }
  else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {

    //cout << "PRESSED RIGHT MOUSE BUTTON" << endl;

    if(translating)
	{
		
		if(tran_posX)
			dx=tdx;
		else dx=0;
		
		if(tran_posY)
			dy=tdy;
		else dy=0;
			
		if(tran_posZ)
			dz=tdz;
		else dz=0;
	}
	
	if(rotating)
	{
		angle=tAngle;
		if(tran_posX)
			xAxis=1.0;
		else xAxis=0;
	
		if(tran_posY)
			yAxis=1.0;
		else yAxis=0;
		
		if(tran_posZ)
			zAxis=1.0;
		else zAxis=0;
	}
	applyTransformations();


    glutPostRedisplay();                // redisplay the picture
  }

  	if ((state == GLUT_DOWN)&&( btn == GLUT_RIGHT_BUTTON || btn == GLUT_LEFT_BUTTON))
	{
		inMotion=1;
	}
	if(state==GLUT_UP)
	{
		inMotion=0;
	}
	if(btn==GLUT_MIDDLE_BUTTON)
	{
		//drawline
	}

}

void MouseMotion(int x, int y)
{
  // If button1 pressed, zoom in/out if mouse is moved up/down.
	prevMouseX=currentMouseX;
	prevMouseY=currentMouseY;
	currentMouseX=x;
	currentMouseY=y;
	int diffX,diffY;
	diffX=currentMouseX-prevMouseX;
	diffY=currentMouseY-prevMouseY;

  if (g_bButton1Down)
    {
      glMatrixMode(GL_MODELVIEW);
	translating=0;
	rotating=0;
	inMotion=0;
 //     glGetFloatv(GL_MODELVIEW_MATRIX ,box);
//      glLoadIdentity();
      /*/gluLookAt(250.0, 250.0, 1200.0,       // *** CAMERA IS AT (3.0, 2.0, 3.0) ***
	    0.0, 0.0, 0.0,       // *** CENTER OF INTEREST IS (0,0,0) ***
	    0.0, 1.0, 0.0);      // *** UP DIRECTION IS POSITIVE Y ***/
      glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
      glRotatef(0.25,diffY,diffX,0.0);     // rotate
      glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
  /*gluLookAt(0,0,0,250.0, 250.0, 1200.0,       // *** CAMERA IS AT (3.0, 2.0, 3.0) ***
	    0.0, 1.0, 0.0);      // *** UP DIRECTION IS POSITIVE Y ***/
 //     glMultMatrixf(box);
      //glGetFloatv(GL_MODELVIEW_MATRIX ,box);
      glPushMatrix();
      glLoadMatrixf(arr);
      //glTranslatef((minLen[0]+maxLen[0])/2.0,(minLen[1]+maxLen[1])/2.0,(minLen[2]+maxLen[2])/2.0);
      glTranslatef((line_startX+line_endX)/2,(line_startY+line_endY)/2,(line_startZ+line_endZ)/2);
      glRotatef(0.25,diffY,diffX,0.0);     // rotate
      glTranslatef(-(line_startX+line_endX)/2,-(line_startY+line_endY)/2,-(line_startZ+line_endZ)/2);
//      glTranslatef(-(minLen[0]+maxLen[0])/2.0,-(minLen[1]+maxLen[1])/2.0,-(minLen[2]+maxLen[2])/2.0);
      glGetFloatv(GL_MODELVIEW_MATRIX ,arr);
      glPopMatrix();
      glutPostRedisplay();
    }
}


void print_instructions()
{
  cout << "Type x/X to rotate counter-clockwise around x-axis." << endl;
  cout << "Type y/Y to rotate counter-clockwise around y-axis (vertical axis)." << endl;
  cout << "Type z/Z to rotate counter-clockwise around z-axis." << endl;
  cout << "Type R to reset to original view." << endl;
  cout << "Type s to display solid surface." << endl;
  cout << "Type w for wireframe display." << endl;
  cout << "Type a/A for zoom-out/zoom-in" << endl;
  cout << "Type t for texture on/off" << endl;
  cout << "Type D for removing the currently drawn stream surfaces"<<endl;
  cout << "Type Right arrow/Left arrow for translation in X axis" << endl;
  cout << "Type Up arrow/Down arrow for translation in Y axis" << endl;
  cout << "Type Pg-up/Pg-Down for translation in Z axis" << endl;
  cout << "Type q or ESC to quit." << endl;
}

