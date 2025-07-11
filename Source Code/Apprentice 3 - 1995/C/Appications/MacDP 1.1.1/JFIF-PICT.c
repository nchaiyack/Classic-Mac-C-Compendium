/*
	File:		JPEGtoPICT.c

	Written by:	Mark Krueger, Apple Computer, Inc.

	Contains:	Converts JPEG interchange files to JPEG PICT files, which can be used by any application.
	
	
				The JPEG File  Interchange Format ( JFIF) is a cross platform standard file format for
				storing JPEG compressed image files. This application shows you how you can easily convert
				these to QuickTime PICT format, or vice-verse. 
				
				You can use this in your application to open JFIF files directly or put a user interface 
				( Drag and Drop would be cool ) on it to make a conversion program ). When files are to 
				be used on the Mac it is best to keep them in QTPict format so they can be easily copied
				and pasted, but JFIF format is useful for transfering data to other platforms that cannot
				use PICT formated JPEG images. 
				
				NOTE: the PICT to JFIF format translator is incomplete in that it only converts PICT
				files which are already in QuickTIme JPEG format and it does not handle banded JPEG
				Picts ( which may be commonly created by QuickTime applications which call 
				CompressPictureFile in low memory conditions or which create their own banded picts)
				To fully handle these images, the individual bands would have to be converted into a 
				single JPEG stream when put into JFIF format, and this code does not show you how
				to do that. 
			
	Modifications: November 19, 1994 by SPDsoft for MacDP 1.0


*/

#define SHOW_IT

#include	<GestaltEqu.h>
#include	<ImageCompression.h>

/************************************************
 *
 *	Function prototypes.
 *
 ************************************************/



char *
MarkerDetect(char *data,short *width,short *height,long *hRes,long *vRes,short *depth);

void 
SwallowHuffTable(char *data);

void 
SwallowQuantTable(char *data);

ImageDescriptionHandle
ScanJPEG(short originalFile,Ptr *data,OpenCPicParams *pictureHeader);

PicHandle
ConvertFromJFIF(StringPtr name,short originalFile, CWindowPtr window );

PicHandle
ConvertFromPict(StringPtr name,short originalFile, CWindowPtr w);

pascal void
StdPixExtract(PixMap *src,Rect *srcRect,MatrixRecord matrix,short mode,RgnHandle mask,PixMap *matte,
	Rect *matteRect,short flags);
	
pascal void GetPICTData ( Ptr dataPtr, short byteCount);


/************************************************
 *
 *	Convert a JFIF file to a PICT file.
 *
 ************************************************/



PicHandle
ConvertFromJFIF(StringPtr name,short originalFile, CWindowPtr window)
{
	OSErr	result = 0;
	short compressedFile = 0;
	OpenCPicParams	header;
	Str31		newName;
	long		l;
	short		i;
	char		zero=0;
	MatrixRecord matrix;

	Ptr			data = nil;
	ImageDescriptionHandle	desc = nil;
	PicHandle	originalPicture = nil;

	
	/************************************************
	 *
	 *	Extract the JPEG data if possible and create Image Description.
	 *
	 ************************************************/

	if ( (desc=ScanJPEG(originalFile,&data,&header)) == 0 ) {
		result = -50;
		goto done;
	}

	originalPicture = OpenCPicture(&header);
		
		
	/************************************************
	 *
	 * we have to use the FDecompressImage call to make sure that the accuracy
	 * param gets set to highQuality which ensures a better quality decode
	 * and real dithering when going to 8-bit screens.
	 *
	 ************************************************/
		
		
	SetIdentityMatrix(&matrix);
	
/*SysBeep(1);	*/
	if ( (result=FDecompressImage(data,desc,((CGrafPtr)qd.thePort)->portPixMap,
			&header.srcRect,&matrix,ditherCopy,(RgnHandle)nil,
			(PixMapHandle)nil,(Rect *)nil,codecHighQuality,anyCodec,0,
			(ICMDataProcRecordPtr)nil,(ICMProgressProcRecordPtr)nil)) ){
		ClosePicture();
		KillPicture((PicHandle)originalPicture);
		originalPicture = nil;
		goto done;
	}
/*SysBeep(1);	*/

	ClosePicture();
	
	if ( GetHandleSize((Handle)originalPicture) == sizeof(Picture) ) {
		KillPicture((PicHandle)originalPicture);
		originalPicture = nil;
		goto done;
	}


done:	
	FSClose(originalFile);

/*	if ( originalPicture )*/
/*		KillPicture(originalPicture);*/

	if ( data )
		DisposPtr(data);
	if ( desc ) 
		DisposHandle((Handle)desc);

	return originalPicture;
}


/************************************************

	Scan a file for valid JPEG data, and fill in a picture header and ImageDescription
	for it.

*************************************************/

ImageDescriptionHandle
ScanJPEG(short originalFile,Ptr *data,OpenCPicParams *pictureHeader)

{
	short w,h;
	ImageDescriptionHandle desc;
	long	l;
	char	*bitStream,*scanData,*buffer;
	long	hRes = 72L<<16,vRes = 72L<<16;
	short	depth = 32;
	
	GetEOF(originalFile,&l);
	if ( (buffer= NewPtr(l)) == nil ) {
		return(0);
	}
	FSRead(originalFile,&l,buffer);
	bitStream = buffer;
	
	if ( (desc = (ImageDescriptionHandle)NewHandle(sizeof(ImageDescription))) == nil ) {
		return(0);
	}
	if ( (scanData=MarkerDetect(bitStream,&w,&h,&hRes,&vRes,&depth)) == 0 ) {
		return(0);
	}
	(*desc)->idSize = sizeof(ImageDescription);
	(*desc)->width = w;
	(*desc)->height = h;
	(*desc)->temporalQuality = 0;
	(*desc)->spatialQuality = codecNormalQuality;
	(*desc)->dataSize = l;
	(*desc)->cType = 'jpeg';
	(*desc)->version = 0;
	(*desc)->revisionLevel = 0;
	(*desc)->vendor = 0;
	(*desc)->hRes = hRes;
	(*desc)->vRes = vRes;
	(*desc)->depth = depth;
	(*desc)->clutID = -1;
	BlockMove("\pPhoto",(*desc)->name,6);
	SetRect(&pictureHeader->srcRect,0,0,w,h);
	pictureHeader->version = -2;
	pictureHeader->reserved1 = 0;
	pictureHeader->reserved2 = 0;
	pictureHeader->hRes = hRes;
	pictureHeader->vRes = vRes;
	*data = bitStream;
	return(desc);
}



/**********************************************************************

	JPEG specific stuff.
	
***********************************************************************/

/*

	JPEG Marker code definitions.
	
*/

#define	MARKER_PREFIX	0xff
#define	MARKER_SOI	0xd8		/* start of image */
#define	MARKER_SOF	0xc0		/* start of frame */
#define	MARKER_DHT	0xc4		/* define Huffman table */
#define	MARKER_EOI	0xd9		/* end of image */
#define	MARKER_SOS	0xda		/* start of scan */
#define	MARKER_DQT	0xdb		/* define quantization tables */
#define	MARKER_DNL	0xdc		/* define quantization tables */
#define	MARKER_DRI	0xdd		/* define Huffman table */
#define	MARKER_COM	0xfe		/* comment */
#define MARKER_APP0	0xe0		


/**********************************************************************

	Read the quantization table from the JPEG bitstream.
	
***********************************************************************/

void 
SwallowQuantTable(char *data)
{
	long	i;
	long	length,pm,nm;

	length = *(short *)data;			/* read length */
	length -= 2;
	data += 2;
	while ( length ) {
		nm= *data++;					/* read precision and number */
		pm = nm>>4;	
		nm &= 0xf;
		length--;
		if ( pm ) {
			for(i=0;i<64;i++) {
				length -= 2;
				data += 2;
			}
		} else {
			for(i=0;i<64;i++) {
				length--;
				data++;
			}
		}
	}	
}

/**********************************************************************

	Read the huffman table from the JPEG bitstream.
	
***********************************************************************/

void 
SwallowHuffTable(char *data)
{
	short	i,tc,id;
	long	length;
	
	unsigned char	bin[17];
	unsigned char	val[256];

	bin[0] = 0;
	length = *(short *)data;			/* read length */
	data += 2;
	length -= 2;
	while ( length ) {
		id=*data++;				/* read id */
		length--;
		if ( id != 0 && id != 1 && id != 0x10 && id != 0x11) {
			return;
		}
		tc = 0;
		for(i=0;i<16;i++) {
			length--;
			tc += (bin[i+1] = *data++);
		}
		for (i=0; i < tc; i++ ) {
			length--;
			val[i] = *data++;
		}
	}
}
	
	
	
/**********************************************************************

	Scan the JPEG stream for the proper markers and fill in the image parameters
	
	returns nil if it cant comprehend the data, otherwise a pointer to the start
	of the JPEG data.
	
	
	It does a cursory check on the JPEG data to see if it's reasonable.
	Check out the ISO JPEG spec if you really want to know what's going on here.
	
***********************************************************************/

char *
MarkerDetect(char *data,short *width,short *height,long *hRes,long *vRes,short *depth)
{
	short	frame_field_length;
	short	data_precision;
	short	scan_field_length;
	short	number_component,scan_components;
	short	c1,hv1,q1,c2,hv2,q2,c3,hv3,q3;
	short	dac_t1, dac_t2, dac_t3;
	unsigned char	c;
	short	qtabledefn;
	short	htabledefn;
	short	status;
	short	length;
	short	i;
	
	c = *data++;
	qtabledefn = 0;
	htabledefn = 0;
	status = 0;
	while (c != (unsigned char)MARKER_SOS) {
		while (c != (unsigned char)MARKER_PREFIX)
			c = *data++;						/* looking for marker prefix bytes */
		while (c == (unsigned char)MARKER_PREFIX)
			c = *data++;						/* (multiple?) marker prefix bytes */
		if (c == 0)
			continue;									/* 0 is never a marker code */

		if (c == (unsigned char)MARKER_SOF) {

			frame_field_length = *(short *)data;
			data += 2;
			data_precision = *data++;
			
			if ( data_precision != 8 ) { 
				status = 2;
			}

			*height = *(short *)data;
			data += 2;
			*width = *(short *)data;
			data += 2;
						
			number_component = *data++;
			
			switch ( number_component  ) {
			case 3:
				c1 = *data++;
				hv1 = *data++;
				q1 = *data++;
				c2 = *data++;
				hv2 = *data++;
				q2 = *data++;
				c3 = *data++;
				hv3 = *data++;
				q3 = *data++;
				*depth = 32;
				break;
			case 1:		
				c1 = *data++;
				hv1 = *data++;
				q1 = *data++;
				*depth = 40;
				break;
			default:
				status = 3;
				break;
			}
			continue;
		}
	
		if (c == (unsigned char)MARKER_SOS) {
			short tn;
			scan_field_length = *(short *)data;
			data += 2;
			scan_components = *data++;
			for ( i=0; i < scan_components; i++ ) {
				unsigned char cn,dac_t;
				
				cn = *data++;
				dac_t = *data++;
				if ( cn == c1 ) {
					dac_t1 = dac_t;
				} else if ( cn == c2 ) {
					dac_t2 = dac_t;
				} else if ( cn == c3 ) {
					dac_t3 = dac_t;
				} else {	
					status = 29;
					break;
				}
			}
			switch ( tn=(dac_t1 & 0xf) )  {
			case 0:
			case 1:
				break;
			case 0xf:
				break;
			default:
				status = 33;
				break;
			}
			switch (  tn=(dac_t2 & 0xf) )  {
			case 0:
			case 1:
				break;
			case 0xf:
				break;
			default:
				status = 33;
				break;
			}
			switch (  tn=(dac_t3 & 0xf) )  {
			case 0:
			case 1:
				break;
			case 0xf:
				break;
			default:
				status = 33;
				break;
			}


			/*  Initialize the DC tables */
			
			switch (  tn=dac_t1 & 0xf0 )  {
			case 0:
			case 0x10:
				break;
			case 0xf0:
				break;
			default:
				status = 34;
				break;
			}
			switch (  tn=dac_t2 & 0xf0 )  {
			case 0:
			case 0x10:
				break;
			case 0xf0:
				break;
			default:
				status = 34;
				break;
			}
			switch (  tn=dac_t3 & 0xf0 )  {
			case 0:
			case 0x10:
				break;
			case 0xf0:
				break;
			default:
				status = 34;
				break;
			}
			if ( *data++ != 0 )  {
//				status = 18;
			}
			if ( *data++ != 63 )  {
//				status = 19;
			}
			if ( *data++ != 0 ) {
//				status = 20;
			}
			if ( status )
				return(0);
			else
				return(data);
		}

		if (c == (unsigned char)MARKER_DQT) {
			scan_field_length = *(short *)data;
			SwallowQuantTable(data);
			data += scan_field_length;
			continue;
		}
		if (c == (unsigned char)MARKER_DHT) {
			scan_field_length = *(short *)data;
			SwallowHuffTable(data);
			continue;
		}
		if (c == (unsigned char)MARKER_DRI) {
			length = *(short *)data;			/* read length */
			data += 2;
			length = *(short *)data;			
			data += 2;
			continue;
		}
		if (c == (unsigned char)MARKER_DNL) {
			length = *(short *)data;			/* read length */
			data += 2;
			length = *(short *)data;			
			data += 2;
			continue;
		}
		if (c >= (unsigned char)0xD0 && c <= (unsigned char)0xD7) {
			continue;
		}

		if (c == (unsigned char)MARKER_SOI || c == (unsigned char)MARKER_EOI)	/* image start, end marker */
			continue;

		if ( (c >= (unsigned char)0xC1 && c <= (unsigned char)0xcF) || (c == (unsigned char)0xde) || (c == (unsigned char)0xdf) ) {
			status = 12;
			length = *(short *)data;			/* read length */
			data += length;
			continue;
		}
		if (c >= (unsigned char)MARKER_APP0 && c <= (unsigned char)0xEF) {
			length = *(short *)data;			/* read length */
			data += 2;
			length -= 2;
			if ( (c == (unsigned char)MARKER_APP0) && length > 5 ) { /* check for JFIF marker */
				char buf[5];
				buf[0] = *data++;
				buf[1] = *data++;
				buf[2] = *data++;
				buf[3] = *data++;
				buf[4] = *data++;
				length -= 5;
				
				if ( buf[0] == 'J' && buf[1] == 'F'  && buf[2] == 'I'  && buf[3] == 'F' ) {
					short	units;
					long	xres,yres;
					short	version;
					
					
					version = *(short *)data; data += 2;length -= 2;

#ifdef strict_version_check					
					if ( version != 0x100 && version != 0x101 ) {
						status = 44;		// unknown JFIF version
						break;
					}
#else
					if ( version < 0x100 ) {
						status = 44;		// unknown JFIF version
						break;
					}
#endif					
					units = *data++; length--;
					xres = *(short *)data; data += 2; length -= 2;
					yres = *(short *)data; data += 2; length -= 2;

					switch ( units ) {
					case 0:			// no res, just aspect ratio
						*hRes = FixMul(72L<<16,xres<<16);
						*vRes = FixMul(72L<<16,yres<<16);
						break;
					case 1:			// dots per inch
						*hRes = xres<<16;
						*vRes = yres<<16;
						break;
					case 2:			// dots per centimeter (we convert to dpi )
						*hRes = FixMul(0x28a3d,xres<<16);
						*vRes = FixMul(0x28a3d,xres<<16);
						break;	
					default:
						break;
					}
					xres = *data++; length--;
					yres = *data++; length--;
					
					/* skip JFIF thumbnail */
					
					xres *= yres;
					data += xres*3; length -= xres*3;
					
					if (  length != 0 ) {
						status = 44;		// bad jfif marker
						break;
					}
				}
			}
			data += length;
			continue;
		}
		if (c == (unsigned char)MARKER_COM) {
			length = *(short *)data;			/* read length */
			data += length;
			continue;
		}
		if (c >= (unsigned char)0xf0 && c <= (unsigned char)0xfd) {
			length = *(short *)data;			/* read length */
			data += length;
			continue;
		}
		if ( c == 0x1 )
			continue;
		if ( (c >= (unsigned char)0x2 && c <= (unsigned char)0xbF) ) {
			length = *(short *)data;			/* read length */
			status = 13;
			data += length;
			continue;
		}
	}
	return(0);
}





short extractionFile;
OSErr extractionErr;
short globalRef;

PicHandle
ConvertFromPict(StringPtr name,short originalFile,CWindowPtr w)
{
	CQDProcs	cprocs;
	CQDProcsPtr savedprocs;
	CGrafPtr	savePort;
	OSErr		result = 0;
	PicHandle	thePic = nil, OutPict = nil;
	long		longCount,myEOF,filePos;
	Rect		destRect;
	long		hRes = 72L<<16,
				vRes = 72L<<16;
	
	OpenCPicParams	Header;
		
	GetPort((GrafPtr *) &savePort);
	SetPort((GrafPtr) w);
	globalRef = originalFile;
	
	
	thePic = (PicHandle)NewHandle(sizeof(Picture));

	result = GetEOF(originalFile,&myEOF);
	if (result != noErr) goto done;

	result = SetFPos(originalFile,fsFromStart,512);	/*skip header*/
	if (result != noErr) goto done;

	longCount = sizeof(Picture);
	result = FSRead(originalFile,&longCount,(Ptr)*thePic);
	if (result != noErr) goto done;
	
	
	BlockMove(&(*thePic)->picFrame, &destRect, sizeof(Rect));
	BlockMove(&destRect, &(Header.srcRect), sizeof(Rect));
	Header.version = -2;
	Header.reserved1 = 0;
	Header.reserved2 = 0;
	Header.hRes = hRes;
	Header.vRes = vRes;
	
	OutPict=OpenCPicture(&Header);
	savedprocs = (CQDProcsPtr) qd.thePort->grafProcs;
	
		SetStdCProcs(&cprocs);
		cprocs.getPicProc = GetPICTData;
//		cprocs.newProc1 = (Ptr)StdPixExtract;
		qd.thePort->grafProcs = (QDProcsPtr) &cprocs;
		

		DrawPicture( thePic, &destRect);
		
	
	qd.thePort->grafProcs = (QDProcsPtr) savedprocs;
	ClosePicture();
	
	if (EmptyRect(&(*OutPict)->picFrame))
	{
		SysBeep(1);
		result = 1;
	}
				
done:
	FSClose(originalFile);

	if(thePic)
		DisposHandle((Handle)thePic);
	thePic=nil;
	
	if (result!=noErr)
	{	
		if(OutPict)
			DisposHandle((Handle)OutPict);
		OutPict=nil;
	}
	SetPort((GrafPtr)savePort);
	return(OutPict);
}


/*replacement for the QuickDraw bottleneck routine*/

pascal void GetPICTData ( Ptr dataPtr, short byteCount)
{
	OSErr 	err;
	long 	longCount;

	longCount = byteCount;
	err = FSRead(globalRef,&longCount,dataPtr);
		/*can't check for an error because we don't know how to handle it*/
}


/************************************************************************************

	bottle neck proc to extract data from quicktime pict
	
*************************************************************************************/


pascal void
StdPixExtract(PixMap *src,Rect *srcRect,MatrixRecord matrix,short mode,RgnHandle mask,PixMap *matte,
	Rect *matteRect,short flags) 
{


#pragma	unused(srcRect,matrix,matte,matteRect,flags,mode,mask)

	ImageDescriptionHandle	idh;
	long buffSize;
	ICMDataProcRecord dataProc;
	ICMProgressProcRecord progressProc;
	Ptr	data;
	
	long		lb,l;
	short		i,m;
	char 		*buf,*abuf = 0,tbuf[32];

	
	
	/************************************************************
	
	 first check to see if it's a compressed pict opcode  
	 
	************************************************************/
	
	if ( GetCompressedPixMapInfo(src,&idh,&data,&buffSize,&dataProc,&progressProc) == 0 ) {
	
	
		/*******************************
		
		 verify compression type 
		 
		 ********************************/
		
		
		if (  (*idh)->cType != 'jpeg' ) {
			extractionErr = paramErr;		// we only wanted jpeg
			return;
		}


		l = (*idh)->dataSize;
	

		/********************************
		
		   make a JFIF header opcode in case the data doesn't have one -
		   QT JPEG always includes one but other JPEG compressors may not 
		   
		********************************/
		  
	
		{
			buf = tbuf;
			m = 0;
			buf[m++] = MARKER_PREFIX;				// marker prefix
			buf[m++] = MARKER_APP0;				// APP0 marker
			buf[m++] = 0;					// size word ( filled in later )
			buf[m++] = 0;
			buf[m++] = 'J';					// tag 5 bytes
			buf[m++] = 'F';
			buf[m++] = 'I';
			buf[m++] = 'F';
			buf[m++] = 0;		
			buf[m++] = 1;					// version word
			buf[m++] = 1;
			buf[m++] = 1;					// specify dpi
			buf[m++] = (*idh)->hRes>>24;	// hres word
			buf[m++] = (*idh)->hRes>>16;
			buf[m++] = (*idh)->vRes>>24;	// vres word
			buf[m++] = (*idh)->vRes>>16;
			buf[m++] = 0;					// thumbnail x 
			buf[m++] = 0;					// thumbnail y 
		}
		buf[2] = (m-2)>>8;					// fill in size
		buf[3] = (m-2);
		i = 0;
		
		/********************************
		
		Scan the JPEG data till we get to the start of image marker.
		
		********************************/
		
		
		while ( data[i] == (char)MARKER_PREFIX ) {
			if ( i++ > 1024 ) {
				extractionErr = -1;
				if ( abuf )
					DisposPtr(abuf);
				return;
			}
		}
		if ( data[i++] != (char)MARKER_SOI ) { 
			extractionErr = -1;
			if ( abuf )
				DisposPtr(abuf);
			return;
		}
		
		/*******************************
		
		Write out all the data up to and including the start of image marker
		
		*******************************/
		
		lb = i;
		if ( dataProc.dataProc ) {
			if ( extractionErr = dataProc.dataProc(&data,lb,dataProc.dataRefCon))
				return;
		}
		if ( extractionErr = FSWrite(extractionFile,&lb,data))  {
			if ( abuf )
				DisposPtr(abuf);
			return;
		}
		
		/*******************************
		
		Check to see if there is a JFIF marker in the data already
		
		*******************************/
	
		
		if ( data[i] == (char)MARKER_PREFIX &&  data[i+1] == (char)MARKER_APP0 ) {	// already have JFIF marker in stream
			data += i;
			l -= i;
			goto no_jfif_marker;
		} else {
			data += i;
			l -= i;
		}
	
		lb = m;
		if ( extractionErr = FSWrite(extractionFile,&lb,buf))  {
			if ( abuf )
				DisposPtr(abuf);
			return;
		}
no_jfif_marker:
		if ( abuf )
			DisposPtr(abuf);
	
		/*******************************
		
		Read the rest of the JPEG data and write it into the output file.
		
		*******************************/
	
	
		if (  dataProc.dataProc ) {
			lb = buffSize;
			while ( l > buffSize ) {
				if ( extractionErr = dataProc.dataProc(&data,lb,dataProc.dataRefCon))  {
					return;
				}
				if ( extractionErr = FSWrite(extractionFile,&lb,data)) {
					return;
				}
				data += lb;
				l -= lb;
			}
			if ( extractionErr = dataProc.dataProc(&data,l,dataProc.dataRefCon)) {
				return;
			}
			if ( extractionErr = FSWrite(extractionFile,&l,data)) {
				return;
			}
		} else {
			if ( extractionErr = FSWrite(extractionFile,&l,data))
				return;
		}
	
		

	} else {
	
		/*******************************
		
		Not compressed data - return error.
		
		*******************************/
	

		extractionErr = paramErr;
	}
}


