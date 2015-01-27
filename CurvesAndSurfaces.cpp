#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h> 
#include <pthread.h>
#include <cmath>
#include <cfloat>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>

using namespace std;

class Vertex
{
	public:
		float x1;
		float y1;
		float z1;
		float x2;
		float y2;
		float z2;
		float x3;
		float y3;
		float z3;
		float mean;
		
		Vertex()
		{}
};

int i = pthread_getconcurrency();
float** controlpoints;
float** surfacepoints;
int**   colorpalette;
int sampleR,sampleC,row,col,color;			
int main_window;
Vertex* ver = NULL;
vector<Vertex> triangles;
float avg = 0.0,avgx = 0.0,avgy = 0.0,avgz = 0.0;
int samples,mode=1;
int transx = 0,transy=0,transz=0,rotAngle=0;

int selx,sely;
float minN,maxX;
float mosX = 0.0 ,mosY = 0.0, camX = 0.0, camY = 0.0;

int factorial(int a)
{
	if(a == 0)
		return 1;
	
	else 
		return a*factorial(a-1); 
}

void bernsteinPolynom(int u,int v)
{
	int i,j;
	float x=0,y=0,z=0;
	
	for(i = 0 ; i < row ; i++)
	{
		float b_im = ((float)factorial(row-1)/(factorial(i)*factorial(row-1-i)))*pow((1.0/(sampleR-1))*u,i)*pow((1-(1.0/(sampleR-1))*u),(row-1-i));
		for(j = 0 ; j < col ; j++)
		{
			float b_nj = ((float)factorial(col-1)/(factorial(j)*factorial(col-1-j)))*pow((1.0/(sampleC-1))*v,j)*pow((1-(1.0/(sampleC-1))*v),(col-1-j));
			x = x + b_im*b_nj*controlpoints[i][3*j];
			y = y + b_im*b_nj*controlpoints[i][3*j+1];
			z = z + b_im*b_nj*controlpoints[i][3*j+2];
		}
	}
	
	surfacepoints[u][3*v] = x;		
	avgx += x;		
	surfacepoints[u][3*v+1] = y;
	avgy += y;
	surfacepoints[u][3*v+2] = z;
	avgz += z;
}


void drawBezier( void )
{
	surfacepoints = new float*[sampleR];
	for(int i = 0; i < sampleR ;i++)
		surfacepoints[i] = new float[sampleC*3];
		
		
	for(int u = 0 ; u < sampleR ; u++)
	{
		for(int v = 0 ; v < sampleC ; v++)
		{
			bernsteinPolynom(u,v);
		}
	}
	
	triangles.clear();				
	minN = FLT_MAX;
	maxX = FLT_MIN;
	for(int r = 0 ; r < (sampleR-1) ; r++)
	{
		for(int m = 0 ; m < (sampleC-1) ; m++)
		{
			ver =  new Vertex();
			
			ver->x1 = surfacepoints[r][3*m]; ver->y1 = surfacepoints[r][3*m+1]; ver->z1 = surfacepoints[r][m*3+2];					
			ver->x2 = surfacepoints[r+1][3*m]; ver->y2 = surfacepoints[r+1][3*m+1]; ver->z2 = surfacepoints[r+1][3*m+2];				
			ver->x3 = surfacepoints[r][3*(m+1)]; ver->y3 = surfacepoints[r][3*(m+1)+1]; ver->z3 = surfacepoints[r][3*(m+1)+2];		
			
			avg = (ver->y1 + ver->y2 + ver->y3)/3.0;
			
			if(avg < minN ) minN = avg;
			if(avg > maxX ) maxX = avg;
			
			ver->mean = avg;
			triangles.push_back(*ver);
			
			ver = new Vertex();
			ver->x1 = surfacepoints[r+1][3*m]; ver->y1 = surfacepoints[r+1][3*m+1]; ver->z1 = surfacepoints[r+1][3*m+2];
			ver->x2 = surfacepoints[r+1][3*(m+1)]; ver->y2 = surfacepoints[r+1][3*(m+1)+1]; ver->z2 = surfacepoints[r+1][3*(m+1)+2];
			ver->x3 = surfacepoints[r][3*(m+1)]; ver->y3 = surfacepoints[r][3*(m+1)+1]; ver->z3 = surfacepoints[r][3*(m+1)+2];
			
			avg = (ver->y1 + ver->y2 + ver->y3)/3.0;
			
			if(avg < minN ) minN = avg;
			if(avg > maxX ) maxX = avg;
			
			ver->mean = avg;
			triangles.push_back(*ver);
		}
	}
}

void myGlutDisplay( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// Clear color buffer with glClearColor() RGB values
	glLoadIdentity();

	glRotatef(camX,0,1,0);
	glRotatef(camY,1,0,0);
	gluLookAt(100,100,100, 0, 0, 0, 0, 1, 0); // Set camera position and direction
	
	
	
	glTranslatef(transx,transy,transz);
	glTranslatef((avgx/(sampleC*sampleR)),(avgy/(sampleC*sampleR)),(avgz/(sampleC*sampleR)));
	glRotatef(rotAngle,0,1,0);
	glTranslatef((-1)*(avgx/(sampleC*sampleR)),(-1)*(avgy/(sampleC*sampleR)),(-1)*(avgz/(sampleC*sampleR)));
	glTranslatef(transx*(-1),transy*(-1),transz*(-1));	
	glTranslatef(transx,transy,transz);
	
	//glColor3f(0.5,0.5,0.5);
	glBegin(GL_TRIANGLES);
	
	float diff = maxX - minN;
	
	for(int i=0;i<samples;i++)
	{
		int ind = ((triangles[i].mean - minN)*(color-1))/diff;
	
			if(ind == 0)
				ind=1;
		
		glColor3f((float)(colorpalette[ind-1][0])*1.0/255,(float)(colorpalette[ind-1][1])*1.0/255,(float)(colorpalette[ind-1][2])*1.0/255);
		glVertex3f(triangles[i].x1,triangles[i].y1,triangles[i].z1);
		glVertex3f(triangles[i].x2,triangles[i].y2,triangles[i].z2);
		glVertex3f(triangles[i].x3,triangles[i].y3,triangles[i].z3);
	}
	
	glEnd();
	
	
	if(mode == 2)
	{
		glBegin(GL_POINTS);	
		
		for(int i=0;i<row;i++)
		{
			for(int j=0;j<col;j++)
			{
				if( i==selx && j == sely )
				{
					glColor3f(1,0,0);
					glVertex3f( controlpoints[i][3*j], controlpoints[i][3*j+1], controlpoints[i][3*j+2]);
				}
				
				else
				{
					glColor3f(0.5,1,0);
					glVertex3f( controlpoints[i][3*j], controlpoints[i][3*j+1], controlpoints[i][3*j+2]);
				}
			
			}
		}
		
		glEnd();
	
	}
	
	glutSwapBuffers();	// Swap buffers in double buffered scene
}

void myGlutKey (unsigned char key, int x, int y)
{
	if(key == '1')
	{
		mode = 1;				//transformation mode
	}
	
	else if(key == '2')
	{
		mode = 2;				//surface mode
		selx = 0;
		sely = 0;
	}
		
	if(mode==1)
	{	
		if(key == 'q')
			transx--;
		
		else if(key == 'w')
			transx++;
		
		else if(key == 'a')
			transy--;
			
		else if(key == 's')
			transy++;
			
		else if(key == 'z')
			transz--;
		
		else if(key == 'x')
			transz++;
			
		else if(key == 'r')
			rotAngle++;
			
		else if(key == 't')
			rotAngle--;
		
	}
	
	else if(mode == 2)
	{
		if(key == 'q')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely]--;
		}
		
		else if(key == 'w')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely]++;
		}	
		
		
		else if(key == 'a')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely+1]--;
		}	
		
		
		else if(key == 's')
		{							
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely+1]++;
		}	
		
		else if(key == 'z')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely+2]--;
		}	
		
		else if(key == 'x')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			
			controlpoints[selx][3*sely+2]++;
		}
			
			
		else if(key == 'i')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
			if(sampleR > 1)
				sampleR = sampleR/2;
		}
		
		else if(key == 'o')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
		
			sampleR = sampleR*2;
		}
		
		else if(key == 'k')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
		
			if(sampleC > 1)
				sampleC = sampleC/2;
		}
		
		else if(key == 'l')
		{
			for(int s = 0 ; s < sampleR ; s++)
				delete[] surfacepoints[s];
			delete[] surfacepoints;
		
			sampleC = sampleC*2;
		}
		
		
		triangles.clear();
		avg = 0.0; avgx = 0.0; avgy = 0.0; avgz = 0.0;
		//transx = 0; transy=0; transz=0; rotAngle=0;
		samples = (sampleC-1)*(sampleR-1)*2;
		drawBezier();
	}
	
    glutPostRedisplay();
}

void myGlutMotion(int x,int y)
{
	if(x < mosX){
		camX -= 0.1;
		mosX = x;
	}
	
	else if(x > mosX ){
		mosX = x;
		camX += 0.1;
	}	
		
	if( y < mosY){
		mosY = y;						//+
		camY -= 0.1;
	}
	
	else if(y > mosY){
		mosY = y;
		camY += 0.1;					//-
	}
	
	
	glutPostRedisplay();
}


void myGlutSpecial(int key , int x , int y)
{
	if(mode == 2)
	{
		if(key == GLUT_KEY_UP)
		{
			if(sely != (col-1))
			{
				sely = sely+1;
			}
		}
			
		else if(key == GLUT_KEY_LEFT)
		{
			if(selx != 0 )
			{
				selx = selx-1 ;
			}
		}
				
		else if(key == GLUT_KEY_DOWN)
		{
			if(sely != 0)
			{
				sely = sely-1;
			}
		}
		
		else if(key == GLUT_KEY_RIGHT)
		{
			if(selx != (row-1) )
			{
				selx = selx+1 ;
			}
		}
	}
    glutPostRedisplay();
}


void myInit(void)
{
	// Any initialization before the main loop of GLUT goes here

	glEnable(GL_DEPTH_TEST);	// Enable depth Buffering 
	glEnable(GL_COLOR_MATERIAL);	// Enable color tracking
	glEnable(GL_NORMALIZE);	// Enables vector normalization (optional)
	glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); 	// Set initial value of color buffer (Set background color also)

	// Lighting initialization
	
	glDisable(GL_LIGHTING);

	glPointSize(5);

	// Initialize camera
	glMatrixMode(GL_PROJECTION);	// Switch  to projection matrix
	glLoadIdentity();	// Clear current matrix to identity matrix
	gluPerspective(60, 1 , 1 , 1000000);	// Set projection of camera (You can modify the arguments if needed.)

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



int main(int argc,char* argv[])
{
	/**************************************************/
	
	glutInit(&argc,argv);	// Glut initialization function
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );	// Set display mode
	glutInitWindowPosition( 50, 50 );	// Set window position
	glutInitWindowSize( 800, 800 );	// Set window size

	main_window = glutCreateWindow( "HW 2" );	// Create new window with 
	glutDisplayFunc( myGlutDisplay );	// Set display function
	glutKeyboardFunc( myGlutKey );
	glutSpecialFunc( myGlutSpecial );
	glutMotionFunc(myGlutMotion);
	//glutMouseFunc(myGlutMouse);
	
	/**************************************************/
	
	int i,count;
	string fname = argv[1];
	string line;
	string a,b,c;
	vector<int> rcmn;
	
	
	ifstream file ;
	file.open(fname.c_str());	
	
	for(i = 0 ; i < 2 ; i++)
	{	
		file >> a >> b ;
		rcmn.push_back(atoi(a.c_str()));
		rcmn.push_back(atoi(b.c_str()));
		
	}	
	
	/**************************************************/
	
	sampleR = rcmn[0];
	sampleC = rcmn[1];
	row     = rcmn[2];
	col     = rcmn[3];
	rcmn.clear();
	
	samples = (sampleC-1)*(sampleR-1)*2;
	
	controlpoints = new float*[row];						//controlpoints array is created
	for(i = 0 ; i < row ; i++)
		controlpoints[i] = new float[col*3];
	
	rcmn.clear();
	
	/**************************************************/
	
	getline(file,line);		//to pass next line
	
	for(i = 0 ; i < row ; i++ )
	{
		count = 0;
		
		getline(file,line);
		stringstream ss(line);
		while( ss >> a)
		{
			controlpoints[i][count] = atof(a.c_str());
			count++;
		}
	}
	
	
	/**************************************************/
	
	file >> a ;
	color = atoi(a.c_str());
	colorpalette = new int*[color];
	for(i = 0 ; i < color ; i++)
		colorpalette[i] = new int[3];
		
	getline(file,line);		//to pass next line
	
	for(i = 0 ; i < color ; i++)
	{
		getline(file,line);
		stringstream ss(line);
		while( ss >> a >> b >> c)
		{
			colorpalette[i][0] = atof(a.c_str());
			colorpalette[i][1] = atof(b.c_str());
			colorpalette[i][2] = atof(c.c_str());
		}
	}
	
	file.close();

	drawBezier();
	
	myInit();	// Call defined init function
	glutMainLoop();	// Start main loop after initialization
	
	
}
