#include <stdio.h>

#include "d:\ed\edit.h"



AVL_EDIT_WINDOW_PTR avl_w;



void AVL_GET_DESC()

{

    FILE *fp, *fopen();

    unsigned n;

    fp = fopen("ADD","r");

    if (fp == NULL)  {

    	printf("\nCan not interface with GWAda!\n");

    	exit(1);

    	}

/*	if (fread(avl_w,sizeof(AVL_EDIT_WINDOW),1,fp) != 1)  {

    	printf("\nCan not interface with GWAda!\n");

    	exit(1);

    	}

*/

	fscanf(fp,"%u", &n);

    fclose(fp);

    avl_w = (AVL_EDIT_WINDOW_PTR) n;

    if (avl_w -> ns > 0)

		avl_w -> fix(); 

}

