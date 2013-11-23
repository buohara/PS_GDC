/*
file: main_gl.c
author: Ben O'Hara
about: GL-based front end for visualizing power systems.
*/

#include "ps.h"
#include "dcsimsep.h"
#include <GL/glew.h>
#include <GL/freeglut.h>

/*
shader text
*/
char *vsh_src=
{
"#version 400\n" \
"uniform mat4 m_mod;\n" \
"uniform mat4 m_view;\n" \
"uniform mat4 m_proj;\n" \
"uniform float ov_curr_sc;\n" \
"layout(location=0) in vec3 in_position;\n" \
"layout(location=1) in float br_ovcurr;\n" \
"out vec4 ex_color;\n" \
"vec3 get_jet_color(float value){\n" \
     "if(value<0.0)return vec3(0.9,0.9,0.9);\n" \
     "float four_val=4*value*ov_curr_sc;\n" \
     "float red=min(four_val-1.5,-four_val+4.5);\n" \
     "float green=0.6*min(four_val-0.5,-four_val+3.5);\n" \
     "float blue=min(four_val+0.5,-four_val+2.5);\n" \
     "return clamp(vec3(red,green,blue),0.0,1.0);\n" \
"}\n" \
"void main(void){\n" \
    "vec3 out_rgb=get_jet_color(br_ovcurr);\n" \
    "gl_Position=m_proj*m_view*m_mod*vec4(in_position,1.0);\n" \
	"ex_color=vec4(out_rgb,1.0);\n" \
"}\n"
},
*fsh_src=
{
"#version 400\n" \
"in vec4 ex_color;\n" \
"out vec4 out_color;\n" \
"void main(void){\n" \
	"out_color=ex_color;\n" \
"}\n"
};

/*
global variables
*/
int win_w=1280,win_h=768,narea=1;
double xmin=0.0,xmax=0.0,ymin=0.0,ymax=0.0;
PS *ps;
float r=1.0,l=0.0,t=1.0,b=0.0,n=-1.0,f=1.0;
double imax=0.0,imag=0.0,bmva_inv=0.0,ov_curr=0.0,ts=0.0,dt=0.0;
GLenum err;
Outage *outages;
GLuint vshader,fshader,ps_program,vao,vbo;

/*
initial MVP matrices
*/
float m_rot[4][4]=
{
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,0.0,1.0}
};

float m_trans[4][4]=
{
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,0.0,1.0}
};

float m_proj[4][4]=
{
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,0.0,1.0}
};

/*
functions declarations
*/
static void Usage(void);
static void Render(void);
static void Reshape(int w,int h);
static int InitScene();
static void Cleanup(void);

/*
get and display OpenGL errors
*/
inline void GetErr(void)
{
	err=glGetError();
	if(err!=GL_NO_ERROR)printf("Error: %s\n",gluErrorString(err)); 
	else printf("No OpenGL Errors\n");
}

/*
print command line flag usage
*/
void Usage(void)
{
	printf("Usage:\n");
	printf(" -f <ps_file_name>\n");
	exit(8);
}

int InitScene()
{
	//prep variables
	int i,nbus=ps->nbus,nbranch=ps->nbranch;
	float *branch_verts=malloc(6*nbranch*sizeof(float)),x,y,ov_curr_max=0.0,
	*br_ovcurrs=malloc(2*nbranch*sizeof(float));
	BUS *buses=ps->buses;
	BRANCH *branches=ps->branches;
	GLint m_mod,m_view,ov_curr_sc;
	
	//knock out some branches to get simulation going
	outages=malloc(sizeof(Outage));
	outages->branches=NULL;
	outages->t=NULL;
	add_branch_out(outages,4,16,0.0);
	add_branch_out(outages,11,17,0.0);
	
	/*
	perform exogenous branch trips. find subgraphs, and redispatch generation as needed
	*/
	narea=1;
	trip_branches(ps,outages);
	find_subgraphs(ps);
	if(narea<ps->narea)
	{
		narea=ps->narea;
		redispatch(ps);	
	}
	
	//do initial load flow
	dcpf(ps);
	
	//generate and bind a vertex array
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);
	
	/*
	initialize branch physical locations and set overcurrent
	values to zero.
	*/
	for(i=0;i<nbranch;i++)
	{
		//add from bus vertex
		x=buses[branches[i].from].loc_x;
		y=buses[branches[i].from].loc_y;
		if(x>xmax)xmax=x;
		if(x<xmin)xmin=x;
		if(y>ymax)ymax=y;
		if(y<ymin)ymin=y;
		branch_verts[6*i]=x;
		branch_verts[6*i+1]=y;
		branch_verts[6*i+2]=0.f;
		
		//add to bus vertex
		x=buses[branches[i].to].loc_x;
		y=buses[branches[i].to].loc_y;
		if(x>xmax)xmax=x;
		if(x<xmin)xmin=x;
		if(y>ymax)ymax=y;
		if(y<ymin)ymin=y;
		branch_verts[6*i+3]=x;
		branch_verts[6*i+4]=y;
		branch_verts[6*i+5]=0.f;
		
		//initialize overcurrents
		imag=branches[i].imag;
		if(branches[i].status)
		{
			br_ovcurrs[2*i]=(float)imag;
			br_ovcurrs[2*i+1]=(float)imag;
			if(imag>ov_curr_max)ov_curr_max=imag;
		}
		else
		{
			br_ovcurrs[2*i]=-1.f;
			br_ovcurrs[2*i+1]=-1.f;
		}
	}

	/*
	bind a vertex buffer and initialize its data
	*/
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER,8*nbranch*sizeof(float),NULL,GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,6*nbranch*sizeof(float),branch_verts);
	glBufferSubData(GL_ARRAY_BUFFER,6*nbranch*sizeof(float),2*nbranch*sizeof(float),br_ovcurrs);

	/*
	set offsets for the position and overcurrent attributes
	*/ 
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(const GLvoid*)0);
	glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,(GLvoid*)(6*nbranch*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	
	//compile the mesh vertex (pass through) shader
	vshader=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader,1,(const GLchar**)&vsh_src,NULL);
	glCompileShader(vshader);

	//compile the fragment shader
	fshader=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader,1,(const GLchar**)&fsh_src,NULL);
	glCompileShader(fshader);	

	//create the mesh program.
	ps_program=glCreateProgram();	
	glAttachShader(ps_program,vshader);
	glAttachShader(ps_program,fshader);
	glLinkProgram(ps_program);
	glUseProgram(ps_program);

	//initialize mvp matrices for both shader programs
	m_mod=glGetUniformLocation(ps_program,"m_mod");
	glUniformMatrix4fv(m_mod,1,GL_FALSE,(const GLfloat*)m_rot);

	m_view=glGetUniformLocation(ps_program,"m_view");
	glUniformMatrix4fv(m_view,1,GL_TRUE,(const GLfloat*)m_trans);
	
	ov_curr_sc=glGetUniformLocation(ps_program,"ov_curr_sc");
	glUniform1f(ov_curr_sc,(GLfloat)(1.0/ov_curr_max));
	
	free(branch_verts);
	free(br_ovcurrs);
	
	return 0;
}

//release our GL resources
void Cleanup(void)
{
	//unbind and delete the vertex buffer	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1,&vbo);

	//unbind and delete the vertex array buffer
	glBindVertexArray(0);
	glDeleteVertexArrays(1,&vao);

	//detach the shader program and shaders
	glUseProgram(0);
	glDetachShader(ps_program,vshader);
	glDetachShader(ps_program,fshader);

	//delete shader program and shaders
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(ps_program);
	
	//free up our outages
	free_outages(outages);
	
	//free ps
	ps_free(ps);
}

/*
GLUT render function
*/
void Render(void)
{
	//prep variables
	int i,nbranch=ps->nbranch;
	BRANCH *branches=ps->branches;
	float ov_curr_max=0.0,*br_ovcurrs=malloc(2*nbranch*sizeof(float));
	GLint ov_curr_sc;
	
	//check if any line overloads tripped relays
	dt=update_relays(ps,outages,ts);
	if(dt==INFINITY)
	{
		printf("No overcurrents. Press ENTER to exit.\n");
		getchar();
		exit(0);
	}
	ts+=dt;
		
	//check for any new grid separation and redispatch if so
	find_subgraphs(ps);
	if(narea<ps->narea)
	{
		narea=ps->narea;
		redispatch(ps);	
	}
	
	//run load flow
	dcpf(ps);
	
	//push branch currents to gpu
	for(i=0;i<nbranch;i++)
	{	
		//initialize overcurrents
		imag=branches[i].imag;
		if(branches[i].status)
		{
			br_ovcurrs[2*i]=(float)imag;
			br_ovcurrs[2*i+1]=(float)imag;
			if(imag>ov_curr_max)ov_curr_max=imag;
		}
		else
		{
			br_ovcurrs[2*i]=-1.f;
			br_ovcurrs[2*i+1]=-1.f;
		}
	}
	
	/*
	bind a vertex buffer and initialize its data
	*/
	glClear(GL_COLOR_BUFFER_BIT);
	glBufferSubData(GL_ARRAY_BUFFER,6*nbranch*sizeof(float),2*nbranch*sizeof(float),br_ovcurrs);
	ov_curr_sc=glGetUniformLocation(ps_program,"ov_curr_sc");
	glUniform1f(ov_curr_sc,(GLfloat)(1.0/ov_curr_max));
	glDrawArrays(GL_LINES,0,6*ps->nbranch);
	glutSwapBuffers();
	glutPostRedisplay();
	
	free(br_ovcurrs);
	
	//prompt user to continue
	printf("t=%lg\nPress ENTER to continue...\n",ts);
	getchar();
}

/*
GLUT window resize callback
*/
void Reshape(int w, int h)
{	
	/*
	update the projection matrix to use the right
	aspect ratio for the new window
	*/
	float ortho_w,aspect;
	GLint m_proj_h;
	aspect=(float)w/h;
	ortho_w=aspect*(t-b);
	r=ortho_w/2.f;
	l=-r;
	l=xmin-0.25*(xmax-xmin);
	r=xmax+0.25*(xmax-xmin);
	t=ymax+0.25*(ymax-ymin);
	b=ymin-0.25*(ymax-ymin);
	m_proj[0][0]=2.f/(r-l);
	m_proj[0][3]=-(r+l)/(r-l);
	m_proj[1][1]=2.f/(t-b);
	m_proj[1][3]=-(t+b)/(t-b);
	m_proj[2][2]=-2.f/(f-n);
	m_proj[2][3]=-(f+n)/(f-n);

	m_proj_h=glGetUniformLocation(ps_program,"m_proj");
	glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
	
	//update the window size and viewport	
	win_w=w;
	win_h=h;	
	glViewport(0,0,w,h);
	glutSwapBuffers();
	glutPostRedisplay();
}

/*
GLUT mouse scroll zoom function
*/
void MouseFunc(int button,int state,int x,int y)
{	
	GLint m_proj_h;
	switch(button)
	{
		case 3:
			xmin*=0.9;
			xmax*=0.9;
			ymin*=0.9;
			ymax*=0.9;
			if(state==GLUT_UP)break;
			l=xmin-0.25*(xmax-xmin);
			r=xmax+0.25*(xmax-xmin);
			t=ymax+0.25*(ymax-ymin);
			b=ymin-0.25*(ymax-ymin);
			m_proj[0][0]=2.f/(r-l);
			m_proj[0][3]=-(r+l)/(r-l);
			m_proj[1][1]=2.f/(t-b);
			m_proj[1][3]=-(t+b)/(t-b);
			m_proj[2][2]=-2.f/(f-n);
			m_proj[2][3]=-(f+n)/(f-n);
			m_proj_h=glGetUniformLocation(ps_program,"m_proj");
			glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
			break;		
		case 4:
			if(state==GLUT_UP)break;
			l=xmin-0.25*(xmax-xmin);
			r=xmax+0.25*(xmax-xmin);
			t=ymax+0.25*(ymax-ymin);
			b=ymin-0.25*(ymax-ymin);
			m_proj[0][0]=2.f/(r-l);
			m_proj[0][3]=-(r+l)/(r-l);
			m_proj[1][1]=2.f/(t-b);
			m_proj[1][3]=-(t+b)/(t-b);
			m_proj[2][2]=-2.f/(f-n);
			m_proj[2][3]=-(f+n)/(f-n);
			m_proj_h=glGetUniformLocation(ps_program,"m_proj");
			glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
			break;
	}
	
	glutSwapBuffers();
	glutPostRedisplay();
}

/*
main
*/
int main(int argc,char** argv)
{	
	char ps_file[256]={0},ps_name_f[256]={0};
	BRANCH *branches;
	int i,ncmpt,nbus;
	cs_di **bbuses;
	double *vang;
	
	//Parse command line arguments
	//if(!(argc%2))Usage();
	while ((argc>1)&&(argv[1][0]=='-'))
	{		
		switch (argv[1][1]){
			case 'f':
				strcpy(ps_file,&argv[2][0]);
				argv+=2;
				argc-=2;
				break;
			case 'h':
				Usage();
			default:
				printf("Unknown flag: %s\n", argv[1]);
				Usage();
		}
	}
	
	//load and display ps
	ps=load_ps(ps_file);
	ps_print(ps,1);
	
	// Setup GLUT and OpenGL
	glutInit(&argc,argv);
	glutInitContextVersion(4,0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE);
	glutInitWindowSize(win_w,win_h);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);	
	glutCreateWindow("DCSIMSEP");
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutCloseFunc(Cleanup);
	glutMouseFunc(MouseFunc);

	//load gl extensions
	glewExperimental=GL_TRUE;
	err=glewInit();
	if(err!=GLEW_OK)
	{
		printf("GLEW Error: %s",glewGetErrorString(err));
		return 1;
	}

	//initialize the scene and begin rendering
	InitScene();
	glClearColor(1.0f,1.0f,1.0f,1.0f);
	//glClearColor(0.1f,0.2f,0.3f,0.0f);
	//glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glLineWidth(10.0f);
	glutMainLoop();
}
