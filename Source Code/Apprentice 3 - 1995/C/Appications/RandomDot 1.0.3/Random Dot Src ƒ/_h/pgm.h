/* pgm.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#ifndef __QDOFFSCREEN__
#include <QDOffscreen.h>
#endif	/* __QDOFFSCREEN__ */
Boolean IsPGMFile(ParmBlkPtr io);

OSErr PGMFileToGWorld(FSSpecPtr fs, ScriptCode code, GWorldPtr *gworld);
