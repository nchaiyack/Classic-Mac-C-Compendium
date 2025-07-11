//	Copyright 1993 Ralph Gonzalez

/*
*	FILE:		error.c
*	AUTHOR:		R. Gonzalez
*	CREATED:	October 6, 1990
*
*	methods for error class, for reporting errors.
*/

# include	"error.h"
# include	<stdlib.h>
# include	<stdio.h>
# include	<string.h>

/******************************************************************
*	initialize
******************************************************************/
Error::Error(void)
{
	error_file = fopen("error.fil","w");
}

/******************************************************************
*	report error
******************************************************************/
void	Error::report(char *error_string)
{
	fprintf(error_file,"%s\n",error_string);
}

/******************************************************************
*	destroy
******************************************************************/
Error::~Error(void)
{
	fclose(error_file);
}

