/************************************************************************/
/*																		*/
/*	FILE:		BrotCode.c -- Code for computing Mandelbrots.			*/
/*																		*/
/*	PURPOSE:	Code to communicate with DSP to calculate Mandelbrots.	*/
/*																		*/
/*	AUTHOR:		George T. Warner										*/
/*																		*/
/*	REVISIONS:															*/
/*	03/26/94	Minor cleaning up.										*/
/*	08/22/93	First version.											*/
/*																		*/
/************************************************************************/

#include <Quickdraw.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DSPManager.h"
#include "DSPConstants.h"
#include "BrotCode.h"
#include "ComUtil_ARTAbrot.h"	/* Common */
#include "AB_Alert.h"

struct DSPCPUDeviceParamBlk pb, *pbPtr;
DSPTaskRefNum taskRefNum;
DSPModuleRefNum modRefNum;
DSPSectionRefNum BroBufRefNum;

/* Offscreen pixMap stuff. */
extern GWorldPtr offscreenGWorld;	/* Off screen graphics world. */
extern Rect	GWorldBounds;			/* Boundaries of graphics world. */

int processing_brot = false;
short *data_mem = nil;	/* Array for uploading Mandelbrot data into. */
unsigned long ypoint;	/* Current y being processed. */
int xpix, ypix;
float fxdelta, fydelta;
double distance_per_pixel, fheight;

#define NumElements 128L

typedef struct
{
	long Startflag;
	long Doneflag;
	long Points;
	long Maxiter;
	float Xstart;
	float Delta;
	float Ystart;
	long Inited;
	short OutputData[640];
}
MyPBuff;

MyPBuff *mypbuffptr;


pascal void fatalError(DSPMessagePtr errorBlockPtr)
{
#pragma unused (errorBlockPtr)
char tmp_string[256];

	sprintf(tmp_string, "MessageActionProc called.");
	AB_Alert(tmp_string);
}

void start_brot()
{
char tmp_string[256];
OSErr err;
	
	SetPort(WPtr_ARTAbrot);

	/* Get width of image to calculate. */
	xpix = (WPtr_ARTAbrot->portRect.right-WPtr_ARTAbrot->portRect.left)-SCROLLBARWIDTH;
	/* Get height of image to calculate. */
	ypix = (WPtr_ARTAbrot->portRect.bottom-WPtr_ARTAbrot->portRect.top)-SCROLLBARWIDTH;
	/* Find the distance between pixels. */
	distance_per_pixel = fwidth/(double)xpix;
	/* Find the height of the image. */
	fheight = (double)ypix * distance_per_pixel;

	pb.pbhDeviceIndex		= 0;					/* Just use device 0. */
	pb.pbhClientPermission	= kdspReadPermission;	/* Not sure what this means. */
	pb.pbhDeviceICON		= nil;					/* No icon desired. */
	pb.pbhClientICON 		= nil;
	pb.cpuClientMessageActionProc = fatalError;
	BlockMove("\pARTAbrot", pb.pbhClientName, 32);
	
	pbPtr = &pb;
	
	/* Get information on DSP 0. */
	if (DSPGetIndexedCPUDevice(pbPtr)) {
		sprintf(tmp_string, "Failure: No DSPs attached!");
		AB_Alert(tmp_string);
		return;
	}

	/* Open device driver. */
	if (DSPOpenCPUDevice(pbPtr)) {
		sprintf(tmp_string, "Failure opening DSP device driver.");
		AB_Alert(tmp_string);
		return;
	}
	
	/* Create a new task structure. */
	if (DSPNewTask(pbPtr, &fatalError, "\pMandelbrot", &taskRefNum)) {
		sprintf(tmp_string, "Failure creating new task structure.");
		AB_Alert(tmp_string);
		goto close_device;
	}

	/* Load a module into the task structure. */
	err = DSPLoadModule("\pmbrot", 	/* Load MBrot resource. */
					taskRefNum, 	/* Pass task structure reference number. */
					kdspAnyPositionInsert, /* Put it anywhere. */
					(DSPModuleRefNum)NULL, 	/* Optional reference module number. */
					&modRefNum, 	/* Pointer to get module reference number. */
					NumElements);	/* Scale multiplier (not sure about this yet). */
	if (err) {
		sprintf(tmp_string, "Failure loading MBrot module.");
		AB_Alert(tmp_string);
		goto dispose_task;
	}

	/* Insert Task into Task List */
	if (DSPInsertTask(pbPtr, taskRefNum, kdspAnyPositionInsert, kdspTimeShare, (DSPTaskRefNum)NULL)) {
		sprintf(tmp_string, "Failure inserting task.");
		AB_Alert(tmp_string);
		goto dispose_task;
	}

	/* Get section reference numbers for the parameter buffer. */
	if (DSPGetSection(modRefNum,"\pbrobuf",&BroBufRefNum)) {
		sprintf(tmp_string, "Failure getting parameter buffer reference number.");
		AB_Alert(tmp_string);
		goto remove_task;
	}

	/* Get pointer to parameter buffer. */
	err = DSPGetSectionData(BroBufRefNum,(Ptr *)&mypbuffptr);
	if (err) {
		sprintf(tmp_string, "Failure getting parameter buffer pointer.");
		AB_Alert(tmp_string);
		goto remove_task;
	}

	/* Allocate buffer to hold one line of data. */
	data_mem = (short *)calloc(xpix, sizeof(short));
	if(!data_mem) {
		sprintf(tmp_string, "Failure allocating data buffer.");
		AB_Alert(tmp_string);
		goto remove_task;
	}

	fydelta = fxdelta = fwidth/xpix;
	/* Download left coordinate of image. */
	mypbuffptr->Xstart = fxcenter-(fwidth/2);
	mypbuffptr->Delta = fxdelta;
	/* Download bottom coordinate of image. */
	mypbuffptr->Ystart = fycenter-(fheight/2);
	/* Tell num. of pixels to do. */
	mypbuffptr->Points = xpix;
	mypbuffptr->Maxiter = fiters;
	/* Start with first line. */
	mypbuffptr->Doneflag = 0;
	mypbuffptr->Startflag = 0;

	/* Start calculations! */
	if (DSPSetTaskActive(taskRefNum)) {
		sprintf(tmp_string, "Failure setting task active.");
		AB_Alert(tmp_string);
		goto quit_task;
	}

	/* Wait for DSP module to initialize. */
	while (!mypbuffptr->Inited) {
		/* Force stop waiting if user hits command-period. */
		if (check_stop()) {
			goto inactivate_task;
		}
	}

	/* Start processing first line. */
	mypbuffptr->Startflag = 1;
	ypoint = 0;
	processing_brot = true;

	return;

inactivate_task:
	/* Inactivate task. */
	if (DSPSetTaskInactive(taskRefNum)) {
		sprintf(tmp_string, "Failure setting task inactive.");
		AB_Alert(tmp_string);
	}

quit_task:
	if (data_mem)
		free(data_mem);

remove_task:
	/* Remove the Task */
	if (DSPRemoveTask(taskRefNum)) {
		ShowCursor();
		sprintf(tmp_string, "Failure removing task.");
		AB_Alert(tmp_string);
	}

dispose_task:
	/* Dispose the Task */
	if (DSPDisposeTask(taskRefNum)) {
		ShowCursor();
		sprintf(tmp_string, "Failure disposing task.");
		AB_Alert(tmp_string);
	}

close_device:
	/* Close the CPU Device */
	if (DSPCloseCPUDevice(pbPtr)) {
		ShowCursor();
		sprintf(tmp_string, "Failure closing DSP driver.");
		AB_Alert(tmp_string);
	}

	new_coordinates=FALSE;
	ShowCursor();
}

void continue_brot()
{
Ptr base_addr;
long row_bytes;	/* Used for PixMap calculations. */
Rect rect_to_copy;
unsigned char *pixel_address;
long xpoint;
	
	if (!mypbuffptr->Doneflag) {
		/* If no data to process, just return. */
		return;
	}

	SetPort(WPtr_ARTAbrot);

	/* Lock the offscreen GWorld. */
	LockPixels(offscreenGWorld->portPixMap);
	row_bytes = ((**(offscreenGWorld->portPixMap)).rowBytes) & 0x3fff;
	base_addr = GetPixBaseAddr(offscreenGWorld->portPixMap);

	/* Copy data from DSP buffer. */
	BlockMove(mypbuffptr->OutputData, data_mem, xpix * sizeof(short));

	ypoint++;

	if (ypoint != ypix) { /* Don't start new line if we don't need it. */
		/* Download new Y coordinate. */
		mypbuffptr->Ystart = (fycenter-(fheight/2)) + (ypoint * fxdelta);
		/* Acknowledge that last line was completed. */
		mypbuffptr->Doneflag = 0;
		/* Tell it to go. */
		mypbuffptr->Startflag = 1;
	}

	/* Convert results to bytes for indexed color display. */
	pixel_address = (unsigned char *)((unsigned long)base_addr + ((ypoint-1) * (unsigned long)row_bytes));
	for (xpoint=0; xpoint < xpix; xpoint++)
		*pixel_address++ = data_mem[xpoint];

	SetRect(&rect_to_copy, GWorldBounds.left, GWorldBounds.top+ypoint-1, GWorldBounds.right, GWorldBounds.top+ypoint); /* left, top, right, bottom */

	/* Display line on screen. */
	CopyBits((BitMap *)*offscreenGWorld->portPixMap, &(WPtr_ARTAbrot->portBits), &(rect_to_copy), &(rect_to_copy), srcCopy, 0);

	UnlockPixels(offscreenGWorld->portPixMap);

	if (ypoint == ypix) {
		/* If we are done, clean up. */
		finish_brot();
	}
}


void finish_brot()
{
char tmp_string[256];

	processing_brot = false;
	
	while (!mypbuffptr->Doneflag) {
		/* Wait for last line to complete. */
		if (check_stop()) {
			/* If we can't wait or are hung, just get out. */
			break;
		}
	}

	/* Inactivate task. */
	if (DSPSetTaskInactive(taskRefNum)) {
		sprintf(tmp_string, "Failure setting task inactive.");
		AB_Alert(tmp_string);
	}

	if (data_mem)
		free(data_mem);

	/* Remove the Task */
	if (DSPRemoveTask(taskRefNum)) {
		ShowCursor();
		sprintf(tmp_string, "Failure removing task.");
		AB_Alert(tmp_string);
	}

	/* Dispose the Task */
	if (DSPDisposeTask(taskRefNum)) {
		ShowCursor();
		sprintf(tmp_string, "Failure disposing task.");
		AB_Alert(tmp_string);
	}

	/* Close the CPU Device */
	if (DSPCloseCPUDevice(pbPtr)) {
		ShowCursor();
		sprintf(tmp_string, "Failure closing DSP driver.");
		AB_Alert(tmp_string);
	}

	new_coordinates=FALSE;
}



