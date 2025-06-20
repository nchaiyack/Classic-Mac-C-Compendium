// d3d_v2_objects.h
// Darrell Anderson

#if 0

#ifndef __d3d_v2_objects__
#define __d3d_v2_objects__

/*
typedef struct d3dFacet {
	short numVertices;
	d3dPoint *vertex;
	
	// linked list
	typedef struct d3dFacet *next;
} d3dFacet;

typedef struct d3dObject {

	// where in space is the object
	d3dPoint pos;
	d3dPoint ort;

	// how many and what are the points in the faces (in universe frame)
	short numFacets;
	d3dFacet *facetList;
} d3dObject;

d3dFacet *d3dCreateFacet(numVerices);
void d3dAddFacetToObject(d3dObject *obj, d3dFacet *newFacet);
*/

typedef struct d3dFacet4 {
	d3dPoint vertex[4];
}

typedef struct d3dObject4 {
	short numFacet4s;
	d3dFacet4 *facet;
}

typedef struct d3dInstance4 {
	short object4num;

	d3dPoint pos, ort;
}

void d3dDrawFacet4( d3dFacet4 *facet, d3dPoint *pos, d3dPoint *ort, short lineColor, short fillColor );
#endif

#endif
