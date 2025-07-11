/* GIF.h */

#include <QDOffScreen.h>
#include <Files.h>

#define	strCantOpenData		128									/* Constants for gMessage */
#define	strGIFNoMem			129									/* You can change them, and you ought to move */
#define	strCantRead			130									/* them to a global #include file */
#define	strInvalidGIFSig		131									/* so that the caller sees them too */
#define	strCantSetFPos			132
#define	strUnknownGIFBlock		133
#define	strNoColorMap			134

void DrawGIF (FSSpec *spec, long FileSize, long worldFlags, GWorldPtr *world, long *neededMemory);
