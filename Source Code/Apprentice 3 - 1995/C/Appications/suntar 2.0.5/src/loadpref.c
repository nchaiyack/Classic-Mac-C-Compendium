/*******************************************************************************\

preferences module

part of suntar, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/*

tanto vale fare unloadseg dopo tutte le chiamate da fuori di qui, cioè:
*load_STR_prefs_except_bufsizes+must_be_reloaded (dialog.c)
*prefs_da_risorsa (chiamata da init windows.c, stesso segmento)
*save_options (chiamata solo da fuori, ho messo l'UnloadSeg nella routine stessa)
*save_opts_quitting (suntar.c, ma inutile unloadSeg perché quitto subito)
ma ovviamente metterci anche le routines di init di MainEvent.c e windows.c
*InitConsole (suntar.c)
*load_char_tables (suntar.c)


	al reset:
	   se non c'è OPTs 128 allora eredita risorse, a meno che nel pref Folder
	   	ci sia un file di pref di suntar, e allora non ereditare nulla e apri quello
	   	senza nemmeno chiedere
	   else se la OPTs 128 dice che devo tenere pref nel system folder, cerca di aprire
	   	file di pref esistente: se non c'è, non ha importanza, usa comunque ciò che
	   	hai entro di te
	   poi carica tutto dal res file corrente
	al quit:
		se uno dei menu è cambiato o comunque mi sono segnato di doverli salvare
			se la OPTs 128 attualmente vista dice che devi salvare nel pref folder
			e il file di pref non è aperto, crea file copiandoci tutto quanto serve,
			esclusi i due MENU

			changed resource, write resource sui due MENU, a meno che nel file non
			compaiano e allora creaceli
	al click su temporary, ignora qualunque modifica a quel checkbox (è una opzione
			intrinsecamente permanente)
	al click su "permanent"
		se l'opzione "salva in pref folder" era ed è diventata:
		no->no:		come ora, changed resource+write resource
		sì->sì:		se nel pref folder non c'è il file giusto crealo (poteva non
					esistere perché finché esso sarebbe identico alle risorse
					poste nel file non lo creo). ATTENZIONE ai menu che vanno alterati
					prima di salvarli, meglio non salvarli e segnarsi di doverlo fare
					al quit. O alterare la copia....
					else changed resource+write resource
		no->sì:		salva tutte le OPTs perché se no non so che devo usare il pref
					folder, a quel punto però è inutile creare il file che verrà
					creato la prossima volta che serve. Ma se esiste, aggiornalo
					perché non contiene il settaggio attuale, e aprilo; o addirittura
					cancellalo
		sì->no		chiudi il file di pref e poi salva tutto entro te stesso.
					Ovviamente questo può comportare creare la OPTs 128 che non
					esiste ancora, ma le altre ci sono tutte (anche se forse
					è meglio non fidarsi). Eventualmente cancellalo.
		NB: se l'applicazione suntar è read only il checkbox può essere grigio
			per non fare pasticci
*/


pascal OSErr FindFolder(short vRefNum,OSType folderType,Boolean createFolder,
    short *foundVRefNum,long *foundDirID)
    = {0x7000,0xA823};

#include "windows.h"
#include "suntar.h"

#ifdef THINK_C_5
#include <packages.h>
#else
#include <IntlPkg.h>
#endif

#define n_elems_of(x)	(sizeof(x)/sizeof((x)[0]))

void replace_resources(OSType,short,short,Boolean,short*);
void eredita_risorse(SFReply *);
void replace_one_resource(Handle,short,short,OSType,Str255);
void replace_one_menu(short,short,short,Boolean);
short open_pref_file(Boolean);
short crea_WD(short,long);
void register_WD(short);
void cancella_file_aperto(short);
void reassign_type_creator(short,unsigned char*,OSType,OSType);
void del_expert_items(void);
void add_expert_items(void);
void load_char_tables(void);
Boolean this_file_is_writable(short);
void disable_write_items(void);
void add_expert_menu(void);
void compute_delta_time(void);

extern Boolean gHasFindFolder;
extern SysEnvRec	gMac;

short size_ext_table;
char opt_variable_blocks;
byte opt_force_mode_select;
Handle h_type,h_creat,h_extens,h_comments,h_ascii;
Handle binary_types_h;
short default_type,default_device_num,def_cambiato;
short pref_frefn,appl_frefn;
Boolean pref_aperta;

unsigned char direct_table[256],inverse_table[256],isascii_table[256];


static unsigned char no_pref_file[]="\pError creating preferences file";


void transfer_pref_resources(short,short,Boolean);
static void transfer_pref_resources(from,to,del_orig)
/* lascia "to" come current res file */
short from,to;
Boolean del_orig;
{
	static short tab[]={129,130,131,132,133,134,-1};
	replace_resources('STR ',to,from,del_orig,NULL);
	replace_resources('OPTs',to,from,del_orig,tab);
	replace_one_menu(145,to,from,del_orig);
	replace_one_menu(131,to,from,del_orig);
}

static void eredita_risorse(reply)
SFReply *reply;
{
short oldres,refnum,oldvRefNum;
Str63 name;

oldres=CurResFile();
SetResLoad(false);		/* I should save the old status to restore
						it, but it should never be false... */
if(GetVol (&name,&oldvRefNum)!=noErr) oldvRefNum=0;
 
SetVol(NULL,reply->vRefNum);
refnum= OpenResFile(reply->fName);

if(refnum!=-1 && refnum!=oldres){
	transfer_pref_resources(refnum,oldres,true);
	CloseResFile(refnum);
	}
if(oldvRefNum) SetVol (&name,oldvRefNum);
UseResFile(oldres);
SetResLoad(true);
}

static void replace_one_menu(resID,to,from,del_orig)
short resID;
short to,from;
Boolean del_orig;
{
	Handle h;
	UseResFile(from);
	h=Get1Resource('MENU',resID);
	if(h!=NULL){
		LoadResource(h);
		UseResFile(to);
		replace_one_resource(h,resPreload,resID,'MENU',PNS);
		if(del_orig)
			ReleaseResource(h);
		}
	else
		UseResFile(to);	/* per avere una situazione finale uguale in tutti i casi */
}


static void replace_resources(tipo,to,from,del_orig,nonpurge)
/* trasferisce tutte le risorse di tipo 'tipo' (eccetto le STR 128 e 153) dal file
"from" al file "to". Se non esistono le crea, rendendole purgeable a meno che
siano presenti in nonpurge */
OSType tipo;
short to,from;
short nonpurge[];
Boolean del_orig;
{
Handle h;
short i,n,resID;
OSType resType;
Str255 name;

UseResFile(from);
n=Count1Resources(tipo);
for(i=1;i<=n;i++){
	UseResFile(from);
	h=Get1IndResource (tipo,i);
	LoadResource(h);
	GetResInfo (h, &resID, &resType, name);
	/*printf("replacing %.4s %d\n",&resType,resID);*/
	if(resType!='STR ' || resID != 128 && resID != 153){	/* perché in 1.2, 1.2.1 e 1.2.2
			c'era un STR 128 'GIF creator' non più usato. E nella 2.0.2 e 2.0.3
			c'era un "SCSI names in open device" non più usato. Per la verità nel 1.2
			c'era anche un STR 135 "use sys 7 put file" ma 1.2.1 già non l'aveva
			e visto che la 1.2 era tanto buggata spero che nessuno la usi più.
			E ovviamente, ad essere pignolo bisognerebbe tradurre il GIF creator
			nel nuovo formato, ma non esageriamo... */
		short attrs;
		if(nonpurge==NULL)
			attrs= resPreload|resPurgeable;
		else{
			int k=0;
			while(nonpurge[k]!=-1 && nonpurge[k]!=resID) k++;
			attrs= nonpurge[k]==-1 ?  resPreload|resPurgeable : resPreload;
			}
		UseResFile(to);
		replace_one_resource(h,attrs,resID,tipo,name);
		if(del_orig) ReleaseResource(h);
		}
	}
}


static void replace_one_resource(h,attrs,resID,tipo,name)
Handle h;
short attrs,resID;
OSType tipo;
Str255 name;
/* aggiorna la risorsa specificata da tipo e resID a contenere lo
stesso che l'handle h. Se non esiste, la crea assegnandovi name e attrib */
/* NB: in molti casi sarebbe molto più semplice usare DetachResource, ma
in alcuni casi essa può creare problemi, che così non ci sono
*/
{
Handle h1;
Size l;
SignedByte oldflags;

	h1=Get1Resource(tipo,resID);
	l=GetHandleSize(h);
	if(h1==NULL){
		h1=NewHandle(l);
		if(h1==NULL) return;
		AddResource (h1, tipo,resID,name);
		if(ResError()) return;
		SetResAttrs(h1,attrs);
		/*printf("creo nuova %.4s ID %d\n",&tipo, resID);*/
		}
	else{
		/*printf("aggiorno vecchia %.4s ID %d\n",&tipo, resID);*/
		LoadResource(h1);
		SetHandleSize(h1,l);
		if(MemError()) return;
		}
	oldflags=HGetState(h);
	HLock(h); HLock(h1);
	mcopy(*h1,*h,l);
	if(tipo=='MENU') (**(MenuHandle)h1).menuProc=NULL;	/* without it, horrible things happen when the MENU
							is then loaded... */

	HSetState(h,oldflags);
	HUnlock(h1);
	ChangedResource(h1);
	WriteResource(h1);
}


/**********************/

void load_OSType(short,OSType *);
static void load_OSType(id,dest)
short id;
OSType *dest;
{
Handle h;
if((h=GetResource ('STR ',id))==NULL)
	*dest='????';
else{
	HLock(h);
	mcopy((char*)dest,(char*)*h+1,4);
	ReleaseResource(h);
	/* *dest = (((long)*(unsigned char*)(*h+1))<<24) +
				  (((long)*(unsigned char*)(*h+2))<<16) +
				  (((long)*(unsigned char*)(*h+3))<<8) +
				  ((long)*(unsigned char*)(*h+4));	*/
	}
}

long load_number(short,short);
static long load_number(id,deflt)
short id;
short deflt;
{
Handle h;
if((h=GetResource ('STR ',id))==NULL)
	return deflt;
else{
	long temp;
	HLock(h);
	temp=pstrtoi(*h);
	ReleaseResource(h);
	return temp;
	}
}

Boolean load_boolean(short);
static Boolean load_boolean(id)
short id;
{
Handle h=GetResource ('STR ',id);
Boolean b;
b= h && **h && ((*h)[1] | 0x20)!='n';
ReleaseResource(h);
return b;
}

char load_firstchar(short);
static char load_firstchar(id)
short id;
{
Handle h=GetResource ('STR ',id);
char c;

if(h && **h ){
	c=1;
	while((*h)[c]==' ' && c<**h)
		c++;
	c= (*h)[c];
	}
else
	c='\0';
ReleaseResource(h);
return c;
}



char *load_string_pref(short);
static char *load_string_pref(id)
short id;
{
char *p;
Size s;
Handle h=GetResource ('STR ',id);
if(h){
	p=NewPtr((s=GetHandleSize(h)));
	mcopy(p,*h,(short)s);
	my_p2cstr(p);
	ReleaseResource(h);
	}
else{
	p=NewPtr(1);	/* DEVE essere un NewPtr perché ci si deve poter fare
					un DisposPtr */
	p[0]='\0';
	}
return p;
}


Handle load_ostype_list(short);
static Handle load_ostype_list(id)
short id;
{
Size s;
register Handle h;
h=Get1Resource('OPTs',id);
if(h==NULL){
	/* la risorsa non esiste, creala */
	h=NewHandle((s=4*size_ext_table));
	fillmem(*h,'?',s);
	AddResource (h,'OPTs',id,PNS);
	SetResAttrs(h,resPreload);
	/*ChangedResource(h);*/
	WriteResource(h);
	}
else if((s=GetHandleSize(h))<4*size_ext_table){
	/* la risorsa è troppo corta, allungala */
	SetHandleSize(h,4*size_ext_table);
	fillmem(*h+s,'?',4*size_ext_table-s);
	ChangedResource(h);
	WriteResource(h);
	}
return h;
}

Handle load_boolean_list(short);
static Handle load_boolean_list(id)
short id;
{
Size s;
register Handle h;
h=Get1Resource('OPTs',id);
if(h==NULL){
	/* la risorsa non esiste, creala */
	h=NewHandle((s=size_ext_table));
	fillmem(*h,0,s);
	AddResource (h,'OPTs',id,PNS);
	SetResAttrs(h,resPreload);
	/*ChangedResource(h);*/
	WriteResource(h);
	}
else if((s=GetHandleSize(h))<size_ext_table){
	/* la risorsa è troppo corta, allungala */
	SetHandleSize(h,size_ext_table);
	fillmem(*h+s,'?',size_ext_table-s);
	ChangedResource(h);
	WriteResource(h);
	}
return h;
}


Handle load_string_list(short);
static Handle load_string_list(id)
short id;
{
register Handle h;
h=Get1Resource('OPTs',id);
if(h==NULL){
	h=NewHandle(size_ext_table+2);
	fillmem(*h,0,size_ext_table+2);
	*((*h)+1)=size_ext_table;
	AddResource (h,'OPTs',id,PNS);
	SetResAttrs(h,resPreload);
	/*ChangedResource(h);*/
	WriteResource(h);
	}
else if(*(short*)*h < size_ext_table){
	register char *p=*h;
	register short i;
	i=*(short*)p;
	p+=2;
	while(i--)
		p+=*p+1;
	i=p-*h;
	SetHandleSize(h,i+size_ext_table-*(short*)*h);
	fillmem(*h+i,0,size_ext_table-*(short*)*h);
	*(short*)*h = size_ext_table;
	ChangedResource(h);
	WriteResource(h);
	}
return h;
}

long load_bitlist(short);
static long load_bitlist(id)
short id;
{
Handle h;
if((h=GetResource ('STR ',id))==NULL)
	return 0L;
else{
	long temp=0;
	char *p;
	HLock(h);
	p=*h;
	my_p2cstr(p);
	while(*p!='\0'){
		while(*p==' '||*p==',')
			p++;
		if(*p>='0' && *p<='9'){
			short n=0;
			while(*p>='0' && *p<='9'){
				n=n*10+ *p-'0';
				p++;
				}
			temp |= 1L<<n;
			}
		else
			break;	/* parsing error */
		}

	ReleaseResource(h);
	return temp;
	}
}

void load_STR_prefs_except_bufsizes(void);
void load_STR_prefs_except_bufsizes()
{
extern long my_uid,my_gid;
extern long supported_ops,unsupported_ops;
extern Boolean SB3_compatibility;
extern char *uname,*gname;
extern long rewindTimeout,writeTimeout;
extern OSType tar_creator,bar_creator,bar_type,macbin_creator;
extern unsigned short max_hqx_header;
extern short tar_block_size;
extern short back_prio;


load_OSType(129,&tar_creator);
load_OSType(136,&bar_creator);
load_OSType(137,&bar_type);
load_OSType(144,&macbin_creator);

max_hqx_header=load_number(134,10240);

my_uid=load_number(130,0);
my_gid=load_number(131,0);
tar_block_size=load_number(145,20);
SB3_compatibility=load_boolean(141);

if(uname!=NULL) DisposPtr(uname);
if(gname!=NULL) DisposPtr(gname);
uname=load_string_pref(132);
gname=load_string_pref(133);

back_prio = load_number(140,4);
supported_ops=load_bitlist(146);
unsupported_ops=load_bitlist(147);
opt_variable_blocks=load_firstchar(148) | 0x20;
opt_force_mode_select=load_boolean(149);

rewindTimeout=60L*load_number(151,120);
writeTimeout=60L*load_number(152,2);
/*names_in_opendevice=load_boolean(153);*/

/* il 150 non lo carico qui ma essendo una GetResource, per di più
non lockata, viene automaticamente aggiornata dalla ChangedResource
del dialogo rarely_used
e il 135 per definizione ha senso solo al lancio
*/
}

Boolean must_be_reloaded(short);
Boolean must_be_reloaded(short id)
/* ritorna true se la STR id non diventa attiva chiamando load_STR_prefs_except_bufsizes */
{
if(id>=129 && id<=137 ||
   id>=140 && id<=141 ||
   id>=144 && id<=153 )
   return false;
return true;
}



void prefs_da_risorsa(void);
void prefs_da_risorsa()
/* read all the preference settings which depend on a resource, including
the INTL 0 resorce which is in the System file and contains the code
for the nation, and compute the centered locations for those dialogs which
are difficultly moved after their creation
*/
{
Handle h;
short i;
static Rect bmr={0,0,96,300};
static short SF_IDs[]={199,200,-3999,-4000,1043,400,399};
static unsigned char pref_is_damaged[]="\pThe preferences file is damaged";
extern Boolean gHasCustomPutFile;
extern char *printf_buf;
extern char *uname,*gname;
extern short floppy_buffer_size;
extern long hd_buffer_size;
extern short default_type,default_device_num;
extern Point badmount_point;
#define verItSwiss 36


/* assign the defaults: they may be useful */
fillmem(&options,0,sizeof(options));
text_creator='EDIT';
gif_creator='QGif';
resolve_aliases=true;
useSys7_SF=true;		/* soppresso nella 2.0.4, ma se qualcuno volesse ereditare
						da parte di una versione più vecchia... */
non_text_ASCII=1;
smallFilesAreASCII=1;
/*delay_back=300;		/* soppresso nella 2.01 (e inutile dalla 2.0) */
min_to_beep=500;
del_incompl=true;

uname=gname=NULL;	/* dovrebbero già esserlo per via che sono var globali,
					ma non si sa mai */

appl_frefn=CurResFile();

h=GetResource('OPTs',128);
if(h==NULL){
	/* suntar non configurato: ma c'è ancora la possibilità che esista un file di prefs */
	pref_aperta=open_pref_file(false)==noErr;
	if(pref_aperta && ((h=GetResource('OPTs',128))==NULL || ((options_type*)*h)->separate_prefs!=true)){
		/* non creo mai un file di pref senza una OPTs 128, né con una OPTs 128
		che dichiara non doverci essere un file di prefs, ma non si sa mai,
		il file potrebbe essersi danneggiato */
		ParamText(pref_is_damaged,PNS,PNS,PNS);
		my_alert();
		ExitToShell();
		}
	if(!pref_aperta){
		/* dialogo: eredita o setta defaults */
		/* niente ParamText, per una volta ho messo la stringa direttamente nel dialogo */

		i=my_modal_dialog(154,NULL,0);

		if(i==3)	/* quit */
			ExitToShell();
		else{	/* 1 (use default) oppure 2 (inherit) */
			if(!this_file_is_writable(appl_frefn)){	/* se sono read only, non posso
				non creare un file di prefs */
				options.separate_prefs=true;
				open_pref_file(true);	/* DEVO crearlo perché una opzione
							(separate_prefs) ha un valore diverso da quello di
							default, e se suntar è già read-only non c'è altro
							modo di impedire di ripresentare il dialogo modale
							ad ogni lancio */
				}
			 if(i==2){	/* inherit */
				SFTypeList	myTypes;
				FileParam fpb;
				myTypes[0]='APPL';
				myTypes[1]='pref';
				my_SF_Get(2,myTypes);
		
				if(reply.good){
					fpb.ioFVersNum = 0;
					fpb.ioFDirIndex = 0;
					fpb.ioVRefNum=reply.vRefNum;
					fpb.ioNamePtr=reply.fName;

					if (!PBGetFInfoSync(&fpb) && fpb.ioFlFndrInfo.fdCreator=='S691'){
						eredita_risorse(&reply);
						h=GetResource('OPTs',128);
						if(!this_file_is_writable(appl_frefn)){
							/* ho copiato nel file esterno, che però deve dire
							che devo usare un file esterno, se no... */
							if(h!=NULL && ((options_type*)*h)->separate_prefs!=1){
								((options_type*)*h)->separate_prefs=1;
								ChangedResource(h);
								WriteResource(h);
								}
							}
						else{
							/* se ho ereditato entro di me e ora dice di
							usare un file esterno, oppure non c'è una OPTs 128,
							allora devo decidere di usare file esterno */
							if(h==NULL || ((options_type*)*h)->separate_prefs==1){
								options.separate_prefs=true;
								open_pref_file(true);
								}
							}
						}
					}
				}
			else{	/* per esclusione 1, use defaults */
				ParamText("\pSuggestion: assign at least the options \"tar version\" and \"Text files creator\"",PNS,PNS,PNS);
				my_alert();
				}
			}
		}
	}
else{
	/* suntar già configurato, ma potrebbe essere configurato per prefs esterne */
	if(((options_type*)*h)->separate_prefs){
		pref_aperta=open_pref_file(false)==noErr; /* se il file di prefs non esiste,
				inutile crearlo finché conterrebbe le stesse cose della copia principale */
		if(pref_aperta && ((h=GetResource('OPTs',128))==NULL || ((options_type*)*h)->separate_prefs!=true)){
			ParamText(pref_is_damaged,PNS,PNS,PNS);
			my_alert();
			CloseResFile(pref_frefn);
			pref_aperta=false;
			}
		}
	}


hqxM=GetMenu(hqxID);	/* must be loaded only after if was inherited... */
InsertMenu(hqxM, -1);
GetItemMark (hqxM,bmDisable,&i);
disable_binhex = i==checkMark;
GetItemMark (hqxM,bmShowExtr,&i);
if(i==checkMark) disable_binhex |=2;
GetItemMark (hqxM,bmSaveInfo,&i);
if(i==checkMark) disable_binhex |=8;
GetItemMark (hqxM,bmShowList,&i);
if(i==checkMark) disable_binhex |=4;

load_STR_prefs_except_bufsizes();

list_buf_size=load_number(142,128);
printf_buf=NewPtr((Size)list_buf_size);

size_ext_table=load_number(143,10);

h_type=load_ostype_list(130);
h_creat=load_ostype_list(131);
h_extens=load_string_list(129);
h_comments=load_string_list(132);
h_ascii=load_boolean_list(133);

if((h=GetResource ('STR ',135))!=NULL ){
	if(**h){
		char c= *(*h+1)&~0x20;
		if(c=='I')
			in_Italia=true;
		else if(c=='E')
			in_Italia=false;
		else
			h=NULL;
		}
	else
		h=NULL;
	}

#ifdef THINK_C_5
#define Intl0Vers intl0Vers
#endif
if(h==NULL){
	h=IUGetIntl (0);
	if(h!=NULL){
		in_Italia=(((Intl0Rec*)*h)->Intl0Vers>>8)==verItaly || 
			(((Intl0Rec*)*h)->Intl0Vers>>8)==verItSwiss;
		ReleaseResource(h);
		}
	}

floppy_buffer_size =load_number(138,18);
hd_buffer_size = load_number(139,20)<<9;

binary_types_h=GetResource ('STR ',150);

load_options();
compute_delta_time();

if((h=GetResource('OPTs',134))!=NULL){
	default_type =**h;
	default_device_num=*(*h+1);
	ReleaseResource(h);
	}

/* compute the centered position for the the disk initialization dialog */
PositionDialog(&bmr);		/* finestra usata da DIBadMount, uso dimensioni fisse 
			che però sono quelle della finestra personalizzata e dovrebbero coincidere
			con quelle di sistema */
*(long*)&badmount_point=*(long*)&bmr;
/* sotto il System 7 passare {0,0} allo standard file provoca un dialogo 
centrato, ma purtroppo col system 6 non è così... */

/* and the positions for the standard file dialogs */
for(i=0;i<sizeof(SF_IDs)/sizeof(short);i++){
	AlertTHndl	alertHandle = (AlertTHndl)GetResource('DLOG',SF_IDs[i]);
	if(alertHandle){
		PositionDialog( &((**alertHandle).boundsRect));
		where[i]=*(Point*)&((**alertHandle).boundsRect);
		}
	else{
		static Point	def_where={80,80};
		where[i]=def_where;
		}
	}
}

void create_opts_128(void);
static void create_opts_128()
{
	Handle h=NewHandle(sizeof(options));
	HLock(h);
	mcopy(*h,&options,sizeof(options));
	HUnlock(h);
	AddResource (h, 'OPTs',128,PNS);
	SetResAttrs(h,resPreload|resPurgeable);

	ChangedResource(h);
	WriteResource(h);
}

static void load_options()
{
Handle h;


if((h=GetResource ('OPTs',128))==NULL){
	/* create the options resource if it doesn't exist: probably that will not
	be useful to the user, we'll make suntar 1.2 and 1.3 available with an OPTs
	resource (otherwise, a file which grows could make somebody think about
	viruses) but during the development of the program it was useful,
	each time we changed the format of the resource, deleting the old one
	one obtains the creation of the new one. That has become important
	with 2.0 which may inherit options from another copy (typically an
	older version) and a missing 128 means that suntar was not configured */
	create_opts_128();
	}
else{
	Size s=GetHandleSize(h);
	if(s<sizeof(options)){		/* somebody is using the old format of the OPTs
								resource (that of 1.2), convert it to the new format:
								it's always wise to support old formats for
								anything */
		SetHandleSize (h,(Size)sizeof(options));
		*( *h+(&options.opt_bytes[0]-(char*)&options) ) <<= 1;	/* it's no more a Boolean... */
		ChangedResource(h);
		WriteResource(h);
		}
	mcopy(&options,*h,sizeof(options));
	}
ReleaseResource(h);
}

#include <stdarg.h>

void save_options(Handle lh,...)
{
Handle h;
va_list pH;
unsigned char old_sep_prefs;

h=GetResource ('OPTs',128);

old_sep_prefs= ((options_type*)*h)->separate_prefs;

mcopy(*h,&options,sizeof(options));

va_start(pH,lh);

if(options.separate_prefs==old_sep_prefs || options.separate_prefs){
	/* the resources were loaded from where they must be saved, unless
	the options declares there must be a prefs file and such a file does not
	exist (quite common since I create it only if its contents are different
	from the values stored in the application) */

	if(!old_sep_prefs){	/* no -> sì. Visto comunque devo salvare almeno quella
						opzione dentro di me (e tanto vale salvarle tutte)
						e visto che sì->sì prevede la creazione,
						l'unica cosa extra che devo fare è cancellare il
						file di prefs, se esiste, è certo più pratico che
						aggiornarlo */
		if(open_pref_file(false)==noErr)
			cancella_file_aperto(pref_frefn);
		UseResFile(appl_frefn);
		}
	else if(options.separate_prefs && old_sep_prefs){
		/* sì -> sì */
		if(!pref_aperta){
			pref_aperta=open_pref_file(true)==noErr;
			if(!pref_aperta){
				ParamText(no_pref_file,PNS,PNS,PNS);
				my_alert();
				UnloadSeg(save_options);
				return;
				}
			/* e a questo punto, dovrei attaccare tutti gli handle h_text etc.
			alle nuove risorse e non alle vecchie !
			per il dialogo principale (OPTs 128) il problema non si pone in quanto
			l'Handle non lo conservo e ricarico la risorsa ogni volta che entro nel
			dialogo */
			ReleaseResource(h_type);
			ReleaseResource(h_creat);
			ReleaseResource(h_extens);
			ReleaseResource(h_comments);
			ReleaseResource(h_ascii);
			h_type=load_ostype_list(130);
			h_creat=load_ostype_list(131);
			h_extens=load_string_list(129);
			h_comments=load_string_list(132);
			h_ascii=load_boolean_list(133);
			/* per i MENU resto attaccato ai vecchi, visto che il menu manager usa
			la risorsa stessa e non una copia non posso fare altrimenti */
			}
		}
	ChangedResource(h);
	WriteResource(h);
	ReleaseResource(h);

	while((h=va_arg(pH, Handle))!=NULL){
		ChangedResource(h);
		WriteResource(h);
		}
	}
else if(!options.separate_prefs){	/* sì -> no */
	UseResFile(appl_frefn);	/* le risorse che creo o aggiorno non devono andare nel
					file di pref ! */
	h=Get1Resource('OPTs',128);
	if(h==NULL){
		create_opts_128();
		h=Get1Resource('OPTs',128);
		if(h==NULL){
			ParamText("\pNo write permissions on the application",PNS,PNS,PNS);
			my_alert();
			ExitToShell();
			}
		}
	else
		SetHandleSize(h,sizeof(options));
	mcopy(*h,&options,sizeof(options));
	ChangedResource(h);
	WriteResource(h);

	if(pref_aperta){
		if(expert_mode) del_expert_items();

		transfer_pref_resources(pref_frefn,appl_frefn,false);
	
		ParamText("\psuntar is quitting, relaunch to use it without the preferences file",PNS,PNS,PNS);
		/* forse potrei cavarmela meglio facendo un DetachResource e segnandomi
		di non dover fare ChangedResource+WriteResource al quit... ma comnque, ho
		disattivato il checkbox se fase!=non_faccio_nulla, quindi quittare non
		è dannoso */
		my_alert();
		cancella_file_aperto(pref_frefn);
		ExitToShell();	/* i MENU restano attaccati alle risorse del file di pref, e
			visto che non sono template ma proprio i dati usati dal Menu manager, non
			posso chiuderlo senza anche quittare */
		}
	}
va_end(pH);
UnloadSeg(save_options);
}




void save_opts_quitting(void);
void save_opts_quitting()
{
extern Boolean preferences_changed;

if((preferences_changed || def_cambiato) && options.separate_prefs && !pref_aperta){
	pref_aperta=open_pref_file(true)==noErr;
	if(!pref_aperta){
		ParamText(no_pref_file,PNS,PNS,PNS);
		my_alert();
		return;
		}
	}

if(preferences_changed){

/*attento ai menu, se quando creo il file non ce li creo... e comunque, se l'ho creato
appena ora rischio che "changed resource" faccia riferimento alla mia risorsa
e non a quella fuori, dovrei fare un GetResource e non usare l'handle già noto ! */
	/* save the preferences settings */
	if(expert_mode)
		del_expert_items();

	(**myMenus[prefM]).menuProc=NULL;	/* without it, horrible things happen */
	ChangedResource(myMenus[prefM]);
	WriteResource(myMenus[prefM]);

	(**hqxM).menuProc=NULL;
	ChangedResource(hqxM);
	WriteResource(hqxM);
	}
if(def_cambiato){
	Handle h=GetResource('OPTs',134);
	if(h==NULL){
		/* la risorsa non esiste, creala */
		h=NewHandle(2);
		AddResource (h,'OPTs',134,PNS);
		SetResAttrs(h,resPreload);
		}
	**h=default_type;
	*(*h+1)=default_device_num;
	ChangedResource(h);
	WriteResource(h);
	}
}


static short open_pref_file(create_flag)
Boolean create_flag;
{
short pref_vrefnum,io;
long pref_dirid;
static unsigned char prefs_name[]="\psuntar prefs";

if(gHasFindFolder){
	io=FindFolder(gMac.sysVRefNum,'pref',create_flag,&pref_vrefnum,&pref_dirid);
	if(io!=noErr) return io;
	pref_vrefnum=crea_WD(pref_vrefnum,pref_dirid);
	register_WD(pref_vrefnum);
	}
else	/* sono sotto System 6: crealo nella cartella sistema */
	pref_vrefnum=gMac.sysVRefNum;

pref_frefn=OpenRFPerm (prefs_name,pref_vrefnum,fsWrPerm);	/* più che altro
	per non fare un SetVol, l'option box può essere chiamato nel bel
	mezzo di un'operazione */
if(pref_frefn==-1){
	io=ResError();
	if(io==fnfErr && create_flag){
		short oldvRefNum;
		Str63 name;
		if(GetVol (&name,&oldvRefNum)!=noErr) oldvRefNum=0;
		SetVol(NULL,pref_vrefnum);

		CreateResFile(prefs_name);
		io=ResError();

		SetVol(NULL,oldvRefNum);
		if(io==noErr){
			reassign_type_creator(pref_vrefnum,prefs_name,'pref','S691');
			pref_frefn=OpenRFPerm(prefs_name,pref_vrefnum,fsWrPerm);
			io=ResError();
			if(io==noErr){
				Handle h;
				if(expert_mode) del_expert_items();
				transfer_pref_resources(appl_frefn,pref_frefn,false);
				if(expert_mode) add_expert_items();
				/* attento all'OPTs 128 ! se non c'è devo crearla,
				ma solo nel pref file */
				UseResFile(pref_frefn);
				if(Get1Resource('OPTs',128)==NULL){
					create_opts_128();
					#if 0
					/* trasferiscila ! */
					DetachResource(h);	/* disconnect h from the resource inside suntar */
					UseResFile(pref_frefn);
					AddResource(h,'OPTs',128,PNS);
					WriteResource(h);
					#endif
					}
				}
			}
		}
	}
else
	io=noErr;

return io;
}



void load_char_tables()
/* loads the tables to translate chars */
{
extern short openfile_vrefnum;
extern long openfile_dirID;
#define ioDirID ioFlNum


OSErr i;
ParamBlockRec pb;	/* per non sporcare la var globale... */
long bytes_in_buffer=0;
register unsigned char c,*cpt;
Boolean skipping=0;
unsigned int n;
Handle h;


for(n=0;n<256;n++){
	direct_table[n]=inverse_table[n]=n;
	isascii_table[n]= n>=' ' && n <= 126;
	}
isascii_table['\t']=isascii_table['\f']=1;

get_openfile_location(appl_frefn);
h=GetResource('STR ',154);
if(h==NULL) return;
HLock(h);

fillmem(&pb,0,sizeof(pb));
pb.fileParam.ioNamePtr=*h;
pb.fileParam.ioVRefNum=openfile_vrefnum;
pb.fileParam.ioDirID=openfile_dirID;
pb.ioParam.ioPermssn=fsRdPerm;

i=PBHOpenSync(&pb);
ReleaseResource(h);

if(i==noErr){
	for(;;){
		do{	/* skip blanks & comment lines */
			if(bytes_in_buffer==0){
				bytes_in_buffer=512;
				i=FSRead(pb.ioParam.ioRefNum,&bytes_in_buffer,disk_buffer);
				if(bytes_in_buffer==0 || i!=noErr && i!=eofErr){
					FSClose(pb.ioParam.ioRefNum);
					return;
					}
				cpt=disk_buffer;
				}
			bytes_in_buffer--;
			c=*cpt++;
			if(c==CR || c==LF)
				skipping=false;
			else if(c=='*')
				skipping=true;
			}
		while(c==' ' || c==CR || c==LF || c=='\t' || skipping);
		decode_err:
		if(c<'0' || c>'9'){
			printf("error in decoding char table\n");
			skipping=true;
			continue;
			}
		/* now, decode the number */
		n=c-'0';
		for(;;){
			if(bytes_in_buffer==0){
				bytes_in_buffer=512;
				i=FSRead(pb.ioParam.ioRefNum,&bytes_in_buffer,disk_buffer);
				if(bytes_in_buffer==0 || i!=noErr && i!=eofErr)
					goto decode_err;
				cpt=disk_buffer;
				}
			bytes_in_buffer--;
			c=*cpt++;
			if(c=='=') break;
			if(c<'0'||c>'9') goto decode_err;
			n = n*10+ (c-'0');
			}
		/* and this is the corresponding char: */
		if(bytes_in_buffer==0){
			bytes_in_buffer=512;
			i=FSRead(pb.ioParam.ioRefNum,&bytes_in_buffer,disk_buffer);
			if(bytes_in_buffer==0 || i!=noErr && i!=eofErr)
				goto decode_err;
			cpt=disk_buffer;
			}
		bytes_in_buffer--;
		c=*cpt++;

		if(n<256){
			direct_table[n]=c;
			if(c<' '||c>=127) inverse_table[c]=n;
			isascii_table[n]=1;
			}
		}
	}
}


/*********************************************************/

/* not a "load pref", but it's code executed only at startup */

void add_menu(void);
void add_menu()	/* create application specific menus */
{
extern MenuHandle writeMenu,popupMenu[5];
short markChar;

	CheckItem(myMenus[prefM],pmEnglish-!expert_mode,!in_Italia);
	CheckItem(myMenus[prefM],pmAutowrap,false);
	CheckItem(myMenus[prefM],pmConfirm-1,false); /* è salvato non in expert mode... */

#ifdef V_122
	GetItemMark (myMenus[prefM],pmSmallText,&markChar);
	smallFilesAreASCII= markChar==checkMark;
#endif
	GetItemMark (myMenus[prefM],pmExpert,&markChar);
	expert_mode= markChar==checkMark;
	InsertMenu(writeMenu = GetMenu(writeID), 0);
	disable_write_items();
	if(expert_mode) add_expert_menu();	/* its items always start un-checked... */
	popupMenu[0] = GetMenu(tarPopupID);
	popupMenu[1] = GetMenu(ntAPopupID);
	popupMenu[2] = GetMenu(148);
	popupMenu[3] = GetMenu(149);
	popupMenu[4] = GetMenu(150);
}


