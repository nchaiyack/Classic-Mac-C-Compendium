#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Types.r>
#else
	#include "Types.r"
#endif

#include "boxmaker constants.h"

resource 'STR#' (kErrStringID) {
	{
		"This application requires Apple events. "
		"Please upgrade to System 7.0 or later to use.",

		"An error occured during Apple event processing.",

		"The output file could not be created",
		
		"ll-R can't run without creating an output file"
	}
};
