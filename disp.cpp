/*   CS580 HW1 display functions to be completed   */

#include   "stdafx.h"  
#include	"Gz.h"
#include	"disp.h"


int GzNewFrameBuffer(char** framebuffer, int width, int height)
{
/* HW1.1 create a framebuffer for MS Windows display:
 -- allocate memory for framebuffer : 3 bytes(b, g, r) x width x height
 -- pass back pointer 
 */

	*framebuffer = new char[height*width * 3];

	return GZ_SUCCESS;
}

int GzNewDisplay(GzDisplay	**display, int xRes, int yRes)
{
/* HW1.2 create a display:
  -- allocate memory for indicated resolution
  -- pass back pointer to GzDisplay object in display
*/


	GzDisplay *temp_disp=new GzDisplay;
	(*temp_disp).xres = xRes;
	(*temp_disp).yres = yRes;
	(*temp_disp).fbuf = new GzPixel[xRes*yRes];

	*display= temp_disp;

	return GZ_SUCCESS;
}


int GzFreeDisplay(GzDisplay	*display)
{
/* HW1.3 clean up, free memory */
	
	delete [] (* display).fbuf;

	return GZ_SUCCESS;
}


int GzGetDisplayParams(GzDisplay *display, int *xRes, int *yRes)
{
/* HW1.4 pass back values for a display */

	*xRes = (*display).xres;
	*yRes = (*display).yres;


	return GZ_SUCCESS;
}


int GzInitDisplay(GzDisplay	*display)
{
/* HW1.5 set everything to some default values - start a new frame */

	int x = (*display).xres;
	int y = (*display).yres;

	GzIntensity default = 1000; 
	GzIntensity defa = 1;
	GzDepth defz = MAXINT;

	GzPixel defPixel = { default, default, default, defa, defz };

	for (int i = 0; i < (x*y); i++)
	{
		(*display).fbuf[i] = defPixel;
	}

	return GZ_SUCCESS;
}


int GzPutDisplay(GzDisplay *display, int i, int j, GzIntensity r, GzIntensity g, GzIntensity b, GzIntensity a, GzDepth z)
{
/* HW1.6 write pixel values into the display */



	int x = (*display).xres;
	int y = (*display).yres;

//bounds checking
	if (i<0)
		i = 0;
	if (j<0)
		j = 0;

	if (i>=x)
		i = x-1;
	if (j>=y)
		j = y-1;


	//upper-bound check for colors
	if (r>4095)
		r = 4095;

	if (g>4095)
		g = 4095;

	if (b>4095)
		b = 4095;


	//lower-bound check for colors
	if (r<0)
		r = 0;

	if (g<0)
		g = 0;

	if (b<0)
		b = 0;


	GzPixel defPixel = { r, g, b, a, z };

	int index = (j*y) + i; 


	if(i>=0 && i < (x) && j>=0 && j<y)
		(*display).fbuf[index] = defPixel;

	
	return GZ_SUCCESS;
}


int GzGetDisplay(GzDisplay *display, int i, int j, GzIntensity *r, GzIntensity *g, GzIntensity *b, GzIntensity *a, GzDepth *z)
{
/* HW1.7 pass back a pixel value to the display */
	int x = (*display).xres;
	GzPixel temp = (*display).fbuf[(i*x) + j];

	*r = temp.red;
	*g = temp.green;
	*b = temp.blue;
	*a = temp.alpha;
	*z = temp.z;



	return GZ_SUCCESS;
}


int GzFlushDisplay2File(FILE* outfile, GzDisplay *display)
{

/* HW1.8 write pixels to ppm file -- "P6 %d %d 255\r" */

	fprintf (outfile, "P6 %d %d 255\n", (*display).xres, (*display).yres);

	int x = (*display).xres;
	int y = (*display).yres;
	unsigned char rgb[3];
	
	for (int i = 0; i < x*y; i++)
	{
		rgb[0] = (((*display).fbuf[i]).red)>>4;
		rgb[1] = (((*display).fbuf[i]).green) >> 4;
		rgb[2]= (((*display).fbuf[i]).blue) >> 4;

		//fprintf(outfile, "%d%d%d", r, g, b);
		fwrite(rgb, sizeof(unsigned char), sizeof(rgb), outfile);
	}
	

		/*
		for (int j = 0; j < y; j ++)
		{
			int index = (i*x) + j;

			if (index >= (x*y))
				index = x*y - 1;

			else if (index < 0)
				index = 0;

			int r = (((*display).fbuf[index]).red) >> 4;
			int g = 0; //(((*display).fbuf[index]).green) >> 4;
			int b = 55;// (((*display).fbuf[index]).blue) >> 4;

			fprintf(outfile, "%d %d %d", r, g, b);
		}

		*/


	return GZ_SUCCESS;
}

int GzFlushDisplay2FrameBuffer(char* framebuffer, GzDisplay *display)
{

/* HW1.9 write pixels to framebuffer: 
	- put the pixels into the frame buffer
	- CAUTION: when storing the pixels into the frame buffer, the order is blue, green, and red 
	- NOT red, green, and blue !!!
*/
	
	int x = (*display).xres;
	int y = (*display).yres;

	int ibuf = 0;
	for (int i = 0; i < (x*y*3); i+=3)
	{
		
			framebuffer[i] =  (((*display).fbuf[ibuf]).blue) >> 4;
			framebuffer[i+1] = (((*display).fbuf[ibuf]).green) >> 4;
			framebuffer[i+2] = (((*display).fbuf[ibuf]).red) >> 4;
			ibuf++;
	}
	

	return GZ_SUCCESS;
}