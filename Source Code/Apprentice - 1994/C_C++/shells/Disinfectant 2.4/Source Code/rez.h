/*______________________________________________________________________

	rez.h - Sample Resource File Constants and Other Constants.
	
	Copyright © 1988, 1989, 1990, Northwestern University.
	
	The constants defined in this header file must match the definitions
	in the file rez.r.
_____________________________________________________________________*/

#ifndef __rez__
#define __rez__


#define	mBarID				128		/* menu bar */

#define	appleMID				128 		/* menu id of apple menu */
#define	aboutCommand		1			/* item number of about command */
#define	helpCommand			2			/* item number of help command */

#define	fileMID 				129 		/* menu id of file menu */
#define	closeCommand		1			/* item number of close command */
#define	saveAsCommand		2			/* item number of save as command */
#define	pageSetupCommand	4			/* item number of page setup command */
#define	printCommand 		5			/* item number of print command */
#define	printOneCommand	6			/* item number of print one command */
#define	prefsCommand		8			/* item number of preferences command */
#define	quitCommand 		10			/* item number of quit command */

#define	editMID 				130 		/* menu id of edit menu */
#define	undoCommand 		1			/* item number of undo command */
#define	cutCommand			3			/* item number of cut command */
#define	copyCommand 		4			/* item number of copy command */
#define	pasteCommand		5			/* item number of paste command */
#define	clearCommand		6			/* item number of clear command */

#define	scanMID				131		/* menu id of scan menu */
#define	fileCommand			1			/* item number of file command */
#define	folderCommand		2			/* item number of folder command */
#define	floppiesCommand	3			/* item number of floppies command */
#define	allDisksCommand	4			/* item number of all disks command */
#define	someDisksCommand	5			/* item number of some disks command */
#define	sysFileCommand		6			/* item number of sys file command */
#define	sysFolderCommand	7			/* item number of sys folder command */
#define	desktopCommand		8			/* item number of desktop files command */

#define	numMenus				4			/* number of menus */

#define	mainWindID		128		/* main window */

#define	driveID			128		/* drive button */
#define	ejectID			129		/* eject button */
#define	scanID			130		/* scan button */
#define	cancelID			131		/* cancel button */
#define	quitID			132		/* quit button */
#define	resetID			133		/* reset button */
#define	firstControl	128		/* first control */
#define	numControls		6			/* number of controls */

#define	mainPopUpID		1000		/* menu id of main window popup menu */
#define	repHeadID		150		/* report header STR# rsrc */

#define	helpWindID		129		/* help window */

#define	prefsWindID		400		/* prefs dialog window */

#define	prefsNotTitle1	7			/* item number of first line of notification
												title */
#define	prefsNotTitle2	8			/* item number of second line of notification
												title */
#define	prefsNotTitle3	9			/* item number of third line of notification
												title */
#define	prefsFirstRule	10			/* item number of first rule useritem */
#define	prefsHorRule1	10			/* item number of first horizontal rule */
#define	prefsHorRule2	11			/* item number of second horizontal rule */
#define	prefsHorRule3	12			/* item number of third horizontal rule */
#define	prefsLastRule	16			/* item number of last rule useritem */
#define	prefsBeep		18			/* item number of num beeps edittext item */
#define 	prefsStation	20			/* item number of scanning station checkbox */
#define	prefsScan		21			/* item number of scan radio button */
#define	prefsDisinfect	22			/* item number of disinfect radio button */
#define	prefsRepLabel	23			/* item number of report creator label */
#define	prefsDocLabel	24			/* item number of doc creator label */
#define	prefsRepPop		25			/* item number of report creator popup menu */
#define	prefsDocPop		26			/* item number of doc creator popup menu */
#define	prefsRepType	27			/* item number of report creator edittext item */
#define	prefsDocType	28			/* item number of doc creator edittext item */
#define	prefsNotPict	29			/* item number of notification picture */
#define	prefsDiamond	30			/* item number of diamond only notif radio but */
#define	prefsIcon		31			/* item number of also icon notif radio but */
#define	prefsAlert		32			/* item number of also alert notif radio but */
#define	prefsDim			33			/* item number of dim notif alert section
												user item */

#define	prefsRepMenuID	200		/* menu id of report creator popup menu */
#define	prefsDocMenuID	201		/* menu id of doc creator popup menu */

#define	prefsRepCreaID	200		/* id of report creator list */
#define 	prefsDocCreaID	201		/* id of doc creator list */

#define	abouWindID		130		/* about window */

#define	abouInstrument	190		/* resource id of instrument for Sousa march */
#define	abouNotes		128		/* resource id of Sousa 'TUNE' */
#define	abouSplat		191		/* resource id of splat sound */
#define  abouCreditsID	129		/* about box credits STR# rsrc */
#define	abouVNamesID	130		/* about box virus names STR# rsrc */

#define	rectType			'RCT#'	/* rsrc type of rectangle list */
#define	rectID			128		/* rectangle list */
#define	volIconRect		0			/* vol sicn rect */
#define	volNameRect		1			/* vol name rect */
#define	foldIconRect	2			/* folder sicn rect */
#define	foldNameRect	3			/* folder name rect */
#define	fileIconRect	4			/* file sicn rect */
#define	fileNameRect	5			/* file name rect */
#define	thermRect		6			/* thermometer rect */
#define	counterRect		7			/* counter rect */
#define	reportRect		8			/* report rect */
#define	docRect			9			/* help window doc text rect */
#define  tconRect			10			/* help window tcon rect */
#define	meRect			11			/* about box picture of me rect */
#define	creditsRect		12			/* about box credits rect */
#define	footRect			13			/* about box foot rect */
#define	numRects			14			/* number of rectangles */

#define	repFullID		128		/* report full alert */
#define	badSysID			129		/* bad system alert */
#define	saveRepID1		130		/* save report alert on quit */
#define	unlockID			131		/* unlock and reinsert alert */
#define	diskFullID		132		/* disk full alert */
#define  restartID      133      /* restart alert */
#define	oldRestartID	134		/* restart alert for old systems */
#define	noMemID			135		/* out of memory alert */
#define	noHFSID			136		/* need HFS alert */
#define	unableRepID		137		/* unable to repair alert */
#define	saveRepID2		138		/* save report alert on clear */
#define	printErrID		139		/* printing error alert */
#define	noDriverID		140		/* missing printer driver alert */
#define	illegalSizeID	141		/* illegal font size alert */
#define	noPagesID		142		/* no pages in range alert */
#define	marTooBigID		143		/* margins too big alert */
#define	unlockSSID		144		/* unlock and reinsert alert for scanning
												stations auto floppy repair mode */
#define	initRestartID	145		/* restart after installing INIT alert */
#define	initOldRestartID	146	/* restart after installing INIT alert
												for old systems */
#define	replaceINITID	147		/* replace exiting INIT file alert */
#define	diskLockedID	148		/* disk locked alert */
#define	unexpectedSaveID	149	/* unexpected error on file save alert */
#define	noPrinterID		150		/* no printer selected alert */
#define	printDskFullID	151		/* printing disk full alert */
#define	printLockedID	152		/* printing disk locked alert */
#define	updateINITID	153		/* install new INIT alert */
#define	fileLockedID	154		/* file locked alert */
#define	truncRightID	155		/* page box too narrow alert */
#define	truncBottomID	156		/* page box too short alert */
#define	prefsBadID		157		/* prefs file insane alert */

#define	getFFID			200		/* get file or folder dialog */
#define	prDlogID			201		/* printing dialog */
#define	plsWaitID		202		/* please wait dialog */
#define	volListID		203		/* volume list dialog */
#define	getFileID		204		/* get file dialog */

#define	pageSetupID		300		/* extra items for Page Setup dlog */

#define  strListID		128		/* strings STR# resource */
#define	prefsVersStr	1			/* prefs file version number string */
#define	putPromptStr	2			/* main wind SFPutFile prompt */
#define	abPromptStr		3			/* help window SFPutFile prompt */	
#define	abDefNameStr	4			/* help window SFPutFile default file name */
#define	repContStr		5			/* report continuation message */
#define	infectedStr		6			/* file infected message */
#define	pInfectedStr1	7			/* file partially infected message 1 */
#define	lastModStr		8			/* last mod infected msg */
#define	ioRepairStr1	9			/* I/O error on repair msg 1 */
#define	ioRepairStr2	10			/* I/O error on repair msg 2 */
#define	ddStartStr		11			/* disk disinfection run started message */
#define	dsStartStr		12			/* disk scanning run started message */
#define	scanDateStr		13			/* run start/finish date/time message */
#define	ddEndStr			14			/* disk disinfection run complete message */
#define	dsEndStr			15			/* disk scanning run complete message */
#define	summaryStr		16			/* summary title line */
#define	totFilesStr		17			/* total files summary line */
#define	totErrorsStr	19			/* total errors summary line */
#define	totInfectedStr	21			/* total infected files summary line */
#define	infWithStr		23			/* "files infected with" summary line */
#define	earInfectedStr	25			/* earliest infected file summary line */
#define 	allOKVolStr		26			/* no infections on disk msg */
#define	fDelStr			27			/* file deleted msg */
#define	fRepStr			28			/* file repaired msg */
#define	chkBusyStr		29			/* check file busy msg */
#define	repBusyStr		30			/* repair file busy msg */
#define	topsStr			31			/* can't scan sys files over TOPS */
#define	cancelStr		32			/* scan canceled msg */
#define	unexpectedStr	33			/* unexpected error msg */
#define	pInfectedStr2	34			/* file partially infected message 2 */
#define	allOKFoldStr	35			/* no infections in folder message */
#define	allOKFileStr	36			/* file not infected msg */												
#define	fdStartStr		37			/* folder disinfection run started msg */
#define	fsStartStr		38			/* folder scanning run started message */
#define	fdEndStr			39			/* folder disinfection run complete msg */
#define	fsEndStr			40			/* folder scanning run complete message */
#define	xdStartStr		41			/* file disinfection run started message */
#define	xsStartStr		42			/* file scanning run started message */
#define	xdEndStr			43			/* file disinfection run complete msg */
#define	xsEndStr			44			/* file scanning run complete message */
#define	nextDiskStr		45			/* please insert next disk msg */
#define	firstDiskStr	46			/* please insert first disk msg */
#define	restartStr		47			/* infected file in sys folder msg */
#define	oldRestartStr	48			/* infected file on sys disk msg */
#define	stillInfStr		49			/* file is still infected msg */
#define	opRepErrStr1	50			/* unexpected error on open for repair 1 */
#define	opRepErrStr2	51			/* unexpected error on open for repair 2 */
#define	mayStillInfStr	52			/* file may still be infected */
#define	memFullChkStr	53			/* mem full on check msg */
#define	memFullRepStr	54			/* mem full on repair msg */
#define	noPrivs1			55			/* no privs msg 1 */
#define	noPrivs2			56			/* no privs msg 2 */
#define	noPrivs3			57			/* no privs msg 3 */
#define	noPrivs4			58			/* no privs msg 4 */
#define	noPrivs5			59			/* no privs msg 5 */
#define	noPrivs6			60			/* no privs msg 6 */
#define	noPrivs7			61			/* no privs msg 7 */
#define	noPrivs8			62			/* no privs msg 8 */
#define	noPrivs9			63			/* no privs msg 9 */
#define	noPrivs10		64			/* no privs msg 10 */
#define	noPrivs11		65			/* no privs msg 11 */
#define	noPrivs12		66			/* no privs msg 12 */
#define	noPrivs13		67			/* no privs msg 13 */
#define	noPrivs14		68			/* no privs msg 14 */
#define	noPrivs15		69			/* no privs msg 15 */
#define	gateStr1			70			/* GateKeeper msg 1 */
#define	gateStr2			71			/* GateKeeper msg 2 */
#define	gateStr3			72			/* GateKeeper msg 3 */
#define	repFulStr		73			/* disk full on repair msg */
#define	errNoteStr1		74			/* error message note 1 */
#define	errNoteStr2		75			/* error message note 2 */
#define	errNoteStr3		76			/* error message note 3 */
#define	errRepStr1		77			/* error on repair msg 1 */
#define	errRepStr2		78			/* error on repair msg 2 */
#define	prbStillInfStr	79			/* probably still infected */
#define	ioChkStr1		80			/* i/o error on check msg 1 */
#define	ioChkStr2		81			/* i/o error on check msg 2 */
#define	noCheckStr1		82			/* damaged, cannot be checked msg 1 */
#define	noCheckStr2		83			/* damaged, cannot be checked msg 2 */
#define	damaged1			84			/* damaged by virus msg 1 */
#define	damaged2			85			/* damaged by virus msg 2 */
#define	damaged3			86			/* damaged by virus msg 3 */
#define	pInfAorB1		87			/* nVIR A or B partial infec msg 1 */
#define	prRepTitle		88			/* printed report title for header */
#define	prRepTmpl		89			/* printed report header template */
#define	hlpDocTitle		90			/* printed doc title for header */
#define	hlpDocTmpl		91			/* printed doc header template */
#define	counter1			92			/* "Files scanned" counter */
#define	counter2			93			/* "Infected files" counter */
#define	counter3			94			/* "Errors" counter */
#define	tconTitle		95			/* tcon title for help window */
#define	getFFPrompt		96			/* get file/folder prompt */
#define	getFoldPrompt	97			/* get folder prompt */
#define	getFilePrompt	98			/* get file prompt */
#define	scannedWord		99			/* "scanned" */
#define	disinfectedWord	100	/* "disinfected" */
#define	prefsFileName	101		/* "Sample Prefs" file name */
#define	infecNotif		102		/* infected file found notification */
#define	requiresAttn	103		/* requires attention notification */
#define	extractPrompt	104		/* prompt string for Extract INIT */
#define	initDefName		105		/* default INIT name */
#define	useFinderStr	106		/* use Finder instead of MultiFinder */
#define	badDiskStr1		107		/* bad disk inserted msg 1 */
#define	badDiskStr2		108		/* bad disk inserted msg 2 */

#define	firstDocID			200		/* first doc STR# rsrc */
#define	tconID				128		/* TCON resource id */
#define	tagID					128		/* TAG resource id */
#define	cellID				128		/* CELL resource id */
#define	lDefID				128		/* LDEF for report module */

#define	iconID				128		/* icon */

#define	floppyIconID		128		/* small floppy icon */
#define	hardDriveIconID	129		/* small hard drive icon */
#define	folderIconID		130		/* small folder icon */
#define	fileIconID			131		/* small file icon */
#define	smallDfectIconID	132		/* small Sample icon */

#define	helpCursID			201		/* help question mark cursor */

#define	mePictID				128		/* about box picture of me */
#define	footPictID			129		/* about box picture of foot */

#endif