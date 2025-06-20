/* AdaFileTypes.h */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


/*--------------------------------------------------------
   Different types of files created by this system.

	1) Source files are 'TEXT' files that can be read by
		any mac text editor (TeachText, Alpha, BBEdit, etc.)
	
	2) Listing files are also 'TEXT' files that can be read
		by any text reader.  GADA differentiates between these
		two based on filename extention.  .lis files are read
		in as listings, and .ada are read in as source files.
	
	3) Library files are files created by the adafront,
		adagen, and adabind programs.
	
	4) The predef library has it's own type of file/icon,
		so that the user can differentiate it from other
		files.
	
	5)	Output files, created by adaexec are of type 'TEXT'
		also.

--------------------------------------------------------*/

void	FileType(long type);
