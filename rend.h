#include "disp.h" /* include your own disp.h file (e.g. hw1)*/

/* Camera defaults */
#define	DEFAULT_FOV	35.0
/* world coords for image plane origin */
#define	DEFAULT_IM_Z	(-10.0)  
#define	DEFAULT_IM_Y	(5.0)    
#define	DEFAULT_IM_X	(-10.0)
/* default look-at point = 0,0,0 */

#define	DEFAULT_AMBIENT	{0.1, 0.1, 0.1}
#define	DEFAULT_DIFFUSE	{0.7, 0.6, 0.5}
#define	DEFAULT_SPECULAR	{0.2, 0.3, 0.4}
#define	DEFAULT_SPEC		32

#define	MATLEVELS	100	/* how many matrix pushes allowed */
#define	MAX_LIGHTS	20	/* how many lights allowed */

#define PI 3.14159265

#ifndef GZRENDER
#define GZRENDER
typedef struct {			/* define a renderer */
  GzDisplay	*display;
  GzCamera		camera;
  short		matlevel;  /* top of stack -> current xform */
  GzMatrix		Ximage[MATLEVELS];	/* stack of xforms (Xsm) */
  GzMatrix		Xnorm[MATLEVELS];	/* xforms for norms (Xim) */
  GzMatrix		Xsp;		 /* NDC to screen (pers-to-screen) */
  GzColor		flatcolor;  /* color for flat shaded triangles */
  int			interp_mode;
  int			numlights;
  GzLight		lights[MAX_LIGHTS];
  GzLight		ambientlight;
  GzColor		Ka, Kd, Ks;
  float		    spec;		/* specular power */
  GzTexture		tex_fun;    /* tex_fun(float u, float v, GzColor color) */

  float Xoffset;
  float Yoffset; 
  float weight;
}  GzRender;
#endif

// Function declaration
// HW2
int GzNewRender(GzRender **render, GzDisplay *display);
int GzFreeRender(GzRender *render);
int GzBeginRender(GzRender	*render);
int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer *valueList);
int GzPutTriangle(GzRender *render, int	numParts, GzToken *nameList,
	GzPointer *valueList);
void sort_vertices(GzCoord verts[3], int &max_y_tri, int &sec_y_tri, int &third_y_tri);
void get_bounding_box(GzCoord verts[3], int &x_min, int &y_min, int &x_max, int &y_max);
void compute_abc(GzCoord verts[3], int tail, int head, float &a, float &b, float &c);
short	ctoi(float color);
float get_min(float x, float y, float z);
float get_max(float x, float y, float z);
void handle_special(GzCoord verts[3], int &left, int &right, int &top);


// HW3
void get_vector(GzCoord verts[3], GzCoord vec, int tail, int head);
int GzPutCamera(GzRender *render, GzCamera *camera);
int GzPushMatrix(GzRender *render, GzMatrix	matrix);
int GzPopMatrix(GzRender *render);
void MatrixXnorm(GzMatrix out, GzMatrix in);
void matrix_mul(GzMatrix result, GzMatrix mat1, GzMatrix mat2);
void set_identity(GzMatrix mat);
void vector_unit( GzCoord vec);
void vector_mult(GzCoord result, float val, GzCoord vec2);
void vector_sub(GzCoord result, GzCoord left, GzCoord right);
void vector_sum(GzCoord result, GzCoord left, GzCoord right);
float dot_product(GzCoord mat1, GzCoord mat2);
void cross_product(GzCoord result, GzCoord left, GzCoord right);
void xform(GzCoord result, GzMatrix mat, GzCoord coords);
float deg_to_rads(float degree);

// Object Translation
int GzRotXMat(float degree, GzMatrix mat);
int GzRotYMat(float degree, GzMatrix mat);
int GzRotZMat(float degree, GzMatrix mat);
int GzTrxMat(GzCoord translate, GzMatrix mat);
int GzScaleMat(GzCoord scale, GzMatrix mat);

// HW4 
void shading(GzRender *render, GzColor output, GzCoord normal);
void phong_shading(GzRender *render, GzColor output, GzCoord normal, GzColor color);


// HW5
int GzFreeTexture();

void shiftVerts();