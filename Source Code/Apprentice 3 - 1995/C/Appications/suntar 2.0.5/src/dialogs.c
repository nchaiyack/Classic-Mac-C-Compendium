/*******************************************************************************\

dialogs module

part of suntar, �1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

#include "antiglue.h"
/* #include "system7.h" */
pascal OSErr FindFolder(short vRefNum,OSType folderType,Boolean createFolder,
    short *foundVRefNum,long *foundDirID)
    = {0x7000,0xA823};

#include "windows.h"
#include "suntar.h"

#include <string.h>

#define CONTROL_ON			1   /* Control value for on */
#define CONTROL_OFF			0   /* Control value for off */

#define ESC 27

#define TITLEBARHEIGHT 20

#define obSet		1
#define obSave		2
#define obCancel	3

#define ob800		 6
#define obAlias		 7
#define obCreaDir	 8
#define obModiDate	 10
#define obDelInc	 12
#define obTrunc14	 13
#define obUnderscore 14
#define obSupprShell 15
#define obVerify	 16
#define obSmallASCII 17
#define obLongPaths	 18
#define obPresAllChars 19
#define obSeparatePrefs 20

#define obFirstCheck	ob800
#define obLastCheck		(obFirstCheck+last_check_option-first_check_option)

#define obTextCr	21
#define obGIFCr		22
#define obMinBeep	23
#define obTarPopup	5
#define obTarPopupTitle (obTarPopup-1)
#define obWrASCPopup 24
#define obMoreOptions 27
#define obTimeZone	29

#define sdFind		4
#define sdExtract	2
#define sdCancel	3
#define sdScrollingList 1
#define sdStartSect 5
#define sdStSectTxt 4
#define sdScanSCSI	6

#define fdFind		1
#define fdCancel	2
#define fdCheck		3
#define fdSText		4
#define fdText		5

#define mouseClicksDisabled -1

#define sf_popup 9

extern char FINITO[],DONE[];
extern short size_ext_table,curr_sf_format,forza_formato,formato_encode;
extern Handle h_type,h_creat,h_extens,h_comments,h_ascii;

extern Boolean gHasSCSIManager;
extern Boolean ask_verbosity;
extern char which_popup;

void SetCheckBox(DialogPtr,short);
Boolean set_option_var(DialogPtr,short,char *);
void get_option_var(DialogPtr,short,Boolean);
void find_and_select(void);
void select_list_filter(EventRecord *);
void prepara_nome(char*,char*,long,short,short);
static void list_updater(EventRecord *);
static void list_activater(EventRecord *);
int parse_hour(Str255);
void compute_delta_time(void);
pascal Boolean modal_filter (DialogPtr,EventRecord *,short *);
pascal Boolean textedit_filter (DialogPtr,EventRecord *,short *);
Boolean replace_resource(Handle,Handle);
void fill_scsi_list(char*,short,Boolean);
void button_active(DialogPtr,short);
void button_inactive(DialogPtr,short);
void hilite_name_remapping(DialogPtr);
Boolean must_be_reloaded(short);
void load_STR_prefs_except_bufsizes(void);
short crea_WD(short,long);
pascal Size memory_overflow(Size);
Boolean this_file_is_writable(short);
void adjust_mode_select(void);

static short list_button_code;
static short max_paste_size;
DialogPtr ListDialog=NULL;
ListHandle LHandle;
static Rect cornice;
static short list_width;
short last_list_cell;
static Boolean ci_stanno_certo;
char causale_list_dialog;
static char find_options=0;
short stored_vrefnums;
static short vrn_list[15],idx_list[15];
extern short default_type,default_device_num,def_cambiato;


static Boolean semimodal_list,list_with_textedit;
#define at_start 1
#define remove_extras 2

static char msg_annulla[]="\pAnnulla";

#define n_elems_of(x)	(sizeof(x)/sizeof((x)[0]))
extern Boolean gHasFindFolder;
/*#define gHasFindFolder gHasResolveAlias	it's the same trap, different selector,
but anyway since there is a Gestalt for it it's better doing the Gestalt */


void my_alert()
/* a centered alert, with an "OK" button */
{
AlertTHndl	alertHandle;
SignedByte oldflags;

flush_all();
check_foreground();
alertHandle = (AlertTHndl)GetResource('ALRT',ErrorAlert);
oldflags=HGetState(alertHandle);
HNoPurge((Handle)alertHandle);
PositionDialog( &((**alertHandle).boundsRect));
SetCursor(&arrow);
Alert(ErrorAlert, modal_filter);	/* we've tried to use NoteAlert &Co, but
	the icon has the same feature of the button outline, it disappears
	when a screen saver blanks the screen (even without our event filter):
	to avoid that, one must add an icon item to the DITL, and not to use
	NoteAlert at all */
HSetState(alertHandle,oldflags);
}

short my_edt_dialog(dialog_id,buffer,maxlen,initial)
short dialog_id;
unsigned char *buffer;
short maxlen;		/* max len of string. Beware, the output buffer may
					temporarly hold strings longer than this */
const unsigned char *initial;
/* a simple dialog with an editable text */
{
	AlertTHndl	alertHandle;
	SignedByte oldflags;
	DialogPtr myDialog;
	short item;

	flush_all();
	check_foreground();
	SetCursor(&arrow);

	alertHandle = (AlertTHndl)GetResource('DLOG',dialog_id);
	if (alertHandle) {
		oldflags=HGetState(alertHandle);
		HNoPurge((Handle)alertHandle);
		PositionDialog( &((**alertHandle).boundsRect));
		myDialog=GetNewDialog(dialog_id,NULL,(WindowPtr)-1L);
		HSetState(alertHandle,oldflags);

		if(in_Italia&&dialog_id!=150) set_item_text(myDialog,2,msg_annulla);
		set_item_text(myDialog,3,initial);
		SelIText(myDialog,3,0,32767);
		ShowWindow(myDialog);

		max_paste_size=100;
		ModalDialog(textedit_filter,&item);
		if(item!=2){
			get_item_text(myDialog,3,buffer);
			if(buffer[0]>maxlen) buffer[0]=maxlen;
			}

		DisposDialog(myDialog);
		return item;
		}
	return 0;
}


short my_modal_dialog(dialog_ID,titoli_italiani,n_titoli)
/* A centered modal dialog. It may display button titles in both
English and Italian
*/
short dialog_ID,n_titoli;
unsigned char **titoli_italiani;
{
	DialogPtr myDialog;
	short item;
	AlertTHndl	alertHandle;
	short	kind;
	Handle	h;
	Rect	r;
	SignedByte oldflags;

	flush_all();
	check_foreground();
	SetCursor(&arrow);

	alertHandle = (AlertTHndl)GetResource('DLOG',dialog_ID);
	if (alertHandle) {
		oldflags=HGetState(alertHandle);
		HNoPurge((Handle)alertHandle);
		PositionDialog( &((**alertHandle).boundsRect));
		myDialog=GetNewDialog(dialog_ID,NULL,(WindowPtr)-1L);
		HSetState(alertHandle,oldflags);
		if(in_Italia && titoli_italiani){
			short fatti=0;
			for(item=1;fatti<n_titoli;item++){
				GetDItem(myDialog,item,&kind,&h,&r);
				if((kind&0x7F)==ctrlItem+btnCtrl){
					SetCTitle(h,*titoli_italiani++);
					fatti++;
					}
				}
			}
		ShowWindow(myDialog);
		ModalDialog(modal_filter,&item);

		DisposDialog(myDialog);
		return item;
		}
	return 0;
}

short my_semimodal_dialog(dialog_ID,titoli_italiani,n_titoli,p1,p2,p3)
/* A semimodal dialog with almost the same calling interface than the
previous function (the differences: it does not use ParamText,
and the item containing the text must be a userItem, not static text)
(Otherwise, I might use ParamText and copy the strings pointed by
the four low memory global variables DAStrings, at 0xAA0)
*/
short dialog_ID,n_titoli;
char **titoli_italiani;
Str255 p1,p2,p3;
{
return semimodalDialog(dialog_ID,NULL,NULL,titoli_italiani,n_titoli,p1,p2,p3,
	teJustLeft,true,NULL);
}


/**********************************************************/


static pascal Boolean modal_filter (theDialog,theEvent,itemHit)
DialogPtr theDialog;
register EventRecord *theEvent;
short *itemHit;
{
/* unfortunately some events are simply ignored during some Toolbox calls:
the event loop contained in the Alert or ModalDialog system routine can't know 
how to handle update an activate routines for user windows; furthermore, even the 
outline to the default button is not redrawn: if, for example, a screen saver 
blanks the screen, the outline is lost because ModalDialog updates only what it 
knows. Hence, I had to create an event filter for all modal dialogs, including
standard file dialogs and Alert.
Really, the best thing would be if Apple allowed to use the windowPic field in the window
record in a different way: place there a pointer to an application-defined routine
which handles updates and activates and is called implicitly by GetNextEvent, which 
then will return false (so solving the problems for such events) and furthermore
if a button could have a standard attribute "outlined" so that, DrawDialog could
do the right job (that second thing seems to have been done, see TN 304).
Another thing that GetNextEvent should do is calling FindWindow for mouseDown events,
so that you get the information about which window was clicked even if that window
was closed between the time of the click and the moment you get the event (for example,
suntar 1.0 closed the about box when the Button function returned true, but the 
mouseDown event remained in the event queue... In suntar 1.2, the same problem 
has remained for the "going to background" dialog)
*/
short	kind;
Handle	h;
Rect	r;
switch(theEvent->what){
case keyDown:
case autoKey:
	if((unsigned char)theEvent->message==CR || (unsigned char)theEvent->message==enter_key){
		GetDItem(theDialog,1,&kind,&h,&r);	/* the default button in my dialogs is
					always item 1: one may use a global variable to change the
					routine if that's not the case */
		if( ((ControlRecord*)*h)->contrlHilite==0){
			SelectButton(h);
			*itemHit=1;
			return true;
			}
		else
			theEvent->what=nullEvent;
		}
	break;
case updateEvt:
	if((DialogPtr)theEvent->message==theDialog){
		/* ModalDialog would do the update, but without OutlineControl... 
		hence, I handle the event directly here */
		WindowPtr w=NULL;
		short item;

		DialogSelect(theEvent,&w,&item);

		GetDItem(theDialog,1,&kind,&h,&r);
		OutlineControl(h);
		theEvent->what=nullEvent;
		}
	else
		UpdateFilter(theEvent);
	break;
case activateEvt:
	UpdateFilter(theEvent);
	break;
/* luckily, ModalDialog masks out disk insertions, so that they are received
only after the dialog is closed: otherwise, I would not know what to
do in that case... */
}

return false;
}

/* constants for positioning the default item within its box */
#define leftSlop	13			/* leave this much space on left of title */
#define rightSlop	5			/* this much on right */
#define botSlop		5			/* this much below baseline */

extern MenuHandle popupMenu[];
static short curr_tarpopup,curr_ntApopup;

pascal void redrawPopup(WindowPtr,short);
pascal void redrawPopup(theWindow,itemNo)
WindowPtr theWindow;
short itemNo;
{
/* inspired to Apple's Sample Code 006, but adding the triangle */
	short	kind;
	Handle	h;
	Rect	r;
	char	title[32];
	MenuHandle menu_h;
	short curr_item;
	PolyHandle triangle;
	#define triang_top	-12
	#define triang_left  -18
GetDItem(theWindow,itemNo,&kind,&h,&r);

/* OK, OK, that's not good programming style. Since I have very few dialogs
containing a popup and since all popups have different item numbers, I'm using
the item number to know which menu must be shown and which global variable
must represent internally the currently selected item.
No more true since now get_file_with_popup may use two different popups. So,
(bad programming style) only in that case I have a variable which_popup.
*/

if(itemNo==obTarPopup){
	menu_h=popupMenu[0];
	curr_item=curr_tarpopup;
	}
else if(itemNo==obWrASCPopup){
	menu_h=popupMenu[1];
	curr_item=curr_ntApopup;
	}
else if(itemNo==9){	/* sf Put */
	if(!which_popup){
		menu_h=popupMenu[2];
		curr_item=curr_sf_format;
		}
	else{
		menu_h=popupMenu[4];
		curr_item=formato_encode;
		}
	}
else{	/* sf Get */
	menu_h=popupMenu[3];
	curr_item=forza_formato;
	}

GetItem (menu_h,curr_item,&title);
InsetRect(&r,-1,-1);		/* make it a little bigger */

/* draw the box and its drop shadow */
FrameRect(&r);
MoveTo(r.right,r.top+2); LineTo(r.right,r.bottom);
LineTo(r.left+2,r.bottom);
/* draw the string */
MoveTo(r.left+leftSlop,r.bottom-botSlop);
DrawString(title);

/* draw the triangle (according to the Guidelines of IM Vol 6) */
triangle=OpenPoly();
MoveTo(r.right+triang_left,r.bottom+triang_top);
LineTo(r.right+triang_left+12, r.bottom+triang_top);
LineTo(r.right+triang_left+6,r.bottom+triang_top+6);
ClosePoly();
FillPoly(triangle,black);
KillPoly(triangle);
}


void do_popup_selection(DialogPtr,short);
void do_popup_selection(theDialog,my_item)
DialogPtr theDialog;
short my_item;
{
/* again, inspired to Apple's Sample Code 006 */
short	kind;
Handle	h;
Rect	r,promptBox;
Point popLoc;
long chosen;
MenuHandle menu_h;
short curr_item;

if(my_item==obTarPopup){
	GetDItem(theDialog,obTarPopupTitle,&kind,&h,&promptBox);
	InvertRect(&promptBox);
	menu_h=popupMenu[0];
	curr_item= curr_tarpopup;
	}
else if(my_item==obWrASCPopup){
	menu_h=popupMenu[1];
	curr_item=curr_ntApopup;
	}
else if(my_item==9){	/* SF Put */
	GetDItem(theDialog,sf_popup+1,&kind,&h,&promptBox);
	InvertRect(&promptBox);
	if(!which_popup){
		menu_h=popupMenu[2];
		curr_item=curr_sf_format;
		}
	else{
		menu_h=popupMenu[4];
		curr_item=formato_encode;
		}
	}
else{	/* item 11, SF Get */
	GetDItem(theDialog,11+1,&kind,&h,&promptBox);
	InvertRect(&promptBox);
	menu_h=popupMenu[3];
	curr_item=forza_formato;
	}

InsertMenu(menu_h,-1);
GetDItem(theDialog,my_item,&kind,&h,&r);
popLoc = *(Point*)&r.top;
LocalToGlobal(&popLoc);
CalcMenuSize(menu_h); /*Work around Menu Mgr bug*/
EraseRect(&r);
chosen = PopUpMenuSelect(menu_h, popLoc.v, popLoc.h, curr_item);
InvalRect(&r);
if(my_item==obTarPopup){
	InvertRect(&promptBox);
	DeleteMenu(tarPopupID);
	if(chosen) curr_tarpopup = loword(chosen);
	GetDItem(theDialog,obLongPaths,&kind,&h,&r);
	HiliteControl(h,(curr_tarpopup==tar_singlevol+1 ||
		curr_tarpopup==tar_POSIX+1 || curr_tarpopup==tar_GNU+1 )?0:255);
	}
else if(my_item==obWrASCPopup){
	DeleteMenu(ntAPopupID);
	if(chosen) curr_ntApopup = loword(chosen);
	}
else if(my_item==9){
	InvertRect(&promptBox);
	if(!which_popup){
		DeleteMenu(148);
		if(chosen) curr_sf_format = loword(chosen);
		}
	else{
		DeleteMenu(150);
		if(chosen) formato_encode = loword(chosen);
		}
	}
else{	/* 11 */
	InvertRect(&promptBox);
	DeleteMenu(149);
	if(chosen) forza_formato = loword(chosen);
	}
}


static pascal Boolean textedit_filter (theDialog,theEvent,itemHit)
DialogPtr theDialog;
register EventRecord *theEvent;
short *itemHit;
{
if(theEvent->what==keyDown && (char)theEvent->message==ESC)
	theEvent->what=nullEvent;	/* the dialog has no equivalent for Cancel,
								if I don't remove it it will go in the text
								(TextEdit does not know that ESC has a special
								meaning) */
else if(theEvent->what==keyDown && (theEvent->modifiers & cmdKey) ){
	/* handle the edit commands in the dialog... */
	/*printf("questo l'ho fatto io !\n");*/
	if(((char)theEvent->message|0x20)=='c'){
		DlgCopy (theDialog);
		theEvent->what=nullEvent;
		}
	else if(((char)theEvent->message|0x20)=='x'){
		DlgCut (theDialog);
		theEvent->what=nullEvent;
		}
	if(((char)theEvent->message|0x20)=='v'){
		if( TEGetScrapLen() <= max_paste_size)
			DlgPaste (theDialog);
		else
			SysBeep(5);		/* don't paste a very long string to a very small field */
					/* unfortunately, some widely used INITs believe that any application
					does not handle cut&paste in dialogs, so they cheat everybody by
					handling that themselves, and this protection is bypassed. The
					problem is that the filter/hook method is almost object-oriented
					but is not truly object-oriented, so that there is not an easy way
					to say "if nobody installed an handler for this, then..."
					*/
		theEvent->what=nullEvent;
		}
	return false;
	}
else
	return modal_filter(theDialog,theEvent,itemHit);
}


pascal Boolean options_filter (DialogPtr,EventRecord *,short *);
static pascal Boolean options_filter (theDialog,theEvent,itemHit)
DialogPtr theDialog;
register EventRecord *theEvent;
short *itemHit;
{
short my_item;

if(theEvent->what==mouseDown){
	GrafPtr	savePort;
	Point mouseLoc= theEvent->where;
	GetPort(&savePort);
	SetPort(theDialog);
	GlobalToLocal(&mouseLoc);

	my_item=FindDItem(theDialog, mouseLoc)+1;
	
	if ( my_item== obTarPopup || my_item==obWrASCPopup){
		do_popup_selection(theDialog,my_item);
		theEvent->what=nullEvent;
		}
	SetPort(savePort);
	return false;
	}
else{
	max_paste_size=8;
	return textedit_filter(theDialog,theEvent,itemHit);
	}
}

struct more_options{
	Handle h_tp,h_cr,h_ex,h_co,h_as;
	};
void dialogo_tipi_creatori(struct more_options*);

void options_box()
{
short i,currentRadio;
Str255 buffer;
/*EventRecord myEvent;*/
DialogPtr myDialog;
short		kind;
Handle		h;
Rect		r;
short itemHit;
AlertTHndl	alertHandle;
SignedByte oldflags;
struct more_options more={NULL,NULL,NULL,NULL};

SetCursor(&arrow);

if (alertHandle = GetResource('DLOG',128)) {
	oldflags=HGetState(alertHandle);
	HNoPurge((Handle)alertHandle);
	PositionDialog( &((**alertHandle).boundsRect));
	myDialog = GetNewDialog (128,NULL,(char*)-1);
	HSetState(alertHandle,oldflags);

	if(non_text_ASCII >3 ) non_text_ASCII=0;
	#ifdef V_122
	currentRadio= obFirstRadio+non_text_ASCII;
	SetRadioButton(myDialog,currentRadio,CONTROL_ON);
	#endif

	buffer[0]=4;
	mcopy(&buffer[1],&text_creator,4);
	set_item_text(myDialog,obTextCr,buffer);

	/*buffer[0]=4;*/
	mcopy(&buffer[1],&gif_creator,4);
	set_item_text(myDialog,obGIFCr,buffer);

	/*my_itoa((long)delay_back,buffer);
	my_c2pstr(buffer);
	set_item_text(myDialog,obDelay,buffer);*/

	my_itoa((long)min_to_beep,buffer);
	my_c2pstr(buffer);
	set_item_text(myDialog,obMinBeep,buffer);


	my_itoa((long)(options.fuso_orario/60),buffer);
	i=options.fuso_orario;
	if(i<0) i=-i;
	i%=60;
	if(i){
		strcat(buffer,".");
		my_itoa((long)i,buffer+strlen(buffer));
		}
	my_c2pstr(buffer);
	set_item_text(myDialog,obTimeZone,buffer);

	for(i=first_check_option;i<=last_check_option;i++){
		if(i!=skip_check_option1 && i!=skip_check_option2)
			get_option_var(myDialog,obFirstCheck-first_check_option+i,options.opt_bytes[i]);
		}
	hilite_name_remapping(myDialog);
	{extern short appl_frefn;
	if(fase!=non_faccio_nulla || !this_file_is_writable(appl_frefn))
		button_inactive(myDialog,obSeparatePrefs);	/* it's not a button, but the routine*/
	}												/*accepts any control */

	GetDItem(myDialog,obTarPopup,&kind,&h,&r);
	SetDItem (myDialog,obTarPopup, kind, (Handle)redrawPopup, &r);
	curr_tarpopup=tar_version+1;
	GetDItem(myDialog,obWrASCPopup,&kind,&h,&r);
	SetDItem (myDialog,obWrASCPopup, kind, (Handle)redrawPopup, &r);
	curr_ntApopup=non_text_ASCII+1;
	ShowWindow(myDialog);

	GetDItem(myDialog,obLongPaths,&kind,&h,&r);
	HiliteControl(h,(tar_version==tar_singlevol || tar_version==tar_POSIX || tar_version==tar_GNU)?0:255);
	do {
		ModalDialog(options_filter,&itemHit);
		if(itemHit>=obFirstCheck && itemHit <= obLastCheck){
			SetCheckBox(myDialog,itemHit);
			if(itemHit==obPresAllChars)
				hilite_name_remapping(myDialog);
			}

		#ifdef V_122
		else if(itemHit>=obFirstRadio && itemHit <= obLastRadio){
			SetRadioButton(myDialog,currentRadio,CONTROL_OFF);
			SetRadioButton(myDialog,(currentRadio=itemHit),CONTROL_ON);
			}
		#endif
		else if(itemHit==obMoreOptions){
			/*HideWindow(myDialog); NO: if I do that, it may happen that when
				the second window is closed, this windows becomes the front window
				even if it's still invisible => crash !
			*/
			dialogo_tipi_creatori(&more);
			/*ShowWindow(myDialog);*/
			}
		}
	while (itemHit != obSave && itemHit != obCancel && itemHit != obSet );

	if( itemHit != obCancel){
		Boolean changed=false;	/* I do NOT like those programs which create
								a preferences file even when I've never changed
								its settings which is still equal to the defaults */
		get_item_text(myDialog,obTextCr,buffer);
		for(i=buffer[0];i<=4;)
			buffer[++i]=' ';
		if(!compare_mem(&text_creator,&buffer[1],4)){
			changed=true;
			mcopy(&text_creator,&buffer[1],4);
			}

		get_item_text(myDialog,obGIFCr,buffer);
		for(i=buffer[0];i<=4;)
			buffer[++i]=' ';
		if(!compare_mem(&gif_creator,&buffer[1],4)){
			changed=true;
			mcopy(&gif_creator,&buffer[1],4);
			}

		get_item_text(myDialog,obMinBeep,buffer);
		i= pstrtoi(buffer);
		if(i>0 && i!=min_to_beep){
			changed=true;
			min_to_beep = i;
			}

		get_item_text(myDialog,obTimeZone,buffer);
		i=parse_hour(buffer);
		if(i!=options.fuso_orario){
			options.fuso_orario=i;
			compute_delta_time();
			changed=true;
			}

		for(i=first_check_option;i<=last_check_option;i++){
			if(i!=skip_check_option1 && i!=skip_check_option2 && (i!=obSeparatePrefs-obFirstCheck+1||itemHit==obSave) )
				changed |= set_option_var(myDialog,obFirstCheck-first_check_option+i,&options.opt_bytes[i]);
			}

		/*non_text_ASCII = currentRadio - obFirstRadio;*/
		changed |= (tar_version!=curr_tarpopup-1) || (non_text_ASCII!=curr_ntApopup-1);
		tar_version=curr_tarpopup-1;
		non_text_ASCII=curr_ntApopup-1;
		if(more.h_tp!=NULL){
			changed |= replace_resource(h_type,more.h_tp);
			changed |= replace_resource(h_creat,more.h_cr);
			changed |= replace_resource(h_extens,more.h_ex);
			changed |= replace_resource(h_comments,more.h_co);
			changed |= replace_resource(h_ascii,more.h_as);
			}

		if(itemHit==obSave && changed){
			save_options(h_type,h_creat,h_extens,h_comments,h_ascii,NULL);
			}
		}
	else if(more.h_tp!=NULL){
		DisposHandle(more.h_tp);
		DisposHandle(more.h_cr);
		DisposHandle(more.h_ex);
		DisposHandle(more.h_co);
		DisposHandle(more.h_as);
		}

	DisposDialog(myDialog);
	}
}

static void hilite_name_remapping(dialog)
DialogPtr dialog;
{
	short	kind;
	Handle	h;
	Rect	r;
	short newstate;
GetDItem(dialog,obPresAllChars,&kind,&h,&r);
newstate=!GetCtlValue(h)?0:255;
GetDItem(dialog,obUnderscore,&kind,&h,&r);
HiliteControl(h,newstate);
GetDItem(dialog,obSupprShell,&kind,&h,&r);
HiliteControl(h,newstate);
}


static char*help_string;

pascal void redrawList(WindowPtr,short);
static pascal void redrawList(theWindow,itemNo)
WindowPtr theWindow;
short itemNo;
{
Rect r;
	LUpdate(((WindowPtr)ListDialog)->visRgn,LHandle);
	FrameRect (&cornice);
	r.top=cornice.bottom+2;
	r.bottom=r.top+16;
	r.left=cornice.left;
	r.right=cornice.right+100;
	TextFont(3);
	TextSize(9);

	TextBox(help_string,(long)strlen(help_string),&r,teJustLeft);
	TextFont(0);	/* restore the system font, Chicago 12 */
	TextSize(12);
	if(list_with_textedit){	/* ce ne � solo uno e ha un bottone di default */
		Handle h;
		Rect r;
		short kind;
		GetDItem(ListDialog,sdExtract,&kind,&h,&r);
		OutlineControl(h);
		}

}


void new_list_dialog(dlg_id,behind,drawit)
short dlg_id;
WindowPtr behind;
Boolean drawit;
{
	GrafPtr savePort;
	short i;
	Rect rView;
	static Point nullPoint={0,0};
	AlertTHndl	alertHandle;
	SignedByte oldflags;

	help_string=in_Italia?"usa command-clic o shift-clic per estendere/ridurre la selezione":
		"use command-click or shift-click to extend/shrink the selection";
		/* devo assegnare qui e non in updateList perch� non posso cambiare stringa
		mentre � visualizzata (a meno di cancellare tutto, ovvero fare un invalidRect) */
	if(dlg_id==152) help_string="";
	semimodal_list=false;
	list_with_textedit=false;

	alertHandle = (AlertTHndl)GetResource('DLOG',dlg_id);
	/*if(alertHandle==NULL) return; o ExitToShell, � un errore gravissimo */
	oldflags=HGetState(alertHandle);
	HNoPurge((Handle)alertHandle);
	(**alertHandle).boundsRect.top-=TITLEBARHEIGHT;
	PositionDialog( &((**alertHandle).boundsRect));
	(**alertHandle).boundsRect.top+=TITLEBARHEIGHT;

	ListDialog=GetNewDialog(dlg_id,NULL,behind);
	GetPort( &savePort );
	SetPort(ListDialog);
	HSetState(alertHandle,oldflags);

	if(dlg_id==131)
		SetWTitle (ListDialog,in_Italia?"\pSelezione files":"\pFile selection");
	else if (dlg_id==148)
		SetWTitle (ListDialog,in_Italia?"\pSelezione	files":"\pFile	selection");
		/* these names contain tabs, on the screen they look like
		spaces, but the Help Manager which uses the window title to choose the
		balloons considers them different...
		(OK, I do not like the way the Help manager is done. I wish I could
		create a resource with the same ID as the DLOG, as I do for menus,
		and not to do anything else: I do that but then I must explicitly
		connect the hdlg to the dialog thorough the title, but the title is
		NOT a good way to recognize a window, and using the windowKind
		field should work but does not (the dialog manager dos not handle
		correctly a window whose windowKind is not 2�). Furthermore,
		connecting both a hrct and a hdlg to the same dialog should work
		but does not)  */
	else if (dlg_id==152)
		SetWTitle (ListDialog,in_Italia?"\pScelta device":"\pDevice selection");

	install_handlers(ListDialog,list_updater,list_activater);

/*printf("windowKind=%d\n",((WindowPeek)ListDialog)->windowKind); 2!*/
	((WindowPeek)ListDialog)->refCon= -2;

	{
		static char *titoli[]={"\pEstrai",msg_annulla,"\pTrova�"};
		static char *titoli_dev[]={"\pApri",msg_annulla};
		short	kind;
		Handle	h;
		Rect	r;

		for(i=sdExtract;i<=(dlg_id==152?sdCancel:sdFind);i++){
			GetDItem(ListDialog,i,&kind,&h,&r);
			if(dlg_id==152){
				if(in_Italia) SetCTitle(h,titoli_dev[i-sdExtract]);
				if(i==sdExtract) HiliteControl(h,255);
				}
			else{
				if(in_Italia) SetCTitle(h,(i!=sdExtract||dlg_id==131)?titoli[i-sdExtract]:"\pScrivi");
				if(drawit) HiliteControl(h, 255);
				}
			}

		if(in_Italia&&dlg_id==152){
			GetDItem(ListDialog,sdStSectTxt,&kind,&h,&rView);
			SetIText(h,"\pSettore\riniziale");
			}

		GetDItem(ListDialog,sdScrollingList,&kind,&h,&rView);
		SetDItem (ListDialog,sdScrollingList, kind, (Handle)redrawList, &rView);
	}

	/*TextFont(3);*/
	{FontInfo	fInfo;
	GetFontInfo(&fInfo);
	i=fInfo.ascent+fInfo.descent+fInfo.leading;
	}
	rView.bottom= rView.top+ ((rView.bottom-rView.top)/i) * i;
	cornice=rView;
	InsetRect(&cornice,-1,-1);

	rView.right -= 15;
	list_width=rView.right-rView.left;
	ci_stanno_certo= list_width/CharWidth('m');

	/*PenSize(1,1);*/
	FrameRect (&cornice);

	{Rect dataBounds;
	dataBounds.top = 0;
	dataBounds.left = 0;
	dataBounds.bottom = 0;
	dataBounds.right = 1;
	LHandle = LNew(&rView,	/* position in window */
				&dataBounds,/* initial size of array */
				nullPoint,			/* cell size (0 = default) */
				0,			/* resource ID of LDEF */
				ListDialog,	/* window pointer */
				drawit,		/* drawit */
				false,		/* has grow */
				false,		/* scrollHoriz */
				true);		/* scrollVert */
	}
	(**LHandle).selFlags = /*lNoExtend |*/ lNoRect | lUseSense | lExtendDrag | lNoNilHilite;
	if(dlg_id==152) (**LHandle).selFlags = lOnlyOne | lNoNilHilite;

	if(drawit){
		ShowWindow(ListDialog);
		/*DrawDialog(ListDialog);*/
		}

	SetPort( savePort );
	last_list_cell=0;
}

void append_to_list(buffer,size)
char*buffer;
short size;
{
Point myCell;
#if 0
	GrafPtr savePort;
	GetPort( &savePort );
	SetPort(ListDialog);
#endif
	last_list_cell=LAddRow(1,last_list_cell+1,LHandle);
	myCell.v=last_list_cell;
	myCell.h=0;
	LSetCell(buffer, size, myCell, LHandle);
#if 0
	SetPort( savePort );
#endif
}

void dispose_list_dialog()
{
	if(ListDialog){
		if(LHandle!=0) LDispose(LHandle);
		LHandle=NULL;
		DisposDialog(ListDialog);
		remove_handlers(ListDialog);
		ListDialog=NULL;
		my_event_filter=NULL;
		}
}

void hilite_list_buttons(short);
void hilite_list_buttons(state)
short state;
{
short i;
for(i=sdExtract;i<=sdFind;i++){
	short		kind;
	Handle		h;
	Rect		r;
	GetDItem(ListDialog,i,&kind,&h,&r);
	HiliteControl(h, state);
	}

}

/******************************************************/

void select_and_extract()
{
/* this routine was recently broken into pieces, but it still uses the List manager
and a few static variables too often and can't be placed in another source file */

long length;
Boolean more_files;
char buffer[150];
Point myCell;
sector_t last_header;
sector_t ** sectorList;
Size sectListSize;
SignedByte oldflags;
static char s1[]="Nessun file parte qui\n", s2[]="No file starts in this volume\n";

ListDialog=NULL;
LHandle=NULL;
sectListSize=0;
if(setjmp(main_loop)<0) {
	dispose_list_dialog();
	if(sectListSize) DisposHandle(sectorList);
	return;
	}

if(aspetta_inserzione(in_Italia?"\pInserisci il disco in formato UNIX":
	"\pInsert the disk in UNIX format",0))	return;
if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized) return;

fase=reading_disk;

if(di.tipo_device==d_scsi&&di.v.si.DeviceType==TAPE_UNIT)
	adjust_mode_select();

{
enum formats fmt;
previousFormat=tar_unknown;
fmt=identify_format();

if(fmt==tar_format){
	bar_archive=false;
	last_header=0;
	if(di.tipo_device==d_scsi || di.tipo_device==d_driver&&di.v.fi.opened_by_insertion!=1)
		last_header=di.archive_start_sector;
	}
else if(fmt==bar_format){
	bar_archive=true;
	floppy_n=untar_dec_number( ((barh_type*)disk_buffer)->volume_num,true);
	last_header= di.archive_start_sector + (untar_number( ((barh_type*)disk_buffer)->size,true)+1023)>>9;
	}
else{
	if(di.v.fi.opened_by_insertion==1) diskEject(true);
	return;		/* i messaggi sono gi� comparsi... */
	}
}

if(last_header>=di.sectors_on_floppy){		/* only for bar... */
	printf(in_Italia ? s1:s2);
	return;
	}

new_list_dialog(131,(WindowPtr)-1L,true);
SetPort(ListDialog);
causale_list_dialog=-1;

for(;;){		/* loop on disks, for POSIX only */
	extern Boolean disco_espulso;
	short files_on_floppy=0;
	sector_t longname_header;

	my_event_filter=select_list_filter;
	disco_espulso=false;
	listonly=1;
	reinit_full_name();
	while(last_header<di.sectors_on_floppy){
		unsigned char linkflag;
		/* printf ("leggo %ld\n",(long)last_header); */
		check_events(false);
		leggi_settore(last_header,disk_buffer);
		if(check_error()) raise_error();

		sect_n=last_header;

		if(GetHandleSize((*LHandle)->cells)>=32700){
			printf("List too long, it was truncated\n");
			break;
			}

		if(check_all_zero(disk_buffer)){
			if(last_header==0||(bar_archive||hasVheader)&&last_header==1)
				printf(in_Italia?"Archivio vuoto\n":"Empty archive\n");
			break;
			}

		linkflag=get_linkflag(&length,true);
		if(linkflag=='L'||linkflag=='K'){
			previousFormat=tar_GNU;
			if(linkflag=='L'){
				void untar_long_name(void);
				longname_header=last_header;
				if(last_header<=di.sectors_on_floppy-3){/* cio�, estrarlo non comporterebbe eiettare */
					end_of_file();
					sect_n=last_header+1;
					mcopy(&tarh,disk_buffer,512);
					untar_long_name();
					end_of_file();
					}
				}
			}
		else if( linkflag=='\0'||linkflag=='0'||linkflag=='7' ){
			/*print_info(bar_archive ? ((barh_type*)disk_buffer)->name :
				((tarh_type*)disk_buffer)->name,length);*/
			if(!sectListSize)
				sectorList= NewHandle(sectListSize=sizeof(sector_t));
			else{
				SetHandleSize (sectorList, sectListSize+=sizeof(sector_t) );
				if(MemError()!=noErr)
					memory_overflow(1);
				}

			if(!bar_archive && full_name){
				*(sector_t*)(((char*)(*sectorList))+sectListSize-sizeof(sector_t)) = longname_header;
				prepara_nome(full_name, buffer, length, ci_stanno_certo, list_width);
				}
			else{
			/* it should be:
				(*sectorList)[sectListSize/sizeof(sector_t)-1] = last_header;
				but optimizing:
				*/
				*(sector_t*)(((char*)(*sectorList))+sectListSize-sizeof(sector_t)) = last_header;
				prepara_nome(
					bar_archive ? ((barh_type*)disk_buffer)->name :
					((tarh_type*)disk_buffer)->name,
					buffer, length, ci_stanno_certo, list_width);
					/* usare il campo name va ancora bene per nomi lunghi POSIX, tanto
					per farli stare nella finestra taglierei proprio quel pezzo */
				}

			append_to_list(buffer, strlen(buffer));
			files_on_floppy++;
			reinit_full_name();
			}
		else
			reinit_full_name();

		last_header += (length+1023)/512;
		}	/* end while */

	if(files_on_floppy==0){
		if(last_header>=di.sectors_on_floppy)
			printf(in_Italia?s1:s2);
		else
			printf(in_Italia?"Fine archivio\n":"End of archive\n");

		if(previousFormat==tar_GNU||previousFormat!=tar_POSIX&&tar_version==tar_GNU||
		   last_header<di.sectors_on_floppy){
			if(di.v.fi.opened_by_insertion==1) diskEject(true);	/* ho finito */
			break;
			}
		}

	append_to_list(buffer, 0);	/* an empty cell, at the only purpose to
		be able to click on it and deselect all other items: I hate not to
		be able to return to that initial state ! */
	/*LDoDraw(true,LHandle); /* useful only if LNew had drawit = false */

	hilite_list_buttons(0);	/* riabilito i bottoni...*/
	fase=ricevuto_comando;

	list_button_code=0;
	while(!list_button_code){
		if(files_on_floppy!=0)
			MainEvent();
		else
			list_button_code=sdExtract;

		if(list_button_code==sdFind){
			find_options|=remove_extras;
			find_and_select();
			list_button_code=0;
			}
		else if(list_button_code==sdExtract){
			myCell.h=0;
			myCell.v=0;
			more_files=LGetSelect(true,&myCell,LHandle);
			if(myCell.v==last_list_cell)	/* � la cella vuota in fondo
								--the empty cell */
				more_files=false;
			if(more_files)
				select_directory();
			else
				reply.good=true;
			if( !reply.good || !more_files&&(tar_version!=tar_POSIX||previousFormat==tar_GNU))
				list_button_code=0;
			}
		accept_abort_command();
		}

	if(list_button_code==sdCancel)
		break;
	if(list_button_code==sdExtract){	/* se sono qui, ho gi� selezionato il folder
				e ho gi� trovato il primo item selezionato
				-- note that some operations handling that event have been
				performed inside the event loop above */
		hilite_list_buttons(255);	/* disabilito i bottoni...*/
		/* avverti il filtro di non accettare click sulla lista:
		-- disable the handling of the list by the event filter
		*/
		list_button_code = mouseClicksDisabled;
		listonly=0;
		fase=selected_reading;
		if(archive_rewind()){
			dispose_list_dialog();
			if(sectListSize) DisposHandle(sectorList);
			return;
			}

		while(more_files){
/* il get va fatto prima, sia per non chiedere destinazione se nessuno � selezionato,
sia per togliere la finestra prima di iniziare l'ultima estrazione
-- find next file selected before extracting the current one: there are a couple of 
reasons to do that before rather than after
*/
			Point curr_cell;
			/*
			i=sizeof(buffer);
			LGetCell(buffer,&i,myCell,LHandle);
			buffer[i]=0;
			printf("%d:%s [%ld]\n",myCell.v,buffer,(long)(*sectorList)[myCell.v]);
			*/
			sect_n = (*sectorList)[myCell.v];

			curr_cell = myCell;
			myCell.v++;
			more_files=LGetSelect(true,&myCell,LHandle);
			if(myCell.v==last_list_cell) more_files=false;
			if(!more_files&& (tar_version!=tar_POSIX || previousFormat==tar_GNU || last_header<di.sectors_on_floppy) ){
				/* tolgo il dialogo subito, per non rischiare di fare confusione se 
					dovesse saltar fuori anche il dialogo di disk insert
					-- remove the dialog before starting extracting the last file,
					that reduces the confusion in case the disk insertion dialog appears...
					*/
				dispose_list_dialog();
				}

			leggi_settore(sect_n,&disk_buffer);
			if(check_error()) raise_error();

			listonly=0;
			if(bar_archive)
				unbar();
			else{
				reinit_full_name();
				untar();
				if(full_name){
					leggi_settore(sect_n,disk_buffer);
					if(check_error()) raise_error();
					untar();
					}
				}
			if(ListDialog){		/* deseleziono il nome del file appena salvato...
							-- deselect the file name */
				LSetSelect(false,curr_cell,LHandle);
				LAutoScroll(LHandle);
				}
			}
		if(tar_version==tar_POSIX && last_header>=di.sectors_on_floppy){
			/* svuota la lista !
			-- delete the current items in the list... */
			LDelRow (0,0,LHandle);
			sectListSize=0;
			last_list_cell=0;
			/* non c'� bisogno di deallocare sectorList perch� tanto ci
			penser� la SetHandleSize */
			/* get next disk */
			last_header-=di.sectors_on_floppy;
			if(!disco_espulso){
				riprova:
				diskEject(true);
				if(aspetta_inserzione(in_Italia?
					"\pInserisci il prossimo disco tar":
					"\pInsert next tar disk",button_is_stop)) break;
				if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized) goto riprova;	/* cannot use a do-while,
						the above break must exit from another loop... */
				}
			else
				last_header=sect_n;
			previousFormat=tar_POSIX;
			list_button_code=0;
			fase=reading_disk;
			}
		else{
			printf(in_Italia?FINITO:DONE);
			fine_lavoro();
			break;
			}
		}
	}
dispose_list_dialog();
if(sectListSize) DisposHandle(sectorList);

}

void select_and_write(short);
void select_and_write(vrefnum)
short vrefnum;
{
Point myCell;
Boolean more_files;
Boolean was_desktop;
short index_vrn;
short foundref; long founddir;

	/* if lista vuota d� un dialogo di alert */
	if(last_list_cell==0){
		dispose_list_dialog();
		ParamText(in_Italia?"\pNon ci sono files in questa cartella":
			"\pThere are no files in this folder",PNS,PNS,PNS);
		my_alert();
		return;
		}

	if(setjmp(main_loop)<0) {
		dispose_list_dialog();
		return;
		}

	semimodal_list=true;
	append_to_list(NULL, 0);
	LDoDraw(true,LHandle);
	ShowWindow(ListDialog);
	SelectWindow(ListDialog);
	SetPort( ListDialog );
	my_event_filter=select_list_filter;
	list_button_code=0;
	fase=ricevuto_comando;

	while(!list_button_code){
		extern long last_selection;
		if(!gInBackground && FrontWindow()!=ListDialog)
			SelectWindow(ListDialog);
		MainEvent();
		if(list_button_code==sdFind){
			find_options&=~remove_extras;
			find_and_select();
			list_button_code=0;
			}
		else if(list_button_code==sdExtract){
			myCell.h=0;
			myCell.v=0;
			if(! LGetSelect(true,&myCell,LHandle) || myCell.v==last_list_cell)
				list_button_code=0;
			}
		accept_abort_command();
		}
	if(list_button_code==sdCancel){
		dispose_list_dialog();
		return;
		}
	list_button_code = mouseClicksDisabled;
	hilite_list_buttons(255);
	was_desktop= gHasFindFolder &&
		FindFolder(-SFSaveDisk,'desk',0,&foundref,&founddir)==noErr &&
		founddir==CurDirStore;

	index_vrn=-1;
	do{
		extern Boolean write_in_corso;
		extern char alias_prefix[100];
		Point curr_cell;
		char buffer[64];
		short i;

		i=sizeof(buffer);
		LGetCell(&buffer[1],&i,myCell,LHandle);
		buffer[0]=i;
		/*printf("%d:%P\n",myCell.v,buffer);*/
		fase=writing_disk;
		write_in_corso=1;
		alias_prefix[0]='\0';

		if( stored_vrefnums>1 && was_desktop){	/* if it was not the desktop, really
				I don't know why stored_vrefnum might be >1, but then it's better to
				try to behave normally than redirect all reads from the Desktop */
			
			/* may happen only if the files are from the "Desktop" folder, which
			is a "merge" of different folders, so a single vrefnum is NOT enough */
			if(index_vrn<0 || index_vrn<stored_vrefnums-1 && idx_list[index_vrn+1]<myCell.v){
				do
					index_vrn++;
				while(index_vrn<stored_vrefnums-1 && idx_list[index_vrn+1]<myCell.v);
				if(gHasFindFolder && FindFolder(vrn_list[index_vrn],'desk',0,&foundref,&founddir)==noErr ){
					vrefnum=crea_WD(foundref,founddir);
					register_WD(vrefnum);
					}
				}
			}

		curr_cell = myCell;
		myCell.v++;
		more_files=LGetSelect(true,&myCell,LHandle);
		if(myCell.v==last_list_cell) more_files=false;
		if(!more_files)
			dispose_list_dialog();
	
		curr_vrefnum=vrefnum;	/* non si sa mai... */
		SetVol(NULL,curr_vrefnum);
		tar_file_or_folder(buffer);
		if(ListDialog){
			LSetSelect(false,curr_cell,LHandle);
			LAutoScroll(LHandle);
			}

		if(!more_files){
			azzera_settore();
			write_in_corso=0;
			}
		}
	while(more_files);

}


static void prepara_nome(name_ptr, buffer, length, ci_stanno_certo, pixel_size)
/* modify the file name so that it contains the file size and fits within the
limited space of the list */
char *name_ptr;
register char *buffer;
long length;
short ci_stanno_certo,pixel_size;
{
short ln,ls,l;
char len_buff[20];

len_buff[0]=' ';
len_buff[1]='(';
if(length>9999){
	my_itoa((long)((length+512)>>10),&len_buff[2]);
	ls=strlen(len_buff);
	strcpy(&len_buff[ls]," Kbyt");
	ls += 5;
	}
else{
	my_itoa((long)length,&len_buff[2]);
	ls=strlen(len_buff);
	strcpy(&len_buff[ls]," byt");
	ls += 4;
	}

ln=strlen(name_ptr);

mcopy(buffer,name_ptr,ln);
mcopy(&buffer[ln],len_buff,ls+1);

if(ln+ls > ci_stanno_certo){
	register short i;
	GrafPtr	savePort;

	GetPort( &savePort );
	SetPort( ListDialog );	/* perch� � dalla GrafPort che StringWidth deduce
							font e size da usare... */
	my_c2pstr(buffer);
	i=1;
	while(StringWidth(buffer)>pixel_size){
		while(i<ln && buffer[i]!='/') i++;
		if(i>=ln) break;	/* non c'� nessun path da tagliare, meglio lasciare
							cos� come � */
		buffer[1]='�';
		mcopy(&buffer[2],&buffer[i],ln+ls-i+2);
		buffer[0] -= i-2; ln -= i-2;
		i=3;	/* per non ritrovare lo stesso'/' */
		}
	my_p2cstr(buffer);
	SetPort( savePort );
	}
mcopy(&buffer[ln+ls],"es)",6);
}


pascal short my_search(char *,char *,short,short);
static pascal short my_search(p1,p2,l1,l2)
register char *p1,*p2;
register short l1,l2;
{
short i=0;
#define bufsize 80
unsigned char buffer[bufsize];
register unsigned char *p;

if(!l1) return 1;

/* i nomi sono passati attraverso prepara_nome, quindi devo cominciare col
togliere il numero di bytes
-- P2 is the search string, in lowercase. P1 is the string installed in the list, 
which was not converted to lowercase and was altered by prepara_nome so that it
contains the byte count in parentheses: it's better to extract the meaningful
part, translating to lowercase, and in order to use strncmp do that as a C string
*/
p=&buffer[bufsize-1];
*p='\0';
if(find_options&remove_extras){
	while(l1>0 && p1[--l1]!='(')
		;
	l1--;	/* lo spazio... */

	while(l1-->0 && p1[l1] != '/' ){
		if(p1[l1]>='A' && p1[l1]<='Z')
			*--p = p1[l1] + 'a'-'A';
		else
			*--p = p1[l1];
		}
	}
else{
	while(l1-- > 0){
		if(p1[l1]>='A' && p1[l1]<='Z')
			*--p = p1[l1] + 'a'-'A';
		else
			*--p = p1[l1];
		}
	}

/* printf("!%s!\n",p); */

#if 0	/* the version used till suntar 1.3.2 */
while(*p){
	if(!strncmp(p,p2,(size_t)l2) ) return 0;*/
	if(find_options&at_start) return 1;
	p++;
	}
#else
if(find_options&at_start){
	char buf[100];
	strncpy(buf,p2,(size_t)l2);
	buf[l2]=0;
	if(match(p,buf)) return 0;
	}
else
	while(*p){
		if(!strncmp(p,p2,(size_t)l2) ) return 0;
		p++;
		}
#endif

return 1;
}

static void find_and_select()

/* handle the find dialog and the find operation */
{
Str255 buffer;
DialogPtr myDialog;
short		kind;
Handle		h;
Rect		r;
short itemHit;

{
AlertTHndl	alertHandle;
register Rect *p;
short vs,hs;
GrafPtr savePort;
SignedByte oldflags;

alertHandle = (AlertTHndl)GetResource('DLOG',135);
oldflags=HGetState(alertHandle);
HNoPurge((Handle)alertHandle);

/* center the dialog near the Find button from which it was generated */

GetPort(&savePort);
SetPort(ListDialog);

GetDItem(ListDialog,sdFind,&kind,&h,&r);

LocalToGlobal(&r);		/* really, r.topLeft */

p= &(**alertHandle).boundsRect;
vs= p->bottom - p->top;
hs= p->right - p->left;

if( (p->top=r.top-80)<(kind=MBARHEIGHT+15))
	p->top=kind;
else if(p->top > (kind=screenHeight-vs-15))
	p->top = kind;

if( (p->left=r.left-50)<15)
	p->left=15;
else if(p->left > (kind=screenWidth-hs-15))
	p->left = kind;

p->bottom=p->top+vs;
p->right=p->left+hs;
SetPort(savePort);

myDialog = GetNewDialog (135,NULL,(char*)-1);
HSetState(alertHandle,oldflags);
}

if(myDialog!=NULL){
	if(in_Italia){
		#if 0
		static char* titoli[]={"\pTrova","\pNon trovare",
			"\pil testo � all\'inizio del nome","\pSeleziona i nomi contenenti" };
		#else
		static char* titoli[]={"\pTrova","\pNon trovare",
			"\pusa * ? [] \\ stile shell UNIX " };
		#endif
		short i;
		for(i=fdFind;i<fdSText;i++){
			set_item_text(myDialog,i,titoli[i-fdFind]);
			}
/*		set_item_text(myDialog,fdSText,titoli[3]);*/
		}
	ShowWindow(myDialog);
/* handle the dialog */
	do {
		GetDItem(myDialog,fdCheck,&kind,&h,&r);
		if(find_options&at_start){
			set_item_text(myDialog,fdSText,in_Italia?"\pSeleziona i nomi che matchano":
				"\pSelect file names matching");
			SetCtlValue((ControlHandle)h, 1 );
			}
		else{
			set_item_text(myDialog,fdSText,in_Italia?"\pSeleziona i nomi contenenti":
				"\pSelect file names containing");
			SetCtlValue((ControlHandle)h, 0 );
			}

		max_paste_size=50;
		ModalDialog(textedit_filter,&itemHit);
		if(itemHit==fdCheck){
			find_options^=at_start;
			SetCheckBox(myDialog,itemHit);
			}
		} 
	while (itemHit != fdFind && itemHit != fdCancel );

	get_item_text(myDialog,fdText,buffer);

	DisposDialog(myDialog);

	if(itemHit==fdFind){
		register short i;
		Point myCell;

		if(buffer[0]!=0){
			for(i=1;i<=buffer[0];i++){
				if(buffer[i]>='A'&&buffer[i]<='Z')
					buffer[i] += 'a'-'A';
				/* I wished to add something like this:
				else if(buffer[i]=='/')
					find_options|=match_on_path;
				so that if the search string contained a / the search was
				performed on the whole path name, while if it did not it
				was performed on the filename component only. But in the
				list some long pathnames are abbreviated, and I should keep
				the full name elsewhere. Not worth of it
				*/
				}
			/* if(buffer[0]=='/') remove it; */
			/* buffer[buffer[0]+1]='\0'; */

			myCell.v=myCell.h=0;
			while( LSearch(&buffer[1],buffer[0],my_search,&myCell,LHandle)){
				/*printf("%d,%d\n",myCell.v,myCell.h);*/
				LSetSelect(true,myCell,LHandle);
				myCell.v++;
				}
			LAutoScroll(LHandle);
			}
		/*printf("%d:%P\n",find_options,buffer);*/
		}
	}
}

static void list_updater(theEvent)
EventRecord *theEvent;
{
short j;
DialogPtr w=NULL;
DialogSelect(theEvent,&w,&j);
}

static void list_activater(theEvent)
EventRecord *theEvent;
{
short j;
DialogPtr w=NULL;
if(theEvent->modifiers & activeFlag) SetPort( ListDialog );
LActivate((theEvent->modifiers & activeFlag)!=0,LHandle);
DialogSelect(theEvent,&w,&j);
}

static void select_list_filter(theEvent)
register EventRecord *theEvent;
{
short i,j;

if(list_with_textedit){
	if( theEvent->what==keyDown &&
	    ((char)theEvent->message==ESC||(char)theEvent->message==CR||(char)theEvent->message==enter_key)){
			Handle h;
			Rect r;
			short kind;
			GetDItem(ListDialog,sdExtract,&kind,&h,&r);
			if( ((ControlRecord*)*h)->contrlHilite==0){
				SelectButton(h);
				list_button_code=sdExtract;
				}
			theEvent->what=nullEvent;
			}
	}

switch(theEvent->what){
/* no, non c'� un bottone di default ! e devo lasciare attivi i comandi
case keyDown:
case autoKey:
	if((unsigned char)theEvent->message==CR || (unsigned char)theEvent->message==enter_key)
		list_button_code=sdExtract;
	theEvent->what=nullEvent;
	break;
	*/
case mouseDown:
	{
	WindowPtr whichWindow;
	short i;

	i=FindWindow( theEvent->where, &whichWindow );

	if(semimodal_list){
		if(i==inMenuBar||i==inDesk||i==inSysWindow){
			break;
			}
		else if(whichWindow!=(WindowPtr)ListDialog){
			if(i==inDrag && (theEvent->modifiers&cmdKey))
				return;		/* il tasto command comporta il non rendere corrente la finestra
						-- accept a command-drag of another window */
			else{
				theEvent->what=nullEvent;
				break;
				}
			}
		}

	if(whichWindow==(WindowPtr)ListDialog){
		GrafPtr	savePort;

		GetPort( &savePort );
		SetPort( ListDialog );
		switch(i){
		case inContent:
			if (whichWindow != FrontWindow() )
				SelectWindow(whichWindow);
			else{
				Point localPt = theEvent->where;
				ControlHandle 	theControl;

				GlobalToLocal(&localPt);

				if (localPt.h > cornice.right+5){
#if 0
					if( (FindControl(localPt, whichWindow, &theControl)) != 0) {
						if(TrackControl(theControl, localPt, 0L)){
							short		kind;
							Handle		h;
							Rect		r;
							short i=0;
							do
								GetDItem(ListDialog,++i,&kind,&h,&r);
							while(h!=theControl && i<=sdFind);
							if(h==theControl)
								list_button_code=i;
							}
						}
#endif

					if( IsDialogEvent(theEvent)){
						if(DialogSelect(theEvent,&ListDialog,&j))
							list_button_code=j;
						}
					}
				else{
					 if(list_button_code!=mouseClicksDisabled || localPt.h > cornice.right-15){
					 	#if 1
						short doubleClick;
						doubleClick =LClick(localPt,theEvent->modifiers,LHandle);
						if(doubleClick && ((**LHandle).selFlags & lOnlyOne) )
							list_button_code=sdExtract;
						#else
						/* should check for double clicks here */
						LClick(localPt,theEvent->modifiers,LHandle);
						#endif
						}
					}
				}
			theEvent->what=nullEvent;
			SetPort(savePort);
			return;
		case inDrag:
			SetPort(savePort);	/* MainEvent lo gestisce correttamente */
			return;
		}
		/*theEvent->what=nullEvent;*/
		SetPort(savePort);
		}
	}
	break;
}
}

#if 0	/* no more used, but may return useful in the future */

short dialogo_con_radio(dlg_id)
short dlg_id;
{
short i,currentRadio;
DialogPtr myDialog;
short		kind;
Handle		h;
Rect		r;
short itemHit;
AlertTHndl	alertHandle;
SignedByte oldflags;

short first_radio;

flush_all();
if (alertHandle = GetResource('DLOG',dlg_id)) {
	oldflags=HGetState(alertHandle);
	HNoPurge((Handle)alertHandle);
	PositionDialog( &((**alertHandle).boundsRect));
	myDialog = GetNewDialog (dlg_id,NULL,(char*)-1);
	HSetState(alertHandle,oldflags);

	first_radio=0;
	do{
		GetDItem(myDialog,++first_radio,&kind,&h,&r);
		}
	while((kind&0x7F)!=ctrlItem+radCtrl);
	currentRadio= first_radio;
	if(dlg_id==134) currentRadio++;
	SetRadioButton(myDialog,currentRadio,CONTROL_ON);

	do {
		ModalDialog(modal_filter,&itemHit);
		if(itemHit!=1){
			SetRadioButton(myDialog,currentRadio,CONTROL_OFF);
			SetRadioButton(myDialog,(currentRadio=itemHit),CONTROL_ON);
			}
		}
	while (itemHit != 1 );

	DisposDialog(myDialog);
	}
return currentRadio-first_radio;
}
#endif


static void dialogo_tipi_creatori(params)
register struct more_options* params;
{
short n_items,n_altri_items;
short i,j;
Rect	r;
Handle	h_ditl;
DialogPtr myDialog;
char *p;

h_ditl = GetResource('DITL',149);
if(h_ditl==NULL) return;
HNoPurge (h_ditl);	/* modifying a resource in memory would be useless if it may be purged */


if(params->h_tp==NULL){
	params->h_tp=h_type;
	params->h_cr=h_creat;
	params->h_ex=h_extens;
	params->h_co=h_comments;
	params->h_as=h_ascii;
	
	HandToHand(&params->h_tp);
	HandToHand(&params->h_cr);
	HandToHand(&params->h_ex);
	HandToHand(&params->h_co);
	HandToHand(&params->h_as);
	}

{Rect rView;
short pos,height,step;
/* create the editable text items (I preferred not to place them in the
resource because it would be very difficult to modify the number and
spacing of so many items with ResEdit, while that's easy at run time.
 As you can see, the number of items may become a variable, and that's
impossible with a fixed resource ! */

/*HLock(h_ditl);*/
p=*h_ditl;
n_items=n_altri_items=*(short*) p;
p+=2;
if(n_items< 5*size_ext_table){
	n_altri_items--;
	while(n_items>0){
		n_items--;
		p += 14 + ((p[13]+1)&~1);
		}
	/*printf("item type=%d\n",p[12]);
	printf("rect=%d %d %d %d\n",*(Rect*)(p+4) );*/

	i=p-*h_ditl;
	/*HUnlock(h_ditl);*/	/* SetHandleSize prefers unlocked handles */
	SetHandleSize(h_ditl, i + 14*5*size_ext_table);
	/*HLock(h_ditl);*/
	p= *h_ditl +i;
	r=*(Rect*)(p+4);
	step=23;
	while(step*size_ext_table>(r.bottom-r.top)+6) step--;
	height=step-5;
	if(step<18) height++;
	for(i=0;i<size_ext_table;i++){
		pos=0;
		rView.top=r.top+i*step+2;
		rView.bottom=rView.top+height;
		for(j=0;j<5;j++){
			short width=j<3 ? 44: j==3 ? -5+3 : r.right-r.left-pos-16-5;
			if(j==3){	/* checkbox */
				rView.left=r.right-16;
				rView.right=r.right;
				}
			else{
				rView.left=r.left+pos;
				rView.right=rView.left+width;
				}
			/*if(i==size_ext_table-1&&!j) printf("altezza=%d\n",rView.bottom-r.top);*/
			*(long*)p = 0L;
			*(Rect*)(p+4) = rView;
			*(p+12) = (j==3)?  ctrlItem+chkCtrl : editText+itemDisable;
			*(p+13) = 0;
			p+=14;
			pos+=width+5;
			if(j==0) pos+=3;
			}
		}
	p=*h_ditl;
	*(short*)p += 5*size_ext_table-1;	/* n_items */
	}
else
	n_altri_items -= 5*size_ext_table;
/*HUnlock(h_ditl);*/
}
{
	short item;
	AlertTHndl	alertHandle;
	SignedByte oldflags;
	short str_size,str_size_com;
	char *q;

	flush_all();
	check_foreground();
	SetCursor(&arrow);

	alertHandle = (AlertTHndl)GetResource('DLOG',149);
	if (alertHandle) {
		oldflags=HGetState(alertHandle);
		HNoPurge((Handle)alertHandle);
		PositionDialog( &((**alertHandle).boundsRect));
		myDialog=GetNewDialog(149,NULL,(WindowPtr)-1L);
		HSetState(alertHandle,oldflags);
		/*if(in_Italia)
			set_item_text(myDialog,item,"\p...");
			}
		*/

/* Now, fill the initial values of the strings */
		p=*h_ditl;
		n_items=n_altri_items+2;
		HLock(params->h_ex);
		HLock(params->h_co);
		p=*(params->h_ex) +2;
		q=*(params->h_co) +2;
		for(i=0;i<size_ext_table;i++){
			char buf_ostype[5];
			set_item_text(myDialog,n_items++,p);
			if(*p==0 && *((*(params->h_tp))+(i<<2))==' '){	/* no data => empty type & creator & non-checked */
				*((char*)(*(params->h_as))+i)=0;
				buf_ostype[0]=0;
				}
			else
				buf_ostype[0]=4;
			mcopy(&buf_ostype[1],(*(params->h_tp))+(i<<2),4);
			set_item_text(myDialog,n_items++,buf_ostype);
			mcopy(&buf_ostype[1],(*(params->h_cr))+(i<<2),4);
			set_item_text(myDialog,n_items++,buf_ostype);
			get_option_var(myDialog,n_items++,*((char*)(*(params->h_as))+i));
			set_item_text(myDialog,n_items++,q);
			p+=*p+1;
			q+=*q+1;
			}		
		HUnlock(params->h_ex);
		HUnlock(params->h_co);


		/* show the dialog and let the user handle it */
		ShowWindow(myDialog);
		max_paste_size=40;
		for(;;){
			ModalDialog(textedit_filter,&item);
			if(item!=1)
				SetCheckBox(myDialog,item);
			else
 				break;
 			}

		/* finally, collect the final values */
		str_size=2;
		str_size_com=2;
		p=*h_ditl;
		n_items=n_altri_items+2;


		for(i=0;i<size_ext_table;i++){
			Str255 buf;
			get_item_text(myDialog,n_items++,buf);
			buf[0]=min(buf[0],12);
			
			SetHandleSize(params->h_ex,(Size)str_size+buf[0]+1);
			pStrcpy(*(params->h_ex)+str_size,buf);
			str_size+=buf[0]+1;

			get_item_text(myDialog,n_items++,buf);
			for(j=buf[0];j<=4;)
				buf[++j]=' ';
			mcopy(*(params->h_tp)+(i<<2),&buf[1],4);

			get_item_text(myDialog,n_items++,buf);
			for(j=buf[0];j<=4;)
				buf[++j]=' ';
			mcopy(*(params->h_cr)+(i<<2),&buf[1],4);

			set_option_var(myDialog,n_items++,(char*)(*(params->h_as))+i);

			get_item_text(myDialog,n_items++,buf);
			buf[0]=min(buf[0],40);
			
			SetHandleSize(params->h_co,(Size)str_size_com+buf[0]+1);
			pStrcpy(*(params->h_co)+str_size_com,buf);
			str_size_com+=buf[0]+1;
			}

		DisposDialog(myDialog);
		}
}


HPurge (h_ditl);
return;
}


/***************************************/

static char str_SCSI[]="SCSI  ", str_drive[]="drive ";

short decode_dev_name(Point,char*,short*);
static short decode_dev_name(theCell,buff,p_i)
/* very, very primitive */
Point theCell;
char* buff;	/* a Str255 */
short *p_i;
{
*p_i=256-1;
LGetCell(buff,p_i,theCell,LHandle);
buff[*p_i]='\0';
		
if(buff[0]==str_SCSI[0]){	/* the first letter is either a 'S' (SCSI) or 'd' (drive) */
	my_atoi(&buff[sizeof(str_SCSI)-2],p_i);
	return 1;
	}
else{
	my_atoi(&buff[sizeof(str_drive)-1],p_i);
	return 2;
	}
}


void scegli_device()
{
short i;
Str255 buff;
DrvQEl *q;
short get_driver_name(short,unsigned char *);
short used_refnums[20],n_uses[20], n_used;
char c;

n_used=0;
if(setjmp(main_loop)<0) {
	dispose_list_dialog();
	return;
	}

ask_verbosity=(command_modifiers&optionKey)!=0;

SetCursor(&waitCursor);
new_list_dialog(152,(WindowPtr)-1,false);
semimodal_list=true;
list_with_textedit=true;

q=GetDrvQHdr()->qHead;
while(q){
	strcpy(buff,str_drive);
	my_itoa(q->dQDrive,buff+strlen(buff));
	strcat(buff," ");
	i=strlen(buff);
	if(get_driver_name(q->dQRefNum,buff+i)){
		my_p2cstr(buff+i);
		if(q->dQRefNum<=-33 && q->dQRefNum>=-39){	/* such refnums are reserved for
				drivers of SCSI devices */
			strcat(buff," (ID=");
			my_itoa(~(q->dQRefNum+32),buff+strlen(buff));
			strcat(buff,")");
			}
		c= *(((char*)q)-3);
		if(c!=8&&c!=0x48&&c!=1&&c!=2){	/* not present... */
			/* some device drivers (MountImage, Syquest INIT) create a lot of
			drive records, just in case one wants to open so many volumes. But
			such drives are not real and they uselessly make a very long list;
			obviously, if the disk is present it must be possible to open it,
			same for physical devices which are really not present but may be
			inserted later, which obviously are never more than three */
			for(i=0;i<n_used;i++)
				if(used_refnums[i]==q->dQRefNum){
					n_uses[i]++;
					break;
					}
			if(i==n_used){
				n_used++;
				used_refnums[i]=q->dQRefNum;
				n_uses[i]=1;
				}
			if(n_uses[i]<=3)
				append_to_list(buff,strlen(buff));
			}
		else
			append_to_list(buff,strlen(buff));
		}
	q=q->qLink;
	}

if(gHasSCSIManager){	/* All Macs since the Plus have SCSI, but A/UX has no SCSI manager... */
	#if 0
	for(i=0;i<7;i++){
		strcpy(buff,str_SCSI);
		buff[5]='0'+i;
		append_to_list(buff,6);
		}
	#else
	strcpy(buff,str_SCSI);
	fill_scsi_list(buff,default_type==1?default_device_num:-1,0);
	#endif
	}
else
	button_inactive(ListDialog,sdScanSCSI);

append_to_list(buff,0);


{	/* select the "default" device, the last one opened */
Point myCell;
short j,k;

myCell.h=0;

for(myCell.v=0;myCell.v<last_list_cell;myCell.v++){
	j=decode_dev_name(myCell,buff,&i);
	if(j==default_type && i==default_device_num){
		LSetSelect(true,myCell,LHandle);
		LAutoScroll(LHandle);
		break;
		}
	}
}


LDoDraw (true,LHandle);
SetCursor(&arrow);

list_button_code=0;
fase=ricevuto_comando;

{
Point myCell;
Handle	h;
{	/* must know the handle to the button */
Rect	r;
short	kind;
GetDItem(ListDialog,sdExtract,&kind,&h,&r);
}

while(!list_button_code){
	EventRecord myEvent;
	short j;
	extern long max_sleep_time;
	myCell.h=0;
	myCell.v=0;
	if(LGetSelect(true,&myCell,LHandle))
		HiliteControl(h,0);
	else
		HiliteControl(h,255);

	max_sleep_time=GetCaretTime();
	if(get_event(&myEvent)){
		max_sleep_time=-1;
		if(myEvent.what==mouseDown||myEvent.what==keyDown&&!(myEvent.modifiers & cmdKey)){
			select_list_filter(&myEvent);
			if( IsDialogEvent(&myEvent)){
				if(DialogSelect(&myEvent,&ListDialog,&j))
					list_button_code=j;
				}
			else
				handle_event(&myEvent);
			}
		else if(myEvent.what==keyDown && (myEvent.modifiers & cmdKey) ){
			/* see IM vol 1, IsDialogEvent does not handle that */
			if(((char)myEvent.message|0x20)=='q')
				handle_event(&myEvent);
			else{
				/*textedit_filter (ListDialog,&myEvent,&j); ???? non funziona ?
				problemi di incomprensione tra scrap ? (forse activate della mia
				finestra NON � accompagnato dai TEtoScrap ? */
				}
			}
		else if( myEvent.what!=diskEvt && IsDialogEvent(&myEvent)){
			if(DialogSelect(&myEvent,&ListDialog,&j))
				list_button_code=j;
			}
		else{
			handle_event(&myEvent);
			}
		}
	max_sleep_time=-1;

	if(!gInBackground && FrontWindow()!=ListDialog){
		SelectWindow(ListDialog);
		}
	accept_abort_command();
	if(di.tipo_device!=d_notopen){	/* an unexpected disk insertion resulted into
				opening the inserted disk ! */
		dispose_list_dialog();
		return;
		}
	if(list_button_code==6){
		button_inactive(ListDialog,sdScanSCSI);
		button_inactive(ListDialog,sdCancel);
		button_inactive(ListDialog,sdExtract);

		/* a questo punto, cancella gli elementi SCSI e ricreali coi nomi */
		if(LGetSelect(true,&myCell,LHandle))
			LSetSelect(false,myCell,LHandle);

		LDelRow (8,last_list_cell-7,LHandle);
		last_list_cell-=8;

		strcpy(buff,str_SCSI);
		fill_scsi_list(buff,0,1);
		append_to_list(buff,0);
		LScroll (0,1,LHandle);

		list_button_code=0;
		button_active(ListDialog,sdScanSCSI);
		button_active(ListDialog,sdCancel);
		}
	}
if(list_button_code==sdExtract){

	{EventRecord myEvent;
	EventAvail (1,&myEvent);	/* want a null event, and do not extract
								it from the queue: it's only to know the current state
								of the option key */
	if(myEvent.modifiers&optionKey) ask_verbosity=true;
	}

	HideWindow(ListDialog);	/* otherwise the window may be deleted from the screen after
					other events (e.g. the alert in open_scsi) */
	get_item_text(ListDialog,sdStartSect,buff);
	di.archive_start_sector= pstrtoi(buff);	/* it returns 0 in case of error... */
	di.writing_allowed=false;

	myCell.h=0;
	myCell.v=0;
	if(LGetSelect(true,&myCell,LHandle)){
		short j=decode_dev_name(myCell,buff,&i);
		if(j!=default_type||i!=default_device_num){
			default_type=j;
			default_device_num=i;
			def_cambiato=1;
			}
		printf("\nNow opening %s\n",buff);
		if(j==1){	/* SCSI */
			dispose_list_dialog();
			open_SCSI_device(i);

			return;
			}
		else{	/* j==2, drive */
			/* printf(" (drive number %d)\n",i); */
			q=GetDrvQHdr()->qHead;
			while(q){
				if(q->dQDrive==i){
					short inPlace;
					char c;

					di.drive_number=i;
					di.m_vrefnum=q->dQRefNum;
					di.signature_avail=2;
					di.v.fi.opened_by_insertion=0;
					di.tipo_device=d_driver;
					aggiorna_nome_aperto();
					c= *(((char*)q)-3);
					di.isEjectable= c!=8&&c!=0x48;

					di.sectors_on_floppy=0;
					reinit_buffering(512);
					if(testa_stato(&inPlace,1))
						; /* niente, mi basta aver stampato qualcosa...*/
					if(!inPlace)
						printf("Warning, the physical media seems to be missing\n");
					new_ddriver_menu();
					break;
					}
				q=q->qLink;
				}
			if(!q) error_message("Drive no more existing\n");
			}
		}
	}
}

dispose_list_dialog();

}


static Handle str_hand;

static pascal Boolean rare_filter (theDialog,theEvent,itemHit)
DialogPtr theDialog;
register EventRecord *theEvent;
short *itemHit;
{
unsigned char name[100];
get_item_text(theDialog,6,name);
if(!EqualString(name,*str_hand,true,true)){
	button_active(theDialog,4);
	/*segnati di averlo fatto per non ripetere all'infinito !!! o condizionalo
	a cntrolHilite */
	}
return modal_filter(theDialog,theEvent,itemHit);
}

void rare_update(DialogPtr);
static void rare_update(theDialog)
DialogPtr theDialog;
{
unsigned char name[100];
	/*if(str_hand==NULL) DebugStr("\pg1");*/
	HUnlock(str_hand);
	get_item_text(theDialog,6,name);
	SetHandleSize(str_hand,name[0]+1);
	/*if( GetHandleSize(str_hand)<name[0]+1)
		DebugStr("\pg2");*/
		
	HLock(str_hand);
	mcopy(*str_hand,name,name[0]+1);
	ChangedResource(str_hand);
	/*if(ResError()) DebugStr("\pg3");*/
	WriteResource(str_hand);
	/*if(ResError()) DebugStr("\pg4");*/
	button_inactive(theDialog,4);
}



void rare_options(int);
void rare_options(quitting)
int quitting;
{
	DialogPtr myDialog;
	short item;
	AlertTHndl	alertHandle;
	SignedByte oldflags;
	Boolean changed=false;
	short n_opts,curr_shown;
	short curr_opt=1;
	short id;
	ResType typ;
	unsigned char name[100];

	flush_all();
	SetCursor(&arrow);

	n_opts=Count1Resources ('STR ');

	alertHandle = (AlertTHndl)GetResource('DLOG',157);
	if (alertHandle) {
		oldflags=HGetState(alertHandle);
		HNoPurge((Handle)alertHandle);
		PositionDialog( &((**alertHandle).boundsRect));
		myDialog=GetNewDialog(157,NULL,(WindowPtr)-1L);
		HSetState(alertHandle,oldflags);
		button_active(myDialog,1);
		button_inactive(myDialog,3);

		if(quitting){
			curr_opt--;
			do{
				curr_opt++;
				str_hand=Get1IndResource ('STR ',curr_opt);
				GetResInfo (str_hand,&id,&typ,name);
				}
			while(id!=138);
			}
		str_hand=NULL;
		curr_shown=-1;

		ShowWindow(myDialog);
		do{
			if(curr_shown!=curr_opt){
				if(str_hand) HUnlock(str_hand);
				str_hand=Get1IndResource ('STR ',curr_opt);
				/*if(str_hand==NULL) Debugger();*/
				GetResInfo (str_hand,&id,&typ,name);
				HLock(str_hand);
				set_item_text(myDialog,6,*str_hand);
				set_item_text(myDialog,5,name);
				curr_shown=curr_opt;
				button_inactive(myDialog,4);
				}
			ModalDialog(rare_filter,&item);
			if(item==1 || item==3 || item==2){
				get_item_text(myDialog,6,name);
				if(!EqualString(name,*str_hand,true,true)){
					ParamText("\pUpdate ?",NULL,NULL,NULL);
					if(my_modal_dialog(130,NULL,0)==1){
						rare_update(myDialog);
						changed |= must_be_reloaded(id);
						}
					else{
						set_item_text(myDialog,6,*str_hand);
						item=0;
						button_inactive(myDialog,4);
						}
					}
				}

			if(item==1){	/* next */
				if(curr_opt<n_opts)
					curr_opt++;
				if(curr_opt==n_opts)
					button_inactive(myDialog,1);
				button_active(myDialog,3);
				}
			else if(item==3){	/* prev */
				if(curr_opt>1) curr_opt--;
				if(curr_opt==1)
					button_inactive(myDialog,3);

				button_active(myDialog,1);
				}
			else if(item==4){
				rare_update(myDialog);
				changed |= must_be_reloaded(id);
				}
			}
		while(item!=2);

		if(str_hand) HUnlock(str_hand);

		DisposDialog(myDialog);
		if(changed && !quitting){
			ParamText("\pYou must quit and relaunch suntar in order to use the new buffer sizes",
				PNS,PNS,PNS);
			my_alert();
			}
		load_STR_prefs_except_bufsizes();
		UnloadSeg(load_STR_prefs_except_bufsizes);	/* stesso seg di must_be_reloaded */
		}
}

/**************************************/

int parse_hour(buf)
Str255 buf;
{
register char *p;
int n1,n2,n3,delta_time;
Boolean negativo=false;

my_p2cstr(buf);
p=buf;
while(*p==' ') p++;
if(*p=='-'||*p=='+'){
	negativo=*p=='-';
	p++;
	}
n1=n2=0;
while(*p>='0' && *p<='9'){
	n1=n1*10+*p++ -'0';
	}
if(*p++=='.'){
	n3=0;
	while(*p>='0' && *p<='9'){
		n2=n2*10+*p++ -'0';
		n3++;
		}
	if(n3==1 && n2==5)	/* somebody might type 1.5 meaning 1.30... */
		n2=30;
	else if(n3!=2)
		n2=0;
	else if(n2>60)
		n2=0;
	}
delta_time=n1*60 + n2;
if(negativo) delta_time=-delta_time;
return delta_time;
}

void compute_delta_time()
{
#define UNIXTIME	2082844800L  /* Jan 1, 1970 00:00:00 */

extern long unixtime;
unixtime=(long)options.fuso_orario*60 + UNIXTIME;
}

/**************************************/


void append_to_refnum_list(int);
void append_to_refnum_list(n)
int n;
{
if(!stored_vrefnums || vrn_list[stored_vrefnums-1]!=n){
	vrn_list[stored_vrefnums]=n;
	idx_list[stored_vrefnums]=last_list_cell;
	if(stored_vrefnums<n_elems_of(vrn_list)) stored_vrefnums++;
	}
}


/************ some simple utilities **********/

static Boolean set_option_var(dialog,item,var)
	DialogPtr	dialog;
	short		item;
	char		*var;
{
	short		i;
	Handle		h;
	Rect		r;

	GetDItem(dialog,item,&i,&h,&r);
	i=GetCtlValue(h);
	if(*var!= i){
		*var=i;
		return true;
		}
	return false;
}

static void get_option_var(dialog,item,var)
	DialogPtr	dialog;
	short		item;
	Boolean		var;
{
	short		kind;
	Handle		h;
	Rect		r;

	GetDItem(dialog,item,&kind,&h,&r);
	SetCtlValue(h,var);
}

void SetCheckBox(dialog,item)
	DialogPtr	dialog;
	short		item;
{
	short		kind;
	Handle		h;
	Rect		r;

	GetDItem(dialog,item,&kind,&h,&r);
	SetCtlValue((ControlHandle)h,!GetCtlValue(h));
}


void set_item_text(theDialog,item,text)
DialogPtr theDialog;
short item;
const unsigned char *text;
/* I often forget that a static or editable text requires a different call
to set its string, and I've caused a number of crashes using SetCTitle
when SetIText was required. That's why this routine does both */
{
short		kind;
Handle		h;
Rect		r;
GetDItem(theDialog,item,&kind,&h,&r);
if((kind&0x7C)==ctrlItem)
	SetCTitle(h,text);
else
	SetIText(h,text);
}

void get_item_text(theDialog,item,text)
DialogPtr theDialog;
short item;
unsigned char *text;
{
short		kind;
Handle		h;
Rect		r;
GetDItem(theDialog,item,&kind,&h,&r);
GetIText(h,text);
}

Boolean replace_resource(dest,source)
Handle dest,source;
{
/* given a Handle to a resource and a Handle to some memory, it replaces the
contents of the copy in RAM of the resource with a copy of the other data,
and deallocates the originals */
/* that's done in a way which is rather complex and expensive, but it's
dangerous to directly change master pointers to resources, hence I can't
simply swap the master pointers... */
Size s;
Boolean changed;
s=GetHandleSize(source);
changed = s!=GetHandleSize(dest) || !compare_mem(*dest,*source,s);

if(changed){
	SetHandleSize(dest,s=GetHandleSize(source));
	mcopy(*dest,*source,(short)s);
	}
DisposHandle(source);
return changed;
}


void button_inactive(theDialog,item)
DialogPtr theDialog;
short item;
{
	short	kind;
	Handle	h;
	Rect	r;

	GetDItem(theDialog,item,&kind,&h,&r);
	HiliteControl(h,255);
}

void button_active(theDialog,item)
DialogPtr theDialog;
short item;
{
	short	kind;
	Handle	h;
	Rect	r;

	GetDItem(theDialog,item,&kind,&h,&r);
	HiliteControl(h,0);
}

