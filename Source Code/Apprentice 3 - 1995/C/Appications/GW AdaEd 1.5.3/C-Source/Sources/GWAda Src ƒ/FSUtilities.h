/* FSUtilities.h */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

/* routines to traverse the folder contents */
long	CountItemsInFolder(const FSSpecPtr folder);
void	GetItemInFolder(long index, const FSSpecPtr folder, FSSpecPtr file);
Boolean	ItemIsFolder(const FSSpecPtr file);

