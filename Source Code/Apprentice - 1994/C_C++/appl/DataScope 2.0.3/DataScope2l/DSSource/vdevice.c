#include "vdevice.h"

/*************************************************************************/
/* Virtual drawing device code.
*
*  
*  InitVDevice()		fill in the fields of a vdevice and allocate the gdevice.
*  SetVDevice()         set the device and port for off-screen drawing.
*                       this only pushes the old values 1-deep, no stack.
*  UnsetVDevice()       set the device and port back to what they were.
*  TrashVDevice()       dispose and close all fields in the vdevice.
*  ColorVDevice()       set the color palette for the 8-bit vdevice.
*/
GDHandle v_savegd;		/* the saved gdevice */
CGrafPtr v_saveport;	/* the saved port from SetPort */

/*************************************************************************/
/* InitVDevice
*  allocate an off-screen pixmap and off-screen gdevice which can be drawn
*  in without the palette manager affecting the colors.  If we use MakeITable
*  ourselves and don't install the gdevice into the gdevice list or make it
*  active, then the palette manager ignores it, but QuickDraw works on it.
*
*  set vdev->bounds to the size you need before calling.
*  allocate vdev->bp enough space to hold one byte per pixel of a rectangle that
*  size.
*
*  returns 0 if ok,
*  returns -1 or other negative on other errors.
*/
InitVDevice(vdev)
	VDevicePtr vdev;
{
	GDHandle thegd;
	PixMapHandle pm;
	Rect tr;
	CTabHandle ct;
	int width;

	GetPort((GrafPtr *)&v_saveport);
	v_savegd = GetGDevice();			/* get old values */
	
	tr = vdev->bounds;					/* get size of device to create */
	if (tr.right - tr.left < 1 ||
		tr.bottom - tr.top < 1)
		return(-1);						/* check for simple mistake */
		
	width = tr.right - tr.left;
	
	if (!vdev->bp)
		return(-2);						/* another simple mistake */
/*
*  Allocate the off-screen PixMap for drawing a duplicate copy.  The off-screen
*  gdevice, CPort and PixMap form a virtual drawing space with its own color map.
*  For this program, we have chosen to make this virtual space an 8-bit drawing
*  device.  When drawing, use SetVDevice and UnsetVDevice to turn on and off.
* 
*/
	thegd = vdev->vgd = NewGDevice( 0, -1 );
	
	pm = (PixMapHandle) NewHandle(sizeof(PixMap));

	if (width & 1)	{					/* must be even */
		--tr.right;
		--width;
	}
	(*pm)->baseAddr = vdev->bp;			/* get memory to use */
	(*pm)->rowBytes = width | 0x8000;	/* setting high flag bit */
	(*pm)->bounds = tr;
	(*pm)->pixelSize = 8;				/* source is 8-bits */
	(*pm)->pixelType = 0;				/* chunky */
	(*pm)->cmpCount = 1;				/* chunky */
	(*pm)->cmpSize = 8;					/* chunky */
	(*pm)->hRes = 72;
	(*pm)->vRes = 72;					/* resolution = 72dpi */
	(*pm)->planeBytes = 0;				/* chunky */
	(*pm)->pmVersion = 0;				/* chunky */
	(*pm)->packSize = 0;				/* chunky */
	(*pm)->packType = 0;				/* chunky */
	ct = (*pm)->pmTable = (CTabHandle)NewHandle(sizeof(ColorTable));
	(*ct)->ctSeed = GetCTSeed();
	(*ct)->ctFlags = 0x8000;
	(*ct)->ctSize = 1;					/* 1-length color table (empty) */
	
	(*thegd)->gdResPref = 3;			/* inverse table size preferred */
	(*thegd)->gdType = 0;				/* type = CLUT type */
	(*thegd)->gdRect = tr;				/* device size boundary */

	(*thegd)->gdPMap = pm;				/* copy pixmap handle */
	
	SetDeviceAttribute(thegd,noDriver,true);
	SetDeviceAttribute(thegd,gdDevType,true);	/* set to color, not monochrome */
	
	SetGDevice(thegd);
						/* CPort inherits from the gdevice, including pixmap */
	OpenCPort(&vdev->vport);			/* initialize the port struct */
	SetPort((GrafPtr)&vdev->vport);
	ClipRect(&tr);						/* set clip region */

/*
*  Erase the image region in the virtual device. 
*/
	PaintRect(&tr);

/*
*  Restore the environment.
*/
	SetGDevice(v_savegd);
	SetPort((GrafPtr) v_saveport);
	
	return(0);
}

/*******************************************************************************/
/* SetVDevice
*  Set the gdevice and port to our off-screen space.
*  Save the old values for unset.
*/
SetVDevice(vdev)
	VDevicePtr vdev;
{

	GetPort((GrafPtr *)&v_saveport);
	v_savegd = GetGDevice();
	if (!vdev->vgd)
		return(-1);
	SetGDevice(vdev->vgd);
	SetPort((GrafPtr) &vdev->vport);
	
	return(0);
}

/*******************************************************************************/
/* UnsetVDevice
*  Set the vdevice back to the saved values.
*/
UnsetVDevice()
{
	SetGDevice(v_savegd);
	SetPort((GrafPtr) v_saveport);
}

/*******************************************************************************/
/* TrashVDevice
*  Get rid of the devices that we created with InitVDevice.
*
*  Remember to free up the vdev->bp after the gdevice is gone.
*/
TrashVDevice(vdev)
	VDevicePtr vdev;
{
	
	(*(*vdev->vgd)->gdPMap)->baseAddr = NULL;	/* drop old value, bp has a copy */
	
	/* dispose of the color table. CloseCPort seems not to want to do that. */
	
	DisposHandle ((Handle) (**(**vdev->vgd).gdPMap).pmTable);
	
	CloseCPort(&vdev->vport);		/* lose the cport, disposes the pixmap */
	(*vdev->vgd)->gdPMap = NULL;	/* destroy second copy of the pixmaphandle */
	DisposGDevice (vdev->vgd);		/* disposes current gdevice and pixmap */
	DisposPtr ((Ptr) vdev->bp);

}

/*******************************************************************************/
/*  ColorVDevice
*   input:  vdev and 
*        palette handle.
*   
*   Use palette2ctab to install the color table into the off-screen gdevice and
*   make the inverse color table for it.  Also install the palette into the current
*   window.
*/
ColorVDevice(vdev,pal)
	VDevicePtr vdev;
	PaletteHandle pal;
{
	CTabHandle ct;

	ct = (*vdev->vport.portPixMap)->pmTable;
	if (!ct)
		return;

	Palette2CTab( pal, ct );

	(*ct)->ctSeed = GetCTSeed();					/* give it a unique seed */
	(*ct)->ctFlags = 0  /* 0x8000 */;
	
	MakeITable( ct, (*vdev->vgd)->gdITable, 3 );	/* 3-bit inverse table  */

}

