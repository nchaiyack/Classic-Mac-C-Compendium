/* list of ID's used to store preferences:

Folder/File: alias to folder/file
ticks: long
tstyl: TextStyle
ApplID: GetAppleNameAndID (OSType followed by packed pascal string)

id		type			description (Boolean: text for value == true)
----------------------------------------------
Vers	long			Preferences version
Ver-	long			Oldest version that can use this preference file

SvIP	long			Server IP
SvNa	Str255		Server name
SvPo	short			NNTP port (119)
STio	long			Server timeout in seconds
Mntp	long			Maximun number of idle nntp connections
Tntp	ticks			Max idle time for idle nntp connections
XHSz	long			XHDR batch size (number of articles)
Mart	long			Maximum number of articles in group database
CNAT	long			Minutes between check for new articles

WYPe	VRect			Your name preferences dialog VRect
WNPe	VRect			News-server preferences dialog VRect
WBPe	VRect			Binaries preferences dialog VRect
WEPe	VRect			Editor preferences dialog VRect
WMPe	VRect			Mailer preferences dialog VRect
WPas	VRect			AskPassword preferences dialog VRect
WIPe	VRect			Misc preferences dialog VRect
WIEx	VRect			Expire preferences dialog VRect

FBin	Folder		Folder to store extracted binaries in.
BLau	Boolean		Launch application after BinHex extract
BLOP	Boolean		Open document in BinHex appl (obsolote)
BLid	File			BinHexExtract appl
BiAs	Boolean		Ask for filename when extracting binaries (obsolote)
UUna	Boolean		Use uu-supplied filename (obsolote)

WSav	Boolean		Save windows info?
Wind	stream		Open-windows info
AuUp	Boolean		Auto update subscribed groups
ChNw	Boolean		Check for new groups at startup

WArt	VRect			Article window VRect
TSar	tstyl			Article body text style

WDis	VRect			DiscListView window VRect
TSdi	tstyl			Disclist text style

TSgl	tstyl			GroupList text style
TSgt	tstyl			GroupTree text style

FNot	Folder		Folder for note files

FEdi	Folder		Folder for edit of article to post
EDid	File			Alias of editor
EDsi	OSType		Signature of editor
DUSi	Boolean		Default use signature (obsolote)
Sigu	File			Signature file
EdHe	Boolean		Edit headers: put headers in article-file
EdSi	Boolean		Edit signature: put signature in article-file (obsolote)
WrLn	long			Auto wrap line length
Tran	Str255		Name of taBL resource (obsolote)
tabI	Str255		Name of tabI resource
tabO	Str255		Name of tabO resource
AMem	long			Article text cache pre fetch max size
2022	Boolean		Use ISO-2022 encoding when posting (Japanese)

NaOK	Boolean		@adr, Name & Orga is valid (can be used when posting)
@adr	Str255		@-address of person (eg. 'speck@dat.ruc.dk')
Name	Str255		name of person (eg. 'Peter Speck')
Orga	Str255		name of organization (eg. 'Roskilde UniversitetsCenter')
UNam	Str255		user name
Gate	Str255		password (crypted)
AlAu	Boolean		Always authenticate, even before NNTP asks for it.

EuAp	File			Eudora alias
EuOp	Boolean		Launch Eudora with document
Eudo	File			Eudora launch document

artS	short			Show all/new articles in TArticleListView
disS	short			Show all/todays/withNewArticles discussions in DiscList
Head	Boolean		Show article headers
RT13	Boolean		Use ROT13

Xday	long			No article has been added to it for XX days
Xsize	long			The database contains more than XXX threads:

*/