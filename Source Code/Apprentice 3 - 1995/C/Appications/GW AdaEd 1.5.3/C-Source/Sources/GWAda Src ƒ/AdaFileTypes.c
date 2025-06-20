/* AdaFileTypes.c */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#include "AdaFileTypes.h"

/* These two are used by the ANSI library when
 * files are created using the C I/O package.
 */
extern long _ftype;
extern long _fcreator;


static void SetFileType(long type, long creator)
{
	_ftype = type;
	_fcreator = creator;
}

//
// Only the first two are opened from the GADA application.
// The type of the others is used for the only purpose of
// getting icons for these files.
//
void	FileType(long type)
{
	switch (type) {
		// These are text (ASCII) files that can
		// be opened from GW Ada.
		case 'TEXT':
		case 'AMSG':
		case 'AERR':
		case 'ALIS':
			SetFileType('TEXT', 'GADA');
			break;

		// These are library files...  AIC is a
		// temporary file, but it exists between
		// executions (front and gen), thus give
		// it an icon.
		case 'AAXQ':
		case 'ATRC':
		case 'AAIC':
			SetFileType('LADA', 'GADA');
			break;

		// These are temporary files created by
		// the Mac version
		case 'AOPT':
		case 'ARTN':
			SetFileType('ATMP', 'GADA');
			break;

		// These are temporary files created by
		// the compiler (NYU stuff)
		case 'ASTn':
			SetFileType('????', 'GADA');
			break;

		default:
			SetFileType(type, 'GADA');
			break;
	}
}

