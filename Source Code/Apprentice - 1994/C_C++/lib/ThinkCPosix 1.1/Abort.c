/* $Id: $ */

#include "ThinkCPosix.h"

void Abort(void)
{
	fprintf(stderr, "Abnormal program termination\n");
	exit(3);
}
