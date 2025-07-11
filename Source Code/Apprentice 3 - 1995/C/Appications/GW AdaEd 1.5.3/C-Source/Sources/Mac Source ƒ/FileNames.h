/* FileNames.h */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */



enum {
	flName = 1, flPath = 2, flExtension = 4
};


void	ParseFile(const Str255 file, short options, Str255 results);
void	Uppercase(Str255 name);
void	Lowercase(Str255 name);
