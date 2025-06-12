#ifndef lint
static char *SCCSid = "%W%	(NCSA)	%G%";
#endif
/*
*    binsubs.c
*
*	MacBinary Subroutines.	
*
*	Called by:
*		bkgr.c
*/

#include "stdio.h"
#include "memory.h"
#include "fcntl.h"
#include "mpw.h"
#include "whatami.h"

#include "Windows.h"
#include "Files.h"
#include "Packages.h"

#include "MacBinary.h"


char *strncpy();

#define BLOCKS(x)	((x+127)/128)
#define lmove(f,t)	movmem(f,t,4)

MBHead
	*mbh;
char 
	buffer[128];

char *strsave( p)
char *p;
{
	char *t;

	t= (char *)NewPtr(strlen(p));
	strncpy(t,p,strlen(p));
	
	return(t);
}

GetFileInfo(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{
	iop->ioNamePtr = name;
	iop->ioVRefNum=vol;
	iop->ioFVersNum=iop->ioFDirIndex=0;
	PBGetFInfo((ParmBlkPtr) iop, FALSE);
}

SetFileInfo(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{
	iop->ioNamePtr = name;
	iop->ioVRefNum=vol;
	iop->ioFVersNum=iop->ioFDirIndex=0;
	PBSetFInfo( (ParmBlkPtr) iop, FALSE);
}

MakeTextFile(vol,name,iop)
short vol;
char *name;
FileParam *iop;
{	GetFileInfo(vol,name,iop);
	iop->ioFlFndrInfo.fdType='TEXT';
	iop->ioFlFndrInfo.fdCreator='EDIT';
	SetFileInfo(vol,name,iop);
}

isMacBinary(p)
MBHead *p;
{
	return( (p->nlen > 0)   &&
			(p->nlen < 65)  &&
			(p->zero1 == 0) &&
			(p->zero2 == 0) &&
			(p->zero3 == 0));
}

MBsize( mbfp )
MBFile *mbfp;
{
	long int size;
	int ret;
	
	size = 0;
	
	ret = GetEOF( mbfp->fd, &size );			/* length of file data fork */
	if (ret != noErr) 
		size = 0;

	return(size);
}

MBFile *MBopen( file, vrefnum, mode)
char *file;
short vrefnum,mode;
{
	MBFile *mbfp;
	int err;

	if (mode & MB_WRITE)
		putln("MBOpen for write");
	else 
		putln("MBOpen for read");

	if (mode & MB_DISABLE)
		putln("MacBinary Protocol Disabled");

	mbfp= (MBFile *)NewPtr( sizeof(MBFile));
	if (mbfp==0L)
		return(0L);
	movmem( file, mbfp->name,64);
	putln(mbfp->name);
	c2pstr(mbfp->name);
	mbfp->vrefnum=vrefnum;
	mbfp->mode = mode;

	if ((err=FSOpen( mbfp->name, vrefnum, &mbfp->fd))) {
		if ((err==-43) && (mode & MB_WRITE)) {
			Create( mbfp->name, vrefnum, '????','TEXT');
			if (FSOpen( mbfp->name, vrefnum, &mbfp->fd)) 
				return( 0L);
			}
		else 
			return(0L);
		}

	mbfp->binary=0;
	mbfp->sector1=1;
	mbfp->fork=0;
	return( mbfp);
}

MBwrite( out, buffer, size)
MBFile *out;
int size;
char *buffer;
{
	int rsize;
	
	if (size < 1)
		return(0);

	rsize=size;

	if (out->sector1 && (size >= sizeof(struct MBHead)) &&
		isMacBinary(buffer) && (!(out->mode & MB_DISABLE))) {
		putln("First sector of MacBinary file");
		ProcessMBHead( out, buffer);
		buffer+=128;
		if ((size-=128) <1)
			return(rsize);
		}

	if (bwrite( out,buffer,size))
		return(-1);
	else
		return( rsize);
}

bwrite( out, buffer, size)
MBFile *out;
int size;
char *buffer;
{
	long len=size;
	int error;

	error = 0;
	
	if (out->binary) {
		if (out->bytes>0) {
			if (out->bytes < len) len = out->bytes;
			error= FSWrite( out->fd, &len, buffer);
			out->bytes -= len;
			buffer +=len;
			size -= (int)len;
			}
		if (out->bytes<= 0) {
			if (!out->fork) {
				out->fork=1;
				out->bytes=BLOCKS(out->rlen)*128;
				SetEOF( out->fd, (long) out->dlen);
				FSClose( out->fd);
				if (out->bytes) {
					OpenRF( out->name, out->vrefnum,&out->fd);
					if (size) {
						len = (long) size;
						error= FSWrite( out->fd, &len, buffer);
						}
					}
				else
					out->fd=0;
				}
			else SetEOF( out->fd, (long) out->rlen);
			}
		}
	else {
		error = FSWrite( out->fd, &len, buffer);
		}
	return (error);
}

MBclose( out)
MBFile *out;
{
	FileParam finfo;
	long fpos;

	putln("MBclose");
	
	if (!out->fd)
		return;

	if (!(out->mode & MB_DISABLE) && (out->mode & MB_WRITE)) {
		if (out->fork)
			SetEOF( out->fd, (long) out->rlen);
		else
			SetEOF( out->fd, (long) out->dlen);

		FSClose( out->fd);
		GetFileInfo( 0, out->name, &finfo);
	
		movmem( &out->header.type[0], &finfo.ioFlFndrInfo, sizeof(FInfo));
		lmove( &out->header.cdate[0], &finfo.ioFlCrDat);
		lmove( &out->header.mdate[0], &finfo.ioFlMdDat);
		finfo.ioFlFndrInfo.fdFlags &= 0xfeff;
		finfo.ioFlRLgLen=out->rlen;
		finfo.ioFlLgLen =out->dlen;
	
		SetFileInfo( 0, out->name, &finfo);
		}
	else if (out->mode & MB_WRITE) {
		GetFPos( out->fd, &fpos);
		SetEOF(  out->fd,  fpos);
		FSClose( out->fd);
		}
	else
		FSClose( out->fd);
}

ProcessMBHead( out, header)
MBFile *out;
MBHead *header;
{
	int err;

	movmem( header, &out->header, sizeof(MBHead));
	out->binary=1;
	lmove( &header->dflen[0], &out->dlen);
	lmove( &header->rflen[0], &out->rlen);
	out->bytes= BLOCKS(out->dlen)*128;
	out->fork=0;
	out->sector1=0;

	FSClose(out->fd);
	if (FSDelete( out->name, out->vrefnum))
		putln("Error Deleting Old File ");
		
	movmem( &out->header.nlen, out->name,63);
	
	MBstat( &out->header.nlen, 1, (int)(BLOCKS(out->dlen)+BLOCKS(out->rlen)) );

	if (out->bytes) {
		if ((err=FSOpen( out->name, out->vrefnum, &out->fd))) {
			if (err=-43) {
				long cre,typ;

/*  this crashes Mac Pluses				
				typ = *(long *)out->header.type;
				cre = *(long *)out->header.creator;
*/
				lmove(out->header.type, &typ);
				lmove(out->header.creator, &cre);

				Create( out->name, out->vrefnum, cre,typ);
				if (FSOpen( out->name, out->vrefnum, &out->fd)) 
					return( 0L);
				}
			else {
				return(0L);
				}
			}
		}
	else {
		if ((err=OpenRF( out->name, out->vrefnum, &out->fd))) {
			if (err=-43) {
				long cre,typ;
				
/*  this crashes Mac Pluses				
				typ = *(long *)out->header.type;
				cre = *(long *)out->header.creator;
*/
				lmove(out->header.type, &typ);
				lmove(out->header.creator, &cre);

				Create( out->name, out->vrefnum, cre,typ);
				if (OpenRF( out->name, out->vrefnum, &out->fd)) 
					return( 0L);
				}
			else {
				return(0L);
				}
			}
		out->fork = 1;
		out->bytes=BLOCKS(out->rlen)*128;
		}
}

MBread( in, buffer, size)
MBFile *in;
int size;
char *buffer;
{
	char *p;
	int rsize=size;


	if (in->fork<0) {
		return(-1);
		}

	p=buffer;
	if (in->sector1) {
		FileParam finfo;

		setmem( &in->header, sizeof(MBHead), 0);
		movmem(  in->name, &in->header.nlen, 64);
		GetFileInfo( in->vrefnum, in->name, &finfo);
		movmem( &finfo.ioFlFndrInfo, &in->header.type[0], sizeof(FInfo) );
		in->header.protected = (in->header.zero2 & 0x40)?1:0;
		in->header.zero2 = 0;
		lmove( &finfo.ioFlLgLen, &in->header.dflen[0]);
		lmove( &finfo.ioFlRLgLen,&in->header.rflen[0]);
		lmove( &finfo.ioFlCrDat, &in->header.cdate[0]);
		lmove( &finfo.ioFlMdDat, &in->header.mdate[0]);
		in->dlen=finfo.ioFlLgLen;
		in->rlen=finfo.ioFlRLgLen;
		if (! (in->mode & MB_DISABLE) ) {
			if (size<128) return(-1);

			movmem( &in->header, p, 128);
			p +=128;
			size -= 128;
			in->bytes= BLOCKS(in->dlen)*128;
			in->binary=1;
			}
		else {
			in->bytes = in->dlen;
			in->rlen=0;
			in->binary=0;
			}
		in->sector1=0;
		MBstat( &in->header.nlen, 1, (int) (BLOCKS(in->dlen)+BLOCKS(in->rlen)) );
		}

	if ( size >0) {
		long length = (long)size;
		int err;

		err = FSRead( in->fd, &length, p);

		size -=(int)length;
		in->bytes -=length;
		p += length;

		if (err == -39 || (in->bytes<=0) ) {
			FSClose( in->fd );
			if (in->bytes<0L) in->bytes=0L;
			size -= (int)in->bytes;
			p    +=      in->bytes;				/* Make adjustments for necessary 128 byte term */
			if (!in->fork ) {
				in->fork=1;
				in->bytes= BLOCKS(in->rlen)*128;
				if (in->bytes) {
					OpenRF( in->name, in->vrefnum, &in->fd);
#ifdef READ
					length=(long)size;
					if (length >0L) {
						err = FSRead( in->fd, &length, p);
						size -= (int)length;
						in->bytes -=length;
						}
#endif READ
					}
				else {
					in->fd=0;
					in->fork=-1;					/* Time to close up shop */
					}
				}
			else {
				in->fd=0;
				in->fork=-1;					/* Time to close up shop */
				}
			}
		}
	return( rsize-size); 
}
