/*
 * Mac Workstation exec to watch incomming and outgoing Mac Workstation
 * message traffic
 * The project type must be code resource
 * Check the box for custom header in the project type
 * Created 12:45:39 AM  10/26/89 by Aaron Wohl, aw0g+@andrew.cmu.edu
 * Aaron Wohl
 * 6393 Penn Ave #303
 * Pittsburgh PA,15206
 * home:(412)731-6159
 * work:(412)268-5032
 * Do what you will with it
 * See MWS_EXEC.c for how to build this
 * Probably needs LightspeedC 4.0, haven't tried 3.xx
 */

/*
 *	Modified for use with MPW C
 *	Color window bug fixed
 *
 *	2/28/90 Garth Cummings, Apple Computer, Inc.
 *
 */
 
/*configuration constants*/
/*min ticks between scroll operations, makes scroll slow enough to use on 68030*/
#define MIN_SCROLL_TIME (4)

/* define USE_MEM_RATHER_THAN_STACK as desired*/

/*
 * expand the click point this many pixels to obtain the area
 * considered to be the same for clicking to determine double clicks
 */
#define CLICK_IN_SAME_SPOT (1)

#include "mws.h"
#include <memory.h>
#include <scrap.h>
#include "mws_exec.h"

#define SBarWidth 16
#define DELCH (8)		/*character for deleting*/
#define imax(xx_1,xx_2) (((xx_1)>(xx_2))?(xx_1):(xx_2))
#define imin(xx_1,xx_2) (((xx_1)<(xx_2))?(xx_1):(xx_2))

typedef struct watcher_globals_R {
	Rect LastKnownSize;
	WindowPtr win;
	ControlHandle vScroll;
	TEHandle TEH;
	short LinesInView;
	Boolean AreActive;
	RgnHandle save_clip_in_auto_scroll;
	char **send_me;		/*message we are transmiting from paste*/
	short apply_speed_limit;
} watcher_globals, *wGlobalPtr;

static watcher_globals gl;

#define MAKE_WINDOW_IF_NEEDED {if (gl.win == nil) create_window(xecRef);}

void SetVScrollMax(void);
void SetVScrollMax()
{
	register short n;
	/*get line number of top line*/
	n = (*gl.TEH)->nLines-gl.LinesInView;

	/*count correctly if last line is terminated*/
	if ((*gl.TEH)->teLength > 0 &&
		 (*((*gl.TEH)->hText))[(*gl.TEH)->teLength-1]=='\n')
		n++;
	n=imax(0,n);
	if(GetCtlMax(gl.vScroll)!=n)
		SetCtlMax(gl.vScroll,n);
}

void ScrollTextToMatchControl(void);
void ScrollTextToMatchControl()
{
	short		oldScroll, newScroll, delta;

	oldScroll = (*gl.TEH)->viewRect.top - (*gl.TEH)->destRect.top;
	newScroll = GetCtlValue(gl.vScroll) * (*gl.TEH)->lineHeight;
	delta = oldScroll - newScroll;
	if (delta != 0)
	  TEScroll(0, delta,gl.TEH);
}

void SetCtlLoc(short newloc);
void SetCtlLoc(short newloc)
{
	/*don't scroll too fast on fast machines*/
	if(++gl.apply_speed_limit !=0) {
		static long last_scroll_time=0;
		long now;
		long dif;
		now=TickCount();
		dif=now-last_scroll_time;
		if(dif<MIN_SCROLL_TIME)
			return;
		last_scroll_time=now;
	}

	{short max=GetCtlMax(gl.vScroll);
	/*don't set the ctl value if it hasn't changed, it causes flickering*/
	newloc=imax(0,newloc);
	newloc=imin(newloc,max);
	if(newloc==GetCtlValue(gl.vScroll))
		return;
	SetCtlValue(gl.vScroll,newloc);
	ScrollTextToMatchControl();
	}
}

void ScrollSelectionIntoView(void);
void ScrollSelectionIntoView()
{
	register short topLine, bottomLine, theLine;
	
	SetVScrollMax();
	ScrollTextToMatchControl();
	
	topLine = GetCtlValue(gl.vScroll);
	bottomLine = topLine + gl.LinesInView-1;
	
	if ((*gl.TEH)->selStart < (*gl.TEH)->lineStarts[topLine] ||
			(*gl.TEH)->selStart >= (*gl.TEH)->lineStarts[bottomLine]) {
		for (theLine = 0; (*gl.TEH)->selStart >= (*gl.TEH)->lineStarts[theLine]; theLine++)
			;
		SetCtlLoc(theLine - gl.LinesInView / 2);
	}
}

void make_wm_window(THExecRef xecRef);
void make_wm_window(THExecRef xecRef)
{
	static char 	wTitle[] = "\pMWS Message Watcher";
	Rect			bounds;
	SysEnvRec		thisMachine;
	
	if (SysEnvirons(2, &thisMachine) != noErr) thisMachine.hasColorQD = false; 

	SetRect(&bounds,5,45,500,200);
	
	if (thisMachine.hasColorQD) {
		gl.win = (WindowPtr) NewCWindow(nil, &bounds, wTitle, true, zoomDocProc, 
										((WindowPtr)-1L), true, (long)xecRef);
	} else {
		gl.win = NewWindow(nil, &bounds, wTitle, true, zoomDocProc, ((WindowPtr)-1L), true,
							(long)xecRef);
	}
	
	((WindowPeek)gl.win)->windowKind = wExec;
	
	(*xecRef)->xOptions=
		(xCallIdle|xCallKey|xCallSend|xCallFilter|
		xCallReceive|xCallMouse|xCallStdMenuEvt);
	SetPort(gl.win);
	gl.AreActive=true;
}

void SetView(void);
void SetView()
{
	(*gl.TEH)->viewRect = gl.win->portRect;
	(*gl.TEH)->viewRect.right -= SBarWidth;
	InsetRect(&(*gl.TEH)->viewRect, 4, 4);
	gl.LinesInView = ((*gl.TEH)->viewRect.bottom-(*gl.TEH)->viewRect.top)/(*gl.TEH)->lineHeight;
	(*gl.TEH)->destRect.right = (*gl.TEH)->viewRect.right;
	TECalText(gl.TEH);
}

void WindowSizeHasChanged(void);
void WindowSizeHasChanged(void)
{
	Rect SafeRect;
	SafeRect=(*gl.vScroll)->contrlRect;
	InvalRect(&SafeRect);
	MoveControl(
		gl.vScroll,
		gl.win->portRect.right-SBarWidth+1,
		-1);
	SizeControl(
		gl.vScroll,
		SBarWidth,
		((gl.win->portRect.bottom-gl.win->portRect.top)-SBarWidth)+3);
	InvalRect(&SafeRect);
	SetView();
	SetVScrollMax();
}

void RealScrollProc(ControlHandle theControl,short theCode);
void RealScrollProc(ControlHandle theControl,short theCode)
{
	int	pageSize;
	int	scrollAmt;

	if (theCode == 0)
		return ;

	pageSize = ((*gl.TEH)->viewRect.bottom-(*gl.TEH)->viewRect.top) / 
			(*gl.TEH)->lineHeight - 1;
			
	switch (theCode) {
		case inUpButton: 
			scrollAmt = -1;
			break;
		case inDownButton: 
			scrollAmt = 1;
			break;
		case inPageUp: 
			scrollAmt = -pageSize;
			break;
		case inPageDown: 
			scrollAmt = pageSize;
			break;
		}
	SetCtlLoc(GetCtlValue(theControl)+scrollAmt);
}

void RealAutoScrollProc(void);
void RealAutoScrollProc()
{
	Point here;
	GrafPtr savePort;
	GetPort(&savePort);
	GetClip(gl.save_clip_in_auto_scroll);
	SetPort(gl.win);
	ClipRect(&gl.win->portRect);
	GetMouse(&here);
	if(here.v<(*gl.TEH)->viewRect.top)
		RealScrollProc(gl.vScroll,inUpButton);
	else if(here.v>=(*gl.TEH)->viewRect.bottom)
		RealScrollProc(gl.vScroll,inDownButton);
	SetClip(gl.save_clip_in_auto_scroll);
	SetPort(savePort);
}

pascal void ScrollProc(ControlHandle theControl,short theCode);
pascal void ScrollProc(ControlHandle theControl,short theCode)
{
	RealScrollProc(theControl,theCode);
}

pascal Boolean AutoScrollProc(void);
pascal Boolean AutoScrollProc(void)
{
	RealAutoScrollProc();
	return true;
}

/*bit table of characters that have printing width*/
#define BITS_PER_ENTRY (sizeof(long)*8)
long ok_ch[256/BITS_PER_ENTRY];

#define OK_IN_DISPLAY(xx_arg) \
	{ok_ch[(xx_arg)/BITS_PER_ENTRY]|=(1L<<((xx_arg)%BITS_PER_ENTRY));}

void set_widths(void);
void set_widths()
{
	int i;
	for(i=' ';i<0x7f;i++)
		OK_IN_DISPLAY(i);
	OK_IN_DISPLAY('\n');
}

/* insert a string that might have some
 * unprintable characters
 */
void insert_nasty_string(char *astr,int len);
void insert_nasty_string(astr,len)
register char *astr;
register int len;
{
	/* **MAKE THIS static TO TRADE STACK SPACE INTO PERMINANT SPACE** */
	char
#ifdef USE_MEM_RATHER_THAN_STACK
		static
#endif
		buf[4*kMsgMaxData+10];
	register char *dst=buf;
	register unsigned char ch;
	static char octal_digit[9]="01234567";
	static previous_string_ended_in_cr=false;
	if(len==0)return;
	if(previous_string_ended_in_cr) {
		*dst++='\n';
		previous_string_ended_in_cr=false;
	}
	while(--len >=0) {
		ch=*astr++;
		if(ch=='\\') {
			*dst++='\\';
			*dst++='\\';
		}else if((ok_ch[ch/BITS_PER_ENTRY]&(1L<<(ch%BITS_PER_ENTRY)))!=0)
			*dst++=ch;
		else {
			register int adigit;
			*dst++='\\';
			adigit=ch/0100;
			ch-=adigit*0100;
			*dst++=octal_digit[adigit];
			adigit=ch/0010;
			ch-=adigit*0010;
			*dst++=octal_digit[adigit];
			*dst++=octal_digit[ch];
		}
	}
	len=dst-buf;
	if(buf[len-1]=='\n') {
		previous_string_ended_in_cr=true;
		len--;
	}
	if(len!=0)
 		TEInsert(buf,len,gl.TEH);
}

void install_window_contents(void);
void install_window_contents()
{
	Rect vScrollRect;
	Rect viewRect;
	gl.save_clip_in_auto_scroll=NewRgn();
	TextFont(4);
	TextSize(9);
	vScrollRect=gl.win->portRect;
	vScrollRect.left=vScrollRect.right-15;
	vScrollRect.right+=1;
	vScrollRect.bottom-=14;
	vScrollRect.top-=1;
	gl.vScroll=NewControl(gl.win,&vScrollRect,"\p",1,0,0,0,
		scrollBarProc, 0L);

	viewRect=gl.win->portRect;
	viewRect.right-=SBarWidth;
	viewRect.bottom-=SBarWidth;
	InsetRect(&viewRect,4,4);
	gl.TEH=TENew(&viewRect,&viewRect );
	SetView();
	(*gl.TEH)->clikLoop=AutoScrollProc;
	set_widths();
}

void create_window(THExecRef xecRef);
void create_window(THExecRef xecRef)
{
	make_wm_window(xecRef);
	install_window_contents();
}


void doIdle(THExecRef xecRef);
void doIdle(xecRef)
THExecRef xecRef;
{
#pragma unused(xecRef)
#ifdef XXXYYY
	user iterface guid says read only windows don't blink intsert bar
	TEIdle(gl.TEH);
#endif
	if((gl.win->portRect.bottom!=gl.LastKnownSize.bottom)||
		(gl.win->portRect.right!=gl.LastKnownSize.right)||
		(gl.win->portRect.top!=gl.LastKnownSize.top)||
		(gl.win->portRect.right!=gl.LastKnownSize.right)) {
		gl.LastKnownSize=gl.win->portRect;
		WindowSizeHasChanged();
	}
}

#define TRIM_TRIGGER_SIZE (29000)
#define TRIM_HYSTERESIS (6000)

/*
 * cut some data off the start of a handle
 * returns true if it is now empty
 */
void trim_handle(Handle ahan,int shorten_amount);
void trim_handle(Handle ahan,int shorten_amount)
{
	int new_size=GetHandleSize(ahan)-shorten_amount;
	register char *src;
	register char *dst;
	register int i;
	dst=(*ahan);
	src=dst+shorten_amount;
	for(i=0;i<new_size;i++)
		*dst++=*src;
	SetHandleSize((Handle)gl.TEH, new_size);	
}

void append_to_log(char *astr,int len);
void append_to_log(char *astr,int len)
{
	int buflen;
	buflen=(*gl.TEH)->teLength;
	if(buflen>TRIM_TRIGGER_SIZE) {
		trim_handle((*gl.TEH)->hText,TRIM_HYSTERESIS);
		TECalText(gl.TEH);
		buflen=(*gl.TEH)->teLength;
	}
   	TESetSelect(buflen,buflen,gl.TEH);
   	insert_nasty_string(astr,len);
 	gl.apply_speed_limit= -1;	/*speed limit doesn't apply to insert*/
 	ScrollSelectionIntoView();
}

long strnchr(char *astr,char ch,long limit);
long strnchr(astr,ch,limit)
register char *astr;
register char ch;
register long limit;
{
	char *orig_astr=astr;
	while(limit-- >0) {
		if(*astr++ ==ch)
			return astr-orig_astr-1;
	}
	return -1;
}

long strlen(char *astr);
long strlen(astr)
register char *astr;
{
	register long result=0;
	while(*astr++ !=0)
		result++;
	return result;
}

void cat_to_log(char *astr);
void cat_to_log(char *astr)
{
  append_to_log(astr,strlen(astr));
}

void report_msg(char *kind,THExecRef xecRef);
void report_msg(char *kind,THExecRef xecRef)
{
	char buf[kMsgMaxData+20];
	register char *dst=buf;
	register char *src=(*xecRef)->xMsg->msgData;
	register char ch;
	register int i;
  	if(!((*xecRef)->xMsg->msgValid))
  		return;
  	while ((ch=*kind++)!=0)
  		*dst++=ch;
  	for(i=(*xecRef)->xMsg->msgLength;i>0;i--)
  		*dst++=*src++;
  	*dst++='\n';
  	append_to_log(buf,dst-buf);
}

void doSend(THExecRef xecRef);
void doSend(xecRef)
THExecRef xecRef;
{
	if(gl.win==nil)return;
	report_msg(">",xecRef);
}

void doReceive(THExecRef xecRef);
void doReceive(xecRef)
THExecRef xecRef;
{
	if(gl.win==nil)return;
	report_msg("<",xecRef);
}

void doFilter(THExecRef xecRef);
void doFilter(xecRef)
THExecRef xecRef;
{
	if(gl.win==nil)return;
	report_msg("<",xecRef);
}

/*
 * This isn't a resource to make
 * installing it simpler
 */
char help_text[]=
 "! CMU Mac WorkStation message watcher\n"
 "! by Aaron Wohl, aw0g+@andrew.cmu.edu (412)268-5032\n"
 "! \n"
 "! Modified by Garth Cummings, Apple Computer, Inc.\n"
 "! \n"
 "! cmd-c       => copy to clipboard stripping leading '<' and '>'\n"
 "! cmd-c-shift => copy to clipboard without stripping leading '<' and '>'\n"
 "! cmd-v       => Paste to local MacWorkStation\n"
 "! cmd-v-shift => Paste to host\n"
 "! cmd-h       => This help message\n"
 "! Start this exec in CCL logon script to see all messages eg 'Exec 200'\n"
 ;

void help(void);
void help()
{
	cat_to_log(help_text);
}

void doInvoke(THExecRef xecRef);
void doInvoke(xecRef)
THExecRef xecRef;
{
	MAKE_WINDOW_IF_NEEDED
	if(gl.win == nil) return;
	help();
}

void close_window(void);
void close_window()
{	
	if(gl.TEH != nil)
		TEDispose(gl.TEH);
	gl.TEH = nil;
	
	if (gl.win != nil)
		DisposeWindow(gl.win);
	gl.win = nil;
	gl.AreActive = false;
}


void doUnload(THExecRef xecRef);
void doUnload(xecRef)
THExecRef xecRef;
{
#pragma unused(xecRef)
	close_window();
}

void doCommand(THExecRef xecRef);
void doCommand(xecRef)
THExecRef xecRef;
{
	MAKE_WINDOW_IF_NEEDED
	if (gl.win == nil) return;
	report_msg("*",xecRef);
}

void doActivate(THExecRef xecRef);
void doActivate(xecRef)
THExecRef xecRef;
{
	Rect r;
	r=(*gl.win).portRect;
	r.top=r.bottom - (SBarWidth+1);
	r.left=r.left - (SBarWidth+1);
	InvalRect(&r);
	gl.AreActive=(*xecRef)->xActive;
	if(gl.AreActive) {
		TEActivate(gl.TEH);
		ShowControl(gl.vScroll);
		TEFromScrap();
	} else {
		TEDeactivate(gl.TEH);
		HideControl(gl.vScroll);
		ZeroScrap();
		TEToScrap();
	}
}

void draw_grow_with_no_horizontial_scroll(void);
void draw_grow_with_no_horizontial_scroll()
{
	Rect new_clip;
	GetClip(gl.save_clip_in_auto_scroll);
	new_clip=gl.win->portRect;
	new_clip.left=new_clip.right-SBarWidth+1;
	ClipRect(&new_clip);
	DrawGrowIcon(gl.win);
	SetClip(gl.save_clip_in_auto_scroll);
}

void doDraw(THExecRef xecRef);
void doDraw(xecRef)
THExecRef xecRef;
{
	WindowPtr theWindow;
	theWindow=(*xecRef)->xWindow;
	if(theWindow!=gl.win)
		SysBeep(20);
	BeginUpdate(theWindow);
	/* EraseRect(&theWindow->portRect); */
	EraseRect(&theWindow->portRect);
	DrawControls(gl.win);
	TEUpdate(&gl.win->portRect,gl.TEH);
	draw_grow_with_no_horizontial_scroll();
	EndUpdate(theWindow);
}

char ftolower(char ch);
char ftolower(char ch)
{
	if((ch>='A')&&(ch<='Z'))
		ch+=('a'-'A');
	return ch;
}


void UsedEvent(THExecRef xecRef);
void UsedEvent(THExecRef xecRef)
{
	(*xecRef)->xEvent->what=nullEvent;
	(*xecRef)->xKey=0;
}

/*
 * Filter out the < and > characters in buffer
 */
void FixTEScrap(void);
void FixTEScrap()
{
	register char *src;
	register char *dst;
	register long len;
	long new_len=0;
	register char ch;
	len=TEGetScrapLen();
	src=dst=(*TEScrapHandle());
#define GetCH {if(--len <0)goto FixTEExit; ch=*src++;}
	while (true) {
		GetCH
		if((ch=='>')||(ch=='<'))
		  GetCH;
		while(ch!='\n') {
			*dst++=ch;
			new_len++;
			GetCH
		}
		*dst++=ch;
		new_len++;
	};
#undef GetCH
	FixTEExit:
		TESetScrapLen(new_len);
}

/*
 * prepare the next message for transmision
 */
int extract_msg_from_scrap(TPMsg out);
int extract_msg_from_scrap(out)
register TPMsg out;
{
	long len;
	long i;

	if(gl.send_me==0)return false;
	len=GetHandleSize(gl.send_me);
	if(len<=0) {
		DisposHandle(gl.send_me);
		gl.send_me=0;
		return false;
	}
	i=strnchr(*gl.send_me,'\n',len);
	if(i==-1) {		/*no return so use rest of buffer*/
		i=len;
		len=0;
	} else 
		len-=i+1;	/*consume str including cr, but don't send it*/
	i=imin(i,kMsgMaxData);
	BlockMove((*gl.send_me),out->msgData,i);
	out->msgIndex=0;
	out->msgLength=i;
	out->msgValid=true;
	if(len<=0) {
		DisposHandle(gl.send_me);
		gl.send_me=0;
	} else
		trim_handle(gl.send_me,i);
	return true;
}

/*
 * Emit the text in the te scrap as in comming message
 * to macworkstation or an outgoing message to the host
 */
void EmitScrap(THExecRef xecRef,int to_host);
void EmitScrap(THExecRef xecRef,int to_host)
{
	char **to_send;

	if(gl.send_me!=0) {
		SysBeep(20);
		cat_to_log("! Sorry, a message paste is still in progress\n");
		return;
	}
	to_send=TEScrapHandle();
	if(HandToHand(&to_send)!=0) {
		SysBeep(20);
		cat_to_log("! Sorry, can't get memory for message paste\n");
		return;
	}
	SetHandleSize(to_send,TEGetScrapLen());
	gl.send_me=to_send;
	{
		TRMsg m;
		while (extract_msg_from_scrap(&m))
			if(to_host)
				MWS_Send(&m,xecRef);
			else
				MWS_Command(&m,xecRef);
	}
}

void doCmdKey(THExecRef xecRef);
void doCmdKey(xecRef)
THExecRef xecRef;
{
	char cmd_ch;
	cmd_ch=((*xecRef)->xEvent->message)& 0xFF;
	cmd_ch=ftolower(cmd_ch);
	switch(cmd_ch) {
	case 'c':
		TECopy(gl.TEH);
		UsedEvent(xecRef);
		if(((*xecRef)->xEvent->modifiers & shiftKey)==0)
			FixTEScrap();
		break;
	case 'v':
		EmitScrap(xecRef,((*xecRef)->xEvent->modifiers & shiftKey)!=0);
		UsedEvent(xecRef);
		break;
	case 'h':
		help();
		UsedEvent(xecRef);
		break;
	default:
		break;
	}
}

void doKey(THExecRef xecRef);
void doKey(xecRef)
THExecRef xecRef;
{
	if(!gl.AreActive)return;
	if(((*xecRef)->xEvent->modifiers & cmdKey)!=0)
		doCmdKey(xecRef);
	else {
		SysBeep(20);
		UsedEvent(xecRef);
	}
}

long last_click_time=0;

void not_double(void);
void not_double()
{
	last_click_time=0;
}

short is_double(Point here);
short is_double(Point here)
{
	long now=TickCount();
	static Rect click_local;
	static int dcount;
	int result;
	if(!PtInRect(here,&click_local))
		last_click_time=0;
	/* topLeft(click_local)=here; */
	click_local.top = here.v;
	click_local.left = here.h;
	/* botRight(click_local)=here; */
	click_local.bottom = here.v;
	click_local.right = here.h;
	
	InsetRect(&click_local,-CLICK_IN_SAME_SPOT,-CLICK_IN_SAME_SPOT);

	if((now-last_click_time)<GetDblTime())
		dcount++;
	else
		dcount=1;

	last_click_time=now;

	result=dcount;
	if(dcount>=3)
		dcount=0;

	return result;
}

void select_line(void);
void select_line()
{
	register int sel_start=(*gl.TEH)->selStart;
	register int sel_end=sel_start;
	register char *src;

	/*extend selection back to start of line*/
	if(sel_start!=0) {
		src=(*(*gl.TEH)->hText)+sel_start;
		do {
			if(*--src == '\n')
				goto found_start;
			} while(--sel_start>=0);
			sel_start=0;
	}
	found_start:

	/*extend selection forward to end of line*/
	if(sel_end!=0) {
		int size=(*gl.TEH)->teLength;
		src=(*(*gl.TEH)->hText)+sel_end;
		do {
			if(*src++ == '\n')
				goto found_end;
			} while(sel_end++<size);
			sel_end=size;
	}

	found_end:
	TESetSelect(sel_start,sel_end,gl.TEH);
}

void do_teclick(int double_count,Point here,int shift);
void do_teclick(int double_count,Point here,int shift)
{
	if(double_count>=3) {
		select_line();
	} else
	  TEClick(
		here,
		shift,
		gl.TEH);
}

void doMouse(THExecRef xecRef);
void doMouse(xecRef)
THExecRef xecRef;
{
	Point here;
	ControlHandle actl;
	int part;
	int double_count;
	here=((*xecRef)->xMousePt);
	GlobalToLocal(&here);
	double_count=is_double(here);
	part=FindControl(here,gl.win,&actl);
	if (part==nil) {
		if (PtInRect(here,&(*gl.TEH)->viewRect))
			do_teclick(
				double_count,
				here,
				((*xecRef)->xShift));
	} else if (part==inThumb) {
		TrackControl(actl,here, 0L);
		ScrollTextToMatchControl();
	}
	else
		TrackControl(actl,here,(ProcPtr)&ScrollProc);

}

void doKill(THExecRef xecRef);
void doKill(xecRef)
THExecRef xecRef;
{
#pragma unused(xecRef)
}

void doDismiss(THExecRef xecRef);
void doDismiss(xecRef)
THExecRef xecRef;
{
#pragma unused(xecRef)

	close_window();
}

void doStdMenuEvt(THExecRef xecRef);
void doStdMenuEvt(xecRef)
THExecRef xecRef;
{
#pragma unused(xecRef)

SysBeep(100);
}

pascal void EXEC(xecCode,xecRef)
short xecCode;
THExecRef xecRef;
{
	GrafPtr savePort;

#ifdef Debug
	DebugStr("\pEntering exec");
#endif

	if (xecCode == xCtlInvoke)
		doInvoke(xecRef);
	if ((!gl.AreActive) && (xecCode==xCtlIdle))
		return;
	
	GetPort(&savePort);
	
	if (gl.win != nil) SetPort(gl.win);
	
	switch(xecCode) {
		case xCtlIdle:			doIdle(xecRef); break;
		case xCtlSend:			doSend(xecRef); break;
		case xCtlReceive:		doReceive(xecRef); break;
		case xCtlFilter:		doFilter(xecRef); break;
		case xCtlUnload:		doUnload(xecRef); break;
		case xCtlCommand:		doCommand(xecRef); break;
		case xCtlActivate:		doActivate(xecRef); break;
		case xCtlDraw:			doDraw(xecRef); break;
		case xCtlKey:			doKey(xecRef); break;
		case xCtlMouse:			doMouse(xecRef); break;
		case xCtlKill:			doKill(xecRef); break;
		case xCtlDismiss:		doDismiss(xecRef); break;
		case xCtlStdMenuEvt:	doStdMenuEvt(xecRef); break;
		default:				break;
	}
	SetPort(savePort);
	
#ifdef Debug
	DebugStr("\pLeaving exec");
#endif

}
