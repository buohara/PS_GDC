//includes

#include "varint.h"

/*
global variables
*/
GLuint vshader,vcolorshader,fshader,mesh_program,color_program,vao,vbo,idxbuff,vfo;
GLenum err;
char *vsh_src=NULL,*vsh_color_src=NULL,*fsh_src=NULL,*vsh_file="shaders/pass_thru.vert",*fsh_file="shaders/pass_thru.frag";
int win_w=1280,win_h=768,nsmp,render=0;
float q_old[4]={0.0,0.0,0.0,1.0},q_new[4]={0.0,0.0,0.0,1.0},q_f[4]={0.0,0.0,0.0,1.0},mx_old=0.0,my_old=0.0,mx_new=0.0,my_new=0.0,r=0.5,l=-0.5,t=0.5,b=-0.5,n=2.0,f=5.0,*bctrs=NULL,*f1=NULL,*f2=NULL,*f3=NULL,ts=0.f;
mesh *msh=NULL;
kchn *chn=NULL,*fidcs=NULL;
cs_di *maxwell=NULL;

/*
initial MVP matrices
*/
float m_rot[4][4]={
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,0.0,1.0}
};

float m_trans[4][4]={
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,-2.5},
	{0.0,0.0,0.0,1.0}
};

float m_proj[4][4]={
	{1.0,0.0,0.0,0.0},
	{0.0,1.0,0.0,0.0},
	{0.0,0.0,1.0,0.0},
	{0.0,0.0,-1.0,0.0}
};

/*
functions declarations
*/
static void Usage(void);
static void Render(void);
static void Reshape(int w,int h);
static void MouseFunc(int button,int state,int x,int y);
static int InitScene(const char* mesh_file);
static void Cleanup(void);
static void IdleFunc(void);
static void GetErr(void);
static void KeyboardFunc(unsigned char key,int x,int y);

/*
get and display OpenGL errors
*/
inline void GetErr(void){
	err=glGetError();
	if(err!=GL_NO_ERROR){
		printf("Error: %s\n",gluErrorString(err));
	} else{
		printf("No OpenGL Errors\n");
	}
}

/*
read a shader from file
*/
char* LoadShader(const char* shader_file){
    struct stat stat_buf;
    FILE* fp=fopen(shader_file, "r");
    char* buf;

    stat(shader_file,&stat_buf);
    buf=(char*)malloc(stat_buf.st_size+1*sizeof(char));
	if(!buf) return NULL;
    fread(buf,1,stat_buf.st_size,fp);
    buf[stat_buf.st_size]='\0';
    fclose(fp);
    return buf;
}

/*
Prepare the scene. This function loads a mesh from file and populates
its associated data strucutres, loads shaders, prepares GL vertex and
index buffers, and loads MVP matrices.
*/
int InitScene(const char* mesh_file){
	
	int i,j,k,*smps,ele_sz;
	float *msh_verts,*chn_verts,*chn_field,field_val; 
	
	//boundary used for computing maxwell update matrix
	bdry *bdry;
	
	//mvp matrix uniforms
	GLint m_mod,m_view;
	
	//allocate the global mesh struct
	msh=malloc(sizeof(mesh));
	msh->verts=NULL;

	//load 2-chain and mesh verts from file
	chn=kchn_load_2chn(mesh_file,msh,1);
	msh_verts=msh->verts;
	nsmp=chn->nsmp;

	//calculate the chain boundary and maxwell matrix
	bdry=kchn_bdry(chn);
	maxwell=maxwell_matrix(chn,bdry);
	
	//discard bdry information, 
	kchn_free_bdry(bdry);
	bdry=NULL;
	
	//grab the chain vertices by indexing into the mesh verts
	ele_sz=chn->ele_sz;
	smps=chn->smps;
	chn_verts=malloc(9*nsmp*sizeof(float));
	for(i=0;i<nsmp;i++){
		for(j=0;j<3;j++){
			for(k=0;k<3;k++)chn_verts[9*i+3*j+k]=msh_verts[3*(smps[i*ele_sz+j]-1)+k];
		}
	}
	
	/*
	grab the 2-chain vertex indices for rendering. we can toss the original
	chain when we're done
	*/
	kchn_free(chn);
	chn=NULL;

	//generate and bind a vertex array
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

	//generate, bind, and fill a vertex buffer with mesh vertex positions
	glGenBuffers(1,&vbo);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER,12*nsmp*sizeof(float),NULL,GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,9*nsmp*sizeof(float),chn_verts);

	//free cpu chain vertex memory
	free(chn_verts);
	chn_verts=NULL;

	/*
	chain and vertex data have been pushed to GPU memory and are no longer required.
	free up their memory.
	*/
	kchn_free(fidcs);
	FreeMesh(msh);
	msh=NULL;
	msh_verts=NULL;

	//get rid of barycenter vertices in cpu memory
	free(bctrs);
	bctrs=NULL;

	/*
	allocate field strength memory on the host. we need 2*nsmp values so we can
	ping-pong the updates. 
	*/
	f1=malloc(5*nsmp*sizeof(float));
	f2=f1+nsmp;
	f3=f1+2*nsmp;

	//initialize the field with dummy values and push them to the gpu
	
	for(i=0;i<nsmp;i++)f1[i]=f2[i]=f3[3*i+0]=f3[3*i+1]=f3[3*i+2]=0.0;
	f1[0]=f2[0]=f3[0]=f3[1]=f3[2]=0.1;
	glBufferSubData(GL_ARRAY_BUFFER,9*nsmp*sizeof(float),3*nsmp*sizeof(float),f3);

	/*
	set offsets for the position and color attributes of the field. positions form
	the first 3/4 of the vfo buffer and the colors for the last 1/4.
	*/ 
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(const GLvoid*)0);
	glVertexAttribPointer(1,1,GL_FLOAT,GL_FALSE,0,(const GLvoid*)(9*nsmp*sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	//load shader strings from file
	vsh_src=LoadShader(vsh_file);
	fsh_src=LoadShader(fsh_file);
	
	//compile the mesh vertex (pass through) shader
	vshader=glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader,1,(const GLchar**)&vsh_src,NULL);
	glCompileShader(vshader);

	//compile the fragment shader
	fshader=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader,1,(const GLchar**)&fsh_src,NULL);
	glCompileShader(fshader);	

	//create the mesh program.
	mesh_program=glCreateProgram();	
	glAttachShader(mesh_program,vshader);
	glAttachShader(mesh_program,fshader);
	glLinkProgram(mesh_program);
	glUseProgram(mesh_program);

	//initialize mvp matrices for both shader programs
	m_mod=glGetUniformLocation(mesh_program,"m_mod");
	glUniformMatrix4fv(m_mod,1,GL_FALSE,(const GLfloat*)m_rot);

	m_view=glGetUniformLocation(mesh_program,"m_view");
	glUniformMatrix4fv(m_view,1,GL_TRUE,(const GLfloat*)m_trans);

	//free up shader source memory
	free(vsh_src);
	free(vsh_color_src);
	free(fsh_src);
	vsh_src=fsh_src=vsh_color_src=NULL;
	
	return 0;
}

//release our GL resources
void Cleanup(void){

	//unbind and delete the vertex buffer	
	glBindBuffer(GL_ARRAY_BUFFER,0);
	glDeleteBuffers(1,&vbo);
	glDeleteBuffers(1,&vfo);

	//unbind and delete index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);	
	glDeleteBuffers(1,&idxbuff);

	//unbind and delete the vertex array buffer
	glBindVertexArray(0);
	glDeleteVertexArrays(1,&vao);

	//detach the shader program and shaders
	glUseProgram(0);
	glDetachShader(mesh_program,vshader);
	glDetachShader(mesh_program,fshader);

	//delete shader program and shaders
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(mesh_program);

	//free the maxwell matrix and field strength values
	cs_spfree(maxwell);
	maxwell=NULL;
	free(f1);
	f1=f2=f3=NULL;
}

//print command line flag usage
void Usage(void){
	printf("Usage:\n");
	printf(" -f <mesh_file_name>\n");
	printf(" -d <name>\n");
	exit(8);
}

/*
GLUT render function
*/
void Render(void){

	int i,n=maxwell->n;
	float *ftmp;
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	if(render){	
		cs_gaxpy_float(maxwell,f1,f2);
		ftmp=f1;
		f1=f2;
		f2=ftmp;
		for(i=0;i<n;i++){
			f3[3*i]=f3[3*i+1]=f3[3*i+2]=f1[i];
			f2[i]=-f2[i];
		}
		//f1[0]=0.0;
		glBufferSubData(GL_ARRAY_BUFFER,9*nsmp*sizeof(float),3*nsmp*sizeof(float),f3);
		f1[0]=f2[0]=f3[0]=f3[1]=f3[2]=cos(1.25*ts);//exp(-(ts-10.f)*(ts-10.0f)/3.f);//cos(2.0*ts);
		ts+=0.1;
		//printf("%g %g %g\n",f1[0],f1[1],f1[82]);
	}

	//bind the mesh geometry buffer, the pass-through program, and draw	
	glDrawArrays(GL_TRIANGLES,0,3*nsmp);
	glutSwapBuffers();
	glutPostRedisplay();
}

/*
GLUT idle function
*/
void IdleFunc(void){
	static int prev_time1,prev_time2;
	char speed[64];
	int new_time,elapsed1,elapsed2;
	
	//calculate rendering time in ms
	new_time=glutGet(GLUT_ELAPSED_TIME);
	elapsed1=new_time-prev_time1;
	prev_time1=new_time;
	elapsed2=new_time-prev_time2;
	//if(elapsed2>100)render=1;
	if(elapsed2>500){
		prev_time2=new_time;
		sprintf(speed,"VarInt - Frame Render Time: %d ms",elapsed1);
		glutSetWindowTitle(speed);
	}
}

/*
GLUT window resize callback
*/
void Reshape(int w, int h){
	
	//update the projection matrix to use the right
	//aspect ratio for the new window
	float ortho_w,aspect;
	GLint m_proj_h;
	aspect=(float)w/h;
	ortho_w=aspect*(t-b);
	r=ortho_w/2.0;
	l=-r;
	m_proj[0][0]=2.0*n/(r-l);
	m_proj[0][3]=-(r+l)/(r-l);
	m_proj[1][1]=2.0*n/(t-b);
	m_proj[1][3]=-(t+b)/(t-b);
	m_proj[2][2]=-(f+n)/(f-n);
	m_proj[2][3]=-2.0*f*n/(f-n);

	m_proj_h=glGetUniformLocation(mesh_program,"m_proj");
	glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
	
	//update the window size and viewport	
	win_w=w;
	win_h=h;	
	glViewport(0,0,w,h);
	glutPostRedisplay();
}

/*
GLUT mouse click callback. get mouse coordinates and normalize them
for trackball motion.
*/
void MouseFunc(int button, int state, int x, int y){	
	
	GLint m_proj_h;
	
	switch(button){
		case GLUT_LEFT_BUTTON:
			switch(state){
				case GLUT_DOWN:
					mx_new=2.0*(x-((float)win_w/2.0))/win_w;
					my_new=-2.0*(y-((float)win_h/2.0))/win_h;
					break;
				default:
					break;
			}
		case 3:
			if(state==GLUT_UP)break;
			n*=1.1;
			m_proj[0][0]=2.0*n/(r-l);
			m_proj[1][1]=2.0*n/(t-b);
			m_proj[1][3]=-(t+b)/(t-b);
			m_proj[2][2]=-(f+n)/(f-n);
			m_proj[2][3]=-2.0*f*n/(f-n);
			m_proj_h=glGetUniformLocation(mesh_program,"m_proj");
			glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
			break;
		case 4:
			if(state==GLUT_UP)break;
			n*=0.9;
			m_proj[0][0]=2.0*n/(r-l);
			m_proj[1][1]=2.0*n/(t-b);
			m_proj[1][3]=-(t+b)/(t-b);
			m_proj[2][2]=-(f+n)/(f-n);
			m_proj[2][3]=-2.0*f*n/(f-n);
			m_proj_h=glGetUniformLocation(mesh_program,"m_proj");
			glUniformMatrix4fv(m_proj_h,1,GL_TRUE,(const GLfloat*)m_proj);
			break;
		default:
			break;		
	}
}

/*
GLUT mouse motion func. Compute model rotation matrix using trackball
*/
void MotionFunc(int x, int y){

	GLint m_mod;

	mx_old=mx_new;
	my_old=my_new;
	mx_new=2.0*(x-((float)win_w/2.0))/win_w;
	my_new=-2.0*(y-((float)win_h/2.0))/win_h;

	q_old[0]=q_new[0];
	q_old[1]=q_new[1];
	q_old[2]=q_new[2];
	q_old[3]=q_new[3];

	trackball(q_new,mx_old,my_old,mx_new,my_new);
	add_quats(q_new,q_old,q_f);
	build_rotmatrix(m_rot,q_f);

	q_new[0]=q_f[0];
	q_new[1]=q_f[1];
	q_new[2]=q_f[2];
	q_new[3]=q_f[3];

	m_mod=glGetUniformLocation(mesh_program,"m_mod");
	glUniformMatrix4fv(m_mod,1,GL_FALSE,(const GLfloat*)m_rot);

	glutPostRedisplay();
}

/*
GLUT keyboard function
*/
void KeyboardFunc(unsigned char key,int x,int y){
	switch(key){
		case 'r':
			if(render)render=0;
			else render=1;
			break;
		default:
			break;
	}
}

/*
main
*/
int main(int argc, char** argv){

	char mesh_file[256];

	//Parse command line arguments
	if(!(argc%2))Usage();
	while ((argc>1)&&(argv[1][0]=='-')){		
		switch (argv[1][1]){
			case 'f':
				strcpy(mesh_file,&argv[2][0]);
				break;
			case 'd':
				printf("%s\n",&argv[2][0]);
				break;
			case 'h':
				Usage();
			default:
				printf("Unknown flag: %s\n", argv[1]);
				Usage();
		}
		argv+=2;
		argc-=2;
	}

	// Setup GLUT and OpenGL
	glutInit(&argc,argv);
	glutInitContextVersion(4,0);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(win_w,win_h);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE,GLUT_ACTION_GLUTMAINLOOP_RETURNS);	
	glutCreateWindow("VarInt");
	
	glutDisplayFunc(Render);
	glutReshapeFunc(Reshape);
	glutCloseFunc(Cleanup);
	glutIdleFunc(IdleFunc);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutKeyboardFunc(KeyboardFunc);

	//load gl extensions
	glewExperimental=GL_TRUE;
	err=glewInit();
	if(err!=GLEW_OK){
		printf("GLEW Error: %s",glewGetErrorString(err));
		return(1);
	}

	//initialize the scene and begin rendering
	InitScene(mesh_file); 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(1.0f,1.0f,1.0f,0.0f);
	//glClearColor(0.1f,0.2f,0.3f,0.0f);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glutMainLoop();

	return 0;
}
