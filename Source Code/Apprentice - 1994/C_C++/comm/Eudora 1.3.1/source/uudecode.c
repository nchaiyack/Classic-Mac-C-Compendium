#define FILE_NUM 46
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/* Major modifications Copyright (c)1991-1992, Apple Computer Inc. */
/************************************************************************
 * functions to convert files from uuencoded applesingle (yuck!)
 * Major modifications (c)1991-1992, Apple Computer Inc.
 * released to public domain.
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Abomination

typedef struct
{
	uLong type;
	uLong offset;
	uLong length;
} Map, *MapPtr;

typedef struct
{
	uLong magic;
	uLong version;
	char homefs[16];
	uShort mapCount;
	Map maps[9];
} UUHeader;

typedef struct
{
	UUHeader header; /* AppleSingle header */
	AbStates state; /* Current decoder state */
	UHandle buffer; /* receive map buffer */
	short bSpot; /* current point in receive map buffer */
	short bSize; /* Size of receive map buffer */
	short vRef; /* volref num for file */
	short refN; /* current file refnum */
	Str63 name; /* real file name */
	Str63 tmpName; /* temporary file name */
	long offset; /* Offset into the stream */
	long currmap; /* Current map that we are working on, set in AbNextState */
	Boolean seenFinfo; /* Have we found the Finfo in the stream yet? */
	Boolean seenName; /* Have we found the real file name in the stream yet? */
	Boolean hasName; /* Are we going to find the real file name in the stream ? */
	Boolean usedTemp; /* Did we use a temporary name? */
	Boolean noteAttached; /* Did we attache the enclosure note yet? */
	FInfo info; /* Macintosh file information */
} UUGlobals, **UUGlobalsHandle;

UUGlobalsHandle UUG;
#define Header (*UUG)->header
#define HeaderData ((UPtr)&Header)
#define State (*UUG)->state
#define Buffer (*UUG)->buffer
#define BSpot (*UUG)->bSpot
#define BSize (*UUG)->bSize
#define VRef (*UUG)->vRef
#define Name (*UUG)->name
#define TmpName (*UUG)->tmpName
#define Maps Header.maps
#define Info (*UUG)->info
#define InfoData ((UPtr)&Info)
#define RefN (*UUG)->refN
#define Offset (*UUG)->offset
#define CurrMapNum (*UUG)->currmap
#define CurrMap Header.maps[(*UUG)->currmap]
#define SeenFinfo (*UUG)->seenFinfo
#define SeenName (*UUG)->seenName
#define HasName (*UUG)->hasName
#define UsedTemp (*UUG)->usedTemp
#define NoteAttached (*UUG)->noteAttached

short UULine(UPtr text, long size);
Boolean UUData(uShort byte);
short AbOpen(void);
short AbClose(void);
short AbWriteBuffer(void);
Boolean AbNameStuff(uShort byte);
Boolean AbNextState( void );
Boolean AbTempName( void );
Boolean AbSetFinfo(uShort byte);
Boolean BeginAbomination( void );

/************************************************************************
 * IsAbLine - does the UUencoded applesingle file begin?
 ************************************************************************/
#pragma segment POP
Boolean IsAbLine(UPtr text, long size)
{
	UPtr spot;
	Str63 name;
	char *namePtr;
	short i = 0;
	
	namePtr = name;
	if (size<11) return(False);
	if (strncmp(text,"begin ",6)) return(False);
	spot = text + 7;
	while (*spot>='0' && *spot<='7') spot++;
	if (*spot!=' ' || spot-text > 10) return(False);
	if (spot[1]=='\n') return(False);
	if( !BeginAbomination() ) return(False);
	namePtr++; /* Skip the size */
	while( (*spot++ != '\n') && (i < 63) ){
				*namePtr++ = *spot;
				i++;
	}
				if( i>27 ) i = 27; /* Trim filname so number can fit on end */
	name[0] = i;
	PCopy(Name,name);
	return(True);
}

Boolean BeginAbomination( void )
{
	if (UUG==nil)
	{
		if ((UUG=NewZH(UUGlobals))==nil) return( false );
		State = AbDone;
		Offset = -1;
								SeenFinfo = false;
								NoteAttached = false;
								SeenName = false;
								HasName = false;
								UsedTemp = false;
	}
	return( true );
}

/************************************************************************
 * SaveAbomination - returns the state of the converter
 ************************************************************************/
short SaveAbomination(UPtr text, long size)
{
	if (!text)
	{
		if (UUG)
		{
			if (AbClose()) BadBinHex = True;
			if (Buffer) DisposHandle(Buffer);
			ZapHandle(UUG);
		}
		return(AbDone);
	}
	if (!BeginAbomination()) return(AbDone);
	return(UULine(text,size));
}

#pragma segment Abomination
#define UU(c) (((c)-' ')&077)
/************************************************************************
 * UULine - handle a line of uuencoded stuff
 ************************************************************************/
short UULine(UPtr text, long size)
{
	short length = UU(*text);
	Boolean result=True;
	
	if ((size==3 || size==4) && !striscmp("end\n",text))
		return(SaveAbomination(nil,0));
		
	if (*text<' ' || *text>'`')
	{
		WarnUser(BINHEX_BADCHAR,*text);
		SaveAbomination(nil,0);
		BadBinHex = True;
		return(AbDone);
	}
	text++; size--;
	if (State==AbDone) State=NotAb;
	if (length<=0) return(State);
	if (text[size-1]=='\n') size--;
	if ((length+2)/3 != ((size*3)/4)/3)
	{
		WarnUser(UU_BAD_LENGTH,(length+2)/3-((size*3)/4)/3);
		SaveAbomination(nil,0);
		return(AbDone);
	}
			
	for (;length>0;text+=4,length-=3)
	{
		if (text[0]<' ' || text[0]>'`' || text[1]<' ' || text[1]>'`' ||
				length>1 && (text[2]<' ' || text[2]>'`') ||
				length>2 && (text[3]<' ' || text[3]>'`'))
		{
			WarnUser(BINHEX_BADCHAR,0);
			SaveAbomination(nil,0);
			BadBinHex = True;
			return(AbDone);
		}
		if (!(result=UUData(0xff & (UU(text[0])<<2 | UU(text[1])>>4)))) break;
		if (length>1 && !(result=UUData(0xff & (UU(text[1])<<4 | UU(text[2])>>2))))
			break;
		if (length>2 && !(result=UUData(0xff & (UU(text[2])<<6 | UU(text[3])))))
			break;
	}
	if (!result) return(SaveAbomination(nil,0));
	return(State);
}

/************************************************************************
 * UUData - handle a data character
 ************************************************************************/
Boolean UUData(uShort byte)
{
	Boolean result=True;
	
	Offset++;
	switch (State)
	{
		case NotAb:
								State = AbHeader;
		case AbHeader:
								HeaderData[BSpot++] = byte;
								if (BSpot>=(sizeof(UUHeader)-(sizeof(Map)*9))) {
												if( Header.magic != 0x00051600 ) {
																WarnUser(UU_BAD_ENCLOSURE,Header.magic);
																SaveAbomination(nil,0);
																BadBinHex = True;
																return(false);
												}
												if( (Header.version != 0x00010000) && (Header.version != 0x00020000) ) {
																WarnUser(UU_BAD_VERSION,Header.version);
																SaveAbomination(nil,0);
																BadBinHex = True;
																return(false);
												}
												if( (Header.mapCount<1) || (Header.mapCount>9) ) {
																WarnUser(UU_INVALID_MAP,Header.mapCount);
																SaveAbomination(nil,0);
																BadBinHex = True;
																return(false);
												}
								}
								if( BSpot>=(sizeof(UUHeader) - (sizeof(Map) * (9 - Header.mapCount))) ) {
												AbNextState();
												if(State != AbName){
																result = AbTempName();
												}
												BSpot = 0;
								}
								break;
								
	case AbName:
		if (Offset<CurrMap.offset) break;
		result = AbNameStuff(byte);
		break;
				
				case AbFinfo:
		if (Offset<CurrMap.offset) break;
		result = AbSetFinfo(byte);
		break;

	case AbResFork:
				case AbDataFork:
								if (Offset<CurrMap.offset) break;
								if (!CurrMap.length) {
												result = !AbClose();
												Offset--;AbNextState();
												if (result) {result=UUData(byte);}
								}
								else if (!RefN && AbOpen())
												result = False;
								else {
												(*Buffer)[BSpot++] = byte;
												CurrMap.length--;
												if (BSpot>=BSize && AbWriteBuffer()) result=False;
								}
								break;
								
				case AbSkip:
								if( Offset < CurrMap.offset ) break;
								if( !CurrMap.length ) {
												Offset--;
												AbNextState();
												BSpot = 0;
												result = UUData( byte );
								} else {
												CurrMap.length--;
								}
								break;
								
				default:
								/* invalid state! */
								WarnUser(UU_INVALID_STATE,0);
								result = False;
	}
	return(result); 		
}

Boolean AbTempName( void )
{
	Str63 name;
	short vRef;
	short err;

	UsedTemp = true;
	PCopy(name, Name);
	if (AutoWantTheFile(name,&vRef,False) || WantTheFile(name,&vRef))
	{
		VRef = vRef;
		PCopy(TmpName,name);
								PCopy(Name,name);
		if (err=MakeResFile(name,VRef,0,'CSOm','TEMP'))
		{
			FileSystemError(BINHEX_CREATE,name,err);
			(void) SaveAbomination(nil,0);
			return(False);
		}
				AbNextState();
		BSpot = 0;
	}
	else
	{
		State = AbDone;
		return(False);
	}
	
	return(True);
}


Boolean AbNameStuff(uShort byte)
{
	Str63 name;
	short vRef;
	short err;

	Name[++BSpot] = byte;
	if (BSpot<CurrMap.length) return(True);
				if (CurrMap.length > 27){ /* Trim name so number fits! */
								*Name = 27;
				} else {
				*Name = CurrMap.length;
				}
	PCopy( name, Name );
	SeenName = true;
	if( !UsedTemp ){
					if (AutoWantTheFile(name,&vRef,False) || WantTheFile(name,&vRef))
					{
								VRef = vRef;
								PCopy( Name, name );
								PCopy( TmpName, name );
								if (err=MakeResFile(name,VRef,0,'CSOm','TEMP'))
								{
									FileSystemError(BINHEX_CREATE,name,err);
									(void) SaveAbomination(nil,0);
									return(False);
								}
								AbNextState();
								BSpot = 0;
								if( SeenFinfo ){
												AddAttachNote(VRef,name,Info.fdCreator,Info.fdType);
												NoteAttached = true;
								}
					}
					else
					{
								State = AbDone;
								return(False);
					}
	} else {
		PCopy(name,Name);
				if( AutoWantTheFile(name,&vRef,False)){
								if( Rename( TmpName, VRef, name ) == noErr ){
												PCopy( Name, name );
												PCopy( TmpName, name );
								} else { /* Rename failed, name stays TmpName */
												PCopy( name, TmpName );
												PCopy( Name, name );
								}
				} else { /* Not auto downloading, so name has to remain the TMPname... */
								PCopy( name, TmpName ); /* For the attach note */
								PCopy( Name, name ); /* Set the real name back to the tmp name */
				}
				AbNextState();
				BSpot = 0;
				if( SeenFinfo ){
								AddAttachNote(VRef,name,Info.fdCreator,Info.fdType);
								NoteAttached = true;
				}
	}
	return(True);
}

Boolean AbSetFinfo(uShort byte)
{
	Str63 name;
	FInfo info;
	short err;

				if(!SeenFinfo){ /* Type 2 AppleSingle have additional FinderInfo (Sys 7) -- we don't care */
								if( BSpot<sizeof(FInfo) ){
												InfoData[BSpot++] = byte;
								} else {
												BSpot++;
								}
								if (BSpot<CurrMap.length) return(True);
								SeenFinfo = true;
				}
				PCopy( name, Name );
				if( (!NoteAttached && SeenName) || !HasName ){
								AddAttachNote(VRef,name,Info.fdCreator,Info.fdType);
				}
				info = Info;
				info.fdFlags &= ~fOnDesk;
				info.fdFlags &= ~fInvisible;
				info.fdFlags &= ~fInited;
				if (err=SetFInfo(name,VRef,&info))
				{
					FileSystemError(BINHEX_OPEN,name,err);
					(void) SaveAbomination(nil,0);
					return(False);
				}
				BSpot = 0;
				AbNextState();
				return(True);
}
/************************************************************************
 *
 ************************************************************************/
short AbOpen(void)
{
	Str63 name;
	short err;
	short refN;
	UHandle buffer;
	
	if (!Buffer)
		if (buffer=NuHandle(GetRLong(RCV_BUFFER_SIZE)))
			Buffer = buffer;
		else
			return(WarnUser(BINHEX_MEM,err=MemError()));
	BSize = GetHandleSize(Buffer);
	PCopy(name,Name);
	if (err=(State==AbResFork?OpenRF(name,VRef,&refN):FSOpen(name,VRef,&refN)))
		FileSystemError(BINHEX_OPEN,name,err);
	else
		RefN = refN;
	BSpot = 0;
	return(err);
}

/************************************************************************
 *
 ************************************************************************/
short AbClose(void)
{
	short wrErr=0;
	short err;
	
	if (!RefN) return(noErr);
	if (BSpot) wrErr = AbWriteBuffer();
	err = FSClose(RefN);
	if (!wrErr && err) FileSystemError(BINHEX_WRITE,Name,err);
	RefN = 0;
	BSpot = 0;
	return(wrErr ? wrErr : err);
}

/************************************************************************
 *
 ************************************************************************/
short AbWriteBuffer(void)
{
	long writeBytes = BSpot;
	int err;
	
	if (err=FSWrite(RefN,&writeBytes,LDRef(Buffer)))
		FileSystemError(BINHEX_WRITE,Name,err);
	UL(Buffer);
	BSpot = 0;
	return(err);
}

Boolean AbNextState( void )
{
				short i;
				long biggest = 0;

				CurrMapNum = 0;
				for( i=0; i<Header.mapCount; i++){ /* Find the biggest offset */
								if( Header.maps[i].offset > biggest ) biggest = Header.maps[i].offset + 1;
								if( Header.maps[i].type == 3 ) HasName = true;
				}
				
				for( i=0; i<Header.mapCount; i++){ /* Find the header that is next */ 	
								if ( (Header.maps[i].offset > Offset) && (Header.maps[i].offset < biggest) ) {
												CurrMapNum = i;
												biggest = CurrMap.offset + 1;
												switch (CurrMap.type){
																case 1: /* Data Fork */
																				State = AbDataFork;
																				break;
																case 2: /* Resource Fork */
																				State = AbResFork;
																				break;
																case 3: /* Real file name */
																				State = AbName;
																				break;
																case 9: /* File Finder information */
																				State = AbFinfo;
																				break;
																default:
																				AddAttachInfo( (int)UU_SKIP_MAP_INFO, (int)CurrMap.type );
																				State = AbSkip;
																				break;
												}
								}
				}
				return( true );
}

