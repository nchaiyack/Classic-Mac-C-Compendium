/* Copyright 1988, Gail Zacharias.  All rights reserved.
 * Permission is hereby granted to copy, reproduce, redistribute or
 * otherwise use this software provided there is no monetary profit
 * gained specifically from its use or reproduction, it is not sold
 * rented, traded or otherwise marketed, and this copyright notice
 * and the software version number is included prominently in any copy
 * made.
 * This was mtar version 1.0.
 *
 */

/* the original mtar program was then altered to become part of suntar:
*/

/*******************************************************************************\

tar writing module

part of suntar, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/


/*
comments in the text explain which routines were added for suntar by us and
which ones are from mtar, but most of the latter were modified more or less 
heavily by us. In fact, the file is divided into three sections:
a few declarations by Gail Zacharias
a series of routines 100% by Speranza
a series of old routines by Zacharias: only a few of them are still in the
	original form, but the calling interface is usually unchanged
*/

#include "system7.h"
#include "PB_sync.h"
#include "antiglue.h"

#include <string.h>
/*#include <FileMgr.h>
#include <OSUtil.h>
#include <HFS.h>*/

#include "suntar.h"

#include "windows.h"

#define TAPE_UNIT 1

#if 0 /* un-optimized version */
void fillmem(dest, ch, len)
register char*dest;
register char ch;
register short len;
{
	while (--len>=0) *dest++ = ch;
}
#endif

#define tarsz(n)	(((n)+511L) & -512L)
#define macbinsz(n)	(((n)+127L) & -128L)

static HFileInfo statb;

static Boolean binary = false, 
	dataonly = false;
short conversion_mode;
extern long unixtime;

#define	dirmode 0777
#define	filemode 0666

char *uname,
     *gname;

/* Unfortunately, the Mac keeps only local time, so this will be off by
   the time zone... */
#define UNIXTIME	2082844800L  /* Jan 1, 1970 00:00:00 */

/*
main (argc, argv)
  char **argv;
{
  fprintf(stderr, 
"This is freeware.  If you paid money for this program, you got ripped off.\n");
}
*/
/*********************** start of routines by Speranza **************************/

/* dovrò riorganizzare un po' le cose: ha senso che, come in untar, ci siano due
modi di chiamare le routine di output: uno per quelli che sanno che esistono i settori
(scrittura header) e uno per quelli che preferiscono vedere un flusso continuo di bytes:
ovviamente ci vuole una routine per riallinearsi al settore dopo una serie di
chiamate del secondo gruppo, ma questa esiste già, svuota_buffer
-- Beware: the disk handling routines are organized with an external interface
which looks like a continuous stream of bytes (they trasparently handle volume
headers and disk swapping) but in several places the caller must know about
the internal behaviour, about sector and disk numbers (e.g., it must know in
which sector of which disk a file header was written) and it does so by reading
the internal variables and knowing whether they are incremented before or after.
 That makes things a little tricky, often less comprehensible than the untar
module where the low level routines are less clever
*/


#define ALIAS_BIT 0x8000


static sector_t last_header,		/* il settore contenente l'ultimo header di file
						-- the sector containing the last file header */
	last_sector;	/* il settore vuoto che sarà scritto alla prossima occasione;
					nota che in questo modulo è gestito in modo un po' "sotterraneo":
					eccetto che per cose strane (header speciali, che non riguardano
					le normali routines) scrivi_settori viene sempre chiamata col
					tramite di write_next_sector (che si occupa di gestire un eventuale
					cambio disco: bisogna chiedergli comunque un settore a partire da 0,
					sa lei che a causa degli header di disco poi non è detto si vada
					al 0, ed è impossibile per bar), ma last_sector va incrementata
					subito dopo tale chiamata, col che appunto è sempre il prossimo
					settore. Attenzione che write_block butta fuori quando ha il buffer
					pieno (cioè non lo lascia con 512 bytes, lo svuota subito) 
					-- next sector to be written: after a call to write_next_sector
					which fills a sector, it was already incremented so it points
					to the next sector (or beyond the end of disk)
					*/
/*static short vol_numb_of_last_h; !!! non funziona, però era una buona idea, devo
	rimettercelo per bene */
short more_space;	/* number of free bytes in buffer, that is 512 - number of 
					bytes currently in the buffer */
/*static IOParam pb;*/
extern ParamBlockRec pb;
					/* è diventata una variabile globale per poter chiudere il file in
					caso di errore; per la verità non è necessario, basta un singolo
					campo, il RefNum... */
short inf_refn;		/* refnum for an input file, used for write tar/bar and overwrite sectors */
Boolean inf_is_open=false;
Boolean write_in_corso=0;	/* remembers that I was writing, hence what is in
				the disk is not correctly terminated by a null header */
Boolean devo_chiudere_in=false;

static Boolean questo_e_un_header;		/* set just before writing an header sector */
unsigned char previousFormat;	/* solo per tar, quelli bar sono sempre multivolume
			-- for tar format only, bar archives are always allowed to become multivolume */
extern short floppy_n;
extern unsigned char mac_file_name[];
extern short openfile_vrefnum;
extern long openfile_dirID;
extern Handle binary_types_h;

unsigned char alias_prefix[100];	/* new for suntar 1.3.2: if a/b/c happens
		to be the alias of d, then the file is opened as d but must be a/b/d
		in the tar archive. and if d is a directory and d/e is alias of f,
		it's opened as f but will be a/b/d/f... */
static char bar_date[12];
static char volLabel[100];
static short label_n;	/* used with volLabel for GNU archives */
long my_uid,my_gid;

static char *miei_titoli[]={"\pEspelli","\pOk","\pInizializza 720k"};
char FERRORita[]="Errore di lettura %d\n",
	FERRORing[]="Error %d during read\n";

void chiedi_altro_disco(void);
void set_bar_date(void);
Boolean surely_not_mac(char*);
void alias_error(short);
void write_GNU_V_header(void);
void fill_tmagic(tarh_type *);
void write_tar_volume_header(void);
void write_bar_volume_header(void);
short bar_header(unsigned char *,long,barh_type *);
void tardir (unsigned char *);
void tarfile (unsigned char*);
void tardata (unsigned char*,short);
void tarmacbin(unsigned char*,short);
void tar_writefork (long,short);
short tarheader (unsigned char *,long, char*);
void mac_to_unix(char*,unsigned char*);
void fill_checksum(tarh_type *);
short getvrefnum (unsigned char *);
void nametoolong (char *);
Boolean split_name_prefix(char*);
void this_was_end_of_file(void);
void init_hqx_encoding(struct binh_type*);
void encode_hqx_fork(long);
void fine_binhexed(void);

#define FLUSH_MIN_SIZE 10240


#if 1
void fillmem(dest, ch, len)
register char*dest;
register char ch;
register short len;
/* optimized version: it's not totally in assembly language as the optimized mcopy
but implements essentially the same optimization, only in a special case
(fillmem is usually used to clear memory, and I didn't want to write the code to
replicate a byte into the four bytes of a long) */
{
	if(len>10 && ch==0){
		 if(((long)dest)&1)
		 	*dest++=0, len--;
		 if(((long)dest)&2){
		 	#if 0
		 	*dest++=0, *dest++=0;
		 	#else
		 	asm{
		 		clr.w (dest)+
		 		}
		 	#endif
		 	len-=2;
		 	}
		 ch=len;
		 len>>=2;
		 /*
		 do{
		 	*((long*)dest)++ = 0; forbidden, Think C considers it an error
		 	(a cast transforms a variable name into an rvalue...)
		 	}
		 while(--len);
		 */
		 asm{
		 			moveq.l #0,d0	/* a move.l is faster than a clr.l ! misteries of
		 							the microcode */
		 			bra.s @inloop
		 	loop:	move.l d0,(dest)+
		 	inloop: dbra len,@loop
		 	}
		 if(!(ch&=3)) return;
		 len=ch;
		 ch=0;
		}
	while (--len>=0) *dest++ = ch;
}
#endif

Boolean pStrchr(unsigned char*,unsigned char);
Boolean pStrchr(str,c)
/* as strchr but for a Pascal string */
register unsigned char *str,c;
{
register unsigned char n=*str++;
while(n){
	if(*str++==c) return true;
	n--;
	}
return false;
}

void pascal_to_c(char *,unsigned char*);
void pascal_to_c(dest,src)
char *dest;
unsigned char*src;
/* unlike p2cstr it pays attention to NUL characters which are skipped hence do
not cause the C string to appear truncated */
{
unsigned char n=*src++;
while(n){
	if(*src)
		*dest++=*src++;
	else
		src++;
	n--;
	}
*dest='\0';
}

static void chiedi_altro_disco()	/* ask for another disk */
{
short i;
extern Boolean sm_checkbox_status,ins_checkbox_status;

ins_checkbox_status=false;
diskEject(true);

for(;;){
	sm_checkbox_status=ins_checkbox_status;
	i=aspetta_inserzione(in_Italia?"\pInserisci un altro disco su cui scrivere":
	"\pInsert another disk to be written",examine_disk|button_is_stop);
	ins_checkbox_status=sm_checkbox_status;
	if(i!=0) raise_error();		/* un annulla... 
					-- the user clicked the Cancel button
					*/
	if((is_wrprot()&1) && (di.tipo_device==d_driver&&di.v.fi.is_not_initialized||!ins_checkbox_status)){
		printf_protetto();
		}
	else if(di.tipo_device==d_driver && di.v.fi.is_not_initialized)
		disk_format(false);
	else{
		if(ins_checkbox_status){
			esamina_disco();
			if(is_wrprot()&1)
				printf_protetto();
			else{
				i=warning_400_800();
				if(i==0){
					/*
					ParamText(in_Italia?"\pLa scrittura distruggerà il contenuto attuale del disco":
						"\pWriting will destroy anything is currently on this disk",PNS,PNS,PNS);
					if(my_modal_dialog(133,miei_titoli,2) == 2)
					*/
					if(my_semimodal_dialog(146,miei_titoli,2,in_Italia?
						"\pLa scrittura distruggerà il contenuto attuale del disco":
						"\pWriting will destroy anything is currently on this disk",
						PNS,PNS) == 2)
						return;
					diskEject(true);
					}
				else if(i==2)
					return;
				}
			}
		else{
			if(warning_400_800()!=1) return;
			}
		}
	}
}


void reset_sector_count()
{
/* viene chiamata quando si dà un comando di create o append, quindi è il posto
buono per inserire qualunque inizializzazione
-- called for the Create and Append commands, it's the right place for initializing
some variables
*/
last_header=last_sector=di.archive_start_sector;
previousFormat= tar_unknown;
floppy_n=0;
/*vol_numb_of_last_h=floppy_n;*/
volLabel[0]=0;
set_bar_date();		/* deve essere comune per tutto l'archivio
					-- the same value must be used for all volumes in the archive */
more_space=512;		/* aggiunta per suntar 2.0, mi faceva comodo per MacBinary */
}

void write_next_sector(void);
static void write_next_sector()
{

	if((di.tipo_device==d_driver || di.tipo_device==d_scsi) && last_sector>=di.sectors_on_floppy){
		if(!bar_archive && previousFormat<=tar_singlevol&&tar_version<=tar_singlevol){
			start_of_line();
			printf("Disk full, write aborted !\n");		/* non dovrebbe capitare mai, */
			tronca_archivio();		/* visto che controllo prima, ma nel caso...
						-- that should never happen, that's checked at every header,
						but if that check should contain a bug, don't crash
						*/
			raise_error();
			}
		chiedi_altro_disco();
		last_sector=di.archive_start_sector;	/* deve essere =0 perché scrivo sempre
				tutti i settori (non per niente questa routine si chiama così...) */
		}
	if(last_sector==di.archive_start_sector){
		Boolean bad;
		floppy_n++;
		warning_first_write(0,&bad);
		if(bar_archive){
			write_bar_volume_header();
			}
		else{
			if(volLabel[0] && tar_version==tar_GNU) write_GNU_V_header();
			if(floppy_n!=1 && !questo_e_un_header &&
			   (previousFormat==tar_GNU || tar_version==tar_GNU&&previousFormat!=tar_POSIX) ){
			   		/* scrivi un header di continuazione file 
					-- write a 'M' header for a file splitted between disks */
				write_tar_volume_header();
				}
			}
		}
	scrivi_settore(last_sector,disk_buffer);
	check_wr_err();
	questo_e_un_header=false;
	settori_passati++;
}


void dim_720k_button(WindowRecord *);
static void dim_720k_button(w)
WindowRecord *w;
/* dim the button which can't be used without a SuperDrive */
{
	short	kind;
	Handle	h;
	Rect	r;
	GetDItem ((DialogPtr)w,3,&kind,&h,&r);
	HiliteControl(h, 255);
}

short warning_400_800()
{
/* returns:
0 no dialog
1 dialog and the disk was ejected
2 dialog and the disk is OK
*/
#define SUPERDRIVE	4
register short i=0;
if( di.tipo_device==d_driver && (di.sectors_on_floppy==800 || di.sectors_on_floppy==1600) && !accetta_GCR){
	unsigned char b[2];
	b[0]=1;
	b[1]= di.sectors_on_floppy==800 ? '4' : '8';
	#if 0	/* version 1.3.1 */
	if(in_Italia )
		ParamText("\pUn disco da ",b,"\p00K non può essere\rletto su un sistema UNIX",PNS);
	else
		ParamText("\pA ",b,"\p00K disk can\'t be read\ron an UNIX machine",PNS);
	my_alert();
	#else
	{static Point wPos={-1,-1};

	i=semimodalDialog(145,&wPos,NULL,miei_titoli,3,
		in_Italia?"\pUn disco da ":"\pA ",b,
		in_Italia?"\p00K non può essere\rletto su un sistema UNIX":
		"\p00K disk can\'t be read\ron an UNIX machine",
		teJustCenter,true, di.v.fi.supports_720K ? NULL : dim_720k_button);
	}
	#endif

	if(i==1)	/* Eject */
		diskEject(true);
	else if(i==3){	/* initialize 720k */
		short inPlace;
		
		disk_format(false);
		if( di.tipo_device==d_notopen || di.sectors_on_floppy!=1440 ||	
					/* disk_format may fail, or may be
					canceled by the user: it would be better to have a return code
					from it, but I can succeed to understand what happened without
					any return code */
		    testa_stato(&inPlace,1) || !inPlace){
			diskEject(true);
			return 1;
			}
		else
			return 2;
		}
	}
return i;
}


void printf_protetto()
{
	start_of_line();
	printf(in_Italia?"Disco protetto da scrittura !\n":"Write protected disk !\n");
	if(di.v.fi.opened_by_insertion==1) diskEject(true);
}

static Boolean surely_not_mac(buffer)
char buffer[512];
{
if(di.v.fi.disk_code!=noErr&&di.v.fi.disk_code!=noMacDskErr) return true;
leggi_settore(2,buffer);
return err_code!=0;
}

short warning_first_write(n,bad_sects)
short n;
Boolean *bad_sects;
/* n=0 if called by a command in the write menu (don't return without a writable disk),
1 for initialize */
{
/* called before the first write to a disk which is still containing
what it was containing when it was inserted: it's the last chance to
avoid a data loss
returns:
-1: (only if n=1) the user canceled the operation but the disk wasn't ejected yet
0:	the user approved the operation
1:	it's not a Mac disk (no dialog)
2:	it's an empty Mac disk (no dialog)
*/

char buffer[512];
unsigned char mess_ita[]="\pQuesto disco contiene dei settori marcati difettosi",
			mess_ing[]="\pIn this disk some sectors are marked bad";
extern char *titoli_si_no[];
char *titoli_156[]={"\pTermina","\pOk"};

if(di.tipo_device==d_file ||di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
	return 1;	/* there should be no danger to trash data on such devices... */
if(di.tipo_device==d_scsi){
	if(di.os==mac_HFS)	/* was assigned by inquire_SCSI_device */
		ParamText(in_Italia?
				"\pQuesto disco contiene partizioni in formato Macintosh, sei sicuro di volerci scrivere ?":
				"\pThis disk contains some Macintosh partitions, do you really want to write to it ?",
				PNS,PNS,PNS);
	else
		ParamText(in_Italia?
				"\pQuesto disco non è in formato Macintosh, ma sei sicuro non contenga dati da conservare ?":
				"\pThis disk is not in Macintosh format, but are you sure it does not contain good data ?",
				PNS,PNS,PNS);
	if( my_modal_dialog(139,titoli_si_no,2) ==1)
		return 0;
	else
		raise_error();
	}

for(;;){
	Boolean guarda_files;

	*bad_sects=false;
	while(surely_not_mac(buffer)){
		short inPlace;
		testa_stato(&inPlace,0);
		if(!n&&di.tipo_device==d_driver&&di.v.fi.is_not_initialized){
			disk_format(false);
			if(di.tipo_device!=d_notopen){
				if(testa_stato(&inPlace,1) || !inPlace)
					diskEject(true);
				else if(is_wrprot()&1)
					printf_protetto();
				else
					return 1;
				}
			chiedi_altro_disco();
			}
		else
			return 1;
		}
/*printf("n files=%d\n",*(int*)&buffer[12]);
printf("n dirs=%d\n",*(int*)&buffer[82]);
printf("n files tot=%ld\n",*(long*)&buffer[84]);
printf("n dirs tot=%ld\n",*(long*)&buffer[88]);
*/
/* if it's a Macintosh disk (same test used by is_mac_disk in suntar.c) see if
it contains more than one file (the first one is the desktop) or more than zero folders
*/

	if( (*(short*) buffer== 0xD2D7 || *(short*) buffer== 0x4244) ){ /* MFS o HFS */
		guarda_files=true;
		if( *(short*) buffer== 0x4244 && ((*(short*)&buffer[10])&0x200)){
			/* see TN 287: System 7 sets that flag if one or more bad sectors were 
			found and marked as unusable */
			*bad_sects=true;
			ParamText(in_Italia?mess_ita:mess_ing,PNS,PNS,PNS);
			if(!di.isEjectable){
				if( my_modal_dialog(156,titoli_156,2) ==1)
					raise_error();
				}
			else{
				if( my_modal_dialog(133,miei_titoli,2) ==1){
					if(n) return -1;
					guarda_files=false;
					}
				}
			}
		if(guarda_files){
			if(*(unsigned char*)&buffer[36]<=27 && /* length of volume name */
 			   (*(short*)&buffer[12]>1 ||		/* files in root directory, e il primo è il Desktop */
 			   (*(short*) buffer== 0x4244 &&*(short*)&buffer[82]>0)) ){	/* folders in root directory */
				ParamText(in_Italia?
				"\pQuesto disco contiene files in formato Macintosh, verranno sovrascritti":
				"\pThis disk contains some Macintosh files, their data will be overwritten",
				PNS,PNS,PNS);
				if(!di.isEjectable){
					if( my_modal_dialog(156,titoli_156,2) ==1)
						raise_error();
					return 0;
					}
				else{
					if( my_modal_dialog(133,miei_titoli,2) ==2){
						if(!n)de_Mac_ize(buffer);
						return 0;
						}
					if(n) return -1;
					}
				}
			else{
				if(!n) de_Mac_ize(buffer);
				return 2;
				}
			}
		chiedi_altro_disco();
		}
	else if(di.os==msdos){
		short ris[3];
		mcopy(buffer,disk_buffer,512);
		if(!visita_FAT_msdos(ris) && ris[1]){
			*bad_sects=true;
			mcopy(disk_buffer,buffer,512);
			ParamText(in_Italia?mess_ita:mess_ing,PNS,PNS,PNS);
			if(!di.isEjectable){
				if( my_modal_dialog(156,titoli_156,2) ==1)
					raise_error();
				}
			else{
				if( my_modal_dialog(133,miei_titoli,2) ==1){
					if(n) return -1;
					chiedi_altro_disco();
					}
				else
					return 1;
				}
			}
		else{
			mcopy(disk_buffer,buffer,512);
			return 1;
			}
		}
	else
		return 1;
	}
}

void de_Mac_ize(buffer)
char buffer[512];
/* the Finder keeps a copy of sector 2 in the sector before the last one, and
PBMountVol (called by GetNextEvent) when sees a disk with an invalid sector 2
(what happens for any tar disk with more than 1024 bytes of data on it)
but a valid sector 2878 (or 1598 or whatever) tells it's a Mac disk. (At first,
we believed that the cause were tags, but a technical note explained that tags
are no more required by the file system, hence they do not exist in drive where
the hardware doesn't provide them, and the MFM format doesn't have tags).
So it's better to deMac-ize disks when creating an archive. I prefer NOT to do
that when sector 2 was overwritten by the UNIX tar, which obviously does not know
about the Mac and the Finder, in that case in order to de-Macize I should write
to the disk even if the user asked only to read, not a good thing
*/
{
leggi_settore(di.sectors_on_floppy-2,buffer);
if(err_code) return;
if(*(short*) buffer== 0xD2D7 || *(short*) buffer== 0x4244 )
	buffer[0]++;	/* it's better to preserve everything, only change the
					code */
write_sectors(di.sectors_on_floppy-2,buffer,1);	/* non-buffered write */
di.v.fi.disk_code=noMacDskErr;
}

void write_tar_volume_header()
/* writes the 'M' continuation header on GNU tar volumes following the first one */

{
tarh_type buffer;
long old_size,delta_size;
short i;

/* get the needed data, which were saved when writing the last normal header */
mcopy(&buffer,ultimo_header,sizeof(ultimo_header));
fillmem(&buffer.name[sizeof(ultimo_header)],0,512-sizeof(ultimo_header));
old_size=untar_number(buffer.size,-1);
fillmem(buffer.size, '\0', 12);

delta_size = (long)avail_sectors_for_file<<9;
last_offset += delta_size;

if(old_size<=delta_size) return;	/* il file terminava proprio 
			nell'ultimo settore del disco precedente
			-- the file happened to end in the last sector of the previous disk,
			hence this disk must not contain an 'M' continuation header
			*/

numstr(&buffer.size, old_size-delta_size, 12);
numstr(&buffer.offset, last_offset, 12);
buffer.linkflag = 'M';
fill_tmagic(&buffer);
fill_checksum(&buffer);

/* update the saved informations */
copia_ultimo_header(&buffer,(sector_t)1);

/* last_offset l'ho già calcolato */
ultimo_disco_espulso=true;	/* forse... bisogna vedere come ritrovo l'header da 
	azzerare in tronca_archivio !!!
	-- unused variable, but in the future it should serve to help clearing
	the last_header in case of error
	*/

scrivi_settore(last_sector,&buffer);
check_wr_err();
last_sector++;
}

static void write_GNU_V_header()
{
tarh_type buffer;
long curr_time;
fillmem(&buffer.name,0,512);
buffer.linkflag = 'V';
strcpy(buffer.name,volLabel);
strcat(buffer.name," Volume ");	/* the real GNU tar writes the volume number
								only for multivolume archives, but suntar selects
								the files interactively and can't know in advance
								whether it will get a single-volume archive */
my_itoa((long)++label_n,&buffer.name[strlen(buffer.name)]);

GetDateTime(&curr_time);
numstr(&buffer.mtime, curr_time - unixtime, 12);
fill_checksum(&buffer);

scrivi_settore(last_sector,&buffer);	/* last_sector is surely 0 */
check_wr_err();
last_sector++;
}


static void set_bar_date()
/* read the current date an time and store it in the format of the date field
of the volme header for bar archives */
{
DateTimeRec cdate; 
register char*p = bar_date;
register short i;

GetTime (&cdate);
cdate.year %= 100;	/* solo le ultime due cifre dell'anno... */
for(i=0;i<5;i++){
	*p++ = '0'+ ((short*)&cdate.year)[i] /10;
	*p++ = '0'+ ((short*)&cdate.year)[i] %10;
	}
*p='\0';
}

void write_bar_volume_header()
{
/* when there were doubts about what should be written (see untar.c)
I preferred to write clean information rather than to follow the
silly rules of the original bar: there seems to be no compatibility
problem, since bar ignores the contents of the fields which it
writes in a silly way
*/
barh_type buffer;
register long oldsize;
short i;

fillmem(&buffer,0,512);
my_itoa(my_uid,&buffer.uid);
my_itoa(my_gid,&buffer.gid);
buffer.bar_magic[0]='V';
buffer.bar_magic[1]='\0';
my_itoa((long)floppy_n,&buffer.volume_num);
buffer.compressed='0';
mcopy(&buffer.cdate,bar_date,12);
strcpy(buffer.name,volLabel);

if(floppy_n==1)
	oldsize = 0L;
else{
	oldsize = untar_number(((barh_type*)ultimo_header)->size,-1);
	oldsize = (oldsize+511)>>9;		/* numero di settori necessari */
	oldsize -= avail_sectors_for_file;	/* meno quelli già usati 
			-- size = number of needed sectors at the time of the last header
				minus number of available sectors on previous disk
			*/
	avail_sectors_for_file += di.sectors_on_floppy-1;
	}

numstr(buffer.size, oldsize<<9, 12);
bar_checksum(&buffer);

scrivi_settore(last_sector,&buffer);
check_wr_err();
last_sector++;
}

static short bar_header(fname, fsize, barh)
  unsigned char *fname;
  long fsize;
  register barh_type *barh;
{
	fillmem(barh, 0, 512);
	mac_to_unix(barh->name, fname);
	if (strlen(uname) >= 200) nametoolong(my_p2cstr(fname));
	if(!strcmp("./",barh->name) || !barh->name[0] ) return -1;		/* succede nel system 7.0 se 
				 come folder da salvare seleziono un volume
				-- I get that string in System 7, a whole volume such as "HD40"
				becomes "./", but it's better not to save any entry for "HD40" !
				*/

	if (statb.ioFlAttrib & ioDirMask)
		if (barh->name[strlen(barh->name)-1] != '/')
			strcat(barh->name,"/");
	numstr(barh->mode, (long)((statb.ioFlAttrib & ioDirMask) ? dirmode : filemode), 8);
	numstr(barh->uid, my_uid, 8);
	numstr(barh->gid, my_gid, 8);
	numstr(barh->size, fsize, 12);
	numstr(barh->mtime, (long) statb.ioFlMdDat - unixtime, 12);
	barh->linkflag = ((statb.ioFlAttrib & ioDirMask) ? '\0' : '0');	/* it's not clear
		what I should write here, see the discussion un untar.c */
	bar_checksum(barh);
	questo_e_un_header=true;
	writeblock(barh, 512);
	/*vol_numb_of_last_h=floppy_n;*/
	last_header=last_sector-1;	/* ho bisogno di last_sector NON ancora incrementato
			al settore successivo, ma incrementato degli eventuali header extra !
			-- last_sector was already incremented, and I can't get the value before
			calling write_block since it may add a volume header */
	copia_ultimo_header(barh,last_sector);	/* last_sector incrementato */
	return 0;
}

void bar_checksum(buf)
char *buf;
{
register short i;
register long chk= ' '*8;
for(i=0;i<48;++i)
	chk += (unsigned char) buf[i];
for (i+=8; i < 512; ++i)
	chk += (unsigned char) buf[i];
numstr(((barh_type*)buf)->chksum,chk,8);
}

void writeblock(p,len)
register char*p;
short len;
{
if(len>=more_space){
	mcopy(&disk_buffer[512-more_space],p,more_space);
	p+=more_space;
	len-=more_space;
	write_next_sector();
	last_sector++;
	more_space=512;
	check_events(false);
	if(!len) return;
	}
mcopy(&disk_buffer[512-more_space],p,len);
more_space-=len;
}

unsigned char get_linkflag(length,doerror)
long *length;
short doerror;
{
unsigned char linkflag;
	if(bar_archive){
		linkflag=guess_bar_linkflag();
		if( (*length=untar_number( ((barh_type*)disk_buffer)->size,doerror)) ==-1)
			return 0xFF;
		if(linkflag=='1'||linkflag=='5') *length=0;
		}
	else{
		/* for directories, the size field may be used to store some informations,
		hence I can't rely on it being 0 */
		linkflag=((tarh_type*)disk_buffer)->linkflag;
		if(linkflag=='5' || ((tarh_type*)disk_buffer)->name[
			strlen(((tarh_type*)disk_buffer)->name)-1] == '/'){
			linkflag='5';
			*length=0;
			}
		else if(linkflag=='V'||linkflag=='1'||linkflag=='2')
			*length=0;
		else{
			if( (*length=untar_number(((tarh_type*)disk_buffer)->size,doerror)) ==-1)
				return 0xFF;
			}
		}
	return linkflag;
}


void cerca_fine()	/* find the end of the archive: used for Append */
{
long length;
enum formats fmt;
Boolean message_given=false;

previousFormat=tar_unknown;
listonly=1;		/* don't use buffering */

if(di.sectors_on_floppy==0){	/* empty file */
	printf(in_Italia? "Archivio vuoto, uso il formato tar come default\n":"Empty archive, defaulted to tar format\n");
	last_sector=last_header=0;
	bar_archive=false;
	floppy_n=1;
	/*vol_numb_of_last_h=floppy_n;*/
	return;
	}
else
	fmt=identify_format();
if(fmt==tar_format){
	bar_archive=false;
	if( di.sectors_on_floppy && hasVheader || ((tarh_type*)disk_buffer)->linkflag=='M' ) {
		previousFormat=tar_GNU;	/* se è già
				multivolume, vuol dire che il GNU tar è disponibile...
				-- if it's already GNU multivolume, later I must not bother the
				user asking if I may create a multivolume archive, and I must
				use the GNU format even if the current setting of tar_version
				is POSIX */
		/*vol_numb_of_last_h=floppy_n;*/
		last_header=di.archive_start_sector;
		if(hasVheader) last_header++;
		}
	floppy_n=1;
	}
else if(fmt==bar_format){
	bar_archive=true;
	floppy_n=untar_dec_number( ((barh_type*)disk_buffer)->volume_num,true);
	mcopy(bar_date,((barh_type*)disk_buffer)->cdate,12);
	/*vol_numb_of_last_h=floppy_n;*/
	last_header= di.archive_start_sector + (untar_number( ((barh_type*)disk_buffer)->size,true)+1023)>>9;
	}
else
	raise_error();

for(;;){
	static char sita[]=" (dovresti assegnare l\'opzione \"tar version\")\n",
				sing[]=" (you should set the \"tar version\" option)\n",
				snew[]="\n";
	register unsigned char linkflag;
	disable_autoflush(1);
	while(last_header>=di.sectors_on_floppy){
		if(di.tipo_device==d_file){
			if(last_header==di.sectors_on_floppy){	/* tutto OK, semplicemente manca il settore nullo */
				last_sector=last_header;
				enable_autoflush();
				return;
				}
			else
				error_message("Incomplete archive\n");
			return;
			}
		if(fmt==bar_format||previousFormat==tar_GNU||previousFormat!=tar_POSIX&&tar_version==tar_GNU){
			Boolean opt_ok= fmt==bar_format||tar_version==tar_GNU;
			if(in_Italia)
				printf("La fine dell\'archivio non è in questo volume%s",opt_ok||message_given ? snew : sita);
			else
				printf("The end of archive is not in this volume%s\n",opt_ok||message_given ? snew : sing);
			message_given=true;
			raise_error();
			}
		else{	/* potrebbe essere POSIX...
				-- it could be POSIX, but maybe the user has not set the option
				and in that case suntar suggests him to do that */
			short i;
			Boolean was_POSIX;
			last_header-=di.sectors_on_floppy;
			do{
				diskEject(true);
				i=aspetta_inserzione(in_Italia?
					"\pInserisci il prossimo disco tar":"\pInsert next tar disk",button_is_stop);
				if(i) raise_error();
				}
			while(di.tipo_device==d_driver&&di.v.fi.is_not_initialized);
			was_POSIX=previousFormat==tar_POSIX;
			i = tar_check_settore0(0,false);
			if(i==-1){		/* bad header, but it's the normal case for POSIX disks
							following the first one */
				if(previousFormat<=tar_singlevol&&tar_version!=tar_POSIX){
					start_of_line();
					if(in_Italia)
						printf("Probabile formato POSIX multivolume%s",message_given?snew:sita);
					else
						printf("Probable multivolume POSIX format%s",message_given?snew:sing);
					message_given=true;
					}
				previousFormat=tar_POSIX;
				}
			else if(previousFormat==tar_GNU){		/* it was not so when entering in
					this while, but check_settore_0 may assign it */
				if(was_POSIX) error_message("Not a multivolume POSIX tar disk !\n");
				if(in_Italia)
					printf("Formato GNU tar%s",message_given?snew:sita);
				else
					printf("GNU tar format%s",message_given?snew:sing);
				message_given=true;
				/*vol_numb_of_last_h=floppy_n;*/
				last_header= hasVheader ? 1 : 0;
				}
			}
		}
	check_events(false);
	leggi_settore(last_header,disk_buffer);
	if(check_error()) raise_error();

	last_sector=last_header;
	if(check_all_zero(disk_buffer)){
		if(last_header==0||(bar_archive||hasVheader)&&last_header==1)
			printf(in_Italia?"Archivio vuoto\n":"No files in the archive\n");
		enable_autoflush();
		return;
		}
	linkflag= get_linkflag(&length,true);
	if(linkflag=='L'||linkflag=='K')
		previousFormat=tar_GNU;
	else if(linkflag!='1'&&linkflag!='2'&&linkflag!='V'&&linkflag!='M')
		print_info(bar_archive ? ((barh_type*)disk_buffer)->name :
			((tarh_type*)disk_buffer)->name,length);
	last_header += (length+1023)/512;
	}
}

void svuota_buffer()	
/* empty the buffer, used at the end of a file, next data
will be a file header which must be aligned at the start of a sector;
really, it's useful only for MacBinary files, since tardata internally
groups the data in 512 bytes chunks
*/
{
if(more_space!=512){
	/* succede solo per MacBinary, visto che tardata scrive comunque in blocchi di 512 */
	fillmem(&disk_buffer[512-more_space],0,more_space);	/* azzera quello che resta: 
				il tar per UNIX non lo fa, ma non è bello lasciare della robaccia...
				-- clear the empty part of the buffer before writing it out: the
				UNIX tar doesn't do that, but it's not professional to write
				garbage data (well, the Macintosh too does that, the 128 bytes
				of reserved data in the resource fork are garbage: but, think
				what would happen if that garbage happens to be a part of your
				personal mail, or reserved informations... The reserved bytes in
				the suntar application often happen to contain a chunk of its source
				code, which was saved before compiling and is still in the disk 
				buffer)
				Furthermore, that garbage is less redundant than a sequence of
				zeroes, hence a compression program will get worse results...
				*/
	write_next_sector();
	last_sector++;
	}
more_space=512;
}

static void svuota_buffer_macbin()
/* for the New command */
{
long count;
if(more_space!=512){
	flush_buffers(2);
	fillmem(&disk_buffer[512-more_space],0,more_space);
	count= ((512-more_space)+127)&~127;	/* this dissincronyzes the output which must
		be at a 512 bytes multiple for the buffering module, but it's the last
		operation just before closing everything... */
	err_code=FSWrite(di.m_vrefnum,&count,disk_buffer);
	}
more_space=512;
}

void svuota_buffer_binhex(void);
void svuota_buffer_binhex()
/* for the New command */
{
long count;
if(more_space!=512){
/*dp("ancora %d caratteri\n",512-more_space);*/
	flush_buffers(2);
	count= (512-more_space);	/* this dissyncronizes the output which must
		be at a 512 bytes multiple for the buffering module, but it's the last
		operation just before closing everything... */
	err_code=FSWrite(di.m_vrefnum,&count,disk_buffer);
	}
more_space=512;
}


void azzera_settore()	/* clear a sector, that is write an end of archive header */
{
/* note that when writing multiple files (e.g. a folder, or using the
"write selected files" button) it's called only once, hence it would not be a bad idea
to perform some expensive cleanup operation (flush_buffers or something like that)  */
svuota_buffer();
fillmem(disk_buffer, 0, 512);
questo_e_un_header=true;
write_next_sector();
/* niente last_sector++, per ovvie ragioni
-- don't increment last_sector, another write must overwrite this sector
*/
this_was_end_of_file();

last_header=last_sector;
}

void tronca_archivio()
/* truncate an archive, by clearing the last header: used when,
due to an error, what follows that header can't be completed */
{

if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT) return; /* really,
	I may do that if that sector is still in the buffer, but chances are not good */
/*if(vol_numb_of_last_h != floppy_n) return;*/

last_sector=last_header;
/* multivolume, se il disco è stato espulso deve richiederlo !!
o forse non lo faccio qui, ma nella gestione setjmp, questa qui è chiamata anche 
in caso di quit
o forse dare un messaggio, in fondo capita raramente...
-- in multivolume archives, the sector to be cleared may be on another disk,
which was ejected. The correct behaviour should be to ask the user to insert
back that disk, carefully check that it's the right disk and clear the sector,
or, if the user refuses to insert it back, print which sector (and file) 
must be cleared. By now, in that case I do nothing
*/
/*printf("<%ld>",(long)last_sector);*/
	/* I may avoid to flush things which are AFTER the last header: not only that
	saves time, but if the operation was interrupted because that's the wrong
	disk it might save good data */
invalid_after(last_sector);

read_sectors(last_sector,disk_buffer,1);	/* don't use the value stored in the buffers!!! */
if(err_code==0){
	/* compare it with the stored content of the last sector: it would be
	terrible to clear a sector in a disk which happens to be in that drive
	but is not the tar/bar disk I was writing !
	*/
	if(compare_mem(ultimo_header,disk_buffer,sizeof(ultimo_header))){
		fillmem(disk_buffer, 0, 512);
		scrivi_settore(last_sector,disk_buffer); /* senza verifica di errore */
		this_was_end_of_file();
		flush_buffers(0);
		if(di.tipo_device==d_file){
			/* truncate the file... */
			long size;
			if(di.v.file_current_s==1) di.v.file_current_s=0; /* the end-of-file
					sector is useless if it's all what is in the file... */
			if (GetEOF(di.m_vrefnum,&size)==noErr && size>(di.v.file_current_s<<9)){
				/*dp("tronco da %ld a %ld\n",size,di.v.file_current_s<<9);*/
				SetEOF(di.m_vrefnum,di.v.file_current_s<<9);
				}
			}
		}
	else
		invalid_buffers();
	}
else
	invalid_buffers();
more_space=512;
}

Boolean compare_mem(p,q,s)
register char*p,*q;
register short s;
{
while(s){
	if(*p++ != *q++) return false;
	s--;
	}
return true;
}

void print_ready()
{
long sz;
char print_present=di.tipo_device==d_file ||
	di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT ||
	(unsigned long)di.sectors_on_floppy>=0x70000000;


if(print_present)
	sz=last_header;
else
	sz=di.sectors_on_floppy-last_header-1;
printf("%ld",sz/2);
if(sz&1)printf(in_Italia?",5":".5");
if(print_present)
	printf(in_Italia?" Kbytes nell\'archivio\n":" Kbytes in the archive\n");
else
	printf(in_Italia?" Kbytes disponibili\n":" Kbytes available\n");

}

/********************************/

void check_and_eject()
/* eseguita in caso di QUIT ma anche in caso di errore
-- executed in case of error or Quit
*/
{
if(write_in_corso){
	tronca_archivio();
	write_in_corso=false;
	}
else{
	if(dirty_buffers()) invalid_buffers();
	}
if(di.tipo_device==d_driver&&di.v.fi.opened_by_insertion)
	diskEject(true);
}

/************************/
void check_wr_err()
{
if(err_code){
	start_of_line();
	if(err_code==-65)
		printf(in_Italia?"Errore, disco assente\n":"Error, missing disk\n");
	else if (err_code==-34)
		printf(in_Italia?"Errore, disco pieno\n":"Error, disk full\n");
	else
		printf("Write error %d\n",err_code);
	if(!ignore_errors ){
		if(write_in_corso)
			tronca_archivio();
			
		raise_error();
		}
	}
}
/*********************************/

void my_tar(conv_mode)	/* handles a command from the Write menu */
short conv_mode;
{
short charsRead;

short n_types;
SFTypeList	myTypes;
extern unsigned char*nomeFormato;
extern unsigned char rewind_proposal[];

if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT &&
   di.v.si.current_tape_block!=0 && last_sector==0){
	if(my_semimodal_dialog(155,NULL,0,rewind_proposal,NULL,NULL)==2){
		di.v.si.current_tape_block=0x7FFFFFFF;
		archive_rewind();
		}
	else
		raise_error();
	}

conversion_mode=conv_mode;
if(conversion_mode==wmSetLabel){
	/* that's not a file format... */
	ParamText(in_Italia?"\pNome da assegnare all\'archivio :":
		"\pName assigned to the archive :",PNS,PNS,PNS);
	if(my_edt_dialog(142,volLabel,bar_archive?99:99-10,"\p")==1){
		my_p2cstr(volLabel);
		label_n=0;
		}
	return;
	}

if(conversion_mode==wmASCII){
	n_types=1;
	myTypes[0]='TEXT';
	binary=false;
	dataonly=true;
	nomeFormato="\p    (ASCII)";
	}
else if(conversion_mode==wmWriteTar){
	#if 0
	if(bar_archive)
		n_types=-1;
	else
		n_types=3;
	myTypes[0]='TARF';
	myTypes[1]='TEXT';	/* quelli estratti con MacCompress se provengono da UNIX
						(niente resource fork) sono TEXT KAHL
						-- .tar.Z files extracted with MacCompress are TEXT-KAHL,
						.tar files created by tar 3.0 are TARF TAR , those created
						by suntar may be TARF (copy to mac file) or TEXT (save sectors)
						*/
	myTypes[1]='????';	/* those created by StuffIt Deluxe, or coming from UNIX
						with another conversion */
						e se vengono da MS-DOS allora sono forse bina, o... meglio
						lasciar perdere e non filtrare
	#else
		n_types=-1;
	#endif						
	}
else{	/* wmDataFork o wmWriteMacBin o wmWriteSmart */
	n_types=-1;
	binary=true;
	dataonly= conversion_mode==wmDataFork;
	nomeFormato=conversion_mode==wmWriteMacBin?"\p(MacBinary)":
		conversion_mode==wmDataFork?"\p(Data fork)":
		conversion_mode==wmWriteSmart?"\p (Mixed DF)":"\p (Mixed MB)";
	}

more_space=512;
if(conversion_mode!=wmWriteTar){
	Str255 name;
	short i=get_file_or_folder(name,n_types,&myTypes);
	if(i<0) return;	/* cancel...*/
	if(i==2){
		extern char causale_list_dialog;
		causale_list_dialog=conversion_mode;
		select_and_write(curr_vrefnum);
		flush_buffers(0);
		return;
		}
	fase=writing_disk;
	write_in_corso=1;
	alias_prefix[0]='\0';
	tar_file_or_folder(name);
	azzera_settore();
	flush_buffers(0);
	write_in_corso=0;
	}
else{	/* file tar, non c'è da creare header o altro...
		-- write tar file: handle it directly here...
		*/
	sector_t more_sectors=0;
	long length;

	my_SF_Get(n_types,myTypes);
	if(!reply.good)return;

	if(apri_file("rb",&inf_refn))
		return;
	inf_is_open=true;
	fase=writing_disk;
	write_in_corso=1;
	if(bar_archive){
		/* read (and skip) the volume header, with some checks */
		charsRead = mac_fread(disk_buffer, 512, inf_refn);
		if(charsRead<0) error_message_1(in_Italia?FERRORita:FERRORing,err_code);
		if(((barh_type*)disk_buffer)->bar_magic[0]!= 'V' || 
  			 ((barh_type*)disk_buffer)->bar_magic[1] != 0)
			error_message(in_Italia?"Questo non è un archivio in formato bar\n":
				"Not a bar archive !\n");
		if(floppy_n==0 && last_sector==0) mcopy(bar_date,((barh_type*)disk_buffer)->cdate,12);
		}
	do{
		charsRead = mac_fread(disk_buffer, 512, inf_refn);
		if(charsRead<0) error_message_1(in_Italia?FERRORita:FERRORing,err_code);
		if(charsRead>=0 && charsRead<512)
			fillmem(&disk_buffer[charsRead], 0, 512-charsRead); /* non indispensabile, 
									ma è brutto che lo spazio extra
									nell'ultimo settore del file resti garbage.
									-- see before, clear the buffer to avoid writing
									garbage at the end of the last sector
									*/
		/* meglio non controllare errore, i file di tar2 sono cattivi e invece va poi 
		tutto bene...*/
		if(more_sectors==0){
			last_header=last_sector;
			if(charsRead==0 || check_all_zero(disk_buffer))
				charsRead=0;	/* to exit the loop */
			else{
			/* it was an header: do the standard work for headers, extract the size 
			field and continue */
				questo_e_un_header=true;
				copia_ultimo_header(disk_buffer,last_sector+1);
				last_offset=0;
				(void)get_linkflag(&length,true);
				if(length<0){
					check_and_eject();
					raise_error();
					}
				else{
					more_sectors=(length+511)/512;
					print_info(bar_archive?((barh_type*)disk_buffer)->name:disk_buffer,length);
					if(di.tipo_device!=d_file && !bar_archive) controlla_che_ci_stia(more_sectors);
					}
				}
			}
		else{
			if(charsRead==0)
				error_message(in_Italia?"Archivio incompleto\n":"Incomplete archive\n");
			more_sectors--;
			}
		check_events(false);
		write_next_sector();
		last_sector++;
		}
	while(charsRead>0);
	flush_buffers(0);
	write_in_corso=0;

	FSClose(inf_refn);
	inf_is_open=false;
	last_sector=last_header;
	}
}


void close_input_files()
{	/* da chiamare in caso che una longjmp esca dalla routine precedente */
if(inf_is_open){
	FSClose(inf_refn);
	inf_is_open=false;
	}
if(devo_chiudere_in){
	(void) PBCloseSync(&pb);
	devo_chiudere_in=false;
	}
}

void FillMacBin2Fields()		/* transforms a MacBinary header into a MacBinary II
								header: the data are stored in tarh, which is currently
								used as a generic binary buffer */
{
register short i;

/* can't declare a pointer to binh_type since it's misaligned by one byte */
tarh.name[101]=tarh.name[74];		/* because I didn't clear that byte, as the
					MacBinary standard says, but left there the finder flags which 
					MacBinary II says must be stored elsewhere */
tarh.name[74]=0;	/* but now it's better to follow the standard */
tarh.name[122]=tarh.name[123]=129;
current_crc=0;
for(i=0;i<124;i++)
	CalcCRC(tarh.name[i]);
CalcCRC(0);
CalcCRC(0);
*(short*)&tarh.name[124]=current_crc;
}

void assegna_tar_version()
{
extern char *titoli_si_no[];

if((di.tipo_device!=d_scsi||di.v.si.DeviceType!=TAPE_UNIT) && dirty_buffers())
	flush_buffers(0);
tar_version=my_modal_dialog(140,NULL,0);

ParamText(in_Italia?
	"\pDevo assegnare l\'opzione \"tar version\" permanentemente ?":
	"\pSet the option \"tar version\" permanently ?",PNS,PNS,PNS);
if(my_modal_dialog(130,titoli_si_no,2)==1)
	save_options(NULL);
}


void controlla_che_ci_stia(n_sectors)
sector_t n_sectors;
/* check that there is enough space in the disk for next file: if not,
a multivolume archive could be generated, but the standard tar does not
accept multivolume archives
*/
{
if(previousFormat<=tar_singlevol &&tar_version<=tar_singlevol &&
   last_header+n_sectors+1>=di.sectors_on_floppy){	/* l'header va nel 
			settore last_header, seguono n_sectors settori di dati fino a last_header +
			n_sectors e il settore last_header+n_sectors+1 deve contenere gli zeri di
			indicazione fine archivio */
	if(previousFormat==tar_unknown&&tar_version==tar_unknown){
		if((di.tipo_device!=d_scsi||di.v.si.DeviceType!=TAPE_UNIT)&&dirty_buffers())
			flush_buffers(0);
		ParamText(in_Italia?
			"\pSpazio insufficiente, creo un archivio multidisco ?":
			"\pNot enough space, OK to create a multivolume archive ?",PNS,PNS,PNS);
		assegna_tar_version();
		}

	if(tar_version==tar_singlevol){
		printf(in_Italia?"Errore, spazio insufficiente\n":"Error, not enough space\n");
		/*tronca_archivio();	/* bisogna riazzerare l'ultimo header per avere un disco consistente */
		raise_error();	/* che chiama check_and_eject che chiama tronca_archivio, per cui
				quella chiamata sopra non serve */
		}
	}
}

static void alias_error(err)
short err;
{
if(err!=noErr){
	beep_in_foreground();
	error_message_1("Error %d in resolving alias\n",err);
	}
}

static Boolean split_name_prefix(uname)
char*uname;
{
short name_len;
if((name_len=strlen(uname))<100){
	strcpy(tarh.name,uname);
	}
else if(name_len>255)
	return false;
else{
	/* a few instructions from GNU cpio, file tar.c 
	© Free Software Foundation */
	/* If the length is between 100 and 256,
	then the filename will fit only if it is possible to break it
	into a 155 character "prefix" and 100 character "name".  There
	must be a slash between the "prefix" and the "name", although
	the slash is not stored or counted in either the "prefix" or
	the "name", and there must be at least one character in both
	the "prefix" and the "name" */
	/* Fit as much as we can into 'name', the rest into 'prefix'.  */
	char *suffix = uname + name_len - 100;

	/* We have to put the boundary at a slash.  */
	name_len = 100;
	while (*suffix != '/'){
		if(!--name_len) return false;
		++suffix;
		}
	strncpy (tarh.name, suffix + 1, name_len);
	if((name_len=suffix - uname)>155) return false;
	strncpy (tarh.POSIX_prefix, uname, name_len);
	}
return true;
}

Boolean absolutely_binary(OSType);
static Boolean absolutely_binary(t)
OSType t;
{
OSType c;
char *p;
short n_tipi;
LoadResource(binary_types_h);
HLock(binary_types_h);
p=*binary_types_h;
n_tipi=(*p++)>>2;
while(n_tipi--){
	mcopy(&c,p,4);
	if(t==c){
		HUnlock(binary_types_h);
		return true;
		}
	p+=4;
	}
HUnlock(binary_types_h);
return false;
}

void tar_uu(unsigned char *,short);
static void tar_uu(fname,vrefnum)
  unsigned char *fname;
  short vrefnum;
{
	void encode_uu_datafork(long);
	{char name[257];
	#if 0
	long uusize = ... +strlen(unixname) + 62*(statb.ioFlLgLen/45) +
						qualcosa che dipende da statb.ioFlLgLen%45 in modo
						non semplicissimo ma comunque univoco;
	if(di.file_format!=ff_macbin&&di.file_format!=ff_binhex){
		if(bar_archive)
			bar_header(fname, uusize, &tarh);
		else
			tarheader(fname, uusize, name);
		if(expert_mode) print_sector_n(last_sector-1);
		}
	else
		strcpy(name,tarh.name);
	#endif

	printf("uuencode %P (%ld bytes)\n",fname,statb.ioFlLgLen);

	if(statb.ioFlLgLen>=FLUSH_MIN_SIZE) flush_console();

	pb.ioParam.ioVRefNum = vrefnum;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsRdPerm;
	pb.ioParam.ioMisc = 0;
	pb.ioParam.ioNamePtr = fname;
	if (PBOpenSync(&pb)) pbsyserr(&pb);
	devo_chiudere_in=true;

	writeblock("begin 644 ",10);
	alias_prefix[0]='\0';	/*!!!*/
	mac_to_unix(tarh.name,fname);
	writeblock(tarh.name,strlen(tarh.name));
	}
	encode_uu_datafork(statb.ioFlLgLen);
}

/*********** end of routines by Speranza ****************/

/****** start of routines by Gail Zacharias (modified by Speranza) *****************/


void tar_file_or_folder (file_name)
unsigned char *file_name;	/* Pascal string */
{

disable_autoflush(1);

statf(file_name);

if (statb.ioFlAttrib & ioDirMask) {
	statb.ioVRefNum = getvrefnum(file_name);
	tardir(file_name);
	}
else{
	char *cp;
	#if 0	/* working on a C string. It's not correct on a name containing a \0 */
			/* (I've been told that some INITs do) */

	my_p2c(filename);
	cp = file_name+strlen(file_name);
	while (cp != file_name && cp[-1] != ':') --cp;
	if ((macbinh.nlen = strlen(cp)) >= 64) nametoolong(file_name);
	strcpy(macbinh.name, cp);
	tarfile(my_c2p(file_name));

	#else

	cp = & file_name[1+file_name[0]];
	while (cp != &file_name[1] && cp[-1] != ':') --cp;
	if ((macbinh.nlen = & file_name[1+file_name[0]] - cp) >= 64) nametoolong(my_p2cstr(file_name));
	mcopy(&macbinh.name, cp,macbinh.nlen);
	tarfile(&macbinh.nlen);

	#endif
	}
}

static void tardir (dname)
unsigned char *dname;	/* Pascal string */
{
	short isvolume;
	char name[256];
	long dirid = statb.ioDirID;
	short index = 0;
	unsigned char dlen;

	isvolume = bar_archive ? bar_header(dname, 0L, &tarh) : tarheader(dname, 0L, name);
	if(!isvolume){
		if(expert_mode) print_sector_n(last_sector-1); /* -1 perché è già stato
			incrementato; potrei spostare la printf sopra, ma allora non saprei se è
			un volume... */
		printf(in_Italia?"Cartella %s\n":"Folder %s\n",
			bar_archive?((barh_type*)&tarh)->name:name);
		}

	if(pStrchr(dname, ':'))
		pStrcpy(name, dname);
	else{
		pStrcpy(name,"\p:");
		pStrcat(name, dname);
		}
	if (name[name[0]] != ':') pStrcat(name,"\p:");
	while (1) {	/* visit the folder */
		statb.ioFDirIndex = ++index;
		statb.ioDirID = dirid;
		statb.ioNamePtr = &macbinh.nlen;
		if (PBGetCatInfoSync(&statb) || statb.ioResult) {
			if (statb.ioResult != fnfErr ) pbsyserr(&statb);
			return;	/* no more entries */
			}
		if(macbinh.nlen >= 64 || name[0]+macbinh.nlen>=sizeof(name)) nametoolong(my_p2cstr(&macbinh.nlen));
		dlen=name[0];
		pStrcat(name, &macbinh.nlen);	/* append the file/folder name
										to the current path */
		if (statb.ioFlAttrib & ioDirMask) 
			tardir(name);
		else
			tarfile(name);
		name[0]=dlen;
		}
}


static void tarfile (fname)
/* heavily modified by Speranza, in order to handle aliases */
  unsigned char *fname;
{
char name[256];
short vrefnum=0;
short old_as_len=alias_prefix[0];

extern char s_spazi[];

pStrcpy(name,fname);
if (statb.ioFRefNum) {
	/* I believe that if I have write permission on the file, than
	nobody else may open it, hence the "first" access path returned by
	PBGetCatInfo is also the only one. */
	if(di.tipo_device==d_file && di.m_vrefnum==statb.ioFRefNum){
		printf("The destination file %P has been skipped\n", name);
		return;
		}
	printf("Warning: file %P was already open\n", name);
	}

if( resolve_aliases && (statb.ioFlFndrInfo.fdFlags & ALIAS_BIT) ){
	if(! gHasResolveAlias){
		if((di.tipo_device!=d_scsi||di.v.si.DeviceType!=TAPE_UNIT)&&dirty_buffers())
			flush_buffers(0);
		ParamText(in_Italia?"\pPer risolvere gli alias ci vuole il System 7":
		"\pAliases can\'t be resolved without System 7",PNS,PNS,PNS);
		my_alert();
		}
	else{
		FSSpec mySpec;
		WDPBRec param;
		char is_folder,is_aliased;
		short io,start;

		io=FSMakeFSSpec(0,0L,name,&mySpec);
		alias_error(io);
		io=name[0];
		while(io>0 && name[io]!=':') io--;
		if(!alias_prefix[0]){
			start=alias_prefix[0]=1;
			alias_prefix[1]=':';
			}
		if(alias_prefix[0]+io<=99){
			if(io!=0 && name[1]==':')
				mcopy(&alias_prefix[alias_prefix[0]+1],&name[2],--io);
			else
				mcopy(&alias_prefix[alias_prefix[0]+1],&name[1],io);
			alias_prefix[0]+=io;
			}

		io=ResolveAliasFile(&mySpec,true,&is_folder,&is_aliased);
		if(io!=noErr){
			printf(in_Italia?"Alias %P: non trovato l\'originale\n":
			"Alias %P: original not found\n",name);
			alias_prefix[0]=old_as_len;
			return;
			}
/* ora, il metodo regolamentare sarebbe fare
io=FSpOpenDF(&mySpec,fsRdPerm,&pb.ioRefNum);	(data fork)
io=FSpOpenRF(&mySpec,fsRdPerm,&pb.ioRefNum);	(resource fork)
ma tutto il programma è impostato sui vecchi formati: quale è il nuovo
equivalente di PBGetCatInfo ? Ovvero, come la si deve chiamare se si hanno
solo le informazioni vecchio stile ? Per ora preferisco tradurre la specifica nuova 
in una vecchia, forse il modo che uso non è il migliore ma per ora l'importante 
è che funzioni 
-- at this point, I have a FSSpec, while all the routines by Zacharias expect
the old couple vRefNum-filename
A conversion to the new style would be more elegant, a conversion to the old
style is more compatible and easier to implement, hence that's what I did
*/
		if(is_folder){
#ifdef V_122
			PathNameFromDirID(mySpec.parID, mySpec.vRefNum, name);
			path_len=name[0]-1;	/* ci sarebbe da discutere che path name 
						mettere nell'archivio, quello dell'alias o quello dell'
						originale ? Così non ci metto nessun pathname, gli alias di
						folder appaiono al livello esterno
						-- which path must be written in the archive ? to remove
						the problem, aliases are written with the last name only:
						that's not the best solution, but it's simple to implement
						*/
			pStrcat(name,mySpec.name);
			pStrcat(name,"\p:");
			tar_file_or_folder (name);
#else
			short oldref;
			pStrcpy(name,"\p:");
			pStrcat(name,mySpec.name);
			pStrcat(name,"\p:");
			oldref=curr_vrefnum;

			param.ioVRefNum=mySpec.vRefNum;
			/*printf("FSSpec=%d %ld %P\n",mySpec.vRefNum,mySpec.parID,mySpec.name);*/
			param.ioNamePtr=NULL;
			param.ioWDProcID=signature;
			param.ioWDDirID=mySpec.parID;
			io=PBOpenWDSync(&param);
			alias_error(io);
			curr_vrefnum=param.ioVRefNum;
			SetVol(NULL,curr_vrefnum);
			register_WD(param.ioVRefNum);

			tar_file_or_folder (name);
			SetVol(NULL,curr_vrefnum=oldref);
#endif
			alias_prefix[0]=old_as_len;

			return;
			}
/* ora mi creo una WD sul file origine: poi non la chiudo per lo stesso motivo
che in Untar (gestione duplicate file name)
-- PBGetCatInfo may be called without a working directory ID (by using PBHGetCatInfo),
but tardata and other routines expect a vrefnum, hence I create a working directory. */
		param.ioVRefNum=mySpec.vRefNum;
		param.ioNamePtr=NULL;
		param.ioWDProcID=signature;
		param.ioWDDirID=mySpec.parID;
		io=PBOpenWDSync(&param);
		alias_error(io);

		vrefnum=param.ioVRefNum;	/* ad uso delle varie open...*/
		register_WD(param.ioVRefNum);

		/* fill the informations which statf should have returned */
		statb.ioNamePtr = mySpec.name;
		statb.ioFVersNum = 0;
		statb.ioVRefNum = param.ioVRefNum;
		statb.ioFDirIndex = 0;
		statb.ioDirID = 0;
		if (PBGetCatInfoSync(&statb) ) pbsyserr(&statb);

		statb.ioNamePtr = NULL;
		pStrcpy(name,mySpec.name);
		}
	}

if(di.file_format==ff_macbin||di.file_format==ff_binhex){
	binary=1;
	if(conversion_mode==100)
		tar_uu(name,vrefnum);
	else
		tarmacbin(name,vrefnum);
	}
else if(conversion_mode==wmWriteSmart || conversion_mode==wmWriteSmartMB){
	if(statb.ioFlFndrInfo.fdType == 'TEXT'){
		binary=0;
		tardata(name,vrefnum);	/* saved as ASCII */
		}
	else if(statb.ioFlRLgLen ||
	        conversion_mode==wmWriteSmartMB && !absolutely_binary(statb.ioFlFndrInfo.fdType) ){
		binary=1;
		tarmacbin(name,vrefnum);
		}
	else{
		binary=1;
		tardata(name,vrefnum);
		}
	}
else if (dataonly){
	if(binary || (statb.ioFlFndrInfo.fdType == 'TEXT'&&!(statb.ioFlFndrInfo.fdFlags&ALIAS_BIT)))
		tardata(name,vrefnum);
	else	/* Write ASCII on non-text files */
		switch(non_text_ASCII){
		case 0:	/* not saved */
			if(expert_mode) printf(s_spazi);
			printf("File %P (%s)\n",name,in_Italia?"non salvato":"not saved");
			break;
		case 1:	/* data */
			binary=1;
			tardata(name,vrefnum);
			binary=0;
			break;
		case 2:	/* ASCII */
			tardata(name,vrefnum);
			break;
		case 3:	/* macbinary */
			binary=1;
			tarmacbin(name,vrefnum);
			binary=0;
			break;
		}
	}
else
	tarmacbin(name,vrefnum);
alias_prefix[0]=old_as_len;
if(di.file_format==ff_macbin)
	svuota_buffer_macbin();	/* per approssimare per eccesso a multipli di 128 bytes... */
else if(di.file_format==ff_binhex)
	if(conversion_mode==100)
		svuota_buffer_binhex();
	else
		fine_binhexed();
else
	svuota_buffer();		/* per approssimare per eccesso a multipli di 512 bytes... */
}


static void tardata (fname,vrefnum)
  unsigned char *fname;
  short vrefnum;
{
	{char name[257];
	if(bar_archive)
		bar_header(fname, statb.ioFlLgLen, &tarh);
	else
		tarheader(fname, statb.ioFlLgLen, name);
	if(expert_mode) print_sector_n(last_sector-1);

	printf("File %s (%ld bytes)",bar_archive?((barh_type*)&tarh)->name:name, 
		statb.ioFlLgLen);
	}
	if(!binary) printf(" ASCII\n"); else vai_a_capo();
	if(statb.ioFlLgLen>=FLUSH_MIN_SIZE) flush_console();

	pb.ioParam.ioVRefNum = vrefnum;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsRdPerm;
	pb.ioParam.ioMisc = 0;
	pb.ioParam.ioNamePtr = fname;
	if (PBOpenSync(&pb)) pbsyserr(&pb);

	devo_chiudere_in=true;
	tar_writefork(statb.ioFlLgLen, 0);
}

static void tarmacbin(fname,vrefnum)
  unsigned char *fname;
  short vrefnum;
/* really, I'm using it also for BinHex */
{
	{char name[257];
	long fsize = macbinsz(statb.ioFlLgLen)+macbinsz(statb.ioFlRLgLen)+128L;
	if(di.file_format!=ff_macbin&&di.file_format!=ff_binhex){
		if(bar_archive)
			bar_header(fname, fsize, &tarh);
		else
			tarheader(fname, fsize, name);
		if(expert_mode) print_sector_n(last_sector-1);
		}
	else{
		pStrcpy(&macbinh.nlen,fname);
		pStrcpy(name,fname);	/* solo per stamparlo nel messaggio...*/
		my_p2cstr(name);
		}

	if(di.file_format==ff_binhex)
		printf("BinHex");
	else
		printf("MacBinary");
	printf(" %s (data %ld+res %ld bytes)\n",
				bar_archive ? ((barh_type*)&tarh)->name : name,
				statb.ioFlLgLen, statb.ioFlRLgLen);
	}
	if(statb.ioFlLgLen+statb.ioFlRLgLen>=FLUSH_MIN_SIZE) flush_console();
	macbinheader();
	if(di.file_format==ff_binhex)
		init_hqx_encoding((struct binh_type*)&tarh.name[1]);
	else
		writeblock(&tarh, 128);
	pb.ioParam.ioVRefNum = vrefnum;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsRdPerm;
	pb.ioParam.ioMisc = 0;
	pb.ioParam.ioNamePtr = fname;
	if (PBOpenSync(&pb)) pbsyserr(&pb);
	devo_chiudere_in=true;
	if(di.file_format==ff_binhex)
		encode_hqx_fork(statb.ioFlLgLen);
	else
		tar_writefork(statb.ioFlLgLen, 1);
	pb.ioParam.ioVRefNum = vrefnum;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsRdPerm;
	pb.ioParam.ioMisc = 0;

#define ioDirID ioFlNum
	if((pb.fileParam.ioDirID=openfile_dirID)!=0){
		pb.ioParam.ioVRefNum=openfile_vrefnum;
		pb.ioParam.ioNamePtr=mac_file_name;
		}
#undef ioDirID		/* must not conflict with statb.ioDirID */

	if (PBHOpenRFSync(&pb)) pbsyserr(&pb);
	devo_chiudere_in=true;
	if(di.file_format==ff_binhex)
		encode_hqx_fork(statb.ioFlRLgLen);
	else
		tar_writefork(statb.ioFlRLgLen, 2);
		/* if (fsize & 511L) writeblock(&tarh, 512 - (fsize & 511L));
		NO, ci deve pensare svuota_buffer */
}

void macbinheader()
{
typedef struct binh_type bht;
#include <stddef.h>	/* offsetof */

  fillmem(&macbinh.name[macbinh.nlen], 0, sizeof(macbinh)-offsetof(bht,nlen)-macbinh.nlen);	
  	/* anche i campi che seguono vanno azzerati... */
  mcopy(&macbinh.finfo, &statb.ioFlFndrInfo, sizeof(FInfo));
  macbinh.protected = 0;
  macbinh.zero = 0;
  macbinh.dflen = statb.ioFlLgLen;
  macbinh.rflen = statb.ioFlRLgLen;
  macbinh.cdate = statb.ioFlCrDat;
  macbinh.mdate = statb.ioFlMdDat;
  if(di.file_format!=ff_macintosh){
	tarh.name[0]=0;	/* use tarh as a generic binary buffer */
	mcopy(&tarh.name[1], &macbinh, sizeof(macbinh));
	if(di.file_format!=ff_binhex ) FillMacBin2Fields();
	}
}

void tar_writefork (fsize, macbinp)
register long fsize;
  short macbinp;
{
extern unsigned char inverse_table[];
extern char*hd_buffer;
extern long hd_buffer_size;
  long n_non_ASCII=0;
  short blocksz = (macbinp ? 128 : 512);
  long bytes_in_buffer=0;
  short next_chunk;
  char *data_pnt;
  register unsigned prev_char=0;
  long soppressi=0;

  pb.ioParam.ioPosMode = fsAtMark;
  pb.ioParam.ioBuffer = hd_buffer; /* attualmente non lo uso per altro, quindi...*/
  /*pb.ioParam.ioReqCount = blocksz;*/
  
  while (fsize>0) {
  	if(bytes_in_buffer<=0){
    	if (fsize < hd_buffer_size)
    		pb.ioParam.ioReqCount = fsize;
   		else
     		pb.ioParam.ioReqCount = hd_buffer_size;
		if (PBReadSync(&pb) || pb.ioParam.ioActCount != pb.ioParam.ioReqCount) {
		  short err = pb.ioParam.ioResult;
		  (void) PBCloseSync(&pb);
		  devo_chiudere_in=false;
		  pb.ioParam.ioResult = err;
		  pbsyserr(&pb);
		  }

		bytes_in_buffer=pb.ioParam.ioReqCount;
		data_pnt=hd_buffer;

	    if (!macbinp && !binary) {
	    	/* see macize_ASCII in suntar.c: Since the norm is NOT a MS-DOS
	    	file, I don't want to slow down the loop for something which is useful
	    	only for MS-DOS files, hence I have two loops, a fast one and a
	    	MS-DOS one */
			register unsigned char *cp = hd_buffer;
			register long len = bytes_in_buffer;
			register unsigned char c;

			do{
				if((c=*cp)==26 || c==LF && prev_char==CR){
					/* remove the char, and from now to the end copy bytes... */
					register char*dest=cp;
					cp++;
					do{
						if(c==26 || c==LF && prev_char==CR){
							/* remove the char */
							bytes_in_buffer--;
							fsize--;	/* fsize is the number of bytes
										not yet written to output */
							soppressi++;
							prev_char=c;
							}
						else{
							prev_char=inverse_table[c];
							if(c==CR)
								*dest++=LF;
							else
								*dest++=prev_char;
							if(prev_char >= 128 && prev_char==c)
	        					n_non_ASCII++;
	        				}
	        			c=*cp++;
						}
					while(--len);
					break;	/* obviously, exit from the external loop */
					}
				else{
					prev_char=inverse_table[c];
					if(c==CR)
						*cp=LF;
					else if(prev_char!=c)
						*cp=prev_char;
					else if(prev_char>= 128 )
	        			n_non_ASCII++;
	        		cp++;
	        		}
				}
			while(--len);
			}
		}
    if(fsize>=blocksz || fsize!=bytes_in_buffer){
     	next_chunk=min(bytes_in_buffer,blocksz);
   		fsize-=next_chunk;
    	}
    else{
    	if(!soppressi){
     		writeblock(data_pnt, bytes_in_buffer);

   			fillmem(data_pnt=hd_buffer,0,next_chunk=blocksz-(short)fsize);  /* il tar
    			per UNIX non lo fa, ma è talmente sporco non farlo
    			-- UNIX tar does not do that, but see in the caller a comment
    			which explains why I hate programmers who don't do that */
    		}
    	else
    		next_chunk=fsize;
    	fsize=0;
    	}
    writeblock(data_pnt, next_chunk);
	bytes_in_buffer -= next_chunk;
	data_pnt += next_chunk;
  }

  if(soppressi){
  	/* really, if the device is random-access, and this is the same volume which
  	contained the header, I could adjust the length in the header. Probably
  	I'll do that, but when I'll introduce a write BinHex (which has the same problem)
  	and beware, in that case maybe the previous fillmem... */

  	printf(in_Italia?"la conversione MS-DOS -> UNIX ha ridotto la dimensione di %ld bytes\n":
  			"MS-DOS -> UNIX conversion shortened the file size by %ld bytes\n",soppressi);

	if( /*!!!!(di.tipo_device==d_file || vol_numb_of_last_h == floppy_n) &&*/
		(di.tipo_device!=d_scsi || di.v.si.DeviceType!=TAPE_UNIT) ){
		/* device ad accesso casuale e volume non cambiato, quindi posso diminuire...*/

		leggi_settore(last_header,(char*)&tarh);
		if(compare_mem(ultimo_header,(char*)&tarh,sizeof(ultimo_header)) &&
		    (fsize=untar_number(bar_archive ? ((barh_type*)&tarh)->size:tarh.size,-1))!=-1 ){
		    if(bar_archive){
		    	fillmem(((barh_type*)&tarh)->size,0,8);
				numstr(((barh_type*)&tarh)->size, fsize-soppressi, 12);
				bar_checksum((barh_type*)&tarh);
				}
			else{
				numstr(&tarh.size, fsize-soppressi, 12);
				fill_checksum(&tarh);
				}
			soppressi=0;
			scrivi_settore(last_header,(char*)&tarh);
			check_wr_err();
			}
		}

	if( soppressi){
		fillmem(&tarh,0,512);
		while(soppressi>512){
			writeblock(&tarh, 512);
			soppressi-=512;
			}
		writeblock(&tarh, soppressi);
		}
	}

  if(macbinp==1)
	get_openfile_location(pb.ioParam.ioRefNum);

  if (PBCloseSync(&pb)) pbsyserr(&pb);
  devo_chiudere_in=false;
  if(n_non_ASCII != 0) printf(in_Italia?"Scritti %ld caratteri non ASCII\n":
  	"%ld non-ASCII characters were written\n",n_non_ASCII);
}

static short tarheader (fname, fsize, uname)
unsigned char *fname;	/* original Mac-like name. It may contain a part of pathname
						which must NOT be converted to UNIX */
char *uname;			/* buffer where the UNIX name must be placed */
  long fsize;
{
	char linkflag;
	long n_sectors=(fsize+511)/512;

	fillmem(&tarh, 0, 512);
	if(uname==NULL){	/* GNU tar 1.11, recursive call for the false header */
		uname=fname;	/* in that case, and only then it's a C string */
		linkflag='L';
		}
	else{
		mac_to_unix(uname, fname);
		linkflag = ((statb.ioFlAttrib & ioDirMask) ? '5' : '0');
		}
	if(!strcmp("./",uname) || !uname[0] ) return -1;
	if (statb.ioFlAttrib & ioDirMask)
		if (uname[strlen(uname)-1] != '/')
			strcat(uname, "/");
  	if ( use_long_paths && (tar_version==tar_singlevol || tar_version==tar_POSIX)){
		if(!split_name_prefix(uname))
   			nametoolong(fname);
   	  	}
  	else{
  		if(strlen(uname) >= 100){
  			if(use_long_paths && tar_version==tar_GNU){ /* GNU tar 1.11 ! */
  				tarheader("././@LongLink",strlen(uname)+1,NULL);	/* this is a special
  							case recognized by tarheader: uname=NULL and macname as
  							a C string rather than a Pascal string */
				fillmem(&tarh,0,512);
				strcpy(&tarh,uname);
 				writeblock(&tarh,512);
 				fillmem(&tarh,0,512);
				/* now, make uname shorter. I don't want to use the very bad solution
 				of GNU tar 1.11 (which simply truncates), but I don't want to call
 				split_POSIX_name (which may fail...) hence I do this: */
 				/* too simple (but still better than GNU tar 1.11)
  				uname[99]=0;
 				if(uname[98]=='/') uname[99]=0;
 				*/
				{char *p=strchr(&uname[strlen(uname)-99],'/');
 				if(p==NULL||p>=&uname[strlen(uname)-1])
 					strcpy(uname,&uname[strlen(uname)-99]);
 				else
 					strcpy(uname,p+1);
 				}
 				}
			else
  				nametoolong(fname);
  			}
		strcpy(tarh.name,uname);
  		}

	numstr(&tarh.mode, linkflag=='L' ? 0 : (long)((statb.ioFlAttrib & ioDirMask) ? dirmode : filemode), 8);
	numstr(&tarh.uid, my_uid, 8);
	numstr(&tarh.gid, my_gid, 8);
	numstr(&tarh.size, fsize, 12);
	numstr(&tarh.mtime, (long) statb.ioFlMdDat - unixtime, 12);
	tarh.linkflag = linkflag;
	fill_tmagic(&tarh);
	fill_checksum(&tarh);
	last_offset=0;
	questo_e_un_header=true;
	writeblock(&tarh, 512);
	last_header=last_sector-1;	/* mi serve il valore pre-incremento, ma eventuali
				header di volume devono essere contati... */

/* remember what you are going to write: it's useful for the 'M' continuation
header, and also for checks before clearing this same sector later if
I can't successfully save on disk the whole file
*/
	copia_ultimo_header(&tarh,last_sector);	/* valore post-incremento */
	if(di.tipo_device!=d_file) controlla_che_ci_stia((short)n_sectors);
	return 0;
}

static void fill_tmagic(t)
tarh_type *t;
{
	if(uname[0]||gname[0]){
		strcpy(t->magic, "ustar  ");
		strcpy(t->uname, uname);
		strcpy(t->gname, gname);
		}
	if(use_long_paths && (tar_version==tar_singlevol || tar_version==tar_POSIX)){
		static char tt[]={'u','s','t','a','r','\0','0','0'};
		mcopy(t->magic,tt,sizeof(tt));
		}
}

void fill_checksum(t)
register tarh_type *t;
{
	register long chksum = 0;
	register short i;
	fillmem(t->chksum, ' ', 8);
	for(i=0; i<512; ++i) chksum += (unsigned char)t->name[i];
	numstr(t->chksum, chksum, 8);
}

/* Don't allow absolute names.   "HD:foo:bar" becomes "./foo/bar" */
void mac_to_unix(uname, mname)
char *uname;
unsigned char*mname;
{
  register char *up = uname, *cp;
  register unsigned char c;
  register short namelen;
  char buffer[400];

if(alias_prefix[0] && alias_prefix[0]+mname[0]<256){
	pStrcpy(buffer,alias_prefix);
	my_p2cstr(buffer);
	if(buffer[alias_prefix[0]-1]==':'  && mname[0]>0 && mname[1]==':')
		/* alias_prefix always terminates by ':', but testing that does not harm */
		pascal_to_c(&buffer[alias_prefix[0]]-1,mname);
	else
		pascal_to_c(&buffer[alias_prefix[0]],mname);
	}
else
	pascal_to_c(buffer,mname);
mname=buffer;

  if (cp = strchr(mname, ':')) {
  #ifdef DOT_SLASH_NAME
  	*up++ = '.', *up++ = '/';
  #endif
  	 ++cp;
  	 }
  else
  	cp = mname;

  for(;;) {
	while (*cp == ':') *up++ = '.', *up++ = '.', *up++ = '/', ++cp;
	namelen=0;
	while (*cp && *cp != ':'){
		/* modified by Speranza to handle in some way non-ASCII codes 128 and upper,
		and characters which the UNIX shell uses.
		*/
		c=*cp++;

		if(pres_all_chars)
			;	/* skip following operations */
		else if(c>=127){
			static char s1[]="ÄÖäößÆæŒœ™";
			static char s2[]="©ØøÀÃÕÿƒ";
			char *p;
			if((p=strchr(s1,c))){	/* special characters with a two-character transliteration */
				if(!trunc_14 || namelen<14)
					*up++="AOaosAaOot"[p-s1];
				namelen++;
				c=    "eeeeseeeem"[p-s1];
				}
			else if(c>=128 && c<160)	/* remove accents, tilde etc. */
				c= "AACENOUaaaaaaceeeeiiiinooooouuuu"[c-128];
			else if((p=strchr(s2,c)))
				c= "cOoAAOyf"[p-s2];
			else if( (c==208 || c==209) && namelen!=0)
				c='-';
			else if(c==202&&!suppress_shell_chars)	/* non-breakable space */
				c=' ';
			else
				c='\0';
			}
		else if(suppress_shell_chars){
				/* many characters have a special meaning to the UNIX shell, but some
				only when used as first character of an identifier.
				Really we're not so expert of UNIX to know the better thing to do with
				every character under every shell and/or file utility: the rule which
				is often given is "use only letters, digits, underscore and dot" but
				a few other characters are usually good */
			if(c==' ')
				c='_';
			else if(strchr(";|^*?\'\\`\"!$@<>&()[],",c) )
				c= '\0';
			else if(namelen==0 && strchr("~%-.#",c) )
				c= '\0';
			}
		if (c<' ' && !pres_all_chars ){
			#define REPLACER '+'
			if(namelen!=0 && (*(up-1)==REPLACER||*(up-1)=='_' ) )
				;		/* suppress it ! */
			else{
				if(!trunc_14 || namelen<14) *up++ = REPLACER;
				}
			}
		else{
			if(c=='/') c=':';
			if(!trunc_14 || namelen<14) *up++ = c;
			}
		namelen++;
		}
	if (!*cp++) break;
	*up++ = '/';
  }
  *up='\0';
}

void numstr (p, num, count)
register char *p;
register long num;
register short count;
{
/* come fatto da Zacharias lasciava un sacco di '0' in testa, l'unico modo 
per non farlo è scrivere in un buffer e poi copiare
-- I (G. Speranza) don't like how Zacharias did it, he wrote directly to
the destination filling extra space at the beginning with '0's !  It's
also a risk for compatibility, since the UNIX tar and bar don't write 
things in that way (Well, the POSIX document specifies that there should
be '0's, and anyway tar uses a sscanf which does not worry).
*/
char buffer[14];
  buffer[--count] = '\0';
  if (count != 11) buffer[--count] = ' ';
  if(count!=0){
  	do{
		buffer[--count] = '0' + ((short)num & 7);
		num >>= 3;
		}
  	while (count && num);
	}
strcpy(p,&buffer[count]);
}

void statf (fname)
  unsigned char *fname;
{
  char name[256];

  pStrcpy(&name, fname);
  statb.ioNamePtr = name;
  statb.ioFVersNum = 0;
  statb.ioVRefNum = curr_vrefnum;
  statb.ioFDirIndex = 0;
  statb.ioDirID = 0;
  if (PBGetCatInfoSync(&statb) || statb.ioResult) pbsyserr(&statb);
  statb.ioNamePtr = NULL;
}

static short getvrefnum (fname)
unsigned char *fname;	/* Pascal string */
{
/* ho il sospetto che non serva più, se serve solo a sapere il vrefnum attuale...
no, a volte curr_vrefnum è 0, forse serve, e comunque sono poche istruzioni
*/
  HVolumeParam hpb;
  char name[256];

  pStrcpy(name, fname);
  hpb.ioNamePtr = name;
  hpb.ioVRefNum = 0;
  hpb.ioVolIndex = 0;
  if (PBHGetVInfoSync(&hpb) || hpb.ioResult) pbsyserr(&hpb);
  return hpb.ioVRefNum;
}

void nametoolong (name)
  char *name;
{
  printf("name too long \"%s\"\n", name);
  raise_error();
}
