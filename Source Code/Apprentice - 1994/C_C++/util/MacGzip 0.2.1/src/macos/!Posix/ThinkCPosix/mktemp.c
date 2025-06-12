/* $Id: $ */

/*
 * This presently overwrites the contents of template.
 */

#include "ThinkCPosix.h"

char *mktemp(char *template)
{
	if (template == NULL || strlen(template) < 8) {
		fprintf(stderr, "mktemp: template too short\n");
		return NULL;
	}
	return tmpnam(template);
}
