/*******************************************************************************\

SCSI module

part of suntar, �1991-95 Sauro & Gabriele Speranza

This module is NOT public domain as the rest of suntar:
we're keeping the copyright, so you may:
a) recompile it (unmodified or with minor changes) and link to the rest of suntar
b) read it, to get suggestions
c) use small parts of it (no more than about 20 instructions)
For any other use, you MUST get our explicit permission

\*******************************************************************************/

/* prima o poi bisogner� spezzarlo in un pezzo ad alto livello (public domain)
e uno che fa le operazioni SCSI */


/* BEWARE !!!
It's quite useless to try reading this file without having the SCSI documents.
These are available in Internet from a number of sites:
SCSI17B: the last draft of the SCSI-1 standard (I think that the final
         version is copyrighted and can't be got for free, but it's
         available in printed form for a small price. From the same
         source, it's also available a printed copy of CCS, an agreement
         among some vendors about which "optional" features of SCSI should
         have been implemented by everybody. Anyway, the SCSI17B draft
         is quite enough)
SCSI-2 (any draft which is the latest at the moment)

*/


#include "suntar.h"
#include "windows.h"
#include <SCSI.h>
#include <string.h>

/*#define DEBUG to redirect SCSI read/write operations to a floppy. No more
used since I've got a SyQuest and have something on which to write... */
/*#define VERY_VERBOSE*/
/*#define CHEAT_TAPE*/


#define TUTTI_INSIEME
int verbosity;
Boolean ask_verbosity;
/*Boolean names_in_opendevice;*/
extern char opt_variable_blocks;
extern byte opt_force_mode_select;
extern short all_buff_size;
extern char* buffers_base;
long rewindTimeout,writeTimeout;


static char SENSE_MSG[]="error=%d, returned status=%d, returned message=%d\n";

#define PRINT_SENSE \
	if(err==noErr){			\
		if((returnedStat&stat_mask)==CHECK_CONDITION)		\
			do_sense(scsiID);								\
		else{												\
			sense_err_code=-2;								\
			if(verbosity>=1 && verbosity<3 && (err_code||returnedStat||returnedMessage))	\
				printf(SENSE_MSG,err_code,returnedStat,returnedMessage);	\
			}						\
		}							\
	else							\
		sense_err_code=-2;

#define PRINT_SENSE_POCO_VERBOSO \
	if(err==noErr){			\
		if((returnedStat&stat_mask)==CHECK_CONDITION)		\
			do_sense_poco_verboso(scsiID);								\
		else{												\
			sense_err_code=-2;								\
			if(verbosity>=1 && verbosity<3 && (err_code||returnedStat||returnedMessage))	\
				printf(SENSE_MSG,err_code,returnedStat,returnedMessage);	\
			}						\
		}							\
	else							\
		sense_err_code=-2;


#define CLEAR_RECOVERED_ERROR if(sense_err_code==0 &&(req_sense_response[0]&0x7E)==error_class_mask && (req_sense_response[2]&0xF)<=1) returnedStat=0;



/* alcuni dovrebbero andare nel .h !!! */

extern short tar_block_size;

short inquire_SCSI_device(short);
short do_scsi_command_in(short,byte*,char*,long,short*,short*,unsigned long*,unsigned long);
short do_scsi_command_out(short,byte*,char*,long,short*,short*,unsigned long);
void tape_rewind(short);
void tape_skip(short,short,long);
void tape_load_unload(short,unsigned char);
void do_scsi_inquiry(short,struct InquiryResponse *,short);
void do_write_filemarks(short,long);
void do_tape_erase(short,Boolean);
void do_request_sense(short,unsigned char*);
void do_scsi_generic(short,byte,char*,short);
void print_request_sense_response(unsigned char *);
void do_sense(short);
void do_sense_poco_verboso(short);
void print_sense(short);
OSErr fill_sense_data(void);
void fast_scsi_scan(struct fastscantype*);
void fill_scsi_list(char*,short,Boolean);
void skip_next_block(short);
int wait_for_rewind_complete(void);
short read_one_variable_block(short,char*,short );
short read_many_fixed_blocks(short,char*,short);
short read_one_fixed_block(short,char*);
void do_scsi_message(short,short);
void reposition_tape(void);
Boolean this_device_is_writable(void);

Boolean volume_in_place(short);

void do_mode_select(short,long);
void do_mode_sense(short,char*,short);


OSErr unmount_volume(short);
void EnableDisableItem(MenuHandle,short,Boolean);

#ifdef CHEAT_TAPE
#define do_read_block_limits(scsiID,buffer) err_code=0,*(long*)buffer=512,*(short*)&buffer[4]=512
#else
#define do_read_block_limits(scsiID,buffer) do_scsi_generic(scsiID,READ_BLOCK_LIMITS,buffer,6)
#endif
#define rezero_unit tape_rewind	/* same command has different meaning on non-tapes */
#define start_stop_unit tape_load_unload /* same command has different meaning on non-tapes */

#define TEST_UNIT_READY 0
#define REWIND 0x1	/* tape only */
#define REZERO_UNIT 0x1	/* anything except tapes */
#define REQUEST_SENSE 0x3
#define READ_BLOCK_LIMITS 0x5
#define SCSI_READ 0x8
#define SCSI_WRITE 0x0A
#define SCSI_SEEK 0x0B
#define WRITE_FILEMARKS 0x10
#define SCSI_SPACE 0x11
#define INQUIRY 0x12
#define MODE_SELECT 0x15
#define ERASE 0x19
#define MODE_SENSE 0x1A
#define LOAD_UNLOAD 0x1B
#define ALLOW_MEDIUM_REMOVAL 0x1E
#define READ_CAPACITY 0x25
#define SCSI_READ_10 0x28	/* as READ but with 10 byte command containing a 32-bit sector number */
#define SCSI_WRITE_10 0x2A
#define SCSI_SEEK_10 0x2B

/*#define SUPPORTED(l,command) (((l)>>(31-(command)))&1) � OK ma io la
uso quasi sempre con un command costante e l'espressione sotto � pi�
efficiente effettuando lo shift a compile time, e un compilatore intelligente
potrebbe anche rendersi conto che si pu� usare BTST */
#define SUPPORTED(l,command) ((l)&(1L<<(31-(command))))

#define stat_mask 0x1E
#define CHECK_CONDITION 0x2
#define STATUS_BUSY 0x8

#define error_class_mask 0x70

#define normalTimeout	4
#define skipBlockTimeout 300
#define deltaSkipTimeout 15
#define shortEraseTimeout 600
#define loadTimeout 1200
#define readVariableBlockTimeout 300	/* (the timer starts after the data was really */
						/* transferred, so I believe that normalTimeout should
						be good, but just in case...) */
#define readFixedBlockTimeout 300
#define extraTimeout 32		/* per il caso di lettura di n blocchi */

/*
controlErr e statusErr possono andar bene per comandi non esistenti,
se riesco a sapere che la causa � quella
-35   nsvErr          No such volume; volume not found
-36   ioErr           I/O error (bummers)
-43   fnfErr          File not found; Folder not found;
                      Edition container not found; Target not found
-44   wPrErr          Diskette is write protected; Volume is locked
                      through hardware in caso di write fallita, ma devo
                      fare indagini ulteriori per sapere che la causa era quella
                      (meglio non farle prima, per il caso che il device
                      dica di essere write protected ma non lo sia)
-56   nsDrvErr        no such drive (tried to mount a bad drive num) per ID not found...
-68   dataVerErr      read verify compare failed
-81   sectNFErr       sector number never found on a track

-470  scsiBadPBErr    invalid field(s) in the parameter block
-471  scsiOverrunErr  attempted to transfer too many bytes
-472  scsiTransferErr write flag conflicts with data transfer phase
-473  scsiBusTOErr    bus error during transfer (TO=timeout...)
-474  scsiSelectTOErr scsiSelTO exceeded (selection failed)
-475  scsiTimeOutErr  scsiReqTO exceeded
-476  scsiBusResetErr the bus was reset, so your request was aborted
-477  scsiBadStatus   non-zero (not "Good") status returned
-478  scsiNoStatusErr device did not go through a status phase
*/
#define scsiBadPBErr -470
#define scsiOverrunErr -471
#define scsiBusTOErr -473
#define scsiSelectTOErr -474
#define scsiBusResetErr -476
#define scsiBadStatus -477

#define tmRewind 1
#define tmLoad 2
#define tmUnload 3
/*#define tmVariableBlocks 4*/
#define tmEnd 4
#define tmSkipFile 5
#define tmSkipFileBack 6
#define tmWriteFilemark 7
#define tmRetension 8
#define tmShortErase 9
#define tmLongErase 10
#define tmReset 11
#define tmStartHere 12
#define tmPrintSense 13
#define tmTestUnitReady 14
#define tmModeSense 15
#define tmModeSelect 16
#define tmTestRead 17


#define rmAllowWriting 1
#define rmUnmount 2
#define rmMount 3
#define rmEject 4
#define rmRezero 5
#define rmReset 6
#define rmStart 7
#define rmStop 8
#define rmPrintSense 9

/* maschere per supported_ops */
#define skip_file_forward	 1
#define skip_file_backwards	 2
#define skip_block_forward	 4
#define skip_block_backwards 8
#define multiple_blocks		16
#define standard_variable_blocks 32
#define sense_0bytes		64	/* no more used */
#define write_filemark		128
#define use_software_fixed	256	/* no more used, replaced by "block stategy" */
#define short_erase			512
#define long_erase			1024
#define fast_scan_complete	4096 /* no more used, it crashed too often */
#define end_of_medium		2048
#define rewind_immediate	8192
#define inquire_all_luns	16384


#define kVendorIDSize 8
#define kProductIDSize 16
#define kRevisionSize 4
struct InquiryResponse{
	byte DeviceType; /*0*/
	byte DeviceQualifier; /*1*/
	byte Version; /*2*/
	byte ResponseFormat; /*3*/
	byte AdditionalLength; /*4*/
	byte VendorUse1; /*5*/
	byte Reserved1[2]; /*6-7*/
	char VendorID[kVendorIDSize]; /*8-15*/
	char ProductID[kProductIDSize]; /*16-31*/
	char Revision [kRevisionSize]; /*32-35*/
	byte VendorUse2[20]; /*36-55*/
	byte Reserved2[42]; /*56-97*/
	byte VendorUse3[158]; /*98-255*/
	};

struct fastscantype{
	short errCode;
	byte DeviceType;
	byte DeviceQualifier;
	char VendorID[kVendorIDSize];
	};


extern unsigned char *titoli_si_no[];
static Boolean printed_inquiry,printed_medium;
static short bytes_for_reqsen;
static unsigned char req_sense_response[32];
static short sense_err_code;
long supported_ops,unsupported_ops;
long supp_ops,unsupp_ops;	/* may be different from the previous values
	since here I mark as unsupported any command which caused troubles */


static char *tipi[]={"Hard disk","Tape unit","Printer","CPU unit",
	"WORM disk","CD-ROM device","Scanner","Optical device",
	"Jukebox unit","Network device","Prepress unit A",
	"Prepress unit B"};
static char str_unk[]="Unknown device",
	str_rem[]="Removable disk";
/* #define DEVTYPE_STILLVIDEO   	0x9f	*/	/* Still video drive ??? */

static unsigned char * titoli_ok_canc[]={"\pOk","\pAnnulla"};

static char BUS_RESET[]="The device locked the SCSI bus, which was reset\n";
static char UNLOCKED_BUS[]="The device locked the SCSI bus, unlocking operation successful\n";
static char DEVICE_RESET[]="The device locked the SCSI bus, the device (not the bus) was reset\n";
static char REQSENRESP[]="Request sense response:\n";
static char ERROR_IN_SENSE[]="Error in getting the sense code\n";
static char MEDIUM_TYPE[]="medium type %d\n";
static char WRITE_PROT[]="write protected: %s\n";
static char YES[]="yes",NO[]="no";
static char DENS_BLOCKS[]="density code %d blocks %ld ";
static char BLOCK_SIZE[]="block size %ld\n";
static char QUALIFIER[]=" qualifier: %d";
static char SET_UNSUPP_14[]="You should set unsupported op 14 to avoid this (see documentation)\n";
static char STR_NS[]="\n  %s\n";
static char STR_PAS[]=" (%P)";
static char BLOCK_NOT_MULTIPLE[]="block not multiple of 512 bytes !\n";
static char CANT_REWIND[]="Drive is not ready, can't rewind\n";
char rewind_proposal[]="\pThe tape head should be positioned before the first block of the archive.\rRewind the tape ?";

/******* the first routines are called by suntar for some events *******************/


MenuHandle deviceMenu;


void open_SCSI_device(ID)
short ID;
{
extern short floppy_buffer_size;
short inPlace;


verbosity=0;
/*is_wangdat=false;*/
sense_err_code=-2;
bytes_for_reqsen=-10000;	/* an impossible value */
invalid_buffers();	/* non si sa mai, anche se, insomma... */


#ifdef DEBUG
supported_ops=0;
unsupported_ops=0xF;
#endif
supp_ops=supported_ops;
unsupp_ops=unsupported_ops;

di.tipo_device=d_scsi;
di.drive_number=ID;
aggiorna_nome_aperto();


di.m_vrefnum=ID;	/* see write_scsi_random */
printed_inquiry=false;
printed_medium=false;
test_SCSI_device(&inPlace,true);

if(di.v.si.DeviceType==TAPE_UNIT){
	InsertMenu(deviceMenu = GetMenu(deviceID), 0);
	/*EnableItem(deviceMenu,tmVariableBlocks);*/
	if(verbosity>=1){
		AppendMenu(deviceMenu,"\pMODE SENSE");
		if(di.v.si.block_mode!=fixed_blocks){
			AppendMenu(deviceMenu,"\pMODE SELECT");
			AppendMenu(deviceMenu,"\pTest SCSI READ");
			}
		}

	DrawMenuBar();	/* durante il dialogo deve esserci..*/

	di.writing_allowed=true;
	/* now, I MUST have a large buffer: if there is memory I allocate one,
	otherwise at least warn the user */
	if( floppy_buffer_size==18 ){	/* cio� se � il valore di default, e non
								stato forzato qualcos'altro /
		/* normalmente a questo punto ho circa 30k, se ne vedo di pi� la utilizzo
		come buffer... */
		long free=MaxBlock();

		free&=~1023L;	/* multiplo intero di 1k...*/
		if(free>=50*1024L){	/* 50 K � circa ci� di cui ha bisogno suntar
								per non rischiare di stare stretto */
			free-=50*1024L;
			if(free>256*1024L){
				free-=(free-256*1024L)>>2;	/* non esageriamo, lascio un quarto
												di ci� che supera i 256k...*/
				free&=~1023L;
				}
			if((free>>9)>all_buff_size){
				if(verbosity>0) dp("Allocating a %ldK buffer\n",free>>10);
				reinit_buffering(free);
				}
			}
		if(all_buff_size<100){	/* 50 k... */
			ParamText("\pBeware: using a small buffer on a slow device is not recommended.\rGive me more memory !",PNS,PNS,PNS);
			my_alert();
			}
		}
	if(my_semimodal_dialog(155,NULL,0,rewind_proposal,NULL,NULL)==2){
		di.v.si.current_tape_block=0x7FFFFFFF;
		archive_rewind();
		}
	}
else{

	InsertMenu(deviceMenu = GetMenu(deviceID+1), 0);
	
	adjust_random_access_menu();
	DrawMenuBar();
	}
di.v.si.current_tape_block=0;
di.v.si.tape_block_number=0;

}


void adjust_random_access_menu()
{
	char buffer[100];
	short refnum;
	long space;
	if(ID_to_drive_number(di.drive_number,&refnum))
		di.mounting_state=unmountable;
	else{

		buffer[0]=0;
		if( (err_code=GetVInfo (refnum, buffer, &refnum,&space)) == nsvErr ) /* no such volume: not mounted */
			di.mounting_state=not_mounted;
		else{
			di.mounting_state=mounted;
			if(check_startup(refnum)) di.mounting_state=mounted_as_startup;
			}
		}
}

void do_end_of_write_for_tape(void);
void do_end_of_write_for_tape()
{
Boolean was_eof(void);
Boolean eof_in_buf=was_eof();

if(dirty_buffers())
	flush_buffers(2);

if(eof_in_buf &&!(unsupp_ops&write_filemark)){
	printf("Now writing a filemark\n");
	do_write_filemarks(di.drive_number,1);	/* che sia dirty_buffers
				non basta, devo sapere se ci vuole, cio� se l'ultima operazione
				era una azzera_settore: ho introdotto una variabile per questo,
				ma � statica di diskDriver e preferisco rimanga tale */
	di.v.si.current_tape_block=0;
	}
}


void close_SCSI_device()
{
/* special actions to be performed, e.g. flushing the buffer (for random-access
device that's not needed, since suntar always flushes it in idle times, but
it's always better to do that anyway, and sequential-access tapes need
writing in large blocks and do not allow to overwrite an "end of archive"
sector by one more file header, hence they may not be flushed in idle times) */

if(di.v.si.DeviceType==TAPE_UNIT){
	do_end_of_write_for_tape();
	
	DelMenuItem (deviceMenu, tmTestRead);
	DelMenuItem (deviceMenu, tmModeSelect);
	DelMenuItem (deviceMenu, tmModeSense);
	}
else
	flush_buffers(2);
DeleteMenu (deviceID);
DeleteMenu (deviceID+1);
DrawMenuBar();
}

void adjust_mode_select(void);
void adjust_mode_select()
{
if(opt_force_mode_select){
	if(di.v.si.block_mode==variable_blocks || di.v.si.block_mode==software_fixed_blocks)
		do_mode_select(di.drive_number,0);
	else if(di.v.si.block_mode==firmware_fixed_blocks)
		do_mode_select(di.drive_number,di.v.si.block_size);
	}
}

void start_appending()
{
short l;
sector_t inizio_blocco_buono;

inizio_blocco_buono=get_first(&l);
err_code=0;
if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT){
	if(!(unsupp_ops&skip_block_backwards) && ((supp_ops&skip_block_backwards) ||
	   di.v.si.supported_group0==0 || SUPPORTED(di.v.si.supported_group0,SCSI_SPACE)) ){
		tape_skip(di.drive_number,0,-1);

		if(err_code) printf("Failed to backspace one block, now rewinding and repositioning\n");
		}
	else{
		printf("Now rewinding and repositioning the tape one block backwards\n");
		err_code=-1;
		}
	if(!err_code)
		di.v.si.current_tape_block -= di.v.si.block_size>>9;
	else{
		short n_read=0;
		long old_block_n=di.v.si.tape_block_number;
		if(di.v.si.block_mode==fixed_blocks)
			old_block_n=inizio_blocco_buono / (di.v.si.block_size>>9);
		else if(di.v.si.block_mode==software_fixed_blocks)
			old_block_n -= l / (di.v.si.block_size>>9);	/* the number of blocks
									currently in the buffer */
		else	/* variable_blocks */
			old_block_n--;	/* the buffer contains one block... */
		/*dp("ero posizionato a %ld (%ld)\n",inizio_blocco_buono,old_block_n);*/

		if(archive_rewind()) raise_error();
		/* se space supportato, eventualmente prova a farlo in avanti */
		#if 0
		while(di.v.si.current_tape_block+n_read<=inizio_blocco_buono){
			sector_t l=di.v.si.current_tape_block;
			n_read=read_scsi_tape(di.drive_number,&l,buffers_base,all_buff_size);
			if(err_code){
				invalid_buffers();
				return;
				}
			}
		if(di.v.si.current_tape_block!=inizio_blocco_buono)
			error_message("Could not reposition\n");
		else
			dp("posizionato a %ld:%ld\n",di.v.si.current_tape_block,di.v.si.current_tape_block+n_read-1);
		#endif

		if(old_block_n){
			if(!(unsupp_ops&skip_block_forward) && ((supp_ops&skip_block_forward) ||
	  		 di.v.si.supported_group0==0 || SUPPORTED(di.v.si.supported_group0,SCSI_SPACE)) ){
				tape_skip(di.drive_number,0,old_block_n);
				if(err_code&&expert_mode) printf("Fast forward movement failed\n");
				}
			else{
				while(old_block_n--)
					skip_next_block(di.drive_number);
				}
			if(!err_code){
				di.v.si.current_tape_block=inizio_blocco_buono;
				di.v.si.tape_block_number=old_block_n;
				}
			}
		else
			err_code=0;
		}
	}
}

int archive_rewind()
/* used for extract selected files, but also in other cases */
{
err_code=0;

if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT){
	if(di.v.si.current_tape_block==0) return;
	invalid_buffers();
	if(!(unsupp_ops&skip_file_backwards) && ((supp_ops&skip_file_backwards) ||
	   di.v.si.supported_group0==0 || SUPPORTED(di.v.si.supported_group0,SCSI_SPACE)) ){
		invalid_buffers();
		tape_skip(di.drive_number,1,-1);
		if(err_code==0){
			tape_skip(di.drive_number,1,1);	/* I'm now positioned before the filemark,
				I must go after it. It never happens that a tape supports backwards
				SPACE without supporting forward SPACE */
			err_code=0;
			}
		else if(err_code==eofErr)
			err_code=0;	/* beginning of medium, va benissimo cos�, anzi sar� la normalit� */
		else{
			do_scsi_generic(di.drive_number,TEST_UNIT_READY,NULL,0);
			if(err_code==1){
				printf(CANT_REWIND);
				return -1;
				}
			printf("Skip file backwards failed, trying rewind\n");
			}
		}
	else{
		do_scsi_generic(di.drive_number,TEST_UNIT_READY,NULL,0);
		if(err_code==1){
			printf(CANT_REWIND);
			return -1;
			}
		err_code=-1;
		}
	if(err_code){
		tape_rewind(di.drive_number);
		if(err_code) /* e aspettare che dica di avere finito ? */
			tape_rewind(di.drive_number);
		if(err_code)
			tape_rewind(di.drive_number);
		if(err_code) printf("Can't be sure that the tape was rewound, trying anyway\n");
		}
	return wait_for_rewind_complete();
	}
return 0;
}

static int wait_for_rewind_complete()
{
enum tipo_fase oldfase;
jmp_buf savebuf;

	err_code=0;
	/*SetCursor(&waitCursor);*/
	oldfase=fase;
	fase=reading_disk;

	mcopy((char*)savebuf,(char*)main_loop,sizeof(jmp_buf));
	if(setjmp(main_loop)<0){ 	/* l'unico longjmp possibile � un comando Abort */
		fase=oldfase;
		di.v.si.current_tape_block = 0x7FFFFFFF;
		mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));

		return -1;
		}
	do{
		check_events(0);
		do_scsi_generic(di.drive_number,TEST_UNIT_READY,NULL,0);
		/* 0 disco c'� 1 disco non c'� (o occupato...) <0 errore */
		/*dp("err code=%d\n",err_code);*/
		}
	while(err_code==1 );
	/*SetCursor(&arrow);*/

	fase=oldfase;
	di.v.si.current_tape_block =0;
	di.v.si.tape_block_number=0;
	mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
	return 0;
}

void do_sense(scsiID)
short scsiID;
{
short olderrc=err_code;
do_request_sense(scsiID,req_sense_response);
sense_err_code=err_code;

if(verbosity>=1 || !sense_err_code &&
  (req_sense_response[0]&0x7E)==error_class_mask && ((req_sense_response[2]&0xF) ||
  req_sense_response[12]||req_sense_response[13]) ){
	if(!sense_err_code){
		if(verbosity>=1) printf(REQSENRESP);
		print_request_sense_response(req_sense_response);
		vai_a_capo();
		}
	else
		printf(ERROR_IN_SENSE);
	}
err_code=olderrc;
}

void do_sense_poco_verboso(scsiID)
/* as above but prints fewer things */
short scsiID;
{
short olderrc=err_code;
do_request_sense(scsiID,req_sense_response);
sense_err_code=err_code;

if(verbosity>=1 ){
	if(!sense_err_code){
		printf(REQSENRESP);
		print_request_sense_response(req_sense_response);
		vai_a_capo();
		}
	else
		printf(ERROR_IN_SENSE);
	}
err_code=olderrc;
}


void print_sense(scsiID)
short scsiID;
{
one_empty_line();
if(sense_err_code){
	do_request_sense(scsiID,req_sense_response);
	sense_err_code=err_code;
	}

if(!sense_err_code){
	printf(REQSENRESP);
	print_request_sense_response(req_sense_response);
	vai_a_capo();
	}
else
	printf(ERROR_IN_SENSE);
sense_err_code=-2;	/* this value is tested to avoid printing twice */
}


static OSErr fill_sense_data()
{
short olderrc=err_code;
do_request_sense(di.drive_number,req_sense_response);
if(err_code){
	olderrc=err_code;
	return ioErr;
	}
else{
	olderrc=err_code;
	return 0;
	}
}

void test_tape_EOF(void);
void test_tape_EOF()
/* read_scsi_tape often returns err_code -1 o ioErr, in such cases I call this
routine to see whether it was an EOF instead... */
{
if(! sense_err_code && (req_sense_response[0]&0x7E)==0x70 &&
	(req_sense_response[2]&0xC0) ) /* EOM or filemark */
	err_code=eofErr;
}


OSErr test_SCSI_device(inPlace,verbose)
short *inPlace,verbose;
{
/* it's called when opening, but also every time an extract or create archive command
is issued (the device is tested again to be sure to have the latest informations,
the medium might have been swapped) */
char my_buffer[100];

*inPlace=1;
di.os=unknown_os;

if(verbose &&!printed_inquiry){
	printed_inquiry=true;
	if(inquire_SCSI_device(di.drive_number)){	/* it assigns some fields of di,
		but none of them may be altered by the insertion of a removable disk
		hence that need not be repeated */
		do_request_sense(di.drive_number,(unsigned char*)my_buffer);	/* according to SCSI-1 a
							device may have no INQUIRY, but it can't have no REQUEST SENSE */
		if(err_code){
			printf("No device is responding\n");
			return err_code;
			}
		}
	}
else
	do_request_sense(di.drive_number,(unsigned char*)my_buffer);	/* in order to bring
		the device out of a possible UNIT ATTENTION, inquire_SCSI_device already has
		such an instruction hence I do that only if that was not called */

#ifdef VERY_VERBOSE
if(verbose) printf("\nTEST UNIT READY test\n");
#endif

do_scsi_generic(di.drive_number,TEST_UNIT_READY,NULL,0);
		/* 0 disco c'� 1 disco non c'� (o occupato...) <0 errore */
if(!printed_medium && err_code==0) verbose=true;
if(verbose) printf(err_code==1 ? "Unit not ready\n":err_code==0?"Unit ready\n":"Error in unit ready test!\n");
*inPlace=err_code<=0;	/* in case of error, it's better (less irritating to the user)
	to try to use a non-ready device than refuse to work on something that is ok */

if(verbose){	/* that is the first time, when opening the device */
	di.sectors_on_floppy=0x7FFFFFFF;	/* infinite */
	di.v.si.block_mode=fixed_blocks;	/* also for hard disks, it helps me to skip
										some operations */
	di.v.si.block_size=512;
	}
#ifdef DEBUG
di.v.si.DeviceType=TAPE_UNIT;
printf("imbroglio\n");
#endif

if(di.v.si.DeviceType==TAPE_UNIT){
	/*
	 A tape may work in fixed size mode or variable block mode. This is
	what I've understood about that.
	 If a tape unit supports only fixed size, then READ BLOCK LIMITS
	returns two equal numbers. Trying to use that unit in variable size
	mode probably would yield an error, and anyway it is not a good idea,
	since fixed-sized mode appears more powerful to a program conceived for
	random-access devices.
	 If the unit supports both fixed and variable, or variable only, there is
	only one way to know whether the current tape was written in variable or
	fixed blocks, and their sizes: read all the blocks. Hence, it's better to
	set the tape in variable blocks mode and start reading, since blocks written
	as fixed-size are readable also in variable size.
	 For tapes, READ CAPACITY is not provided.
	  About MODE SENSE and MODE SELECT, for a while I thought they were useless,
	but then I saw a Exabyte unit which works in this way:
	when a MODE SELECT with size n is performed, the unit works in emulation
	of a unit with fixed blocks of size n: but the controller does NOT handle
	the real block size and concatenate/break data in order to obtain
	data blocks of the requested size, rather it considers an error that
	the "fixed" size is not the size of the blocks on the tape. According to
	the standard, a READ in variable mode is rejected as an "ILLEGAL REQUEST".
	And this is the default mode at startup, at least on the Exabyte.
	If n is 0, the unit operates in variable blocks.
	MODE SENSE always returns the value of the last MODE SELECT (obviously,
	just after reset there is a preset value chosen by the manufacturer)
	*/
	char old_block_mode=di.v.si.block_mode;

	do_read_block_limits(di.drive_number,my_buffer);
	my_buffer[0]=0;	/* 24 -> 32 bit conversion */

	if(!err_code){
		printed_medium=true;
		if( verbose && *(unsigned short*)&my_buffer[4]>512){
			reinit_buffering(*(unsigned short*)&my_buffer[4]);	/* because that routine never
			decreases the buffer size, and it's called again before returning,
			its only purpose is to increase the buffer if it's too small for the
			minimum block size, a very rare event anyway */
			}
		}
	if(!err_code && *(short*)&my_buffer[4] && *(short*)&my_buffer[4] == *(long*)my_buffer){
		/* nonzero min size==max size => fixed blocks */
		di.v.si.block_mode=fixed_blocks;
		di.v.si.block_size= *(long*)my_buffer;
		if(verbose){	/* the first tme, while executing the Open device */
			reinit_buffering(di.v.si.block_size);
			}
		}
	else{	/* including an error (the Apple tape unit supports READ BLOCK LIMITS
			but refuses to return any byte in the Data in phase�) */
		if(verbose){	/* that is the first time, when called by "open device" */
			if( (opt_variable_blocks=='y'||opt_variable_blocks=='v') ){
				short nb;
				/* compute a reasonable buffer size: not very useful since a lot of
				other places compute a buffer size, and the largest is always the winner */
				di.v.si.block_size= *(short*)&my_buffer[4];
				if(err_code || di.v.si.block_size<512) di.v.si.block_size=512;
				nb=tar_block_size/(short)(di.v.si.block_size>>9);
				if(nb==0) nb=1;
				di.v.si.block_size *= nb;

				di.v.si.block_mode=variable_blocks;

				if(opt_force_mode_select) do_mode_select(di.drive_number,0);
				}
			else if(opt_variable_blocks=='f' ){

				di.v.si.block_mode=firmware_fixed_blocks;

				di.v.si.block_size=(long)tar_block_size<<9;
				if(unsupported_ops&standard_variable_blocks){
					do_mode_sense(di.drive_number,&my_buffer[0],0);
					my_buffer[4]=0;
					if(!err_code && *(long*)&my_buffer[4]!=0 && !(*(long*)&my_buffer[4]&0x1FF) )
						di.v.si.block_size=*(long*)&my_buffer[4];
					}
				if(opt_force_mode_select) do_mode_select(di.drive_number,di.v.si.block_size);
				}
			else{
				di.v.si.block_mode=software_fixed_blocks;

				di.v.si.block_size=(long)tar_block_size<<9;

				if(unsupported_ops&standard_variable_blocks){
					do_mode_sense(di.drive_number,&my_buffer[0],0);
					my_buffer[4]=0;
					if(!err_code && *(long*)&my_buffer[4]!=0 && !(*(long*)&my_buffer[4]&0x1FF) )
						di.v.si.block_size=*(long*)&my_buffer[4];
					}
				if(opt_force_mode_select) do_mode_select(di.drive_number,0);
				}
			reinit_buffering(di.v.si.block_size);	/* at least the default size for a tar archive... */
			}
		}
	if(verbose || di.v.si.block_mode!=old_block_mode || verbosity>=2){
		static char * nomi_strat[]={"really variable","fixed","software fixed","firmware fixed"};
		one_empty_line();
		printf("Block strategy: %s blocks",nomi_strat[di.v.si.block_mode]);
		if(di.v.si.block_mode!=variable_blocks)
			printf(" (%ld bytes)",di.v.si.block_size);
		printf("\n\n");
		}
	}
else{	/* anything else is considered a direct-access device. Strange error
		messages will be given after trying to open a printer or a scanner, but
		not forbidding strange (or unknown) devices allows the program to work
		with more kinds of devices */
	char *block_ptr;
	fillmem(my_buffer,0,8);
	do_scsi_generic(di.drive_number,READ_CAPACITY,my_buffer,8);
	if(!err_code){
		printed_medium=true;
		if(verbose ){
			if(*(long*)&my_buffer[4]>=512) di.v.si.block_size= *(long*)&my_buffer[4];
			if(*(long*)&my_buffer[0]) di.sectors_on_floppy= (*(long*)&my_buffer[0] +1) * (di.v.si.block_size>>9);
			}
		}
	else{	/* if read capacity fails, try MODE SENSE: for hard disks it should return
			the same numbers */
		do_mode_sense(di.drive_number,&my_buffer[0],0);

		if(!err_code){
			my_buffer[8]=0;
			if(verbose){
				if(*(long*)&my_buffer[8]>=512) di.v.si.block_size= *(long*)&my_buffer[8];
				my_buffer[4]=0;
				if(*(long*)&my_buffer[4]) di.sectors_on_floppy= *(long*)&my_buffer[4] *
					(di.v.si.block_size>>9);
				}
			}
		}

	block_ptr=NewPtr(di.v.si.block_size);
	di.os=unknown_os;
	if(block_ptr!=NULL){
		char*sect_p;
		sector_t in_buffer=0;
		read_scsi_random(di.drive_number,0,block_ptr,di.v.si.block_size>>9);
		if(!err_code){
			static char nampm[]= "Not a Macintosh partition map\n";
			printed_medium=true;
			one_empty_line();
			if(verbose) printf("Partition map test\n");
	
			if(!err_code && *(short*)block_ptr=='ER'){
				if(verbose)printf("Disk in Apple format block size=%u number of blocks=%ld\n",
					*(short*)&block_ptr[2],*(long*)&block_ptr[4]);
				di.os=mac_HFS;
				}
			else if(!err_code && check_all_zero(block_ptr) ){
					/* it should be illegal, but I've seen a number of CD-ROMs (some produced by Apple!)
					which have a null sector 0 followed by a valid partition map...*/
				if(di.v.si.block_size==512){
					read_scsi_random(di.drive_number,1,block_ptr,1);
					sect_p=block_ptr;
					if(!err_code) in_buffer=1;
					}
				else
					sect_p=block_ptr+512;
				if(!err_code&&*(short*)sect_p=='PM'){
					di.os=mac_HFS;
					if(verbose) printf("No header data in sector 0\n");
					}
				else if(verbose)
					printf(nampm);
				}
			else if(verbose)
				printf(nampm);
			if(verbose&&di.os==mac_HFS){
				sector_t sn;
				short i;
				sn=0;
				i=0;
				for(;;){
					if(++sn>=in_buffer+(di.v.si.block_size>>9)){
						read_scsi_random(di.drive_number,sn,block_ptr,di.v.si.block_size>>9);
						if(err_code ) break;
						in_buffer=sn;
						}
					sect_p=block_ptr+((long)(sn-in_buffer)<<9);
	
					if(*(short*)sect_p=='TS'){	/* old partition map, documented in IM vol 4 */
						long* pt=(long*)&sect_p[2];
						while(pt[0]){
							printf("old-format partition start=%ld size=%ld\n",pt[0],pt[1]);
							pt+=3;
							i=1;
							}
						break;
						}
					if(*(short*)sect_p!='PM')
					break;
					/* new format, see IM vol V */
					i=1;
					printf("partition name=\"%.32s\" type=\"%.32s\" start=%ld size=%ld\n",
						&sect_p[16],&sect_p[48],*(long*)&sect_p[8],
						*(long*)&sect_p[12]);
					}
				if(!i) printf("Unknown partition map format\n");
				}
			}
		DisposPtr(block_ptr);
		}
	reinit_buffering(max(512,(long)di.v.si.block_size));
	}

#ifdef DEBUG
printf("block size=%ld\n",di.v.si.block_size);
if(verbose || di.v.si.block_mode!=software_fixed_blocks){
	printf("imbroglio, lo considero un tape unit\n");
	di.v.si.DeviceType=TAPE_UNIT;
	return noErr;
	}
#endif

#if 0	/* esperimento */
	{ char scsiCMD[10];
	short err,returnedStat,returnedMessage;
	fillmem(scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0] = 0x40;	/* change definition: my SyQuest drive locks the bus
						when I send this command, so this is a way to test
						the bus unlocking method */
	scsiCMD[3] = 2;
	err=do_scsi_command_in(di.drive_number,&scsiCMD,NULL,0,&returnedStat,
		&returnedMessage,NULL,normalTimeout);
	do_sense(di.drive_number);

	printf("errs=%d %d %d %d\n",err,err_code,returnedStat,returnedMessage);
	}
#endif

/*do_mode_select(1024); test, my CD-ROM drive accepts it */

return noErr;
}

static void do_mode_select(scsiID,new_block_size)
short scsiID;
long new_block_size;
/* experiment: a CD-ROM drive supports a number of block sizes
*/
{

	byte scsiCMD[12];
	short s,err,returnedStat,returnedMessage,i;
	byte my_buffer[100];

	do_mode_sense(scsiID,(char*)&my_buffer[0],0);
	if(err_code){
		if(verbosity) printf("MODE SENSE failed, MODE SELECT not performed\n");
		return; /* it's too dangerous to continue...*/
		}

	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));

	scsiCMD[0]=MODE_SELECT;
	scsiCMD[4]=12;

	my_buffer[0]=my_buffer[2]=0;	/* other bytes are left unchanged from the MODE SENSE */
	my_buffer[1]=0;

	fillmem((char*)&my_buffer[5],0,3);	/* I suppose that's the correct thing to do,
		and anyway the Exabyte issues an error if I do't do that */
	*(long*)&my_buffer[8]= new_block_size;

	err=do_scsi_command_out(scsiID,scsiCMD,(char*)my_buffer,12,&returnedStat,&returnedMessage,normalTimeout);

	PRINT_SENSE
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;

#if 0
	do_mode_sense(scsiID,&my_buffer[0],0);
	printf("err=%d\n",err_code);

	printf("block descriptors size=%d\n",my_buffer[3]);

	printf(MEDIUM_TYPE,(unsigned char)my_buffer[1]);
	printf(WRITE_PROT,(my_buffer[2]&0x80) ? YES : NO);

	for(i=0;i<my_buffer[3];i+=8){
		long l=0;
		mcopy(((char*)&l)+1,&my_buffer[5+i],3);
		printf(DENS_BLOCKS,my_buffer[4+i],l);
		l=0;
		mcopy(((char*)&l)+1,&my_buffer[9+i],3);
		printf(BLOCK_SIZE,l);
		}
	enable_autoflush();
#endif

}

Boolean this_device_is_writable()
{
char my_buffer[100];
if(di.v.si.DeviceType==5)	/* CD-ROM */
	return false;
do_mode_sense(di.drive_number,&my_buffer[0],0);
if(err_code) return true;	/* non vale la pena di considerarlo un errore fatale
							-- Having got no information, let the caller continue and try to write */
if(my_buffer[2]&0x80) return false;	/* write protected */
return true;
}


void sistema_items_device_menu_scsi(void);
void sistema_items_device_menu_scsi()
{
short i;
if(di.v.si.DeviceType==TAPE_UNIT){
	if(fase==non_faccio_nulla){
		EnableItem(deviceMenu,tmRewind);
		EnableItem(deviceMenu,tmLoad);
		EnableItem(deviceMenu,tmUnload);
		EnableItem(deviceMenu,tmSkipFile);
		EnableItem(deviceMenu,tmRetension);
		EnableItem(deviceMenu,tmReset);
		EnableItem(deviceMenu,tmStartHere);
		EnableDisableItem(deviceMenu,tmSkipFileBack,!(unsupported_ops&skip_file_backwards));
		EnableDisableItem(deviceMenu,tmWriteFilemark,!(unsupported_ops&write_filemark));
		EnableDisableItem(deviceMenu,tmShortErase,!(unsupported_ops&short_erase));
		EnableDisableItem(deviceMenu,tmLongErase,!(unsupported_ops&long_erase));
		EnableDisableItem(deviceMenu,tmEnd,!(unsupported_ops&end_of_medium));
		if(verbosity>=1 && di.v.si.block_mode!=fixed_blocks){
			EnableItem(deviceMenu,tmModeSelect);
			EnableItem(deviceMenu,tmTestRead);
			}
		}
	else{
		for(i=tmRewind;i<=tmStartHere;i++)
			DisableItem(deviceMenu,i);
		if(verbosity>=1 && di.v.si.block_mode!=fixed_blocks){
			DisableItem(deviceMenu,tmModeSelect);
			DisableItem(deviceMenu,tmTestRead);
			}
		}
	}
else{	/* direct access */
	if(fase!=non_faccio_nulla){
		for(i=rmUnmount;i<rmStop;i++)
			DisableItem(deviceMenu,i);
		}
	else{
			/* Rezero, Reset and Stop are dangerous if performed on a mounted
			volume, stopping the motor may freeze the Mac since the Finder
			does not understand what's happened and retries getting only error
			messages... Rezeroing either does nothing or may change important
			settings (e.g. the number of bytes per logical sectors...) which
			again will freeze the Finder */
		EnableDisableItem(deviceMenu,rmRezero,di.mounting_state<mounted);
		EnableDisableItem(deviceMenu,rmReset,di.mounting_state<mounted);
		EnableDisableItem(deviceMenu,rmStop,di.mounting_state<mounted);
		EnableItem(deviceMenu,rmStart);

		EnableDisableItem(deviceMenu,rmUnmount,di.mounting_state==mounted);	/* and this is nonsense on the
												startup volume...*/
		EnableDisableItem(deviceMenu,rmMount,di.mounting_state==not_mounted);
		EnableDisableItem(deviceMenu,rmEject,di.isEjectable && di.mounting_state!=mounted_as_startup );
		}
	EnableDisableItem(deviceMenu,rmAllowWriting,!di.writing_allowed && di.mounting_state!=mounted_as_startup);
	}
}	




void handle_device_menu(item)
short item;
{
Boolean warning_writing_dangers(void);
static char does_not_supp[]="This unit does not support this operation\n";
err_code=0;

if(di.v.si.DeviceType==TAPE_UNIT)
	switch(item){
	case tmRewind:	/* rewind */
		flush_buffers(2);
		tape_rewind(di.drive_number);
		if(!err_code){
			di.v.si.current_tape_block=0;
			di.v.si.tape_block_number=0;
			}
		invalid_buffers();
		break;
	case tmLoad:	/* load */
		tape_load_unload(di.drive_number,1);
		break;
	case tmUnload: /* unload */
		flush_buffers(2);
		invalid_buffers();
		tape_load_unload(di.drive_number,0);
		di.v.si.current_tape_block=0x7FFFFFFF;
		break;
	case tmEnd: /* end of medium */
		flush_buffers(2);
		invalid_buffers();
		tape_skip(di.drive_number,3,1);
		di.v.si.current_tape_block=0x7FFFFFFF;
		break;
	case tmSkipFile: /* skip filemark */
		if(di.v.si.supported_group0!=0 && !SUPPORTED(di.v.si.supported_group0,SCSI_SPACE) ||
		   (unsupp_ops&skip_file_forward) ){
			/* if the SPACE command can't be used, skip the file by a number
			of READ commands */
			Boolean filemark;
			enum tipo_fase oldfase;
			jmp_buf savebuf;
			invalid_buffers();
			di.v.si.current_tape_block=0;
			oldfase=fase;
			mcopy((char*)savebuf,(char*)main_loop,sizeof(jmp_buf));

			DisableItem(deviceMenu,0);
			if(setjmp(main_loop)<0){
				fase=oldfase;
				mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
				EnableItem(deviceMenu,0);
				DrawMenuBar();	/* per il caso che qualcun altro l'abbia chiamata nel frattempo...*/
				di.v.si.current_tape_block=0;
				break;
				}
			fase=reading_disk;
			do{
				long first=di.v.si.current_tape_block;
				check_events(0);
				if(!read_scsi_tape(di.drive_number,&first,buffers_base,all_buff_size)
					&& !err_code) err_code=ioErr;
				do_sense(di.drive_number);
				filemark=sense_err_code==0 && (req_sense_response[0]&0x7E)==0x70 &&
					(req_sense_response[2]&0xC0);	/* filemark oppure end of medium */
				}
			while(!err_code && ! filemark);
			if(!filemark) printf("Filemark not reached !\n");
			di.v.si.current_tape_block=0;
			fase=oldfase;
			mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
			EnableItem(deviceMenu,0);
			DrawMenuBar();

			if(err_code==eofErr) err_code=0;	/* perch� � questo che cercavo */
			}
		else{
			flush_buffers(2);
			invalid_buffers();
			tape_skip(di.drive_number,1,1);
			di.v.si.current_tape_block=0;
			if(err_code==eofErr){
				printf("End of medium reached before finding a filemark\n");
				flush_console();
				return;
				}
			}
		break;
	case tmSkipFileBack: /* skip filemark backwards */
		if(di.v.si.supported_group0!=0 && !SUPPORTED(di.v.si.supported_group0,SCSI_SPACE) ||
		   (unsupp_ops&skip_file_backwards) )
			printf(does_not_supp);
		else{
			flush_buffers(2);
			invalid_buffers();
			tape_skip(di.drive_number,1,-1);
			if(err_code==eofErr){
				printf("Beginning of medium reached before finding a filemark\n");
				err_code=0;
				}
			else if(!err_code){
				/* e ora bisogna portarsi dopo il filemark, mentre questo
				comando porta la testina prima di esso
				 "A negative value -N in the count field shall cause reverse medium
				movement over N blocks or filemarks ending on the beginning-of-medium
				side of the last block or filemark" (SCSI-1) */
				tape_skip(di.drive_number,1,1);
				err_code=0;
				}
			if(!err_code){
				di.v.si.current_tape_block=0;
				di.v.si.tape_block_number=0;
				}
			}
		break;
	case tmRetension:	/* retension */
		flush_buffers(2);
		invalid_buffers();
		tape_load_unload(di.drive_number,2);	/* unload & retension, however it's
					Eject on all direct-access devices and also a number of tape units
					give it this meaning */
		break;
	case tmWriteFilemark: /* write filemark */
		if(!(unsupported_ops&write_filemark)){
			ParamText("\pDo you really want to write a filemark ?",PNS,PNS,PNS);
			if(my_modal_dialog(130,titoli_si_no,2)==1){
				flush_buffers(2);
				do_write_filemarks(di.drive_number,1);
				if(!err_code) di.v.si.current_tape_block=0;
				}
			}
		else
			printf(does_not_supp);
		break;
	case tmShortErase: /* short erase */
		if(!(unsupported_ops&short_erase)){
			ParamText("\pDo you really want to erase (short) ?",PNS,PNS,PNS);
			if(my_modal_dialog(130,titoli_si_no,2)==1){
				flush_buffers(2);
				do_tape_erase(di.drive_number,0);
				}
			}
		else
			printf(does_not_supp);
		break;
	case tmLongErase: /* long erase */
		if(!(unsupported_ops&long_erase)){
			DisableItem(deviceMenu,0);

			ParamText("\pThe tape will be erased, that will take a lot of time",PNS,PNS,PNS);
			if(my_modal_dialog(153,titoli_ok_canc,2)==1){
				invalid_buffers();
				printf("Rewinding...\n");
				tape_rewind(di.drive_number);		/* according to SCSI 1 some devices may
							reject a full erase if not positioned at the beginning-of-medium */	
				if(!wait_for_rewind_complete()){
					printf("Erasing...\n");
					do_tape_erase(di.drive_number,1);
					di.v.si.current_tape_block=0x7FFFFFFF;	/* a rewind is necessary to
							restore it to a valid value: the medium position following
							an ERASE command with a long bit of one is not defined by
							SCSI 1 standard */
					if(!err_code){
						printf("Rewinding...\n");
						tape_rewind(di.drive_number);	/* the tape is quite useless at the end
										of tape... */
						if(!err_code){
							di.v.si.current_tape_block=0;
							di.v.si.tape_block_number=0;
							printf("Done\n");
							}
						}
					}
				}
			EnableItem(deviceMenu,0);
			DrawMenuBar();

			}
		else
			printf(does_not_supp);
		break;
	case tmReset:
		ParamText("\pDo you really want to reset this unit ?",PNS,PNS,PNS);
		if(my_modal_dialog(130,titoli_si_no,2)==1){
			do_scsi_message(di.drive_number,0x0C);
			/*if(err_code) printf("operation failed\n");*/
			}
		break;
	case tmStartHere:
		flush_buffers(2);
		di.v.si.current_tape_block=0;
		di.v.si.tape_block_number=0;
		invalid_buffers();
		printf("The current position of tape is now sector 0\n");
		/*flush_console();*/
		return;
	case tmPrintSense:
		print_sense(di.drive_number);
		flush_console();
		return;
	case tmTestRead:
		{ long trlen;
		static long nbytes=512;
		static char tipoblocco='v';
		int i;
		enum tipo_fase oldfase;
		jmp_buf savebuf;

		printf("bytes to read: ");
		if(tipoblocco=='v'&&di.v.si.block_mode==firmware_fixed_blocks) tipoblocco='f';
		disk_buffer[0]=tipoblocco;
		my_itoa(nbytes,(char*)&disk_buffer[1]);
		oldfase=fase;
		mcopy((char*)savebuf,(char*)main_loop,sizeof(jmp_buf));

		DisableItem(deviceMenu,0);	/* dovrei anche chiamare DrawMenuBar per far
					vedere che 'Device' � grigio, ma � cos� poco importante,
					anzi finirebbe per disturbare */
		if(setjmp(main_loop)<0){
			fase=oldfase;
			mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
			EnableItem(deviceMenu,0);
			DrawMenuBar();	/* per il caso che qualcun altro l'abbia chiamata nel frattempo...*/
			break;
			}
		fase=reading_sect_n;
		prompt((char*)disk_buffer,99);
		fase=oldfase;
			
		mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
		EnableItem(deviceMenu,0);
		DrawMenuBar();

		i= (disk_buffer[0]|32)=='f' || (disk_buffer[0]|32)=='v';
		if(!my_atol((char*)&disk_buffer[i],&trlen)){
			byte scsiCMD[6];
			short err,returnedStat,returnedMessage;
			i=verbosity;
			verbosity=3;
			nbytes=trlen;

#if 1
			if( (disk_buffer[0]|32)!='f'){
				tipoblocco='v';
				scsiCMD[0] = SCSI_READ;
				scsiCMD[1] = 0;	/* variable */
				scsiCMD[2] = lowbyte(hiword(nbytes));
				scsiCMD[3] = highbyte(loword(nbytes));
				scsiCMD[4] = lowbyte(loword(nbytes));
				scsiCMD[5] = 0;
				}
			else{
				tipoblocco='f';
				scsiCMD[0] = SCSI_READ;
				scsiCMD[1] = 1;
				scsiCMD[2] = 0;
				scsiCMD[3] = 0;
				scsiCMD[4] = 1;
				scsiCMD[5] = 0;
				}
			if(((long)all_buff_size<<9)>=nbytes){	/* nel buffer ci stanno */
				long j;
				err=do_scsi_command_in(di.drive_number,scsiCMD, buffers_base,nbytes,&returnedStat,
					&returnedMessage, (unsigned long*)&trlen,normalTimeout );	/* variable, non buttar via i dati */

				printf("n. bytes=%ld\n",trlen);
				disable_autoflush(1);
				if(trlen>1024) trlen=1024;
				
				for(j=0;j<trlen;j++){
					if(j%26==0 && j!=0)
						vai_a_capo();
					if(buffers_base[j]>=' '&&buffers_base[j]<127)
						printf(" %c ",buffers_base[j]);
					else
						printf((unsigned char)buffers_base[j]<16 ? "0%x " : "%x ",(unsigned char)buffers_base[j]);
					}
				vai_a_capo();
				enable_autoflush();
				}
			else	/* butta via tutto */
				err=do_scsi_command_in(di.drive_number,scsiCMD, (char*)disk_buffer,512,&returnedStat,
					&returnedMessage,(unsigned long*)-1L,normalTimeout);	/* variable, con buttar via i dati */
#else	/* experiments, which I temporarily connected to this menu command */
			dp("n bytes=%ld ",nbytes);
			fillmem(hd_buffer,'A',nbytes);
			if( (disk_buffer[0]|32)!='f'){
				tipoblocco='v';
				scsiCMD[0] = SCSI_WRITE;
				scsiCMD[1] = 0;	/* variable */
				scsiCMD[2] = lowbyte(hiword(nbytes));
				scsiCMD[3] = highbyte(loword(nbytes));
				scsiCMD[4] = lowbyte(loword(nbytes));
				scsiCMD[5] = 0;
				err=do_scsi_command_out(di.drive_number,&scsiCMD,hd_buffer/*disk_buffer*/,nbytes,&returnedStat,
					&returnedMessage,writeTimeout);	/* variable, con buttar via i dati */
				}
			else{
				tipoblocco='f';
				scsiCMD[0] = SCSI_WRITE;
				scsiCMD[1] = 1;
				scsiCMD[2] = 0;
				scsiCMD[3] = 0;
				scsiCMD[4] = 1;
				scsiCMD[5] = 0;
				err=do_scsi_command_out(di.drive_number,&scsiCMD,hd_buffer/*disk_buffer*/,nbytes,&returnedStat,
					&returnedMessage,writeTimeout);
				}
#endif
			/* mi secca perdere l'informazione sulla transfer size, ma d'altra parte un
			buffer grosso non l'ho e la modalit� skip attualmente non conta il
			numero di volte che riparte dall'inizio del buffer */
			/*printf("err1=%d err2=%d status=%d message=%d\n", err,err_code,returnedStat, returnedMessage);*/
			if(err==noErr && (returnedStat&stat_mask)==CHECK_CONDITION)
				sense_err_code=fill_sense_data();
			else
				sense_err_code=-2;
			verbosity=i;
	
			if(!sense_err_code){
				if((req_sense_response[0]&0xFE)==0xF0 && /* extended format, valid bit=1 */
		  		 (req_sense_response[2]&0x20) ){	/* incorrect length indicator */
					long l;
					mcopy((char*)&l,(char*)&req_sense_response[3],4);
					if(l<=0){
						printf("block size too small");
						if(l) printf(" (correct size=%ld)",nbytes-l);
						vai_a_capo();
						}
					else
						printf("block size too large (correct size=%ld)\n",nbytes-l);
					}
				print_sense(di.drive_number);
				}
			}
		}

		flush_console();
		return;
	case tmModeSelect:
		{ long trlen;
		jmp_buf savebuf;
		enum tipo_fase oldfase;

		oldfase=fase;
		mcopy((char*)savebuf,(char*)main_loop,sizeof(jmp_buf));

		DisableItem(deviceMenu,0);
		if(setjmp(main_loop)<0){
			fase=oldfase;
			mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
			EnableItem(deviceMenu,0);
			DrawMenuBar();
			break;
			}

		printf("block size: ");
		my_itoa((long)512,(char*)disk_buffer);
		fase=reading_sect_n;
		prompt((char*)disk_buffer,99);
		fase=oldfase;
			
		mcopy((char*)main_loop,(char*)savebuf,sizeof(jmp_buf));
		EnableItem(deviceMenu,0);
		DrawMenuBar();
		if(!my_atol((char*)&disk_buffer[0],&trlen)){
			do_mode_select(di.drive_number,trlen);
			if(!err_code){
				/* devo adeguare block strategy */
				if(trlen!=0)
					di.v.si.block_mode=firmware_fixed_blocks;
				else if( opt_variable_blocks=='y'||opt_variable_blocks=='v' )
					di.v.si.block_mode=variable_blocks;
				else
					di.v.si.block_mode=software_fixed_blocks;
				}
			}
		}
		break;
	case tmModeSense:
		{int i/*,j*/;
		char my_buffer[100];
		disable_autoflush(1);
		/*for(j=0;j<4;j++)*/{
			fillmem(my_buffer,0,sizeof(my_buffer));

			do_mode_sense(di.drive_number,&my_buffer[0],0 /*j*/);
			if(err_code)
				printf("err=%d\n",err_code);
			else{
				/*static char*tab_cosa[4]={"Current values\n","Mask of changeable values\n",
					"Default values\n","Saved values\n"};
				printf(tab_cosa[j]);*/
				/*printf("block descriptors size=%d\n",my_buffer[3]);*/
	
				printf(MEDIUM_TYPE,(unsigned char)my_buffer[1]);
				printf(WRITE_PROT,(my_buffer[2]&0x80) ? YES : NO);
				printf("buffering mode: %d\n",(my_buffer[2]>>4)&7);
	
				for(i=0;i<my_buffer[3];i+=8){
					long l=0;
					mcopy(((char*)&l)+1,&my_buffer[5+i],3);
					printf(DENS_BLOCKS,my_buffer[4+i],l);
					l=0;
					mcopy(((char*)&l)+1,&my_buffer[9+i],3);
					printf(BLOCK_SIZE,l);
					}
				}
			}
		enable_autoflush();
		}
		return;
	case tmTestUnitReady:
		do_scsi_generic(di.drive_number,TEST_UNIT_READY,NULL,0);
		/* 0 disco c'� 1 disco non c'� (o occupato...) <0 errore */
		if(err_code==0)
			printf("unit ready\n");
		else if(err_code==1){
			printf("unit not ready\n");
			err_code=0;
			}
		flush_console();
		return;
	}
else
	switch(item){
	case rmAllowWriting:
		if(warning_writing_dangers())
			di.writing_allowed=true;
		return;
	case rmUnmount: /* unmount */
		if(ID_to_drive_number(di.drive_number,&item) || unmount_volume(item)<0){
			printf("Can\'t unmount\n");
			flush_console();
			}
		else
			di.mounting_state=not_mounted;
		return;
	case rmMount: /* mount */
		{ParamBlockRec param;
		if(ID_to_drive_number(di.drive_number,&param.volumeParam.ioVRefNum)==noErr &&
		   ((di.v.fi.disk_code=PBMountVol (&param)) == noErr || di.v.fi.disk_code==volOnLinErr))
			di.mounting_state=mounted;
		else
			printf("Mount failed, error code %d\n",di.v.fi.disk_code);
		}
		flush_console();
		return;
	case rmEject: /* eject */
		flush_buffers(2);
		if(!ID_to_drive_number(di.drive_number,&item)){
			if(!unmount_volume(item))
				di.mounting_state=not_mounted;

			if(!volume_in_place(~(di.drive_number+32)) || Eject(NULL,item)){
				if(verbosity>=1) printf("The device driver could not eject, now trying the SCSI eject command\n");
				start_stop_unit(di.drive_number,2);
				}
			else
				err_code=0;
			}
		else
			start_stop_unit(di.drive_number,2);
		break;
	case rmRezero:	/* rezero unit */
		rezero_unit(di.drive_number);
		break;
	case rmReset:
		do_scsi_message(di.drive_number,0x0C);
		/*if(err_code) printf("operation failed\n");*/
		break;
	case rmStart:	/* start motor */
		start_stop_unit(di.drive_number,1);
		break;
	case rmStop:	/* stop motor */
		flush_buffers(2);
		start_stop_unit(di.drive_number,0);
		break;
	case rmPrintSense:
		print_sense(di.drive_number);
		break;
	}

if(err_code){
	printf("SCSI command terminated with error");
	if(verbosity>1)
		printf(" %d\n",err_code);
	else
		vai_a_capo();
	}
flush_console();
}

/***********************************************************/

void scan_SCSI_bus()
{
struct InquiryResponse inq_response;
short scsiID;

	if(setjmp(main_loop)<0) 
		return;

	one_empty_line();
	printf("Scanning the SCSI bus\n\n");

	for(scsiID=0;scsiID<7;scsiID++){
		fase=hack_listing;
		check_events(true);	/* the scan takes time, so it may be aborted */
		printf("ID=%d",scsiID);
		fillmem((char*)&inq_response,0,sizeof(inq_response));
		do_scsi_inquiry(scsiID,&inq_response,0);
		if(!err_code){
			short i,err;
			short lun,nluns;
			/* the problem here is that a lot of SCSI units don't support
			the concept of "logical unit". And it would be silly to print
			the same informations 8 times only because the controller ignores
			the LUN field. So, if LUN 0 and LUN 7 appear to be identical I suppose
			all 8 subunits are identical because the controller does not support the
			LUN field, otherwise I print the informations for all LUNs that do
			answer. Since I do that only if LUN 0 exists, I do not behave correctly
			for devices with multiple LUNs none of which is LUN 0, but that should
			not be a problem, since using 0 is so much standard...
			*/
			fillmem((char*)disk_buffer,0,sizeof(inq_response));

			if(!(supp_ops&inquire_all_luns))
				err_code=scsiBadPBErr;
			else
				do_scsi_inquiry(scsiID,disk_buffer,7);
			if(err_code==scsiBusResetErr || err_code==scsiBadPBErr )	/* bus reset, don't try again ! */
				nluns=1;
			else if(err_code){	/* my CONNER hard disk answers only once, but that's not a
							good reason to print "LUN=0" near the only
							answer and create confusion since for the SyQuest
							I don't print that */
				nluns=7;
				while(nluns>1){
					do_scsi_inquiry(scsiID,disk_buffer,nluns-1);
					if(!err_code) break;
					nluns--;
					}
				}
			else if(!compare_mem(&inq_response,disk_buffer,5+inq_response.AdditionalLength)){
				nluns=8;
				}
			else
				nluns=1;		/* my syQuest removable disk and the Apple CD 150 answer
								8 times, I must not print 8 identical messages */
			for(lun=0;lun<nluns;lun++){
				if(lun!=0)
					/* if(lun==7 mcopy(&inq_response,disk_buffer...) else... it's not worth of it */
					do_scsi_inquiry(scsiID,&inq_response,lun);
				else
					err_code=0;
				if(!err_code && (lun==0||(inq_response.DeviceType&0xE0)==0)){
					/* un controller "serio" SCSI-2 risponde per tutte le LUN ma la risposta
					dichiara (nei tre MSB di devicetype) che quella LUN non esiste */
					if(nluns!=1){
						if(lun!=0) printf("ID=%d",scsiID);
						printf(" LUN=%d",lun);
						}
					disable_autoflush(1);
					i=(inq_response.DeviceType)&0x1F;
					if(i>=sizeof(tipi)/sizeof(tipi[0]))
						printf("\n  %s (%d)\n",str_unk,inq_response.DeviceType);
					else if(i==0&&(inq_response.DeviceQualifier&0x80))
						printf(STR_NS,str_rem);
					else
						printf(STR_NS,tipi[i]);
					if(inq_response.AdditionalLength>=31 && ((inq_response.Version&7)>=1 ||
					  (inq_response.ResponseFormat&0xF)>=1 ) ){ /* enough
						bytes and at least CCS compliance (SCSI-1 did not specify the three
						product identification fields) */
						printf("  VendorID: %.8s\n",inq_response.VendorID);
						printf("  ProductID: %.16s",inq_response.ProductID);
						printf("  revision: %.4s\n",inq_response.Revision);
						}
					}
				}
			}
		else
			printf(" not present\n");
		enable_autoflush();
		}
#if 1
	{Handle h;
	extern SysEnvRec	gMac;
			/*printf("n=%d\n",gMac.machineType);*/
	if((h=GetResource('STR#',-16395))!=NULL){	/* this System resource contains the names of all Mac models */
		GetIndString (&inq_response,-16395,gMac.machineType+2);	/* quello su cui si sta girando... */
		ReleaseResource(h);
		printf("ID=7 %P\n\n",&inq_response);
		}
	else
		printf("ID=7 Macintosh\n\n");
	}
#else
	printf("ID=7 Macintosh CPU\n\n");
#endif
}


void fill_scsi_list(buff,default_dev,all_names)
char*buff;
short default_dev;
Boolean all_names;
{
struct fastscantype dati[7];
int i,j;
/*if(names_in_opendevice)
	fast_scsi_scan(&dati);*/

for(i=0;i<7;i++){
	buff[5]='0'+i;
	buff[6]=' ';
	buff[7]='\0';
	/*
	if(names_in_opendevice){
		if( dati[i].errCode==noErr){
			if((dati[i].DeviceType&0x1F)>=sizeof(tipi)/sizeof(tipi[0]))
				strcat(buff,str_unk);
			else if((dati[i].DeviceType&0x1F)==0&&(dati[i].DeviceQualifier&0x80))
				strcat(buff,str_rem);
			else
				strcat(buff,tipi[dati[i].DeviceType&0x1F]);
			strcat(buff," ");
			strncat(buff,dati[i].VendorID,8);
			}
		}
	else */
	if(i==default_dev || all_names){
		struct InquiryResponse inq_response;

		do_scsi_inquiry(i,&inq_response,0);
		if(!err_code){

			if((inq_response.DeviceType&0x1F)>=sizeof(tipi)/sizeof(tipi[0]))
				strcat(buff,str_unk);
			else if((inq_response.DeviceType&0x1F)==0&&(inq_response.DeviceQualifier&0x80))
				strcat(buff,str_rem);
			else
				strcat(buff,tipi[inq_response.DeviceType&0x1F]);
			if(inq_response.AdditionalLength>=31 && ((inq_response.Version&7)>=1 ||
						  (inq_response.ResponseFormat&0xF)>=1 ) ){ /* enough
							bytes and at least CCS compliance (SCSI-1 did not specify the three
							product identification fields) */
				strcat(buff," ");
				strncat(buff,inq_response.VendorID,8);
				}
			}
		}
	append_to_list(buff,strlen(buff));
	if(all_names){
		extern short last_list_cell;
		extern ListHandle LHandle;
		Cell myCell;
		myCell.v=last_list_cell;
		myCell.h=0;

		LSetSelect(true,myCell,LHandle);
		LAutoScroll(LHandle);
		LSetSelect(false,myCell,LHandle);
		}
	/*printf("ID=%d err=%d type=%d q=%d v=%.8s\n",i,dati[i].errCode,dati[i].DeviceType,
		dati[i].DeviceQualifier,dati[i].VendorID);*/
	}
}


short inquire_SCSI_device(scsiID)
short scsiID;
{
short err;
struct InquiryResponse inq_response;
short i;
byte my_buffer[100];

	verbosity=0;
	if(ask_verbosity){
		char buffer[32];
		ask_verbosity=false;
		printf("verbosity level (0 to 3): ");
		my_itoa((long)1,buffer);
		/*fase=reading_sect_n; no, in questo caso non devo accettare un abort */
		prompt(buffer,sizeof(buffer)-1);

		if(my_atoi(&buffer[0],&verbosity))
			verbosity=0;
		disable_autoflush(1);
		}

	/* if(verbosity>=3)
		printf("\nREQUEST SENSE test:\n\n"); */

	do_request_sense(scsiID,(unsigned char*)my_buffer);	/* incondizionato, per assegnare
			correttamente bytes_for_reqsen */
	if(verbosity>=3 && !err_code)print_request_sense_response(my_buffer);
/*dp("valore scelto=%d\n",bytes_for_reqsen);*/

#ifdef VERY_VERBOSE
	printf("testing SCSI ID: %d\n", scsiID);
	printf("\nSCSI INQUIRY Test:\n\n");
#endif
	di.v.si.supported_group0=di.v.si.supported_group1=0;
	fillmem((char*)&inq_response,0,sizeof(inq_response));
	do_scsi_inquiry(scsiID,&inq_response,0);
#ifdef CHEAT_TAPE	/* for debugging tape commands on a non-tape... */
	inq_response.DeviceType=TAPE_UNIT;
#endif
	

	/* stampa_buffer(0,&inq_response,0); */
	if(err_code){
		di.v.si.DeviceType=0xFF;
		di.isEjectable=true;
		return -1;
		}

	di.v.si.DeviceType=inq_response.DeviceType&0x1F;

	printf("Device type: ");
	i=(inq_response.DeviceType)&0x1F;
	di.isEjectable=(inq_response.DeviceQualifier&0x80)!=0;
	if(i>=sizeof(tipi)/sizeof(tipi[0]))
		printf("%s (%d)\n",str_unk,i);
	else if(i==0&&di.isEjectable)
		printf("%s\n",str_rem);
	else
		printf("%s\n",tipi[i]);
	printf("Device type qualifier: %d\n",inq_response.DeviceQualifier&0x7F);
	printf("Removable: %s\n",(inq_response.DeviceQualifier&0x80) ? YES:NO);

	printf("ANSI version: %d",inq_response.Version&7);
	if((inq_response.Version&7)==1 || (inq_response.Version&7)==2)
		printf(" (SCSI-%d)",inq_response.Version&7);
	vai_a_capo();
	if((inq_response.Version&7)>=2) printf("Response format: %d\n",inq_response.ResponseFormat&15);
	if(!err_code && inq_response.AdditionalLength>=31){
		disable_autoflush(1);
		printf("VendorID : %.8s\n",inq_response.VendorID);
		printf("ProductID: %.16s\n",inq_response.ProductID);
		printf("Revision : %.4s\n\n",inq_response.Revision);

		if(verbosity) printf("Additional length: %d\n",inq_response.AdditionalLength);

		if(expert_mode){
			char *p;
			long l;
			p=&inq_response.VendorUse2[2];
			mcopy((char*)&l,&inq_response.VendorUse2[3],4);
			if(inq_response.AdditionalLength>=36 && inq_response.VendorUse2[2]<=7 && l!=0){
				if(inq_response.AdditionalLength>=34) printf("Number of Extents: %x (hex)\n",*(short*) inq_response.VendorUse2);
	
				/* really, I've never seen a formal definition of the list of supported
				commands: maybe it was in CCS ? Anyway, a number of SCSI-1 drives support it
				(SCSI-2 drives do NOT support it, since SCSI-2 did not make it official so
				implicitly declaring it an outlaw) and I've understood the format by comparison
				with what was printed by the freeware utility "SCSI Spy"
				*/
				printf("List of supported commands:\n");

				while(*p<=7 && p<(char*)&inq_response+inq_response.AdditionalLength+4){
					long l;
					short j;
					printf ("group %d:",p[0]);
					mcopy((char*)&l,p+1,4);
					if(p[0]==0)
						di.v.si.supported_group0=l;
					else if(p[0]==1)
						di.v.si.supported_group1=l;
					for(j=0;j<32;j++)
						if(SUPPORTED(l,j)) printf(" %x",j+p[0]*32);
					vai_a_capo();
					p+=5;
					}
				}
			else{
				printf("The list of supported commands is not available\n");
				while(!*p && p<(char*)&inq_response+inq_response.AdditionalLength+4)
					p++;
				}

			if(p<(char*)&inq_response+inq_response.AdditionalLength+4){
				if(p[-1]>=' '&&p[-1]<=126) p--;		/* di solito il number of extents */
				if(p[-1]>=' '&&p[-1]<=126) p--;		/* non c'� se non ci sono i supported commands */
				if(p<(char*)&inq_response+inq_response.AdditionalLength+5){
					printf("Additional data:\n");
					while(p<(char*)&inq_response+inq_response.AdditionalLength+5)
						if(*p>=' '&&*p<127)
							printf("%c",*p++);	/*solo se ASCII !*/
						else if(verbosity>=1)
							printf("<%x>",(unsigned char)*p++);
						else
							break;
					vai_a_capo();
					}
				}
			}
		enable_autoflush();
		}

	fillmem(my_buffer,0,sizeof(my_buffer));

#ifdef VERY_VERBOSE
	printf("\nMODE SENSE test\n\n");
#endif

	do_mode_sense(scsiID,&my_buffer[0],0);

	if(!err_code){
		/*printf("block descriptors size=%d\n",my_buffer[3]);*/

		printf(MEDIUM_TYPE,(unsigned char)my_buffer[1]);
		printf(WRITE_PROT,(my_buffer[2]&0x80) ? YES : NO);

		for(i=0;i<my_buffer[3];i+=8){
			long l=0;
			mcopy(((char*)&l)+1,&my_buffer[5+i],3);
			printf(DENS_BLOCKS,my_buffer[4+i],l);
			/*di.sectors_on_floppy=l;*/
			l=0;
			mcopy(((char*)&l)+1,&my_buffer[9+i],3);
			printf(BLOCK_SIZE,l);
			}
		}
	enable_autoflush();

	if((inq_response.DeviceType&0x1F)!=TAPE_UNIT){
		/*set up the command buffer with the READ CAPACITY command*/

#ifdef VERY_VERBOSE
		printf("\nREAD CAPACITY test\n");
#endif

		fillmem(my_buffer,0,8);
		do_scsi_generic(scsiID,READ_CAPACITY,my_buffer,8);

		if(!err_code){
			printf("blocks %ld\nblock size %ld\n",*(long*)&my_buffer[0] ? *(long*)&my_buffer[0] +1 : 0,*(long*)&my_buffer[4]);
			}
		else
			printf("Error\n");
		}
	else{	/* tape device... */

#ifdef VERY_VERBOSE
		printf("\nREAD BLOCK LIMITS test\n");
#endif

		fillmem(my_buffer,0,6);

		do_read_block_limits(scsiID,my_buffer);

		if(!err_code){
#ifdef VERY_VERBOSE
			printf("reserved byte=%d\n",my_buffer[0]);
#endif
			my_buffer[0]=0;
			printf("minimum block size=%u maximum=%ld\n",*(short*)&my_buffer[4],*(long*)my_buffer);
			}
		}

return 0;
}

static void print_request_sense_response(buffer)
unsigned char *buffer;
{
/* io ovviamente ne stampo solo alcuni, che spero siano i pi� comuni. Comunque,
i codici hanno una gerarchia, per cui il qualifier aggiunge informazioni
al "additional sense code" per cui se il qualifier � non zero e non lo trovo,
almeno stampo il messaggio per quando il qualifier � 0, e quelli ce li ho messi
quasi tutti, lasciando fuori solo quelli evidentemente usati solo da scanner
e stampanti
N.B. sarebbe pi� elegante se anche la tabella dei codici fosse in una risorsa

-- I'm printing the code only for some errors (see the SCSI-2 documents for
the list), and for some I print only the "generic" message for the "additional
sense code" even if there is a nonzero qualifier which specifies more informations

*/

static unsigned char con_add_sense[]={
	0x00,0x01,
	0x00,0x02,
	0x00,0x03,
	0x00,0x04,
	0x00,0x05,
	0x00,0x06,
	0x03,0x01,
	0x03,0x02,
	0x04,0x01,
	0x04,0x03,
	0x11,0x01,
	0x11,0x02,
	0x11,0x03,
	0x11,0x05,
	0x11,0x06,
	0x11,0x08,
	0x11,0x09,
	0x11,0x0A,
	0x14,0x01,
	0x14,0x02,
	0x14,0x03,
	0x14,0x04,
	0x15,0x01,
	0x15,0x02,
	0x26,0x01,
	0x26,0x02,
	0x30,0x01,
	0x30,0x02, 
	0x30,0x03,
	0x3B,0x01,
	0x3B,0x02,
	0x3B,0x08,
	0x53,0x02,
	0x5A,0x01,
	0x5A,0x02
};


static unsigned char sense_key[]={

	0x01,0x02,0x03,0x04,0x05,
	0x06,0x07,0x08,0x09,0x0A,
	0x0C,0x10,0x11,0x12,0x13,
	0x14,0x15,0x16,0x17,0x18,
	0x19,0x1A,0x1C,0x1D,0x1E,
	0x20,0x21,0x22,0x24,0x25,
	0x26,0x27,0x28,0x29,0x2A,
	0x2B,0x2C,0x2D,0x2F,0x30,
	0x31,0x32,0x33,0x37,0x39,
	0x3A,0x3B,0x3D,0x3E,0x3F,
	0x40,0x41,0x42,0x43,0x44,
	0x45,0x46,0x47,0x48,0x49,
	0x4A,0x4B,0x4C,0x4E,0x50,
	0x51,0x52,0x53,0x57,0x5A,
	0x5C,0x63,0x64
	/* son quasi tutti, si risparmierebbe memoria a elencare chi non c'�
	invece che chi c'�, o a metterceli proprio tutti */
	};

/*printf("Additional bytes: %d\n",buffer[7]);*/
	/*printf("err=%d\n",err_code);*/
	if((buffer[0]&error_class_mask)!=error_class_mask){
		printf("Error Class=%d Error Code=%d\n",(buffer[0]&error_class_mask)>>4,
			buffer[0]&0xF);
		if(buffer[0]&0x80){
			buffer[0]=0;
			buffer[1]&=0x1F;
			printf("Logical Block Address =%ld\n",*(long*)buffer);
			}
		}
	else if(buffer[0]&0xE)
		printf("Extended sense: unknown data format %x\n",buffer[0]);
	else{	/* extended sense data format */
		static char *tab_sense[]={ "NO SENSE KEY (no error pending)",
			"RECOVERED ERROR (a first attempt failed but another one succeeded)",
			"NOT READY (disk/tape not present, or unit busy)",
			"MEDIUM ERROR (error reading magnetic media)",
			"HARDWARE ERROR (this is a very serious error)",
			"ILLEGAL REQUEST (the unit can't execute the last command)",
			"UNIT ATTENTION (the unit has just reset)",
			"DATA PROTECTION (write protected, or maybe password protected)",
			"BLANK CHECK (blank medium reading, or nonblank writing to WORM)",
			"Vendor unique",
			"COPY ABORTED",
			"ABORTED COMMAND",
			"EQUAL",
			"VOLUME OVERFLOW (disk or tape full)",
			"MISCOMPARE (verify operation failed)",
			"reserved"
			};
		printf("Sense key= %s\n",tab_sense[buffer[2]&0xF]);
		if(buffer[7]>=5) printf("Additional sense key: %d",buffer[12]);
		if(buffer[7]>=6){
			register unsigned char*p;
			short i;
			if(buffer[12]>=128){
				printf(QUALIFIER,buffer[13]);
				printf(" (Vendor specific code)");
				}
			else if(buffer[12]==0x40){
				printf(" (DIAGNOSTIC FAILURE ON COMPONENT %x)",buffer[13]);
				}
			else if(buffer[12]==0 && buffer[13]==0){
				printf(QUALIFIER,buffer[13]);
				printf(" (no additional sense information)");
				}
			else{
				/* vecchia versione, con una sola tabella
				p==&error_keys[0];
				for(i=0;i<sizeof(error_keys)/2;i++){
					if(*p++==buffer[12]){
						if(*p==buffer[13]){
							char string_buf[80];
							GetIndString (string_buf,1100,i+1);
							printf(QUALIFIER,buffer[13]);
							printf(STR_PAS,string_buf);
							break;
							}
						}
					p++;
					}
				*/
				Boolean found=false;
				char string_buf[80];
				p=&con_add_sense[0];
				if(buffer[13]!=0){
					for(i=0;i<sizeof(con_add_sense)/2;i++){
						if(*p++==buffer[12]){
							if(*p==buffer[13]){
								GetIndString (string_buf,1101,i+1);
								printf(QUALIFIER,buffer[13]);
								printf(STR_PAS,string_buf);
								found=true;
								break;
								}
							}
						p++;
						}
					}
				if(!found){
					for(i=0;i<sizeof(sense_key);i++){
						if(buffer[12]==sense_key[i]){
							GetIndString (string_buf,1100,i+1);
							if(buffer[13]==0){
								printf(QUALIFIER,buffer[13]);
								printf(STR_PAS,string_buf);
								}
							else{
								printf(STR_PAS,string_buf);
								printf("\n    ");	/* la stringa pu� essere lunga */
								printf(QUALIFIER,buffer[13]);
								}
							}
						}
					}
				}
			}
		vai_a_capo();
		if(verbosity>=1) printf("Segment Number=%d\n",buffer[1]);
		if(di.v.si.DeviceType==TAPE_UNIT||di.v.si.DeviceType==PRINTER_UNIT)
			printf("Start/end of medium: %s; ",(buffer[2]&0x40)?YES:NO);
		if(di.v.si.DeviceType==TAPE_UNIT){
			printf("Filemark: %s; ",(buffer[2]&0x80)?YES:NO);
			printf("Incorrect length: %s\n",(buffer[2]&0x20)?YES:NO);
			if(verbosity >0 && (buffer[2]&0x20)){
				long l;
				mcopy((char*)&l,&req_sense_response[3],4);
				printf("Delta length= %ld\n",l);
				}
			}
		}
}


short read_scsi_random(scsiID,sn,buffer,n_sect)
/* for random-access devices e.g. a hard disk */
/* if err_code==eofErr, it returns the number of sectors which
were successfully read. Otherwise, the return value is not specified */
short scsiID;
sector_t sn;
char* buffer;
short n_sect;
{
	byte scsiCMD[10];
	short err,returnedStat,returnedMessage;
	long buf_size;
	short sect_chunk;

	if(verbosity>=2) printf("read %d sectors starting from %ld\n",n_sect,sn);

	if(di.v.si.block_size!=512){
		sn /= (di.v.si.block_size>>9);
		n_sect/=(di.v.si.block_size>>9);
		}

	do{
		/*set up the command buffer with the READ command*/

		sect_chunk = n_sect<=16 ? n_sect : 16;	/* well, I don't want to read
						too many bytes a time, it may be a problem for the
						controller if its internal buffer is small. And anyway
						I would not want to go beyond 255 since READ(6) has
						a 1-byte sector count */

		buf_size=sect_chunk*di.v.si.block_size;

		if( (sn&~0x1FFFFF)==0 ){	/* fits in 21 bits, better using READ(6) which is
								mandatory for random access devices */
			scsiCMD[0] = SCSI_READ;
			scsiCMD[1] = lowbyte(hiword(sn))&0x1F;
			scsiCMD[2] = highbyte(loword(sn));
			scsiCMD[3] = lowbyte(loword(sn));
			scsiCMD[4] = sect_chunk;
			scsiCMD[5] = 0;
			}
		else{		/* does not fit in 21 bits, use READ(10), which obviously
					is supported by all units having so many sectors */
			scsiCMD[0] = SCSI_READ_10;
			scsiCMD[1] = 0;
			*(long*)&scsiCMD[2]= sn;	/* bytes 2,3,4,5, in big-endian order */
			scsiCMD[6]=0;
			scsiCMD[7]=0;	/*highbyte(sect_chunk);*/
			scsiCMD[8]=sect_chunk;
			scsiCMD[9]=0;
			}
	
		err=do_scsi_command_in(scsiID,&scsiCMD,buffer,buf_size,&returnedStat,
			&returnedMessage,NULL,readFixedBlockTimeout+sn*extraTimeout);
		/*printf("err,err_code,returnedStat, returnedMessage = %d %d %d %d\n\n", err,err_code,returnedStat, returnedMessage);*/
	
		PRINT_SENSE
		CLEAR_RECOVERED_ERROR
		if(err)
			err_code=err;
		else if(err_code)
			err_code=scsiOverrunErr;
		else if(returnedStat||returnedMessage)
			err_code=scsiBadStatus;

		n_sect-=sect_chunk;
		sn+=sect_chunk;
		buffer+=buf_size;
		}
	while(!err_code && n_sect!=0);

	return 0;
}

void write_scsi_random(scsiID,sn,buffer,n_sect)
/* for random-access devices e.g. a hard disk
*/
short scsiID;
sector_t sn;
char* buffer;
short n_sect;
{

	byte scsiCMD[10];
	short err,returnedStat,returnedMessage;
	long buf_size;
	short sect_chunk;

if(verbosity>=2)printf("write %d sectors starting from %ld\n",n_sect,sn);

		/*set up the command buffer with the WRITE command*/

	if(di.v.si.block_size!=512){
		sn /= (di.v.si.block_size>>9);
		n_sect/=(di.v.si.block_size>>9);
		}
	do{
		sect_chunk = n_sect<=8 ? n_sect : 8;	/* writing may have stricter limits
							than reading...  */
		buf_size=sect_chunk*di.v.si.block_size;

		if( (sn&~0x1FFFFF)==0){	/* fits in 21 bits, better using WRITE(6) which is
								mandatory for random access devices (except CD-ROMs) */
			scsiCMD[0] = SCSI_WRITE;
			scsiCMD[1] = lowbyte(hiword(sn))&0x1F;
			scsiCMD[2] = highbyte(loword(sn));
			scsiCMD[3] = lowbyte(loword(sn));
			scsiCMD[4] = sect_chunk;
			scsiCMD[5] = 0;
			}
		else{		/* does not fit in 21 bits, use WRITE(10) */
			scsiCMD[0] = SCSI_WRITE_10;
			scsiCMD[1] = 0;
			*(long*)&scsiCMD[2]= sn;	/* bytes 2,3,4,5 */
			scsiCMD[6]=0;
			scsiCMD[7]=0;	/*highbyte(sect_chunk);*/
			scsiCMD[8]=sect_chunk;
			scsiCMD[9]=0;
			}

		if(scsiID != di.m_vrefnum){	/* protection against accidental alteration
				of the drive_number variable (in suntar 1.3 and earlier versions it was
				cleared in some error situations to mean "no disk open", but ID 0 usually
				is the internal hard disk, the most important unit, and I don't want that
				a silly bug might erase my hard disk !) */
			invalid_buffers();
			printf("%d %d\n",scsiID, di.m_vrefnum);
			error_message("Internal error\n");
			}
	
		err=do_scsi_command_out(scsiID,&scsiCMD,buffer,buf_size,&returnedStat,&returnedMessage,writeTimeout);
				/* uff, non so, anche se � un direct access potrebbe avere dei tempi morti, ad es. per
				far ripartire il motore... */
		PRINT_SENSE
		CLEAR_RECOVERED_ERROR
	
		/*printf("err,err_code,returnedStat, returnedMessage = %d %d %d %d\n\n", err,err_code,returnedStat, returnedMessage);*/
		if(err)
			err_code=err;
		else if(err_code)
			err_code=scsiOverrunErr;
		else if(returnedStat||returnedMessage)
			err_code=scsiBadStatus;

		n_sect-=sect_chunk;
		sn+=sect_chunk;
		buffer+=buf_size;
		}
	while(!err_code && n_sect!=0);


/*printf("SCSI cmd= %x %x %x %x %x %x\n",scsiCMD[0],scsiCMD[1],scsiCMD[2],scsiCMD[3],scsiCMD[4],scsiCMD[5]);*/

}



void write_scsi_tape(scsiID,sn,buffer,n_sect)
/* for sequential-access devices, i.e. tape units */

/* note: a write routine should not perform raise_error, because the buffer
is not flushed and the same write will be called again */

short scsiID;
sector_t sn;
char* buffer;
short n_sect;
{

byte scsiCMD[6];
short err,returnedStat,returnedMessage;


if(sn<di.v.si.current_tape_block){
	/* may solve by rewinding, but maybe after asking the user for
	confirmation, since it's a costly operation (and dangerous, if the purpose
	is to write...) */
	if(di.v.si.current_tape_block==0x7FFFFFFF)
		printf("Unknown position of tape ! Rewind and retry\n");
	else
		printf("Non-sequential access to tape (%ld<%ld)! Rewind and retry\n",sn,di.v.si.current_tape_block);
	err_code=-1;
	return;
	}

if(sn>di.v.si.current_tape_block){
	printf("non-sequential access to tape (write) %ld to %ld while current=%ld!\n",sn,sn+n_sect-1,di.v.si.current_tape_block);
	/* Might try to solve otherwise: either zero-fill till the requested position
	or (if the medium contains enough data) skip pre-existing blocks. In any
	case a tar archive is always written sequentially, and this situation
	should never happen, hence considering it as an error is not a severe
	restriction */
	err_code=-1;
	return;
	}

		/*set up the command buffer with the WRITE command*/

if(verbosity>=2)printf("writing sectors %ld to %ld\n",sn,sn+n_sect-1);
if(di.v.si.block_mode==fixed_blocks || di.v.si.block_mode==firmware_fixed_blocks){
	short nblocks;
	short nb;

#ifdef CHEAT_TAPE
dp("bl=%ld bf=%d\n",di.v.si.block_size,tar_block_size	);
#endif
	if(di.v.si.block_size>512){
		nblocks=((long)n_sect<<9)/di.v.si.block_size;
		nb=tar_block_size/(short)(di.v.si.block_size>>9);
		}
	else{	/* it's useless to do those divisions */
		nblocks=n_sect;
		nb=tar_block_size;
		}
	if(nb==0 || !(supported_ops&multiple_blocks)) nb=1;
	do{

		if(nb>nblocks) nb=nblocks;
		scsiCMD[0] = SCSI_WRITE;
		scsiCMD[1] = 1;	/* fixed */
		scsiCMD[2] = 0;
		scsiCMD[3] = highbyte(nb);
		scsiCMD[4] = lowbyte(nb);
		scsiCMD[5] = 0;
		nblocks-=nb;
		if(verbosity>=2) printf("%d fixed blocks of %ld bytes\n",nb,(long)di.v.si.block_size);

		err=do_scsi_command_out(scsiID,&scsiCMD,buffer,nb*di.v.si.block_size,
			&returnedStat,&returnedMessage,writeTimeout);
		PRINT_SENSE
		CLEAR_RECOVERED_ERROR
		if(err||err_code||returnedStat||returnedMessage) break;
		buffer+= nb*di.v.si.block_size;
		}
	while(nblocks);
	}
else{	/* variable_blocks: write blocks with all the data, but not
		longer than specified by the blocking factor (the buffer manager
		should allocate an integer multiple of such size, so
		the blocking factor should always be respected) */
	long trlen=(long)n_sect<<9;	/* size of allocated buffer */
	long bytes_to_write=(long)tar_block_size<<9;
	do{
		if(bytes_to_write>trlen) bytes_to_write=trlen;
		trlen-=bytes_to_write;
		scsiCMD[0] = SCSI_WRITE;
		scsiCMD[1] = 0;	/* variable */
		scsiCMD[2] = lowbyte(hiword(bytes_to_write));
		scsiCMD[3] = highbyte(loword(bytes_to_write));
		scsiCMD[4] = lowbyte(loword(bytes_to_write));
		scsiCMD[5] = 0;
		if(verbosity>=2) printf("one variable block of %ld bytes\n",bytes_to_write);
		err=do_scsi_command_out(scsiID,&scsiCMD,buffer,bytes_to_write,&returnedStat,
			&returnedMessage,writeTimeout);
		if(err||err_code||returnedStat||returnedMessage) break;
		PRINT_SENSE
		CLEAR_RECOVERED_ERROR
		buffer+=bytes_to_write;
		}
	while(trlen);
	}
if(err)
	err_code=err;
else if(err_code)
	err_code=scsiOverrunErr;
else if(returnedStat||returnedMessage)
	err_code=scsiBadStatus;
if(!err_code) di.v.si.current_tape_block += n_sect;	/* it's questionable whether
	in case of error the tape has moved or not: probably it has not for "bad command",
	but it has for "bad data": I can't distinguish all error codes ! */

}

static void skip_next_block(scsiID) /* routine pericolosa, meglio controllarla
									per bene !!! */
/* sostituto allo space forward su block se non disponibile:
uso una READ ma buttando via i dati che arrivano */
short scsiID;
{
byte scsiCMD[6];
char my_buffer[512];
short err,returnedStat,returnedMessage;
long trlen;
		/*set up the command buffer with the READ command*/

	scsiCMD[0] = SCSI_READ;
	scsiCMD[5] = 0;

#if 1
	if(di.v.si.block_mode==fixed_blocks || di.v.si.block_mode==firmware_fixed_blocks){
		scsiCMD[1] = 1;	/* fixed */
		scsiCMD[2] = 0;
		scsiCMD[3] = 0;
		scsiCMD[4] = 1;	/* one block */
		}
	else if(di.v.si.block_mode==software_fixed_blocks){
		scsiCMD[1] = 0;	/* variable */
		scsiCMD[2] = lowbyte(hiword(di.v.si.block_size));
		scsiCMD[3] = highbyte(loword(di.v.si.block_size));
		scsiCMD[4] = lowbyte(loword(di.v.si.block_size));
		}
	else{
		trlen=1024L*1024L;	/* dico al tape di trasferire fino a 1Mega, 
							tanto all'SCSI manager dico di buttare via tutto */
		if(unsupported_ops&standard_variable_blocks){	/* allora devo leggere il vero valore o mi d� errore */
			do_mode_sense(scsiID,my_buffer,0);
			my_buffer[8]=0;
			if(!err_code )
				trlen= *(long*)&my_buffer[8];
			}
		scsiCMD[1] = 0;	/* variable */
		scsiCMD[2] = lowbyte(hiword(trlen));
		scsiCMD[3] = highbyte(loword(trlen));
		scsiCMD[4] = lowbyte(loword(trlen));
		}
#else
	/* for debugging I've tested this routine on a hard disk... */
	{long sn=0;
	short n_sect=255;
		scsiCMD[1] = lowbyte(hiword(sn))&0x1F;
		scsiCMD[2] = highbyte(loword(sn));
		scsiCMD[3] = lowbyte(loword(sn));
		scsiCMD[4] = n_sect;
	}
#endif
	err=do_scsi_command_in(scsiID,&scsiCMD,my_buffer,512,
		&returnedStat,&returnedMessage,(long*)-1L,readFixedBlockTimeout); /* comunque
			qualunque timeout dovrebbe essere OK, viene attivato dalla SCSIComplete
			e quindi DOPO che i dati sono stati trasferiti */
	/* a questo punto non faccio nessuna gestione errori perch� non
	� ben chiaro quel che una unit� risponder� a ci� che ho fatto */
	PRINT_SENSE;
	CLEAR_RECOVERED_ERROR
	/* altri errori come CRC error vanno ignorati tanto volevo solo
	skippare... ma il problema � che comunque bisogna capire quanti
	blocchi sono stati passati, per di pi� CRC error � un sottocaso
	di MEDIUM ERROR da identificare tramite additional sense key...
	per ora non lo faccio !!! */

}

static void do_scsi_inquiry(scsiID,response,logical_unit_number)
short scsiID;
struct InquiryResponse *response;
short logical_unit_number;
{
	byte scsiCMD[6];
	short err,returnedStat,returnedMessage;

		/*set up the command buffer with the INQUIRY command*/

	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0] = INQUIRY;
	scsiCMD[1] = logical_unit_number<<5;
	scsiCMD[4] = 6;	/* per far le cose bene NON devo rischiare il solito error 5
					(che SCSIRead ritorna tutte le volte che il numero di bytes ritornati
					dal device non coincide con quello che gli ho chiesto), quindi
					prima gli chiedo solo i primi byte, che contengono anche la lunghezza
					totale della risposta, poi gli chiedo tutto
					--The SCSI manager returns error 5 if the returned bytes are not
					as many as requested: so I ask only 6 bytes, get the full size from
					the answer and now I ask the full answer */
	err=do_scsi_command_in(scsiID,&scsiCMD,response,6,&returnedStat,&returnedMessage,NULL,normalTimeout);
	/*printf("codici=%d %d %d %d\n",err,returnedStat,returnedMessage,err_code);*/
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
	if(err_code) return;
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0] = INQUIRY;
	scsiCMD[4] = 5+response->AdditionalLength;
	err=do_scsi_command_in(scsiID,&scsiCMD,response,5+response->AdditionalLength,
		&returnedStat,&returnedMessage,NULL,normalTimeout);
	/*printf("codici=%d %d %d %d\n",err,returnedStat,returnedMessage,err_code);*/
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;

/*
	if(response->DeviceType==16)
		response->DeviceType= 0;	Apple tape unit */
}


void tape_rewind(scsiID)
short scsiID;
{
short err;
#ifdef DEBUG
err_code=0;
#else
	byte scsiCMD[6];
	short returnedStat,returnedMessage;
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));

		/*set up the command buffer with the REWIND command*/

	scsiCMD[0] = REWIND;
	if(!(unsupported_ops&rewind_immediate)) scsiCMD[1] = 1;	/* return immediately */
/*printf("SCSI cmd= %x %x %x %x %x %x\n",scsiCMD[0],scsiCMD[1],scsiCMD[2],scsiCMD[3],scsiCMD[4],scsiCMD[5]);*/

#ifdef CHEAT_TAPE
	err_code=0; return;
#endif

	err=do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL,scsiCMD[1]?rewindTimeout:normalTimeout);
	PRINT_SENSE
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
#endif
}


void tape_skip(scsiID,what,count)
short scsiID, what;	/* 0 skip blocks, 1 skip filemarks, 3 skip to end of medium (count ignored) */
long count;	/* 1 forward, -1 backwards. Really, other values are accepted:
				e.g. what=0, count=10 means "skip forward 10 blocks": that's why
				it's a long. Beware, not all tapes support backwards SPACE */
{
	byte scsiCMD[6];
	short err,returnedStat,returnedMessage;
	long timeout;

		/*set up the command buffer with the SPACE command*/

	scsiCMD[0] = SCSI_SPACE;
	scsiCMD[1] = what&7;	/* according to SCSI-2, that's a 3-bit field, with
							6 meaningful values */
	scsiCMD[2]=lowbyte(hiword(count));
	scsiCMD[3]=highbyte(loword(count));
	scsiCMD[4]=lowbyte(loword(count));
	scsiCMD[5] = 0;
	if(what==0){
		if(count>1000 || count < 0)
			timeout=rewindTimeout;
		else
			timeout=skipBlockTimeout + count * deltaSkipTimeout;
		}
	else
		timeout=rewindTimeout;
#ifdef CHEAT_TAPE
err_code=0; return;
#endif
	err=do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL,timeout);
	PRINT_SENSE

	if(err==scsiBusResetErr || err==scsiBadPBErr ){	/* that is, it locked the SCSI bus,
					 a thing which should never happen but a lot of units do when
					 they receive a command which is not supported */
		long mask;
		if(what==1)
			mask=count>=0 ? skip_file_forward : skip_file_backwards;
		else if(what==0)
			mask=count>=0 ? skip_block_forward : skip_block_backwards;
		
		unsupp_ops|=mask;
		supp_ops&=~mask;
		printf("Since locking the SCSI bus is NOT recommended, you should\nread the documentation about how setting the \"unsupported ops\" parameter\n");
		}
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedMessage)
		err_code=scsiBadStatus;
	else if((returnedStat&stat_mask)==CHECK_CONDITION && what!=3){
		/* a check condition may mean a lot of things, including "end of medium
		reached before skipping as many as required" */
		do_request_sense(scsiID,req_sense_response);
		if(!err_code && (req_sense_response[0]&0x7E)==0x70 &&
			((req_sense_response[2]&0x40)||(req_sense_response[2]&0x80)&&what==0) ) 
				/* EOM or filemark, il secondo solo per skip block */
			err_code=eofErr;
		else
			err_code=scsiBadStatus;
		}
	else if(returnedStat)
		err_code=scsiBadStatus;
}

void tape_load_unload(scsiID,load)
short scsiID;
byte load;	/* 0= unload/stop motor 1=load/start motor 2=Eject */ 
{
	byte scsiCMD[6];
	short returnedStat,returnedMessage,err;
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0]=LOAD_UNLOAD;

	if(load!=1){
		do_scsi_generic(scsiID,ALLOW_MEDIUM_REMOVAL,NULL,0);
		/* if(err_code) return; well, it's better to continue anyway, maybe
		this error does not imply a failure in the unload */
		}
	scsiCMD[4] = load ;

	scsiCMD[1] = 1;	/* return immediately */
/*printf("SCSI cmd= %x %x %x %x %x %x\n",scsiCMD[0],scsiCMD[1],scsiCMD[2],scsiCMD[3],scsiCMD[4],scsiCMD[5]);*/
	err=do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL,
		(di.v.si.DeviceType!=TAPE_UNIT||load==1) ?loadTimeout:rewindTimeout);
	PRINT_SENSE
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
}

static void do_write_filemarks(scsiID,number_of_filemarks)
short scsiID;
long number_of_filemarks;	/* 24 bits are meaningful */ 
{
	byte scsiCMD[6];
	short returnedStat,returnedMessage,err;
	scsiCMD[0]=WRITE_FILEMARKS;
	scsiCMD[1]=0;
	scsiCMD[2]=lowbyte(hiword(number_of_filemarks));
	scsiCMD[3]=highbyte(loword(number_of_filemarks));
	scsiCMD[4]=lowbyte(loword(number_of_filemarks));
	scsiCMD[5]=0;

/*printf("SCSI cmd= %x %x %x %x %x %x\n",scsiCMD[0],scsiCMD[1],scsiCMD[2],scsiCMD[3],scsiCMD[4],scsiCMD[5]);*/

#ifdef CHEAT_TAPE
printf("scrivo filemark\n");
err=err_code=returnedStat=returnedMessage=0;
#else
	err=do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL,writeTimeout);
#endif
	PRINT_SENSE
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
}

static void do_tape_erase(scsiID,till_endofmedium)
short scsiID;
Boolean till_endofmedium;
{
	byte scsiCMD[6];
	short returnedStat,returnedMessage,err;
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0]=ERASE;
	scsiCMD[1]=till_endofmedium&1;

/*printf("SCSI cmd= %x %x %x %x %x %x\n",scsiCMD[0],scsiCMD[1],scsiCMD[2],scsiCMD[3],scsiCMD[4],scsiCMD[5]);*/
	err=do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL,till_endofmedium?rewindTimeout:shortEraseTimeout);
	PRINT_SENSE
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
}


static void do_scsi_generic(scsiID,command,buffer,buf_size)
/* la uso per READ CAPACITY, TEST UNIT READY */
short scsiID;
byte command;
char*buffer;
short buf_size;
{
	byte scsiCMD[12];
	short err,returnedStat,returnedMessage;
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));

		/*set up the command buffer with the command*/

	scsiCMD[0] = command;
	err=do_scsi_command_in(scsiID,scsiCMD,buffer,buf_size,&returnedStat,&returnedMessage,NULL,normalTimeout);
	if(command!=TEST_UNIT_READY){
		PRINT_SENSE_POCO_VERBOSO
		/* uso quello poco verboso perch� si tratta di READ CAPACITY, READ BLOCK LIMITS
		ALLOW MEDIUM REMOVAL, nessuno di questi � un comando critico durante l'esecuzione */
		}
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedMessage)
		err_code=scsiBadStatus;
	if(!err_code&&returnedStat) err_code=command==TEST_UNIT_READY ? 1 : scsiBadStatus;
			/* per TEST_UNIT_READY returnStat non � un codice di errore */
}

static void do_mode_sense(scsiID,buffer,which)
/* MODE SENSE che ha la stessa necessit� di doppia chiamata di inquiry, e poi
pu� avere un parametro variabile in pi�
--see the comment in do_scsi_inquiry for the reason to execute the command twice
*/
short scsiID;
char *buffer;
short which;	/*  00b	Current values
					01b		Changeable values (bitmask, 1 in changeable fields)
					10b		Default values
					11b		Saved values
				*/

{
	byte scsiCMD[6];
	short err,returnedStat,returnedMessage;

	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	fillmem(buffer,0,12);

		/*set up the command buffer with the command*/

	scsiCMD[0] = MODE_SENSE;
	scsiCMD[2] = which<<6;
	scsiCMD[4] = 12; /* 4 fixed data + at least one 8-bytes info block */
	err=do_scsi_command_in(scsiID,scsiCMD,buffer,12,&returnedStat,&returnedMessage,NULL,normalTimeout);
	PRINT_SENSE_POCO_VERBOSO

	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedMessage)
		err_code=scsiBadStatus;
	else if(!err_code&&returnedStat)
		err_code=scsiBadStatus;
	if(err_code||buffer[3]<=12-4) return;

	/* MODE SENSE requires a second command to get the whole response, now that
	I know its size */
	fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0] = MODE_SENSE;
	scsiCMD[4]=4+buffer[3];
	err=do_scsi_command_in(scsiID,scsiCMD,buffer,scsiCMD[4],&returnedStat,&returnedMessage,NULL,normalTimeout);
	PRINT_SENSE_POCO_VERBOSO
	if(err)
		err_code=err;
	else if(err_code)
		err_code=scsiOverrunErr;
	else if(returnedStat||returnedMessage)
		err_code=scsiBadStatus;
}

static void do_request_sense(scsiID,buffer)
/* in the SCSI jargon, "request sense" means "send me additional status informations":
when performing any operation "one status code, CHECK CONDITION, indicates that
additional information is available".

The problem with such command is that the standard specifies two different
formats for the answer, the rules to specify that I need a specific format
are unclear and a number of devices violate even what is clear. So, the
first time a device is opened I experiment a little to discover the best way
to perform this command (a request sense automatically clears the error
condition hence I can't experiment when it's time to get critical informations
about what caused an operation to fail)


coding of bytes_for_reqsen:
-10000: nothing done yet
>=4: must ask as many bytes, and as many arrive (if it's =32, fewer bytes
	 may arrive, but the trick to count the bytes worked OK)
<0 but >-1000: must ask 0 bytes and -bytes_for_reqsen bytes are returned
*/
short scsiID;
unsigned char*buffer;
{
	byte scsiCMD[6];
	short err,returnedStat,returnedMessage;
	long l;

	short n_to_read=bytes_for_reqsen;
	if(bytes_for_reqsen<=-1000) n_to_read=32;

	while(n_to_read>=4){
		fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
		fillmem(buffer,0,16);	/* in caso di errore non voglio avere garbage, anche
			perch� qualche volta lo stampo senza guardare se c'� stato errore */

		scsiCMD[0] = REQUEST_SENSE;
		scsiCMD[4] = n_to_read;		/* buffer size... */

		err=do_scsi_command_in(scsiID,scsiCMD,buffer,n_to_read,
			&returnedStat,&returnedMessage,n_to_read==32?&l:NULL,normalTimeout);
		if(err)
			err_code=err;
		else if(err_code)
			err_code=scsiOverrunErr;
		else if(returnedStat||returnedMessage)
			err_code=scsiBadStatus;
		else if(n_to_read==32 && l==0)
			err_code=scsiOverrunErr;
		/*if(err_code)
			dp("%d => %d failed\n",n_to_read,n_to_read);
		else
			dp("%d => %d ok !\n",n_to_read,n_to_read);
		*/

		if(!err_code && bytes_for_reqsen< -1000){
			char first_ans[18];
			mcopy(first_ans,buffer,18);
			if((n_to_read != 32  || l!=buffer[7]+8) && n_to_read!=buffer[7]+8 &&
				buffer[7]+8<=32){
				/* se lui dice che ci sono quei bytes, provaci */
				scsiCMD[4]=buffer[7]+8;
				err=do_scsi_command_in(scsiID,scsiCMD,buffer,buffer[7]+8,
					&returnedStat,&returnedMessage,NULL,normalTimeout);
				if(!err&&!err_code&&!returnedStat&&!returnedMessage&&buffer[7]==first_ans[7])
					bytes_for_reqsen=buffer[7]+8;
				else{
					err_code=0;	/* sono dentro un if(!err_code && ), quindi� */
					mcopy(buffer,first_ans,18);
					bytes_for_reqsen= n_to_read;
					}
				}
			else
				bytes_for_reqsen= n_to_read;
			/* uff, questo non � molto naturale, ma ci provo. Ci sono delle
			unit� che a chiedergli 0 bytes ritornano comunque un extended format,
			il che � contrario a ci� che dice lo standard, ma comunque � una
			possibilit� perch� mi evita l'imbarazzo di decidere io. Per cui provo,
			se lui si comporta cos� allora preferisco quel modo
			--some units return an extended format also when asking 0 bytes:
			that violates the standard, but if it happens I exploit that */

			scsiCMD[4]=0;
			err=do_scsi_command_in(scsiID,scsiCMD,buffer,buffer[7]+8,
				&returnedStat,&returnedMessage,NULL,normalTimeout);
			if(!err&&!err_code&&!returnedStat&&!returnedMessage&&
				buffer[7]==first_ans[7] && buffer[0]==first_ans[0])
					bytes_for_reqsen= - (buffer[7]+8);
			else{
				err_code=0;	/* idem come sopra */
				mcopy(buffer,first_ans,18);
				}

			}
		if(!err_code || bytes_for_reqsen>-1000) return;

		/* failed, then try with another size ! */
		if(n_to_read==32)
			n_to_read=18;	/* tanto oltre al byte 14 non saprei interpretarlo:
							chiederne 32 serve solo con verbosity 3, per poter
							leggere a video tutta la risposta */
		else
			--n_to_read;
		}
	if(bytes_for_reqsen<=-1000)
		n_to_read=18;
	else
		n_to_read= -bytes_for_reqsen;


	/* my hard disk (CONNER) does not answer if I ask for 4 bytes, only if I tell it
	that I pretend nonextended format by asking 0 bytes (or ask more than 4 bytes),
	then it returns those 4 bytes; if I ask 18 bytes, then it returns 18 bytes
	of extended format. And an ARCHIVE VIPER tape unit expects a request of 0
	bytes but returns an extended format! */

	while(n_to_read>=4){
		fillmem((char*)scsiCMD,0,sizeof(scsiCMD));
		fillmem(buffer,0,16);	/* in caso di errore non voglio avere garbage, anche
			perch� qualche volta lo stampo senza guardare se c'� stato errore */

		scsiCMD[0] = REQUEST_SENSE;
		/*scsiCMD[4] = 0;	/* force non-extended format for the answer */

		err=do_scsi_command_in(scsiID,scsiCMD,buffer,n_to_read,
			&returnedStat,&returnedMessage,NULL,normalTimeout);
		if(err)
			err_code=err;
		else if(err_code)
			err_code=scsiOverrunErr;
		else if(returnedStat||returnedMessage)
			err_code=scsiBadStatus;
		/*if(err_code)
			dp("0 => %d failed\n",n_to_read);
		else
			dp("0 => %d ok !\n",n_to_read);
		*/

		if(!err_code) bytes_for_reqsen= -n_to_read;
		if(!err_code || bytes_for_reqsen>-1000) return;
		
		/* failed, then try with another size ! */
		--n_to_read;
		}
	return;	/* no other alternative to try */
}

void do_tape_print_sense(void);
static void do_tape_print_sense()
{
if(!sense_err_code){
	if(verbosity>=1) printf(REQSENRESP);
		print_request_sense_response(req_sense_response);
		vai_a_capo();
		}
	else
		printf(ERROR_IN_SENSE);
}


#undef PRINT_SENSE
/* !!!! � un po' troppo complicata come macro, pu� aver senso che almeno
il pezzo centrale sia una funzione */
#define PRINT_TAPE_SENSE \
	if(err==noErr && (verbosity>=1 || (returnedStat&stat_mask)==CHECK_CONDITION &&	\
	   !sense_err_code&&(req_sense_response[0]&0x7E)==error_class_mask &&			\
	   ((req_sense_response[2]&0xF)||req_sense_response[12]||req_sense_response[13])) ){		\
		if((returnedStat&stat_mask)==CHECK_CONDITION)			\
			do_tape_print_sense();								\
		else if(err_code||returnedStat||returnedMessage)		\
			printf(SENSE_MSG,err_code,returnedStat,returnedMessage);	\
		}

/**************************************************/

void reposition_tape()
{
	/*printf("Error: non-sequential access to tape ! (%ld %ld)\n",n_sect,di.v.si.current_tape_block);*/

	if(di.v.si.current_tape_block==0x7FFFFFFF)
		ParamText("\pUnknown position of tape, rewind ?",NULL,NULL,NULL);
	else
		ParamText("\pNon-sequential access to tape, rewind ?",NULL,NULL,NULL);

	if(my_modal_dialog(130,NULL,0)==1){
		printf("Rewind tape...\n");
		if(archive_rewind()) raise_error();
		}
	else{
		if(di.v.si.current_tape_block==0x7FFFFFFF){
			ParamText("\pOnly Rewind, Skip filemark or \"Set\rarchive start here\" can allow you\rto perform further read/writes",NULL,NULL,NULL);
			my_alert();
			}
		raise_error();
		}
}

/**************************************************/

short read_scsi_tape(scsiID,sn,buffer,n_sect)
/* for sequential-access devices, i.e. tape units */
/* it returns the number of 512 bytes sectors which were successfully read, and
may alter *sn if the best base is not the requested one, anyway it's guaranteed that
the first requested sectors is in the buffer, except in case of error.
That's very different from read_sectors and read_scsi_random, for which
it's the caller who chooses the sectors to be returned, but variable-blocks
tapes oblige to do that...
*/
short scsiID;
sector_t *sn;
char* buffer;
short n_sect;
{
short n_read,n_to_read;

#ifdef CHEAT_TAPE
return 0;	/*!!!!!*/
#endif

/*dp("richiesta di settore %ld buffer=%lx\n",*sn,buffer);*/

if(*sn<di.v.si.current_tape_block){
	/* may solve by rewinding, or a backwards skip, but after asking the
	user for confirmation, since it's a costly operation. 
	*/
	reposition_tape();
	}

if(*sn>di.v.si.current_tape_block){
	sector_t l;
	/* skip.... if available, use a SPACE command for long moves over fixed-size
	blocks, since that should be faster. And a tar archive is always written
	in fixed-size blocks */

	if(di.v.si.block_mode!=variable_blocks && ((supp_ops&skip_block_forward) ||
	    !(unsupp_ops&skip_block_forward) && (
	    (di.v.si.supported_group0==0 || SUPPORTED(di.v.si.supported_group0,SCSI_SPACE))))){
		l=(*sn-di.v.si.current_tape_block)/(di.v.si.block_size>>9); /* numero di blocchi da
			passare */
		if(l){
			tape_skip(scsiID,0,l);
			/* ci vorrebbe una interpretazione dell'errore, ad es. EOM o filemark... */
			if(err_code){
				/*if(expert_mode)*/ printf("Fast forward movement failed\n");
				return 0;
				}
			di.v.si.current_tape_block += l*(di.v.si.block_size>>9);
			di.v.si.tape_block_number+=l;
			}
		}
	while(*sn>di.v.si.current_tape_block){
		l=di.v.si.current_tape_block;	/* so the recursive call will not
			enter into this if(*sn>di.v.si.current_tape_block), but will
			proceed with an actual read... */
		n_read=read_scsi_tape(scsiID,&l,buffer,n_sect);
		if(err_code){
			if(n_read) *sn=l;	/* la base per quelli che ho letto */
			return n_read;
			}
		if(l+n_read>*sn){	/* il settore richiesto
				� nel buffer ma non � al primo posto, comunque devo ritornare
				con quello che ho */
			*sn=l;
			return n_read;
			}
		}
	}

/* OK, now the head is in the right place: perform the actual read */

if(di.v.si.block_mode!=variable_blocks){
	*sn -= *sn%(di.v.si.block_size>>9);	/* can't read less than one block */
	n_to_read=n_sect-n_sect%(di.v.si.block_size>>9);	/* nor a non-integer number of blocks.
			Anyway, I can't read more data then requested, hence I can't preserve the
			final sector number if the original data were not aligned: the interface rules of
			this routine guarantee that *sn is read, but say nothing about *sn+n_sect-1 */

	if(verbosity>=2) printf("reading sectors %ld to %ld\n",(long)*sn,(long)*sn+n_to_read-1);
	n_read=0;
	if(di.v.si.block_mode!=software_fixed_blocks && (supported_ops &multiple_blocks)){
		do{
			if(n_to_read<=tar_block_size){
				n_sect=read_many_fixed_blocks(scsiID,buffer,n_to_read);
				n_to_read=0;
				}
			else{
				n_sect=read_many_fixed_blocks(scsiID,buffer,tar_block_size);
				buffer+= (long)n_sect<<9;
				n_to_read -= n_sect;
				}
			n_read+=n_sect;
			}
		while(n_to_read && !err_code);
		}
	else{
		do{
			/* read blocks one by one (may return less than the requested number if EOF
			is reached, and it's easier and safer to count the blocks if they're read one by one) */
			n_sect=read_one_fixed_block(scsiID,buffer);
			n_read+=n_sect;
			buffer+= (long)n_sect<<9;
			}
		while(n_read<n_to_read &&!err_code);
		}
	return n_read;
	}
else{

	return read_one_variable_block(scsiID,buffer,n_sect);
	}

}


/***************************************************************/


static long guess_transfer_length(long trlen)
{
/* The reason for this routine is a big problem with the SCSI manager.
It was conceived to work with hard disks, which have a fixed block size,
and usually from a device driver, which knows a lot of informations about
the device. So Apple never introduced a way to transfer a number of bytes
"until my buffer is full or the device stops to send data, whichever comes
first", though such operation is considered one of the most common ones
in the SCSI-1 documents. That is, you can do that through a SCSIRead
but then you get a "scPhaseErr" error, and there is no place in which
the SCSI manager tells you how many bytes were really transferred.
  I thought I'd found a way to get that information, but Apple has never
told about that, and I discovered that on some newer Macs my method does
not work, and I get a return value "0 bytes transferred". In such a case
I must rely on other methods to get the correct value. See how I rewrote
do_request_sense. Unfortunately I can't do the same for data blocks from
a tape unit.
  Obviously a good way to solve the problem would be to support the new
SCSI manager 4.3, but:
a) there will always be machines which don't support it, and suntar wants
   to run on all Macs.
b) it does not run on our own machine, and currently we've no time to test
   suntar elsewhere
So, this routine must exist, hoping that it will be used in as fewer cases
as possible.
*/
if(sense_err_code==0 && (req_sense_response[0]&0xFE)==0xF0){ /* extended format, valid bit=1 */
	if( (req_sense_response[2]&0x20)) {
		long l;
		mcopy((char*)&l,&req_sense_response[3],4);
				/* "The difference (residue) of the requested length minus the actual
				length in bytes. Targets that do not support negative residues 
				shall set the ILI bit to one and the residue to zero" */
		if(l) return trlen-l;
		}
	else
		return trlen;	/* la dimensione richiesta era quella giusta
						-- no ILI, correct size */
	}
else if(sense_err_code==-2 || err_code==0)	/* no CHECK condition, transferred as many byte as requested */
	return trlen;
return 0;
}


static short read_one_variable_block(scsiID,buffer,n_sect)
short scsiID;
char* buffer;
short n_sect;
{
/* variable_blocks: read only one because
	a) usually one block is big enough
	b) how may I know whether the following block will fit in the
	   buffer but not together with the previous one?
	c) for tar archives block_mode should be set to software_fixed_blocks, so this
	   should happen only for non-tar tapes (and suntar can't do much on non-tar
	   tapes...)
	*/
byte scsiCMD[6];
short err,returnedStat,returnedMessage;
short n_tent=0;
long trlen;


trlen=(long)n_sect<<9;	/* size of allocated buffer */
if(unsupported_ops&standard_variable_blocks){	/* uff, posso sbagliarmi ma credo che
							questo non abbia pi� senso, derivava da una errata
							interpretazione di alcuni messaggi di errore, del
							resto non � mai stato provato.
							Comunque, tanto vale lasciarlo, per il caso un giorno
							possa rivelarsi utile... */
	do_mode_sense(scsiID,buffer,0);
	buffer[8]=0;
	if(!err_code && *(long*)&buffer[8]>=512){
		trlen= *(long*)&buffer[8];
		reinit_buffering(trlen);
		}
	}

scsiCMD[0] = SCSI_READ;
scsiCMD[1] = 0;	/* variable */
scsiCMD[2] = lowbyte(hiword(trlen));
scsiCMD[3] = highbyte(loword(trlen));
scsiCMD[4] = lowbyte(loword(trlen));
scsiCMD[5] = 0;
di.v.si.block_size=0;

*(long*)(buffer+512-sizeof(long)) = 0x15F671A3;	/* see below... */

err=do_scsi_command_in(scsiID,&scsiCMD,buffer,trlen,&returnedStat,
	&returnedMessage,&di.v.si.block_size,readVariableBlockTimeout);	/* variable */

/*di.v.si.block_size=0;	/*debug: used to test guess_transfer_length*/

if(err==noErr && (returnedStat&stat_mask)==CHECK_CONDITION)
	sense_err_code=fill_sense_data();
else
	sense_err_code=-2;

if(di.v.si.block_size==0 && *(long*)(buffer+512-sizeof(long)) != 0x15F671A3)
	di.v.si.block_size=guess_transfer_length(trlen);

if(di.v.si.block_size) di.v.si.tape_block_number++;
PRINT_TAPE_SENSE
CLEAR_RECOVERED_ERROR

if(verbosity>=2) printf("one variable block of %ld bytes\n",di.v.si.block_size);

if(err)
	err_code=err;
else if(err_code)
	err_code=scsiOverrunErr;
else if(returnedMessage)
	err_code=scsiBadStatus;
if(returnedStat && di.v.si.block_size && !err_code){
	/* further investigations to discover whether it was an error */
	if((returnedStat&stat_mask)==CHECK_CONDITION){
		long l;
		mcopy((char*)&l,&req_sense_response[3],4);
			/* "The difference (residue) of the requested length minus the actual
			length in bytes. Targets that do not support negative residues 
			shall set the ILI bit to one and the residue to zero" */

		if(!err_code && (req_sense_response[0]&0xFE)==0xF0 && /* extended format, valid bit=1 */
		   (req_sense_response[2]&0x20)) { /* incorrect length indicator */
			if(l>0 && l==trlen-di.v.si.block_size){	/* the block was correctly
				read but it was shorter than the requested size */
				/* do nothing: don't set err_code */
				}
			else{
				if(l<=0 && trlen==di.v.si.block_size) /* ha trasferito i bytes richiesti
					ma non sono ancora tutti; anche l=0 perch� se ILI � settato vedi
					specifiche sopra su cosa significa */
					error_message("Error: buffer too small, please increase its size (by allocating more memory to suntar) and retry\n");
				err_code=ioErr;
				}
			}
		else
			if(!err_code) err_code=ioErr;
		}
	else
		if(!err_code) err_code=-1;
	}

di.v.si.current_tape_block += di.v.si.block_size>>9;
if(err_code || di.v.si.block_size==0){
	if(sense_err_code==-2)
		sense_err_code=fill_sense_data();

	if((req_sense_response[0]&0x7E)==0x70 && (req_sense_response[2]&0xC0) /* EOM or filemark */
		|| (req_sense_response[0]&0x7E)!=0x70 && di.v.si.block_size /* the End Of Medium
				bit is only in the extended request sense format,
				probably every tape unit will use extended format, but if it does not
				and if some data are available it's safer to tell
				that they are there, and the only way is to report an EOF */
		){
		err_code=eofErr;
		if(di.v.si.block_size&511)
			error_message(BLOCK_NOT_MULTIPLE);
		return di.v.si.block_size>>9;
		}
	else
		return 0;
	}
if(di.v.si.block_size&511)
	error_message(BLOCK_NOT_MULTIPLE);

return di.v.si.block_size>>9;
}

/****************/

static short read_many_fixed_blocks(scsiID,buffer,n_to_read)
short scsiID;
char* buffer;
short n_to_read;
{
/* leggo tutti in una botta sola */
byte scsiCMD[6];
short err,returnedStat,returnedMessage;
short nb;
long trlen;

scsiCMD[0] = SCSI_READ;
scsiCMD[1] = 1;	/* fixed */
scsiCMD[2] = 0;	/* a three-bytes count is too much anyway */
nb=n_to_read/(short)(di.v.si.block_size>>9);
scsiCMD[3] = highbyte(nb);
scsiCMD[4] = lowbyte(nb);	/* all the requested blocks */
scsiCMD[5] = 0;
if(verbosity>=2) printf("%d fixed blocks of %ld bytes\n",nb,di.v.si.block_size);

*(long*)(buffer+512-sizeof(long)) = 0x15F671A3;	/* see below... */

/*dp("chiedo %ld bytes\n",(long)n_to_read<<9);*/
err=do_scsi_command_in(scsiID,&scsiCMD,buffer,(long)n_to_read<<9,
	&returnedStat,&returnedMessage,&trlen,readFixedBlockTimeout+(long)nb*extraTimeout);	/* fixed multiple */
if(err==noErr && (returnedStat&stat_mask)==CHECK_CONDITION)
	sense_err_code=fill_sense_data();
else
	sense_err_code=-2;
PRINT_TAPE_SENSE
CLEAR_RECOVERED_ERROR

/* "the target shall set the valid bit to one and the information bytes
to the difference (residue) between the requested transfer length and
the actual number of blocks successfully read". Cio�, il numero � in
blocchi, non in bytes, non posso chiamare guess_transfer_length... */

if(trlen==0){
	if(sense_err_code==0 && (req_sense_response[0]&0xFE)==0xF0 ){ /* extended format, valid bit=1 */
		if((req_sense_response[2]&0x20)) {
			long l;
			mcopy((char*)&l,&req_sense_response[3],4);
			if(l) trlen= (nb-l)*di.v.si.block_size;
			}
		else{
			if(*(long*)(buffer+512-sizeof(long))!=0x15F671A3)
				trlen=nb*di.v.si.block_size;	/* the contents of the buffer was
										overwritten, hence I may think that trlen=0
										is incorrect, due to the behaviour of the
										SCSI manager with a chip supporting DMA */
			}
		}
	else if(sense_err_code==-2 || err_code==0 && *(long*)(buffer+512-sizeof(long))!=0x15F671A3 )
		trlen=nb*di.v.si.block_size;
	}

if(!err && !err_code && !returnedMessage &&
   (returnedStat&stat_mask)==CHECK_CONDITION &&
   trlen>= di.v.si.block_size && trlen!=((long)n_to_read<<9)){
	if(!sense_err_code && ((req_sense_response[0]&0x7E)==0x70 &&
		(req_sense_response[2]&0xC0) /* EOM or filemark */
		|| (req_sense_response[0]&0x7E)!=0x70)){
		/* � arrivato qualcosa ma non quello che ho chiesto, sono a EOF
		e quindi deduco che il motivo � quello */
		if(verbosity>=1) print_request_sense_response(req_sense_response);
		returnedStat=0;
		di.v.si.current_tape_block+=trlen>>9;
		err_code=eofErr;
		return trlen>>9;
		}
	}

if(err)
	err_code=err;
else if(err_code)
	err_code=scsiOverrunErr;
else if(returnedStat||returnedMessage)
	err_code=scsiBadStatus;

if(err_code ){
	/* comunque non mi � arrivato quanto mi aspettavo. Ma potrebbe
	essermi arrivata della roba buona, e questa roba buona
	potrebbe contenere la fine dell'archivio cos� che non
	mi verr� mai chiesto di leggere dopo di essa */

	di.v.si.current_tape_block = 0x7FFFFFFF;	/* non posso essere certo della
				posizione */
	return trlen>>9;
	}

/* ok, � arrivato esattamente quel che volevo */

di.v.si.current_tape_block += trlen>>9;

return trlen>>9;
}

/*********/

static short read_one_fixed_block(scsiID,buffer)
short scsiID;
char *buffer;
{
byte scsiCMD[6];
short err,returnedStat,returnedMessage;

/*set up the command buffer with the READ command*/

scsiCMD[0] = SCSI_READ;
scsiCMD[5] = 0;

if(di.v.si.block_mode!=software_fixed_blocks){	/* fixed, da leggere un blocco per volta */
	scsiCMD[1] = 1;	/* fixed */
	scsiCMD[2] = 0;
	scsiCMD[3] = 0;
	scsiCMD[4] = 1;	/* one block */
	if(verbosity>=2) printf("one fixed block of %ld bytes\n",di.v.si.block_size);

	err=do_scsi_command_in(scsiID,&scsiCMD,buffer,di.v.si.block_size,
		&returnedStat,&returnedMessage,NULL,readFixedBlockTimeout);	/* fixed */
		/* non so se metterci &trlen anche qui: essendo un singolo
		fixed block solo errori molto gravi possono impedire di
		ritornare le dimensioni previste, ma comunque 512 bytes di dati
		buoni possono esserci e possono essere utili: ma e se invece
		si trattasse di un CRC error i dati possono esserci ma
		essere sbagliati ! ma forse allora sarebbero tutti comunque...*/
	if(err==noErr && (returnedStat&stat_mask)==CHECK_CONDITION)
		sense_err_code=fill_sense_data();
	else
		sense_err_code=-2;
	PRINT_TAPE_SENSE
	CLEAR_RECOVERED_ERROR
	if(err!=scsiSelectTOErr && err_code!=scPhaseErr)
		di.v.si.current_tape_block+= di.v.si.block_size>>9;

	}
else{	/* software fixed, but variable from the point of view of the device */
	/*short n_tent=0;*/

	scsiCMD[1] = 0;	/* variable */
	scsiCMD[2] = lowbyte(hiword(di.v.si.block_size));
	scsiCMD[3] = highbyte(loword(di.v.si.block_size));
	scsiCMD[4] = lowbyte(loword(di.v.si.block_size));

	err=do_scsi_command_in(scsiID,&scsiCMD,buffer,di.v.si.block_size,&returnedStat,
		&returnedMessage,NULL,readVariableBlockTimeout);	/* software fixed */

	if(err!=scsiSelectTOErr && err_code!=scPhaseErr)
		di.v.si.current_tape_block += di.v.si.block_size>>9;

	if(err==noErr && (returnedStat&stat_mask)==CHECK_CONDITION)
		sense_err_code=fill_sense_data();
	else
		sense_err_code=-2;

	PRINT_TAPE_SENSE
	CLEAR_RECOVERED_ERROR
	if(!err && (!err_code||err_code==scPhaseErr) && !returnedMessage &&
	   (returnedStat&stat_mask)==CHECK_CONDITION ){
		long l;
		mcopy((char*)&l,&req_sense_response[3],4);

		if(!sense_err_code && (req_sense_response[0]&0xFE)==0xF0 && /* extended format, valid bit=1 */
		   (req_sense_response[2]&0x20) ){	/* incorrect length indicator */
			err_code=scsiOverrunErr;
			if(di.v.si.current_tape_block==0 || verbosity){	/* sul primo blocco cerco di spiegare all'utente
						che deve fare per rimediare all'errore */
				if(l<=0){
					printf("The block size parameter is too small");
					if(l<0)
						printf(" (current=%ld correct value=%ld)",
							di.v.si.block_size,di.v.si.block_size-l);
					vai_a_capo();
					}
				else
					printf("The block size parameter is too big (current=%ld correct value=%ld)\n",
						di.v.si.block_size,di.v.si.block_size-l);
				}
			}
		}
	}

if(err)
	err_code=err;
else if(err_code)
	err_code=scsiOverrunErr;
else if(returnedStat||returnedMessage)
	err_code=scsiBadStatus;

if(!err_code) return di.v.si.block_size>>9;

if(sense_err_code==-2)
	sense_err_code=fill_sense_data();

if((req_sense_response[0]&0x7E)==0x70 && (req_sense_response[2]&0xC0) /* EOM or filemark */
	|| (req_sense_response[0]&0x7E)!=0x70 )
	err_code=eofErr;
return 0;
}


/**************************************************************************/

#define VM_COMPATIBLE
#ifdef VM_COMPATIBLE


void do_scsi_message(scsiID,msg)
short scsiID, msg;
{
short stat,mess;

if(verbosity>=3){
	start_of_line();
	printf("SCSI msg: %x\n",msg);
	}

if(gHasVirtualMemory){
	HoldMemory((char*)&scsiID -100, (char*)&msg +sizeof(msg) -((char*)&scsiID -100) );	/* all
			the parameters and the local variables, plus 100 bytes for hidden variables
			which the compiler may create and stack for the SCSI manager (which however
			should work on the supervisor stack) */
	/*
	void start_scsi_read(void),end_scsi_read(void);
	HoldMemory(start_msg,(char*)end_msg-(char*)start_msg);
	asm{
		extern start_msg:
		}
	oh, no! I can't do that. Think C lets C see only "extern" asm labels, which
	have a place in the jump table, so that I would Hold a part of jump table.
	If I want to Hold the code itself, I must do that in asm.
	N.B.: in most cases, it would be enough to call a "do nothing" function
	placed at the end of this one: any virtual memory avoids to swap out pages used
	very very recently and if this function is smaller than a page, having
	just executed its first instruction and some code placed just after its end
	guarantees that the whole function body is in recently accessed pages and will
	not be swapped out in the immediate future. But that's a dirty trick and
	since I'm doing this on a LC (68020, no MMU, no chance to have virtual memory)
	I'm not interested in how fast my code is under virtual memory, so I do
	the things in the "officially correct" way of using HoldMemory
	(by the way, also the stack need not be Held, since it was recently used. If the
	"do nothing" functions declares a 100-bytes array and accesses the first
	element of it, I've Hold also enough stack for the called traps. And the buffer
	need be Held only if it's so big that not all of it was filled "recently". If,
	however, the buffer is very large, you MUST Hold it to be sure that every byte of
	it is in memory, otherwise part of it may have been swapped out in order
	to load in memory another part)
	*/
	asm{
		LEA		@start_msg,A0		/* first parameter, the address */
		/*MOVE.L	#(@end_msg-@start_msg),A1	second parameter, the size:
		but the Think C does not allow this, it's not a standard assembler ! */
		LEA		@end_msg,A1
		SUB.L	A0,A1
		MOVEQ	#0,D0		/* hold memory: selector in D0 */
		/* _Debugger / * hey, I must verify that this time the assembler has done
		what I wanted */
		DC.W	0xA05C		/* hold memory: dispatch trap */
		@start_msg:
		}
	}


if(!SCSIGet() && !SCSISelAtn(scsiID) ){
	err_code=SCSIMsgOut(msg); /* ABORT 06H  DEVICE RESET 0CH */
	if(!err_code){
		err_code=SCSIComplete(&stat, &mess, 15);
		if(err_code==2 && (SCSIStat()&0x42)){
			SCSIReset();
			printf(BUS_RESET);
			err_code=scsiBusResetErr;
			}
		else if(err_code)
			err_code=-1;
		}
	}
else{
	err_code=scsiSelectTOErr;
	}


if(gHasVirtualMemory){
	asm{
		@end_msg:
		LEA		@start_msg,A0
		LEA		@end_msg,A1
		SUB.L	A0,A1
		MOVEQ	#1,D0		/* unhold memory: selector in D0 */
		DC.W	0xA05C		/* unhold memory: dispatch trap */
		}
	/*UnholdMemory(start_msg,(char*)end_msg-(char*)start_msg);*/
	UnholdMemory((char*)&scsiID -100, (char*)&msg +sizeof(msg) -((char*)&scsiID -100) );
	}
if(err_code==scsiBusResetErr)
	do_request_sense(scsiID,req_sense_response);	/* per tirarlo fuori dallo
			stato di unit attention */
}


static short do_scsi_command_in(scsiID,scsiCMD,buffer,buf_size,returnedStat,returnedMessage,out_size,timeout)

/* inspired to the snippet "SCSIinquiry.p" , � Apple Computer Inc. DTS; a very similar
routine is also in one of the "New Technical Notes" ("Fear no SCSI")
But our implementation is much improved: it supports Virtual Memory and
handles the situations when the device locks the SCSI bus, and handles
a phaseErr from SCSIRead (inevitable when I don't know in advance
the size of the response).

out_size=NULL => don't accept a different number of bytes
out_size=-1   => accept and discard any data bytes, in such a case the
				 buffer must be 512 bytes long
else out_size is used to return the effective data size (however, on some Macs
			that does not work: suntar 2.0b12 relied a lot on this feature,
			but I've tried to avoid it in 2.01)


May be used for all commands requiring a data in phase (or at least not requiring a
data out phase, e.g. a command without extra parameters such as REWIND)

returns 4 error codes (the return value, the global err_code and the two
parameters returnedStat,returnedMessage). Since interpreting them is
not easy, currently I don't do that, anyway they are there
(Really, in case of error I may get further status information by a
REQUEST SENSE: that makes things even more complex)
*/

byte scsiCMD[];
short scsiID,*returnedStat,*returnedMessage;
long buf_size;
char *buffer;
unsigned long* out_size;
unsigned long timeout;
{
	struct lvr{
		unsigned long waitTime;
		long transf_eff_size;
		SCSIInstr TIB[3];
		short numTries;
		char cmd[12];
		short stat,mess;
		short retcode,transf_err;
		short err;
		Boolean gotSCSIBus,timedOut;
		} lv;

/* stack:

	SP -> ......
		  local variables (lv)
		  ......
	FP -> ......
		  ret addr
		  scsiID
		  scsiCMD
		  buffer
		  buf_size
		  returnedStat
		  returnedMessage
		  out_size
		  timeout
		  ......
*/

/*dp("cmd=%x\n",scsiCMD[0]);*/

	if(verbosity>=3){
		disable_autoflush(1);
		start_of_line();
		printf("SCSI cmd=");
		for(lv.err=0;lv.err< ((scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10) ; lv.err++)
			printf(" %2x",(unsigned char)scsiCMD[lv.err]);
		vai_a_capo();
		if(timeout>60) flush_console();
		}
	if(gHasVirtualMemory){	/* obviously I can't cause a page fault while I'm
			accessing the SCSI bus, the page replacement should access the hard
			disk through the SCSI bus, which is currently busy */
		*(long*)&lv.cmd[0]=*(long*)&scsiCMD[0];	/* don't worry about alignment, VM implies */
		*(long*)&lv.cmd[4]=*(long*)&scsiCMD[4];	/* at least a 68020 */
		*(long*)&lv.cmd[8]=*(long*)&scsiCMD[8];
		scsiCMD=&lv.cmd;	/* it's simpler it copy it than to lock the original */
		HoldMemory((char*)&lv -100, (char*)&timeout +sizeof(timeout) -((char*)&lv -100) );	/* all
			the parameters and the local variables, plus 100 bytes for hidden variables
			which the compiler may create and stack for the SCSI manager (which however
			should work on the supervisor stack) */
		if(buffer) HoldMemory(buffer,buf_size);
		/*HoldMemory(start_scsi_read,(char*)end_scsi_read-(char*)start_scsi_read);*/
		asm{	/* see comments in the previous function */
			LEA		@start_scsi_read,A0
			LEA		@end_scsi_read,A1
			SUB.L	A0,A1
			MOVEQ	#0,D0
			DC.W	0xA05C
			}
		}

	lv.transf_eff_size=0;
	lv.transf_err=noErr;

		/*set up the TIBs*/

	/*scInc=move count (scParam2) bytes to/from buffer (scParam1), increment bufr*/
	lv.TIB[0].scOpcode = scInc;
	lv.TIB[0].scParam1 = (unsigned long)buffer; /*pointer to buffer*/
	lv.TIB[0].scParam2 = buf_size; /*"count", or no. of bytes to move*/

	/*scStop=stop the pseudo-pgm, rtn to calling SCSI Mgr rtne*/
	lv.TIB[1].scOpcode = scStop;
	lv.TIB[1].scParam1 = 0L;
	lv.TIB[1].scParam2 = 0L;

	if( out_size == (long*)-1L){
		/* ciclically rewrite the dato unto the same buffer, until a transfer error
		occurs (typically scPhaseErr meaning no more data is coming from the device)
		Useful to skip a number of blocks when the SPACE command is not supported
		(with one more command one may also get the actual size of the data) */
		lv.TIB[0].scOpcode = scNoInc;
		lv.TIB[1].scOpcode = scLoop;
		lv.TIB[1].scParam1 = -10L;
		lv.TIB[1].scParam2 = 0x7FFFFFFFL;	/* practically infinite */

		lv.TIB[2].scOpcode = scStop;
		lv.TIB[2].scParam1 = 0L;
		lv.TIB[2].scParam2 = 0L;
		}

		/*try to arbitrate and select.  To fall out of the repeat loop, we*/
		/*have to sucessfully do both, or time out some number of times*/
		/*(at which point we clean up and quit)*/

	lv.gotSCSIBus = false;
	lv.timedOut = false;
	lv.numTries = 0;

	asm{
		@start_scsi_read:
		}

	do{
		lv.err = SCSIGet();
		if(lv.err == noErr){
			lv.waitTime=Ticks;	/* Ticks should be Held in memory anyway */
								/* really, under A/UX Ticks is not implemented.
								But neither the SCSI manager is, so that does
								not matter */
			lv.err = SCSISelect(scsiID);
			lv.waitTime=Ticks-lv.waitTime;
			if(lv.waitTime>8) lv.numTries+=2;	/* SCSISelect has an internal timeout of
					1/4 sec (not changeable): it that's the cause of the failure,
					don't try so many times as if the cause is immediate */

			if(lv.err == noErr)
				lv.gotSCSIBus = true;
			}

		/*only tolerate so many errors trying to get and/or select*/
		if(++lv.numTries >= 5)
			lv.timedOut = true;
		}
	while(!( lv.gotSCSIBus || lv.timedOut));

	if(lv.gotSCSIBus){
		/* the size of the command is computed from the command group, that is its
		three most significant bits */
		lv.err = SCSICmd(scsiCMD, (scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10);
		if(lv.err == noErr){
			if(buf_size==0)
				lv.transf_err=noErr;	/* questo comando non richiede dei dati indietro */
			else{
				lv.transf_err = SCSIRead(lv.TIB);
				/* note that if I asked more bytes than available I get an scPhaseErr:
				the routine from Apple knew that INQUIRY has a variable response size
				and expected error 5: anything else, even a noErr code, was considered
				bad. It's certainly better to expect no error and solve in another way
				the problem with variable sized responses (see do_scsi_inquiry), but
				when that can't be done, at least the caller must know how many
				bytes were effectively transferred */
				lv.transf_eff_size=lv.TIB[0].scParam1-(unsigned long)buffer;
				if(out_size && lv.transf_eff_size && lv.transf_err==scPhaseErr ) lv.transf_err=noErr;
				}
			lv.retcode=noErr;
			}
		else{
			/* una request sense per saperne di pi�, ma dopo la SCSIComplete..... */
			lv.retcode=ioErr;	/* errore nel comando */
			}
		}
	else
		lv.retcode=scsiSelectTOErr; /* scsiSelTO exceeded (selection failed) */

	lv.err = SCSIComplete(&lv.stat, &lv.mess, timeout);	/* which, damn it,
		waits for 15 ticks even if I'm passing a waitTime=4: it waits twice,
		for the status and the message, and only the first timeout is configurable */
	/*if(lv.err && !(SCSIStat()&0x42) ) lv.err=0; no, sui Quadra SCSIStat
	non � affidabile */
	if(lv.err==2){
		lv.waitTime=60;
		lv.err = SCSIComplete(&lv.stat, &lv.mess, lv.waitTime);
		/*if(err_code && !(SCSIStat()&0x42) ) lv.err=0; */
		if(lv.err==2 && lv.gotSCSIBus){
			lv.stat=0; lv.mess=0;
				/* to reset the bus may cause bad things: every device on the bus is
				resetted ! All nonvolatile parameters will be resetted to their
				initial values, and if the device driver does not work in those
				conditions and does not try to restore the values it expects...
				And obviosly, some operations may be interrupted, e.g. a printer
				may stop printing, or may lose a few pages and than continue
				printing with a different configuration...
				Hence, I must try to avoid resetting the bus, and a possible way is
				to persuade the unit that the last command was good, by sending
				a good command: */
			lv.numTries=1;
			do{
				if(lv.numTries==1){
					lv.TIB[0].scOpcode = scNoInc;
					lv.TIB[0].scParam1 = (long) &lv.waitTime;
					lv.TIB[0].scParam2 = 4;
					lv.TIB[1].scOpcode = scStop;
					lv.err=scsiCMD[0];	/* non devo perderlo */
					scsiCMD[0] = REQUEST_SENSE;
					scsiCMD[1] &= 0x1F;	/* preserva il LUN */
					scsiCMD[2]=scsiCMD[3]=scsiCMD[5]=0;
					scsiCMD[4] = 4;

					if(!SCSIGet() && !SCSISelect(scsiID) && !SCSICmd(scsiCMD, 6) )
						SCSIRead(lv.TIB);
					scsiCMD[0]=lv.err;	/* needed for a test later... */
					}
				else if(gMac.machineType!=env512KE && gMac.machineType!= envMacPlus){
					/* these SCSI calls were introduced with the 256k ROMs, and
					since they are selectors and not Traps and since one gets
					a system error using them when they are not supported I can't
					see a more elegant way to test for their availability */
					if(!SCSIGet() && !SCSISelAtn(scsiID) )
						SCSIMsgOut(lv.numTries==2?0x6:0xC); /* ABORT 06H  DEVICE RESET 0CH */
					else
						continue;	/* don't do SCSI complete if nothing happened */
					}
				else
					break;
				lv.waitTime=40;
				lv.err = SCSIComplete(&lv.stat, &lv.mess, lv.waitTime);
				/*if(lv.err && !(SCSIStat()&0x42) ) lv.err=0; */
				}
			while(lv.err==2 && ++lv.numTries<=3);
				
			if(lv.err==2){
				SCSIReset();	/* the Apple tape unit does not release the bus it the
							data transfer length is not correct, other units do not
							release it if the command is not supported (that happens also
							with units which do not include the list of supported commands
							in the response to INQUIRY), without a Reset the
							bus remains occupied and no SCSI tranfer is possible
							(since without the hard disk almost nothing may be done, the
							machine appears locked) */
				printf(BUS_RESET);
#ifdef SUNTAR
				if(timeout>60) printf("Note: if the unit was working correctly, maybe the timeout (%ld sec) was too short\n",timeout/60);
				if( scsiCMD[1] & 0xE0)	/* logical unit !=0, I use that
										only during Drive List !!! */
					printf(SET_UNSUPP_14);
#endif
				lv.retcode=scsiBusResetErr;
				}
			else{

				if(lv.numTries>=3){
					printf(DEVICE_RESET);
					if( scsiCMD[1] & 0xE0)
						printf(SET_UNSUPP_14);
					}
				else if( !(scsiCMD[1] & 0xE0))
					printf(UNLOCKED_BUS);
				lv.retcode=scsiBadPBErr;
				}
			}
		}

	if(gHasVirtualMemory){
		asm{
			@end_scsi_read:
			LEA		@start_scsi_read,A0
			LEA		@end_scsi_read,A1
			SUB.L	A0,A1
			MOVEQ	#1,D0
			DC.W	0xA05C
			}

		/*UnholdMemory(start_scsi_read,(char*)end_scsi_read-(char*)start_scsi_read);*/
		UnholdMemory((char*)&lv -100, (char*)&timeout +sizeof(timeout) -((char*)&lv -100) );
		if(buffer) UnholdMemory(buffer,buf_size);
		}
	if(verbosity>=3 && lv.transf_eff_size<128 && out_size!=(long*)-1L){
		printf("answer=");
		if(!lv.transf_eff_size){
			if(scsiCMD[0]==TEST_UNIT_READY && lv.stat>=0)
				printf(lv.stat ? "not ready" : "ready" );
			else
				printf("nothing");
			}
		else
			for(err_code=0;err_code< lv.transf_eff_size ; err_code++)
				printf(" %2x",(unsigned char)buffer[err_code]);
		vai_a_capo();
		}

	if(verbosity >= 3 && (lv.stat||lv.mess||lv.transf_err||lv.err))
		printf("stat=%x, mess=%x errRd=%d err=%d\n", lv.stat, lv.mess,lv.transf_err,lv.err);

	*returnedStat=lv.stat;
	*returnedMessage=lv.mess;
	err_code=lv.transf_err;
	if(out_size!=NULL && out_size!=(long*)-1L) *out_size=lv.transf_eff_size;
	if(lv.err&&lv.retcode==noErr) return ioErr;

 	if((long)out_size==-1L){
 		if( verbosity>=2) printf("%ld bytes skipped\n",(0x7FFFFFFFL-lv.TIB[1].scParam2)<<9);
		if(err_code==scPhaseErr) err_code=0;
		}
	if(lv.retcode==scsiBusResetErr && scsiCMD[0]!=REQUEST_SENSE){
		do_request_sense(scsiID,req_sense_response);	/* per tirarlo fuori dallo
			stato di unit attention. Ma non farlo se era stato un request sense
			a provocare il guaio */
		}

	return lv.retcode;
}


static short do_scsi_command_out(scsiID,scsiCMD,buffer,buf_size,returnedStat,returnedMessage,timeout)

/* Same but with a SCSIWrite, for commands requiring a Data Out phase
*/

byte scsiCMD[];
short scsiID,*returnedStat,*returnedMessage;
long buf_size;
char *buffer;
unsigned long timeout;
{
	struct lvw{
		unsigned long waitTime;
		SCSIInstr TIB[2];
		char cmd[12];
		short stat,mess;
		short retcode,transf_err;
		short err;
		short numTries;
		Boolean gotSCSIBus,timedOut;
		} lv;

	if(verbosity>=3){
		start_of_line();
		printf("SCSI cmd=");
		for(lv.err=0;lv.err< ((scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10) ; lv.err++)
			printf(" %2x",(unsigned char)scsiCMD[lv.err]);
		if(buf_size<512){
			printf("\ncmd data=");
			for(lv.err=0;lv.err<buf_size;lv.err++)
				printf(" %2x",(unsigned char)buffer[lv.err]);
			}
		vai_a_capo();
		}
	#ifdef DEBUG
	if(scsiID==0 ){
		dp("ma che diavolo combini ?\n");
		return;
		}
	#endif
#ifdef CHEAT_TAPE
err_code=0; *returnedStat=0; *returnedMessage=0;
printf("imbroglio, non faccio nulla (%ld)\n",(long)buf_size);
/*if(scsi[0]==SCSI_WRITE)*/ return 0;
#endif



	if(gHasVirtualMemory){
		*(long*)&lv.cmd[0]=*(long*)&scsiCMD[0];
		*(long*)&lv.cmd[4]=*(long*)&scsiCMD[4];
		*(long*)&lv.cmd[8]=*(long*)&scsiCMD[8];
		scsiCMD=&lv.cmd;
		HoldMemory((char*)&lv -100, (char*)&timeout +sizeof(timeout) -((char*)&lv -100) );
		HoldMemory(buffer,buf_size);
		/*HoldMemory(start_scsi_write,(char*)end_scsi_write-(char*)start_scsi_write);*/
		asm{	/* see comments in a previous function */
			LEA		@start_scsi_write,A0
			LEA		@end_scsi_write,A1
			SUB.L 	A0,A1
			MOVEQ	#0,D0
			DC.W	0xA05C
			}

		}

	lv.transf_err=noErr;
		/*set up the TIBs*/

	/*scInc=move count (scParam2) bytes to/from buffer (scParam1), increment bufr*/
	lv.TIB[0].scOpcode = scNoInc;
	lv.TIB[0].scParam1 = (unsigned long)buffer; /*pointer to buffer*/
	lv.TIB[0].scParam2 = buf_size; /*"count", or no. of bytes to move*/

	/*scStop=stop the pseudo-pgm, rtn to calling SCSI Mgr rtne*/
	lv.TIB[1].scOpcode = scStop;
	lv.TIB[1].scParam1 = 0L;
	lv.TIB[1].scParam2 = 0L;
		
		/*try to arbitrate and select.  To fall out of the repeat loop, we*/
		/*have to sucessfully do both*/

	lv.gotSCSIBus = false;
	lv.timedOut = false;
	lv.numTries = 0;

	asm{
		@start_scsi_write:
		}

#if 1
	do{
		lv.err = SCSIGet();
		if(lv.err == noErr){
			lv.waitTime=Ticks;
			lv.err = SCSISelect(scsiID);
			lv.waitTime=Ticks-lv.waitTime;
			if(lv.waitTime>8) lv.numTries+=2;

			if(lv.err == noErr)
				lv.gotSCSIBus = true;
			}

		if(++lv.numTries >= 5)
			lv.timedOut = true;
		}
	while(!( lv.gotSCSIBus || lv.timedOut));

#else
	lv.gotSCSIBus = false;
	lv.err = SCSIGet();
	if(lv.err == noErr){
		lv.err = SCSISelect(scsiID);
		if(lv.err == noErr)
			lv.gotSCSIBus = true;
		}
#endif

	if(lv.gotSCSIBus){
		/* the size of the command is computed from the command group, that is its
		three most significant bits */
		lv.err = SCSICmd(scsiCMD, (scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10);
		if(lv.err == noErr){
			lv.transf_err = SCSIWrite(lv.TIB);
			lv.retcode=noErr;
			}
		else
			lv.retcode=ioErr;
		}
	else
		lv.retcode=scsiSelectTOErr;	/* scsiSelTO exceeded (selection failed) */

	lv.err = SCSIComplete(&lv.stat, &lv.mess, timeout);
	/*if(lv.err && !(SCSIStat()&0x42) ) lv.err=0; */
	if(lv.err==2){
		lv.waitTime=60;
		lv.err = SCSIComplete(&lv.stat, &lv.mess, lv.waitTime);
		/*if(lv.err && !(SCSIStat()&0x42) ) lv.err=0;*/
		if(lv.err==2 && lv.gotSCSIBus){
			lv.stat=0; lv.mess=0;
			lv.retcode=scsiBusResetErr;
			lv.TIB[0].scOpcode = scNoInc;
			lv.TIB[0].scParam1 = (long) &lv.waitTime;
			lv.TIB[0].scParam2 = 4;
			lv.TIB[1].scOpcode = scStop;
			scsiCMD[0] = REQUEST_SENSE;
			scsiCMD[1]=scsiCMD[2]=scsiCMD[3]=scsiCMD[5]=0;
			scsiCMD[4] = 4;

			if(!SCSIGet() && !SCSISelect(scsiID) && !SCSICmd(scsiCMD, 6) )
				SCSIRead(lv.TIB);
			lv.waitTime=60;
			lv.err = SCSIComplete(&lv.stat, &lv.mess, lv.waitTime);


			if(lv.err==2){
				if(!SCSIGet() && !SCSISelAtn(scsiID) ){
					SCSIMsgOut(0xC);
					lv.err = SCSIComplete(&lv.stat, &lv.mess, lv.waitTime);
					if(lv.err!=2)
						printf(DEVICE_RESET);
					else{
						SCSIReset();
						printf(BUS_RESET);
						lv.retcode=scsiBusResetErr;
						}
					}
				}
			/*if(lv.err && !(SCSIStat()&0x42) ) lv.err=0;*/
			if(lv.err==2){
				SCSIReset();
				printf(BUS_RESET);
				lv.retcode=scsiBusResetErr;
				}
			else{
				printf(UNLOCKED_BUS);
				lv.retcode=scsiBadPBErr;
				}
			}
		}

	if(gHasVirtualMemory){
		asm{	/* see comments in a previous function */
			@end_scsi_write:
			LEA		@start_scsi_write,A0
			LEA		@end_scsi_write,A1
			SUB.L 	A0,A1
			MOVEQ	#1,D0
			DC.W	0xA05C
			}

		/*UnholdMemory(start_scsi_write,(char*)end_scsi_write-(char*)start_scsi_write);*/
		UnholdMemory((char*)&lv -100, (char*)&timeout +sizeof(timeout) -((char*)&lv -100) );
		UnholdMemory(buffer,buf_size);
		}
	if(verbosity >= 3 && (lv.stat||lv.mess||lv.transf_err||lv.err))
		printf("stat=%x, mess=%x errWr=%d err=%d\n", lv.stat, lv.mess,lv.transf_err,lv.err);

	/*printf("returnedStat, returnedMessage = %d %d\n\n", stat, message);*/
	*returnedStat=lv.stat;
	*returnedMessage=lv.mess;
	err_code=lv.transf_err;
	if(lv.err&&lv.retcode==noErr) return ioErr;

	if(err_code==scsiBusResetErr)
		do_request_sense(scsiID,req_sense_response);	/* per tirarlo fuori dallo
			stato di unit attention */


	return lv.retcode;
}

#if 0

void fast_scsi_scan(scsi_vector)
struct fastscantype *scsi_vector;
{
/* it looked a good idea: by violating one of Apple's rules about SCSI,
a scan of all SCSI IDs was twice as fast as the "standard" one. But since I've
installed System 7.1 and an external hard disk (which requires its own
device driver and refuses to work with others) my Mac froze so often during
a scan that I decided to remove this function... */
}
#endif

#else


� chiaro che se avviene un page fault mentre il bus SCSI � inutilizzabile da altri che me...

static short do_scsi_command_in(scsiID,scsiCMD,buffer,buf_size,returnedStat,returnedMessage,out_size)

/* inspired to the snippet "SCSIinquiry.p" , � Apple Computer Inc. DTS; a very similar
routine is also in one of the "New Technical Notes" ("Fear no SCSI")

May be used for all commands requiring a data in phase (or at least not requiring a
data out phase, e.g. a command without extra parameters such as REWIND)

returns 4 error codes (the return value, the global err_code and the two
parameters returnedStat,returnedMessage). Since interpreting them is
not easy, currently I don't do that, anyway they are there
(Really, in case of error I may get further status information by a
REQUEST SENSE: that makes things even more complex)
*/

byte scsiCMD[];
short scsiID,*returnedStat,*returnedMessage;
long buf_size;
char *buffer;
unsigned long* out_size;
{
	short err;
	unsigned long waitTime;
	Boolean gotSCSIBus,timedOut;
	short numTries;
	SCSIInstr TIB[2];

		/*set up the TIBs*/

	/*scNoInc=move count (scParam2) bytes to/from buffer (scParam1), don't increment bufr*/
	TIB[0].scOpcode = scInc;
	TIB[0].scParam1 = (unsigned long)buffer; /*pointer to buffer*/
	TIB[0].scParam2 = buf_size; /*"count", or no. of bytes to move*/

	/*scStop=stop the pseudo-pgm, rtn to calling SCSI Mgr rtne*/
	TIB[1].scOpcode = scStop;
	TIB[1].scParam1 = 0L;
	TIB[1].scParam2 = 0L;
		
		/*try to arbitrate and select.  To fall out of the repeat loop, we*/
		/*have to sucessfully do both, or time out some number of times*/
		/*(at which point we clean up and quit)*/

	gotSCSIBus = false;
	timedOut = false;
	numTries = 0;
	do{
		err = SCSIGet();
		if(err == noErr){
			/*waitTime=Ticks;*/
			err = SCSISelect(scsiID);
			/*waitTime=Ticks-waitTime;*/
			if(waitTime>8) numTries+=2;		/* SCSISelect has an internal timeout of
					1/4 sec (not changeable): it that's the cause of the failure,
					don't try so many times as if the cause is immediate */

			if(err == noErr)
				gotSCSIBus = true;
			}

		/*only tolerate so many errors trying to get and/or select*/
		if(++numTries >= 5)
			timedOut = true;
		}
	while(!( gotSCSIBus || timedOut));

	waitTime = 4;

	if(gotSCSIBus){
		/* the size of the command is computed from the command group, that is its
		three most significant bits */
		err = SCSICmd(scsiCMD, (scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10);
		if(err == noErr){
			if(buf_size==0)
				err=0;		/* questo comando non richiede dei dati indietro */
			else{
				err = SCSIRead(TIB);
				/* note that if I asked more bytes than available I get an scPhaseErr:
				the routine from Apple knew that INQUIRY has a variable response size
				and expected error 5: anything else, even a noErr code, was considered
				bad. It's certainly better to expect no error and solve in another way
				the problem with variable sized responses (see do_scsi_inquiry), but
				when that can't be done, at least the caller must know how many
				bytes were effectively transferred */
				if(out_size!=NULL){
					*out_size=TIB[0].scParam1-(unsigned long)buffer;
					if(*out_size && err==scPhaseErr ) err=noErr;
					}
				}
			err_code=err;
			}
		else{
			err = SCSIComplete(returnedStat, returnedMessage, waitTime);
			/* printf("SCSICmd err: %d\n", err); */
			return -2;
			}
		}

	err = SCSIComplete(returnedStat, returnedMessage, waitTime);	/* which, damn it,
		waits for 15 ticks even if I'm passing a waitTime=4 */
	/*printf("returnedStat, returnedMessage = %d %d\n\n", *returnedStat, *returnedMessage);*/
	if(!gotSCSIBus) return -3;

	return err ? -4 : 0;
}

static short do_scsi_command_out(scsiID,scsiCMD,buffer,buf_size,returnedStat,returnedMessage)

/* Same but with a SCSIWrite, for commands requiring a Data Out phase
*/

byte scsiCMD[];
short scsiID,*returnedStat,*returnedMessage;
long buf_size;
char *buffer;
{
	short err;
	unsigned long waitTime;
	Boolean gotSCSIBus;
	SCSIInstr TIB[2];

		/*set up the TIBs*/

	/*scNoInc=move count (scParam2) bytes to/from buffer (scParam1), don't increment bufr*/
	TIB[0].scOpcode = scInc;
	TIB[0].scParam1 = (unsigned long)buffer; /*pointer to buffer*/
	TIB[0].scParam2 = buf_size; /*"count", or no. of bytes to move*/

	/*scStop=stop the pseudo-pgm, rtn to calling SCSI Mgr rtne*/
	TIB[1].scOpcode = scStop;
	TIB[1].scParam1 = 0L;
	TIB[1].scParam2 = 0L;
		
		/*try to arbitrate and select.  To fall out of the repeat loop, we*/
		/*have to sucessfully do both*/

	gotSCSIBus = false;
	err = SCSIGet();
	if(err == noErr){
		err = SCSISelect(scsiID);
		if(err == noErr)
			gotSCSIBus = true;
		}

	waitTime = 4;

	if(gotSCSIBus){
		/* the size of the command is computed from the command group, that is its
		three most significant bits */
		err = SCSICmd(scsiCMD, (scsiCMD[0]&0xE0)==0 ? 6: (scsiCMD[0]&0xE0)==0xA0 ? 12 : 10);
		if(err == noErr){
			err_code = SCSIWrite(TIB);
			}
		else{
			err = SCSIComplete(returnedStat, returnedMessage, waitTime);
			/* printf("SCSICmd err: %d\n", err); */
			return -2;
			}
		}

	err = SCSIComplete(returnedStat, returnedMessage, waitTime);
	printf("returnedStat, returnedMessage = %d %d\n\n", *returnedStat, *returnedMessage);
	if(!gotSCSIBus) return -3;

	return err ? -4 : 0;
}



#endif