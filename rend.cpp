/* CS580 Homework 3 */

#include	"stdafx.h"
#include	"stdio.h"
#include	"math.h"
#include	"Gz.h"
#include	"rend.h"


int GzRotXMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along x axis
// Pass back the matrix using mat value
	set_identity(mat);
	mat[1][1] = cos(deg_to_rads(degree));
	mat[1][2] = -sin(deg_to_rads(degree));
	mat[2][1] = sin(deg_to_rads(degree));
	mat[2][2] = cos(deg_to_rads(degree));

	return GZ_SUCCESS;
}


int GzRotYMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along y axis
// Pass back the matrix using mat value

	set_identity(mat);

	mat[0][0] = cos(deg_to_rads(degree));
	mat[2][0] = -sin(deg_to_rads(degree));
	mat[0][2] = sin(deg_to_rads(degree));
	mat[2][2] = cos(deg_to_rads(degree));

	return GZ_SUCCESS;
}


int GzRotZMat(float degree, GzMatrix mat)
{
// Create rotate matrix : rotate along z axis
// Pass back the matrix using mat value

	set_identity(mat);

	mat[0][0] = cos(deg_to_rads(degree));
	mat[0][1] = -sin(deg_to_rads(degree));
	mat[1][0] = sin(deg_to_rads(degree));
	mat[2][2] = cos(deg_to_rads(degree));


	return GZ_SUCCESS;
}


int GzTrxMat(GzCoord translate, GzMatrix mat)
{
// Create translation matrix
// Pass back the matrix using mat value
	set_identity(mat);

	mat[0][3] = translate[0];
	mat[1][3] = translate[1];
	mat[2][3] = translate[2]; 

	return GZ_SUCCESS;
}


int GzScaleMat(GzCoord scale, GzMatrix mat)
{
// Create scaling matrix
// Pass back the matrix using mat value

	set_identity(mat);

	mat[0][0] = scale[0];
	mat[1][1] = scale[1];
	mat[2][2] = scale[2]; 

	return GZ_SUCCESS;
}


//----------------------------------------------------------
// Begin main functions

int GzNewRender(GzRender **render, GzDisplay *display)
{
/*  
- malloc a renderer struct 
- setup Xsp and anything only done once 
- save the pointer to display 
- init default camera 
*/ 

	GzRender *newRend = new GzRender;

	(*newRend).display = display;

	(*newRend).camera.lookat[X] = 0;
	(*newRend).camera.lookat[Y] = 0;
	(*newRend).camera.lookat[Z] = 0;

	(*newRend).camera.position[X] = DEFAULT_IM_X;
	(*newRend).camera.position[Y] = DEFAULT_IM_Y;
	(*newRend).camera.position[Z] = DEFAULT_IM_Z;

	(*newRend).camera.worldup[X] = 0;
	(*newRend).camera.worldup[Y] = 1;
	(*newRend).camera.worldup[Z] = 0;

	(*newRend).camera.FOV = DEFAULT_FOV;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			(*newRend).camera.Xiw[i][j] = 0; 
			(*newRend).camera.Xpi[i][j] = 0;
			(*newRend).Xsp[i][j] = 0;
		}
	}

	for (int i = 0; i < 3; i++)
	{
		(*newRend).flatcolor[i] = 0;
		(*newRend).Ka[i] = 0;
		(*newRend).Kd[i] = 0;
		(*newRend).Ks[i] = 0;
	}

	(*newRend).matlevel = 0;
	(*newRend).interp_mode = 0;
	(*newRend).numlights = 0;
	(*newRend).spec = 0;

	(*newRend).Xoffset = 0;
	(*newRend).Yoffset = 0;

	(*newRend).tex_fun = NULL;

	*render = newRend;


	return GZ_SUCCESS;

}


int GzFreeRender(GzRender *render)
{
/* 
-free all renderer resources
*/
	return GZ_SUCCESS;
}


int GzBeginRender(GzRender *render)
{
/*  
- setup for start of each frame - init frame buffer color,alpha,z
- compute Xiw and projection xform Xpi from camera definition 
- init Ximage - put Xsp at base of stack, push on Xpi and Xiw 
- now stack contains Xsw and app can push model Xforms when needed 
*/ 

	//initialize frame buffer
	GzInitDisplay((*render).display);

	
	//compute Xsp
	//GzMatrix Xsp;
	set_identity((*render).Xsp);
	(*render).Xsp[0][0] = (*render).display->xres / 2.0f;
	(*render).Xsp[1][1] = -1.0f*(*render).display->yres / 2.0f;
	(*render).Xsp[2][2] = MAXINT;
	(*render).Xsp[0][3] = (*render).display->xres / 2.0f;
	(*render).Xsp[1][3] = (*render).display->yres / 2.0f;
	GzPushMatrix(render, (*render).Xsp);

	set_identity((*render).Xnorm[(*render).matlevel-1]);

	//compute Xpi
	float d = 1.0f / tan(deg_to_rads(((*render).camera).FOV) / 2.0f);
	set_identity((*render).camera.Xpi);
	((*render).camera).Xpi[2][2] = 1 / d; 
	((*render).camera).Xpi[3][2] = 1 / d;
	GzPushMatrix(render, (*render).camera.Xpi);

	set_identity((*render).Xnorm[(*render).matlevel - 1]);


	//computer Xiw
	GzCoord z;
	vector_sub(z, (*render).camera.lookat, (*render).camera.position);
	vector_unit(z);
	
	GzCoord y, temp; 
	vector_mult(temp, dot_product((*render).camera.worldup, z), z);
	vector_sub(y, (*render).camera.worldup, temp);
	vector_unit(y);

	GzCoord x; 
	cross_product(x, y, z);

	set_identity((*render).camera.Xiw);
	(*render).camera.Xiw[0][0] = x[X];
	(*render).camera.Xiw[0][1] = x[Y];
	(*render).camera.Xiw[0][2] = x[Z];
	(*render).camera.Xiw[0][3] = -1.0f*dot_product(x, ((*render).camera).position);

	(*render).camera.Xiw[1][0] = y[X];
	(*render).camera.Xiw[1][1] = y[Y];
	(*render).camera.Xiw[1][2] = y[Z];
	(*render).camera.Xiw[1][3] = -1.0f*dot_product(y, ((*render).camera.position));

	(*render).camera.Xiw[2][0] = z[X];
	(*render).camera.Xiw[2][1] = z[Y];
	(*render).camera.Xiw[2][2] = z[Z];
	(*render).camera.Xiw[2][3] = -1.0f*dot_product(z, ((*render).camera.position));
	GzPushMatrix(render, (*render).camera.Xiw); 
	
	return GZ_SUCCESS;
}



int GzPutCamera(GzRender *render, GzCamera *camera)
{
/*
- overwrite renderer camera structure with new camera definition
*/

	(*render).camera.FOV = (*camera).FOV;
	(*render).camera.lookat[0]= (*camera).lookat[0];
	(*render).camera.lookat[1] = (*camera).lookat[1];
	(*render).camera.lookat[2] = (*camera).lookat[2];

	(*render).camera.position[0] = (*camera).position[0];
	(*render).camera.position[1] = (*camera).position[1];
	(*render).camera.position[2] = (*camera).position[2];

	(*render).camera.worldup[0] = (*camera).worldup[0];
	(*render).camera.worldup[1] = (*camera).worldup[1];
	(*render).camera.worldup[2] = (*camera).worldup[2];

	return GZ_SUCCESS;	
}



int GzPushMatrix(GzRender *render, GzMatrix	matrix)
{
/*
- push a matrix onto the Ximage stack
- check for stack overflow
*/
	GzMatrix Xnorm_mat;
	MatrixXnorm(Xnorm_mat, matrix);

	if ((*render).matlevel == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				((*render).Ximage[0])[i][j] = matrix[i][j];
				((*render).Xnorm[0])[i][j] = Xnorm_mat[i][j];
			}
		}
	}

	else if ((*render).matlevel > 0 && (*render).matlevel<=MATLEVELS)
	{
		matrix_mul((*render).Ximage[(*render).matlevel], (*render).Ximage[((*render).matlevel-1)], matrix);
		matrix_mul((*render).Xnorm[(*render).matlevel], (*render).Xnorm[((*render).matlevel - 1)], Xnorm_mat);
	}

	//for Xnorm


	(*render).matlevel++;

	return GZ_SUCCESS;
}



int GzPopMatrix(GzRender *render)
{
/*
- pop a matrix off the Ximage stack
- check for stack underflow
*/

	if ((*render).matlevel > 0)
		(*render).matlevel--;
	else
		return GZ_FAILURE;

	return GZ_SUCCESS;
}


void MatrixXnorm(GzMatrix out, GzMatrix in)
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			out[i][j] = 0;

	float K = sqrt(in[0][0]* in[0][0] + in[0][1] * in[0][1] + in[0][2] * in[0][2]);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if(K>0)
				out[i][j] = in[i][j] / K;
		}
	}
}



int GzPutAttribute(GzRender	*render, int numAttributes, GzToken	*nameList, 
	GzPointer	*valueList) /* void** valuelist */
{
/*
- set renderer attribute states (e.g.: GZ_RGB_COLOR default color)
- later set shaders, interpolaters, texture maps, and lights
*/
	/*
	GzColor * color = (GzColor *)valueList[0];
	((*render).flatcolor)[0] = (*color)[0];
	((*render).flatcolor)[1] = (*color)[1];
	((*render).flatcolor)[2] = (*color)[2];
	*/

	for (int i = 0; i < numAttributes; i++)
	{
		if (nameList[i] == GZ_RGB_COLOR)
		{
			memcpy(&(*render).flatcolor, valueList[i], sizeof(GzColor));
		}

		else if (nameList[i] == GZ_DIRECTIONAL_LIGHT)
		{
			if ((*render).numlights <= MAX_LIGHTS)
			{
				GzLight * light = (GzLight *)valueList[i];
				(*render).lights[((*render).numlights)] = *(GzLight *)valueList[i];
				(*render).numlights++;
			}
		}
		else if (nameList[i]== GZ_AMBIENT_LIGHT)
		{
			memcpy(&(*render).ambientlight, valueList[i], sizeof(GzLight));
		}
		else if (nameList[i] == GZ_DIFFUSE_COEFFICIENT)
		{
			memcpy(&(*render).Kd, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_INTERPOLATE)
		{
			memcpy(&(*render).interp_mode, valueList[i], sizeof(int));
		}
		else if (nameList[i] == GZ_SPECULAR_COEFFICIENT)
		{
			memcpy(&(*render).Ks, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_AMBIENT_COEFFICIENT)
		{
			memcpy(&(*render).Ka, valueList[i], sizeof(GzColor));
		}
		else if (nameList[i] == GZ_DISTRIBUTION_COEFFICIENT)
		{
			memcpy(&(*render).spec, valueList[i], sizeof(float));
		}
		else if (nameList[i] == GZ_TEXTURE_MAP)
		{
			(*render).tex_fun=(GzTexture)valueList[i];;
		}
		else if (nameList[i] == GZ_AASHIFTX)
		{
			(*render).Xoffset = *(float*)valueList[i];
		}
		else if (nameList[i] == GZ_AASHIFTY)
		{
			(*render).Yoffset = *(float*)valueList[i];
		}
	}

	return GZ_SUCCESS;

}




void shiftVerts(GzRender *render, GzCoord verts[3])
{
	for (int i = 0; i < 3; i++)
	{
		verts[i][0] = verts[i][0] - (*render).Xoffset;
		verts[i][1] = verts[i][1] - (*render).Yoffset;
	}
}

int GzPutTriangle(GzRender	*render, int numParts, GzToken *nameList, GzPointer	*valueList)
/* numParts : how many names and values */
{
/*  
- pass in a triangle description with tokens and values corresponding to 
      GZ_POSITION:3 vert positions in model space 
- Xform positions of verts using matrix on top of stack 
- Clip - just discard any triangle with any vert(s) behind view plane 
       - optional: test for triangles with all three verts off-screen (trivial frustum cull)
- invoke triangle rasterizer  
*/ 

//grab the tri's three vertices
	GzCoord * coord = (GzCoord *)valueList[0];
	GzCoord * norm = (GzCoord *)valueList[1];
	GzTextureIndex * uv = (GzTextureIndex *)valueList[2];
	GzCoord verts[3], norms[3];
	GzTextureIndex uvs[3];

	verts[0][0] = (*coord)[0];
	verts[0][1] = (*coord)[1];
	verts[0][2] = (*coord)[2];
	

	coord++;
	verts[1][0] = (*coord)[0];
	verts[1][1] = (*coord)[1];
	verts[1][2] = (*coord)[2];

	coord++;
	verts[2][0] = (*coord)[0];
	verts[2][1] = (*coord)[1];
	verts[2][2] = (*coord)[2];
	

	
	xform(verts[0], (*render).Ximage[(*render).matlevel - 1], verts[0]);
	xform(verts[1], (*render).Ximage[(*render).matlevel - 1], verts[1]);
	xform(verts[2], (*render).Ximage[(*render).matlevel - 1], verts[2]);
	shiftVerts(render, verts);

	//Read in the Normals
	norms[0][0] = (*norm)[0];
	norms[0][1] = (*norm)[1];
	norms[0][2] = (*norm)[2];
	xform(norms[0], (*render).Xnorm[(*render).matlevel - 1], norms[0]);
	vector_unit(norms[0]);
	
	norm++;
	norms[1][0] = (*norm)[0];
	norms[1][1] = (*norm)[1];
	norms[1][2] = (*norm)[2];
	xform(norms[1], (*render).Xnorm[(*render).matlevel - 1], norms[1]);
	vector_unit(norms[1]);

	norm++;
	norms[2][0] = (*norm)[0];
	norms[2][1] = (*norm)[1];
	norms[2][2] = (*norm)[2];
	xform(norms[2], (*render).Xnorm[(*render).matlevel - 1], norms[2]);
	vector_unit(norms[2]);

	//Read in the UVs
	uvs[0][0] = (*uv)[0];
	uvs[0][1] = (*uv)[1];
	//transform to perspective-space
	float vz0 = verts[0][2] / (INT_MAX - verts[0][2]);
	uvs[0][0] = uvs[0][0] / (vz0 + 1);
	uvs[0][1] = uvs[0][1] / (vz0 + 1);

	uv++;
	uvs[1][0] = (*uv)[0];
	uvs[1][1] = (*uv)[1];
	float vz1 = verts[1][2] / (INT_MAX - verts[1][2]);
	uvs[1][0] = uvs[1][0] / (vz1 + 1);
	uvs[1][1] = uvs[1][1] / (vz1 + 1);
	
	uv++;
	uvs[2][0] = (*uv)[0];
	uvs[2][1] = (*uv)[1];
	float vz2 = verts[2][2] / (INT_MAX - verts[2][2]);
	uvs[2][0] = uvs[2][0] / (vz2 + 1);
	uvs[2][1] = uvs[2][1] / (vz2 + 1);

	

	//computer CW edge orientation
	//sort veritces
	int v0 = -1;
	int v1 = -1;
	int v2 = -1;

	int left_vert = -1;
	int right_vert = -1;
	int top_vert = -1;

	int temp_value = 0;
	float x_across_v1;



	if (verts[0][1] == verts[1][1] || verts[1][1] == verts[2][1] || verts[0][1] == verts[2][1])
	{
		temp_value = 1;
		handle_special(verts, left_vert, right_vert, top_vert);
	}
	else
	{
		sort_vertices(verts, v2, v1, v0); ///v2=max

										  //get left right relationship
										  //v1=mid vert; get line from v2 to v0
		float v2_a, v2_b, v2_c;
		compute_abc(verts, v0, v2, v2_a, v2_b, v2_c); //get line between highest and lowest point

		x_across_v1 = ((-v2_b*verts[v1][1]) - v2_c) / (float)v2_a;
		if (x_across_v1 < verts[v1][0]) //v1 is on the right
		{
			left_vert = v2;
			right_vert = v1;
		}
		else //v1 is the left
		{
			left_vert = v1;
			right_vert = v2;
		}

		top_vert = v0;
	}


	//if (verts[0][2] < 0 && verts[1][2] < 0 && verts[2][2] < 0) return;

	//sanity check /////////////////////////////////////////
	float temp = get_min(verts[0][1], verts[1][1], verts[2][1]);

	GzPixel tempPix;
	tempPix.green = 4000;
	tempPix.blue = 200;
	tempPix.red = 0;
	tempPix.alpha = 1;
	tempPix.z = -300;
	bool error = false;



	//compute A, B, C for each vertex
	float left_a, left_b, left_c;
	compute_abc(verts, left_vert, top_vert, left_a, left_b, left_c);

	float right_a, right_b, right_c;
	compute_abc(verts, top_vert, right_vert, right_a, right_b, right_c);

	float bottom_a, bottom_b, bottom_c;
	compute_abc(verts, right_vert, left_vert, bottom_a, bottom_b, bottom_c);



	//get bounding box
	int box_x_min, box_y_min, box_x_max, box_y_max;
	get_bounding_box(verts, box_x_min, box_y_min, box_x_max, box_y_max);



	//for each pixel in bounding box
	int x = (*(*render).display).xres;
	int y = (*(*render).display).yres;
	int index;

	
	GzCoord edge_0, edge_1;
	get_vector(verts, edge_0, left_vert, top_vert);
	get_vector(verts, edge_1, top_vert, right_vert);
	
	float cross_a = (edge_0[1] * edge_1[2]) + (-edge_0[2] * edge_1[1]);
	float cross_b = (edge_0[2] * edge_1[0]) + (-edge_0[0] * edge_1[2]);
	float cross_c = (edge_0[0] * edge_1[1]) + (-edge_0[1] * edge_1[0]);
	
	float cross_d = -1.0*((cross_a*verts[top_vert][0]) + (cross_b*verts[top_vert][1]) + (cross_c*verts[top_vert][2]));

	int num_in = 0;





	//Interpolate u, v .............................
	float texu_cross_a, texu_cross_b, texu_cross_c, texu_cross_d;
	float texv_cross_a, texv_cross_b, texv_cross_c, texv_cross_d;

	GzCoord texVerts[3];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j <3; j++)
		{
			texVerts[i][j] = verts[i][j];
		}

		texVerts[i][2] = uvs[i][0];
	}


	GzCoord edge_tex0, edge_tex1;
	get_vector(texVerts, edge_tex0, left_vert, top_vert);
	get_vector(texVerts, edge_tex1, top_vert, right_vert);
	texu_cross_a = (edge_tex0[1] * edge_tex1[2]) + (-edge_tex0[2] * edge_tex1[1]);
	texu_cross_b = (edge_tex0[2] * edge_tex1[0]) + (-edge_tex0[0] * edge_tex1[2]);
	texu_cross_c = (edge_tex0[0] * edge_tex1[1]) + (-edge_tex0[1] * edge_tex1[0]);
	texu_cross_d = -1.0*((texu_cross_a*texVerts[top_vert][0]) + (texu_cross_b*texVerts[top_vert][1]) + (texu_cross_c*texVerts[top_vert][2]));


	for (int i = 0; i < 3; i++)
	{
		texVerts[i][2] = uvs[i][1];
	}

	get_vector(texVerts, edge_tex0, left_vert, top_vert);
	get_vector(texVerts, edge_tex1, top_vert, right_vert);
	texv_cross_a = (edge_tex0[1] * edge_tex1[2]) + (-edge_tex0[2] * edge_tex1[1]);
	texv_cross_b = (edge_tex0[2] * edge_tex1[0]) + (-edge_tex0[0] * edge_tex1[2]);
	texv_cross_c = (edge_tex0[0] * edge_tex1[1]) + (-edge_tex0[1] * edge_tex1[0]);
	texv_cross_d = -1.0*((texv_cross_a*texVerts[top_vert][0]) + (texv_cross_b*texVerts[top_vert][1]) + (texv_cross_c*texVerts[top_vert][2]));


	
	/////////////////////////////////////////////////////////
	//FIGURE OUT INTERP COLORS
	GzColor cols[3];
	float red_cross_a, red_cross_b, red_cross_c, red_cross_d;
	float green_cross_a, green_cross_b, green_cross_c, green_cross_d;
	float blue_cross_a, blue_cross_b, blue_cross_c, blue_cross_d;

	float normx_cross_a, normx_cross_b, normx_cross_c, normx_cross_d;
	float normy_cross_a, normy_cross_b, normy_cross_c, normy_cross_d;
	float normz_cross_a, normz_cross_b, normz_cross_c, normz_cross_d;
	if ((*render).interp_mode == GZ_COLOR)
	{
		for (int i = 0; i < 3; i++)
		{
			GzCoord temp_norm = { norms[i][0], norms[i][1], norms[i][2] };
			GzCoord temp_col; 

			shading(render, temp_col, temp_norm);
			cols[i][0] = temp_col[0];
			cols[i][1] = temp_col[1];
			cols[i][2] = temp_col[2];
		}

		GzCoord colVerts[3];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				colVerts[i][j] = verts[i][j];
			}

			colVerts[i][2] = cols[i][0];
		}

		
		GzCoord edge0, edge1; 
		get_vector(colVerts, edge0, left_vert, top_vert);
		get_vector(colVerts, edge1, top_vert, right_vert);
		red_cross_a = (edge0[1]*edge1[2]) + (-edge0[2]*edge1[1]);
		red_cross_b = (edge0[2]*edge1[0]) + (-edge0[0]*edge1[2]);
		red_cross_c = (edge0[0]*edge1[1]) + (-edge0[1]*edge1[0]);
		red_cross_d = -1.0*((red_cross_a*colVerts[top_vert][0]) + (red_cross_b*colVerts[top_vert][1]) + (red_cross_c*colVerts[top_vert][2]));


		for (int i = 0; i < 3; i++)
		{
			colVerts[i][2] = cols[i][1];
		}

		get_vector(colVerts, edge0, left_vert, top_vert);
		get_vector(colVerts, edge1, top_vert, right_vert);
		green_cross_a = (edge0[1] * edge1[2]) + (-edge0[2] * edge1[1]);
		green_cross_b = (edge0[2] * edge1[0]) + (-edge0[0] * edge1[2]);
		green_cross_c = (edge0[0] * edge1[1]) + (-edge0[1] * edge1[0]);
		green_cross_d = -1.0*((green_cross_a*colVerts[top_vert][0]) + (green_cross_b*colVerts[top_vert][1]) + (green_cross_c*colVerts[top_vert][2]));



		for (int i = 0; i < 3; i++)
		{

			colVerts[i][2] = cols[i][2];
		}

		
		get_vector(colVerts, edge0, left_vert, top_vert);
		get_vector(colVerts, edge1, top_vert, right_vert);
		blue_cross_a = (edge0[1] * edge1[2]) + (-edge0[2] * edge1[1]);
		blue_cross_b = (edge0[2] * edge1[0]) + (-edge0[0] * edge1[2]);
		blue_cross_c = (edge0[0] * edge1[1]) + (-edge0[1] * edge1[0]);
		blue_cross_d = -1.0*((blue_cross_a*colVerts[top_vert][0]) + (blue_cross_b*colVerts[top_vert][1]) + (blue_cross_c*colVerts[top_vert][2]));

	}
	else if ((*render).interp_mode == GZ_NORMALS)
	{
		GzCoord normVerts[3];
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j <3; j++)
			{
				normVerts[i][j] = verts[i][j];
			}

			normVerts[i][2] = norms[i][0];
		}

		//compute A, B, C for each vertex
		GzCoord edge0, edge1;
		get_vector(normVerts, edge0, left_vert, top_vert);
		get_vector(normVerts, edge1, top_vert, right_vert);
		normx_cross_a = (edge0[1] * edge1[2]) + (-edge0[2] * edge1[1]);
		normx_cross_b = (edge0[2] * edge1[0]) + (-edge0[0] * edge1[2]);
		normx_cross_c = (edge0[0] * edge1[1]) + (-edge0[1] * edge1[0]);
		normx_cross_d = -1.0*((normx_cross_a*normVerts[top_vert][0]) + (normx_cross_b*normVerts[top_vert][1]) + (normx_cross_c*normVerts[top_vert][2]));

		for (int i = 0; i < 3; i++)
		{
			normVerts[i][2] = norms[i][1];
		}

		//compute A, B, C for each vertex
		
		get_vector(normVerts, edge0, left_vert, top_vert);
		get_vector(normVerts, edge1, top_vert, right_vert);
		normy_cross_a = (edge0[1] * edge1[2]) + (-edge0[2] * edge1[1]);
		normy_cross_b = (edge0[2] * edge1[0]) + (-edge0[0] * edge1[2]);
		normy_cross_c = (edge0[0] * edge1[1]) + (-edge0[1] * edge1[0]);
		normy_cross_d = -1.0*((normy_cross_a*normVerts[top_vert][0]) + (normy_cross_b*normVerts[top_vert][1]) + (normy_cross_c*normVerts[top_vert][2]));

		for (int i = 0; i < 3; i++)
		{
			normVerts[i][2] = norms[i][2];
		}

		//compute A, B, C for each vertex
		get_vector(normVerts, edge0, left_vert, top_vert);
		get_vector(normVerts, edge1, top_vert, right_vert);
		normz_cross_a = (edge0[1] * edge1[2]) + (-edge0[2] * edge1[1]);
		normz_cross_b = (edge0[2] * edge1[0]) + (-edge0[0] * edge1[2]);
		normz_cross_c = (edge0[0] * edge1[1]) + (-edge0[1] * edge1[0]);
		normz_cross_d = -1.0*((normz_cross_a*normVerts[top_vert][0]) + (normz_cross_b*normVerts[top_vert][1]) + (normz_cross_c*normVerts[top_vert][2]));
	
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			GzCoord temp_norm = { norms[i][0], norms[i][1], norms[i][2] };
			GzCoord temp_col;

			shading(render, temp_col, temp_norm);
			cols[i][0] = temp_col[0];
			cols[i][1] = temp_col[1];
			cols[i][2] = temp_col[2];
		}

		for (int i = 0; i < 3; i++)
		{
			(*render).flatcolor[i] = cols[0][i]; // (cols[0][i] + cols[1][i] + cols[2][i]) / 3;
		}
	}

	

	for (int i = box_x_min; i <= box_x_max; i++)
	{
		for (int j = box_y_min; j <= box_y_max; j++)
		{

			//check pixel 
			int in = -1;

			//check left-top vert
			float loc_i = (float)i;
			float loc_j = (float)j;
			float val_left = (left_a*loc_i) + (left_b*loc_j) + left_c;
			float val_right = (right_a*loc_i) + (right_b*loc_j) + right_c;
			float val_bottom = (bottom_a*loc_i) + (bottom_b*loc_j) + bottom_c;

			if (val_left >= 0 && val_right > 0 && val_bottom >= 0) //include 
			{
				in = 1;
				num_in++;
			}


			index = (j*y) + i;


			if (in == 1 && i >= 0 && i < x && j >= 0 && j<y)
			{
				float temp_z = (-1.0*((cross_a*loc_i) + (cross_b*loc_j) + (cross_d))) / cross_c;
				
				
				GzDepth current_pix = ((*(*render).display).fbuf[index]).z;
				if ((float)current_pix >= temp_z && !(temp_z<0))
				{

					//change back U,V to u,v
					float U_new = (-1.0*((texu_cross_a*loc_i) + (texu_cross_b*loc_j) + (texu_cross_d))) / texu_cross_c;
					float V_new = (-1.0*((texv_cross_a*loc_i) + (texv_cross_b*loc_j) + (texv_cross_d))) / texv_cross_c;

					float vz_new = temp_z / (INT_MAX - temp_z);
					float u_new = U_new * (vz_new + 1);
					float v_new = V_new * (vz_new + 1);


					GzColor color_tex;
					if((*render).tex_fun!= NULL)
						(*render).tex_fun(u_new, v_new, color_tex);
					
					
					if ((*render).interp_mode == GZ_COLOR)
					{
						float temp_r = ((-1.0*((red_cross_a*loc_i) + (red_cross_b*loc_j) + (red_cross_d))) / red_cross_c);
						float temp_r_z = ((-1.0*((cross_a*loc_i) + (cross_b*loc_j) + (red_cross_d))) / cross_c);
						if ((*render).tex_fun != NULL)
						{
							GzIntensity r = ctoi(((-1.0*((red_cross_a*loc_i) + (red_cross_b*loc_j) + (red_cross_d))) / red_cross_c))*color_tex[0];
							GzIntensity g = ctoi(((-1.0*((green_cross_a*loc_i) + (green_cross_b*loc_j) + (green_cross_d))) / green_cross_c))*color_tex[1];
							GzIntensity b = ctoi(((-1.0*((blue_cross_a*loc_i) + (blue_cross_b*loc_j) + (blue_cross_d))) / blue_cross_c))*color_tex[2], a = 1;
							GzDepth z = (int)temp_z;


							GzPutDisplay((*render).display, i, j, r, g, b, a, z);
						
						}
						else
						{
							GzIntensity r = ctoi(((-1.0*((red_cross_a*loc_i) + (red_cross_b*loc_j) + (red_cross_d))) / red_cross_c));
							GzIntensity g = ctoi(((-1.0*((green_cross_a*loc_i) + (green_cross_b*loc_j) + (green_cross_d))) / green_cross_c));
							GzIntensity b = ctoi(((-1.0*((blue_cross_a*loc_i) + (blue_cross_b*loc_j) + (blue_cross_d))) / blue_cross_c)), a = 1;

							GzDepth z = (int)temp_z;


							GzPutDisplay((*render).display, i, j, r, g, b, a, z);
						}
					}
					else if ((*render).interp_mode == GZ_NORMALS)
					{
						float norm_x= (-1.0*((normx_cross_a*loc_i) + (normx_cross_b*loc_j) + (normx_cross_d))) / normx_cross_c;
						float norm_y= (-1.0*((normy_cross_a*loc_i) + (normy_cross_b*loc_j) + (normy_cross_d))) / normy_cross_c;
						float norm_z= (-1.0*((normz_cross_a*loc_i) + (normz_cross_b*loc_j) + (normz_cross_d))) / normz_cross_c;
						
						GzCoord temp_norm = { norm_x, norm_y, norm_z };
						vector_unit(temp_norm);
						GzColor col = { 0,0,0 };

						
						if ((*render).tex_fun != NULL)
							phong_shading(render, col, temp_norm, color_tex);
						else
							shading(render, col, temp_norm);
						GzIntensity r = ctoi(col[0]);
						GzIntensity g = ctoi(col[1]);
						GzIntensity b = ctoi(col[2]), a = 1;
						GzDepth z = (int)temp_z;

						//GzPixel defPixel = { r, g, b, a, z };

						//(*(*render).display).fbuf[index] = defPixel;
						GzPutDisplay((*render).display, i, j, r, g, b, a, z);
					}
					else
					{
						GzIntensity r = ctoi(((*render).flatcolor)[0]);
						GzIntensity g = ctoi(((*render).flatcolor)[1]);
						GzIntensity b = ctoi(((*render).flatcolor)[2]), a = 1;
						GzDepth z = (int)temp_z;

						//GzPixel defPixel = { r, g, b, a, z };
						//(*(*render).display).fbuf[index] = defPixel;
						GzPutDisplay((*render).display, i, j, r, g, b, a, z);
					}

				}
			}
		}

	}



	return GZ_SUCCESS;
}


void shading(GzRender *render, GzColor output, GzCoord input)
{
	GzCoord E = { 0, 0, -1 };
	GzColor diff = { 0, 0, 0 };
	GzColor spec_val = { 0, 0, 0 };
	GzColor result = { 0, 0, 0 };

	GzCoord norm = { input[0], input[1],input[2] };
	vector_unit(norm);
	vector_unit(E);


	float NE = dot_product(norm, E);

	int *cases = new int[(*render).numlights];
	//GzCoord *R = new GzCoord[(*render).numlights];
	GzCoord R;
	int caseNum = 0;
	float NL = 0;
	for (int i = 0; i < (*render).numlights; i++)
	{
		GzCoord L_dir = { ((*render).lights[i]).direction[0], ((*render).lights[i]).direction[1], ((*render).lights[i]).direction[2] };
		GzColor L_color = { ((*render).lights[i]).color[0], ((*render).lights[i]).color[1], ((*render).lights[i]).color[2] };

		vector_unit(L_dir);

		NL = dot_product(norm, L_dir);
		if (NL >= 0 && NE >= 0)
		{

			vector_mult(result, NL, norm);
			vector_mult(R, 2.0f, result);
			vector_sub(R, R, L_dir);
			//vector_unit(R);
			cases[i] = 0;

		}
		else if (NL < 0 && NE < 0)
		{
			GzCoord temp_norm;

			temp_norm[0] = -norm[0];
			temp_norm[1] = -norm[1];
			temp_norm[2] = -norm[2];

			vector_mult(R, 2.0f*NL, temp_norm);
			vector_sub(R, R, L_dir);
			vector_unit(R);
			cases[i] = 1;
		}
		else
		{
			cases[i] = 2;
		}

		if (cases[i] != 2)
		{
			float RE = dot_product(R, E);
			if (RE > 1)
				RE = 1.0f;
			else if (RE < 0)
				RE = 0.0f;

			result[0] = result[1] = result[2] = 0;
			RE = pow(RE, (*render).spec);
			vector_mult(result, RE, L_color);
			vector_sum(spec_val, spec_val, result);
		}

		if (cases[i] == 0)
		{
			vector_mult(result, NL, L_color);
			vector_sum(diff, diff, result);
		}

		else if (cases[i] == 1)
		{
			GzCoord temp_norm;

			temp_norm[0] = -norm[0];
			temp_norm[1] = -norm[1];
			temp_norm[2] = -norm[2];


			NL = dot_product(temp_norm, L_dir);
			vector_mult(result, NL, L_color);
			vector_sum(diff, diff, result);
		}
	}


	if ((*render).tex_fun != NULL)
	{
		output[0] = spec_val[0] + diff[0] + ((*render).ambientlight.color[0]);
		output[1] = spec_val[1] + diff[1] + (*render).ambientlight.color[1];
		output[2] = spec_val[2] + diff[2] + (*render).ambientlight.color[2];
	}
	else
	{
	output[0] = ((*render).Ks[0])*spec_val[0] + (*render).Kd[0] * diff[0] + (*render).Ka[0] * ((*render).ambientlight.color[0]);
	output[1] = ((*render).Ks[1])*spec_val[1] + (*render).Kd[1] * diff[1] + (*render).Ka[1] * (*render).ambientlight.color[1];
	output[2] = ((*render).Ks[2])*spec_val[2] + (*render).Kd[2] * diff[2] + (*render).Ka[2] * (*render).ambientlight.color[2];
	}
	


	for (int i = 0; i < 3; i++)
	{
		if (output[i] > 4095)
			output[i]=4095;
	}

}



void phong_shading(GzRender *render, GzColor output, GzCoord input, GzColor color)
{
	GzCoord E = { 0, 0, -1 };
	GzColor diff = { 0, 0, 0 };
	GzColor spec_val = { 0, 0, 0 };
	GzColor result = { 0, 0, 0 };

	GzCoord norm = { input[0], input[1],input[2] };
	vector_unit(norm);
	vector_unit(E);


	float NE = dot_product(norm, E);

	int *cases = new int[(*render).numlights];
	//GzCoord *R = new GzCoord[(*render).numlights];
	GzCoord R;
	int caseNum = 0;
	float NL = 0;

	for (int i = 0; i < 3; i++)
	{
		(*render).Ka[i] = color[i];
		(*render).Kd[i] = color[i];
	}


	for (int i = 0; i < (*render).numlights; i++)
	{
		GzCoord L_dir = { ((*render).lights[i]).direction[0], ((*render).lights[i]).direction[1], ((*render).lights[i]).direction[2] };
		GzColor L_color = { ((*render).lights[i]).color[0], ((*render).lights[i]).color[1], ((*render).lights[i]).color[2] };

		vector_unit(L_dir);

		NL = dot_product(norm, L_dir);
		if (NL >= 0 && NE >= 0)
		{

			vector_mult(result, NL, norm);
			vector_mult(R, 2.0f, result);
			vector_sub(R, R, L_dir);
			//vector_unit(R);
			cases[i] = 0;

		}
		else if (NL < 0 && NE < 0)
		{
			GzCoord temp_norm;

			temp_norm[0] = -norm[0];
			temp_norm[1] = -norm[1];
			temp_norm[2] = -norm[2];

			vector_mult(R, 2.0f*NL, temp_norm);
			vector_sub(R, R, L_dir);
			vector_unit(R);
			cases[i] = 1;
		}
		else
		{
			cases[i] = 2;
		}

		if (cases[i] != 2)
		{
			float RE = dot_product(R, E);
			if (RE > 1)
				RE = 1.0f;
			else if (RE < 0)
				RE = 0.0f;

			result[0] = result[1] = result[2] = 0;
			RE = pow(RE, (*render).spec);
			vector_mult(result, RE, L_color);
			vector_sum(spec_val, spec_val, result);
		}

		if (cases[i] == 0)
		{
			vector_mult(result, NL, L_color);
			vector_sum(diff, diff, result);
		}

		else if (cases[i] == 1)
		{
			GzCoord temp_norm;

			temp_norm[0] = -norm[0];
			temp_norm[1] = -norm[1];
			temp_norm[2] = -norm[2];


			NL = dot_product(temp_norm, L_dir);
			vector_mult(result, NL, L_color);
			vector_sum(diff, diff, result);
		}
	}



	output[0] = ((*render).Ks[0])*spec_val[0] + (*render).Kd[0] * diff[0] + (*render).Ka[0] * ((*render).ambientlight.color[0]);
	output[1] = ((*render).Ks[1])*spec_val[1] + (*render).Kd[1] * diff[1] + (*render).Ka[1] * (*render).ambientlight.color[1];
	output[2] = ((*render).Ks[2])*spec_val[2] + (*render).Kd[2] * diff[2] + (*render).Ka[2] * (*render).ambientlight.color[2];

	for (int i = 0; i < 3; i++)
	{
		if (output[i] > 4095)
			output[i] = 4095;
	}

}



void sort_vertices(GzCoord verts[3], int &max_y_tri, int &sec_y_tri, int &third_y_tri)
{

	if (verts[0][1] > verts[1][1])
	{
		if (verts[0][1] > verts[2][1])
		{
			max_y_tri = 0;

			if (verts[1][1] > verts[2][1])
			{
				sec_y_tri = 1;
				third_y_tri = 2;
			}
			else
			{
				sec_y_tri = 2;
				third_y_tri = 1;
			}
		}
		else
		{
			max_y_tri = 2;
			sec_y_tri = 0;
			third_y_tri = 1;
		}

	}
	else
	{
		if (verts[1][1] > verts[2][1])
		{
			max_y_tri = 1;

			if (verts[0][1] > verts[2][1])
			{
				sec_y_tri = 0;
				third_y_tri = 2;
			}
			else
			{
				sec_y_tri = 2;
				third_y_tri = 0;
			}
		}
		else
		{
			max_y_tri = 2;
			sec_y_tri = 1;
			third_y_tri = 0;
		}
	}
}




void handle_special(GzCoord verts[3], int &left, int &right, int &top)
{


	int same1, same2;
	int other;

	//find which edges are the same
	if (verts[0][1] == verts[1][1])
	{
		same1 = 0;
		same2 = 1;
		other = 2;
	}
	else if (verts[1][1] == verts[2][1])
	{
		same1 = 1;
		same2 = 2;
		other = 0;
	}
	else
	{
		same1 = 0;
		same2 = 2;
		other = 1;
	}


	int on_top = -1; // -1 = bottom

					 //check if they are the top edges or the bottom
	if (verts[other][1]<verts[same1][1])
	{
		on_top = 1;
	}

	if (on_top == 1)
	{
		top = other;

		//check which on is left or right
		if (verts[same1][0] < verts[same2][0])
		{
			left = same1;
			right = same2;
		}
		else
		{
			left = same2;
			right = same1;
		}
	}
	else
	{
		//check which on is left or right
		if (verts[same1][0] < verts[same2][0])
		{
			top = same1;
			right = same2;
		}
		else
		{
			top = same2;
			right = same1;
		}
		left = other;
	}
}


void get_bounding_box(GzCoord verts[3], int &x_min, int &y_min, int &x_max, int &y_max)
{
	x_min = get_min(verts[0][0], verts[1][0], verts[2][0]) - 1;
	y_min = get_min(verts[0][1], verts[1][1], verts[2][1]) - 1;

	x_max = get_max(verts[0][0], verts[1][0], verts[2][0]) + 1;
	y_max = get_max(verts[0][1], verts[1][1], verts[2][1]) + 1;

}


float get_max(float x, float y, float z)
{
	float max = x;

	if (max > y)
	{
		if (max <z)
			max = z;
	}
	else
	{
		max = y;

		if (max < z)
			max = z;
	}

	return max;
}

float get_min(float x, float y, float z)
{
	float min = x;

	if (min < y)
	{
		if (min >z)
			min = z;
	}
	else
	{
		min = y;

		if (min > z)
			min = z;
	}

	return min;
}


void compute_abc(GzCoord verts[3], int tail, int head, float &a, float &b, float &c)
{
	float y0 = verts[tail][1];
	float y1 = verts[head][1];

	float x0 = verts[tail][0];
	float x1 = verts[head][0];

	float temp_a, temp_b;
	temp_a = y0 - y1;
	temp_b = x1 - x0;
	//b = -1*b;
	//c = ((-a*verts[tail][0]) + (b*verts[tail][1]));

	a = temp_a;
	b = temp_b;
	c = (-1.0*((temp_a*(x0 + x1)) + (temp_b*(y0 + y1))) / 2.0f);
}


void get_vector(GzCoord verts[3], GzCoord vec, int tail, int head)
{
	vec[0] = verts[head][0] - verts[tail][0];
	vec[1] = verts[head][1] - verts[tail][1];
	vec[2] = verts[head][2] - verts[tail][2];

}

void matrix_mul(GzMatrix result, GzMatrix mat1, GzMatrix mat2)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			result[i][j] = 0; 
			for (int k = 0; k < 4; k++)
			{
				result[i][j] += mat1[i][k] * mat2[k][j];
			}
		}
	}
}

void set_identity(GzMatrix mat)
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (i == j)
				mat[i][j] = 1.0f;
			else
				mat[i][j] = 0.0f; 
		}
	}
}

void vector_unit(GzCoord vec)
{
	float det = (float)sqrt(vec[0]* vec[0] + vec[1]* vec[1] + vec[2]* vec[2]);

	if (det > 0)
	{
		vec[0] = vec[0] / det;
		vec[1] = vec[1] / det;
		vec[2] = vec[2] / det;
	}
	else
		vec[0] = vec[1] = vec[2] = 0;
}


void vector_mult(GzCoord result, float val, GzCoord vec2)
{
	result[0] = val * vec2[0]; 
	result[1] = val * vec2[1]; 
	result[2] = val * vec2[2];
}


float dot_product(GzCoord mat1, GzCoord mat2)
{
	float result; 
	result = mat1[0] * mat2[0] + mat1[1] * mat2[1] + mat1[2] * mat2[2];
	return result; 
}


void vector_sub(GzCoord result, GzCoord left, GzCoord right)
{
	result[0] = left[0] - right[0]; 
	result[1] = left[1] - right[1];
	result[2] = left[2] - right[2];
}

void vector_sum(GzCoord result, GzCoord left, GzCoord right)
{
	result[0] = left[0] + right[0];
	result[1] = left[1] + right[1];
	result[2] = left[2] + right[2];
}

void cross_product(GzCoord result, GzCoord left, GzCoord right)
{
	result[0] = left[1] * right[2] - left[2] * right[1];
	result[1] = left[2] * right[0] - left[0] * right[2];
	result[2] = left[0] * right[1] - left[1] * right[0];
}

void xform(GzCoord result, GzMatrix mat, GzCoord coords)
{
	float temp[4];
	for (int i = 0; i < 4; i++)
	{
		temp[i] = 0;
		temp[i] = mat[i][0] * coords[0] + mat[i][1] * coords[1] + mat[i][2] * coords[2] + mat[i][3];
	}

	if (temp[3] != 0)
	{
		result[0] = temp[0] / temp[3];
		result[1] = temp[1] / temp[3];
		result[2] = temp[2] / temp[3];
	}
	else
	{
		result[0] = temp[0];
		result[1] = temp[1];
		result[2] = temp[2];
	}
}

float deg_to_rads(float degree)
{
	return PI *degree / 180.0f; 
}

/* NOT part of API - just for general assistance */

short	ctoi(float color)		/* convert float color to GzIntensity short */
{
  return(short)((int)(color * ((1 << 12) - 1)));
}

