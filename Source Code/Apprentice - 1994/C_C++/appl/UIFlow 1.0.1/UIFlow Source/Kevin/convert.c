#pragma segment conv
#define MAC
#include <stdio.h>
#include <vg.h>
#include <vset-prototypes.h>
extern DFR8addimage();
extern DFR8putimage();
extern uint16 DFR8lastref();
extern DFANputlabel();

#ifdef MAC
#define malloc NewPtr
#define free(x) DisposePtr((Ptr)(x))
#endif

/* dtm stuff. */
int   inport, outport;

/* the data struct we use for the data. */
typedef struct {
	int x;
	int y;
	float val;
} datastruct;

typedef struct {
	datastruct *p1;
	datastruct *p2;
	datastruct *p3;
}triangle;

#include "protypes.h"

/* contains the data read from the HDF file. */
int		numNodes;
float * xs, * ys, * vals = (float *) NULL;
float xmax, xmin, ymax, ymin;
float range, max, min;

/* connectivity info. */
int rank,numPolys; /* triagles or quads. */
int * con;

/* the raster image information. */
int xdim, ydim;
unsigned char * data;

/* parameters. */
short dots = 0, debug = 0, dtm = 0, DFR8 = 0;

/* access to the current hdf vset file. */
VGROUP	*vg;
VDATA	*vs;
DF		*f;

/************************************************************

	ReadVSetData (filename)

	read the vsets from the HDF file filename.

************************************************************/

Boolean ReadVSetData (char * filename)
{

	int		nvertices, vsize, interlace;
	char	vsname[50],fields[50];
	int		vsid;
	int		vgid;


	/* open the HDF file. */
	f = DFopen (filename,DFACC_ALL, 0);
	setjj();

	/* read the vgroup. */
	vgid = -1;
	while ((vgid=Vgetid(f, vgid)) != -1)
		{
		xs = ys = NULL;
		con = NULL;
		vg = (VGROUP *) Vattach (f, vgid, "r");
		vsid = -1;
		while ((vsid = Vgetnext(vg, vsid)) != -1)
			{

			/* read the x values. */

			vs = (VDATA *) VSattach (f, vsid, "r");
			VSinquire (vs, &nvertices, &interlace, fields, &vsize, vsname);
			if (!strncmp (fields, "PX",2))
				{
				VSsetfields (vs, "PX");
				xs = (float *) malloc (nvertices * sizeof(float));
				if (VSread (vs, (unsigned char*)xs, nvertices, interlace)!=nvertices)
					return false;
			}

			/* now get the y values. */
			if (!strncmp (fields, "PY", 2))
				{
				VSsetfields (vs, "PY");
				numNodes = nvertices;
				ys = (float *) malloc (nvertices * sizeof(float));
				if (!ys) return false;
				if (VSread (vs,(unsigned char*)ys, nvertices, interlace)!=nvertices)
					return false;
				}
	
			/* now get the data values. */
			if (!strncmp (fields, "PLIST", 5))
				{
				VSsetfields (vs, fields);
				con = (int *) malloc (vsize * nvertices);
				if (!con) return false;
				numPolys = nvertices;
				rank = vsize/sizeof(int);
				if (VSread (vs,(unsigned char*)con, nvertices, interlace) != nvertices)
					return false;
				}
	
			VSdetach (vs);
			if (con && xs && ys)
				return true;
	
			}
		
		/* free anything that was allocated. We did not find
		all of the things we need ; xs, ys and con */
		if (xs) free (xs);
		if (ys) free (ys);
		if (con) free (con);
		Vdetach (vg);
		}
		
	return false;
}

GetDataMaxMin()
{
	int i;

	max = min = vals[0];

	for (i = 1; i < numNodes; i++)
		/* find max and min values. */
		if (vals[i] > max) max = vals[i];
		else
			if (vals[i] < min) min = vals[i];

	range = max - min;
}

GetDimMaxMin()
{
	int i;

	xmax = xmin = xs[0];
	ymax = ymin = ys[0];

	for (i = 1; i < numNodes; i++)
		{
		/* find the max and min x */
		if (xs[i] > xmax) xmax = xs[i];
		if (xs[i] < xmin) xmin = xs[i];

		/* find the max and min y */
		if (ys[i] > ymax) ymax = ys[i];
		if (ys[i] < ymin) ymin = ys[i];
		}

}

/*********************************************************

	CloseVSetFile (filename)
	
	Open the hdf file containing the vset and set access to 
	the first group.
	
***********************************************************/

CloseVSetFile ()
{
	/* detach the vgroup. */
	
	Vdetach (vg);
	
	/* close the HDF file. */

	DFclose(f);
}


/************

	FillQuad (....

	This routine will fill a quadralateral if lr is not equal to NULL.
	It actuall divides the quad into two triangles, then filles each
	triangle. If lr == NULL it fills only the triangle.

**************/

FillQuad (tl, tr, ll, lr)
	datastruct *tl, *tr, *ll, *lr;
{

	triangle one, two;

	/* get the triangle. */

	GetTriangles (tl, tr, ll, lr, &one, &two);

	FillTriangle (&one);

	if (lr != NULL)
		FillTriangle (&two);

}

/***********
	
	GetTriangles (....

	This routine will take the points passed in and make two triangles out
	of them. The triangles are sorted such that the highest verticle point
	is represented first in the data structure.

***********/

GetTriangles (tl, tr, ll, lr, one, two)
	datastruct *tl, *tr, *ll, *lr;
	triangle * one, * two;
{

	/* lets sort them on the basis of y. */

	if (tl->y < tr->y)
		if (tl->y < ll->y) /* tl above tr */
			{
			/* tl above ll, therefore on top. */
			one->p1 = tl;
			if (ll->y < tr->y)
				{
				/* ll is just below tl, and tr below ll */
				one->p2 = ll;
				one->p3 = tr;
				}
			else /* tr just below tl, and ll on bottom. */
				{
				one->p2 = tr;
				one->p3 = ll;
				}
			}
		else /* tl above tr, and tl below ll, therefore ll on top. */
			{ 
			one->p1 = ll;
			one->p2 = tl;
			one->p3 = tr;
			}
	else
		if (tr->y < ll->y)
			{ /* tr above ll too, so its on top. */
			one->p1 = tr;
			if (ll->y < tl->y)
				{ /* ll above tl so tl on bottom. */
				one->p2 = ll;
				one->p3 = tl;
				}
			else /* ll on bottom. */
				{
				one->p2 = tl;
				one->p3 = ll;
				}
			}
		else
			{ /* ll above tr, tr above tl. */
			one->p1 = ll;
			one->p2 = tr;
			one->p3 = tl;
			}
			


	/* other triagle. */
	if (lr)
    if (lr->y < tr->y)
        if (lr->y < ll->y) /* lr above tr */
            {
            /* lr above ll, therefore on top. */
            two->p1 = lr;
            if (ll->y < tr->y)
                {
                /* ll is just below lr, and tr below ll */
                two->p2 = ll;
                two->p3 = tr;
                }
            else /* tr just below lr, and ll on bottom. */
                {
                two->p2 = tr;
                two->p3 = ll;
                }
            }
        else /* lr above tr, and lr below ll, therefore ll on top. */
            {
            two->p1 = ll;
            two->p2 = lr;
            two->p3 = tr;
            }
    else /* tr above lr. */
        if (tr->y < ll->y)
            { /* tr above ll too, so its on top. */
            two->p1 = tr;
            if (ll->y < lr->y)
                { /* ll above lr so lr on bottom. */
                two->p2 = ll;
                two->p3 = lr;
                }
            else /* ll on bottom. */
                {
                two->p2 = lr;
                two->p3 = ll;
                }
            }
		else
			{
			two->p1 = ll;
			two->p2 = tr;
			two->p3 = lr;
			}
		

}
	
/**************************************************************************

	This routine does the actual gouraud shading of a triangle.
	It works like this: for each verticle raster scan line, we
	need to determin which edges of the triangle are intersected.
	With this done, it is a simple matter of bi-directional interpolation.
	first we compute the values (pixel values) on each of the edges 
	where the horizontal raster scan line or ray intersects. Then we 
	do linear interpolation of the values along the line until we reach
	the other edge.

****************************************************************************/

FillTriangle (tri)
	triangle * tri;
{
	float slope1, yint1; /* params for equation of line 1. */
	int dy1, dy2, dx1, dx2;
	float slope2, yint2; /* params for equation of line 1. */
	int xleft, xright;
	int counter,loc;
	float boundstart1, boundstart2, valueinc1,valueinc2;
	
	/* bres equation for line 1. */
	dy1 = (tri->p2->y - tri->p1->y);
	if ((dx1 = tri->p2->x - tri->p1->x))
		slope1 = ((float)dy1) / (float)dx1;
	yint1 = (float)tri->p1->y - slope1*(float)tri->p1->x;

	/* equation for line two */
	dy2 = (tri->p3->y - tri->p1->y);
	if ((dx2 = tri->p3->x - tri->p1->x))
		slope2 = ((float) dy2) / (float) dx2;
	yint2 = (float)tri->p1->y - slope2*(float)tri->p1->x;

	/* do the top segment->
			.
                      .  . top segment
                    .     .
                   --------------------------------
                      .     . bottom segment
                        .    .
                          .   .
                            .  .
                              . .
                                ..
								  .
	*/

	/* shade the bugger top down. loc is the current raster scan line at
		off set loc. */

	/* compute the number of scan line to be generated for this segment. */

	counter = dy1;

	xright = xleft = tri->p1->x;
	boundstart1 = boundstart2 = tri->p1->val;
	if (dy1)
		valueinc1 = ((float)(tri->p2->val - tri->p1->val))/((float)dy1);
	else valueinc1 = 1.0;

	if (dy2)
		valueinc2 = ((float)(tri->p3->val - tri->p1->val))/((float)dy2);
	else valueinc2 = 1.0;

	for (loc = tri->p1->y; loc <= tri->p2->y; loc++)
		{

		if (dx1)
			if (slope1 != 0.0)
				xleft = (short) (((float)loc - yint1)/slope1);

		if (dx2)
			if (slope2 != 0.0)
				xright =(short) (((float) loc - yint2)/slope2);

		if (xleft == xright)
			SmoothColorLine (data + (loc*xdim) + xright, 1, boundstart2, 0);
		else
			if (xleft > xright)
				SmoothColorLine (data + (loc*xdim) + xright,
					xleft - xright + 1, boundstart2,
					(boundstart1 - boundstart2)/(float)(xleft-xright));
			else
				SmoothColorLine (data + (loc*xdim) + xleft,
					xright - xleft + 1, boundstart1,
					(boundstart2 - boundstart1)/(float)(xright-xleft));

		boundstart1 += valueinc1;
		boundstart2 += valueinc2;
		}

	/* bres equation for line 1. */
	dy1 = (tri->p3->y - tri->p2->y);
	if (dx1 = tri->p3->x - tri->p2->x)
		slope1 = ((float)dy1) / (float)dx1;
	yint1 = (float)tri->p3->y - slope1*(float)tri->p3->x;

	/* compute the number of scan line to be generated for this segment. */

	counter = dy1;

	xleft = tri->p2->x;
	boundstart1 =  tri->p2->val;
	if (dy1)
		valueinc1 = ((float)(tri->p3->val - tri->p2->val))/((float)dy1);
	else
		valueinc1 = 1.0;

	boundstart1 += valueinc1;
	for (loc = tri->p2->y+1; loc <= counter+tri->p2->y; loc++)
		{

		if (dx1)
			if (slope1 != 0.0)
				xleft = (short) (((float)loc - yint1)/slope1);

		if (dx2)
			if (slope2 != 0.0)
				xright =(short) (((float) loc - yint2)/slope2);

		if (xleft == xright)
			SmoothColorLine (data + (loc*xdim) + xright, 1, boundstart2, 0);
		else
			if (xleft > xright)
				SmoothColorLine (data + (loc*xdim) + xright,
					xleft - xright+1, boundstart2,
					(boundstart1 - boundstart2)/(float)(xleft-xright));
			else
				SmoothColorLine (data + (loc*xdim) + xleft,
					xright - xleft+1, boundstart1,
					(boundstart2 - boundstart1)/(float)(xright-xleft));

		boundstart1 += valueinc1;
		boundstart2 += valueinc2;
		}

}

DrawDots (pt)
	datastruct * pt;
{
	*(data + (pt->y * xdim) + pt->x) = 255;
}

Boolean SmoothColorLine (where, howmany, startval, inc)
	register char * where;
	register int howmany;
	register float startval, inc;
{
	register float value;

if (range == 0.0)
	{
	return false;
	}

	for (;howmany; howmany--,where++,startval+=inc)
		{
		value =  ((startval - min)/range) * 253.0 + 1.0;
		*where = (char) value;
		}
}

/*
 *	This is the routine that is called in order to conver the vset data to raster
 *	images. The vset data is read from the file vset.out writen by prataps fortran
 * 	code.
 */
 
int ConvertVSet2Raster(char	* dfFileName)
{
	datastruct tl, tr, ll, lr; 	/* contain vertices for the quads. */
	int 	i; 
	unsigned char * dptr;
	float 	xscale;
	int		*current;
	Boolean firstPut = true;
	int		nvertices, vsize, interlace;
	char	vsname[50],fields[50];
	int		vsid;
	int 	xsize = 0;
	uint16 	ref;
	char 	fName[512];
	int		len;
	
	len = strlen(dfFileName);
	if (len > 512)
		return -1;
		
	strcpy(fName,dfFileName);

	/* the purpose of this program is to demonstrate the 
	interpolated fill of a rectangle. */
	
	if (!ReadVSetData("vset.out"))
		return -2;
		
	if (!xs || !ys || !con)
		return -1;

	GetDimMaxMin();
	if (xsize == 0)
		xsize = 200;
	xdim = xsize;
	ydim = (int) ((ymax-ymin)/(xmax-xmin) * (float)xsize);
	ydim++;
	
	/* try to allocate memory. */
	data = (unsigned char *) malloc (xdim*ydim);
	if (!data)
		return -1;
	xscale = ((float)xdim)/(xmax - xmin);	/* compute the scale */

	/* now get the data values. */
	vsid = -1;
	while ((vsid = Vgetnext(vg, vsid)) != -1)
		{

		/* read the x values. */
		vs = (VDATA *) VSattach (f, vsid, "r");
		VSinquire (vs, &nvertices, &interlace, fields, &vsize, vsname);
		if (!strncmp (fields, "PX",2))
			continue;
			
		if (!strncmp (fields, "PY",2))
			continue;
			
		if (!strncmp (fields, "PLIST",5))
			continue;
			
		/* init the raster image to some background color. */
		for (dptr = data, i = 0 ; i < xdim*ydim; i++,dptr++)
			*dptr = 0;
			
		VSsetfields (vs, fields);
		if (vals == NULL)
			vals = (float *) malloc (nvertices * sizeof(float));
			
		if(VSread (vs,(unsigned char*)vals, nvertices, interlace)!=nvertices)
			{
			free (data); 
			free (xs); 
			free (ys); 
			free (con);
			return -1;
			}
	
		/* shade the quadralaterals */
	
		GetDataMaxMin();
		for (i = 0; i < numPolys; i++)
			{
			/* set up the tl. */
	
			current = con + (i * rank);
	
			tl.x = (xmax - xs[current[0]-1]) * xscale;
			tl.y = (ymax - ys[current[0]-1]) * xscale;
			tl.val = vals[current[0]-1];
	
			tr.x = (xmax - xs[current[1]-1]) * xscale;
			tr.y = (ymax - ys[current[1]-1]) * xscale;
			tr.val = vals[current[1]-1];
	
			lr.x = (xmax - xs[current[2]-1]) * xscale;
			lr.y = (ymax - ys[current[2]-1]) * xscale;
			lr.val = vals[current[2]-1];
	
			if (rank == 4)
				{
				if (current[3] != 0)
					{
					ll.x = (xmax - xs[current[3]-1]) * xscale;
					ll.y = (ymax - ys[current[3]-1]) * xscale;
					ll.val = vals[current[3]-1];
					FillQuad (&tl, &tr, &ll, &lr);
					}
				else
					FillQuad (&tl, &tr, &lr, NULL);
	
				}
			else
				FillQuad (&tl, &tr, &lr, NULL);
		
			}
			
		/* do the hdf stuff. */
		
		if (firstPut)
			{
			DFR8putimage (fName, data, xdim, ydim, NULL);
			firstPut = false;
			}
		else
			DFR8addimage (fName, data, xdim, ydim, NULL);
			
		/* add a label for the image. */
		ref = DFR8lastref ();
		DFANputlabel (fName, (uint16) DFTAG_RIG, ref, vsname);
		
	}
	CloseVSetFile();
	free (data); 
	free (xs); 
	free (ys); 
	free (con);
	
	return 0;
}

