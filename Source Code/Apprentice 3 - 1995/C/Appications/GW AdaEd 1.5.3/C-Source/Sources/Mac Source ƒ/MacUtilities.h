/* MacUtilities.h */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

/* read options from file in current directory with name
 * argv[0].options
 */

int	getoptions(char *prog, char ***argv);

