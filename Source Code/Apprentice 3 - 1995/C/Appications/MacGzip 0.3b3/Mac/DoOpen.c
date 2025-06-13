/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "ICAPI.h"
#include "ICKeys.h"
#include "ICMappings.h"

#include "MacGzip.h"
#include "ThePrefs.h"
#include "MacErrors.h"

#include "tailor.h"

/* 22may95,ejo: added this for "cancel" constant */
#include <Dialogs.h>

/* 24may95,ejo: where does CtoPstr come from? */
#ifndef CtoPstr
#include <Strings.h>
#define CtoPstr(a)	c2pstr (a)
#endif

#define	GvRefNum	0	/* Default dir */

typedef struct {
		OSType	Type;
		OSType	Creator;
		short	Binary;
		short	BinHex;
		char	Suffix[32];
#ifdef	GET_DESC
		char	Description[256]; /* useless for me */
#endif
		Boolean	Found;
		}	TSufMap;
		
TSufMap	SufMap;

int			AsciiMode(char *name, int compress);
void		FixMacFile( char* name, int ascii, int decompress);
OSErr		OpenFile( Str255 fName);

extern int	gzip_main (Str255 fName, short Compress);


OSErr OpenFile( Str255 fName)
{
	char			sTemp[256], *c;

	Boolean			Compress;
	extern Boolean	modKey;
	
	ParamBlockRec	myParamBlock;
	FInfo			vFInfo;
	extern char		*get_suffix(char *name);
	extern char		z_suffix[MAX_SUFFIX+1];		/* default suffix */
	extern int		z_len;						/* strlen(z_suffix) */


/*********** ResFork ***************/

	myParamBlock.fileParam.ioCompletion =	NULL;
	myParamBlock.fileParam.ioNamePtr =		fName;
	myParamBlock.fileParam.ioVRefNum =		0;
	myParamBlock.fileParam.ioFRefNum =		0;
	myParamBlock.fileParam.ioFDirIndex =	0 ;
	
	PBGetFInfo( &myParamBlock, false );
	if ( myParamBlock.fileParam.ioFlRLgLen != 0 )
	{
		/* 24may95,ejo: that #s again */
		/*sprintf(sTemp,"Discard Resource Fork: %#s?",fName);*/
		strcpy (sTemp, "Discard Resource Fork: ");
		sTemp [strlen (sTemp) + *fName] = '\0';
		strncat (sTemp, (char *) &fName[1], *fName);
		if ( Cask( sTemp ) == cancel )
			return(Cancelled);
	}
	
/*	globals in gzip.c */

    strncpy(z_suffix, Z_SUFFIX, sizeof(z_suffix)-1);
	if ( *currPrefs.suffix != 0x00 )
	{
		/* 24may95,ejo: removed #s (not portable) */
		/*sprintf(z_suffix,"%#s",(char*)currPrefs.suffix);*/
		strncpy (z_suffix,
			(char *) &currPrefs.suffix[1], *currPrefs.suffix) [*currPrefs.suffix] = '\0';
#ifdef NO_MULTIPLE_DOTS
		if (*z_suffix == '.')
			memmove(z_suffix,z_suffix+1,strlen(z_suffix+1));
#endif
	}
	z_len = strlen(z_suffix);            

	if((currPrefs.StKeysAlt)&&(modKey))
	{
			Compress = true;
	}
	else
	{
		switch(currPrefs.WhenCompress)
		{
			case NonMacGzip:
				GetFInfo(fName,GvRefNum,&vFInfo);
				
				Compress =	(vFInfo.fdType != GZIP_ID) &&
							(vFInfo.fdType != 'ZIVU') &&
							(vFInfo.fdType != 'ZIVM') &&
							(vFInfo.fdType != 'pZIP');
							
				if (Compress) /* check suffixes */
				{
					/* 24may95,ejo: removed #s */
					/*sprintf(sTemp,"%#s",fName);*/
					strncpy (sTemp, (char *) &fName[1], *fName) [*fName] = '\0';

					c=get_suffix(sTemp);
					
					if ( c == NULL )
						Compress = true;
					else
						Compress = (*c == 0x00);
				}
				break;
				
			case G_UseMenu:
				
				Compress = currPrefs.compress;
				break;
		}
	}
	
	return ( (OSErr) gzip_main ( fName, Compress ));
}


/********************************************************************************
 *
 * Set the correct Creator & Type (called from create_outfile)
 *
 * When using Fetch prefs, this should be called after
 * AsciiMode
 */

void FixMacFile( char* name, int ascii, int decompress)
{
	FInfo	vFInfo;
	Str255	pname;
/*	register char	*ci, *co; */
			
	strcpy((char*)pname,name);
	CtoPstr((char*)pname);
	
	GetFInfo(pname,GvRefNum,&vFInfo);
	

	if ( decompress)
	{
		if ( SufMap.Found )
		{
			vFInfo.fdCreator = SufMap.Creator;
			vFInfo.fdType = SufMap.Type;
		}
		else
		{
			if(ascii)
			{
				/* We must avoid odd address error in 68000 */
				vFInfo.fdType='TEXT';
				BlockMove((char*)currPrefs.textcreator+1,& vFInfo.fdCreator, 4);	
			}
			else
			{
				BlockMove((char*)currPrefs.bincreator+1,& vFInfo.fdCreator, 4);
				BlockMove((char*)currPrefs.bintype+1,& vFInfo.fdType, 4);
			}
		}
	}
	else
	{
		vFInfo.fdType=GZIP_ID;
		vFInfo.fdCreator=GZIP_ID;
	}
	
	SetFInfo(pname,GvRefNum,&vFInfo);
}

/********************************************************************************
 *
 *		Find the correct Creator & Type when expanding
 * 
 *		Set AsciiMode
 */
 
int AsciiMode(char *name, int compress)
{
	
	FInfo	vFInfo;
	int		result, len;
	
	Ptr		pPrefs;
	char	*p, *n, *q;

	extern char		charKey;
	extern Handle	FPrefs;
	extern Size		FPrefsSize;

	/*
	 * Mac->Net
	 */
	if(compress) /*	name is Pascal string */
	{
		if((currPrefs.StKeysComp)&&(charKey!=0x00))
		{
			result = (charKey == 'a');
		}
		else
		{
			switch(currPrefs.AsciiCompress)
			{
				case OnlyText:
					GetFInfo((StringPtr)name,GvRefNum,&vFInfo);
					result = (vFInfo.fdType == 'TEXT');
					break;
					
				case Never:
					result = 0;
					break;
					
				case UseMenu:
					result = currPrefs.ascii;
					break;
				
				case UseICinComp:
					{
						long 		pos = 0;
						ICError 	err;
						ICMapEntry	entry;
						int			MatchingRank, BestMatchingRank = 0;
						
						GetFInfo((StringPtr)name,GvRefNum,&vFInfo);
						result = 0;
						
						while ( noErr == ( err = ICMGetEntry(ICmappings, pos, &entry)))
						{
							/*
							 *	1. type, creator and extension match	11
							 *	2. type and extension match				10
							 *	3. extension and creator match			7
							 *	4. extension matches					6
							 *	5. type and creator match				5
							 *	6. type matches							4
							 *	7. anything else
							 *
							 *	T=4, C=1, E=6
							 */
							 
							pos += entry.total_length;
							MatchingRank = 0;
							
							if (!(entry.flags & ICmap_not_outgoing_mask))
							{
								if( vFInfo.fdType == entry.file_type )
									MatchingRank += 4;
									
								if( vFInfo.fdCreator == entry.file_creator)
									MatchingRank += 1;
								
								len = (int) entry.extension[0];
								
								if (len !=0)
								{
									q = (char *) &entry.extension[1];
									
									for( p = q, n = name + name[0] +1 - len;
									(tolower(*p) == tolower(*n)) && (p<q+len);
									p++, n++)
									;								
									
									if ( n == name + name[0] +1  )
									{
										MatchingRank += 6;
									}						
								}

								if ( MatchingRank > BestMatchingRank )
								{
									BestMatchingRank = MatchingRank;
									result = !(entry.flags & ICmap_binary_mask);
								}
								
							}
						}/* while */
					}
				
					break;
					
				default:
					break;
			
			}
		}
	}
	else  /* OJO *//*	name is C string */
	/*
	 * Net -> Mac
	 */
	{
		SufMap.Found = false;
		
		if((currPrefs.StKeysComp)&&(charKey!=0x00))
		{
			result = (charKey == 'a');
		}
		else
		{
			switch(currPrefs.AsciiUnCompress)
			{
				case UseICinExp:
				
					{
						long 		pos = 0;
						ICError 	err;
						ICMapEntry	entry;
						
						
						while ( noErr == ( err = ICMGetEntry(ICmappings, pos, &entry)))
						{
							pos += entry.total_length;
							
							if (!(entry.flags & ICmap_not_incoming_mask))
							{
								SufMap.Type =		entry.file_type;
								SufMap.Creator =	entry.file_creator;
								SufMap.Binary =		(entry.flags & ICmap_binary_mask);
								SufMap.BinHex =		0;
								/*(entry.flags & ICmap_resource_fork_mask)*/
								
								len = (int) entry.extension[0];
								
								if (len !=0)
								{
									q = (char *) &entry.extension[1];
									
									for( p = q, n = name + strlen(name) - len;
									(tolower(*p) == tolower(*n)) && (p<q+len);
									p++, n++)
									;								
									
									if ( n == name + strlen(name) )
									{
										SufMap.Found = true;
										break;
									}						
								}
							}
						}

						if(SufMap.Found)
						{
							result = !SufMap.Binary;
						}
						else
						{
							result = 0;
						}

					}
					break;
					

					
				case Fetch:
				
					HLock( FPrefs );
					pPrefs = *FPrefs;
					
					

					while( pPrefs < *FPrefs+FPrefsSize )
					{
						SufMap.Type =		*(OSType *)pPrefs;			pPrefs += 4;
						SufMap.Creator =	*(OSType *)pPrefs;			pPrefs += 4;
						SufMap.Binary =		(*(short *)pPrefs != 0);	pPrefs += 2;
						SufMap.BinHex =		(*(short *)pPrefs != 0);	pPrefs += 2;
						
						p = SufMap.Suffix;
						while((*p++ = *(char *)(pPrefs++))!=0)
						;
						
#ifdef	GET_DESC
						p = SufMap.Description;
						while((*p++= *(char *)(pPrefs++))!=0)
						;
#else
						while(*(char *)(pPrefs++)!=0)
						;
#endif
						len = strlen(SufMap.Suffix);
						
						for( p = SufMap.Suffix, n = name + strlen(name) - len;
						(tolower(*p) == tolower(*n)) && (p<SufMap.Suffix+len);
						p++, n++)
						;
					
						if ( n == name + strlen(name) )
						{
							SufMap.Found = true;
							break;
						}						
					}

					HUnlock( FPrefs );

					if(SufMap.Found)
					{
						result = !SufMap.Binary;
					}
					else
					{
						result = 0;
					}
					
					break;

				case Never:
					result = 0;
					break;
					
				case UseMenu:
					result = currPrefs.ascii;
					break;
					
				default:
					break;
			}
		}
	}
	
	return (result);
}
