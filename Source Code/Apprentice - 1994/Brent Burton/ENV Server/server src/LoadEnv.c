/*
** LoadEnv.c
**
** Functions to read environment files which contain
** environment variable definitions.
*/

#include <MacHeaders>
#include <stdio.h>
#include <ctype.h>
#include "LoadEnv.h"
#include "logging.h"
#include "hash.h"
#include "utils.h"

#define skip_spc(x)   while (isspace(x)) x++
#define find_end(x)   while (isalphanum(x) || ((x)=='_')) x--


/************************/
void LoadEnvFile(const char *cfname)
{
	FILE *cfile;
	char msg[100];
	
	cfile = fopen(cfname, "r");
	if (cfile == NULL)
	{
#if LOGGING
		LogEntry("Couldn't read environment file, looking for:");
		LogEntry(cfname);
#endif
		return;
	}
#if LOGGING
	LogEntry("Loading environment file");
#endif
	
	while ( !feof(cfile))
	{
		char s[256], *p;
		char *start;			/* start of a possible xxx=yyy string */
		char *eqp=NULL;			/* pointer to equal sign */
		
		(void)fgets( s, 255, cfile);
		p = mystrchr (s, '\n');	/* remove any newline that may be there */
		if (p) *p = '\000';
		
		p=s;
		
		{	/** catch a comment and terminate the string there **/
			char *pp;
			pp = mystrchr (p, '#');
			if (pp) *pp = '\000';
		}
		
		skip_spc(p);			/* skip initial whitespace */
		
		if ( isalnum(*p) || (*p=='_'))	/* Beginning of expression?? */
		{
			start = p;			/* read in xxx part */
			while ( isalnum(*p) || *p=='_')
				p++;
			
			skip_spc(p);		/* skip whitespace again */
			
			if (*p == '=')		/* found =, now read yyy */
				eqp = p++;
			else
				continue;		/* no equals sign, so skip this line */
			
			skip_spc(p);		/* skip whitespace again */

			while ( *p)		/* increment over var's value */
				p++;

			/*
			** At this point, *p is the ending nul of the string.
			** Now, backup over the line, changing all trailing
			** whitespace to nulls, shortening the string.
			*/
			p--;
			while ( (*p == ' ') || (*p == '\t'))
				*p-- = '\000';
			
			/*** done with this line ***/
			
#if LOGGING
			sprintf(msg, "Attempting to add >%s<", start);
			LogEntry(msg);
			if (hash_add_entry( start) == H_ERROR)
			{
				sprintf(msg, "Failed to add >%s<", start);
				LogEntry(msg);
			}
#else
			if (hash_add_entry( start) == H_ERROR)
			{
			}
#endif
		} /* if */
	} /* while !feof() */
	fclose(cfile);

#if LOGGING
	LogEntry("Finished reading environment file.");
#endif
} /* LoadEnvFile */



