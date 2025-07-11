#ifndef VDEVS
#define VDEVS
/*
*  Virtual device record.
*  Defines the handles and hooks required for the vdevice code.
*/

typedef struct {
	GDHandle vgd;				/* GDevice created off-screen */
	CGrafPort vport;			/* Virtual port created in the GDevice */
	unsigned char *bp;			/* base pointer of data in the virtual device */
	Rect bounds;				/* boundary rectangle for vdevice */
} VDevice, *VDevicePtr;


#endif

#include "vdevice.proto.h"