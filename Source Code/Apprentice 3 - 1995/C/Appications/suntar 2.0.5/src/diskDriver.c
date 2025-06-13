/*******************************************************************************\

disk driver interface module

part of suntar ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/* #define PROVE */



#include "PB_sync.h"
#include "antiglue.h"

#ifndef THINK_C_5
#include	<DiskDvr.h>
#include	<DeviceMgr.h>
#else
#include    <Disks.h>
#include    <files.h>
#define ioRefNum ioCRefNum
#endif

#include "windows.h"
#include "suntar.h"
#include <string.h>


#define SINGLE_SIDED	2
#define TWO_SIDED	3
#define SUPERDRIVE	4


extern short n_superdrives,last_drive,preferred_drive;
extern short drive_type[max_drive];
extern Boolean disco_espulso;

short Sony_driver;

unsigned char first_sector[512];	/* used to verify that the disk was not replaced */
long last_disk_operation;

OSErr get_format(DrvSts*,char*);
Boolean is_msdos_disk(short);
void verify_sectors(sector_t,char *,short);
static OSErr my_DriveStatus(CntrlParam *parblk);
Boolean check_changed_disk(void);
short get_driver_name(short,unsigned char *);


short FindDriverByName( void *);
Boolean volume_in_place(short);
void test_tape_EOF(void);
void reposition_tape(void);

enum os_type riconosci_disco_mac(sector_2)
unsigned char sector_2[512];
{
/* sector 2 contains the essential informations about the disk in both MFS and HFS */
enum os_type i;

if( *(short*) sector_2== 0xD2D7 )
	i=mac_MFS;
else if( *(short*) sector_2== 0x4244)
	i=mac_HFS;
else
	return unknown_os;

if( sector_2[36]<=27 /*lunghezza del nome*/ &&
 	   *(short*)&sector_2[12]>0 /* numero di files nel disco */)
	return i;
else
 	return unknown_os;	/* two possible 16 bit values means 1 probability
 						out of 32768 to recognize as Mac disk a non-Mac one,
 						testing some other fields decreases it... */
}


static Boolean is_msdos_disk(drive_n)
short drive_n;
{
/* ms-dos does not define a field containing some fixed value, but some
fields are much larger than the small numbers they should hold, hence there is much
redundancy, if all those fields have reasonable values, and at least some are not
zero, then it must be an ms-dos disk
Attention ! It uses the value of sectors_on_floppy which must be set to the correct
value for the disk
*/

struct bootsector {		/* from mtools, msdos.h */
/*0*/	unsigned char jump[3];		/* Jump to boot code */
/*3*/	unsigned char banner[8];	/* OEM name & version */
/*11*/	unsigned char secsiz[2];	/* Bytes per sector hopefully 512 */
/*13*/	unsigned char clsiz;		/* Cluster size in sectors */
/*14*/	unsigned char nrsvsect[2];	/* Number of reserved (boot) sectors */
/*16*/	unsigned char nfat;			/* Number of FAT tables hopefully 2 */
/*17*/	unsigned char dirents[2];	/* Number of directory slots */
/*19*/	unsigned char psect[2];		/* Total sectors on disk */
/*21*/	unsigned char descr;		/* Media descriptor=first byte of FAT */
/*22*/	unsigned char fatlen[2];	/* Sectors in FAT */
/*24*/	unsigned char nsect[2];		/* Sectors/track */
/*26*/	unsigned char nheads[2];	/* Heads */
/*28*/	unsigned char nhs[4];		/* number of hidden sectors: 2 bytes for
									normal disks, 4 bytes for >32 Mbytes  */
/*32*/  unsigned char bigsect[4];	/* big total sectors, big disks only */
/*36*/	unsigned char junk[476];	/* who cares? */
};


unsigned char buffer[512];
short i;
long n_s;
extern Boolean old_msdos;
static unsigned char offs[]= {11,12,13,15,16,18,23,25,26,27};
static unsigned char min[]=  {0, 2, 0, 0, 1, 0, 0, 0, 1, 0};
static unsigned char limit[]={0, 2, 32,0, 2, 2, 0, 0, 2, 0};

read_one_sector(0,buffer,drive_n);
if(err_code) return false;

old_msdos=false;
for(i=0;i<sizeof(offs);i++){
	if((unsigned char)buffer[offs[i]]>limit[i] ||
	   (unsigned char)buffer[offs[i]]<min[i]){
		if(i<7) return false;
		old_msdos=true;	/* those fields don't exist in disks formatted by a very 
						old version of ms-dos */
		}
	}

lowbyte(i)= buffer[19];
highbyte(i)= buffer[20];
n_s=i;

if(i==0){	/* it's 0 for disks formatted by a very old MS-DOS, but also for
			partitions >32 Mbytes in DOS 5 */
	register char *p=&buffer[36], *q=(char*)&n_s;
	*q++=*--p;		/* as usual, revert the byte order */
	*q++=*--p;
	*q++=*--p;
	*q++=*--p;
	}
else{
	if((unsigned char)buffer[13]>8) return false;	/* the check on cluster size
			had been kept less stringent for big disks... */
	}


if(buffer[24]==0 || n_s != di.sectors_on_floppy || n_s%buffer[24]!=0 )
	old_msdos=true;
		/* bisogna accettare 9 e 18 ma anche 8 (160K) e 16 (320K) e 15 (1200K) 
		e tanto vale anche un numero di settori qualunque purche' multiplo intero 
		del numero di settori per traccia (a meno che l'hardware possa fornire i 
		"veri" valori per questi parametri)
		-- I can't test the exact value of sectors per track since somebody
		could find a way to connect to the .Sony driver a disk which is not
		720 nor 1440, but 360, 1200 or even the now obsolete 180, 160 and 320K
		formats
		*/
if(!old_msdos) return true;
/* pero' in quei casi, la FAT sta comunque nel settore 1 e il primo byte della
FAT (byte 0) vale FE o FC o FD o FF a seconda della densita' (160,180,320,360)
-- in very old DOS some of the test variables were not defined, the remaining
ones are too few to guarantee good probabilities of correctly guessing, but
in those cases the FAT is in sector 1 and the first byte of FAT is
FE or FC or FD or FF according to the density (160,180,320,360)
*/
if(buffer[21]>=0xFC /* && buffer[21]<=0xFF */)
	return true;
read_one_sector(1,buffer,drive_n);
if(!err_code && buffer[0]>=0xFC /*0xF8???*/)
	return true;
return false;
}


/***********************************************************************/

short fast_macize(volume_name)
Str255 volume_name;
/*
HFS formatting routine

it was written to generate an exact copy of a floppy disk formatted
by System 7.1, without trying to understand the meaning of all the data
it generates. Only the "write count" field in sector 2 is not correctly initialized,
since I've not understood what it means and neither the Finder nor any
disk utility complains if it's initialized to 0 rather than the values
(pseudorandom?) which the system initialization routine puts in it.
Anyway, since this routine does not verify the disk looking for bad sectors,
it's up to 50 times faster than DIZero (in System 7), and it's almost as fast as
the ms-dos formatting routine
  It can create only allocation blocks of 1 sector, hence can't be used for
disks larger than 3+16+65535+2= 65556 sectors
*/
{
short catalog,extent,namelen,k;
sector_t n_s;
register char*p;
short n_sect_bitmap;

n_sect_bitmap=(di.sectors_on_floppy-6+0xFFF)>>12;	/* number of sectors of the bitmap:
	it's 1 for disks shorter than 4100 sectors (currently all cases, but the future 
	may bring us 2.88Mbyte or 20Mbyte floppies, and obviously other devices
	may already have larger values) */

fillmem(disk_buffer,0xF6,512);
scrivi_settore(di.sectors_on_floppy-1,disk_buffer);
if(err_code) return err_code;

catalog=extent=di.sectors_on_floppy>>7;
fillmem(disk_buffer,0,512);

*(short*)&disk_buffer[0]=0x4244;	/* HFS signature */
*(long*)&disk_buffer[2]=*(long*)&disk_buffer[6]= Time;	/* time of initialization */
disk_buffer[10]=1;	/* volume attributes: this set flag means "this disk is currently
	not mounted", if it's 0 on insertion the Mac thinks that the system crashed
	while using the disk */
disk_buffer[15]=3;	/* first block of volume bitmap */
*(short*)&disk_buffer[22]=512;	/* allocation block size */
disk_buffer[26]=8;		/* default clump size */
disk_buffer[29]=3+n_sect_bitmap;		/* block corresponding to bit 0 in bitmap */
disk_buffer[33]=16;		/* next unused file number */
*(short*)&disk_buffer[18]= (short)di.sectors_on_floppy-5-n_sect_bitmap;	/* total allocation blocks, not
						counting sector 0 to 2, the bitmap and the last two sectors */
*(short*)&disk_buffer[34]= di.sectors_on_floppy-5-n_sect_bitmap-catalog-extent;	/* free
											allocations blocks */
pStrcpy(&disk_buffer[36],volume_name);
*(long*)&disk_buffer[74]=*(long*)&disk_buffer[130]=(long)extent<<9;
*(long*)&disk_buffer[78]=*(long*)&disk_buffer[146]=(long)catalog<<9;
/* *(short*)&disk_buffer[134]=0; initial block of extent tree */
*(short*)&disk_buffer[136]=extent;	/* size of extent tree */
*(short*)&disk_buffer[150]=extent;		/* initial block=size of previous data structure */
*(short*)&disk_buffer[152]=catalog;
scrivi_settore(di.sectors_on_floppy-2,disk_buffer);
if(err_code) return err_code;
scrivi_settore(2,disk_buffer);
if(err_code) return err_code;

fillmem(disk_buffer,0xF6,512);
scrivi_settore(0,disk_buffer);
if(err_code) return err_code;
scrivi_settore(1,disk_buffer);
if(err_code) return err_code;

n_s=3;
fillmem(disk_buffer,0,512);
for(k=0;k<catalog+extent;k++)
	BitSet(disk_buffer,(long)k);
scrivi_settore(n_s++,disk_buffer);		/* bitmap */
if(err_code) return err_code;

if(n_sect_bitmap>1){
	fillmem(disk_buffer,0,512);
	k=n_sect_bitmap-1;
	do{
		scrivi_settore(n_s++,disk_buffer);	/* more bitmap */
		if(err_code) return err_code;
		}
	while(--k>0);
	}

disk_buffer[8]=1;
disk_buffer[11]=3;
disk_buffer[32]=2;
disk_buffer[35]=7;
*(short*)&disk_buffer[38]=extent;
*(short*)&disk_buffer[42]=extent-1;
disk_buffer[248]=0x80;
*(long*)&disk_buffer[504]=0x01F800F8;
*(long*)&disk_buffer[508]=0x0078000E;
scrivi_settore(n_s++,disk_buffer);	/* first sector of extent tree */
if(err_code) return err_code;

fillmem(disk_buffer,0,512);
while(n_s<3+n_sect_bitmap+extent){
	scrivi_settore(n_s++,disk_buffer);
	if(err_code) return err_code;
	}

disk_buffer[8]=1;
disk_buffer[11]=3;
disk_buffer[15]=disk_buffer[19]=1;
disk_buffer[23]=2;
disk_buffer[27]=disk_buffer[31]=1;
disk_buffer[32]=2;
disk_buffer[35]=37;
*(short*)&disk_buffer[38]=extent;
*(short*)&disk_buffer[42]=extent-2;
disk_buffer[248]=0xC0;
*(long*)&disk_buffer[504]=0x01F800F8;
*(long*)&disk_buffer[508]=0x0078000E;
scrivi_settore(n_s++,disk_buffer);			/* last sector of extent tree */
if(err_code) return err_code;

fillmem(disk_buffer,0,512);
*(long*)&disk_buffer[8]=0xFF010002;
namelen=volume_name[0]|1;
disk_buffer[14]=6+namelen;	/* each key entry in the catalog contains the name
							preceded by 6 bytes */
disk_buffer[19]=1;
pStrcpy(&disk_buffer[20],volume_name);
p=disk_buffer+21+namelen;
*p=1;
p+=9;
*p++=2;
*(long*)p = Time;
p+=4;
*(long*)p = Time;
p+=56;
*p=7;
p[5]=2;
p[8]=3;
p+=21;
*p++=1;
pStrcpy(p,volume_name);
*(short*)&disk_buffer[506]=0x91+namelen;
*(short*)&disk_buffer[508]=91+namelen;
disk_buffer[511]=14;
scrivi_settore(n_s++,disk_buffer);		/* first sector of catalog */
if(err_code) return err_code;

fillmem(disk_buffer,0,512);
while(n_s<3+n_sect_bitmap+extent+catalog){
	scrivi_settore(n_s++,disk_buffer);
	if(err_code) return err_code;
	}
flush_buffers(2);
return err_code;
}




/***********************************************************************/


static Handle icon_handle;
static Rect iconRect={10,16,42,48};


pascal Boolean icon_update (DialogPtr,EventRecord *,short *);
static pascal Boolean icon_update (theDialog, theEvent,itemHit)
/* update routine for the disk initialization dialog: we've tried to
copy the standard disk initialization dialog used by DIBadMount
*/
DialogPtr theDialog;
EventRecord *theEvent;
short *itemHit;
{
if(theEvent->what==keyDown &&  ((unsigned char) theEvent->message==enter_key || 
   (unsigned char) theEvent->message==CR)){
	Rect frameRect;
	short	kind;
	Handle	h;
	*itemHit=1;
	GetDItem(theDialog,1,&kind,&h,&frameRect);
	SelectButton(h);
	return true;
	}
else if(theEvent->what==updateEvt){
	if((WindowPtr)theEvent->message==theDialog){
		GrafPtr	savePort;
		Rect frameRect;
		short	kind;
		Handle	h;

		GetPort( &savePort );	
		SetPort(theDialog);
		PlotIcon(&iconRect,icon_handle);
		GetDItem(theDialog,1,&kind,&h,&frameRect);
		OutlineControl(h);

		SetPort(savePort);
		}
	else
		UpdateFilter(theEvent);
	}
else if(theEvent->what==activateEvt)
		UpdateFilter(theEvent);

return false;
}


void disk_format(make_dir)		/* disk initialization routine */
Boolean make_dir;
{
/* see Technical Notes 70 and 272 */

extern Point badmount_point;
cntrlParam 	  ParamBlock;
DialogTHndl	dHandle;
DialogPtr myDialog;
short item,err;


	/* per dischi da 720K voglio usare un formato non ottenibile dalla DIBadMount, 
	per gli altri voglio evitare la caratteristica del System 7 che accetta anche
	dischi con settori difettosi, quindi devo fare tutto da solo, compreso un dialogo 
	possibilmente simile a quello standard
	-- 720K disks can't be obtained by DIBadMount. Other formats can, but System 7
	may return a noErr code for disks with bad sectors, marking them in the allocation
	table, hence it's better to avoid DIBadMount in any case. However, the user
	will feel better if my dialog looks like the standard disk initialization dialog
	*/

invalid_buffers();
if( di.v.fi.supports_720K){
	di.v.fi.format_index=di.v.fi.supports_720K;	/* format_index contains the value for 800k, the
		maximum size, but I don't want that format */
	di.v.fi.max_format=1440;
	}

SetCursor(&arrow);
/* procurati l'icona da mettere nel dialogo*/
ParamBlock.ioRefNum=di.m_vrefnum;
ParamBlock.ioVRefNum=di.drive_number;
ParamBlock.csCode=21;        			/* Return Physical Drive Icon */
/*ParamBlock.ioCompletion=NULL; no, non serve se la chiamata è sincrona */
icon_handle=NewHandle(128);		/* la routine ritorna un Ptr, ma PlotIcon vuole 
					un handle e la Apple sconsiglia vivamente di crearsi un handle 
					senza passare per NewHandle...
					-- I get a Ptr, but PlotIcon needs an handle and Apple discourages
					to create an handle without calling NewHandle, that could crash
					the machine, hence I must create a legal handle and copy the data
					*/
if( PBControlSync(&ParamBlock) != noErr)
	fillmem(*icon_handle,0,128);
else
	mcopy(*icon_handle, *((Ptr*)&ParamBlock.csParam[0]), 128);

/* crea e posiziona il dialogo */
dHandle = (DialogTHndl)GetResource('DLOG',132);
if (dHandle) {
	char buff[8];
	HLock((Handle)dHandle);
	PositionDialog( &((**dHandle).boundsRect));
	myDialog=GetNewDialog(132,NULL,(WindowPtr)-1L);
	HUnlock ((Handle)dHandle);
	if(in_Italia){
		set_item_text(myDialog,1,di.v.fi.is_not_initialized?"\pEspelli":"\pAnnulla");
		set_item_text(myDialog,2,"\pInizializza");
		}
	else if(di.v.fi.is_not_initialized){
		set_item_text(myDialog,1,"\pEject");
		}
	my_itoa((long)(di.v.fi.max_format>>1),&buff);
	my_c2pstr(buff);
	if(di.v.fi.is_not_initialized)
		ParamText(in_Italia ? "\pDisco non inizializzato, verrà inizializzato a " :
			"\pUninitialized disk, it will be initialized at ",
			buff,"\p Kbytes",PNS);
	else
		ParamText(in_Italia ? "\pIl disco verrà inizializzato\ra " :
			"\pThis disk will be initialized\rat ", buff,"\p Kbytes",PNS);

	ShowWindow(myDialog);
	ModalDialog(icon_update,&item);

	if(item!=2){
		if(di.v.fi.is_not_initialized) diskEject(true);
		}
	else{
		short 	*IPtr;
		GrafPtr	savePort;
		static Rect rectScritta={0,54,40,300};	/* abbondare non nuoce */
		GetPort( &savePort );	
		SetPort(myDialog);
		for(item=1;item<=3;item++){
			short	kind;
			Handle	h;
			Rect	r;
			GetDItem(myDialog,item,&kind,&h,&r);
			InsetRect(&r,-4,-4);
			EraseRect(&r);
			}
		MoveTo(64,20);
		DrawString(in_Italia ? "\pInizializzazione..." :"\pInitializing...");
		PlotIcon(&iconRect,icon_handle);	/* ci fosse un update non servito...*/
		SetCursor(&waitCursor);

		{CntrlParam parblk;
		char*pt=&parblk.csParam;
		parblk.ioRefNum=di.m_vrefnum;
		parblk.ioVRefNum=di.drive_number;
		parblk.csCode=9;        			/* Track Cache Control */
		pt[0]=0;					/* disable */
		pt[1]=-1;					/* remove */
		PBControlSync(&parblk);
		}

		ParamBlock.ioRefNum=di.m_vrefnum;
		ParamBlock.ioVRefNum=di.drive_number;		/*1 for internal -- 2 for external drive*/
		ParamBlock.csCode=6;						/*Format control code*/
		IPtr= (short*)&ParamBlock.csParam;   		/*point to it; pretend it's an int */
		*IPtr=di.v.fi.format_index;						/* see TN 272 */

		FlushVol(PNS,di.drive_number);
		di.v.fi.disk_code=noMacDskErr;
		di.signature_avail=false;
		err=PBControlSync(&ParamBlock);
		if(err==controlErr)
			printf("This device does not support the standard initialization !\n");
		else
			di.sectors_on_floppy=0;

		{CntrlParam parblk;
		char*pt=&parblk.csParam;
				/* the initialization has changed the content of the disk,
				but the content of the track cache are NOT automatically
				invalidated, at least not always (it's not random, since when it
				happens it happens regularly, but I've not found any cause for 
				that) hence I disable the track cache, do a write to be sure,
				and enable it again. I do not guarantee that it's OK now, since
				when it does not happen it does not happen regularly, and I
				can't do hundreds of formatting just to verify...*/

		fillmem(disk_buffer,0xF6,512);
		write_sectors(0,disk_buffer,1);
		parblk.ioRefNum=di.m_vrefnum;
		parblk.ioVRefNum=di.drive_number;
		parblk.csCode=9;        			/* Track Cache Control */
		pt[0]=1;			/* enable */
		pt[1]=1;			/* install */
		PBControlSync(&parblk);
		}

		if(err==noErr){

			EraseRect(&rectScritta);
			MoveTo(64,20);
			DrawString(in_Italia ? "\pVerifica..." :"\pVerifying...");
			if(DIVerify(di.drive_number) == noErr){
				if(make_dir){
					EraseRect(&rectScritta);
					MoveTo(64,20);
					DrawString(in_Italia ? "\pCreazione catalogo..." :"\pCreating directory...");
					di.v.fi.disk_code=DIZero (di.drive_number,di.v.fi.max_format==1440 ? "\p720 Kbytes volume" :
						"\pUntitled");
					FlushVol (PNS,di.drive_number);	/* contro le stranezze del system 7 che spesso lo
						richiede immediatamente dopo averlo espulso */
					UnmountVol(NULL,di.drive_number);
					}
				}
			else{
				beep_in_foreground();
				printf(in_Italia ? "Verifica fallita\n":"Verification failed\n");
				diskEject(true);
				}
			}
		else{
			beep_in_foreground();
			printf(in_Italia?"Inizializzazione fallita\n":"Initialization failed\n");
			diskEject(true);
			}
		SetPort(savePort);
		SetCursor(&arrow);
		}
	DisposDialog(myDialog);
	/*update_console();*/
	DisposHandle(icon_handle);
	}
}


/************** testa stato ************/

short testa_stato(inPlace,verbose)
/* calls the device driver to get informations about the current disk,
then does some testing for useful data that the driver does not know;
informations are returned in inPlace and the global variables
di.sectors_on_floppy , di.is_not_initialized, di.os (useful for any disk),
di.max_format, di.format_index, di.supports_720K (useful only if the disk
need be initialized)
*/

short *inPlace;
Boolean verbose;
{
short i;
OSErr err;
CntrlParam parblk;
short prev_err;

*inPlace=1;

if(di.tipo_device==d_file)
	return noErr;
else if(di.tipo_device==d_scsi){
	return test_SCSI_device(inPlace,verbose);
	}
{
char sector_buffer[512];
read_one_sector(0,sector_buffer,di.drive_number);	/* well, the write protected field
	of status may be incorrect if I don't do that before. Don't ask me why */
prev_err=err_code;
i=5;
do{	/* vedi get_format sul perché faccio questo */
	err=my_DriveStatus(&parblk);
	if(err) read_one_sector(0,sector_buffer,di.drive_number);
	}
while(--i && err!=noErr);
if(err){
	if(verbose){
		start_of_line();
		printf("Device driver error %d\n",err);
		}
	return -1;
	}

*inPlace=((DrvSts*)parblk.csParam)->diskInPlace;

err=get_format(((DrvSts*)parblk.csParam),sector_buffer);
if(err!=noErr){
	if(verbose&&*inPlace) printf("Device driver error %d\n",err);
	return -1;
	}

/* discover if the disk is not initialized: there is not a device driver call
for this purpose, but that range of errors is typical for non-initialized disks */

if(prev_err<=-66&&prev_err>=-71)	/* se non è inizializzato ci prova tante
		volte, riprovare non è altro che uno spreco di tempo
		-- a read on a non-initialized disk takes a lot of time before failing,
		don't do that more than once */
	err_code=prev_err;
else
	read_one_sector(2,sector_buffer,di.drive_number);
if(	(di.v.fi.is_not_initialized = err_code<=-66 && err_code>=-71) ){
	di.os=unknown_os;
	di.v.fi.disk_code=ioErr;

	if(di.sectors_on_floppy==800)
		di.sectors_on_floppy=1600;

	if(verbose) stampa_descrizione_stato();
	return noErr;
	}
if(err_code!=noErr) read_one_sector(2,sector_buffer,di.drive_number); 
if(err_code){
	if(verbose && !(err_code==-65 && di.tipo_device==d_driver && !di.v.fi.opened_by_insertion))
		disk_error_message();
	di.v.fi.disk_code=err_code;
	di.os=unknown_os;
	return noErr;
	}
else
	*inPlace=1;	/* if the read succeeded...*/

di.os=riconosci_disco_mac(sector_buffer);
if(di.os==unknown_os){
	di.v.fi.disk_code=noMacDskErr;
	if(is_msdos_disk(di.drive_number) )
		di.os=msdos;
	}
else
	di.v.fi.disk_code=noErr;

if(verbose) stampa_descrizione_stato();
return noErr;
}
}

void stampa_descrizione_stato()
{
char buff[20];
my_itoa((long)di.sectors_on_floppy>>1,buff);
if(di.sectors_on_floppy&1) strcat(buff,in_Italia?",5":".5");

if(	di.tipo_device==d_driver&&di.v.fi.is_not_initialized){
	if(di.sectors_on_floppy==800||di.sectors_on_floppy==1600)
		printf(in_Italia?"Disco non inizializzato da 400/800/720 Kbytes\n":
			"400/800/720 Kbytes disk, not initialized\n");
	else
		printf(in_Italia?"Disco non inizializzato da %s Kbytes\n":
			"%s Kbytes disk, not initialized\n",buff);
	}
else if(di.os==msdos){
	one_empty_line();
	printf(in_Italia?"Questo è un disco MS-DOS da %sK\n":
		"MS-DOS %sK disk\n",buff);
	}
else if(di.os==unknown_os){
	one_empty_line();
	printf(in_Italia?"Disco da %sK\n":"%sK disk\n",buff);
	}
else{	/* mac_hfs o mac_mfs */
	one_empty_line();
	printf(in_Italia ? "Questo è un disco Macintosh da %sK\n" :
			"Macintosh %sK disk\n", buff);
	}
}


short is_wrprot()
{
	CntrlParam parblk;

	if(di.tipo_device==d_file) return 0;	/* !!!!! */
	if(di.tipo_device==d_scsi) return 0;	/* !!!! */

	my_DriveStatus(&parblk);
	if(!((DrvSts*)parblk.csParam)->diskInPlace) return 2;
	return (((DrvSts*)parblk.csParam)->writeProt&0x80) != 0;
}

static OSErr get_format(status,buffer)
DrvSts*status;
char*buffer;	/* only to be used as temporary storage, it does not carry
				any information, but why allocate another buffer ? */
{
/* cerca il formato del disco attualmente nel drive; ritorna noErr se l'ha trovato,
oppure il codice di errore ritornato dal device driver .Sony; poi, -1 se nessun
formato è quello del disco corrente (disco assente ?). Nella variabile
sectors_on_floppy lascia il numero di settori, valido solo per noErr; comunque
aggiorna supports_720K;
-- uses the new status calls to get informations about the current disk: it 
may return the error code given by PBStatus, or noErr if succeeded, or -1
if none of the supported disk formats is marked as being that of the current disk;
supports_720K is adjusted in any case.
*/
long tabella[12];
CntrlParam parblk;
short j;
OSErr err;

/* a volte succede che inserisco il disco quasi insieme all'emissione del
comando da menù, il disco viene espulso ma NON da parte della diskEject che libera
il drive, ma da parte del chiamante che si vede ritornare -1 da testa_stato,
perché la PBStatus ritorna offLinErr. Unica soluzione, fare una read appena
prima, e certo riprovare più volte aiuta
*/
j=5;
do{
	fillmem(&(parblk.qLink),0,sizeof(CntrlParam));
	parblk.ioRefNum=di.m_vrefnum;
	parblk.ioVRefNum=di.drive_number;
	parblk.csCode=6;		/* Return Drive Info */
	parblk.csParam[0]=6;	/* dim tabella */

	*(long*)(&(parblk.csParam[1])) = (long)&(tabella[0]);
	#ifdef PROVE
	err=-18;
	#else
	err=PBStatusSync (&parblk.qLink);
	#endif

	/*dp("err =%d(%d)\n",err,di.drive_number);*/
	if(err==offLinErr) read_one_sector(0,buffer,di.drive_number);
	}
while(err==offLinErr&&--j);
di.v.fi.supports_720K=0;

if(err == noErr){
	di.sectors_on_floppy=0;
	/*di.format_index=0;*/
	di.v.fi.max_format=-1;
	for(j=0;j<parblk.csParam[0];j++){
		if(tabella[j+j+1] & 0x40000000L)	/* bit "current disk has this format" */
			di.sectors_on_floppy=tabella[j+j];
		if(tabella[j+j]==1440) di.v.fi.supports_720K=j+1;
		if(tabella[j+j]>di.v.fi.max_format){
			di.v.fi.max_format=tabella[j+j];
			di.v.fi.format_index=j+1;
			}
		}
	/*di.sectors_on_floppy=100;	/*per le prove multivolume, mica voglio perdere tempo!
								-- for debugging multivolume formats, put few data
								on each disk, otherwise the tests will last hours ! */
	if(di.sectors_on_floppy==0) return -1;
	}
else if(err==statusErr){
	if(di.m_vrefnum== Sony_driver){
			/* non c'è una versione recente del device driver,
			ma allora non supporta i SuperDrive e i formati possibili sono questi:
			-- the device driver does not support the new status calls, but
			old drivers don't support SuperDrives and DriveStatus suffices to
			build the informations I need
			*/
		di.sectors_on_floppy= (status->twoSideFmt==-1) ? 1600 : 800;
		if(status->sides&0x80){
			di.v.fi.max_format=1600;
			di.v.fi.format_index=2;
			}
		else{
			di.v.fi.max_format=800;
			di.v.fi.format_index=1;
			}
		}
	else{
		/* the above assumptions are not generally valid for other devices,
		try to find the information from elsewhere */
		DrvQEl *q;
		di.v.fi.format_index=1;
		/*printf("Return Drive Info not suppported, trying to get the disk size otherwise\n");*/

		/* guarda prima alla Drive Queue */
		q=GetDrvQHdr()->qHead;
		while(q){
			long l;
			if(q->dQDrive==di.drive_number){
				if(q->qType==0 && q->dQDrvSz && q->dQDrvSz!=0xFFFF && q->dQDrvSz!=0xFF){
					/*printf("From Drive Queue: blocks=%u\n",q->dQDrvSz);*/
					di.sectors_on_floppy=q->dQDrvSz;
					break;
					}
				else if(q->qType==1 && 
						(loword(l)=q->dQDrvSz,hiword(l)=q->dQDrvSz2, l!=0) && l!=-1){
					/*printf("From Drive Queue: blocks=%ld\n",l);*/
					di.sectors_on_floppy=l;
					break;
					}
				}
			q=q->qLink;
			}
		if(!q){
			static long sect_finali[]={0,800,1440,1600,2880,2*2880,50000L,320000L,
				2000000L,10000000L};
			short i;
			if(di.sectors_on_floppy!=0)
				/* the following operation is expensive, hence it's performed only once
				per disk, even if testa_stato may be called in many places (typically, at the
				start of all commands). Hence, di.sectors_on_floppy is cleared by diskEject,
				disk_format and when a disk insertion is detected, and this routine 
				considers it to be valid if it's non-zero */
				return noErr;

			/*printf("Size information in Drive Queue was not valid\n");
			printf("Now searching the last sector by reading the disk\n");*/

			di.sectors_on_floppy=sect_finali[sizeof(sect_finali)/sizeof(sect_finali[0])-1];
			for(i=1;i<sizeof(sect_finali)/sizeof(sect_finali[0]);i++){
				read_one_sector(sect_finali[i],buffer,di.drive_number);
				if(err_code==sectNFErr || i>1 &&err_code){
					read_one_sector(sect_finali[i]-1,buffer,di.drive_number);
					if(err_code==noErr){
						di.sectors_on_floppy=sect_finali[i];
						break;
						}
					else{	/* the true size is NOT the value in the table, use
							a binary search to find the exact value */
						sector_t succeeds=sect_finali[i-1],
							fails=sect_finali[i]-1, s;
						while(fails-succeeds > 1){
							s=succeeds+((fails-succeeds)>>1);
							read_one_sector(s,buffer,di.drive_number);
							if(err_code==noErr)
								succeeds=s;
							else
								fails=s;
							}
						di.sectors_on_floppy=fails;
						break;
						}
					}
				else if(err_code)
					return err_code;
				}
			}

		di.v.fi.max_format=di.sectors_on_floppy;
		di.v.fi.format_index=1;
		}
	}
else
	return err;
return noErr;
}

void identifica_hardware()
/* get informations about the device drivers and the disk drives  */
{
/* i 4 LSB di parblk.csParam[1] valgono:
-- from TN272: the four LSB of parblk.csParam[1] are coded as follows:
0	no such drive
1	unspecified drive
2	400K Sony
3	800K Sony
4	SuperDrive (400K/800K GCR, 720K/1440K MFM)
5	reserved
6	reserved
7	Hard Disk 20
8-15	reserved

anyway, since in the News somebody tells that Apple is going to put out
a new floppy disk drive (maybe 2.88M, maybe a floptical 20M), suntar
accepts 5 as a SuperDrive-compatible disk: there is no reason why Apple
should not use that code nor make that drive SuperDrive-incompatible
*/
CntrlParam parblk;
short i,n_drives=0;

last_disk_operation=TickCount();
/* new for suntar 1.3.2: till now we've used the fixed number -5, and
it always worked, but it's more elegant to use the name, one never knows
what will happen on future Macs (or Mac-compatible operating systems...)
 And version 2.0 use find_driver, so any driver may be used, but
obviously the .Sony driver has some "privileges" */
Sony_driver=FindDriverByName("\p.Sony");
if(!Sony_driver) Sony_driver=-5;

di.m_vrefnum=Sony_driver;
n_superdrives=0;
last_drive=0;
for (i=1;i<=max_drive;i++){		/* ci possono essere buchi, ma comunque mai oltre l'unità 4 ! 
						-- a Macintosh can't go beyond unit 4, but there may be
						an hole since old systems assigned 3 to an external drive even 
						if unit 2 was not present
						*/
	drive_type[i-1]=0;
	parblk.ioRefNum=di.m_vrefnum;
	parblk.ioVRefNum=i;
	parblk.csCode=23;		/* Return Format List */

	err_code=PBControlSync (&(parblk.qLink));
	/*printf("%d %x %d\n",i,parblk.csParam[1],err_code);*/
	if(err_code==controlErr){	/* una versione del disk driver anteriore a quella
								che gestisce i SuperDrive
								-- an old version of the device driver: get 
								informations from DriveStatus
								*/
		err_code=my_DriveStatus(&parblk);	/* per sapere se il drive esiste */
		if(err_code==noErr && ((DrvSts*)parblk.csParam)->installed>=0){
			if(n_superdrives==0) preferred_drive=i;
			last_drive=i;
			n_drives++;
			drive_type[i-1]= (((DrvSts*)parblk.csParam)->sides&0x80) ? TWO_SIDED|0x8000 : SINGLE_SIDED|0x8000;
			}
		}
	else if(err_code==noErr){	/* OK, può essere un SuperDrive */
		short j;
		drive_type[i-1]=parblk.csParam[1]&0x7FFF;
		j= parblk.csParam[1] &0xF;
		if(j==SUPERDRIVE || j==SUPERDRIVE+1){
			preferred_drive=i;
			n_superdrives++;
			}
		if(n_superdrives==0) preferred_drive=i;
		last_drive=i;
		n_drives++;
		}
	}

if(n_superdrives>1 || n_superdrives==0&&n_drives>1)
	preferred_drive=0;


#if 0		/* no more useful, since now I do my own buffering */
if(preferred_drive){
	parblk.ioRefNum=di.m_vrefnum;
	parblk.ioVRefNum=preferred_drive;
	parblk.csCode=9;        			/* Track Cache Control */
	*(short*)&parblk.csParam =1*256+1;	/* Track cache on: pare che sia la normalità, 
		ma non si sa mai
		-- track cache on: suntar is much faster. Really, that's the default mode on
		our Mac, but we don't know about other Macs. Well, in theory one should restore
		the original situation when quitting, but there seems to be no easy way to know
		which was the current setting: "When the cache is removed, 680x0 register D0 
		contains the previous size of the cache", (TN 272) but that does NOT work !,
		in D0 it always return 0 for noError, looking at D0.L, D1.L and A0.L does not
		yield any number that could be that thing */

	PBControlSync(&parblk);
	}
#endif


#if 0

if(n_superdrives==0 && !accetta_GCR){
		ParamText(in_Italia ?
"\pNon hai un SuperDrive, quindi non potrai\rleggere dischi scritti su un sistema UNIX":
"\pYou have no SuperDrive, you can\'t read\rdisks written on an UNIX machine",
		PNS,PNS,PNS );
		my_alert();
	}
#endif

}

/******************* buffering *********************************/

/* suntar either writes or reads, and when it does neither of the two
it's better to flush the buffers anyway, since the disk might be ejected
without letting him know. Hence it's better to let the buffer empty even
in cases when the data is still good. Otherwise, this is a simple
implementation of a write-back cache, consisting of a single "cache line" with
status information for each sector.

Probably it would be faster using Async calls, but that's more difficult,
and an old Apple Technical Note told that the first versions of MultiFinder
did not let an application go to background while it had some Async calls
pending.
Really, in many cases the best thing would be to launch one independent Async
call for each sector, so the operation will start as sooner as possible,
with the highest degree of parallelism. But that's even more difficult and
might cause more risks for incompatibility; and is the Mac hardware and software
able to exploit that potential parallelism, or is everything serialized anyway ?.
*/


short floppy_buffer_size,all_buff_size;	/* it must be initialized by prefs_da_risorsa */
static short curr_buff_size;	/* the third variable
	may be different from the first one only for devices which do NOT accept
	a read for a 512 byte sector, but have larger block sizes; the second
	one is the allocated size (again, different only for big sector sizes) */

char* buffers_base=NULL;
static sector_t first_in_buffer;
static enum{empty, full, dirty, prev_error };
static char  *buffer_state;
static Boolean traccia_difettosa,eof_is_in_buffer;
static OSErr delayed_error;


sector_t get_first(len)
short*len;
{
if(len) *len=curr_buff_size;
return first_in_buffer;
}

void invalid_buffers()
{
short i;
for(i=0;i<curr_buff_size;i++)
	buffer_state[i]=empty;
first_in_buffer= -1000;
/*curr_buff_size=0;*/
traccia_difettosa=eof_is_in_buffer=false;

/*Debugger();*/
}

void invalid_after(start_sector)
sector_t start_sector;
{
short i;
for(i=0;i<curr_buff_size;i++)
	if(first_in_buffer+i >= start_sector) buffer_state[i]=empty;
}

void init_buffering()
{

ResrvMem ((Size)floppy_buffer_size);
all_buff_size=curr_buff_size=floppy_buffer_size;
buffer_state=NewPtr((Size)floppy_buffer_size);
ResrvMem ((Size)floppy_buffer_size<<9);
buffers_base=NewPtr((Size)floppy_buffer_size<<9);
check_allocated(buffers_base);
invalid_buffers();
}

void reinit_buffering(long);
void reinit_buffering(new_block_size)
long new_block_size;	/* in bytes  */
{
short wbs;

if(new_block_size<512) new_block_size=512;
new_block_size>>=9;
if(all_buff_size<(short)new_block_size){	/* obviously the buffer must keep at
			least one physical block... */
	short old=floppy_buffer_size;
	DisposPtr(buffer_state);
	DisposPtr(buffers_base);
	floppy_buffer_size=new_block_size;
	init_buffering();
	floppy_buffer_size=old;
	}
/* otherwise, never decrease the currently allocated buffer, only use as much of
it as possible */
wbs=all_buff_size - all_buff_size%(short)new_block_size;	/* approximate
	to an integer multiple of the block size... */
if(wbs!=curr_buff_size){
	curr_buff_size=wbs;
	invalid_buffers();
	}
/*dp("new buf size=%d\n",wbs);*/
}


void this_was_end_of_file(void);
void this_was_end_of_file()
/* it tells to the buffering module that the "end of archive" sector has just been
written.
In practice, it's useful only for tapes, which require a filemark, but anyway
the information is maintained also for other devices
*/
{
eof_is_in_buffer=true;
}

Boolean was_eof(void);
Boolean was_eof()
{
return eof_is_in_buffer;
}

void check_allocated(p)
void *p;
{
extern long hd_buffer_size;
void rare_options(int);
if(p==NULL){
	ParamText("\pCan\'t allocate disk buffers\rReduce their size or increase the partition",
		PNS,PNS,PNS);
	my_alert();
	if((hd_buffer_size>>9) + floppy_buffer_size >400)
		rare_options(1);
	ExitToShell();
	}
}

static void load_original_values(primo,sectors_per_block)
{
/* return with all the cache entries from primo to primo+sectors_per_block-1
either full or dirty (originally at least one was dirty and at least one was not dirty)
*/
short i;
Handle h;

i=0;
for(;;){
	if(buffer_state[primo+i]==empty)
		break;
	i++;
	if(i==sectors_per_block) return;
	}

h=NewHandle(di.v.si.block_size);
if(h==NULL) error_message("Insufficient memory for allocating a block\n");
HLock(h);
read_sectors(first_in_buffer+primo,*h,sectors_per_block);
i=primo;
while(sectors_per_block--){
	if(buffer_state[i]==empty){
		mcopy(buffers_base+((sector_t)i<<9),*h+((sector_t)(i-primo)<<9),512);
		buffer_state[i]=full;
		}
	i++;
	}
DisposHandle(h);
}


void flush_buffers(do_invalid)
short do_invalid;
/* do_invalid=0  voglio essere sicuro che i dati su disco siano OK
	ma per il resto non me ne importa niente. Però su nastro in realtà non
	voglio neanche flushare, perché possono arrivare altre richieste di
	scrittura nella stessa zona, mentre una volta che il nastro sia avanzato
	non potrei più soddisfarle
   =1 idem, e ho paura che possa avvenire un cambio di disco senza che me
   		ne accorga per cui se è passato tempo dall'ultimo accesso al device
   		invalida. Ma non per file (non si può cambiare senza farmelo sapere).
   =2 flusha e invalida comunque
*/
{
short primo=0, ultimo;
short sectors_per_block;

/*short i;for(i=0;i<curr_buff_size;i++)
	printf("%d ",buffer_state[i]);
vai_a_capo(); */

if(first_in_buffer<0){
	err_code=0;
	return;
	}

if(di.tipo_device==d_scsi){
	if(di.v.si.DeviceType==TAPE_UNIT && do_invalid<2) return;
	sectors_per_block=di.v.si.block_size>>9;
	}

while(primo<curr_buff_size){
	/* find a sequence of consecutive dirty sectors in buffer */
	while(buffer_state[primo]!=dirty && primo<curr_buff_size) primo++;

	if(primo>=curr_buff_size) break;
	ultimo=primo+1;

	if(di.tipo_device==d_scsi && di.v.si.DeviceType!=TAPE_UNIT && di.v.si.block_size!=512 &&
		primo%sectors_per_block ){
		/* sorry, the write must be in integer blocks, hence I must read the current
		contents of the block and then extract from it the 512-bytes sectors which
		were not overwritten. But that's impossible for tape units, let the operation
		proceed and cause an error. No, for Append on tapes I must go on... */
		primo -= (primo%sectors_per_block);
		load_original_values(primo,sectors_per_block);
		}
	if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT && primo>0){
		if(first_in_buffer>=0 && di.v.si.current_tape_block>=first_in_buffer &&
		   di.v.si.current_tape_block <=first_in_buffer+primo){
					/*!!!! il modo più logico è chiedere un posizionamento in avanti
					(ho perfino un'area di buffer libero delle dimensioni esatte...)
					anche perché meno si scrive e meglio è,
					ma posso anche allungare l'operazione di write */
			primo=di.v.si.current_tape_block-first_in_buffer;
			}
		}
		
	while(ultimo<curr_buff_size && buffer_state[ultimo]==dirty ) ultimo++;

	if(di.tipo_device==d_scsi && (di.v.si.block_size!=512 && ultimo%sectors_per_block ||
		di.v.si.DeviceType==TAPE_UNIT&&di.v.si.block_mode==variable_blocks)){
		if(di.v.si.DeviceType==TAPE_UNIT){
			/* can't load previous values, but this should happen if and only if the
			archive is terminated here: so, do a longer write (after clearing, the
			UNIX tar writes garbage but that's not a good way to do things) */
			while(ultimo%sectors_per_block){
				if(buffer_state[ultimo]==empty){
					fillmem(buffers_base+((sector_t)ultimo<<9),0,512);
					buffer_state[ultimo]=full;
					}
				ultimo++;
				}
			}
		else{
			ultimo -= (ultimo%sectors_per_block);
			load_original_values(ultimo,sectors_per_block);
			ultimo += sectors_per_block;
			}
		}

	/*if(di.tipo_device==d_scsi)dp("write %ld,%ld\n",(long)(first_in_buffer+primo),(long)(first_in_buffer+ultimo-1));*/
	write_sectors(first_in_buffer+primo,buffers_base+((sector_t)primo<<9),ultimo-primo);
	if(err_code && (di.tipo_device!=d_scsi||di.v.si.DeviceType!=TAPE_UNIT) ){
		/*dp("errore, riprovo\n");*/
		write_sectors(first_in_buffer+primo,buffers_base+((sector_t)primo<<9),ultimo-primo);
		}
	if(err_code){
		invalid_buffers();
		return;
		}
	if(verify_writes && di.tipo_device==d_driver){
		verify_sectors(first_in_buffer+primo,buffers_base+((sector_t)primo<<9),ultimo-primo);
		if(err_code){
			start_of_line();
			printf("Write/verify: verification failed (sector");
			if(primo==ultimo-1)
				printf(" %ld)\n",(long)(first_in_buffer+(long)primo));
			else
				printf("s %ld to %ld)\n",(long)(first_in_buffer+(long)primo),
					first_in_buffer+(long)ultimo-1);
			invalid_buffers();
			raise_error();
			}
		}
	primo=ultimo;
	}
if(do_invalid==1){
	 if(di.tipo_device==d_file)
	 	do_invalid=0;
	 else
		do_invalid= (TickCount()-last_disk_operation)>240;
	}

if(do_invalid)
	invalid_buffers();
else	/* don't forget anything... */
	for(primo=0;primo<curr_buff_size;primo++)
		if(buffer_state[primo]==dirty) buffer_state[primo]=full;
err_code=0;
}

Boolean dirty_buffers()
{
short i;
for(i=0;i<curr_buff_size;i++)
	if(buffer_state[i]==dirty) return true;
return false;
}

void leggi_settore(n_sect,buffer)
/* read one sector, buffered: it's the main entry to the disk buffering module
(leggi_settore means read_sector: a programmer is often in search of different
names for routines which do the same thing at different levels of abstraction,
and not having English as mother language at least doubles the list of
possible names: in my opinion a high level function is easier to use, hence
its name must be in my own language, a low level function is "experts only"
hence its name must be in the language of computer experts i.e. English) */
sector_t n_sect;
char *buffer;
{
char *pos_in_buffer;
short i;

/*dp("leggo settore %ld\n",n_sect);*/

if( !(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT) && n_sect>=di.sectors_on_floppy){
	err_code=n_sect==di.sectors_on_floppy?eofErr:	/* may happen for files, if there is not the cleared
						sector used as end-of-archive marker */
		sectNFErr;
	return;
	}

if(first_in_buffer>=0 && n_sect>=first_in_buffer && n_sect<first_in_buffer+curr_buff_size){	/* cache match */
	pos_in_buffer = buffers_base+((n_sect-first_in_buffer)<<9);

	if( buffer_state[n_sect-first_in_buffer]!=empty){
/*dp("match lettura %ld(%ld)\n",n_sect,first_in_buffer);*/
		if(buffer_state[n_sect-first_in_buffer]==prev_error){
			buffer_state[n_sect-first_in_buffer]=empty;
			err_code=delayed_error;
			}
		mcopy(buffer,pos_in_buffer,512);
		err_code=noErr;
		return;
		}
	if(traccia_difettosa){		/* defective sector on the track... */
		goto read_single;
		}
	if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT){
		err_code=eofErr;	/* that's the only cause which may let
							part of the buffer empty, since there is
							no sector-by-sector read on such device, and for
							variable blocks curr_buff_size is adjusted every time */
		return;
		}
	}

flush_buffers(0);

traccia_difettosa=eof_is_in_buffer=false;

if( di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT){
	if(n_sect<di.v.si.current_tape_block){	/* devo farlo PRIMA di invalid_buffers,
			tanto per non buttare via dati che possono poi servire se l'utente
			cancella l'operazione così provocando un raise_eror !!!!! */
		reposition_tape();
		}
	invalid_buffers();
	first_in_buffer=n_sect;	/* it's the tape routine which chooses the values
					for first_in_buffer and curr_buff_size... */
	i=read_scsi_tape(di.drive_number,&first_in_buffer,buffers_base,all_buff_size);
	if(err_code==ioErr || err_code==-1 ) test_tape_EOF();
	if(i){
		curr_buff_size=i;
		for(i=0;i<curr_buff_size;i++)
			buffer_state[i]=full;
		if(n_sect-first_in_buffer<curr_buff_size){
			pos_in_buffer= buffers_base+((n_sect-first_in_buffer)<<9);
			mcopy(buffer,pos_in_buffer,512);
			if(err_code!=0 && curr_buff_size<all_buff_size){
				buffer_state[curr_buff_size++]=prev_error;
				delayed_error=err_code;
				start_of_line();
				printf("Warning: a tape read error occurred at sector %ld\n\
some data were returned from previous sectors, but they might be incorrect\n",
first_in_buffer+curr_buff_size-1);
				}
			err_code=0;
			}
		else if(!err_code)
			err_code=ioErr;	/* should never happen */
		}
	else{
		/*curr_buff_size= ??? tanto vale lasciarlo al vecchio valore, anche
		perché ho già fatto un invalid_buffers fino a quella dimensione */
		if(!err_code)
			err_code=-1;
		}
	return;
	}

invalid_buffers();
first_in_buffer= n_sect - n_sect%curr_buff_size;
pos_in_buffer= buffers_base+((n_sect-first_in_buffer)<<9);

if((fase==reading_disk||fase==hack_reading) && ! listonly ||
		n_sect<=2 ){ 	/* sectors 0 and 2 are read many times,
						it's better to read all of them even during a List */
   	short n_to_read,nr;
	/*dp(" leggo multiplo %ld\n",n_sect);*/
	if(first_in_buffer+curr_buff_size<=di.sectors_on_floppy)
		n_to_read= curr_buff_size;
	else
		n_to_read=di.sectors_on_floppy-first_in_buffer;
	nr=read_sectors(first_in_buffer,buffers_base,n_to_read);

	if(! err_code || err_code==eofErr&&nr){
		short i;

		if(err_code!=eofErr)
			nr=n_to_read;

		for(i=0;i<nr;i++)
			buffer_state[i]=full;
		for(;i<curr_buff_size;i++)
			buffer_state[i]=empty;
		if(!err_code || n_sect-first_in_buffer<nr){
			mcopy(buffer,pos_in_buffer,512);
			err_code=noErr;
			}
		return;
		}
	traccia_difettosa=true;	/* c'è stato errore nel leggere una traccia intera,
			ma singoli settori possono ancora essere leggibili. Nota che di
			solito anche se la read ha dato errore può dire che alcuni settori
			sono stati letti correttamente, ma è più semplice e meno pericoloso
			non fidarsi */
	}

/*dp(" leggo singolo (list)%ld\n",n_sect);*/
read_single:
if(di.tipo_device!=d_scsi||di.v.si.block_size==512){
	short nr= read_sectors(n_sect, pos_in_buffer, 1);
	if(err_code==eofErr && nr) err_code=0;
	if(!err_code) buffer_state[n_sect-first_in_buffer]=full;
	}
else{
	short sectors_per_block=di.v.si.block_size>>9;
	sector_t start=n_sect -(n_sect%sectors_per_block);
	char *new_pos_in_buffer;
	short nr;
	new_pos_in_buffer= buffers_base+((start-first_in_buffer)<<9);
	nr=read_sectors(start, new_pos_in_buffer, sectors_per_block);
	if(err_code==eofErr && nr) err_code=0;
	if(!err_code)
		while(sectors_per_block--){
			buffer_state[start-first_in_buffer]=full;
			start++;
			}
	}
mcopy(buffer,pos_in_buffer,512);
}


void scrivi_settore(n_sect,buffer)
/* write one sector, buffered */
sector_t n_sect;
char *buffer;
{
/* visto che nel caso dei nastri il buffer viene ridimensionato da una read,
nel fare una write devo riportare il buffer alla dimensione normale.
In pratica, su nastro posso vietare del tutto accessi misti read-write:
solo per il comando "append" posso avere il buffer pieno quando faccio
una write, altrimenti in ogni caso otterrei un errore quando tento di flushare
il buffer (la testina è ormai dopo...), a meno che si tratti dei primi
blocchi (e allora basta un rewind... ma tanto vale pretendere che l'utente
lo faccia esplicitamente prima di scrivere).
quindi, SE nel fare una write vedo che c'è match controllo la pos. del
nastro per dare errore subito e non al momento del flush. Inutile testare
buffer_state per sapere se c'è qualcosa full, tanto alla fin fine quello
che conta è la posizione del nastro.
SE invece non c'è match, provvedo a ridimensionare la dim. del buffer,
perché la dim corrente può essere quella dell'ultimo blocco letto che
non è detto sia la dim giusta per scrivere

*/

/*dp("buffer contiene da %ld a %ld\n",first_in_buffer,first_in_buffer+curr_buff_size-1);*/
if(first_in_buffer<0 || n_sect<first_in_buffer || n_sect>=first_in_buffer+curr_buff_size ){
	/* cache miss */
	flush_buffers(2);
	if(err_code) return;
	if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT){
		curr_buff_size= all_buff_size - all_buff_size % (int)(di.v.si.block_size>>9);
		invalid_buffers();	/* per la nuova dimensione...*/
		first_in_buffer=n_sect;
		}
	else{
		/*invalid_buffers();già fatto dal flush_buffers(2); */
		first_in_buffer= n_sect - n_sect%curr_buff_size;
		}
	}
mcopy(buffers_base+((n_sect-first_in_buffer)<<9),buffer,512);
buffer_state[n_sect-first_in_buffer]=dirty;
err_code=noErr;
eof_is_in_buffer=false;	/* comunque lo scrivere qualcos'altro lo rende falso,
						a meno che si scriva un altro end of file che però
						richiama di nuovo l'apposita routine... */

if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT){
	if(first_in_buffer!=di.v.si.current_tape_block){
		/* the simplest way to let the error messages appear is to call
		write_scsi_tape now, it does not perform any writing if the
		position is incorrect */
		write_scsi_tape(di.drive_number,first_in_buffer,buffers_base,curr_buff_size);
		}
	}
return;
}


void store_disk_signature()
{
/* well, sector 0 is not always very good as a disk signature (it often
contains all 0's or all F6) but I must recognize tar and bar archives, which
typically have an header in it */

if(di.tipo_device!=d_driver||!di.v.fi.opened_by_insertion) return;
read_one_sector(0,first_sector,di.drive_number);
di.signature_avail= !err_code;
last_disk_operation=TickCount();
}

Boolean not_my_disk()
{
if(di.signature_avail!=1 )
	return false;
else{
	char buffer[512];
	read_one_sector(0,buffer,di.drive_number);
	if(err_code) return false;

	return !compare_mem(buffer,first_sector,512);
	}
}

static Boolean check_changed_disk()

/* checks whether the user has accidentally replaced the disk with
another one: it happens rather often (even to me, for a multivolume
archive storing more data than the free space on the disk, I extract
some files, eject the disk (during a pause or a "confirm saves" dialog),
examine the files, throw away some AND save the others to a floppy disk.
When I return to suntar, I want to continue extraction and I may forget
to replace the disk. And if that's bad for reading, that's a disaster
when writing a tar archive ! */
{
long temp= TickCount()-last_disk_operation;	/* better to have a variable, so that
		an overflow of Ticks does not affect the result */
if(temp>10 * 60 && not_my_disk() ){		/* 10 seconds from the last disk operation
			(i.e. the last call to this routine) is a reasonable compromise: it does
			not affect speed, and it's difficult to pause suntar, eject the
			disk, insert a different disk, use it, return to suntar AND not
			remember that it needs another disk, all in less than 10 seconds ! */
	diskEject(di.tipo_device==d_driver && di.v.fi.opened_by_insertion==1);
	ParamText(in_Italia?"\pDisco sbagliato !":"\pWrong disk !",PNS,PNS,PNS);
	my_alert();
	return true;
	}
return false;
}

static OSErr my_DriveStatus(parblk)
CntrlParam *parblk;
/* can't use DriveStatus because it's hardwired to vrefnum -5 */
{
OSErr err;

    parblk->ioRefNum=di.m_vrefnum;
	parblk->ioVRefNum=di.drive_number;
	parblk->csCode=8 /*drvStsCode*/;
	((DrvSts*)parblk->csParam)->diskInPlace=1;	/* just in case the driver does not
												write anything on this field (e.g. AppDisk) */
	((DrvSts*)parblk->csParam)->writeProt= 0;	/* idem */
	err=PBStatusSync (parblk);
	#ifdef PROVE
err=-18;
	#endif
	if(err==statusErr){
		DrvQEl *q;
		/*printf("Drive Status not supported, trying to get informations from the Drive Queue\n");*/
		q=GetDrvQHdr()->qHead;
		while(q){
			if(q->dQDrive==di.drive_number){
				di.m_vrefnum=q->dQRefNum;
				break;
				}
			q=q->qLink;
			}
		if(!q) return err;

		((DrvSts*)parblk->csParam)->diskInPlace= *(((char*)q)-3)==1 || *(((char*)q)-3)==2 || *(((char*)q)-3)==8;
		((DrvSts*)parblk->csParam)->writeProt= *(((char*)q)-4);
		((DrvSts*)parblk->csParam)->twoSideFmt=-1;
		((DrvSts*)parblk->csParam)->sides=0x80;
		err=noErr;
		}

	return err;
}


OSErr find_driver(void);

OSErr find_driver()
{
DrvQEl *q;
q=GetDrvQHdr()->qHead;
while(q){
	if(q->dQDrive==di.drive_number){
		di.m_vrefnum=q->dQRefNum;
		/*printf("detected a disk insertion in drive %d handled by driver %d\n",di.drive_number,Sony_driver);*/
		return noErr;
		}
	q=q->qLink;
	}
/*printf("detected a disk insertion in drive %d Error: unknown driver\n",di.drive_number);*/

return -1;
}

OSErr ID_to_drive_number(short,short*);
OSErr ID_to_drive_number(scsiID,ris)
short scsiID,*ris;
{
DrvQEl *q;
q=GetDrvQHdr()->qHead;
while(q){
	if(~(scsiID+32)==q->dQRefNum){
		*ris=q->dQDrive;
		return noErr;
		}
	q=q->qLink;
	}
return -1;
}

Boolean volume_in_place(d_refnum)
short d_refnum;
{
DrvQEl *q;
q=GetDrvQHdr()->qHead;
while(q){
	if(d_refnum==q->dQRefNum){
		if( *(((char*)q)-3)==1 || *(((char*)q)-3)==2 ) return true;
		}
	q=q->qLink;
	}
return false;
}


#ifdef THINK_C_5
	/*#define numEntriesInTable (*( (short*)UnitNtryCnt ))*/
	/* UnitNtryCnt is a constant in sysEqu.h ? NO, it's a variable in LoMem.h,
	but MacHeaders.c may be compiled to include either one or the other... */
	#define numEntriesInTable UnitNtryCnt
#else
	#define numEntriesInTable UnitNtryCnt
	/* UnitNtryCnt is a variable in DeviceMgr.h */
#endif
/*#else
#define numEntriesInTable (*( (short*)0x1D2 ))
#endif*/
#define UnitToRef(unitNo)		( -(unitNo) - 1 )

#if 0
static void prova_chiamate(ref_altro)
short ref_altro;
{
	short err,j;
long tabella[12];
CntrlParam parblk;
	printf("Now testing if driver %d supports the required services\n",ref_altro);
	one_empty_line();

    parblk.ioRefNum=ref_altro;
	parblk.ioVRefNum=di.drive_number;
	parblk.csCode=8 /*drvStsCode*/;

	err=PBStatusSync (&parblk.qLink);

	if(err==0){
		printf("DriveStatus is supported\n");
		}
	else
		printf("DriveStatus is not supported (error code =%d)\n",err);

j=5;
do{
	fillmem(&(parblk.qLink),0,sizeof(CntrlParam));
	parblk.ioRefNum=ref_altro;
	parblk.ioVRefNum=di.drive_number;
	parblk.csCode=6;
	parblk.csParam[0]=6;	/* dim tabella */

	*(long*)(&(parblk.csParam[1])) = (long)&(tabella[0]);
	err=PBStatusSync (&parblk.qLink);

	/*dp("err =%d(%d)\n",err,di.drive_number);*/
	if(err==offLinErr) read_one_sector(0,disk_buffer,di.drive_number);
	}
while(err==offLinErr&&--j);

	if(err==0)
		printf("Return Drive Info is supported\n");
	else if(err==offLinErr)
		printf("Return Drive Info could not be tested (no disk in drive)\n");
	else
		printf("Return Drive Info is not supported (error code =%d)\n",err);



	parblk.ioRefNum=ref_altro;
	parblk.ioVRefNum=di.drive_number;
	parblk.csCode=23;

	err=PBControlSync (&(parblk.qLink));

	if(err==0)
		printf("Return Format List is supported\n");
	else
		printf("Return Format List is not supported (error code =%d)\n",err);

one_empty_line();

}
#endif

void stampa_driver_info()
{
short unitNumber;

one_empty_line();
printf("Informations from the Unit Table (device drivers):\n");
one_empty_line();
/*printf("The table has %d entries\n",numEntriesInTable);*/
for ( unitNumber=0;unitNumber<(numEntriesInTable&0xFFF); unitNumber++ ){
	Str255 buffer;
	static char *tabm[]={"RAM","RAM nonrelocatable","ROM"};
	short i;

	i=get_driver_name(UnitToRef(unitNumber),buffer);
	if(i)
		printf("driver nr. %3d is %P  (%s)\n",UnitToRef(unitNumber),buffer,tabm[i-1]);
	}
}

void stampa_volume_info()
{
DrvQEl *q;
char c;
one_empty_line();
printf("Informations from the Drive Queue (volumes):\n\n");
q=GetDrvQHdr()->qHead;
while(q){
	start_of_line();
	printf("drive %2d driver %3d ",q->dQDrive,q->dQRefNum);
	{Str255 buffer;
	if(get_driver_name(q->dQRefNum,buffer)) printf("(%P)",buffer);
	}
	if(q->dQRefNum>=-39 && q->dQRefNum<=-33)
		printf(" ID=%d",~(32+q->dQRefNum));	

	if(q->dQRefNum==Sony_driver){
		short i=q->dQDrive-1;
		printf("  Standard floppy disk drive\n   ");
		if(drive_type[i]){
			switch(drive_type[i]&0xF){
			case 7:
				printf("Hard disk 20 ");
				break;
			case 2:
				printf("Single sided (GCR 400k) ");
				break;
			case 3:
				printf("Double sided (GCR 800k) ");
				break;
			case 4:
				printf("SuperDrive (GCR 800k-MFM 720/1440k) ");
				break;
			case 5:
				{Handle h=GetResource('STR ',200);
				if(h){
					HLock(h);
					printf("%P ",*h);
					HUnlock(h);
					break;
					}
				}
				/* else unknown type */
			default:
				printf("Unknown drive type ");
			}
		}
	else{
		CntrlParam parblk;
		parblk.ioRefNum=q->dQRefNum;
		parblk.ioVRefNum=q->dQDrive;
		parblk.csCode=23;		/* Return Format List */

		if(PBControlSync (&(parblk.qLink))==noErr){
			printf((parblk.csParam[1]&0x800) ?" primary":" secondary");
			/*printf((parblk.csParam[1]&0x400) ?" fixed":" removable"); doppione*/
			printf((parblk.csParam[1]&0x100) ?" external":" internal");
			if(parblk.csParam[1]&0x200) printf(" (SCSI)");
			}
		}
	}

	/* printf(" FS= %x",q->dQFSID); */


	{HParamBlockRec param;
	short err;
	unsigned char buffer[64];
	fillmem(&param,0,sizeof(param));
	/*param.ioVolIndex=*/
	param.volumeParam.ioVRefNum=q->dQDrive;
	param.volumeParam.ioNamePtr=buffer;
	
	err=PBHGetVInfo (&param,false);
	/*printf("err= %d FSID=%x ",err,param.volumeParam.ioVFSID);*/
	c= *(((char*)q)-3);
	if(q->dQRefNum==Sony_driver)	/* inutile dire che è ejectable */
		printf( (c==1||c==2) ?
			(err?" present":"") :
			" not present");
	else
		printf(" %s",(c==8||c==0x48) ? "fixed disk" : (c==1||c==2) ?
			(err?"ejectable disk, present":"ejectable disk") : /* inutile dire present se è mounted */
			"ejectable disk, not present");
	if(!err) printf(" mounted as \"%P\"",buffer);
	}

	if(q->qType==0){
		if(c==8||c==0x48||(c==1||c==2)&&q->dQRefNum!=Sony_driver || (q->dQDrvSz&255)!=255) /* non stampare un valore privo di significato */
			if(q->dQDrvSz) printf("; blocks=%u",q->dQDrvSz);
		}
	else if(q->qType==1){
		long l;
		loword(l)=q->dQDrvSz;
		hiword(l)=q->dQDrvSz2;
		if(l) printf("; blocks=%ld",l);
		}
	else
		printf(" unknown qType");
	if(c==1||c==2){
		c= *(((char*)q)-4);
		if(c&0x80) printf("; locked");
		/* c= *(((char*)q)-1);
		if(c&0x80) printf("; single sided"); no, l'informazione è spesso
		scorretta per .Sony, e ha poco senso stamparla per gli altri */
		}
#if 0
	c= *(((char*)q)-3);
	if(c!=8 && c!=0x48 /*&& q->dQRefNum!=-5*/){
		short old=di.drive_number;
		di.drive_number=q->dQDrive;
		one_empty_line();
		prova_chiamate(q->dQRefNum);
		di.drive_number=old;
		}
#endif
	q=q->qLink;
	}
one_empty_line();
}



/************** assembly language *******************************/
/*
#undef Sony_driver
#define Sony_driver #-5
*/

#define ASM
/*#undef ioRefNum*/
#define ioDrvNum ioVRefNum

/* for suntar 1.0 we copied the examples in IM vol. II, which were in
assembly language. Only for version 1.3.2 we wrote the equivalents in C.
 That's the only reason to choose assembly rather than C */


/* from Inside Macintosh volume II page 216 */



void diskEject(chiudi)
Boolean chiudi;
{
if(di.tipo_device!=d_driver || di.drive_number==0) return;

ultimo_disco_espulso=true;
disco_espulso=true;
flush_buffers(2);
di.sectors_on_floppy=0;

#ifdef ASM
	asm{
		moveq	#31,D0  /* #<ioVQElSize/2>-1,D0 */
clearloop:
		clr.w	-(SP)
		dbra	D0,@clearloop
		move.l	SP,A0
		move.w	di.m_vrefnum,0x18(A0)	/* #-5,ioRefNum(A0) */
		move.w	di.drive_number,0x16(A0)		/* di.drive_number,ioDrvNum(A0) */
		move.w	#7,0x1A(A0)		/* #ejectCode,csCode(A0) */
		dc.w 0xA017				/* _Eject */
		add.w	#0x40,SP		/* #ioVQElSize,SP */
	}
#else
	#if 0
	Eject(NULL,di.drive_number);	/* funziona perché un drive_number è accettato
			anche dove ci vorrebbe un vrefnum, ma probabilmente è meglio fare
			invece una PBControl
			-- that's an high level call, and that means it may have some unknown
			side effects, it's better to call the low-level operation */
	#else
	{
	CntrlParam parblk;
	parblk.ioRefNum=di.m_vrefnum;
	parblk.ioVRefNum=di.drive_number;
	parblk.csCode=7;        			/* Eject */
	PBControlSync(&parblk);
	}
	#endif
#endif

if(chiudi){
	di.drive_number=0;
	di.tipo_device=d_notopen;
	aggiorna_nome_aperto();

	if(di.v.fi.opened_by_insertion!=1){
		DeleteMenu(deviceID+2);	/* !!! è piuttosto sporco metterlo qui */
		DrawMenuBar();
		}
	}
di.signature_avail=false; /* inutile, lo testo solo se drive_number!=0 */
}

/**********************************/
short read_sectors(sect_n,buffer,n_sectors)
/* if err_code==eofErr, and only in that case, it returns the number of sectors which
were successfully read */
short n_sectors;

sector_t sect_n;
char *buffer;
{
long count;

/*printf("leggo %d settori\n",n_sectors);*/

if(di.tipo_device==d_file){
	/* if working from a file rather than a disk... */

/* sectors_on_floppy is used to store the file length so that it's never
called with a request for more than 511 bytes after EOF, in that case it
clears those extra bytes and returns noErr. That's not a very good way to
handle EOF, but suntar was written for disks, and in disks End Of Medium
is never placed inside a sector !
*/

	err_code=0;
	if(sect_n!=di.v.file_current_s)
		err_code=SetFPos(di.m_vrefnum,fsFromStart,(long)sect_n<<9);
	if(err_code==eofErr)
		err_code=sectNFErr;		/* sector not found error */
	else if(!err_code){
		count=(long)n_sectors<<9;
		err_code=FSRead(di.m_vrefnum,&count,buffer);
		if(!err_code && !count) err_code=eofErr;
		if(err_code==eofErr && count!=0){
			fillmem(&buffer[count],0,(n_sectors<<9)-(short)count);
			/*err_code=noErr; no, the caller must know that eofErr does not mean
			that no data is available */
			di.v.file_current_s=-1;
			return (count+511)>>9;
			}
		}
	if(err_code)
			di.v.file_current_s=-1;
	else
			di.v.file_current_s=sect_n+n_sectors;
	return 0;
	}
else if(di.tipo_device==d_scsi){
	if(di.v.si.DeviceType==TAPE_UNIT){
		/* per la verità non dovrei mai arrivare qui essendo intercettato
		da leggi_settore */
		sector_t n=sect_n;
		short i;
		i= read_scsi_tape(di.drive_number,&n,buffer,n_sectors);
		if(err_code==ioErr || err_code==-1 ) test_tape_EOF();
		if(n!=sect_n || n_sectors!=i)
			error_message("Invalid request for tape unit\n");
		return 0;
		}
	else{
		last_disk_operation=TickCount();
		return read_scsi_random(di.drive_number,sect_n,buffer,n_sectors);
		}
	}

/* else, it's a floppy disk operation, made through the device driver */

if(check_changed_disk()){
	err_code= -65;	/* missing disk */
	return 0;
	}

#ifdef ASM
asm{
	moveq	#24,D0 			/* #<ioQElsize/2>-1,D0 */
clrloop:	
	clr.w	-(SP)
	dbra	D0,@clrloop
	move.l	SP,A0
	move.w	di.m_vrefnum,0x18(A0)	/* #-5,ioRefNum(A0) */
	move.w	di.drive_number,0x16(A0)		/* di.drive_number,ioDrvNum(A0) */
	move.w	#1,0x2C(A0)		/* #1,ioPosMode(A0) absolute positioning */
#if SECTOR_T_SIZE==4
	move.l	sect_n,D0
	lsl.l	#4,D0			/* 9 is too big... */
	lsl.l	#5,D0
#else
	move.w	sect_n,D0
	mulu	#512,D0			/* does both short->long conversion and shift */
#endif
	move.l	D0,0x2E(A0)		/* D0,ioPosOffset(A0) */

	move.w	n_sectors,D0
	mulu	#512,D0
	move.l	D0,0x24(A0)		/* D0,ioReqCount(A0)	/* read n_sectors sectors */
	move.l	buffer,A1
	move.l	A1,0x20(A0)		/* A1,ioBuffer(A0) */
	dc.w 0xA002				/* _Read */
	move.w	0x10(A0),err_code	/* ioResult(A0),err_code */
	/*move.l  0x28(A0),count*/		/* ioActCount(A0),count) */
	add.w	#0x32,SP		/* #ioQElSize,SP */
	}
#else
{
ParamBlockRec pb;
fillmem(&pb,0,sizeof(pb));
pb.ioParam.ioRefNum=di.m_vrefnum;
pb.ioParam.ioDrvNum=di.drive_number;
pb.ioParam.ioPosMode=fsFromStart;
pb.ioParam.ioPosOffset=((long)sect_n)<<9;
pb.ioParam.ioReqCount=((long)n_sectors)<<9;
pb.ioParam.ioBuffer=buffer;

PBReadSync(&pb);

err_code=pb.ioParam.ioResult;
/* count=pb.ioParam.ioActCount; */
}
#endif
if(!err_code) last_disk_operation=TickCount();

return 0;	/* for a device, sectors_on_floppy is the correct value, hence
			read_sectors never tries to read after the EOF, so it's useless
			to examine the contents of ioActCount (except for a disk with a bad
			sector, but probably it isn't worth of it) */
}

void read_one_sector(sect_n,buffer,drive_n)
/* it reads from the disk even when read_sectors is redirected to read from a file or
SCSI device... (but anyway it uses di.m_vrefnum, which MUST have been assigned
correctly, typically by find_driver) */
sector_t sect_n;
char *buffer;
short drive_n;
{

#ifdef ASM
asm{
	moveq	#24,D0 			/* #<ioQElsize/2>-1,D0 */
clrloop:
	clr.w	-(SP)
	dbra	D0,@clrloop
	move.l	SP,A0
	move.w	di.m_vrefnum,0x18(A0)	/* #-5,ioRefNum(A0) */
	move.w	drive_n,0x16(A0)		/* drive_n,ioDrvNum(A0) */
	move.w	#1,0x2C(A0)		/* #1,ioPosMode(A0) absolute positioning */
#if SECTOR_T_SIZE==4
	move.l	sect_n,D0
	lsl.l	#4,D0			/* 9 is too big... */
	lsl.l	#5,D0
#else
	move.w	sect_n,D0
	mulu	#512,D0			/* does both short->long conversion and shift */
#endif
	move.l	D0,0x2E(A0)		/* D0,ioPosOffset(A0) */

	move.l	#512,0x24(A0)	/* #512,ioReqCount(A0) */
	move.l	buffer,A1
	move.l	A1,0x20(A0)		/* A1,ioBuffer(A0) */
	dc.w 0xA002				/* _Read */
	move.w	0x10(A0),err_code	/* ioResult(A0),err_code */
	add.w	#0x32,SP		/* #ioQElSize,SP */

	}
#else
ParamBlockRec pb;
fillmem(&pb,0,sizeof(pb));
pb.ioParam.ioRefNum=di.m_vrefnum;
pb.ioParam.ioDrvNum=drive_n;
pb.ioParam.ioPosMode=fsFromStart;
pb.ioParam.ioPosOffset=((long)sect_n)<<9;
pb.ioParam.ioReqCount=512;
pb.ioParam.ioBuffer=buffer;

PBReadSync(&pb);

err_code=pb.ioParam.ioResult;
#endif
}


/******************************/

void write_sectors(sect_n,buffer,n_sectors)
sector_t sect_n;
char *buffer;
short n_sectors;
{
/*printf("scrivo %d settori\n",n_sectors);*/
if(di.tipo_device==d_file){
	long count;
	/* if working from a file rather than a disk... */
	err_code=0;
	if(sect_n!=di.v.file_current_s)
		err_code=SetFPos(di.m_vrefnum,fsFromStart,(long)sect_n<<9);
	if(err_code==eofErr)
		err_code=sectNFErr;		/* sector not found error */
	else if(!err_code){
		count=(long)n_sectors<<9;
		err_code=FSWrite(di.m_vrefnum,&count,buffer);
		}
	if(err_code)
		di.v.file_current_s=-1;
	else{
		di.v.file_current_s=sect_n+n_sectors;
		if(di.v.file_current_s>di.sectors_on_floppy)
			di.sectors_on_floppy=di.v.file_current_s;
		}
	return;
	}
else if(di.tipo_device==d_scsi){
	if(di.v.si.DeviceType==TAPE_UNIT)
		write_scsi_tape(di.drive_number,sect_n,buffer,n_sectors);
	else{
		/* anyway, the user must pass a number of alert dialogs before allowing
		the program to write to a hard disk */
		last_disk_operation=TickCount();
		write_scsi_random(di.drive_number,sect_n,buffer,n_sectors);
		}
	return;
	}

if(check_changed_disk()){
	err_code= -65;	/* missing disk */
	return;
	}

#ifdef ASM
asm{
	moveq	#24,D0 			/* #<ioQElsize/2>-1,D0 */
clrloop:
	clr.w	-(SP)
	dbra D0,@clrloop
	move.l	SP,A0
	move.w	di.m_vrefnum,0x18(A0)	/* #-5,ioRefNum(A0) */
	move.w	di.drive_number,0x16(A0)		/* di.drive_number,ioDrvNum(A0) */
	move.w	#1,0x2C(A0)		/* #1,ioPosMode(A0) absolute positioning */
#if SECTOR_T_SIZE==4
	move.l	sect_n,D0
	lsl.l	#4,D0
	lsl.l	#5,D0
#else
	move.w	sect_n,D0
	mulu	#512,D0
#endif
	move.l	D0,0x2E(A0)		/* D0,ioPosOffset(A0) */

	move.w	n_sectors,D0
	mulu	#512,D0
	move.l	D0,0x24(A0)		/* D0,ioReqCount(A0)	/* write n_sectors sectors */
	move.l	buffer,A1
	move.l	A1,0x20(A0)		/* A1,ioBuffer(A0) */
	dc.w 0xA003				/* _Write */
	move.w	0x10(A0),err_code	/* ioResult(A0),err_code */
	add.w	#0x32,SP		/* #ioQElSize,SP */
	}
#else
ParamBlockRec pb;
fillmem(&pb,0,sizeof(pb));
pb.ioParam.ioRefNum=di.m_vrefnum;
pb.ioParam.ioDrvNum=di.drive_number;
pb.ioParam.ioPosMode=fsFromStart;
pb.ioParam.ioPosOffset=((long)sect_n)<<9;
pb.ioParam.ioReqCount=((long)n_sectors)<<9;
pb.ioParam.ioBuffer=buffer;

PBWriteSync(&pb);

err_code=pb.ioParam.ioResult;

#endif

if(sect_n==0){
	di.signature_avail= !err_code;
	mcopy(first_sector,buffer,512);
	}
if(!err_code) last_disk_operation=TickCount();
}


static void verify_sectors(sect_n,buffer,n_sectors)
sector_t sect_n;
char *buffer;
short n_sectors;
{

#ifdef ASM
asm{
	moveq	#24,D0 			/* #<ioQElsize/2>-1,D0 */
clrloop:	
	clr.w	-(SP)
	dbra	D0,@clrloop
	move.l	SP,A0
	move.w	di.m_vrefnum,0x18(A0)	/* #-5,ioRefNum(A0) */
	move.w	di.drive_number,0x16(A0)		/* di.drive_number,ioDrvNum(A0) */
	move.w	#65,0x2C(A0)		/* #1+64,ioPosMode(A0) absolute positioning &
								read/verify */
#if SECTOR_T_SIZE==4
	move.l	sect_n,D0
	lsl.l	#4,D0			/* 9 is too big... */
	lsl.l	#5,D0
#else
	move.w	sect_n,D0
	mulu	#512,D0			/* does both short->long conversion and shift */
#endif
	move.l	D0,0x2E(A0)		/* D0,ioPosOffset(A0) */

	move.w	n_sectors,D0
	mulu	#512,D0
	move.l	D0,0x24(A0)		/* D0,ioReqCount(A0)	/* read n_sectors sectors */
	move.l	buffer,A1
	move.l	A1,0x20(A0)		/* A1,ioBuffer(A0) */
	dc.w 0xA002				/* _Read */
	move.w	0x10(A0),err_code	/* ioResult(A0),err_code */
	add.w	#0x32,SP		/* #ioQElSize,SP */

	}
#else
ParamBlockRec pb;
fillmem(&pb,0,sizeof(pb));
pb.ioParam.ioRefNum=di.m_vrefnum;
pb.ioParam.ioDrvNum=di.drive_number;
pb.ioParam.ioPosMode=fsFromStart+rdVerify;	/* absolute positioning & read/verify */
pb.ioParam.ioPosOffset=((long)sect_n)<<9;
pb.ioParam.ioReqCount=((long)n_sectors)<<9;
pb.ioParam.ioBuffer=buffer;

PBReadSync(&pb);

err_code=pb.ioParam.ioResult;
#endif
if(!err_code) last_disk_operation=TickCount();
}


/****************************************************************/

Boolean pStrCmpCi(unsigned char*,unsigned char*);
Boolean pStrCmpCi(s1,s2)
register unsigned char*s1,*s2;
/* a case insensitive Pascal string comparison, required by the following
routine (it's a matter of style, the original code used EqualString, and since
that may allocate memory it had to Lock the handles: in my own programming
style I prefer to write this small routine */
{
register short l=*s1++;
if(*s2++ != l) return false;	/* different length */
while(l--){
	register char c1=*s1++, c2=*s2++;
	if(c1 != c2){	/* they may still differ only in case... */
		if(c1>='A'&&c1<='Z') c1+= 'a'-'A';
		if(c2>='A'&&c2<='Z') c2+= 'a'-'A';
		if(c1 != c2) return false;
		}
	}
return true;
}


/********************

 From the snippet file "Floppy II", © Developers Technical Support, Apple

**********************/

#define kHandleBased		0x0040

typedef struct
{
	short		drvrFlags;		/* dReadEnable, etc */
	short		drvrDelay;		/* for periodic actions via SystemTask */
	short		drvrEMask;		/* for desk acc only */
	short		drvrMenu;		/* for desk acc only */
	short		drvrOpen;		/* offset to open routine */
	short		drvrPrime;		/* offset to prime routine */
	short		drvrCtl;		/* offset to control routine */
	short		drvrStatus;		/* offset to status routine */
	short		drvrClose;		/* offset to close routine */
	unsigned char drvrName[];	/* Pascal string */
} DriverHeader;


short FindDriverByName( void *pString )
{
	short			refNo;
	DCtlHandle		driverInfo;
	DriverHeader	*driverPtr, **driverHandle;
	short			unitNumber;

	/* to find .Sony, one must search backwards: for historical reasons
	there are two .Sony drivers, -5 and -2, and -5 is the one normally
	used for floppy disks (-2 was used by Apple HD20, a product no more
	sold since about 1985). That means skipping a lot of empty entries,
	but this routine is called only once */
	for ( unitNumber=numEntriesInTable-1; unitNumber; unitNumber-- ) {
		refNo = UnitToRef(unitNumber);
		driverInfo = GetDCtlEntry(refNo);
		if (driverInfo) {
			if ((**driverInfo).dCtlFlags & kHandleBased ) {
				driverHandle = (void *)(**driverInfo).dCtlDriver;
				if (!driverHandle || !(*driverHandle))
					continue;

				if (pStrCmpCi(pString, (**driverHandle).drvrName))
					return refNo;
				/*pStrcpy(disk_buffer,(**driverHandle).drvrName);
				printf("%P\n",disk_buffer);*/
				}
			else {
				driverPtr = (void*)(**driverInfo).dCtlDriver;
				if ( !driverPtr ) 
					continue;

				if ( pStrCmpCi( pString, driverPtr->drvrName ) )
					return refNo;
				/*printf("%P\n",driverPtr->drvrName);*/
				}
			}
		}
	return 0;
}

#define ROM_Base 0x2AE


short get_driver_name(refNo,buffer)
short			refNo;
unsigned char *buffer;
{
	DCtlHandle		driverInfo;
	DriverHeader	*driverPtr, **driverHandle;
	driverInfo = GetDCtlEntry(refNo);
	if (driverInfo) {
		if ((**driverInfo).dCtlFlags & kHandleBased ) {
			driverHandle = (void *)(**driverInfo).dCtlDriver;
			if (!driverHandle || !(*driverHandle))
				return 0;
			pStrcpy(buffer,(**driverHandle).drvrName);
			if(buffer[1]=='.')
				return 1;
			}
		else {
			driverPtr = (void*)(**driverInfo).dCtlDriver;
			if ( !driverPtr ) 
				return 0;
			if(driverPtr->drvrName[1]=='.'){
				pStrcpy(buffer,driverPtr->drvrName);
				return (long)driverPtr<*(long*)ROM_Base ? 2:3;
				}
			}
		}
	return 0;
}


