#define FILE_NUM 36
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Misc

#define ARRAY(i)				(array+i*size)
void QSort(UPtr array, int size, int f, int l, int (*compare)(), void (*swap)());
/**********************************************************************
 * QuickSort - sort an array
 * Algorithms, Reingold, Nievergelt, Deo, p. 286
 **********************************************************************/
void QuickSort(UPtr array, int size, int f, int l, int (*compare)(), void (*swap)())
{
	int i,j;
	if (f>=l) return;
	
	(*swap)(ARRAY(f),ARRAY((f+l)/2));
	
	for (i=f+1; (*compare)(ARRAY(i),ARRAY(f))<0; i++);
	
	for (j=l; (*compare)(ARRAY(j),ARRAY(f))>0; j--);
	
	while (i<j)
	{
		(*swap)(ARRAY(i),ARRAY(j));
		for (i++; (*compare)(ARRAY(i),ARRAY(f))<0; i++);
		for (j--; (*compare)(ARRAY(j),ARRAY(f))>0; j--);
	}
	
	(*swap)(ARRAY(f),ARRAY(j));
	
	if ((j-1)-f > l-(j+1))
	{
		QuickSort(array,size,j+1,l,compare,swap);
		QuickSort(array,size,f,j-1,compare,swap);
	}
	else
	{
		QuickSort(array,size,f,j-1,compare,swap);
		QuickSort(array,size,j+1,l,compare,swap);
	}
}

int StrCompar(UPtr s1, UPtr s2)
{
	return(IUCompString(s1,s2));
}

void StrSwap(UPtr s1, UPtr s2)
{
	Str255 temp;
	
	BlockMove(s1,temp,*(unsigned char *)s1+1);
	BlockMove(s2,s1,*(unsigned char *)s2+1);
	BlockMove(temp,s2,*(unsigned char *)temp+1);
}

int CStrCompar(UPtr s1, UPtr s2)
{
#pragma unused(s1,s2)
	return (0);
}

void CStrSwap(UPtr s1, UPtr s2)
{
#pragma unused(s1,s2)
}
