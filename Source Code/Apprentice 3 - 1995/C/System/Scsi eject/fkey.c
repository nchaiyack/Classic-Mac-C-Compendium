/*
an FKEY which ejects a disk in a SCSI device, trying all possible methods
to do that (except, obviously, pushing a mechanical eject button)


main written by Gabriele Speranza, 10 April 1993
the rest was taken from suntar 2.0 beta 5 (June 93)
bug fix for SyQuest 27 June 93
changed the behaviour for virtual memory (both because I'd found a bug
	in the old one, and because I wanted to release the full source
	and I preferred to avoid including the full, powerful SCSI routine
	used by suntar) 24 June 95


this program is free.
That includes the source code, and since it's so simple and small
probably it should be considered public domain. But I'm not concerned
about legal matters and I've never worried about the differences between
"free" and "public domain": in both cases, I'm the author and you
have some rights to use it, OK ?
*/

#define VM_COMPATIBLE


#include <SCSI.h>
typedef unsigned char byte;

#define SCSI_ID 3	/* !!!! change this according to your needs. I compiled */
					/* the FKEY six times for SCSI_ID from 1 to 6 */

short do_scsi_command_in(short,byte*,char*,long,short*,short*,unsigned long*);
void tape_load_unload(short,unsigned char);
void do_scsi_generic(short,byte,char*,short);
OSErr unmount_volume(short);
OSErr ID_to_drive_number(short,short*);
Boolean volume_in_place(short);
void fillmem(char*,char,short);
Boolean _TrapAvailable(short,TrapType);
int dummy_call(void);
void printf(char*, ...);

#define TEST_UNIT_READY 0
#define REQUEST_SENSE 0x3
#define LOAD_UNLOAD 0x1B
#define ALLOW_MEDIUM_REMOVAL 0x1E

OSErr err_code;
#ifdef VM_COMPATIBLE
Boolean		gHasVirtualMemory;
#endif


#if 0
/*  from TESample.c */
/*	Check to see if a given trap is implemented. This is only used by the
	Initialize routine in this program, so we put it in the Initialize segment.
	The recommended approach to see if a trap is implemented is to see if
	the address of the trap routine is the same as the address of the
	Unimplemented trap. */
/*	1.02 - Needs to be called after call to SysEnvirons so that it can check
	if a ToolTrap is out of range of a pre-MacII ROM. */

Boolean TrapAvailable(tNumber,tType)
	short		tNumber;
	Trap_Type	tType;
{
	if ( ( tType == (unsigned char) ToolTrap ) &&
		( gMac.machineType > envMachUnknown ) &&
		( gMac.machineType < envMacII ) ) {		/* it's a 512KE, Plus, or SE */
		tNumber = tNumber & 0x03FF;
		if ( tNumber > 0x01FF )					/* which means the tool traps */
			tNumber = _Unimplemented;			/* only go to 0x01FF */
	}
	return NGetTrapAddress(tNumber, tType) != GetTrapAddress(_Unimplemented);
} /*TrapAvailable*/
#else
/* this one is suggested by Inside Mac volume VI, hence probably it's better;
but we did not want to have three function to do what a single one would do,
and a #define for _GetTrapType is evaluated at compile time... */

#define _GetTrapType(theTrap) ((theTrap & 0x0800) !=0 )
#define TrapAvailable(theTrap) _TrapAvailable(theTrap,_GetTrapType(theTrap) )
#define _InitGraf		0xA86E
#define _Gestalt		0xA1AD
#define _Unimplemented	0xA89F

#define TrapType Trap_Type
typedef unsigned char Trap_Type;		/* changed name, to avoid a conflict with
										newer standard headers which might have that
										declaration */


#ifdef VM_COMPATIBLE
Boolean	_TrapAvailable(short theTrap,TrapType tType)
{
	/* TrapType tType;

	tType = GetTrapType(theTrap); */
	if (tType == ToolTrap) {
		theTrap &= 0x07FF;
		if (theTrap >= 
			(NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E,ToolTrap) ?
				0x0200 : 0x0400) )
			theTrap = _Unimplemented;
	}

	return NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap);
}
#endif

#endif


#if 0
static void print_mess(p)
char* p;
{
	static Rect winRect={30,30,110,330},
		boundsR1={15,0,80,300};
	WindowPtr theWindow;
	GrafPtr	savePort;
	int i=0; char*q;
	q=p; while(*q++) i++;

	theWindow=NewWindow( NULL, &winRect, "\p", 1, dBoxProc, (WindowPtr)-1L, 0, (long)-1);
	GetPort( &savePort );
	SetPort(theWindow);

	TextBox(p,(long)i,&boundsR1,teJustCenter);
	
	while(!Button()) ;
	while( Button()) ;
	SetPort (savePort);
	DisposeWindow(theWindow);
}
#endif


void main()
{
short drn;
short scsiID=SCSI_ID;

#if 0	/* used for testing: the project was initially configured to be an application, */
		/* which used print_mess or even printf to print was was happening;
		the final thing was recompiled as code resource et voila, an FKEY
		was born */
	InitGraf(&thePort);
	InitFonts();
	FlushEvents( everyEvent, 0 );
	InitWindows();
	InitMenus();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
#endif

#ifdef VM_COMPATIBLE
	gHasVirtualMemory=false;
	if(TrapAvailable(_Gestalt /*, ToolTrap*/)){
		long response;
		OSErr err;
		/*#define HAS_GESTALT_GLUE*/
		#ifdef HAS_GESTALT_GLUE
		err=Gestalt('vm  ',&response);
		#else
		asm{
			move.l #'vm  ',D0
			dc.w _Gestalt
			move.l A0,response
			move.w D0,err
			}
		#endif
		if(err) response=0;
		gHasVirtualMemory=response&1;
		}
#endif

/*printf("VM:%d\n",gHasVirtualMemory);*/

if(!ID_to_drive_number(scsiID,&drn)){
	
	unmount_volume(drn);
	/* if the disk is not present in the drive calling Eject may freeze the
	Mac (which tries to deinstall a VBL which is not there), at least that
	happens with my SyQuest. That's a bug of the driver, but why risk ?
	Obviously I won't use this FKEY for the SyQuest which does not eject, but
	other drives might have the same problem, maybe using the same driver */
	if(!volume_in_place(scsiID) || Eject(NULL,drn)){
		tape_load_unload(scsiID,2);
		}
	}
else{
	#if 0
	do_scsi_generic(scsiID,TEST_UNIT_READY,NULL,0);
	/* 0 disco c'� 1 disco non c'� (o occupato...) <0 errore */
	if(err_code>=0)
	#endif
		tape_load_unload(scsiID,2);
	}
}

void fillmem(dest, ch, len)
register char*dest;
register char ch;
register short len;
{
	while (--len>=0) *dest++ = ch;
}

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

Boolean volume_in_place(scsiID)
short scsiID;
{
DrvQEl *q;
q=GetDrvQHdr()->qHead;
while(q){
	if(~(scsiID+32)==q->dQRefNum){
		if( *(((char*)q)-3)==1 || *(((char*)q)-3)==2 ) return true;
		}
	q=q->qLink;
	}
return false;
}


OSErr unmount_volume(vrefn)
short vrefn;
{
short err;
if((err=UnmountVol (NULL,vrefn))==noErr||err==nsvErr){
	return err==nsvErr;	/* 0 no err, 1 no such volume */
	}
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


void tape_load_unload(scsiID,load)
short scsiID;
byte load;	/* 0= unload (sequential access devices i.e. tapes)/stop motor (direct
				access devices i.e. removable disks, CD-ROMs etc.)
			1=load (seq. acc)/start motor (dir. acc.)
			2=Retension (seq. acc.)/Eject (direct acces devices, but also a number
			  of tape units interpret it in this way) */ 
{
	byte scsiCMD[6];
	short returnedStat,returnedMessage;
	fillmem(scsiCMD,0,sizeof(scsiCMD));
	scsiCMD[0]=LOAD_UNLOAD;

	if(load!=1){
		do_scsi_generic(scsiID,ALLOW_MEDIUM_REMOVAL,NULL,0);
		/* if(err_code) return; well, it's better to continue anyway, maybe
		this error does not imply a failure in the unload */
		}
	scsiCMD[4] = load ;

	scsiCMD[1] = 1;	/* return immediately */
	do_scsi_command_in(scsiID,scsiCMD,NULL,0,&returnedStat,&returnedMessage,NULL);
}

static void do_scsi_generic(scsiID,command,buffer,buf_size)
short scsiID;
byte command;
char*buffer;
short buf_size;
{
	byte scsiCMD[12];
	short s,err,returnedStat,returnedMessage;
	fillmem(scsiCMD,0,sizeof(scsiCMD));

		/*set up the command buffer with the command*/

	s=buf_size;
	scsiCMD[0] = command;
	err=do_scsi_command_in(scsiID,scsiCMD,buffer,s,&returnedStat,&returnedMessage,NULL);

/*printf("err, code, stat,mes=%d %d %d %d\n",err,err_code,returnedStat,returnedMessage);*/
	if(err||returnedMessage)
		err_code=-1;
	if(!err_code&&returnedStat) err_code=command==TEST_UNIT_READY ? 1 : -1;	/* per TEST_UNIT_READY returnStat non � un codice
									di errore */
}



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

#ifdef VM_COMPATIBLE
	/*
	That's the problem: virtual memory keeps some parts of memory on the disk,
	and when it's needed, a "page fault" is generated which loads the page.
	But since the SCSI bus is occupied between the SCSIGet and SCSIComplete commands,
	the hard disk can't be accessed and the page fault can't be served: result,
	the computer is freezed and must be reset. That's absolutely intolerable
	for the user and to avoid that I must be sure that between the call to
	SCSIGet and the return from SCSIComplete there is no access to any page
	which is not currently in main memory.
	  This solution is certainly worse than the one used in suntar: anyway,
	this is the idea: any virtual memory manager avoids to swap out pages used
	very very recently and if this function is smaller than a page, having
	just executed its first instructions and dummy_call (placed just after its end)
	guarantees that the whole function body is in recently accessed pages and will
	not be swapped out in the immediate future. Furthermore, the contents of
	"buffer" have been filled very recently by the caller, and dummy_call
	"touched" enough stack space to cover the needs of the SCSI manager functions
	called from here.
	  But that's a dirty trick and it's NOT guaranteed to work in any case:
	for more serious work, it's better to adopt the "officially correct" way of
	using HoldMemory, as suntar does
	*/
	
	if(gHasVirtualMemory) dummy_call();
#endif

	do{
		err = SCSIGet();
		if(err == noErr){
			waitTime=Ticks;
			err = SCSISelect(scsiID);
			waitTime=Ticks-waitTime;
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
				err=noErr;		/* questo comando non richiede dei dati indietro */
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
	/*printf("ReturnedStat, ReturnedMessage = %d %d\n\n", *returnedStat, *returnedMessage);*/
	if(!gotSCSIBus) return -3;

	return err ? -4 : 0;
}

#ifdef VM_COMPATIBLE
static int dummy_call()
{
char stack_buf[100];
return stack_buf[0]+stack_buf[99];
}
#endif
