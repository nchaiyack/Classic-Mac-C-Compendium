/*******************************************************************************\

customized standard file module

© 1991 - 1995 Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

#include "system7.h"
pascal OSErr FindFolder(short vRefNum,OSType folderType,Boolean createFolder,
    short *foundVRefNum,long *foundDirID)
    = {0x7000,0xA823};

#include "PB_sync.h"
#include "antiglue.h"

#include <string.h>
#include "suntar.h"
#include "windows.h"

#define SETDITL
#define CLOSE_SF_WD
#define DA_CANCEL 	/* always force Cancel even when an Open would have the same effect */

#define sf_popup 9

short curr_vrefnum;
short curr_sf_format=1;
char which_popup;

extern short stored_vrefnums;

extern unsigned char PNS[];
extern Boolean gHasCustomPutFile,need_a_touch;
extern short forza_formato,formato_encode;

char *my_p2cstr();
SFReply		reply;
Point		where[7];
static short n_open_WD=0;
#define max_open_WD 20
static short open_WD[max_open_WD];
unsigned char*nomeFormato;
Boolean SB3_compatibility=false;

#ifndef NULL
#define NULL 0L
#endif
#define sfHookNullEvent 100

void set_DITL_title(short,unsigned char *);
pascal short file_or_folder_hook(short,DialogPtr);
pascal Boolean file_or_folder_filter(DialogPtr,EventRecord*,short *);
/*pascal Size memory_overflow(Size);*/
pascal void redrawPopup(WindowPtr,short);
void do_popup_selection(DialogPtr,short);
void append_to_refnum_list(int);

short crea_WD(short,long);

#define pStrcmp(x,y) !EqualString(x,y,true,true)

/************* routine per selezionare un file o directory ******************/

#define writeButton		11
#define cancelButton	3
#define openButton		1

/* these variables are used to communicate with hooks and event filters */
static long		current_dir_ID;
static short	current_val_OK;
static Boolean need_update,is_outline;
static unsigned char *nome_originale;
#define checkbox_state need_update	/* just to avoid using another variable */
#define premuto_pausa need_update	/* just to avoid using another variable */
#define current_vol current_val_OK
#define was_a_true_dialog current_val_OK


static unsigned char string_1[]="\pSalva qui",
	string_2[]="\pAnnulla",
	string_3[]="\pEspelli",
	string_4[]="\pUnità Disco",
	string_5[]="\pgvu1h5%&#*√Æ@πå¥◊Ÿª",
	/*string_5[]="\p\10\0\37";*/	/* an "impossible" file name: no more useful since
		now I simulate a Cancel. Well, not exactly: AppleShare may see the name even 
		when Cancel is simulated, hence there must be a name and it must not contain
		"absurd" characters, otherwise you get a System Error */
	string_6[]="\pApri",
	string_7[]="\pOpen";

#define ESC 27

pascal Boolean SF_filter(DialogPtr,EventRecord*,short *);
static pascal Boolean SF_filter(theDialog, theEvent, itemHit)
DialogPtr theDialog;
EventRecord* theEvent;
short * itemHit;
{

if(theEvent->what==nullEvent){
	/* under System 7, the update events for windows below the System 6-like
	standard file dialogs are not received if I don't explicitly ask for them.
	I don't know whether it's a bug or an attempt to solve the problem described
	in TN304, which for some reason believes that suntar belongs to the category
	of programs which must be cheated.
	System 7 is wonderful, but all its tricks are a damnation for programmers !
	And they are not documented... and there is no way to tell to System 7 that
	I know that I must behave well and I must not be cheated...
	*/
	if(!EventAvail (updateMask,theEvent) )
		return false;
  	}
if(theEvent->what==updateEvt||theEvent->what==activateEvt){
	/*SetPort(theEvent->message);*/
	UpdateFilter(theEvent);
	}
else if(theEvent->what==keyDown){
	short	kind;
	Handle	h;
	Rect	r;
	if((unsigned char)theEvent->message==CR || (unsigned char)theEvent->message==enter_key){
		GetDItem(theDialog,1,&kind,&h,&r);

		if( ((ControlRecord*)*h)->contrlHilite==0){
			SelectButton(h);
			*itemHit=1;
			return true;
			}
		else
			theEvent->what=nullEvent;
		}
	else if((unsigned char)theEvent->message==ESC ||
			(unsigned char)theEvent->message=='.'&&(theEvent->modifiers&cmdKey) ){
		GetDItem(theDialog,3,&kind,&h,&r);		/* the Cancel button is item 3 in a
						get dialog, it's 2 in a put (and 3 is the prompt string, not
						a button) */
		if(kind==ctrlItem+btnCtrl)
			*itemHit= 3;
		else
			GetDItem(theDialog,(*itemHit= 2),&kind,&h,&r);
		SelectButton(h);
		return true;
		}
	}
return false;
}


void my_SF_Put(prompt,fname)
unsigned char *prompt,*fname;
{
	flush_all();
	check_foreground();
	/*SetGrowZone(NULL);*/
	SFPPutFile(where[2],prompt,fname,NULL,&reply, -3999, SF_filter);
	/*SetGrowZone(memory_overflow);*/
	#ifdef CLOSE_SF_WD
	if(reply.good) register_WD(reply.vRefNum);
	#endif
}

void my_SF_Get(numTypes,typeList)
short numTypes;
SFTypeList typeList;
{
	flush_all();
	/*SetGrowZone(NULL);*/
	SFPGetFile(where[3],PNS,NULL,numTypes,typeList,NULL,&reply, -4000, SF_filter);
	/*SetGrowZone(memory_overflow);*/
#ifdef CLOSE_SF_WD
	if(reply.good) register_WD(reply.vRefNum);
#endif
}

extern ListHandle LHandle;
extern DialogPtr ListDialog;


pascal Boolean file_filter(ParmBlkPtr);
static pascal Boolean file_filter(pb)
ParmBlkPtr pb;
{
/* testing to see what can be done: the problem is that if the displayed "folder"
is the Desktop there are a number of "real" folders, each on a different volume,
so I can't rely on -SFSaveDisk and CurDirStore for "write selected folder"
#define ioDirID ioFlNum
short foundref; long founddir;
	printf("vrefnum=%d %ld\n",pb->fileParam.ioVRefNum,pb->fileParam.ioDirID);
	if(noErr==FindFolder(pb->fileParam.ioVRefNum,'desk',0,&foundref,&founddir) )
		printf("desktop =%d %ld\n",foundref,founddir);
	printf("globals=%d %ld \n",-SFSaveDisk,CurDirStore);*/
	/*vrefnum è OK, ioDirID no, diverso per ciascuno: si tratta dei dati ritornati
	da PBGetFInfo che in quel campo ritorna comunque ioFlNum
	-- pb-> contains the data of PBGetFInfo (or PBHGetFInfo, they return the same
	data), so the "ioDirID" field, very, very useful to me, is replaced by the
	almost useless ioFlNum
	*/

if(LHandle){
	/*printf("%P\n",pb->fileParam.ioNamePtr);*/
	append_to_refnum_list(pb->fileParam.ioVRefNum);
	append_to_list(&pb->fileParam.ioNamePtr[1], pb->fileParam.ioNamePtr[0]);
	return true;
	}
return false;
}



#define TITLEBARHEIGHT 20
#define multipleButton 14

static pascal short file_or_folder_hook(item, theDialog)
	short		item;
	DialogPtr	theDialog;
{
	extern Boolean in_Italia;

/*short old;
if(item==-1 ||-SFSaveDisk!=old){
	old=-SFSaveDisk;
	printf("vrefnum=%d\n",old);
	}
*/

	if(item==-1) {
/* initialization: translate button titles to Italian */
		if(in_Italia){
			#ifndef SETDITL
			set_item_text(theDialog,1,string_6);
			#endif
			set_item_text(theDialog,3,string_2);
			set_item_text(theDialog,5,string_3);
			set_item_text(theDialog,6,string_4);
			set_item_text(theDialog,11,"\pScrivi cartella");
			set_item_text(theDialog,12,"\pScegli il file o cartella da archiviare");
			set_item_text(theDialog,14,"\pSeleziona e scrivi files");
			}
		set_item_text(theDialog,13,nomeFormato);
		current_val_OK = false;
		}
	else if(item==writeButton){		/* file o directory selezionata*/
		if (reply.fType != 0 || reply.fName[0]!=0) {
		/* remember which folder was currently selected, if it was a folder */
			current_dir_ID = reply.fType;	/* if the current selection is a folder,
						this field contains its dir ID */
			current_val_OK = reply.fName[0]==0 ? 1 : -1;	/*cioè, solo se è una directory, non un file
						 -- the name is NOT stored here if it was a folder (se IM vol IV) */
			return cancelButton; /* 
					declare that the cancel button was pressed, remembering that 
					it's not so; for files, I could return openButton, but there is 
					a problem: if the selected file is an alias under System 7, 
					the alias is NOT resolved and a beep is done: I wish that 
					unresolved alias, but I don't wish that beep ! */
			}
		else
			return sfHookNullEvent;	/* per dirgli di ignorare la cosa... non dovrebbe capitare comunque
						grazie al filtro di eventi che disabilita il bottone
						-- Really, the event filter disables the button when nothing
						is selected, so I should not receive "button pressed" items,
						but if I do, I ignore it */
		}
	else if(item==multipleButton){
		new_list_dialog(148,(WindowPtr)theDialog,false);
		stored_vrefnums=0;
		current_val_OK = -1;
		return 101;		/* fake item, redraw list, so my filter will capture it */
		}
	if(LHandle) return cancelButton;

	return item;
}

static pascal Boolean file_or_folder_filter(theDialog, theEvent, itemHit)
DialogPtr theDialog;
EventRecord* theEvent;
short * itemHit;
{
short	kind;
Rect	r;
#if 0
ControlHandle openH,writeH;

GetDItem(theDialog,openButton,&kind,&openH,&r);
GetDItem(theDialog,writeButton,&kind,&writeH,&r);
HiliteControl(writeH, (**openH).contrlHilite);	/* the write button is enabled/disabled
			to follow the current state of the Open Button */
#else
ControlHandle writeH;

GetDItem(theDialog,writeButton,&kind,&writeH,&r);

if (reply.fType != 0 && reply.fName[0]==0 || reply.fType=='fdrp')
	/* a folder, or the alias of a folder, is selected */
	HiliteControl(writeH,0);
else
	HiliteControl(writeH,255);
#endif
return SF_filter(theDialog,theEvent,itemHit);
}

short get_file_or_folder(s,n_types,myTypes)
/* presents a modified file dialog which allows to choose either a file or
a folder, with the extra "Write" and "Write selected files" buttons
returns:
-1 cancel
0 folder
1 file
2 list of files
*/
Str255	s;
short n_types;
SFTypeList		*myTypes;
{short i;

	flush_all();
	LHandle=NULL;
#ifdef SETDITL
	set_DITL_title(200,in_Italia?string_6:string_7);
#endif

	/*SetGrowZone(NULL);*/
	SFPGetFile(where[1],PNS,file_filter,n_types,myTypes,file_or_folder_hook,&reply,
		200,file_or_folder_filter);
	/*SetGrowZone(memory_overflow);*/

	reply.fName[1+reply.fName[0]]=0;
	if (current_val_OK>0){
	/* write folder.
	the vRefNum in the reply is not correct, but the low memory variable SFSaveDisk 
	holds the negative of the right value */
#ifdef V_122
		if(i=PathNameFromDirID(current_dir_ID,(curr_vrefnum= -SFSaveDisk),s)) {
			/* TN 238 sconsiglia di creare dei full path name, perché in A/UX i
			full path name vogliono la '/' anche se si fanno chiamate alle routine
			HFS del toolbox... */
			SysBeep(5);
			printf("Pathname error\n");
			raise_error();
			}
#else
		/* get the name of the folder */
		{
		CInfoPBRec	block;
		Str255		directoryName;
		OSErr err;

		block.dirInfo.ioNamePtr = &s[1];
		block.dirInfo.ioDrParID = current_dir_ID;

		block.dirInfo.ioVRefNum = curr_vrefnum= -SFSaveDisk;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;

		err = PBGetCatInfoSync(&block);
		if(err!=noErr) error_message_1("Folder error %d\n",err);
		if(current_dir_ID==2){		/* root directory, it's already a refnum... */
			mcopy(s,&s[1],s[1]+1);	/* must not add a semicolon */
			}
		else{
			s[0] = s[1]+1;
			s[1]=':';

			/* open a WD on the parent directory */

			curr_vrefnum=crea_WD(curr_vrefnum,block.dirInfo.ioDrParID);
			register_WD(curr_vrefnum);
			}
		/* Append a Macintosh style colon (':') */
		pStrcat(s,"\p:");
		SetVol(NULL,curr_vrefnum);
		}
#endif
		return 0;
		}
	if(current_val_OK<0){	/* write file */
		reply.vRefNum=crea_WD(-SFSaveDisk,CurDirStore);
		register_WD(reply.vRefNum);
		}
	else if(!reply.good)
		return -1;	/* cancel */
	/* if I am here, it was a open/write file */
	SetVol(NULL,reply.vRefNum);	/* the old version of the program usually passed
			0 as vrefnum relying on a SetVol, the new one prefers to pass explicitly
			curr_vrefnum, but if we have forgotten to modify one call, doing
			also the SetVol avoid troubles: one day all these SetVol will be
			deleted, but then an accurate beta testing should follow... */
#ifdef CLOSE_SF_WD
	register_WD(reply.vRefNum);
#endif
	curr_vrefnum=reply.vRefNum;

	if(LHandle)
		return 2;

	pStrcpy(s,reply.fName);
	return 1;
}

/********** routines per la selezione di una directory di destinazione *********/

pascal short select_dir_hook (short,DialogPtr);
static pascal short select_dir_hook (item, theDialog)
 short item;
 DialogPtr theDialog;
{

if(item==-1 && in_Italia){
	#ifndef SETDITL
	set_item_text(theDialog,1,string_1);
	#endif
	set_item_text(theDialog,2,string_2);
	set_item_text(theDialog,5,string_3);
	set_item_text(theDialog,6,string_4);
	}
else if(item==1){	/* Save here */
	current_val_OK=true;
	#ifdef DA_CANCEL
	return 2;	/* Cancel: hence, no risk to get a dialog "file existing, OK to replace ?" */
	#endif
	}
return item;
}

pascal Boolean select_dir_filter(DialogPtr,EventRecord*,short *);
static pascal Boolean select_dir_filter(theDialog, theEvent, itemHit)
DialogPtr theDialog;
EventRecord* theEvent;
short * itemHit;
{

if(theEvent->what==keyDown||theEvent->what==autoKey){
	theEvent->what=nullEvent;
	}	/* se no si può cancellare il nome, e il bottone per
								salvare viene disabilitato...
								-- the file name can't be modified ! It will be ignored,
								but if you delete it, the save button will be disabled,
								and save here IS the save button */
/* no, niente da fare, lo standard file non me li passa affatto
-- useless, such events are not passed to my filter
else if(theEvent->what==diskEvt){
	unexpected_disk_insertion(theEvent->message);
	theEvent->what=nullEvent;
	}
*/

return SF_filter(theDialog, theEvent, itemHit);
}


pascal Boolean sys7_select_dir_filter(DialogPtr,EventRecord*,short *,Ptr);
static pascal Boolean sys7_select_dir_filter(theDialog, theEvent, itemHit, myDataPtr)
DialogPtr theDialog;
EventRecord* theEvent;
short * itemHit;
Ptr myDataPtr;
{
if(theEvent->what==updateEvt && (DialogPtr)theEvent->message==theDialog)
	need_update=true;
return SF_filter(theDialog, theEvent, itemHit);
}


pascal short sys7_select_dir_hook(short,DialogPtr,Ptr);
static pascal short sys7_select_dir_hook(item,theDialog,myDataPtr)
short item;
DialogPtr theDialog;
Ptr myDataPtr;
{
/* it is correct to draw an outline to the "save here" button, however
ther are two problems:
1) there is not a good place to do that: in the event filter, I could do that as an
	answer to the update event, but it should be between BeginUpdate and EndUpdate,
	or in the worst case after EndUpdate, while I'm called before. In my_modal
	I handle the whole update by myself, but here doing that could cause 
	incompatibility problems (if something is redrawn directly and not through
	the redraw routine of a dialog item... for example, Boomerang's file info
	disappear in case of an update). Hence I do that periodically, that's
	NOT a good solution but I haven't found a better one
	(probably the "Better" way is to install an user item for the outline...
	maybe I'll try that one day)
2) The return and enter key mean "open" or "click on the default button"
	according to what is currently selected (a folder/volume, or a file/nothing):
	hence, I should outline the Save here button only when return means Save,
	but the in StandardFileReply record only parID and vRefNum are updated, (the name,
	sfIsFolder and sfIsVolume are updated only by Custom G E T file). Furthermore,
	if the current volume is locked the Save button is dimmed, and must not be
	outlined: this time, I should update things when the state change happens, that is
	AFTER the handling of clicks on items which change the situation, but my
	hook routine is called BEFORE the standard handling is performed: this time, a
	periodic action looks like the only way to do things
*/

short	kind;
ControlHandle	h;
Rect	r;
short i;
ControlHandle saveH;
/*
StandardFileReply *sfrep=myDataPtr;
printf("%d %d %ld %d %P\n",sfrep->sfIsFolder,sfrep->sfIsVolume,sfrep->sfFile.parID,
sfrep->sfFile.vRefNum,sfrep->sfFile.name);
*/

if(((WindowPeek)theDialog)->refCon!='stdf') return item;

GetDItem(theDialog,1,&kind,&h,&r);
GetDItem(theDialog,13,&kind,&saveH,&r);
if((**h).contrlHilite!=(**saveH).contrlHilite)
	HiliteControl(saveH, (**h).contrlHilite);	/* the save here button is 
			enabled/disabled to follow the current state of the save Button */
if(!(i=(**h).contrlHilite)){
	char buffer[20];
	GetCTitle(h,buffer);
	i=strcmp("Save",my_p2cstr(buffer));	/* the "Open" string is chosen from the
										system, depending on the nation, but the
										Save string is from my resource... */
	}
if(i){	/* erase the outline */
	/* probably it's possible to avoid all that by exploiting the calls described in
	Technical Note 304, but beta testing of 1.2 has already started, it's too late to
	try new routines */
	if(is_outline){
		Rect r1;
		SetPort(theDialog);
		/* probably I should create a region as difference of two rectangles
		and then erase the region... but this method works and why should I change it ? */
		r1.left=r.left-4;
		r1.right=r.left;
		r1.top=r.top-4;
		r1.bottom=r.bottom+4;
		EraseRect(&r1);
		r1.left=r.right;
		r1.right=r.right+4;
		EraseRect(&r1);
		r1.left=r.left;
		r1.right=r.right;
		r1.top=r.top-4;
		r1.bottom=r.top;
		EraseRect(&r1);
		r1.top=r.bottom;
		r1.bottom=r.bottom+4;
		EraseRect(&r1);
		}
	is_outline=false;
	}
else{
	if(!is_outline||need_update){
		OutlineControl(saveH);
		need_update=false;
		}
	is_outline=true;
	}

if(item==-1&&in_Italia){
	set_item_text(theDialog,13,string_1);
	set_item_text(theDialog,2,string_2);
	set_item_text(theDialog,5,string_3);
	set_item_text(theDialog,6,"\pScrivania");
	}
else if(item==13){	/* Save here */
	current_val_OK=true;
	return 2;	/* Cancel */
	}
else if(item==1){	/* the Save button is out of screen,
			but it's still the default button hence it's "pressed" by a return key */
	GetDItem(theDialog,13,&kind,&h,&r);
	SelectButton(h);
	current_val_OK=true;
	return 2;	/* Cancel */
	}
return item;
}



void select_directory()
{
/* Presents a standard Put File where the save button is replaced by a "Save here"
button and the filename box is not visible. The caller will ignore the name field
in the reply record, using this routine to select the destination folder
*/
	flush_all();
	need_a_touch=false;
	check_foreground();	/* se converto più files...
						-- while converting more files, using drag&drop... */
	current_val_OK=false;
	if(!gHasCustomPutFile /*|| !useSys7_SF suppressed in version 2.0.4 */){
#ifdef SETDITL
		set_DITL_title(199,in_Italia?string_1:"\pSave here");
#endif
		/*SetGrowZone(NULL);*/
		SFPPutFile(where[0],PNS,
			string_5,select_dir_hook,&reply,199,select_dir_filter);
		/*SetGrowZone(memory_overflow);*/
#ifndef DA_CANCEL
		if(reply.good)
			SetVol(NULL,curr_vrefnum=reply.vRefNum); /* setta la directory */
		return;
#endif
		}
	else{
		/* the system 6 dialog is particularly annoying when used under system 7
		(the "Save here" button is renamed open when a folder is selected) and
		it's here that the new "new folder" button is more useful: hence the privilege
		of being the only call to the standard file package done in the system 7 way */
		StandardFileReply sfrep;
		static short act_list[2]={1,7};
		need_update=is_outline=false;
		CustomPutFile(PNS,string_5,&sfrep,1043,where[4],sys7_select_dir_hook,
			sys7_select_dir_filter, act_list,NULL, &sfrep);
		reply.good=false;
		}

	if(current_val_OK){
		OSErr err;
#define LOW_LEVEL_OWD
#ifdef LOW_LEVEL_OWD
		curr_vrefnum=crea_WD(-SFSaveDisk,CurDirStore);
#else
		if((err=OpenWD(-SFSaveDisk,CurDirStore,0L,&curr_vrefnum))){
			error_message_1("Working directory error %d\n",err);
			}
#endif

		register_WD(curr_vrefnum);

		SetVol(NULL,curr_vrefnum); /* setta la directory */
		reply.vRefNum=curr_vrefnum;
		reply.good=true;
		}
}


#if 0	/* a sfGet with a checkbox, no more used */
pascal short checkbox_hook(short,DialogPtr);
static pascal short checkbox_hook(item, theDialog)
	short		item;
	DialogPtr	theDialog;
{
	short	kind;
	Handle	h;
	Rect	r;
	extern Boolean in_Italia;

	if(item==-1) {
/* initialization: translate button titles to Italian */
		GetDItem(theDialog,11,&kind,&h,&r);
		SetCtlValue((ControlHandle)h,checkbox_state);
		if(in_Italia){
			SetCTitle(h,"\pscelta formato automatica");
			#ifndef SETDITL
			set_item_text(theDialog,1,string_6);
			#endif
			set_item_text(theDialog,3,string_2);
			set_item_text(theDialog,5,string_3);
			set_item_text(theDialog,6,string_4);
			}
		}
	else{
		was_a_true_dialog=true;
		if (item==11){
			SetCheckBox(theDialog,item);
			checkbox_state= ! checkbox_state;
			}
		}
	return item;
}


Boolean SFGet_with_checkbox(ck_state)
Boolean *ck_state;
{
SFTypeList		myTypes;

flush_all();

checkbox_state=true;

was_a_true_dialog=false;

#ifdef SETDITL
set_DITL_title(400,in_Italia?string_6:string_7);
#endif

/*SetGrowZone(NULL);*/	/* damned SuperBoomerang ! It tries to allocate a lot of
		memory, if it can't then it manages to do its job anyway, but since
		suntar does never check for null pointers, that request activates its own
		"catastrophe handler" which returns to the Finder with an alert. That's
		why I must remove that safety check during a SF dialog. Well, no
		more, I've changed my overflow routine */

SFPGetFile(where[5],PNS,NULL,-1,myTypes,checkbox_hook,&reply,
		400, SF_filter);
/*SetGrowZone(memory_overflow);*/
if(reply.good){
#ifdef CLOSE_SF_WD
	register_WD(reply.vRefNum);
#endif
	*ck_state= checkbox_state;
	}
if(!reply.good) return 0;
return 1+was_a_true_dialog;
/*return reply.good;*/
}

#endif

pascal short popup_hook(short,DialogPtr);
static pascal short popup_hook(item, theDialog)
	short		item;
	DialogPtr	theDialog;
{
	short	kind;
	Handle	h;
	Rect	r;
	extern Boolean in_Italia;

	if(item==-1) {
/* initialization: translate button titles to Italian */
		GetDItem(theDialog,11,&kind,&h,&r);
		SetDItem (theDialog,11, kind, (Handle)redrawPopup, &r);
		if(in_Italia){
			#ifndef SETDITL
			set_item_text(theDialog,1,string_6);
			#endif
			set_item_text(theDialog,3,string_2);
			set_item_text(theDialog,5,string_3);
			set_item_text(theDialog,6,string_4);
			}
		}
	else{
		was_a_true_dialog=true;
		if (item==11){
			do_popup_selection(theDialog,11);
			}
		}
	return item;
}


Boolean SFGet_with_popup()
{
SFTypeList		myTypes;

flush_all();

forza_formato=1;	/* better to restart every time */
was_a_true_dialog=false;

#ifdef SETDITL
set_DITL_title(400,in_Italia?string_6:string_7);
#endif

SFPGetFile(where[5],PNS,NULL,-1,myTypes,popup_hook,&reply,
		400, SF_filter);
if(reply.good){
#ifdef CLOSE_SF_WD
	register_WD(reply.vRefNum);
#endif
	}
if(!reply.good) return 0;
return 1+was_a_true_dialog;
}

/*******************************/

pascal short SFP_pausa_hook(short,DialogPtr);
static pascal short SFP_pausa_hook(item, theDialog)
	short		item;
	DialogPtr	theDialog;
{

	if(item==-1) {
/* initialization: translate button titles to Italian */
		if(in_Italia){
			#ifndef SETDITL
			set_item_text(theDialog,1,"\pSalva");
			#endif
			set_item_text(theDialog,2,"\pNon salvare");
			set_item_text(theDialog,5,string_3);
			set_item_text(theDialog,6,string_4);
			set_item_text(theDialog,9,"\pPausa");
			}
		current_vol=SFSaveDisk;
		current_dir_ID=CurDirStore;
		}
	else if (item==9){
		premuto_pausa=true;
		return 2; /* cancel...*/
		}
	else if(item==1 && CurDirStore==current_dir_ID && SFSaveDisk==current_vol &&
			!pStrcmp(nome_originale,reply.fName)){
		premuto_pausa=2;
		return 2;	/* cancel */
		}
	return item;
}



Boolean SFPut_con_pausa(fname,message)
unsigned char *fname;
char *message;
{
flush_all();
premuto_pausa=false;
#ifdef SETDITL
set_DITL_title(399,in_Italia?"\pSalva":"\pSave");
#endif
nome_originale=fname;
/*SetGrowZone(NULL);*/
SFPPutFile(where[6],message,fname,SFP_pausa_hook,&reply,399,SF_filter);
/*SetGrowZone(memory_overflow);*/
if(premuto_pausa==1) return true;
if(premuto_pausa==2){
	/* cioè è un Cancel ma solo per evitare il dialogo... */
	reply.good=true;
	reply.vRefNum=crea_WD(-current_vol,current_dir_ID);
	pStrcpy(reply.fName,fname);	/* non si sa mai */
	}

#ifdef CLOSE_SF_WD
if(reply.good) register_WD(reply.vRefNum);
#endif
return false;
}


pascal short SFP_popup_hook(short,DialogPtr);
static pascal short SFP_popup_hook(item, theDialog)
	short		item;
	DialogPtr	theDialog;
{
static unsigned char *name_string[]={"\parchive.tar","\parchive.bar"};
static unsigned char *suffix_string[]={"\p.bin","\p.hqx","\p.uu"};

	if(item==-1) {
		short	kind;
		Handle	h;
		Rect	r;
/* initialization: translate button titles to Italian */
		if(in_Italia){
			#ifndef SETDITL
			set_item_text(theDialog,1,"\pCrea");
			#endif
			set_item_text(theDialog,2,"\pAnnulla");
			set_item_text(theDialog,5,string_3);
			set_item_text(theDialog,6,string_4);
			set_item_text(theDialog,10,"\pformato");
			}
		GetDItem(theDialog,sf_popup,&kind,&h,&r);
		SetDItem (theDialog,sf_popup, kind, (Handle)redrawPopup, &r);
		if(which_popup==0){
			if(curr_sf_format<1 || curr_sf_format>2) curr_sf_format=1;
			set_item_text(theDialog,7,name_string[curr_sf_format-1]);
			SelIText (theDialog,7,0,name_string[curr_sf_format-1][0]-4);	/* sembra
				non funzionare, forse questa routine è chiamata troppo presto,
				prima che venga assegnata la selezione */
			}
		else{
			unsigned char olds=tarh.name[0];
			if(formato_encode<=0 || formato_encode>3) formato_encode=1;
			pStrcat(tarh.name,suffix_string[formato_encode-1]);
			set_item_text(theDialog,7,tarh.name);
			SelIText (theDialog,7,0,olds);
			tarh.name[0]=olds;
			}
		}
	else if (item==9){
		Boolean unchanged;
		unsigned char olds;
		if(which_popup==0)
			unchanged=!pStrcmp(name_string[curr_sf_format-1],reply.fName);
		else{
			olds=tarh.name[0];
			pStrcat(tarh.name,suffix_string[formato_encode-1]);
			unchanged=!pStrcmp(tarh.name,reply.fName);
			}
		do_popup_selection(theDialog,9);
		if(which_popup==0){
			if(unchanged){
				set_item_text(theDialog,7,name_string[curr_sf_format-1]);
				SelIText (theDialog,7,0,name_string[curr_sf_format-1][0]-4);
				}
			}
		else{
			if(unchanged){
				tarh.name[0]=olds;
				pStrcat(tarh.name,suffix_string[formato_encode-1]);
				set_item_text(theDialog,7,tarh.name);
				SelIText (theDialog,7,0,olds);
				}
			tarh.name[0]=olds;
			}
		}
	return item;
}

Boolean SFPut_with_popup(message)
unsigned char *message;
{
flush_all();
#ifdef SETDITL
set_DITL_title(299,in_Italia?"\pCrea":"\pNew");
#endif
/*SetGrowZone(NULL);*/
SFPPutFile(where[6],message,PNS,SFP_popup_hook,&reply,299,SF_filter);
/*SetGrowZone(memory_overflow);*/

#ifdef CLOSE_SF_WD
if(reply.good) register_WD(reply.vRefNum);
#endif
return false;
}


/*********************************************************************/

#ifdef SETDITL
static void set_DITL_title(id,title)
short id;
unsigned char *title;
{
/* System 7 changes the name of the "open" or "save" button to reflect the two
different meanings (open a folder or select a file). In doing that, it forgets
any SetCTitle and restores the title stored in the DITL: hence, the only way to
change the title of those buttons at run time is to modify the DITL itself,
an horribly dirty thing; really, in suntar the two strings usually happen to have
the same size, but it's better not to exploit special cases
*/
unsigned char **h,cur_l,new_l;

h = GetResource('DITL',id);
if(h==NULL) return;
HNoPurge (h);	/* modifying a resource in memory would be useless if it may be purged */

cur_l= (*(*h+15)+1)&~1;	/* the affected button is always item 1, hence I may
						use a fixed offset; otherwise, I should skip the previous
						items by adding their (size+1)&~1 to the pointer */
new_l= (title[0]+1)&~1;
if(cur_l != new_l){
	Size s=GetHandleSize(h);
	if(s<0) return;
	if(new_l==cur_l)
		;
	else if(new_l<cur_l){
		BlockMove(*h+15+cur_l,*h+15+new_l,s-15-cur_l);
		SetHandleSize (h,s+new_l-cur_l);
		}
	else{	/* new_l>cur_l */
		SetHandleSize (h,s+new_l-cur_l);
		BlockMove(*h+15+cur_l,*h+15+new_l,s-15-cur_l);
		}
	}
pStrcpy(*h+15,title);
if(new_l!=title[0]) *(*h+16+new_l)=0;
}
#endif

/*
 Working directories must be closed: because there is a limited space,
but above all because a folder in the trashcan can't be deleted when an
open WD refers to it, causing an annoying dialog when doing an "empty trash",
and disks can't be ejected, what's more annoying since you must close some
applications before ejecting the disk (the trash may be left non-empty, the
disk may not be left mounted if you don't want to swap disks continuously).
But there are some problems.
  PBOpenWD does not create a WD if it already exists, but PBCloseWD closes it even 
if it existed before the PBOpenWD, hence opening and closing it I might close a WD 
which was already used and is still used by another part of the program. The 
ioWDProcID field was meant to solve that problem (in a non-elegant way), but under
MultiFinder that field loses its original meaning.
  TN 190 says that WDs created by standard file must NOT be closed, but it does
not explain why, on the contrary there is that very good reason to close them.
(Maybe "not closing them" is the official solution by Apple for any WD? Closing
them may cause errors (if you have another WD on the same dir, and you use it later), 
while never closing them causes only a few annoyaces))
  By experimenting I discovered that:
-) Under System 6 with MultiFinder, ioWDProcID is really ignored: if I open and
close a WD on a folder already opened by standard file, the closeWD closes the
WD created by standard file: hence, there should be no problem closing it anyway.
-) Under System 7, ioWDProcID is used in the pre-Multifinder way, and WDs created
by standard file are marked 'ERIK', there is a way (nondocumented ?) so that a
process identifier is stored together with ioWDProcID, or maybe multiple lists
are kept, so that different applications have a different set of WDs, in this
situation it could be more dangerous to violate Apple rules and close the WDs
created by standard file, but we've done that for a while without any problem.
(To be safe, suntar 1.2.2 does not do that, but 1.3 and later versions do).

 By now my solution is to remember the vrefnum and close it in the main command 
loop (when no folder is been visited and no file is closed but going to be opened again).
 However, SuperBoomerang 3 & 4 has some problems if I close a WD (even if I created
it by OpenWD ! Remember that it could return a previously existing one ). The result
is that in ANY case it's an error to close them and it's an error to leave them open.
 People at Apple are not totally stupid, a different solution
must exist: probably the only good solution would be NOT to create any WD but use PBH
calls to exploit directly the triple information vrefnum-dirID-filename. But then,
all standard file calls should be modified in order to avoid creating a WD 
(that is using the System 7 calls when it's possible, since simulating a Cancel
does not update the list of recently opened files kept by Boomerang) and
I would be forbidden to perform even one non-PBH routine, a large part of
the program should be carefully searched and modified... too much work.

  In the new technical note "Standard File Pkg Q&As" one reads:
"you don’t have to close these Standard File working directories yourself. If,
however, you want the user to be able to delete the directory *while* your
application is still running, you will have to issue a PBCloseWD() call yourself"
hence probably my choice is the right one, or at least it will be when
SuperBoomerang will be fixed

*/


void register_WD(n_WD)
short n_WD;
{
register short i=0;
/*printf("register %d\n",n_WD);*/
while(i<n_open_WD)
	if(n_WD==open_WD[i++]) return;
if(n_open_WD<max_open_WD) open_WD[n_open_WD++]=n_WD;
}

void close_all_open_WD()
{
extern SysEnvRec	gMac;
short i;
WDPBRec param;
if(!SB3_compatibility) 
	for(i=0;i<n_open_WD;i++){
		param.ioVRefNum=open_WD[i];
		/*printf("closeWD=%d\n",open_WD[i]);*/
		if(param.ioVRefNum!=gMac.sysVRefNum) PBCloseWDSync(&param);
			/* la WD sulla cartella sistema potrebbe servirmi per gestire il prefs file */
		}
n_open_WD=0;
}

short crea_WD(vol,dirID)
short vol;
long dirID;
{
short err;
WDPBRec param;
	param.ioVRefNum= vol;
	param.ioNamePtr=NULL;
	param.ioWDProcID=signature;
	param.ioWDDirID= dirID;
	if(err=PBOpenWDSync(&param)) error_message_1("Working directory error %d\n",err);
	return param.ioVRefNum;
}

