/*******************************************************************************\

main module

suntar 2.0, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

unsigned char version_string[]="\psuntar 2.05";

#include "PB_sync.h"
#include "antiglue.h"

/*#include	<EventMgr.h>
#include	<QuickDraw.h>
#include	<StdFilePkg.h>
*/

/*#ifdef THINK_C_5
#include <packages.h>
#else
#include <IntlPkg.h>
#endif*/
/*

#include	<DiskDvr.h>
#include	<DeviceMgr.h>
*/

#include <string.h>

#include "windows.h"
#include "suntar.h"

#define dmAllow 1	/* ==rmAllow */
#define dmUnmount 2	/* ==rmUmount: ho delle routines che usano dmUnmount anche */
					 /* sul menù per SCSI random access... */
#define dmMount 3	/* ==rmMount */
#define dmEject 4	/* ==rmEject */

/*#define hmExperiments (hmAgain+1)*/

MenuHandle writeMenu,hackersMenu;
/*MenuHandle tarPopupMenu,ntAPopupMenu,sFPutPopupMenu,sFGetPopupMenu;*/
MenuHandle popupMenu[5];
extern MenuHandle deviceMenu;
extern SysEnvRec	gMac;

extern char *titoli_si_no[];
short err_code;

options_type options;
long last_selection=0;
short again_command=0;
OSType tar_creator,bar_creator,bar_type,macbin_creator;
static Boolean append_mode;
Boolean in_Italia=false;
Boolean ignore_errors=false;
Boolean confirm_saves=false;
Boolean expert_mode=false;
Boolean non_convertire=false;
Boolean disco_espulso;		/* was my disk ejected ? */
Boolean sto_lavorando=false;	/* non usarlo se non per lo scopo attuale, dire al calcolo del
								tempo di sleep per WaitNextEvent di ritornare sempre 0, e sapere
								se devo abilitare il comando Pause 
								-- consider it as a read only variable !*/
static Boolean pausable=false;
static unsigned char check_ev_timing;
#ifdef V_122
Boolean smallFilesAreASCII;
#endif

short disable_binhex;			/* bit 0: disable conversion
								   bit 1: show info in extract
								   bit 2: show info in list
								   bit 3: save .info */

short last_drive,preferred_drive;
short back_prio;
short tar_block_size;

struct disk_info di;

Boolean preferences_changed=0;	/* Apple (TN 188) discourages to call 
		ChangedResource if it's not followed immediately by WriteResource, 
		and I want to modify the file only once, and not to modify it if nothing 
		changed */
char ok_to_copy = 0;
sector_t settori_passati;

enum tipo_fase fase;
short n_superdrives;
short drive_type[max_drive];
short dest_wdir=0;
struct t_list_open{
	struct t_list_open**next;
	short vRefNum;
	Str63 fName;
	} **h_list_open,**last_in_list_to_open;
short forza_formato,formato_encode;

extern sector_t next_header_for_POSIX;
unsigned char extra_title[]="\p - console";
unsigned char window_title[sizeof(version_string)+sizeof(extra_title)-1];
short default_flags =READONLY|CONSOLE|NOCLOSEBOX;
extern char which_popup;
extern Boolean need_a_touch;

extern Boolean gHasSCSIManager;


#define use_fixed_sizes() /* no more needed, but it's better not to forget */
						/* where I was using it, just in case... */

void handle_menus(long);
void add_menu(void);
short my_quit_handler(void);
void EnableDisableItem(MenuHandle,short,Boolean);
Boolean list_events(void);
short dialogo_abort(void);
short overwrite_sectors(void);
pascal Size memory_overflow(Size);
void nuovo_file(void);
void enable_write_items(void);
void disable_write_items(void);
void log_menu_command(long);
void adjust_ddriver_menu(void);
void chiudi_file(void);
Boolean volume_in_place(short);
void delete_out_file(void);
void rare_options(int);
Boolean SFPut_con_popup(unsigned char *);
void do_end_of_write_for_tape(void);
enum aa_mode{aa_open,aa_getinfo,aa_load,aa_load_last,aa_drop};
	/* attento, in alcuni casi dipendo dall'ordine, >=aa_load... */
void apri_archivio(enum aa_mode,short);
void stampa_stringa_stato(unsigned char *);
void nuovo_archivio(void);
void save_opts_quitting(void);
Boolean this_file_is_writable(short);
Boolean this_device_is_writable(void);
void load_char_tables(void);
void sistema_items_device_menu_scsi(void);
void sistema_items_device_menu_driver(void);
void touch_if_needed(void);
void adjust_mode_select(void);


static char diskWriteErrMsg[]="Write error %d\n";
char DONE[]="Done\n",FINITO[]="Finito\n";
char START_READ[]="Start reading\n";
extern const char MSDOSTEXT[],UNIXTEXT[];
static const char
	DRAG_DROP_FILE[]="Drag&drop file %P",
	INSERISCI_DISCO_UNIX[]="\pInserisci il disco in formato UNIX",
	INSERT_UNIX_DISK[]="\pInsert the disk in UNIX format",
	DISCO_ASSENTE[]="ll disco non è presente nel drive\n",
	MISSING_DISK[]="The disk is not in the drive\n",
	NON_INIZIALIZZATO[]="Disco non inizializzato\n",
	NOT_INITIALIZED[]="This disk is not initialized\n",
	UNTITLED[]="\pUntitled";
	



/***************** main ******************/


/*
suntar uses an architecture which is unusual, at least in the Mac world:
it has not a main event loop.
 When you have passed the first wave of horror and astonishment, I'll
tell you how it works.
 The MainEvent routine calls GetNextEvent and serves the event, but:
1) it returns to the caller, it does not loop internally
2) it does not know how to serve all events, since it's part of the
   windows module and does not know anything about the tar/untar modules:
   you must enlarge its capabilities by providing an event filter and an 
   "application specific menus handler", and maybe some "updaters" and
   "activaters" for windows (e.g. other filters)
3) some menu items (mainly, preferences) are handled immediately, but others only
	set the "last_selection" variable which the caller then examines and services.
 That is, MainEvent is a relatively low-level routine, which is not allowed to
take important decisions: it's a slave routine, not the master, the control center
of the whole application as is expected for the event handling routine.
 So, suntar has several event loops, which call MainEvent, usually after installing
different event filters. Furthermore, MainEvent is called periodically
while suntar is working, with one further event filter which disables all the
events which are not essential (almost all menu items are disabled during
execution of commands). This architecture may easily emulate a typical Mac
behaviour, but it's more flexible and powerful.
  Finally, MainEvent was broken into two parts, and sometimes one calls
the two parts, doing its own event handling between the two calls, so reducing the
need for a lot of filters.
The result is that the program is organized in a UNIX-like fashion, the
"master", the control center, is the main program, not the event loop. The most notable
difference: rather than calling a "print menu screen & wait for an answer"
suntar sets the "fase" variable (see later) and calls MainEvent in a loop
until the global variable last_selection has a non-zero value.
Other not obvious features in the basic architecture of suntar are:
-) a "fase" (phase) global variable must always tell what suntar is currently
   doing, it's used to control the state of the menus (during work, many 
   items are disabled) and sometimes some minor decisions (e.g. Expert list
   behaves slightly differently from List, but it's executed by the same
   function with the same parameters)
-) background events are controlled from within the lowest level operations, hence
   high level functions cannot avoid to be paused or aborted (through
   a longjmp) nor avoid that the preferences settings be changed: they
   must ensure correct operation even in those situations.

 In other words: the body of suntar is written in a UNIX-like fashion: it
believes to be the only one to take decisions, while Mac-like events (updates?
activation of DAs? what are those things?) are left to another module which
conceptually might be another task. But it's NOT another task, hence the UNIX-like
part of suntar must explicitly activate the event module, by calling check_events
(which obviously calls MainEvent): that's collaborative multitasking.
 Furthermore, on UNIX (or VMS or MS-DOS) a menu selection looks like this:

selection=my_menu(3,
"------------------------\n\
1  First command\n\
2  Second command\n\
3  Last choice\n\
-------------------------\n\
Your choice:");

in suntar, one must have a menu resource and modify MaintainApplSpecificMenus
in order to activate/deactivate them properly in the different phases of work
and according to some status variables. Finally, one does this:

fase= a phase in which that menu is enabled;
last_selection=0;	/ * it should be still 0, but just in case... * /
while(last_selection==0)
	MainEvent();

Now last_selection contains the number of the requested command (note that some
menu items are not known to the UNIX-like portion of suntar and are served by
the other module without any report).
 The Mac-aware module is totally responsible for the text windows (windows,
not window: it's a multiwindow system, but suntar exploits only one as the
console). Since it was adapted from a text editor it may allow
general-purpose text editing, but the UNIX-like portion exploits only its
TTY-like capabilities.
 Anyway, choosing to emulate the most antiquate form of TTY (the one which
printed on paper rather than on a CRT) has given to our module the possibility
to scroll and resize the console window: Symantec in its ANSI library chose
to give more power: modern video terminals have commands to clear the screen or
position the cursor at any position, but in order to implement even only one
such command they were obliged to fix the screen size to 25*80 characters
(you can't position the cursor on the screen if there isn't a coordinate
system on it), with no scroll, a very bad resize and a non-Mac feeling.

 So, a student who programmed mostly on UNIX (Sauro) and an engineer with UNIX
and VMS experience (Gabriele) could write a Macintosh application without
being obliged to change their habits. And anyway, this structure is flexible
enough to allow us to write new fully-Mac routines (e.g. the options box) or
to let the UNIX-like module handle some Mac-like events (easy, since there
is no rigid rule about which module must handle which events: add a new event
filter, and the responsibility for handling an event is moved to it).

*/



void main()
{
extern char *printf_buf;
short command;
short files_to_be_opened;
short last_file_opened=0;

/* install the handlers for events which the window module doesn't know how
to handle */
my_handle_menus=handle_menus;
my_add_menus=add_menu;

/* initialize everything */
/* l'ordine è importante perché a) ci sono delle dipendenze da variabili
   inizializzate da un'altra di esse, e b) determina dove sono caricate le
   risorse di codice e quindi la frammentazione della memoria */


pStrcpy(window_title,version_string);
pStrcat(window_title,extra_title);

printf_buf=disk_buffer,list_buf_size=20;	/* necessario per potere fare
							delle printf prima di chiamare prefs_da_risorsa */
{	/* I want to UnloadSeg CODE 7 but to avoid it leaving a "hole" I must
	load it high, e.g. after 50k */
Handle h=NewHandle(50000);
GetResource('CODE',7);
DisposeHandle(h);
}

InitConsole();
UnloadSeg(InitConsole);

init_buffering();
init_hd_buffering();

init_hqx_tab();


SetGrowZone(memory_overflow);

di.drive_number=0;
di.sectors_on_floppy=0;
di.tipo_device=d_notopen;

di.file_format=0;
h_list_open=NULL;

/* new for suntar 1.3.2: if the user requested to open some files (e.g. by
System 7's drag and drop) then open them */
CountAppFiles(&command,&files_to_be_opened);
if(files_to_be_opened!=0 && command==appPrint){	/* suntar can't print files ! */
	SysBeep(5);
	ExitToShell();
	}

/* look at what disk drives and device driver are available */
identifica_hardware();

/* install the Quit handler */
my_at_exit= my_quit_handler;

load_char_tables();
UnloadSeg(InitConsole);

/* main loop */

/* debugging: open a second window so debugging prints don't mix with the
normal outputs * /
printf("inizio\n");
pStrcpy(window_title,"\pDebug");
new_window();
/ * */

/*{extern Boolean gHasVirtualMemory;
printf("hvm=%d\n",gHasVirtualMemory);
}*/


/*printf("",*(char*)0xF20000);test, causes bus error on my LC */

for(;;){
	fase=non_faccio_nulla;
	set_skip_all(0);
	if(last_file_opened==files_to_be_opened && h_list_open==NULL) close_all_open_WD();
	check_ev_timing=120;
	flush_buffers(1);
	enable_autoflush();
	reinit_full_name();

	last_selection=0;
	while(last_selection==0){
		if( (last_file_opened!=files_to_be_opened||h_list_open!=NULL) && di.tipo_device==d_notopen)
			last_selection=menuItemMess(fileID,fmOpen);
		else
			MainEvent();
		}
	fase=ricevuto_comando;
	settori_passati=0;
	log_menu_command(last_selection);

/* sono possibili solo i menù Special, Write (solo i primi 3 item) e File, perché
Preferences ed Edit sono gestiti subito */
	if(hiword(last_selection)==hackID){
		#if 0
		/* these two commands need no disk, and hacking asks a disk before
		examining the command...*/
		if(loword(last_selection)==hmDriveList){
			DriveList();
			}
		else if(loword(last_selection)==hmExperiments){
			/*prova_SCSI();*/
			/*short j,i=OpenDriver("\p.EDisk",&j);
			printf("ris=%d %d\n",i,j);*/
			printf("sect=%ld\n",di.sectors_on_floppy);
			}
		else
		#endif
			hacking(loword(last_selection));
		}
/* non faccio else perché si può uscire dall'hacking anche con un comando
del menù File, che va eseguito 
-- don't do 'else': I can exit from hacking with a File menu command which must be
executed */

	command=loword(last_selection);

	if(hiword(last_selection)==writeID){
		bar_archive= loword(last_selection)==wmCreateBar;	/* non valido nel caso di append */
		append_mode= loword(last_selection)==wmAppend;
		if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT && di.v.si.current_tape_block!=0){
			extern unsigned char rewind_proposal[];
			if(my_semimodal_dialog(155,NULL,0,rewind_proposal,NULL,NULL)==2){
				di.v.si.current_tape_block=0x7FFFFFFF;
				archive_rewind();
				}
			else{
				ParamText("\pYou must define somehow the position of start archive, or you'll get an error",PNS,PNS,PNS);
				my_alert();
				}
			}
		else if(di.tipo_device==d_file && !this_file_is_writable(di.m_vrefnum)){
			ParamText("\pYou have no write permission on this file",PNS,PNS,PNS);
			my_alert();
			continue;
			}
		else if(di.tipo_device==d_scsi && !this_device_is_writable()){
			ParamText("\pThis device declares to be read-only or write-protected",PNS,PNS,PNS);
			my_alert();
			}
		scrivi_su_floppy();
		}
	else if(hiword(last_selection)==fileID){
		again_command=0;
		last_selection=0;
		if(command==fmGetInfo){
			listonly=2;
			apri_archivio(aa_getinfo,0);
			invalid_buffers();
			}
		else if(command==fmEncode){
			nuovo_file();
			invalid_buffers();
			}
		else if(command==fmNew){
			nuovo_archivio();
			invalid_buffers();
			}
		else if(command==fmOpen){
			invalid_buffers();
			di.isEjectable=false;
			listonly=0;
			if(last_file_opened!=files_to_be_opened){
				++last_file_opened;
				apri_archivio(last_file_opened==files_to_be_opened?aa_load_last:aa_load,
					last_file_opened);
				}
			else if(h_list_open!=NULL)
				apri_archivio(aa_drop,0);
			else{
				dest_wdir=0;
				apri_archivio(aa_open,0);
				}
			}
		else if(command==fmOpenDev){
			invalid_buffers();
			scegli_device();
			}
		else if(command==fmClose){
			chiudi_file();
			}
		else if(command==fmDriveList)
			DriveList();
		else if(command ==fmCopy)
			copia_da_disco_a_file();
		else if(command==fmList){
			listonly=1;
			estrai_da_floppy();
			}
		else if(command==fmExtract){ /* extract, con de-macBinarizzazione e conversione LF->CR */
			listonly=0;
			estrai_da_floppy();
			}
		else if(command==fmSelect)
			select_and_extract();
		else if(command==fmFormat){
			disk_initialize();
			}
		}
	}
}

/**************** apri/chiudi archivio su file ***********/


void select_destination(enum aa_mode);
static void select_destination(mode)
enum aa_mode mode;
{
	if(mode<aa_load || dest_wdir==0){
		select_directory();
		if(reply.good) dest_wdir=reply.vRefNum;
		}
	else{
		reply.good=true;
		reply.vRefNum=dest_wdir;
		SetVol(NULL,curr_vrefnum=dest_wdir);
		}
}

void fine_conversione(enum aa_mode);
static void fine_conversione(mode)
enum aa_mode mode;
{
if(mode==aa_load || mode==aa_drop && h_list_open!=NULL)
	;	/* don't print any message */
else{
	touch_if_needed();
	FlushVol(NULL,dest_wdir);
	printf(in_Italia?FINITO:DONE);
	fine_lavoro();
	}
}


void enqueue_list_to_open(void);
static void enqueue_list_to_open()
{
struct t_list_open **h=NewHandle( sizeof(struct t_list_open) - (63-reply.fName[0]) );
HLock(h);
(*h)->vRefNum=reply.vRefNum;
pStrcpy((*h)->fName,reply.fName);
(*h)->next=NULL;
HUnlock(h);
if(h_list_open==NULL)
	h_list_open=h;
else
	(*last_in_list_to_open)->next=h;
last_in_list_to_open=h;
}

void dequeue_list_to_open(void);
static void dequeue_list_to_open()
{
struct t_list_open **h=h_list_open;
HLock(h);
h_list_open=(*h)->next;
reply.vRefNum=(*h)->vRefNum;
pStrcpy(reply.fName,(*h)->fName);
DisposHandle(h);
}


void gestisci_file_nonconvertibile(void);
static void gestisci_file_nonconvertibile(void)
{
short src_vrefnum;
static char mess1[]="File %P non convertito\n",mess2[]="File %P, no conversion\n";
Str255 name;

	di.sectors_on_floppy=0x7FFFFFFF;

	src_vrefnum=reply.vRefNum;

	pStrcpy(name,reply.fName);
	select_destination(aa_drop);
	if(!reply.good) return;
	if(src_vrefnum==reply.vRefNum)
		printf(in_Italia ?mess1:mess2,name);
	else{
		/* dialogo per chiedere conferma, solo se è la prima volta */
		if(ok_to_copy<3 ){
			ok_to_copy=my_semimodal_dialog(134,titoli_si_no,4,
				in_Italia?"\pNon so convertire questo file, lo copio ?":
				"\pCan't convert this file, copy it ?",PNS,PNS);
			}
		if(ok_to_copy==1 || ok_to_copy==3){
			printf(in_Italia ?"File %P copiato senza conversioni\n":
				"File %P copied with no conversion\n",name);
			SetVol(NULL,curr_vrefnum=src_vrefnum);
			fase=reading_disk;
			copy_mac_file(name);
			}
		else
			printf(in_Italia ?mess1:mess2,name);
		}
	chiudi_archivio();
}


/**********/

void apri_archivio(mode,indice)		/* open a file archive */
enum aa_mode mode;
short indice;
{
long filesize;
short nLF,nCR;
FileParam fpb;
Boolean do_not_open;
extern long file_date;

if(setjmp(main_loop)<0) {
	chiudi_archivio();	/* un errore può scattare sotto questo setjmp
						solo se si tratta di conversione BinHex, MacBinary, uudecode
						etc. ma non di estrazione tar (nell'ultimo caso il file
						viene solo aperto) per cui devo chiuderlo */
	return;
	}
forza_formato=1;	/* per tutti i casi in cui non passo dallo SF */

if(mode==aa_getinfo){
	SFTypeList	myTypes;
	my_SF_Get(-1,myTypes);
	if(!reply.good) return;
	}
else if(mode==aa_open){
	short i;
	i=SFGet_with_popup();

	while(i==1){
		EventRecord myEvent;
		WindowPtr whichWindow;
		enqueue_list_to_open();
		/*printf("accodo %P\n",reply.fName);*/
		last_selection=0;
		i=0;
		/* attento per il debugging, la creazione della console introduce un
		evento che mi fa uscire anticipatamente dal loop */
		#if 0
		if(!OSEventAvail(mDownMask/*everyEvent*/, &myEvent) && myEvent.what==mouseDown &&
			FindWindow( myEvent.where, &whichWindow)==inMenuBar){
			/* essendo un evento "di imbroglio" non è posto nella coda
			e non viene ritornato da OSEventAvail */
		#endif

			fase=drag_and_drop;
			MainEvent();
			fase=non_faccio_nulla;
			if(last_selection==menuItemMess(fileID,fmOpen))
				i=SFGet_with_popup();	/* andrebbe anche un get_file
					normale purché con "was_a_true_dialog" */
			#if 0
			}
			#endif
		}
	#if 0
	{
	printf("deaccodo:\n");
	while(h_list_open!=NULL){
		dequeue_list_to_open();
		printf("%P\n",reply.fName);
		}
	}
	#endif
	ok_to_copy=-1;

	if(i==0) return;	/* anche nel caso di avere una lista, ci penserà il main
			loop a richiamarmi con aa_drop o aa_drop_last */
	}
else if(mode==aa_drop){
	dequeue_list_to_open();
	one_empty_line();
	printf(DRAG_DROP_FILE,reply.fName);
	}
else{	/* aa_load o aa_load_last */
	AppFile fileInfo;
	GetAppFiles (indice,&fileInfo);
	ClrAppFiles (indice);
	reply.vRefNum=fileInfo.vRefNum;
	pStrcpy(reply.fName,fileInfo.fName);
	one_empty_line();
	printf(DRAG_DROP_FILE,reply.fName);
	}

fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;
fpb.ioVRefNum=reply.vRefNum;
fpb.ioNamePtr=reply.fName;
if (PBGetFInfoSync(&fpb)) pbsyserr(&fpb);
filesize=fpb.ioFlLgLen;
di.sectors_on_floppy= (filesize+511)>>9;

if(listonly){
	disable_autoflush(2);
	one_empty_line();
	printf("File %P ",reply.fName);
	printf("(data %ld+res %ld bytes)",fpb.ioFlLgLen,fpb.ioFlRLgLen);
#if 0
#define ALIAS_BIT 0x8000
	if(fpb.ioFlFndrInfo.fdFlags & ALIAS_BIT) printf(" alias");	/* currently
	that's useless, I'm using a non-custom standard file, which resolves aliases ! */
#endif
	vai_a_capo();
	print_one_date(fpb.ioFlCrDat,in_Italia?"creato     ":"created  ");
	vai_a_capo();
	print_one_date(fpb.ioFlMdDat,in_Italia?"modificato ":"modified ");
	vai_a_capo();
	print_type_creator(fpb.ioFlFndrInfo.fdType,fpb.ioFlFndrInfo.fdCreator);
	vai_a_capo();
	if(fpb.ioFlAttrib&1) printf("protected\n");
	enable_autoflush();
	}
else
	one_empty_line();

pStrcpy(tarh.name,reply.fName);
my_p2cstr(tarh.name);	/* these statements are useful only if the file is not */
bar_archive=0;		/* a tar/bar archive */
full_name=strcpy(nam_buf,tarh.name);

/* tarh.mtime is examined later, nothing bad happens if it contains garbage
but that's not good programming style */
strcpy(tarh.mtime,"0");
sect_n=0;
end_of_file();	/* forse non necessario, ma prudente */
copia_ultimo_header(&tarh,(sector_t)0);	/* gli devo passare il numero del
						settore successivo all'header */

forza_formato--;	/* the popup starts from 1, my codes start from 0 */


/*forza_formato=ff_ASCII;*/

do_not_open=true;
if(!forza_formato && fpb.ioFlFndrInfo.fdType=='APPL')
	printf(in_Italia?"Applicazione\n":"Application\n");
else if(!forza_formato && fpb.ioFlFndrInfo.fdType=='INIT')
	printf(in_Italia?"Estensione\n":"Extension\n");
else if(!forza_formato && fpb.ioFlFndrInfo.fdType=='cdev')
	printf(in_Italia?"Pannello di controllo\n":"Control panel\n");
else if(!forza_formato && (fpb.ioFlFndrInfo.fdType=='DFIL'||fpb.ioFlFndrInfo.fdType=='dfil'))
	printf(in_Italia?"Accessorio di scrivania\n":"Desk Accessory\n");
else if(filesize==0)
	printf(in_Italia?"Data fork vuota\n":"Empty data fork\n");
else
	do_not_open=false;
if(do_not_open && mode<aa_load){
	if(!listonly) printf(in_Italia?"Questo tipo di file non può essere aperto da suntar\n":
		"This file type can\'t be opened by suntar\n");

	return;
	}

/* Open the file only now, and don't open files with empty data fork:
Disk Doubler extracts the resource fork of compressed files if I do that,
but that's time consuming and useless */

if(apri_file("rb",&di.m_vrefnum))
	return;

reinit_buffering(512);
di.file_format=ff_tarbar;
di.tipo_device=d_file;
aggiorna_nome_file_aperto();

di.v.file_current_s=-1;
di.archive_start_sector=0;
di.writing_allowed=true;
di.signature_avail=2;

if(do_not_open){
	gestisci_file_nonconvertibile();
	fine_conversione(mode);
	}
else if(forza_formato==ff_packit || forza_formato==0 && fpb.ioFlFndrInfo.fdType=='PIT '){
	/* don't examine the creator, there is at least one program (unpit)
	which creates PackIt files with another creator ('UPIT') */
	if(listonly)
		printf(in_Italia?"Archivio PackIt\n":"PackIt archive\n");
	else{
		select_destination(mode);

		if(reply.good){
			file_date= -1;
			one_empty_line();
			printf(in_Italia?"Estrazione da archivio PackIt %s\n":
				"Unpacking PackIt archive %s\n", tarh.name);
			my_unpit();
			fine_conversione(mode);
			}
		chiudi_archivio();
		}
	}
else if(!forza_formato&&is_hqx_name()>0 || forza_formato==ff_binhex){
	macbinh.cdate=fpb.ioFlCrDat;
	macbinh.mdate=fpb.ioFlMdDat;
/*	dopo di che non deve essere chiamato chkmacbin, che scarabocchia su macbinh...*/

	di.file_format=ff_binhex;
	fase=reading_disk;

	if(listonly)
		untar_file(filesize);
	else{
		select_destination(mode);
		if(reply.good){
			one_empty_line();
			untar_file(filesize);
			fine_conversione(mode);
			}
		chiudi_archivio();
		}
	}
else if((forza_formato==0 || forza_formato==ff_macbin||forza_formato==ff_c_macbin) &&
		chkmacbin (filesize,forza_formato)){
	di.file_format=ff_macbin;
	fase=reading_disk;

	copia_ultimo_header(&tarh,(sector_t)0);
	if(listonly)
		untar_file(filesize);
	else{
		select_destination(mode);

						/* per la verità, salvando un solo file potrei usare uno
						standard put, ma significherebbe fare modifiche più
						pesanti di quanto non voglia
						--having only one file to save I could use SFPutFile, but
						the existing routines don't expect more than a destination 
						folder, and suntar was NOT meant as a file format converter,
						even if it can convert file formats, hence these commands
						are implemented according to the principle of "do that with
						the fewest changes to existing routines"
						*/
		if(reply.good){
			one_empty_line();
			untar_file(filesize);
			fine_conversione(mode);
			}
		chiudi_archivio();
		}
	}
else if(forza_formato==ff_macbin||forza_formato==ff_c_macbin)
	error_message(in_Italia?"Formato non MacBinary\n":"Not a Macbinary file\n");
else if(forza_formato==ff_uuencode || forza_formato==0 &&is_uu_name()){
	di.file_format=ff_uuencode;
	fase=reading_disk;
	file_date=-1;

	if(listonly){
		printf("uuencoded file\n");
		uudecode(filesize);
		}
	else{
		select_destination(mode);
		if(reply.good){
			one_empty_line();
			printf(in_Italia?"Estrazione da file uuencode %P\n":
				"Extracting uuencoded file %P\n",reply.fName);
			uudecode(filesize);
			fine_conversione(mode);
			}
		chiudi_archivio();
		}
	}
else if(isASCII(disk_buffer,filesize,&nLF,&nCR)>0 &&!forza_formato || forza_formato==ff_ASCII ){
	di.file_format=ff_ASCII;
	fase=reading_disk;

	if(listonly){
		printf(in_Italia ?"Contenente testo ASCII":"Containing ASCII text");
		if(nLF>0){
			if(nCR)
				printf(MSDOSTEXT);
			else
				printf(UNIXTEXT);
			}
		vai_a_capo();
		}
	else if(nLF==0 && forza_formato!=ff_ASCII){
		if(mode<aa_load){
			printf(in_Italia?"Testo ASCII in formato Macintosh, nessuna conversione\n":
			"ASCII text in Macintosh format, no conversion\n");
			chiudi_archivio();
			}
		else{
			gestisci_file_nonconvertibile();
			fine_conversione(mode);
			}
		}
	else{
		short src_vrefnum;
		Str63 src_name;
		/* drag&drop con dest diversa -> mantieni il nome */
		if(mode>=aa_load){
			src_vrefnum=reply.vRefNum;
			pStrcpy(src_name,reply.fName);
			select_destination(mode);
			if(!reply.good){
				chiudi_archivio();
				return;
				}
			}
		if(mode>=aa_load && src_vrefnum != reply.vRefNum)
			pStrcpy(reply.fName,src_name);
		else{
			if(reply.fName[0]>63-4) reply.fName[0]=63-4;
			pStrcat(reply.fName,"\p.out");
			my_SF_Put(in_Italia?"\pSalva testo Mac col nome:":"\pSave Mac text as:",reply.fName);
			}
		if(reply.good){
			extern ParamBlockRec pb;
			int i;
			flush_hd_buffer();	/* non si sa mai */
			SetVol(NULL,reply.vRefNum);	/* serve a controlla_spazio */

			for(;;){
				i=new_text_file(&pb.ioParam.ioRefNum);
				if(i) break;	/* errore nell'apertura del file */

				pb.fileParam.ioVRefNum=0;	/* serve a controlla_spazio */
				pb.fileParam.ioNamePtr=reply.fName;	/* e questo nel caso si provochi un errore */
				i=controlla_spazio(filesize,0);
				if(i==1){	/* Skip */
					delete_out_file();	/* it exists but it's currently of 0 size */
					chiudi_archivio();
					return;
					}
				if(i==2){	/* continue elsewhere */
					delete_out_file();
					reply.vRefNum=0;
					/* and retry... */
					}
				else
					break;
				}

			if(i==0){
				short nbytes;
				
				printf(in_Italia?"Converto testo %s \"%s\" in \"%P\"\n":
					"Converting %s text \"%s\" to \"%P\"\n",nCR&&nLF?"MS-DOS":
					"UNIX",tarh.name,reply.fName);
				info_file_open=1;
				init_deDOSize();
				while(filesize>0){
					if(filesize>=512L) nbytes=512; else nbytes= (short)filesize;
					filesize -= nbytes;
					if(readblock(disk_buffer,nbytes)!=0){
						disk_error_message();
						raise_error();
						}
					nbytes=macize_ASCII(disk_buffer,nbytes,nCR&&nLF);

					if(write_hd(disk_buffer,nbytes))
						write_error_message();
					}
				if(flush_hd_buffer())
					write_error_message();
				FSClose(pb.ioParam.ioRefNum);
				info_file_open=0;
				}
			else
				error_message(in_Italia?"Errore nell\'apertura del file\n":"Error opening file\n");
			fine_conversione(mode);
			}
		chiudi_archivio();
		}
	}
else{	/* the file is opened as a tar/bar archive */
	if(!listonly){
		one_empty_line();

		if(mode>=aa_load){
			/* I must not open a non-tar archive, but rather copy it, or at least
			ask. However, identify_format is verbose, and classifies a binary zero as
			"tar archive": better doing the tests directly */
			long dummylong;
			if(filesize>=512){
				leggi_settore(0,disk_buffer);
				if(check_error()){
					chiudi_archivio();
					return;
					}
				}
			if(filesize<512 || check_all_zero(disk_buffer))
				di.file_format=ff_macintosh;
			else if(!untar_checksum(disk_buffer,-1,false))
				;	/* tar */
			else if(unbar_checksum(disk_buffer,-1,&dummylong)>=0)
				bar_archive=1;	/* bar */
			else
				di.file_format=ff_macintosh;

			if(di.file_format==ff_macintosh){
				gestisci_file_nonconvertibile();
				fine_conversione(mode);
				return;
				}
			else{
				static char *titoli[]={"\pEstrai","\pApri","\pCopia"};
				short i;
				/* chiedere se estrarre e chiudere, o se copiare.... */
				#if 0
				if(in_Italia)
					ParamText("\pArchivio in formato ",bar_archive?"\pb":"\pt",
						"\par,\rscegli cosa fare:",PNS);
				else
					ParamText(bar_archive?"\pB":"\pT",
						"\par archive, choose what to do:",PNS,PNS);
				i=my_modal_dialog(151,titoli,3);
				#else
				if(in_Italia)
					i=my_semimodal_dialog(151,titoli,3,"\pArchivio in formato ",bar_archive?"\pb":"\pt",
						"\par,\rscegli cosa fare:");
				else
					i=my_semimodal_dialog(151,NULL,0,bar_archive?"\pB":"\pT",
						"\par archive, choose what to do:",PNS);
				#endif
				if(i==1){
					printf(in_Italia?"Estrazione da file %s (%ld settori)\n":
					"Extracting from file %s (%ld sectors)", tarh.name,di.sectors_on_floppy);
					estrai_da_floppy();
					chiudi_archivio();
					return;
					}
				else if(i==3){
					ok_to_copy=1;
					gestisci_file_nonconvertibile();
					fine_conversione(mode);
					return;
					}
				}
			}
		printf(in_Italia?"Apertura file %s (%ld settori)\n":
			"Opening file %s (%ld sectors)", tarh.name,di.sectors_on_floppy);
		(void)identify_format();
		}
	else
		(void)identify_format();
	}
if(listonly){
	chiudi_archivio();
	/*vai_a_capo();*/
	}
}

/*********************/

void chiudi_archivio()
{
if(di.tipo_device==d_file){	/* the test is required since I call it in raise_error */
	flush_buffers(2);
	di.tipo_device=d_notopen;
	aggiorna_nome_aperto();

	di.file_format=0;
	FSClose(di.m_vrefnum);
	}
}

/**************** new *******************/

#if 0
/* sono stato per un po' incerto: se creo un tar o bar devo presentare
prima lo standard put, per MacBinary e hqx è meglio presentare prima lo
standard get in modo da proporre come nome il nome originale + suffisso
.bin o .hqx.
Per un po' ho usato un pre-dialogo per definire il formato; nella 2.01
ho messo un popup nello standard put, col che privilegiando i formati
tar e bar (routine qui sotto). Nella 2.04 ho deciso di avere due
diversi comandi, uno con popup "tar/bar" in un sfPut e uno con un popup
"MacBinary/BinHex/uuencode" in un sfGet */


void nuovo_file() /* versione 2.01 */
{
extern short curr_sf_format;
#define pStrcmp(x,y) !EqualString(x,y,true,true)

SFPut_con_popup("\pCreate new archive:");
if(!reply.good) return;

if(curr_sf_format<=2){
	reinit_buffering(512);
	bar_archive=curr_sf_format==2;
	if(!apri_file("wb",&di.m_vrefnum)){
		di.file_format=ff_tarbar;
		di.tipo_device=d_file;
		aggiorna_nome_file_aperto();
		di.v.file_current_s=0;
		di.sectors_on_floppy=0;
		di.archive_start_sector=0;
		di.isEjectable=false;
		di.signature_avail=2;
		append_mode= false;
		di.writing_allowed=true;
		scrivi_su_floppy();
		}
	}
else{
	/* è un po' arzigogolato, ma l'avere invertito l'ordine degli
	standard file ha cambiato parecchio le cose... */
	SFTypeList	myTypes;
	char fname[80];
	short dest_refnum=reply.vRefNum;
	pStrcpy(fname,reply.fName);

	my_SF_Get(-1,myTypes);
	if(reply.good){
		short org_refnum;
		pStrcpy(tarh.name,reply.fName);
		org_refnum=reply.vRefNum;
		reply.vRefNum=dest_refnum;
		pStrcpy(reply.fName,fname);

		if(!create_overwrite(curr_sf_format==3?macbin_creator:text_creator,'TEXT',&di.m_vrefnum)){
			di.tipo_device=d_file;
			aggiorna_nome_file_aperto();
			di.v.file_current_s=0;
			di.sectors_on_floppy=0;
			di.archive_start_sector=0;
			di.isEjectable=false;
			SetVol(NULL,curr_vrefnum=org_refnum);	/* sarebbe meglio usare
					get_openfile_location, ma non esageriamo... */

			reset_sector_count();
			fase=writing_disk;
			bar_archive=false;
			if(setjmp(main_loop)>=0) {
				extern short conversion_mode;
				printf(in_Italia?"Conversione di %P\n":"Converting %P\n",tarh.name);
				if(curr_sf_format==3)
					di.file_format=ff_macbin;
				else
					di.file_format=ff_binhex;
				conversion_mode= curr_sf_format==5?100:0;	/* 100 è per uuencode */
				tar_file_or_folder (tarh.name);
				chiudi_archivio();
				printf(in_Italia?FINITO:DONE);
				}
			else
				chiudi_archivio();
			}
		}
	}
}


#else

/********************/

void nuovo_file()		/* versione 2.04 */
{
SFTypeList typeList;

	my_SF_Get(-1,&typeList);
	if(reply.good){
		short i;
		short org_refnum=reply.vRefNum;
		pStrcpy(tarh.name,reply.fName);
		/* non proporre un nome contenente certi caratteri...*/
		for(i=1;i<=tarh.name[0];i++)
			if(tarh.name[i]=='\0'||tarh.name[i]==CR||tarh.name[i]==LF)
				tarh.name[i]='_';
		pStrcpy(disk_buffer,reply.fName);
		which_popup=1;
		
		SFPut_with_popup("\pSave encoded file as:");
		pStrcpy(tarh.name,disk_buffer);

		if(reply.good && !create_overwrite(formato_encode==1?macbin_creator:text_creator,'TEXT',&di.m_vrefnum)){
			extern short conversion_mode;
			di.tipo_device=d_file;

			printf(in_Italia?"Creazione di %P\n":"Creating %P\n",reply.fName);
			if(formato_encode==1)
				di.file_format=ff_macbin;
			else
				di.file_format=ff_binhex;
			conversion_mode= formato_encode==3?100:0;	/* 100 è per uuencode */

			pStrcpy(reply.fName,tarh.name); /* to show the name of the input, not output, file */
			aggiorna_nome_file_aperto();

			di.v.file_current_s=0;
			di.sectors_on_floppy=0;
			di.archive_start_sector=0;
			di.isEjectable=false;
			SetVol(NULL,curr_vrefnum=org_refnum);	/* sarebbe meglio usare
					get_openfile_location, ma non esageriamo... */

			reset_sector_count();
			fase=writing_disk;
			bar_archive=false;
			if(setjmp(main_loop)>=0) {
				tar_file_or_folder (tarh.name);
				chiudi_archivio();
				printf(in_Italia?FINITO:DONE);
				}
			else
				chiudi_archivio();
			}
		}
}

void nuovo_archivio()
{
extern short curr_sf_format;
#define pStrcmp(x,y) !EqualString(x,y,true,true)

	which_popup=0;
	SFPut_with_popup("\pCreate new archive:");
	if(!reply.good) return;

	reinit_buffering(512);
	bar_archive=curr_sf_format==2;
	if(!apri_file("wb",&di.m_vrefnum)){
		di.file_format=ff_tarbar;
		di.tipo_device=d_file;
		aggiorna_nome_file_aperto();
		di.v.file_current_s=0;
		di.sectors_on_floppy=0;
		di.archive_start_sector=0;
		di.isEjectable=false;
		di.signature_avail=2;
		append_mode= false;
		di.writing_allowed=true;
		scrivi_su_floppy();
		}
}
#endif

/**************************************/

void chiudi_file()
{
if(di.tipo_device==d_file){
	flush_buffers(2);
	chiudi_archivio();
	}
else if(di.tipo_device==d_driver){
	flush_buffers(2);
	if(di.v.fi.opened_by_insertion==1)
		diskEject(true);
	else{
		di.tipo_device=d_notopen;
		aggiorna_nome_aperto();
		di.drive_number=0;
		}
	DeleteMenu(deviceID+2);	/* incondizionato, tanto... */	
	DrawMenuBar();
	}
else if(di.tipo_device==d_scsi){
	close_SCSI_device();
	di.tipo_device=d_notopen;
	aggiorna_nome_aperto();
	di.drive_number=0;
	}
invalid_buffers();
}


/***************** copia da disco a file ******************/

void copia_da_disco_a_file()			/* copy a disk archive to a file archive */
/* trasferisce quello che vede in un file tar, senza estrarre i singoli files */
{
sector_t more_sectors;
short inPlace;
Boolean fine;
long length;
short outputFile;
static unsigned char nome_ita[]="\parchivio",  eng_name[]="\parchive";

listonly=0;	/* ad uso di floppy swap, le altre routines non le chiamo */

if(setjmp(main_loop)<0) {
	FSClose(outputFile);	/* non lo metto in raise_error... */
	return;
	}

if(aspetta_inserzione(in_Italia?INSERISCI_DISCO_UNIX:
	INSERT_UNIX_DISK,0) || di.tipo_device==d_driver&&di.v.fi.is_not_initialized){
		FSClose(outputFile);
		return;
		}


more_sectors=0;

fase=reading_disk;
if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
	adjust_mode_select();

bar_archive= identify_format()==bar_format;

if(bar_archive){
	if( my_atoi(((barh_type*)disk_buffer)->volume_num,&floppy_n)) floppy_n = -1;
	if(floppy_n!=1) error_message(in_Italia?"Questo non è il primo volume\n":
		"That\'s not the first volume\n");
	}

pStrcpy(disk_buffer,in_Italia?nome_ita:eng_name);
pStrcat(disk_buffer,bar_archive?"\p.bar":"\p.tar");

my_SF_Put(in_Italia?"\pSalva col nome:":"\pSave as:",disk_buffer);
if(!reply.good) return;

if(apri_file("wb",&outputFile)) return;
connect_to_hd_buffering(outputFile);

one_empty_line();
printf(in_Italia?"Inizio lettura\n":START_READ);

if(!bar_archive)
	tar_check_floppy_swap(1);	/* ci sono delle inizializzazioni */
else{
	leggi_settore(0,&disk_buffer);
	if(check_error()) raise_error();
	if(write_hd(disk_buffer, 512)){
		beep_in_foreground();
		write_error_message();
		}
	bar_check_floppy_swap(1);
	}

leggi_settore(sect_n,&disk_buffer);
if(check_error()) raise_error();

do{
	if(write_hd(disk_buffer, 512)){
		beep_in_foreground();
		write_error_message();
		}
	check_events(false);
	fine = more_sectors==0;		/* fine del file corrente 
								-- end of current file ? */
	if(fine){
		fine= check_all_zero(disk_buffer);	/* fine dell'intero archivio
										-- end of the archive ? */
		if(!fine){
			unsigned char linkflag = get_linkflag(&length,true);
			last_offset=0;
			copia_ultimo_header(disk_buffer,sect_n+1);	/* gli devo passare il numero del
						settore successivo all'header, da qui il +1 */
			if(length<0)
				fine=true;
			else{
				print_info(bar_archive?((barh_type*)disk_buffer)->name:disk_buffer,length);
				more_sectors=(length+511)/512;
				}
			}
		}
	else
		more_sectors--;

	if(!fine){
		sect_n++;
		if(bar_archive)
			bar_check_floppy_swap(more_sectors==0?-1:0);
		else
			tar_check_floppy_swap(more_sectors==0?-1:0);
		leggi_settore(sect_n,&disk_buffer);
		if(check_error()) raise_error();
		}
	}
while(!fine);

one_empty_line();
printf(in_Italia ? "Lettura completata\n" : "Read completed\n");
if(di.v.fi.opened_by_insertion==1) diskEject(true);
if(flush_hd_buffer()){
	beep_in_foreground();
	write_error_message();
	}
FSClose(outputFile);
fine_lavoro();
}

/***************** estrai da floppy ******************/

void estrai_da_floppy()		/* extract or list */
{
enum formats fmt;

if(setjmp(main_loop)<0) {
	return;
	}

if(!listonly){
	select_directory();
	if(!reply.good) return;
	}

if(aspetta_inserzione(in_Italia?INSERISCI_DISCO_UNIX:
	INSERT_UNIX_DISK,0) || di.tipo_device==d_driver&&di.v.fi.is_not_initialized) return;
one_empty_line();
printf(in_Italia?"Inizio lettura\n":START_READ);

disco_espulso=!listonly;	/* per un extract non mi interessa impedire di espellere... */
fase=reading_disk;

if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
	adjust_mode_select();

fmt=identify_format();
if(fmt==tar_format)
	my_untar(true);
else if(fmt==bar_format)
	my_unbar(true);

if(disco_espulso) diskEject(true);	/* espelle se era un List multivolume, non espelle
		se list di un solo disco */
if(!listonly){
	touch_if_needed();
	FlushVol(NULL,0);
	}
fine_lavoro();
}


/************ scrivi su floppy ******************/


Boolean ins_checkbox_status;

void set_exam_checkbox(WindowRecord *);
static void set_exam_checkbox(w)
WindowRecord *w;
{
short	kind;
Handle	h;
Rect	r;
extern Boolean sm_checkbox_status;

sm_checkbox_status=ins_checkbox_status;
GetDItem(w,4,&kind,&h,&r);
SetCtlValue((ControlHandle)h,sm_checkbox_status);
}

void scrivi_su_floppy()		/* handling of the write... items in the Write menu */
{
Boolean ok_to_proceed;
extern Boolean sm_checkbox_status;

if(setjmp(main_loop)<0) {
	return;
	}

ok_to_proceed=false;

ins_checkbox_status=false;

do{
	sm_checkbox_status=false;	/* if the disk was in, nobody else clears it */
	if(aspetta_inserzione(in_Italia?"\pInserisci il disco su cui scrivere":
		"\pInsert the disk to be written",!append_mode /* 0 or 1 !!! */)) return;
	ins_checkbox_status=sm_checkbox_status;

	if((is_wrprot()&1) && (di.tipo_device==d_driver&&di.v.fi.is_not_initialized||!ins_checkbox_status) ){
		printf_protetto();
		SysBeep(5);
		return;
		}

	if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized){
		disk_format(false);
		if(di.tipo_device==d_notopen && append_mode) return;
		}
	else{
		if(ins_checkbox_status){
			esamina_disco();
			if(di.drive_number==0)
				;
			else if((is_wrprot()&1)){
				printf_protetto();
				return;
				}
			else
				ok_to_proceed= warning_400_800()!=1;
			}
		else
			ok_to_proceed= warning_400_800()!=1;
		}
	}
while(!ok_to_proceed);

fase=reading_disk;

reset_sector_count();
one_empty_line();
if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
	adjust_mode_select();
if(append_mode){
	cerca_fine();
	start_appending();
	if(err_code) error_message("Error, could not backspace on tape\n");
	}
else{
	printf(in_Italia?"Nuovo archivio %car\n":"New %car archive\n",bar_archive?'b':'t');
	if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT){
		if(di.v.si.block_mode==variable_blocks){
			di.v.si.block_size=(long)tar_block_size<<9;
			reinit_buffering((long)tar_block_size<<9);
			use_fixed_sizes();
			}
		}
	}

for(;;){
	unsigned char itemname[32];
	fase=in_writing;
	print_ready();
	settori_passati=0;
	enable_write_items();
	GetIndString (itemname,128,bar_archive?2:1);
	SetItem (writeMenu,wmWriteTar,itemname);
	EnableDisableItem(writeMenu,wmSetLabel,bar_archive);

	/*DrawMenuBar(); serviva quando "disable_write_items disabilitava tutto il menù */
	close_all_open_WD();
	check_ev_timing=120;
 	flush_buffers(1);

	enable_autoflush();
	last_selection=0;
	while(last_selection==0){
		if(!bar_archive) EnableDisableItem(writeMenu,wmSetLabel,tar_version==tar_GNU);
		MainEvent();
		}
	disable_write_items();
	/*DrawMenuBar();*/
	log_menu_command(last_selection);

	if(last_selection==menuItemMess(fileID,fmAbort) ||
	   last_selection==menuItemMess(writeID,wmEndWrite) ||
	   last_selection==menuItemMess(fileID,fmEject) ){
		if(di.tipo_device==d_file){
			extern short openfile_vrefnum;

			get_openfile_location(di.m_vrefnum);
			FlushVol(NULL,openfile_vrefnum);
			}

		if(last_selection!=menuItemMess(writeID,wmEndWrite)){
			if(di.tipo_device==d_driver && di.v.fi.opened_by_insertion==1)
				diskEject(true);
			else if(last_selection==menuItemMess(fileID,fmEject))
				chiudi_file();
			}
		else if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
			do_end_of_write_for_tape();
		flush_buffers(2);

		return;
		}
	else if(hiword(last_selection)==writeID){
		my_tar(loword(last_selection));
		fine_lavoro();
		}

	/* non ci sono altri menù attivi: in principio pensavo di lasciare il List 
	(senza espellere!) ma non serve, man mano che scrivo files sulla console
	vengono scritti i nomi ! */
	}
}

/****************************/

void disk_initialize()
{
short inPlace;
short i;
Boolean bad;

fase=initializing;

i=aspetta_inserzione(in_Italia ? "\pInserisci il disco da inizializzare":
	"\pInsert the disk to be initialized",0);

i=testa_stato(&inPlace,0);
if(i!=0 || !inPlace){
	diskEject(true);
	return;
	}
if(is_wrprot()&1){
	printf_protetto();
	SysBeep(5);
	return;
	}
if(warning_first_write(1,&bad)<0){
	diskEject(true);
	return;
	}

disk_format(crea_mac_dir);
if(di.tipo_device!=d_notopen){
	if(testa_stato(&inPlace,1)){
		diskEject(true);
		}
	}
}


/*************************************************/

enum formats identify_format()
/* asks to the device driver about the current disk, than examines the
first sectors to see whether it's a tar or bar archive
It's NOT called for tapes, since it reads some sectors and then it might
not be possible to rewind to the previous position
*/
{
short i;
sector_t j;
extern short Sony_driver;
#define SUPERDRIVE	4

leggi_settore(di.archive_start_sector,disk_buffer);
if(err_code<0){
	SysBeep(5);
	if(di.tipo_device==d_file){
		if(err_code==eofErr)
			printf(in_Italia?"Data fork vuota\n":"Empty data fork\n");
		else
			printf("File access error %d\n",err_code);
		}
	else if(di.tipo_device==d_driver){
		if( di.m_vrefnum==Sony_driver && di.drive_number<=4 &&
		   (i=drive_type[di.drive_number-1]&0x0F)!= SUPERDRIVE &&
	  	   i!= SUPERDRIVE+1){
			printf(in_Italia ? "Disco non inizializzato o in formato 720/1440K\n" : 
				"The disk is either uninitialized or 720/1440K\n");
			printf(in_Italia ? "Prova a inserirlo in un Superdrive" :
				"Try to insert it in a SuperDrive");
			if(n_superdrives==0)
				printf(in_Italia?" (su un Macintosh che ce l\'abbia)":
				" (on a Macintosh which has one)");
			vai_a_capo();
			}
		else if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized)
			printf(in_Italia ? NON_INIZIALIZZATO :
				NOT_INITIALIZED);
		else
			printf(in_Italia ? "Disco illeggibile\n" :
				"The disk is unreadable\n");
		}
	else{	/* SCSI: currently my SCSI routines don't differentiate
			very much among various error causes, but something may be done */
		if(err_code==eofErr && di.v.si.DeviceType==TAPE_UNIT)
			printf("Error: end of file or end of medium. Rewind and retry\n");
		else if(err_code==-474)
			printf("Error: device is not responding\n");
		else
			printf("Error %d reading device\n",err_code);
		}

	return unreadable_disk;
	}
previousFormat=tar_unknown;

if( (fase==non_faccio_nulla || fase==in_writing) && di.os!=unknown_os)
	return unknown_format;	/* durante un list o altro devo dire che il
			formato non è tar, ma per una unexpected_disk_insertion non
			c'è motivo di dare altri messaggi */

if(di.os==mac_MFS||di.os==mac_HFS){
	/* disk_buffer still contains sector 0 */
	if(check_all_zero(disk_buffer) ){
		printf(in_Italia ? "Il disco non è in formato tar né bar\n" :
			"The disk format is neither tar nor bar\n");
		return unknown_format;	/* A Mac disk
		with a null sector 0 should not be classified as an empty tar archive */
		}
	}

i = tar_check_settore0(di.archive_start_sector,false);
if(i>=0){
	if(previousFormat==tar_GNU)
		printf(in_Italia ? "Archivio in formato GNU tar\n" : "GNU tar archive\n");
	else
		printf(in_Italia ? "Archivio in formato tar\n" : "Tar archive\n");
	use_fixed_sizes();

	return tar_format;
	}
i= bar_check_settore0(di.archive_start_sector,&j,false);
if(i>=0){
	printf(in_Italia ? "Archivio in formato bar\n" : "Bar archive\n");
	use_fixed_sizes();
	return bar_format;
	}
else{
/* I controlli fatti da quelle routines sono rigorosi, basta che il checksum 
sia sbagliato...  non vorrei solo per quello perdere ogni
possibilità di esaminare il disco, la vecchia versione non è affatto rigida in
proposito e neanche la nuova lo è per gli altri header, quindi faccio una 
discriminazione molto più approssimativa, tanto my_untar e my_unbar poi hanno
dei controlli di errore */

	if(((barh_type*)disk_buffer)->bar_magic[0]== 'V' && 
	    untar_dec_number(((barh_type*)disk_buffer)->volume_num,-1)!=-1){
		printf(in_Italia ? "Archivio in probabile formato bar\n" : "It looks like a bar archive\n");
		use_fixed_sizes();
 	   	return bar_format;
	   	}
	else if(untar_number(((tarh_type*)disk_buffer)->size,-1)!=-1 && 
		untar_number(((tarh_type*)disk_buffer)->mtime,-1)!=-1){
		printf(in_Italia ? "Archivio in probabile formato tar\n" : "It looks like a tar archive\n");
		use_fixed_sizes();
	 	return tar_format;
	 	}
	else{
		if(fase!=hack_listing){
			SysBeep(5);
			disable_autoflush(2);
			if(di.tipo_device==d_file)
				printf(in_Italia ? "Il file" : "The file");
			else if(di.os!=unknown_os)
				printf(in_Italia ? "Il disco" : "The disk");
			else
				printf(in_Italia ? "Il disco è inizializzato ma" : "The disk is initialized but its");
			printf(in_Italia ? " non è in formato tar né bar" : 
				" format is neither tar nor bar");
			if(di.tipo_device!=d_file && tar_version==tar_POSIX)
				printf(in_Italia?" (ma potrebbe essere un disco tar oltre il primo)":
					" (but it could be a tar disk following the first one)");
			vai_a_capo();
			enable_autoflush();
			}
		return unknown_format;
		}
	}
}


/***************** apri file *********************/

short apri_file(modo,who)		/* opens a file */
char *modo;
short *who;
{
short io;

reply.fName[reply.fName[0]+1]='\0';	/* per il caso debba fare una printf...*/
/* SetVol(NULL,reply.vRefNum);  setta la directory: inutile */

if(modo[0]=='r')
	io=FSOpen (reply.fName, reply.vRefNum, who);
else if(modo[0]=='w')
	if(bar_archive)
		io=create_overwrite(bar_creator, bar_type, who);
	else
		io=create_overwrite(tar_creator, 'TARF', who);
if(io!=noErr) {
	start_of_line();
	printf("failure opening %P (",reply.fName);
	if(io==opWrErr)
		printf("already open with write permission)\n");
	else
		printf("error %d)\n",io);
	return -1;
	}
return 0;
}

void reassign_type_creator(short,unsigned char*,OSType,OSType);
void reassign_type_creator(vrefnum,name,t,c)
short vrefnum;
unsigned char *name;
OSType t,c;
{
FileParam fpb;
fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;
fpb.ioVRefNum=vrefnum;
fpb.ioNamePtr= name;
if (PBGetFInfoSync(&fpb)==noErr){
	fpb.ioFlFndrInfo.fdType=t;
	fpb.ioFlFndrInfo.fdCreator=c;
	PBSetFInfoSync(&fpb);
	}
}


OSErr new_text_file(outputFile)		/* creates and opens a text file */
short *outputFile;
{
short io;
/* SetVol(NULL,reply.vRefNum); setta la directory: inutile e pericoloso */

return create_overwrite(text_creator, 'TEXT',outputFile);
}

OSErr create_overwrite(c,t,who)
OSType t,c;
short *who;
{
OSErr io;
io=Create(reply.fName, reply.vRefNum, c, t);
if (io==noErr)
	io = FSOpen( reply.fName, reply.vRefNum, who );
else if(io==dupFNErr){
	io = open_overwrite( reply.fName, reply.vRefNum, who );
	if(io==noErr)
/* settare tipo e creatore: certo se il file esiste già dovrebbero
essere gli stessi, ma meglio non fidarsi !
-- type and creator must be set even if the file already existed
*/
		reassign_type_creator(reply.vRefNum,&reply.fName,t,c);
	}

return io;
}

short mac_fread(buff,nbytes,refNum)
char *buff;
short nbytes;
short refNum;
{
long count = nbytes;
if( (err_code=FSRead( refNum, &count, buff )) != noErr){
	if(err_code==eofErr){
		fillmem(&buff[count],0,512-(short)count);
		err_code=noErr;
		}
	else
		return -1;
	}
return (short) count;
}

#if 0
/* no more used, it does not do buffering hence it's slower than write_hd */
short mac_fwrite(buff,nbytes,refNum)
char *buff;
short nbytes;
short refNum;
{
long count=nbytes;
if( (err_code=FSWrite( refNum, &count, buff )) != noErr )
	return -1;
return (short)count;
}
#endif

OSErr open_overwrite (fileName,vRefNum,refnum)
/* apre la data fork del file, tagliando a zero bytes sia questa sia la
resource fork
-- opens the file data fork, truncating it and the resource fork to zero length
*/
char *fileName;
short vRefNum,*refnum;
{
short i = FSOpen(fileName,vRefNum, refnum);
if(i==noErr){
	SetEOF(*refnum,0L);
	if( OpenRF (fileName,vRefNum,&i)==noErr){
		SetEOF(i,0L);
		FSClose(i);
		}
	i=noErr;
	}
return i;
}

/**************** gestione eventi durante le operazioni ************/

Boolean my_disk_is_in()
{
/* Called before exiting from a pause: checks whether the disk is where is
expected, and if there are some obvious reasons to believe it's not the
expected disk
*/
static char s1[]="Disco cambiato !\n",
	s2[]="Changed disk !\n";
if(di.tipo_device!=d_driver) return true;
	{
	short inPlace;
	sector_t old_s=di.sectors_on_floppy;
	if (testa_stato(&inPlace,0) || !inPlace){
		printf(in_Italia?"Disco assente !\n" : "Missing disk !\n");
		flush_console();
		return false;
		}
	if(di.sectors_on_floppy!=old_s || not_my_disk() ){
		di.sectors_on_floppy=old_s;
		printf(in_Italia?s1:s2);
		return false;
		}
	di.sectors_on_floppy=old_s;
	}
	{
	char buffer[512];
	short refnum;
	long space;
	enum os_type i;
	if( GetVInfo (di.drive_number, buffer, &refnum,&space) == nsvErr ) return true;
		 /* no such volume: a non-Mac disk is not mounted hence...*/
	read_one_sector(2,buffer,di.drive_number);
	if(!err_code) i=riconosci_disco_mac(buffer);
	if(err_code || i!=mac_HFS&&i!=mac_MFS){
		UnmountVol (NULL,di.drive_number);	/* in spite of the above comment,
		 	in some cases (see comments to de_Mac_ize) it's mounted, at least if
		 	suntar is not in foreground */;
		return true;
		}
	printf(in_Italia?s1:s2);
	return false;
	}
}

void handle_pause()
{
	enum tipo_fase old=fase;
	extern DialogPtr ListDialog;
	fase=paused;
	last_selection=0;

	if(my_windows[0].used) SetWTitle (&my_windows[0],in_Italia?"\p<< in pausa >>":"\p<< paused >>");
	{unsigned char itemname[32];
	GetIndString(itemname,129,5);
	SetItem (myMenus[fileM], fmPause, itemname);
	}
	flush_all();

	while(last_selection!=menuItemMess(fileID,fmPause) && 
		  last_selection!=menuItemMess(fileID,fmAbort)){
		MainEvent();	/* be', non tutti
			i filtri ha senso chiamarli anche durante un pause, ma in realtà
			il pause non è attivo durante i filtri che "lavorano" come
			quello del disk insertion, restano solo quelli dei dialoghi che
			vanno chiamati perché gestiscono gli update
			--not all filters must continue to be called during a pause,
			but when pause is enabled some filters are not used, and the
			filters of semimodal and of the selection dialog must be called
			during pauses
			*/
		if(last_selection==menuItemMess(fileID,fmPause) && di.tipo_device==d_driver &&
			di.drive_number && !my_disk_is_in() ){
			last_selection=0;
			}
		}
	log_menu_command(last_selection);
	if(last_selection==menuItemMess(fileID,fmPause)) last_selection=0;
	{unsigned char itemname[32];
	GetIndString(itemname,129,4);
	SetItem (myMenus[fileM], fmPause, itemname);
	}
	if(my_windows[0].used) SetWTitle (&my_windows[0],window_title);
	fase=old;
}

Boolean check_filter(EventRecord *);	/* Think C 5 no more requires these
	silly prototypes before a function declaration, but only for static functions,
	and anyway suntar must still be compilable by Think C 4 */
static Boolean check_filter(theEvent)
/* event filter used while suntar is working: most events are ignored
without being handled
*/
EventRecord *theEvent;
{
switch(theEvent->what){
case keyDown:
	if(theEvent->modifiers&cmdKey) break;	/* se accetta la selezione menù deve
											accettare anche i command-tasto */
	/* else prosegui in sequenza 
	-- else, since there is no break, continue with the following instruction */
case autoKey:
	return false;	/* the event must not be handled */
	break;
case mouseDown:
	{
	WindowPtr		whichWindow;
	short code=FindWindow( theEvent->where, &whichWindow );
	if(code!=inMenuBar && code!=inSysWindow && code!=inDrag &&
		code!=inGrow && whichWindow==FrontWindow())
		return false;
	else
		SetCursor(&arrow);
	}
	break;
}
return true;
}


void check_events(anyway)
/* handle the events which are allowed while suntar is working, including
the Pause command
*/
Boolean anyway;
{

/* We've discovered that, in typical file extractions, check_events occupied up
to 35% of processing time, even if no event was pending. Hence, events must be
checked less often, but with care, otherwise suntar would lose its feel (it's
good to be able to change preferences on the fly, and while it's working in
background one does not like to notice a huge speed difference in the foreground
application) */

if(last_selection!=menuItemMess(fileID,fmPause)){ /* that's used by the following function */
	EventRecord		myEvent;
	++check_ev_timing;
	if(!anyway && check_ev_timing< (gInBackground?back_prio:(fase!=reading_disk&&fase!=hack_listing||
		!listonly)?18:3) ){
		if((check_ev_timing&1) || !OSEventAvail(everyEvent,&myEvent))
			return;
		}
	check_ev_timing=0;

	sto_lavorando=pausable=true;

	/* what follows is the body of MainEvent, with an extra call in order to ignore
	all the events which are not urgent */

	if(get_event(&myEvent)){
		if(check_filter(&myEvent)){
			if(my_event_filter!=NULL) (*my_event_filter)(&myEvent);
			handle_event(&myEvent);
			}
		}
	sto_lavorando=pausable=false;
	}
if(last_selection==menuItemMess(fileID,fmPause)){
	log_menu_command(last_selection);
	handle_pause();
	}
accept_abort_command();
}

short check_error_and_events()
{
/* in theory it's:
if(check_error()) return -1;
check_events(false);
return 0;
in practice, it was extended to transform a "missing disk" error to a pause,
so you (and we) won't lose your work for an unwanted eject (it happened to us
for a 4Megabytes file, filling 3 disks, and we had to start again after
half an hour of work in background). But the caller must read the sector again,
hence a new return code of 1
*/
if(err_code!=-65){
	if(check_error()) return -1;
	check_events(false);
	return 0;
	}
else{
	beep_in_foreground();
	start_of_line();
	printf(in_Italia?"Disco assente, pausa forzata\n":"Missing disk, a pause was forced\n");
	last_selection=menuItemMess(fileID,fmPause);
	check_events(true);
	return 1;
	}
}

void accept_abort_command()
{
if(last_selection == menuItemMess(fileID,fmAbort)){
	if(fase==reading_disk&&!listonly||fase==writing_disk||fase==selected_reading||fase==paused){
		if(dialogo_abort()==2){
			last_selection=0;
			return;
			}
		}
	one_empty_line();
	error_message(in_Italia?"Comando abortito\n":"Command aborted\n");
	}
}

Boolean is_abort_command()
/* external routines must not know how I receive the Abort command,
but they must be able to know when they must abort themselves */
{
return last_selection == menuItemMess(fileID,fmAbort);
}

Boolean is_pause_command()
{
return last_selection == menuItemMess(fileID,fmPause);
}

/****************** gestione dialogo di inserzione *********/

short filter_insert(EventRecord*);
static short filter_insert(theEvent)
EventRecord*theEvent;
{
if(di.tipo_device!=d_notopen)	/* the user may call the about_box, but about_box does
	not delay the handling of disk insertions (as options_box does), hence it's
	possible that the disk insertion event was handled by unexpected_disk_insertion */
	return -1;
else if(theEvent->what==diskEvt){
	di.drive_number=loword(theEvent->message);

	if(find_driver()==noErr){
		/*FlushVol (PNS,di.drive_number);*/
		UnmountVol (NULL,di.drive_number); /* smonta qualunque 
		cosa si trovasse in precedenza in quel drive, e il disco stesso se è stato
		montato: inutile guardare al codice di errore ritornato
		-- the Finder often mounts even a tar archive, if it was formatted on a Mac
		(see de_Mac_ize in tar.c): anyway, it must not be mounted if I want to use 
		it without being disturbed by the Finder; the returned error code may be 
		ignored, I don't worry to call this only when it was mounted...
		*/
		di.tipo_device=d_driver;
		aggiorna_nome_aperto();
		return -1;
		}
	else{
		di.tipo_device=d_notopen;
		aggiorna_nome_aperto();
		di.drive_number=0;
		}
	}
return 0;
}

/************* aspetta inserzione **************/

short aspetta_inserzione(message,flags)
/* if a file or disk is already open, return immediately; otherwise,
present the disk insertion dialog and update all the status informations
about the current disk
returns
-1 or a error code <0: some error occurred, or the user clicked on Cancel
0: disk correctly inserted
*/
const char*message;
short flags;
{
static Point wPos={-1,-1};
static char*titoli[]={"\pAnnulla","\pEsamina il disco"};
short inPlace;
short i;

if(di.tipo_device==d_driver){
	i=testa_stato(&inPlace,0);

	if(i && !inPlace && !di.v.fi.opened_by_insertion){
			printf(in_Italia?DISCO_ASSENTE:MISSING_DISK);
			return -1;
			}
	if(i==0){
		if(inPlace){
			if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized)
				printf(in_Italia?NON_INIZIALIZZATO:NOT_INITIALIZED);
			return 0;
			}
		else{
			printf(in_Italia?DISCO_ASSENTE:MISSING_DISK);
			return -1;
			}
		}
	else
		diskEject(true);
	}
else if(di.tipo_device==d_file)
	return 0;
else if(di.tipo_device==d_scsi){
	i=test_SCSI_device(&inPlace,0);
	/* analizza i risultati ? */
	return 0;
	}

if(preferred_drive){
/* if there only one disk drive, or only one is a SuperDrive, eject anything
was currently in that drive, since it's the only place where the user may insert
the requested disk. But don't do that if a disk insertion on that drive has just
happened
*/
	EventRecord event;
	extern short Sony_driver;
	if ( !OSEventAvail(diskMask,&event) || preferred_drive!=loword(event.message)){
		/*FlushVol (PNS,preferred_drive);*/
		UnmountVol (NULL,preferred_drive);	/* inutile guardare al codice di errore ritornato */
		di.drive_number=preferred_drive;
		di.m_vrefnum=Sony_driver;
		di.tipo_device=d_driver;
		di.v.fi.opened_by_insertion=true;
		diskEject(true);	/* devo eiettare, non chiudere un device aperto */
		}
	}

invalid_buffers();
i=semimodalDialog((flags&examine_disk)?144:138,&wPos,filter_insert,&titoli,(flags&examine_disk)?2:1,
	(char*)message,in_Italia?"\p\r\rin un disk drive":"\p\r\rin a floppy disk drive",NULL,
	teJustCenter,(flags&button_is_stop)|(fase==ricevuto_comando||fase==initializing),
	(flags&examine_disk)?set_exam_checkbox:NULL);

if(i==-1){
	/* BEWARE ! there are five routines which may "open something":
	aspetta_inserzione, unexpected_disk_insertion, apri_archivio,
	nuovo_file,	scegli_device. Any new field "di." must be filled
	in all these places, unless it's used only if(di.tipo_device==...)
	(currently, only the di.v.m_vrefnum) or it's like di.file_format (which is
	initialized to 0, modified when tipo_device becomes d_file and is
	cleared again when the file is closed)
	*/

	reinit_buffering(512);
	di.sectors_on_floppy=0;	/* per via di testa_stato... */
	di.writing_allowed=true;
	di.signature_avail=false;
	di.v.fi.opened_by_insertion=true;
	di.archive_start_sector=0;
	di.isEjectable=true;	/* non necessariamente, se uno smonta un disco
				non eiettabile il device driver può fare PostEvent di una diskInsertion */
	if(testa_stato(&inPlace,1)){
		if(fase==initializing) return 0;
		diskEject(true);
		return -1;
		}
	store_disk_signature();
	return 0;	/* disco inserito OK */
	}

return -1;	/* i==1, there are no other possible cases */
}

/************************************/


void print_HFS_catalog(void);
static void print_HFS_catalog()
/* visits the HFS catalog without passing trough the File Manager: that's because
I do not want to mount it */
{
int n_cat,n_last,i;
long n_tot_previsti,n_tot=0;
/*leggi_settore(2,disk_buffer); no, the caller has already done that */
n_cat=(*(short*)&disk_buffer[150]) * (*(short*)&disk_buffer[22]>>9) + *(short*)&disk_buffer[28] /*+1 ???*/;
n_last=n_cat + (*(short*)&disk_buffer[152]) * (*(short*)&disk_buffer[22]>>9);
n_tot_previsti=*(long*)&disk_buffer[84];

/*printf("primo sett cata=%d last=%d\n",n_cat,n_last);*/
/*disable_autoflush(1); already done by the caller */
for(i=n_cat;i<=n_last;i++){
	leggi_settore(i,disk_buffer);
	/*printf("sect %d contiene tipo %x lev %d nrecs=%d\n",i,disk_buffer[8]&255,disk_buffer[9],
	*(short*)&disk_buffer[10]); */

	if(disk_buffer[8]==0xFF){
		char *p,*p1;
		int j,n_recs=*(short*)&disk_buffer[10];
		for(j=0;j<n_recs;j++){
			p=&disk_buffer[*(short*)&disk_buffer[510-j-j]];
			/*printf("nome=%P ",p+6);*/
			p1= p+6 + (p[6]|1) +1 ;
			if(*p1==2){
				printf("file   %P\n",p+6);
				/* parent ID=2 significa nella root, vedi IM 4 pag. 92 */
				n_tot++;
				if(n_tot==n_tot_previsti) goto fuori;
				}
			else if(*p1==1 && *(long*)(p+2)!=1)	/* parent id=1 is used for the root itself */
				printf("folder %P\n",p+6);
			#if 0
			else if(*p1==3)
				printf("(thread)\n");
			else{
				printf("(boh %d)\n",*p1);
				/*stampa_buffer(i,disk_buffer,0);*/
				}
			#endif
			}
		}
	}
fuori: ;
/*printf("n_tot=%ld\n",n_tot_previsti);*/
}

/**********************************/

void esamina_disco()			/* examine a disk */
{
/* save global variables which must not be altered, but are used by
some routines which I must call now */
enum tipo_fase oldfase=fase;
unsigned char savePrevFormat=previousFormat;
Boolean savebar_archive=bar_archive;
char savebuffer[512];
enum formats j;
mcopy(savebuffer,disk_buffer,512);

fase=reading_disk;	/* otherwise abort could be disabled */
listonly=1;		/* don't use buffering */
/* I'm in a very peculiar situation, I must do a rather complex operation,
requiring a number of function calls, but this operation is launched from
within a very low level of another high level operation (disk_buffer is
full, etc.). Hence, I must pay a lot of attention, suntar contains a lot
of global variables and most of them can't be altered in this moment.
 Furthermore, this operation could raise an error, but I can't allow 
that raise_error be called, since it resets a lot of global variables,
closes open files...
 The only reasonable solution is not to use the standard way of doing a
List, but recreating it by using low-level functions (low enough that I can
carefully examine all their instructions, and the instructions of the
functions called by them) so that I can be sure that raise_error is not
called, and I may be sure that the list of altered global variables is
accurate.
 I wanted that the Abort command were active during this List (aborting
the List and not anything else), hence I must handle it before it causes
a raise_error, and that requires not using check_events too.
*/

if(di.os==unknown_os)
	j=identify_format();	/* it can't cause a call to raise_error */
else
	j=unknown_format;
if(j==tar_format||j==bar_format){
	bar_archive= j==bar_format;
	list_conciso();
	one_empty_line();
	}
else if (di.os==mac_MFS || di.os==mac_HFS){	/* Try to print some informations,
		but without mounting the disk */
	leggi_settore(2,disk_buffer);
	if(err_code==0){
		static char *tot_files[2]={"Total files=%ld ","Numero totale di files=%ld "};
		disable_autoflush(2);
		printf("Volume %P (%ld KB ",&disk_buffer[36],
		   ((*(unsigned short*)&disk_buffer[34]) * (*(long*)&disk_buffer[20]) )>>10 );
		printf(in_Italia?"disponibili)\n":"free)\n");
		if(di.os==mac_HFS){
			printf(tot_files[in_Italia?1:0],
			 (long) ( max(1L,*(long*)&disk_buffer[84]) -1)  );	/* do NOT count the
			 		Desktop file, but if it does not exist do not declare -1 files */
			printf(in_Italia?"(di cui %u nella radice)":"(%u in root)",
				max(1,*(unsigned short*)&disk_buffer[12]) -1 );
			printf(in_Italia?" numero cartelle totali=%ld\n":" total folders=%ld\n",*(long*)&disk_buffer[88]);
			print_HFS_catalog();
			}
		else{
			printf(tot_files[in_Italia?1:0],
			 (long) ( max(1,*(unsigned short*)&disk_buffer[12]) -1)  );
			vai_a_capo();
			}
		enable_autoflush();
		}
	}
else if(di.os==msdos){
	unsigned short ris[3];
	if(!visita_FAT_msdos(&ris)){
		printf(in_Italia?"%uK usati, %uK liberi, %u settori difettosi\n" :
		"%uK used, %uK free, %u bad sectors\n",(ris[2]-ris[0])>>1,ris[0]>>1,ris[1]);
		}
	}

bar_archive=savebar_archive;
previousFormat=savePrevFormat;
mcopy(disk_buffer,savebuffer,512);
fase=oldfase;
}

void list_conciso()	/* used by the "examine a disk" button */
{
long length;
sector_t s_n=0;

disable_autoflush(1);

printf(in_Italia?"Contenuto attuale del disco:\n":"Current content of this disk:\n");
if(bar_archive){
	short f_n;
	length=untar_number( ((barh_type*)disk_buffer)->size,-1);
	f_n=untar_dec_number( ((barh_type*)disk_buffer)->volume_num,-1);
	
	if(length==-1 || f_n==-1){
		printf(in_Italia?"Testata non in formato bar !\n":
  			"Error: not a bar header !\n");
  		enable_autoflush();
  		return;
  		}
	s_n = (length+1023)>>9;
	printf(in_Italia?"Disco numero %d":"Disk number %d",f_n);
	print_if_string(in_Italia?" dell\'archivio %s":" of the archive %s",
		((barh_type*)disk_buffer)->name,100);
	vai_a_capo();
	}

while(s_n<di.sectors_on_floppy){
	register unsigned char linkflag;

	if(list_events()){
		last_selection=0;
		one_empty_line();
		printf(in_Italia?"List interrotto\n":"List aborted\n");
		enable_autoflush();
		return;
		}
	leggi_settore(s_n,disk_buffer);
	if(check_error()){
		if(err_code==-65){	/* missing disk... */
			 DeleteMenu(deviceID);
			 DeleteMenu(deviceID+1);
			 DeleteMenu(deviceID+2);
			 DrawMenuBar();
			 di.drive_number=0;
			 di.tipo_device=d_notopen;	/* sono in una situazione particolare,
			 	per cui tutte le altre operazioni da fare in caso di chiusura
			 	non hanno senso */
			 aggiorna_nome_aperto();
			 }
		enable_autoflush();
		return;
		}

	if(check_all_zero(disk_buffer)){
		if(s_n==0||(bar_archive||hasVheader)&&s_n==1)
			printf(in_Italia?"Archivio vuoto\n":"No files in the archive\n");
		else
			printf(in_Italia?"Fine archivio\n":"End of archive\n");
		enable_autoflush();
		return;
		}
	linkflag= get_linkflag(&length,-1);
	if(linkflag==0xFF){
 		printf(in_Italia?"Testata non in formato %car !\n":
  			"Error: not a %car header !\n",bar_archive ? 'b' : 't');
		enable_autoflush();
 		return;
		}
	if(linkflag!='1'&&linkflag!='2'){
		if(linkflag=='M'){
			if(!bar_archive) printf(in_Italia?"Parte del file %s\n":
				"Part of file %s\n",((tarh_type*)disk_buffer)->name);
			}
		else if(linkflag=='V'){
			if(!bar_archive) printf(in_Italia?"Il nome del volume è %s\n":
				"The volume name is %s\n",((tarh_type*)disk_buffer)->name);
			}
		else
			print_info(bar_archive ? ((barh_type*)disk_buffer)->name :
				((tarh_type*)disk_buffer)->name,length);
		}
	s_n += (length+1023)/512;
	}
printf(in_Italia?"Continua su un altro volume\n":"Continuing on another volume\n");
enable_autoflush();
}

static Boolean list_events()
/* replaces check_events during list_conciso */
{
EventRecord		myEvent;

++check_ev_timing;
if( check_ev_timing<3) return false;
check_ev_timing=0;

sto_lavorando=true;

if(get_event(&myEvent)){
	if(check_filter(&myEvent)){
		if(my_event_filter!=NULL) (*my_event_filter)(&myEvent);
		handle_event(&myEvent);
		}
	}
sto_lavorando=false;

if(last_selection == menuItemMess(fileID,fmAbort)){
	log_menu_command(last_selection);
	return true;
	}
else
	return false;
}

/*************************************************/

void check_foreground()
{
/* to be called before interacting with the user when I could be 
running in background: if I am, call the Notification Manager to alert
the user and wait for him/her to bring this application to foreground
*/

if(gInBackground){
	NMRec my_notif_rec;

	my_notif_rec.qType=8;
	my_notif_rec.nmMark=1;
#ifndef THINK_C_5
	my_notif_rec.nmSIcon=GetResource ('SICN',128);
#else
	/* with another version of header files, it's */
	my_notif_rec.nmIcon=GetResource ('SICN',128);
#endif
	my_notif_rec.nmSound=(void*)-1;
	my_notif_rec.nmStr=NULL;
	my_notif_rec.nmResp=NULL;
	NMInstall(&my_notif_rec);
	while(gInBackground)
		MainEvent();
	NMRemove(&my_notif_rec);
#ifndef THINK_C_5
	ReleaseResource(my_notif_rec.nmSIcon);
#else
	ReleaseResource(my_notif_rec.nmIcon);
#endif
	}
settori_passati=0;

}

void beep_in_foreground()
{
if(gInBackground)
	check_foreground();		/* which does a beep */
else
	SysBeep(5);
}


/***********************/

void update_about_box(EventRecord*);
static void update_about_box(theEvent)
EventRecord *theEvent;
{
WindowPtr theWindow=(WindowPtr)theEvent->message;
static Rect
	boundsR1={10,105,30,475},
	boundsR2={36,130,148,475},
	boundsR3={148,5,270,280},
	dstRect1={10,5,134,129},
	dstRect2={158,288,268,476};
Handle h;
GrafPtr	savePort;
char*p;

GetPort( &savePort );
SetPort(theWindow);
BeginUpdate( theWindow );

h=GetResource ('PICT',128);
HLock(h);
DrawPicture (h, &dstRect1);
ReleaseResource(h);
h=GetResource ('PICT',129);
HLock(h);
DrawPicture (h, &dstRect2);
ReleaseResource(h);

TextFont(3);
TextSize(12);

TextBox(&version_string[1],(long)version_string[0],&boundsR1,teJustCenter);
TextFont(4);
TextSize(9);

if(h=GetResource('TEXT',128+in_Italia)){
	HLock(h);
	TextBox(*h,(long)GetHandleSize(h),&boundsR2,teJustRight);
	ReleaseResource(h);
	}	

if(h=GetResource('TEXT',130+in_Italia)){
	HLock(h);
	TextBox(*h,(long)GetHandleSize(h),&boundsR3,teJustLeft);
	ReleaseResource(h);
	}

EndUpdate( theWindow );
SetPort (savePort);

}

void about_box()
{
static Rect
	winRect={0,0,272,480};
WindowPtr theWindow;
short stato=0;
EventRecord		myEvent;
enum tipo_fase old_fase=fase;


#undef NULL		/* ????? non capisco che succede */
#define NULL 0L

flush_all();
PositionDialog(&winRect);
theWindow=NewWindow( NULL, &winRect, PNS, 1, dBoxProc, (WindowPtr)-1L, 0, (long)-1);
((WindowPeek)theWindow)->refCon= -3;

install_handlers(theWindow, update_about_box, NULL);
while(stato!=2){
	sto_lavorando=false;	/* cursor shape and delay for WaitNextEvent... */
	fase=paused;			/* for unexpected_disk_insertion */
	if(get_event(&myEvent)){
		if(myEvent.what==diskEvt){
			short olddrive=di.drive_number;
			if(di.tipo_device!=d_driver) olddrive=0;
			unexpected_disk_insertion(myEvent.message);
			if(di.tipo_device==d_driver && olddrive!=di.drive_number) break;
			}
		else if(myEvent.what==mouseDown)
			stato=1;
		else if(stato==1&&myEvent.what==mouseUp)
			stato=2;
		/* all other events are ignored: that's anyway a progress upon the 1.1 version, 
		which never called GetNextEvent during the about box (background tasks and 
		even screen savers were disabled...) */
		}
	}

remove_handlers(theWindow);
DisposeWindow(theWindow);
fase=old_fase;
/*Riattiva();*/
}


/************* check all zero **********/

Boolean check_all_zero(buffer)
char *buffer;
{
register short	i=512/sizeof(long);
register long	n_or=0;
register long*	p= (long*) &buffer[0];

while(--i>=0 &&  (n_or|= *p++) ==0L)
	;
return n_or==0L;
}

/************************/

void print_info(name,length)
char *name;
long length;
{
if(name[strlen(name)-1]=='/')
	printf("Directory %s\n",name);
else
	printf("File %s (%ld bytes)\n",name,length);
}

/*****************************/

void raise_error()
{
/* prima di riprendere, chiamo tutte le routine di "cleanup", che ovviamente
se non c'è niente da chiudere ritornano senza fare niente
-- in suntar, all high level routines must use a setjmp to handle
recovering after an error. All permanent entities (variables, files, windows)
which need to be closed or restored to the normal state must be associated to
a clean-up routine (which does nothing if no clean-up is needed) and that
routine must be called here (beware, there's also a raise_hqx_error, which
aborts the extraction of the current file but not the command)
*/
need_a_touch=false;

SetCursor(&waitCursor);	/* closing files may flush the disk cache, and that
			may take several seconds (at least, it was so before suntar had
			its own caching) */
deall_tree();

if(di.tipo_device!=d_notopen && dirty_buffers()){
	short refnum;
	long space;
	if( di.tipo_device!=d_driver || GetVInfo (di.drive_number, disk_buffer, &refnum,&space) == nsvErr ){
		/* no such volume: a non-Mac disk is not mounted hence...*/
		flush_buffers(2);
		}
	else
		invalid_buffers();
	}

my_event_filter=NULL;
if(fase==writing_disk || last_selection!=menuItemMess(fileID,fmAbort)){
	check_and_eject();
	/*chiudi_archivio(); no, ora non più (2.0.3) */
	}
aggiorna_nome_aperto();	/* se era stato interrotto un find headers... */
last_selection=0;
sto_lavorando=pausable=false;
close_input_files();
close_or_del_out_file();
close_info_file();
enable_autoflush();
close_semimodal();
FlushEvents( everyEvent&~diskMask, 0 );
longjmp(main_loop,-1);
}

void error_message(p)
char *p;
{
start_of_line();
printf(p);
raise_error();
}


void error_message_1(p,n)
char *p;
int n;
{
start_of_line();
printf(p,n);
raise_error();
}

/************************/
short check_error()
{
/* da chiamare esclusivamente dopo leggi_settore
-- to be called after leggi_settore */
if(err_code){
/* e se invece riprovassi a leggere ? Non è che il disk driver ritenti già lui? */
	disk_error_message();
	if(!ignore_errors){
		OSErr olderr=err_code;
		if(di.v.fi.opened_by_insertion==1) diskEject(true);
		err_code=olderr;
		return -1;
		}
	}
return 0;
}

void disk_error_message()
{
	start_of_line();
	if(err_code==-39 || di.tipo_device==d_file && err_code==-81)
		printf(in_Italia?"Fine del file prematura\n":"Error: unexpected end of file\n");
	else if(err_code==-65)
		printf(in_Italia?"Errore: disco assente\n" : "Error: missing disk\n");
	else if(err_code<=-66 && err_code>=-71)
		printf(in_Italia?"Settore illeggibile (errore %d)\n": 
			"Unreadable sector (error code %d)\n",err_code);
	else if(err_code<=-72 && err_code>=-73)
		printf(in_Italia?"Settore difettoso (errore %d)\n": 
			"Defective sector (error code %d)\n",err_code);
	else
		printf(in_Italia?"Errore di lettura n° %d\n":"Read error %d\n",err_code);
}

void write_error_message()
/* to be called when mac_fwrite fails */
{
if(err_code==dskFulErr)
	error_message(in_Italia?"Errore: disco pieno\n":"Error: disk full\n");
else if(err_code==ioErr)
	error_message("I/O error during file write\n");
else if(err_code==wrPermErr)
	error_message("Error: no write permission\n");
else
	error_message_1("File write error %d\n",err_code);
}

/***********************************************************************/

void hacking(command)
short command;
/* handles all the commands in the Special menu: mainly for historical reasons
(suntar 1.0 was rather modal) I preferred to loop internally while commands are 
from this menu rather than returning to the main loop after executing
the first command
*/

{
/* visto che i comandi del menù Special sono attivi se e solo se lo sono quelli
del menù file avrebbe senso eseguire un solo comando e ritornare, preferisco 
ciclare anche qui dentro perché così è più facile gestire il comando Again */


short inPlace;
register short i;
Boolean sect_n_valid;
Boolean out_f_open=false;
sector_t default_sect_n=0;
short outputFile;
short again_modifiers,curr_command_modifiers;
static unsigned char messIta[]="\pInserisci il disco di prova",
	messIng[]="\pInsert the test disk",
	mess_wrprot[]="Write protected disk !\n",
	mess_assente[]="Missing disk\n",
	mess_non_mac_ing[]="\pNon-Mac disk",
	mess_empty_ing[]="\pEmpty Macintosh disk",
	mess_continue_ing[]="\p\rAny data in this disk will be lost. Continue ?",
	mess_failed[]="Operation failed\n";

next_header_for_POSIX=-1;
again_command=0;
if(setjmp(main_loop)<0){
	if(out_f_open) FSClose(outputFile);
	aggiorna_nome_aperto();
	if(again_command!=hmList || di.tipo_device==d_notopen)
		return;			/* il disco è stato espulso, non ha senso restare qui dentro;
						non è detto però, forse a volte non espello ! */
	command=0;
	}

if(aspetta_inserzione(in_Italia?messIta:messIng,0))
	return;

for(;;){
	curr_command_modifiers=command_modifiers;
	if(command==0){
		fase=non_faccio_nulla;
		close_all_open_WD();
		check_ev_timing=120;
		flush_buffers(1);
		enable_autoflush();
		last_selection=0;
		while(last_selection==0){
			MainEvent();
			if(!expert_mode) return;
			}
		curr_command_modifiers=command_modifiers;	/* non si sa mai, anche
				scegliere Options dal preference menu riassegna command_modifiers */

		if(last_selection!= menuItemMess(hackID,hmAgain)) log_menu_command(last_selection);
		command=loword(last_selection);
		fase=ricevuto_comando;
		if(hiword(last_selection)!=hackID)
			return;		/* non tocca a me gestirlo */
		if(again_command==hmView && (command==hmView ||
		   command==hmAgain) && default_sect_n<di.sectors_on_floppy-1) 
				default_sect_n++;	/* tipicamente io guardo
		   		il settore prima di farci sopra un clear o read, per cui voglio un
		   		autoincremento solo se il comando resta view
		   		-- I usually view a sector just before clearing or untar-ring, hence
		   		I want auto-increment only if the following command is another View */
		last_selection=0;
		if(sect_n_valid=(command==hmAgain)){
			curr_command_modifiers=again_modifiers;
			command=again_command;
			if(command==hmList){
				diskEject(true);	/* non avrebbe senso farlo due
					volte sullo stesso disco, ma ha senso farlo su più dischi di fila
					-- Do it again used on Expert list means do it on another disk,
					hence the current disk must be ejected */
				if(aspetta_inserzione(in_Italia?messIta:messIng,0))
					return;
				}
			}
		again_command=0;
		}
	else
		sect_n_valid=false;

	switch(command){
		case hmList:
			{
			enum formats fmt;

			if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
				adjust_mode_select();
			if(di.tipo_device==d_driver&&di.v.fi.opened_by_insertion){ /* solo su disco, non su file */
				again_command=hmList;
				again_modifiers=curr_command_modifiers;
				}
			if(next_header_for_POSIX!=-1 && tar_version==tar_POSIX)
				fase=hack_listing;	/* so identify_format does not print
						its "unknown format" message */
			one_empty_line();
			fmt = identify_format();	/* che legge in disk_buffer il settore 0
							-- which leaves a copy of sector 0 in disk_buffer */
			if(next_header_for_POSIX!=-1 && tar_version==tar_POSIX && fmt==unknown_format){
				/* printf("sn=%ld\n",next_header_for_POSIX); */
				if(next_header_for_POSIX < di.sectors_on_floppy){
					leggi_settore(next_header_for_POSIX,&tarh);
					if(check_all_zero(&tarh)){
						print_sector_n(next_header_for_POSIX);
						printf("End of Archive (dangerous, can\'t check it\'s the right volume)\n");
						break;		/*save the value of next_header_for_POSIX */
						}
					else if(untar_number(tarh.size,-1) != -1 &&
							untar_checksum(&tarh,-1,false) != -2)
						fmt=tar_format;
					else{
						printf("Unknown format (maybe wrong volume)\n");
						break;
						}
					}
				else{
					printf("No file starts here (dangerous, can\'t check it\'s the right volume)\n");
					next_header_for_POSIX-=di.sectors_on_floppy;
					break;
					}
				}
			else
				next_header_for_POSIX=-1;

			fase=hack_listing;
			if(fmt==bar_format){
				short n;
				one_empty_line();
				printf(START_READ);
				if(my_atoi(((barh_type*)disk_buffer)->volume_num,&n)!=0)
					printf("No sequence number\n");
				else
					stampa_info_bar(n);
				listonly=1;
				my_unbar(true);		/* che in caso di fase==hack_listing si comporta diversamente
								dal solito per il cambio disco, e stampa più informazioni
								-- which, when fase==hack_listing, behaves differently
								than when called by a List command */
				}
			else if(fmt==tar_format){
				one_empty_line();
				printf(START_READ);
				listonly=1;
				my_untar(true);
				}
			next_header_for_POSIX=-1;
			}
			check_foreground();
			break;
		case hmClear:	/* clear sector */
		case hmOverwrite:
			one_empty_line();
			i=is_wrprot();
			if(i&1){
				printf(mess_wrprot);
				}
			else if(i&2)
				printf(mess_assente);
			else{
				if(command==hmClear){
					printf("Clearing the sector will erase its previous content.\n");
					printf(
"If it was an header, List, Extract and Append will see it as end of the archive\n");
					sect_n=default_sect_n;
					}
				else{
					printf("The previous content will be lost\nstart ");
					sect_n=0;
					} 
				fase=reading_sect_n;
				if(read_sect_n(&sect_n)==noErr){
					if(command==hmClear){
						fillmem(disk_buffer, 0, 512);
						scrivi_settore(sect_n,disk_buffer);
						flush_buffers(0);
						if(err_code)
							printf(diskWriteErrMsg,err_code);
						else{
							if(di.tipo_device==d_file){
								ParamText("\pTruncate the file ?",NULL,NULL,NULL);
								if(my_modal_dialog(130,NULL,0)==1){
									SetEOF(di.m_vrefnum,(long)(sect_n+1)<<9);
									di.sectors_on_floppy=sect_n+1;
									}
								}
							printf(DONE);
							}
						}
					else{
						if(!overwrite_sectors())
							printf(DONE);
						check_foreground();
						}
					default_sect_n=sect_n;
					}
				}
			break;
		case hmView:		/* view sector */
			fase=reading_sect_n;
			i=(curr_command_modifiers&optionKey)!=0;
			one_empty_line();
			if(sect_n_valid)
				printf("View sector %ld\n",(long)sect_n);
			else{
				sect_n=default_sect_n;
				if(di.tipo_device==d_scsi && di.v.si.DeviceType==TAPE_UNIT){
					sector_t get_first(short*);
					sector_t s;
					short l;
					printf("Sectors currently in buffer: ");
					s=get_first(&l);
					if((long)s<0 || l<=0)
						printf("none\n");
					else
						printf("%ld to %ld\n",s,s+l-1);
					}
				sect_n_valid= read_sect_n(&sect_n)==0;
				}
			if(sect_n_valid){
				fase= hack_listing;	/* to disable disk buffering */
				leggi_settore(sect_n,&disk_buffer);
				if(err_code){
					if(di.tipo_device!=d_scsi || di.v.si.DeviceType!=TAPE_UNIT){
						fillmem(disk_buffer, 0, 512);
						read_sectors(sect_n,&disk_buffer,1);
						stampa_buffer(sect_n,disk_buffer,i);
						}
					if(err_code){
						beep_in_foreground();
						disk_error_message();
						}
					}
				else
					stampa_buffer(sect_n,disk_buffer,i);
				/* se lavoro ad alto livello non ha senso aspettarsi che l'avere
				azzerato il mio buffer abbia azzerato ciò su cui la routine di
				write fisico ha messo i suoi dati, quindi in caso di errore non ha
				senso stampare lo stesso il buffer */
				again_command=command;
				again_modifiers=curr_command_modifiers;
				default_sect_n=sect_n;
				if(sect_n<di.sectors_on_floppy-1) sect_n++;
				}
			break;
		case hmSave:
			{sector_t sect_finale;
			my_SF_Put("\pSave as:","\psectors");
			if(!reply.good) break;

			i=new_text_file(&outputFile);
			connect_to_hd_buffering(outputFile);
			if(i!=noErr) break;
			if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
				adjust_mode_select();
			out_f_open=true;
			sect_n=default_sect_n;
			fase=reading_sect_n;
			one_empty_line();
			if((printf("start "),read_sect_n(&sect_n)==0) && 
			   (sect_finale=sect_n,printf("end "),read_sect_n(&sect_finale)==0) ){

				fase=reading_disk;
				do{
					check_events(false);
					leggi_settore(sect_n,&disk_buffer);
					if(check_error()) raise_error();
					if(write_hd(disk_buffer, 512)){
						beep_in_foreground();
						write_error_message();
						}
					sect_n++;
					}
				while(sect_n<=sect_finale);
				}
			if(flush_hd_buffer()){
				beep_in_foreground();
				write_error_message();
				}
			FSClose(outputFile);
			FlushVol(NULL,reply.vRefNum);
			printf(DONE);
			check_foreground();
			out_f_open=false;
			}
			break;
		case hmFind:
			{short err_n=0;
			short n_err_consec=0;
			long exp_check;
			Boolean stampato=false;
			one_empty_line();
			fase=reading_disk;
			listonly=0;		/* full buffering... */
			for(sect_n=0;sect_n<di.sectors_on_floppy;sect_n++){
				check_events(false);
				if(sect_n && !(sect_n%100) ){
					my_itoa(sect_n,disk_buffer);
					my_c2pstr(disk_buffer);
					stampa_stringa_stato(disk_buffer);
					}
				leggi_settore(sect_n,&disk_buffer);
				if(!err_code) n_err_consec=0;
				if(err_code!=noErr){
					printf("sector %ld could not be read\n",(long)sect_n);
					n_err_consec++;
					if(++err_n>max(20,(short)(sect_n/8)) || n_err_consec>30){
						beep_in_foreground();
						error_message("Too many errors, search aborted\n");
						}
					}
				else if(untar_number(((struct tarh_type*)disk_buffer)->size,-1) != -1 &&
						untar_checksum(&disk_buffer,-1,false) == 0){
					disk_buffer[99]=0;
					disable_autoflush(2);
					printf("tar ");
					print_sector_n(sect_n);
					if(((struct tarh_type*)disk_buffer)->linkflag=='V')
						printf("Volume header: ");
					if(((struct tarh_type*)disk_buffer)->linkflag=='M')
						printf("Continuation: ");
					else if(((struct tarh_type*)disk_buffer)->linkflag=='L' || 
							((struct tarh_type*)disk_buffer)->linkflag=='K' )
						printf("Long-name prefix (extract this, not the following header!): ");
					printf("%s\n",disk_buffer);
					enable_autoflush();
					stampato=true;
					}
				else if(sect_n!=0&&untar_number(((barh_type*)disk_buffer)->size,-1) !=-1 &&
						unbar_checksum(&disk_buffer,-1,&exp_check) == 0 ){
					((barh_type*)disk_buffer)->name[99]=0;
					printf("bar ");
					print_sector_n(sect_n);
					printf(" %s\n",((barh_type*)disk_buffer)->name);
					stampato=true;
					}
				}
			if(!stampato) printf("No headers found\n");
			printf(DONE);
			aggiorna_nome_aperto();
			check_foreground();
			}
			break;
		case hmUntar:
		case hmUnbar:
			reinit_full_name();
			fase=reading_sect_n;
			one_empty_line();
			if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
				adjust_mode_select();

			if(sect_n_valid)
				printf("Un%car at sector %ld\n",((command==hmUntar)?'t':'b'),(long)sect_n);
			else{
				sect_n=default_sect_n;
				sect_n_valid= read_sect_n(&sect_n)==0;	/* come sopra */
				}
			if(sect_n_valid){
				leggi_settore(sect_n,&disk_buffer);
				if(err_code){
					disk_error_message();
					}
				else if(check_all_zero(disk_buffer))
					printf("Null header\n");
				else{
					select_directory();
					if(reply.good){
						listonly=0;
						bar_archive= command==hmUnbar;
						fase=hack_reading;		/* per avvertire di non chiedere un disco
												specifico: forse il settore 0 è danneggiato
												o sovrascritto da un altro header bar
												-- essentially, it's used to remember that
												the floppy_n variable may be invalid,
												hence when asking next disk don't rely on it */
						if(bar_archive){
							unbar();
							}
						else{
							reinit_full_name();
							untar();
							if(full_name){
								leggi_settore(sect_n,disk_buffer);
								if(check_error()) raise_error();
								untar();
								}
							}
						print_sector_n(sect_n);
						touch_if_needed();
						FlushVol(NULL,0);
						printf(DONE);
						check_foreground();
						}
					again_command=command;
					again_modifiers=curr_command_modifiers;
					default_sect_n=sect_n;
					}
				}
			break;
		case hmReadStarting:
			reinit_full_name();
			fase=reading_sect_n;
			one_empty_line();

			if(read_sect_n(&sect_n)==0){
				long exp_check;
				if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
					adjust_mode_select();
				leggi_settore(sect_n,&disk_buffer);
				if(err_code)
					disk_error_message();
				else if(check_all_zero(disk_buffer)){
					printf("Null header\n");
					break;
					}
				else if(untar_number(((struct tarh_type*)disk_buffer)->size,-1) != -1 &&
						untar_checksum(&disk_buffer,-1,false) == 0){
					printf("tar header\n");
					bar_archive=0;
					}
				else if(sect_n!=0&&untar_number(((barh_type*)disk_buffer)->size,-1) !=-1 &&
						unbar_checksum(&disk_buffer,-1,&exp_check) == 0 ){
					printf("bar header\n");
					bar_archive=1;
					}
				else{
					printf("Not a tar/bar header\n");
					break;
					}
				select_directory();
				if(reply.good){
					listonly=0;
					fase=reading_disk;
					if(bar_archive)
						my_unbar(false);
					else
						my_untar(false);
					print_sector_n(sect_n);
					touch_if_needed();
					FlushVol(NULL,0);
					printf(DONE);
					check_foreground();
					}
				default_sect_n=sect_n;
				}
			break;
		case hmMac_ize:

			i=is_wrprot();
			if(i&1){
				printf(mess_wrprot);
				}
			else if(i&2)
				printf(mess_assente);
			else{
				Boolean bad;
				i=warning_first_write(1,&bad);
				if(i>=0){
					ioParam pb;
					Str255 volName;
					if(i>0){
						ParamText(i==1?mess_non_mac_ing:mess_empty_ing,
						mess_continue_ing,PNS,PNS);
						if( my_modal_dialog(139,NULL,2) == 2) break;
						}
					ParamText("\pVolume name :",PNS,PNS,PNS);
					if(di.sectors_on_floppy<=800
						/* it must become an MFS disk, and my own routine does not
						support MFS, let the System do the work */
					   || di.sectors_on_floppy>65556 /*4100*/	/* furthermore, my routine
					   		can't create allocation blocks larger then one sector */
					   || bad ){
						if(my_edt_dialog(143,volName,27,UNTITLED)==2) break;
						i=5;	/* the OK button means "safe", which is button 5 in
								DLOG 150 */
						}
					else{
						if((i=my_edt_dialog(150,volName,27,UNTITLED))==2) break;
						}
					printf("Creating directory...\n");
					FlushVol (NULL,di.drive_number);	/* don't let a dirty
							buffer remain unflushed, flushing it later will
							overwrite its new status */
					if(i==5){
						fillmem(disk_buffer, 0xF6, 512);
						write_sectors(0,disk_buffer,1);
						invalid_buffers();
						if(err_code)
							check_wr_err();
						else{
							DIZero (di.drive_number,volName);
							di.v.fi.opened_by_insertion=0;	/* altrimenti
								espelle il disco, cosa che non voglio affatto */
							chiudi_file();
							pb.ioVRefNum=di.drive_number;
							PBMountVol(&pb);
							printf(DONE);
							return;
							}
						}
					else{
						if(fast_macize(volName))
							printf(mess_failed);
						else{
							ioParam pb;
							pb.ioVRefNum=di.drive_number;
							if(PBMountVol(&pb)==noErr){
								di.v.fi.opened_by_insertion=0;
								chiudi_file();
								printf(DONE);
								return;
								}
							else
								di.os=mac_HFS;
							printf(DONE);
							}
						}
					}
				else{
					diskEject(true);
					return;
					}
				}
			break;
		case hmIBMize:
			i=is_wrprot();
			if(i&1){
				printf(mess_wrprot);
				}
			else if(i&2)
				printf(mess_assente);
			else{
				Boolean bad;
				i=warning_first_write(1,&bad);
				if(i>=0){
					if(i>0){
						ParamText(i==1?mess_non_mac_ing:mess_empty_ing,
						mess_continue_ing,PNS,PNS);
						if( my_modal_dialog(139,NULL,2) == 2) break;
						}
					fillmem(disk_buffer, 0xF6, 512);
					write_sectors(di.sectors_on_floppy-2,disk_buffer,1);
					if(msdos_logical_format(di.sectors_on_floppy) )
						printf(mess_failed);
					else{
						ioParam pb;
						pb.ioVRefNum=di.drive_number;
						if(di.tipo_device==d_driver && PBMountVol(&pb)==noErr){
							di.v.fi.opened_by_insertion=0;
							chiudi_file();
							printf(DONE);
							return;
							}
						else
							di.os=msdos;
						printf(DONE);
						}
					}
				else{
					diskEject(true);
					return;
					}
				}
			break;
		/*case hmExperiments:
			prova_SCSI();
			break;
		*/
		}		/* fine switch */
		command=0;
	}
}

short read_sect_n(n)
sector_t *n;
{
char buffer[32];
short i;
printf("sector number : ");
/* gets(buffer); */
my_itoa((long)*n,buffer);
prompt(buffer,sizeof(buffer)-1);

i=0;
while(buffer[i]==' ') i++;

if(buffer[i]==CR || buffer[i]=='\0') return -2;

#if SECTOR_T_SIZE==4
i=my_atol(&buffer[i],n);
#else
i=my_atoi(&buffer[i],n);
#endif
if(i==0 && (*n<0 || *n >= di.sectors_on_floppy) ){
	printf("Invalid sector number\n");
	return -3;
	}
else if(i==-2)
	printf("Invalid digit\n");
return i;
}

short my_atoi(buffer,n)
register char*buffer;
register short *n;
{
for(;*buffer==' ';buffer++)
	;
if(*buffer>='0'&&*buffer<='9'){
	*n=0;
	while(*buffer>='0'&&*buffer<='9')
			*n= *n * 10 + *buffer++ -'0';
	return 0;
	}
return *buffer? -2 : -1;
}

short my_atol(buffer,n)
register char*buffer;
register long *n;
{
for(;*buffer==' ';buffer++)
	;
if(*buffer>='0'&&*buffer<='9'){
	*n=0;
	while(*buffer>='0'&&*buffer<='9')
			*n= *n * 10 + *buffer++ -'0';
	return 0;
	}
return *buffer? -2 : -1;
}

short untar_dec_number(buffer,doerror)
char*buffer;
short doerror;
{
short i;
if(my_atoi(buffer,&i)){
	if(doerror>=0)
		printf(in_Italia?"Testata non in formato bar !\n":
			"Error: not a bar header !\n");
	if(doerror>0)
		raise_error();
	else
		return -1;
	}
	return i;
}


static unsigned char prev_DOS;	/* used to convert MS-DOS ASCII files, somebody must
	remember that the previous char is a CR so the LF is suppressed rather than
	converted to CR */
void init_deDOSize()	/* to be called for each open file which may use the following service... */
{
prev_DOS=0;
}

long macize_ASCII(buf,len,dos_newlines)
register char *buf;
register long len;
Boolean dos_newlines;
{
/* it can convert also MS-DOS text, by removing any LF preceded by
a CR; but then it must have a way to return a len different than the original
value, and the static variable storing the last char of the previous call
need be cleared at the end of the file
optimization: start with another loop not containing the assignment
to *dest++
*/
extern unsigned char direct_table[];
register unsigned char c,pd=prev_DOS;
long newlen=len;
Boolean no_table = dos_newlines != (direct_table[26]!=26);

if(len==0) return 0;	/* non si sa mai... */
do{
	if((c=*buf)==LF || c==26){	/* MS-DOS text files contain an extra ctrl-Z as
								end of file marker: here it's suppressed without
								checking that it's the last character */
		if(pd!=CR && c==LF){
			*buf++=CR;
			pd=LF;
			}
		else{
			/* now I can't avoid to move characters, hence I must use the most general
			method which shifts the characters while reading them (it would work also
			with dest==buf, hence it may handle the whole conversion, but with useless
			writes which constitute a great performance penalty, expecially in machines
			with a write-through data cache (68030)) */
			register char*dest=buf;
			newlen--;
			pd=*buf++;
			while(--len){
				if(*buf==LF){
					if(pd!=CR)
						*dest++=CR;
					else
						newlen--;
					pd=LF;
					buf++;
					}
				else if(*buf==26){
					newlen--;
					pd=*buf++;
					}
				else{
					/* *dest++ = pd = *buf++;*/
					pd = *buf++;
					if(!no_table)
						*dest++ = direct_table[pd];
					else
						*dest++ =pd;
					}
				}
			prev_DOS=pd;
			return newlen;
			}
		}
	else{
		pd=*buf;
		if(!no_table && (c=direct_table[pd])!=pd)	/* on a machine with a cache performing two
						more instructions to avoid a write to memory is convenient */
			*buf++=c;
		else
			buf++;
		}
	}
while(--len);

prev_DOS=pd;
return newlen;
}

short overwrite_sectors()
{
SFTypeList	myTypes;
short charsRead;
extern char FERRORita[],FERRORing[];
short code=0;

	my_SF_Get(-1,myTypes);
	if(!reply.good) return -1;

	if(apri_file("rb",&inf_refn))
		return -1;
	inf_is_open=true;
	fase=writing_disk;

	while( (charsRead = mac_fread(disk_buffer, 512, inf_refn)) >0 ){
		if(di.tipo_device!=d_file && sect_n>=di.sectors_on_floppy){
			printf("End of disk reached\n");
			code=-1;
			break;
			}
		scrivi_settore(sect_n,disk_buffer);
		check_wr_err();
		if(err_code) raise_error();
		sect_n++;
		check_events(false);
		}
	if(charsRead<0){
		printf(in_Italia?FERRORita:FERRORing,err_code);
		code=-1;
		}

	FSClose(inf_refn);
	inf_is_open=false;
	return code;
}


void print_sector_n(sector_n)
sector_t sector_n;
{
	printf("[sector %4ld] ",(long)sector_n);
}

void stampa_info_bar(n)	/* print informations from a bar volume header */
short n;
{
char buffer[16];
register short i,j;
printf("Volume number %d ", n);
for(i=j=0;i<10;i+=2,j+=3){
	buffer[j]=((barh_type*)disk_buffer)->cdate[i];
	buffer[j+1]=((barh_type*)disk_buffer)->cdate[i+1];
	}
buffer[2]=buffer[5]='/';
buffer[8]=' ';
buffer[11]=':';
buffer[14]='\0';
printf("created %s",buffer);
print_if_string("; archive name is %s",&((barh_type*)disk_buffer)->name[0],100);
vai_a_capo();
}


/*********************************************/

void DriveList()
{
short i;
	disable_autoflush(2);

	if(expert_mode) stampa_driver_info();
	stampa_volume_info();
	enable_autoflush();

	if(gHasSCSIManager) scan_SCSI_bus();	/* the SCSI manager is NOT
		available under A/UX */
}

void fine_lavoro()
{
if(gInBackground)
	check_foreground();
else if(settori_passati>min_to_beep)
	SysBeep(5);
}

void unexpected_disk_insertion(message)
/* called when a disk is inserted and suntar does not expect a disk insertion:
obviously, that disk could be a Mac disk to be passed to the Finder, but it
could be a tar disk, or it could be in the disk drive where suntar believes
there was still the disk it has opened and on which it was working

In suntar 2.0, things are made more complex since the disk may be opened when
it's not in the drive, or may be opened as an SCSI ID while the disk insertion
event obviously reports a device driver
*/
long message;
{
extern Point badmount_point;
extern char *titoli_si_no[];
extern Boolean finestra_sm_aperta;
jmp_buf savebuf;
short inPlace;
Boolean andata_male,in_my_drive;

struct disk_info old_di=di;

mcopy(&savebuf,&main_loop,sizeof(jmp_buf));

andata_male=false;
di.sectors_on_floppy=0;
di.tipo_device=d_driver;

di.drive_number=loword(message);
if(find_driver()) andata_male=true;

if(setjmp(main_loop)<0) {	/* at least identify_format may do a raise_error, and
	raise_error causes a bad crash if no currently running function has performed
	a setjmp, and I don't know what's the current situation */
	andata_male=true;
	}

if(andata_male || testa_stato(&inPlace,0)){
	diskEject(true);
	mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
	di=old_di;
	aggiorna_nome_aperto();
	return;
	}

if(old_di.tipo_device==d_driver)
	in_my_drive= di.drive_number==old_di.drive_number;
else if(old_di.tipo_device==d_scsi)
	in_my_drive= di.m_vrefnum== -33-old_di.drive_number;	/* the device driver for
							ID n is installed as vrefnum ~(32+n), and in 2's complement
							math ~x is -x-1  */
else
	in_my_drive=false;


if( (di.os==mac_HFS||di.os==mac_MFS) && (!in_my_drive || old_di.tipo_device==d_driver&&old_di.v.fi.opened_by_insertion&&!dirty_buffers()) ){
		/* il disco non mi riguarda, lo lascio passare
		-- that disk is of no interest to suntar; do what GetNextEvent would do,
		then what Apple suggests to do when receiving a disk insertion event;
		(I allow a Mac disk to be inserted in my drive, but I will continue
		to use this drive only after checking that my disk was inserted again) */
	ParamBlockRec param;
	extern long last_disk_operation;
	param.volumeParam.ioVRefNum=loword(message);
	if(in_my_drive) last_disk_operation=TickCount()-60*60*10;	/* force the signature check at
		next disk operation */
	if( (hiword(message)=PBMountVol (&param)) != noErr ){
		DIBadMount(badmount_point, message);
		invalid_buffers();
		}
	mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
	di=old_di;
	aggiorna_nome_aperto();
	return;
	}

one_empty_line();
if(fase!=reading_sect_n){
	printf(in_Italia?"Un disco è stato inserito nel drive %d":
		"Detected a disk insertion in drive %d",di.drive_number);
	if(di.m_vrefnum>=-39 && di.m_vrefnum<=-33)
		printf(" (ID=%d)",~(32+di.m_vrefnum));
	vai_a_capo();
	}


if(fase==non_faccio_nulla && old_di.tipo_device==d_driver&&in_my_drive&&old_di.v.fi.opened_by_insertion==1){
	old_di.drive_number=0;		/* there is no reason to ask the disk back if I was
						not working on it, simply close the old one and open
						this one (it might be the same disk, but closing and opening
						it again does not harm) */
	old_di.tipo_device=d_notopen;
	aggiorna_nome_aperto();
	invalid_buffers();
	}

if( old_di.tipo_device!=d_notopen){
	if( in_my_drive ){ /* il disco
			era dentro, probabilmente è stato estratto a tradimento e poi reinserito 
			-- the tar disk was in that drive, probably it was ejected (without using
			suntar's eject) and inserted in back; or maybe it was opened as a device
			when no disk was currently in the drive */
		if((fase==non_faccio_nulla||fase==in_writing) && (old_di.tipo_device==d_scsi||
			old_di.tipo_device==d_driver&&old_di.v.fi.opened_by_insertion!=1)){
				/* that's a typical situation, since you can't insert a non-Mac
				disk into a drive when suntar is not trying to intercept
				disk insertions and avoid the "not a Macintosh disk, initialize
				or eject" dialog.
				Merge the informations from the two structs (informations about the disk
				are kept from the new struct, informations about how it was opened
				from the old one) */
				di.writing_allowed=true;	/* the protection is less strict if
						the device is removable... */
				di.archive_start_sector=old_di.archive_start_sector;
				di.m_vrefnum=old_di.m_vrefnum;
				di.tipo_device=old_di.tipo_device;

				if(di.tipo_device==d_driver){
					di.v.fi.opened_by_insertion=2;
					stampa_descrizione_stato();
					}
				else{
					di.drive_number=old_di.drive_number;
					/*di.v.si=old_di.v.si; useless, it will be reassigned by the following call */
					if(testa_stato(&inPlace,1))	/* I must do a full test since that's
							an SCSI device and the test stored in di was on the device driver */
						; /* niente, mi basta aver stampato qualcosa...*/
					}
				mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
				aggiorna_nome_aperto();
				return;
				}
		else if (di.sectors_on_floppy!=old_di.sectors_on_floppy || not_my_disk() ){
			if(dirty_buffers()){
				diskEject(true);
				ParamText("\pSuntar is writing, you can\'t change disk",PNS,PNS,PNS);
				my_alert();
				}
			else{
				ParamBlockRec param;
				param.volumeParam.ioVRefNum=loword(message);
				if (di.os==unknown_os || (hiword(message)=PBMountVol (&param)) != noErr)
					DIBadMount(badmount_point, message);
				}
			}
		else{
			short item;

			ParamText(in_Italia?"\pQuesto è il disco su cui stavo lavorando ?":
				"\pIs this the disk I was working on ?",PNS,PNS,PNS);
			item=my_modal_dialog(139,titoli_si_no,2);
			if(item==2){
				/*FlushVol (PNS,di.drive_number);*/
				UnmountVol(NULL,di.drive_number);
				diskEject(true);
				}
			}

		if(di.tipo_device==d_scsi &&di.v.si.DeviceType!=TAPE_UNIT)
			adjust_random_access_menu();
		else if(di.tipo_device==d_driver&&di.v.fi.opened_by_insertion!=1)
			adjust_ddriver_menu();
		}
	else{
/* non accetto altri dischi
-- suntar has something open and since it can't open more than one
thing it can't open that disk, do something which looks like the
standard behaviour for non-Mac or non-initialized disks
*/
		if(di.os==msdos){		/* AccessPC or DOS Mounter or PC Exchange
									might be installed... */
			ParamBlockRec param;
			param.volumeParam.ioVRefNum=loword(message);
			if( (hiword(message)=PBMountVol (&param)) != noErr ){
				DIBadMount(badmount_point, message);
				}
			}
		else{
			diskEject(false);
			ParamText(in_Italia?"\pNon si possono aprire due archivi":
				"\pCannot open more than one archive",PNS,PNS,PNS);
			my_alert();
			}
		}
	di=old_di;
	}
else{	/* nothing was open (or what was open could be silently closed),
		open this disk ! */
	invalid_buffers();
	stampa_descrizione_stato();	/* because testa_stato was called with verbose=false */
	di.writing_allowed=true;
	di.v.fi.opened_by_insertion=true;
	di.archive_start_sector=0;
	di.isEjectable=true;
	reinit_buffering(512);

	if(fase!=initializing){
		/*FlushVol (PNS,di.drive_number);*/
		/*i=*/ UnmountVol (NULL,di.drive_number);
		}
	if(di.os==msdos){
		ParamBlockRec param;
		param.volumeParam.ioVRefNum=di.drive_number;
		if( (hiword(message)=PBMountVol (&param)) != noErr ){
			diskEject(true);
			ParamText(in_Italia?
			"\pDisco MS-DOS, per usarlo scegli un comando \"Create…\"":
			"\pMS-DOS disk, to open it select a \"Create…\" command",PNS,PNS,PNS);
			my_alert();
			}
		di.drive_number=0;
		di.tipo_device=d_notopen;
		}
	else if(!(di.tipo_device==d_driver&&di.v.fi.is_not_initialized)){
		store_disk_signature();
		(void)identify_format();
		}
	else
		di.signature_avail=false;

	/* and do NOT restore the old di */
	}
mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
aggiorna_nome_aperto();		/* non si sa mai */
}

/*****************************************/

void log_menu_command(command)
long command;
/* a log file is not very useful if it does not contains the commands selected
by the user */
{
extern short log_refnum;
if(log_refnum){
	unsigned char name[80];
	Handle h=GetResource('MENU',hiword(command));
	if(h!=NULL){
		GetItem(h,loword(command),name);
		start_of_line();
		if(name[name[0]]==(unsigned char)'…') name[0]--;	/* meglio non mettere caratteri non ASCII
										-- it's better to exclude this non-ASCII char */
		printf("Menu item selection: %P\n",name);
		}
	}
}



/************** gestione menù ****************/

void EnableDisableItem(menu, item, flag)
MenuHandle menu;
short item;
Boolean flag;
{
if(flag)
	EnableItem(menu, item);
else
	DisableItem(menu, item);
}

void MaintainApplSpecificMenus()
/* it's an idea borrowed from TEsample: rather than enabling and disabling
items when the situation changes, be sure that some global variables always
tell what the situation is and adjust all the menu items when receiving a 
keyDown event with the Command key or a MouseDown event in the menu bar.
 That's the most reasonable way to do things in a program which has an abort
command and does not quit at the first error.
*/
{ 
extern Boolean finestra_sm_aperta;
extern DialogPtr ListDialog;
extern char causale_list_dialog;

Boolean b;
b= fase==non_faccio_nulla;
if(b){
	EnableDisableItem(myMenus[fileM], fmCopy, di.tipo_device!=d_file); /* a file
			may be copied with usual commands... */
	EnableItem(myMenus[fileM], fmList);
	EnableItem(myMenus[fileM], fmExtract);
	EnableDisableItem(writeMenu, wmCreateTar, di.writing_allowed &&
		di.tipo_device!=d_file || di.tipo_device==d_notopen);	/* a file
		can't be overwritten, you must select New to start a tar file without preserving
		the current content */
	EnableDisableItem(writeMenu, wmCreateBar, di.writing_allowed &&
		di.tipo_device!=d_file||di.tipo_device==d_notopen);
	EnableDisableItem(writeMenu, wmAppend, di.writing_allowed||di.tipo_device==d_notopen);
	EnableDisableItem(myMenus[fileM], fmFormat,
		di.tipo_device==d_driver&&di.writing_allowed ||di.tipo_device==d_notopen);
	EnableItem(myMenus[fileM], fmSelect);
	EnableItem(myMenus[fileM], fmDriveList);
	}
else{
	DisableItem(myMenus[fileM], fmCopy);
	DisableItem(myMenus[fileM], fmList);
	DisableItem(myMenus[fileM], fmExtract);
	DisableItem(writeMenu, wmCreateTar);
	DisableItem(writeMenu, wmCreateBar);
	DisableItem(writeMenu, wmAppend);
	DisableItem(myMenus[fileM], fmFormat);
	EnableDisableItem(myMenus[fileM], fmSelect, ListDialog&&causale_list_dialog<0&&FrontWindow()!=ListDialog);
	DisableItem(myMenus[fileM], fmDriveList);
	}
/* il Think C marker non accetta macro su due righe, e non voglio perdere i suoi servizi ! */
#define abort_mask	( (1<<writing_disk) | (1<<reading_disk) | (1<<reading_sect_n) | (1<<hack_reading) | (1<<hack_listing) | (1<<paused) | (1<<in_writing) | (1<<selected_reading) )

EnableDisableItem(myMenus[fileM], fmPause, pausable || fase==paused /* nel secondo
			caso, l'item è diventato resume */ || finestra_sm_aperta&&((abort_mask>>fase)&1));

b = b && di.tipo_device==d_notopen;

EnableDisableItem(myMenus[fileM], fmGetInfo, b);
EnableDisableItem(myMenus[fileM], fmNew, b);
EnableDisableItem(myMenus[fileM], fmEncode, b);
EnableDisableItem(myMenus[fileM], fmOpenDev, b);

#if 0	/* tentativo, fallito perché l'evento arriva durante il dialogo */
		/* di "Save here", non durante l'estrazione */
if(!b && was_in_mbar ){
	/* suntar does not accept a command while it's working, but a drag&drop
	of multiple files must find the Open menu item active: hence, I must
	know whether this is a true click in the menu bar or a "fake" click
	and in the second case I do activate Open */
	EventRecord myEvent;
	OSEventAvail(kNoEvents, &myEvent);	/* we aren't interested in any events */
										/* just the mouse position */
	if(myEvent.where.v>MBARHEIGHT) b=true;
	if(b) dp("era un drag & drop\n");
	}
#endif
EnableDisableItem(myMenus[fileM], fmOpen, b||fase==drag_and_drop);

{
unsigned char itemname[32];
short i=2;
if(di.tipo_device==d_notopen)
	DisableItem(myMenus[fileM], fmEject);
else{
	if(di.tipo_device==d_file)
		i=1;
	else if(di.tipo_device==d_driver&&di.v.fi.opened_by_insertion!=1||di.tipo_device==d_scsi)
		i=3;
	EnableDisableItem(myMenus[fileM], fmClose, fase==non_faccio_nulla || fase==in_writing);
	}
GetIndString (itemname,129,i);
SetItem (myMenus[fileM],fmClose,itemname);
}

/*EnableDisableItem(myMenus[fileM], fmAbort, fase==writing_disk ||fase==reading_disk ||
	fase==reading_sect_n || fase==hack_reading || fase==hack_listing || fase==paused) || 
	fase==in_writing || fase==selected_reading; */
EnableDisableItem(myMenus[fileM], fmAbort, (abort_mask>>fase)&1 );

b= fase==non_faccio_nulla;
if(expert_mode){
	if(b){
		EnableItem(hackersMenu, hmView);
		EnableDisableItem(hackersMenu, hmClear, di.writing_allowed||di.tipo_device==d_notopen);
		EnableDisableItem(hackersMenu, hmMac_ize,di.tipo_device==d_notopen||di.tipo_device==d_driver&&di.writing_allowed);
		EnableDisableItem(hackersMenu, hmIBMize,di.tipo_device==d_notopen||di.tipo_device==d_driver&&di.writing_allowed);
		EnableDisableItem(hackersMenu, hmOverwrite, di.writing_allowed||di.tipo_device==d_notopen);
		EnableItem(hackersMenu, hmSave);
		EnableItem(hackersMenu, hmFind);
		EnableItem(hackersMenu, hmUntar);
		EnableItem(hackersMenu, hmUnbar);
		EnableItem(hackersMenu, hmReadStarting);
		EnableItem(hackersMenu, hmList);
		/*EnableItem(hackersMenu, hmExperiments);*/
		EnableDisableItem(hackersMenu, hmAgain, again_command!=0);
		}
	else{
		DisableItem(hackersMenu, hmView);
		DisableItem(hackersMenu, hmClear);
		DisableItem(hackersMenu, hmMac_ize);
		DisableItem(hackersMenu, hmIBMize);
		DisableItem(hackersMenu, hmOverwrite);
		DisableItem(hackersMenu, hmSave);
		DisableItem(hackersMenu, hmFind);
		DisableItem(hackersMenu, hmUntar);
		DisableItem(hackersMenu, hmUnbar);
		DisableItem(hackersMenu, hmReadStarting);
		DisableItem(hackersMenu, hmList);
		/*DisableItem(hackersMenu, hmExperiments);*/
		DisableItem(hackersMenu, hmAgain);
		}
	EnableItem(hackersMenu, hmCreateLog);
	}
if(di.tipo_device==d_scsi)
	sistema_items_device_menu_scsi();
else if(di.tipo_device==d_driver && di.v.fi.opened_by_insertion!=1)
	sistema_items_device_menu_driver();
/* No: è semimodal
if(ListDialog && causale_list_dialog>0)
	EnableDisableItem(writeMenu,causale_list_dialog,ListDialog!=FrontWindow());
*/
}

static void enable_write_items()
{
short i;
for(i=wmDataFork;i<=wmEndWrite;i++)
	EnableItem(writeMenu,i);
}

void disable_write_items()
{
short i;
for(i=wmDataFork;i<=wmEndWrite;i++)
	DisableItem(writeMenu,i);
}


void set_skip_all(n)
short n;
{
/* il confirm saves ha tre stati: 
	non selezionato (confirm_saves=0,saveskip_all=0)
	selezionato 	(confirm_saves=1,saveskip_all=0)
	posto in corsivo da una skip all o save all (confirm_saves=0,saveskip_all=1)
questa routine gestisce i passaggi tra i tre stati:

n= 0 -> se è in corsivo torna sel, se no resta immutato;
n= 1 -> da sel commuta a posto in corsivo (e da non sel resta immutato !)
n=-1 -> da chiamare quando seleziono l'entry del menù: se è in corsivo torna selezionato,
		se no inverte lo stato attuale

-- the confirm saves menu item has three states:
	non-selected (confirm_saves=0,saveskip_all=0)
	selected 	(confirm_saves=1,saveskip_all=0)
	italicized by a skip all or save all (confirm_saves=0,saveskip_all=1)
this routine handles all its status changes:
n=0 => I'm in the main loop, if it's italicized then make it selected
  1 => the save all or skip all was clicked (caution, the confirm saves could 
		have been disabled after the window appeared, and the button may still
		be clicked)
 -1 => the confirm saves menu item was selected: if italicized return normal,
		otherwise invert its current state

*/
static Boolean saveskip_all=false;
short item=pmConfirm;

if(!expert_mode) item--;

if(!n || (n<0&&saveskip_all) ){
	if(saveskip_all){
		saveskip_all=false;
		SetItemStyle(myMenus[prefM],item,0);
		confirm_saves=true;
		}
	}
else if (n>0){
	if(confirm_saves||saveskip_all){	/* può capitare che non sia così se 
			deseleziono durante il dialogo e poi clicco su un bottone...*/
		saveskip_all=true;
		SetItemStyle(myMenus[prefM],item, 2);	/* corsivo */
		confirm_saves=false;
		}
	}
else{	/* n=-1, saveskip_all=0 */
	CheckItem(myMenus[prefM],item,confirm_saves^=1);
	}
}


Boolean warning_writing_dangers(void);
Boolean warning_writing_dangers()
{
ParamText(in_Italia?"\pSei veramente sicuro che scrivere su questo dispositivo non sia catastrofico ?":
"\pWriting to a device may destroy all the data on it. Are you really sure that\'s what you want ?",PNS,PNS,PNS);
return my_modal_dialog(139,titoli_si_no,2)==1;
}

void sistema_items_device_menu_driver()
{

if(fase!=non_faccio_nulla && fase !=in_writing){
	DisableItem(deviceMenu,dmUnmount);
	DisableItem(deviceMenu,dmMount);
	DisableItem(deviceMenu,dmEject);
	}
else{
	EnableDisableItem(deviceMenu,dmUnmount,di.mounting_state==mounted);
	EnableDisableItem(deviceMenu,dmMount,di.mounting_state==not_mounted);
	EnableDisableItem(deviceMenu,dmEject,di.isEjectable && di.mounting_state!=mounted_as_startup );
	}
EnableDisableItem(deviceMenu,dmAllow,!di.writing_allowed && di.mounting_state!=mounted_as_startup);
}


void adjust_ddriver_menu()
{
char buffer[100];
short refnum;
long space;

if( GetVInfo (di.drive_number, buffer, &refnum,&space) == nsvErr ) /* no such volume: not mounted */
	di.mounting_state=not_mounted;
else{
	di.mounting_state=mounted;
	if(check_startup(di.drive_number)) di.mounting_state=mounted_as_startup;
	}
}

void new_ddriver_menu()
{

InsertMenu(deviceMenu = GetMenu(deviceID+2), 0);
adjust_ddriver_menu();
DrawMenuBar();
}


OSErr unmount_volume(short);
OSErr unmount_volume(vrefn)
short vrefn;
{
short err;
if((err=UnmountVol (NULL,vrefn))==noErr||err==nsvErr)
	return 0;
else{	/* per lo meno provo a metterlo offline */
	ParamBlockRec parblk;
	parblk.volumeParam.ioNamePtr=NULL;
	parblk.volumeParam.ioVRefNum=vrefn;

	if((err=PBOffLine(&parblk)))
		return fBsyErr;	/* just to return a standard err code... */
	else
		return 1;
	}
}


Boolean check_startup(refnum)
short refnum;
{
	WDPBRec param;
	char buffer[100];

	param.ioNamePtr = buffer;
	buffer[0]=0;
	param.ioVRefNum = gMac.sysVRefNum;
	param.ioWDIndex = 0;
	param.ioWDProcID = 0;
	if(PBGetWDInfoSync(&param)==noErr){
		param.ioVRefNum=refnum;
		refnum=param.ioWDVRefNum;
		buffer[0]=0;
		param.ioNamePtr = buffer;
		param.ioWDIndex = 0;
		param.ioWDProcID = 0;
		if(PBGetWDInfoSync(&param)==noErr && param.ioWDVRefNum==refnum){		/* get the vrefnum */
			/*printf("ris=%d %ld %d %ld %P\n",param.ioVRefNum,param.ioWDProcID,param.ioWDVRefNum,
				param.ioWDDirID,param.ioNamePtr);*/

			ParamText(in_Italia?
				"\pQuesto è il volume di avvio, verrà aperto in sola lettura":
				"\pThis is the startup disk, it will be opened in read-only mode",PNS,PNS,PNS);
			my_alert();
			return true;
			}
		}
	return false;
}


void handle_ddriver_menu(short);
static void handle_ddriver_menu(item)
short item;
{
switch(item){
case dmAllow:
	if(warning_writing_dangers()){
		di.writing_allowed=true;
		}
	break;
case dmUnmount:
	if(unmount_volume(di.drive_number)<0)
		printf("Can\'t unmount\n");
	else
		di.mounting_state=not_mounted;
	break;
case dmMount:
	{ParamBlockRec param;
	param.volumeParam.ioVRefNum=di.drive_number;
	if((di.v.fi.disk_code=PBMountVol (&param)) == noErr || di.v.fi.disk_code==volOnLinErr)
		di.mounting_state=mounted;
	else
		printf("Mount failed, error code %d\n",di.v.fi.disk_code);
	}
	break;
case dmEject:
	{struct disk_info old_di=di;
	if(!unmount_volume(di.drive_number))
		di.mounting_state=not_mounted;
	if(volume_in_place(di.m_vrefnum))
		diskEject(false);
	di=old_di;	/* forse non più necessario da quando ho introdotto il parametro
				a diskEject !!! */
	}
	break;
}
}
	


void add_expert_menu(void);
void add_expert_menu()
{
void add_expert_items(void);

	add_expert_items();
	InsertMenu(hackersMenu = GetMenu(hackID),0);
}


void del_expert_items(void);
void del_expert_items()
{
	DelMenuItem (myMenus[prefM], pmNoConvers);
	DelMenuItem (myMenus[prefM], pmIgnore);
	DelMenuItem (myMenus[prefM], pmRareOpt);
}

void add_expert_items(void);
void add_expert_items()
{
unsigned char itemname[48];
	GetIndString(itemname,130,1);
	AppendMenu(myMenus[prefM],itemname);
	GetIndString(itemname,130,2);
	AppendMenu(myMenus[prefM],itemname);
	GetIndString(itemname,130,3);
	InsMenuItem(myMenus[prefM],itemname,pmExpert);
	/* questo serve solo in loadpref.c che in un caso cancella e poi reinserisce: */
	CheckItem(myMenus[prefM],pmIgnore,ignore_errors);
	CheckItem(myMenus[prefM],pmNoConvers,non_convertire);
}


static void handle_menus(choice)	/* handle application specific menus */
long choice;
{
if(hiword(choice)==appleID)
	about_box();
else if(hiword(choice)==prefID)
	switch((!expert_mode && loword(choice)>pmExpert) ? loword(choice)+1: loword(choice)) {
	case pmOptions:
		flush_all();
		options_box();
		break;
	case pmEnglish:{
		short markChar;
		in_Italia^=1;
		GetItemMark (myMenus[prefM],loword(choice),&markChar);
		CheckItem(myMenus[prefM],loword(choice),!markChar);
		}
		break;
	case pmConfirm:
		set_skip_all(-1);
		break;
#ifdef V_122
	case pmSmallText:
		CheckItem(myMenus[prefM],pmSmallText, smallFilesAreASCII^=1);
		preferences_changed=true;
		break;
#endif
	case pmExpert:
		CheckItem(myMenus[prefM],pmExpert,expert_mode ^=1);
		if(expert_mode){
			add_expert_menu();
			}
		else{
			del_expert_items();
			DeleteMenu (hackID);
			ignore_errors=non_convertire=false;
			}
		DrawMenuBar();
		preferences_changed=true;
		break;
	case pmIgnore:
		CheckItem(myMenus[prefM],loword(choice),ignore_errors^=1);
		break;
	case pmNoConvers:
		CheckItem(myMenus[prefM],loword(choice),non_convertire^=1);
		break;
	case pmRareOpt:
		rare_options(0);
		break;
	}
else if(hiword(choice)==hqxID){
	preferences_changed=true;
	switch(loword(choice)){
	case bmDisable:
		CheckItem(hqxM,bmDisable,(disable_binhex^=1)&1);
		break;
	case bmShowExtr:
		CheckItem(hqxM,bmShowExtr,((disable_binhex^=2)&2)!=0);
		break;
	case bmSaveInfo:
		CheckItem(hqxM,bmSaveInfo,((disable_binhex^=8)&8)!=0);
		break;
	case bmShowList:
		CheckItem(hqxM,bmShowList,((disable_binhex^=4)&4)!=0);
		break;
	}
	}
else if(hiword(choice)>=deviceID&&hiword(choice)<=deviceID+2){
	log_menu_command(choice);
	if(di.tipo_device==d_driver)
		handle_ddriver_menu(loword(choice));
	else
		handle_device_menu(loword(choice));
	}
else{	/* fileID o = oppure hackID, tanto non sono mai abilitati insieme, 
		e comunque non butto via l'informazione
		-- I don't serve the command, rather I remember that it was selected.
		That's a matter of program hierarchy: MainEvent is a slave, it's not the
		master, hence important operations are not executed under it, and
		this routine is called from it.
		*/

	if(ListDialog&&ListDialog != FrontWindow() && choice==menuItemMess(fileID,fmSelect)){
		SelectWindow(ListDialog);
		}
	else
		last_selection= choice;

	}
}

static short dialogo_abort()
{
	ParamText(in_Italia?"\pVuoi interrompere il comando in corso ?":
		"\pDo you want to abort the current command and quit ?",PNS,PNS,PNS);
	/*beep_in_foreground();*/
	SysBeep(10);
	return my_modal_dialog(139,titoli_si_no,2);
}


static short my_quit_handler()
{
void closeLog(void);

if(((abort_mask>>fase)&1) && fase!=in_writing){	/* it's the condition used for the abort menu item */
	if(dialogo_abort()==2) return 0;
	}

check_and_eject();	/* ejects the disk and, if writing, flushes and truncates... */
chiudi_file();

close_or_del_out_file();	/* ExitToShell closes all files, but obviously does not
				flush my internal buffer nor delete incomplete files if that
				option is checked */
closeLog();

save_opts_quitting();

return 1;
}

/******************/

void print_chars(p,n)
char* p;
short n;
{
if(!n) return;
while(--n)
	put_char(*p++);
printf("%c",*p);	/* to flush the buffer and update the scrollbars */
}

void flush_all()
{
if((di.tipo_device!=d_scsi||di.v.si.DeviceType!=TAPE_UNIT) && dirty_buffers())
	flush_buffers(0);

flush_console();
}

pascal Size memory_overflow(s)
Size s;
{
extern char*buffers_base,*hd_buffer;
long theA5;

if(s>2048){
	return 0;	/* per le grosse richieste c'è sempre un controllo del valore
				ritornato !=NULL, sono le piccole che... */
	}
theA5=SetCurrentA5();

if(buffers_base)DisposPtr(buffers_base);
if(hd_buffer)DisposPtr(hd_buffer);

/*my_itoa(s,disk_buffer);
my_c2pstr(disk_buffer);*/
ParamText(in_Italia?"\pMemoria insufficiente, esecuzione terminata":
	"\pInsufficient memory, execution terminated",/*disk_buffer*/ PNS,PNS,PNS);
my_alert();

SetA5(theA5);	/* non necessario se faccio ExitToShell, ma in futuro...*/
ExitToShell();
}


