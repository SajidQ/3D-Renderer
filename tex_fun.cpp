/* Texture functions for cs580 GzLib	*/
#include    "stdafx.h" 
#include	"stdio.h"
#include	"Gz.h"

GzColor	*image=NULL;
int xs, ys;
int reset = 1;

struct complexNum
{
public:
	float r;
	float i;
};


int convert(int x, int y)
{
	return x + xs*y;
}


/* Image texture function */
int tex_fun(float u, float v, GzColor color)
{
  unsigned char		pixel[3];
  unsigned char     dummy;
  char  		foo[8];
  int   		i, j;
  FILE			*fd;

  if (reset) {          /* open and load texture file */
    fd = fopen ("texture", "rb");
    if (fd == NULL) {
      fprintf (stderr, "texture file not found\n");
      exit(-1);
    }
    fscanf (fd, "%s %d %d %c", foo, &xs, &ys, &dummy);
    image = (GzColor*)malloc(sizeof(GzColor)*(xs+1)*(ys+1));
    if (image == NULL) {
      fprintf (stderr, "malloc for texture image failed\n");
      exit(-1);
    }

    for (i = 0; i < xs*ys; i++) {	/* create array of GzColor values */
      fread(pixel, sizeof(pixel), 1, fd);
      image[i][RED] = (float)((int)pixel[RED]) * (1.0 / 255.0);
      image[i][GREEN] = (float)((int)pixel[GREEN]) * (1.0 / 255.0);
      image[i][BLUE] = (float)((int)pixel[BLUE]) * (1.0 / 255.0);
      }

    reset = 0;          /* init is done */
	fclose(fd);
  }

/* bounds-test u,v to make sure nothing will overflow image array bounds */
  if (u < 0)
	  u = 0; 
  else if (u > 1)
	  u = 1; 
  if (v < 0)
	  v = 0;
  else if (v > 1)
	  v = 1;

  
/* determine texture cell corner values and perform bilinear interpolation */
  float tempu = u*(xs - 1);
  float tempv = v*(ys - 1);
  float s = tempu - floor(tempu);
  float t = tempv - floor(tempv);
 

/* set color to interpolated GzColor value and return */
  for (int i = 0; i < 3; i++)
  {
	  color[i] = s*t*image[convert(ceil(tempu), ceil(tempv))][i] + (1-s)*t*image[convert(floor(tempu), ceil(tempv))][i] +s*(1-t)*image[convert(ceil(tempu), floor(tempv))][i] + (1-s)*(1-t)*image[convert(floor(tempu), floor(tempv))][i];
  }

  return GZ_SUCCESS;
}




/* Procedural texture function */
int ptex_fun(float u, float v, GzColor color)
{

	float W = 1, H = 1.5;
	complexNum x, c;
	x.r = (u - (W / 2)) / (W / 2);
	x.i = (v - (H / 2)) / (H / 2);

	c.r = -.7;
	c.i = .27015;

	int N = 200;
	float tempx_u, tempx_v;
	int i;
	for (i = 0; i < N; i++)
	{
		float xr = (x.r*x.r) - (x.i*x.i) + c.r;
		float xi = (x.r*x.i) + (x.i*x.r) + c.i;
		if ((x.r* x.r + x.i*x.i)>2.0)
		{
			break;
		}

		x.r = xr;
		x.i = xi;

	}

	float len = sqrt(x.r *x.r + x.i*x.i);
	if (i == N)
	{
		color[0] = len;
		color[1] = len;
		color[2] = len;
	}
	else
	{
		float tempz = i / (float)N;
		color[0] = tempz * 4;
		color[1] = tempz * 6;
		color[2] = tempz * 8;
	}
	
	return GZ_SUCCESS;
}

/* Free texture memory */
int GzFreeTexture()
{
	if(image!=NULL)
		free(image);
	return GZ_SUCCESS;
}

