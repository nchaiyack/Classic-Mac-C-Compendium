#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mac-specific.h"
#include "Parse_special.h"

#define		MAX_KEYWORD		50

char	*Get_keyword( char *s, short *keytype )
{
	char	keyword[MAX_KEYWORD];
	char	*key_ptr, *keyword_end_ptr;	

	key_ptr = keyword;
	keyword_end_ptr = keyword + MAX_KEYWORD - 1;
	while (*s == ' ') ++s; // scan past any blanks
	while ( (*s != '=') && (*s != '\0') && (key_ptr < keyword_end_ptr) )
	{
		*key_ptr = *s;	// copy the keyword
		++s;
		key_ptr++;
	}
	*key_ptr = '\0';
	if (*s != '=')
	{
		*keytype = sp_nokeyword;
		return s;
	}
	/*
		At this point, we have a keyword and *s == '='.
	*/
	s++;	// skip over the '='
	/*
		Now we have an alleged keyword. See if we recognize it.
	*/
	if ( (strcmp(keyword, "pict") == 0) || (strcmp(keyword, "pictfile") == 0) )
		*keytype = sp_pict;
	else if ( (strcmp(keyword, "width") == 0) || (strcmp(keyword, "wd") == 0) )
		*keytype = sp_width;
	else if ( (strcmp(keyword, "height") == 0) || (strcmp(keyword, "ht") == 0) )
		*keytype = sp_height;
	else if ( (strcmp(keyword, "hscale") == 0) || (strcmp(keyword, "xscale") == 0) )
		*keytype = sp_hscale;
	else if ( (strcmp(keyword, "vscale") == 0) || (strcmp(keyword, "yscale") == 0) )
		*keytype = sp_vscale;
	else if ( strcmp(keyword, "scale") == 0)
		*keytype = sp_scale;
	else
		*keytype = sp_nokeyword;
	
	return s;
}


char	*Get_filename( char *s, StringPtr filename )
{
	char	*file_ptr;
	
	while (*s == ' ') ++s; // scan past any blanks
	file_ptr = (char *) filename;
	if (*s == '"')	// quoted filename
	{
		++s;	// move past the quote
		while ( (*s != '"') && (*s != '\0') )
		{
			*file_ptr = *s;
			++s;
			++file_ptr;
		}
		if (*s == '"') ++s;
	}
	else	// file not quoted, just scan to blank or end
	{
		while ( (*s != ' ') && (*s != '\0') )
		{
			*file_ptr = *s;
			++s;
			++file_ptr;
		}
	}
	*file_ptr = '\0';
	CtoPstr( (char *) filename );
	return s;
}

void Get_scales( char *s, float *hscale, float *vscale,
					short ht, short wd )
{
	char	message[256];
	short	keytype;
	float	value;
	Boolean height_set, width_set;
	
	*hscale = *vscale = 1;
	height_set = width_set = false;
	while (*s) // process the string until the end
	{
		/*
			Look for a keyword.
		*/
		s = Get_keyword( s, &keytype );
		
		/*
			At this point, we have a recognized keyword.  We need to get
			the value, either a decimal number or a filename.
		*/
		while (*s == ' ') ++s; // scan past any blanks
		sscanf( s, "%g", &value );
		switch (keytype)
		{
			case sp_scale:
				*hscale = *vscale = value;
				break;
			case sp_hscale:
				*hscale = value;
				break;
			case sp_vscale:
				*vscale = value;
				break;
			case sp_height:
				*vscale = value / ht;
				height_set = true;
				if (!width_set) *hscale = value / wd;
				break;
			case sp_width:
				*hscale = value / wd;
				width_set = true;
				if (!height_set) *vscale = value / ht;
				break;
		}
		while ( (*s != '\0') && (*s != ' ') ) ++s; // skip over the number
	}
}