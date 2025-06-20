#define FILE_NUM 49
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/* Copyright (c) 1992 by Qualcomm, Inc. */
#pragma load EUDORA_LOAD
#pragma segment LMgr

/************************************************************************
 * FirstSelected - find the first selection (if any)
 ************************************************************************/
Boolean FirstSelected(Cell *c,ListHandle list)
{
	c->h = c->v = 0;
	return(LGetSelect(True,c,list));
}

/************************************************************************
 * Next1Selected - get the index (plus 1) of the next selected cell
 *  in a 1-dimensional list.
 *  returns 0 if no (more) cells selected
 ************************************************************************/
short Next1Selected(short thisOne,ListHandle list)
{
	Point c;
	c.h = 0;
	c.v = thisOne;
	if (LGetSelect(True,&c,list)) return(c.v+1);
	else return(0);
}

/************************************************************************
 * InWhich1Cell - over which cell (in a 1dim list) is the point?
 ************************************************************************/
short InWhich1Cell(Point mouse,ListHandle list)
{
	Rect rView = (*list)->rView;
	short cellHi = (*list)->cellSize.v;
	short scroll = (*list)->visible.top;
	short cell;
	
	if (!PtInRect(mouse,&rView)) return(0);
	
	cell = scroll + (mouse.v-rView.top)/cellHi + 1;
	return(cell);
}


/************************************************************************
 * Cell1Selected - is a cell in a 1dim list selected?
 ************************************************************************/
Boolean Cell1Selected(short which,ListHandle list)
{
	Point pt;
	
	pt.h = 0; pt.v = which-1;
	return(LGetSelect(False,&pt,list));
}