#define FILE_NUM 50
/* Copyright (c) 1992 by Qualcomm, Inc */
/**********************************************************************
 * the log file
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Log

void OpenLog(void);

/************************************************************************
 * ComposeLogR - compose a log file entry, using a resource for a format
 ************************************************************************/
UPtr ComposeLogR(short level,UPtr into,short format,...)
{
	Str255 locl;
	UPtr reallyInto = into ? into : locl;
	va_list args;
	va_start(args,format);
	(void) VaComposeRString(reallyInto,format,args);
	va_end(args);
	Log(level,reallyInto);
	return(into);
}

/************************************************************************
 * ComposeLogS - compose a log file entry, using a string for a format
 ************************************************************************/
UPtr ComposeLogS(short level,UPtr into,UPtr format,...)
{
	Str255 locl;
	UPtr reallyInto = into ? into : locl;
	va_list args;
	va_start(args,format);
	(void) VaComposeString(reallyInto,format,args);
	va_end(args);
	Log(level,reallyInto);
	return(into);
}

/************************************************************************
 * Log - log a string, if the current log level includes the current string
 ************************************************************************/
UPtr Log(short level,UPtr string)
{
	if ((level&LogLevel)!=0)
	{
		OpenLog();
		if (LogRefN)
		{
			Str31 stamp;
			long tickDiff = TickCount()-LogTicks;
			FSWriteP(LogRefN,ComposeString(stamp,"\p%d:%d.%d.%d ",level,
															tickDiff/3600,(tickDiff/60)%60,tickDiff%60));
			FSWriteP(LogRefN,string);
			if (string[*string] != '\n') FSWriteP(LogRefN,"\p\n");
		}
	}
	return(string);
}

/************************************************************************
 * CloseLog - close, rolling over if needed
 ************************************************************************/
void CloseLog(void)
{
	long eof=0;

	if (LogRefN)
	{
	  eof = (GetEOF(LogRefN,&eof) || eof > 100K);
		FSClose(LogRefN);
		LogRefN = 0;
	}
	if (eof)
	{
		Str31 old,new;
		GetRString(old,OLD_LOG);
		GetRString(new,LOG_NAME);
		HDelete(MyVRef,MyDirId,old);
		HRename(MyVRef,MyDirId,new,old);
	}
}


/************************************************************************
 * OpenLog - open log file if needed, rolling over if needed
 ************************************************************************/
void OpenLog(void)
{
	long eof;
	Str255 str;
	Str15 ctext;
	short err;
	long creator;
	
	if (LogRefN && (GetEOF(LogRefN,&eof) || eof > 100K)) CloseLog();
	if (!LogRefN)
	{
		GetRString(str,LOG_NAME);
		if ((err=FSHOpen(str,MyVRef,MyDirId,&LogRefN,fsRdWrPerm))==fnfErr)
		{
			GetPref(ctext,PREF_CREATOR);
			if (*ctext!=4) GetRString(ctext,TEXT_CREATOR);
			BlockMove(ctext+1,&creator,4);
			HCreate(MyVRef,MyDirId,str,creator,'TEXT');
			err = FSHOpen(str,MyVRef,MyDirId,&LogRefN,fsRdWrPerm);
	  }
		if (err) return;
		SetFPos(LogRefN,fsFromLEOF,0);
		LogTicks = TickCount();
		LocalDateTimeStr(str);
		FSWriteP(LogRefN,str);
	}
}

/************************************************************************
 * 
 ************************************************************************/
void LogAlert(short template)
{
	ComposeLogS(LOG_ALRT,nil,"\pALRT %d",template);
}

/************************************************************************
 * 
 ************************************************************************/
void MyParamText(PStr p1,PStr p2,PStr p3,PStr p4)
{
	if (p1&&*p1) Log(LOG_ALRT,p1);
	if (p2&&*p2) Log(LOG_ALRT,p2);
	if (p3&&*p3) Log(LOG_ALRT,p3);
	if (p4&&*p4) Log(LOG_ALRT,p4);
	ParamText(p1,p2,p3,p4);
}

/************************************************************************
 * CarefulLog - log a potentially long and nasty string
 ************************************************************************/
void CarefulLog(short level,short format,UPtr data,short dSize)
{
	Byte logString[128];
	UPtr to, dataEnd, logEnd;
	
	if (!(level&LogLevel)) return;
	
	dataEnd = data+dSize;
	do
	{
		to = logString+1;
		logEnd = to + sizeof(logString)-6;
		while (data<dataEnd)
		{
			if (*data < ' ')
			{
				*to++ = '\\';
				switch (*data)
				{
					case '\r': *to++ = 'n'; break;
					case '\n': *to++ = 'r'; break;
					case '\t': *to++ = 't'; break;
					default:
						*to++ = '0'+(*data/64)%8;
						*to++ = '0'+(*data/8)%8;
						*to++ = '0'+*data % 8;
						break;
				}
			}
			else
				*to++ = *data;
			data++;
			if (to>logEnd || data[-1]=='\r') break;
		}
		*logString = to-logString-1;
		ComposeLogR(level,nil,format,logString);
	}
	while (data<dataEnd);
}
