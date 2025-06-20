#include "CatSearch.h"

#include <stdlib.h>		// bsearch
#include <stdarg.h>

#define NIL nil

void	AddDirectoryTreeToList(short vRefNum, long parID, Handle list);
void	AddDirectoryToList(long, Handle);
Boolean	IsDirectoryInList(long, Handle);
int		DirIDCompare(const void* key, const void* data);

OSErr
PBCatSearchIn(CSParamPtr pb, long parID)
{
	OSErr	err;
	long	loopy;
	long	recordsToMove;
	
	static	long	lastParID = 0;
	static	Handle	goodList = NIL;
	
	if (lastParID != parID)
	{
		unsigned long startTime, endTime;
		lastParID = parID;

		if (goodList)
			SetHandleSize(goodList, 0);
		else
			goodList = NewHandle(0);

		AddDirectoryTreeToList(pb->ioVRefNum, parID, goodList);
	}

	err = PBCatSearchSync (pb);
	
	//
	// Count downwards so that, as we remove unwanted records, we aren't
	// moving forward records that we'll eventually be deleting.
	//
	for (loopy = pb->ioActMatchCount - 1; loopy >= 0; --loopy)
	{
		if (!IsDirectoryInList(pb->ioMatchPtr[loopy].parID, goodList))
		{
			recordsToMove = (pb->ioActMatchCount - 1) - loopy;
			if (recordsToMove > 0)
			{
				BlockMove(	&pb->ioMatchPtr[loopy + 1],
							&pb->ioMatchPtr[loopy],
							recordsToMove * sizeof(FSSpec));
			}
		
			pb->ioActMatchCount--;
		}
	}

	return err;
}

void
AddDirectoryTreeToList(short vRefNum, long parID, Handle list)
{
	CInfoPBRec	pb;
	long		index = 1;
	OSErr		err;

	AddDirectoryToList(parID, list);
	
	pb.hFileInfo.ioNamePtr = NIL;
	pb.hFileInfo.ioVRefNum = vRefNum;

	do {
		pb.hFileInfo.ioDirID = parID;
		pb.hFileInfo.ioFDirIndex = index++;
		
		err = PBGetCatInfoSync(&pb);
		if (err == noErr)
		{
			if ((pb.hFileInfo.ioFlAttrib & ioDirMask) != 0)
			{
				AddDirectoryTreeToList(vRefNum, pb.hFileInfo.ioDirID, list);
			}
		}
	} while (err == noErr);
}

void
AddDirectoryToList(long dirID, Handle list)
{
	long	loopy;
	long	numberOfItems = GetHandleSize(list) / sizeof(long);
	long*	dirs = (long*) *list;
	
	for (loopy = 0; loopy < numberOfItems; loopy++)
	{
		if (dirs[loopy] == dirID)
		{
			// Hmmm ... it's already there...
			return;
		}
		if (dirs[loopy] > dirID)
		{
			Munger(list, loopy * sizeof(long), NIL, 0, &dirID, sizeof(long));
			return;
		}
	}
	
	// If we got here, we add it to the end of the list
	
	Munger(list, numberOfItems * sizeof(long), NIL, 0, &dirID, sizeof(long));
}

Boolean
IsDirectoryInList(long dirID, Handle list)
{
	Boolean result;
	long	numberOfItems = GetHandleSize(list) / sizeof(long);
	long*	dirs = (long*) *list;

	HLock(list);
	result = (bsearch(&dirID, dirs, numberOfItems, sizeof(long), DirIDCompare) != NIL);
	HUnlock(list);
	
	return result;
}

int
DirIDCompare(const void* key, const void* data)
{
	const long* dirID = (const long*) key;
	const long* listItem = (const long*) data;
	
	if (*dirID > *listItem)
		return 1;
	if (*dirID < *listItem)
		return -1;
	return 0;
}

OSErr
CatSearch(long* found, ...)
{
	static CSParam		pbFind;
	static CInfoPBRec	pSpec1;
	static CInfoPBRec	pSpec2;
	static char			pFindBuffer[1024];
	
	static Boolean		subdirectorySearch = FALSE;
	static long			searchDirectory;

	OSErr				result = noErr;
	Boolean				initOnly = FALSE;
	Boolean				restart;
	
	va_list				params;
	int					selector;
	
	
	*found = 0;

	va_start(params, found);
	restart = va_arg(params, int) != csContinue;
	va_end(params);

	if (restart)
	{
		subdirectorySearch = FALSE;

		pbFind.ioNamePtr = NIL;
		pbFind.ioVRefNum = 0;

		pbFind.ioMatchPtr = NIL;
		pbFind.ioReqMatchCount = 0;

		pbFind.ioSearchBits = 0;

		pbFind.ioSearchInfo1 = &pSpec1;
		pbFind.ioSearchInfo2 = &pSpec2;

		pbFind.ioSearchTime = 0;
		pbFind.ioCatPosition.initialize = 0;

		pbFind.ioOptBuffer = pFindBuffer;
		pbFind.ioOptBufSize = sizeof(pFindBuffer);

		memset(&pSpec1, 0, sizeof(pSpec1));
		memset(&pSpec2, 0, sizeof(pSpec2));

		va_start(params, found);

		while ((result == noErr) && ((selector = va_arg(params, int)) != csEndList))
		{
			switch (selector)
			{
				case csVRefNum:
				{
					pbFind.ioVRefNum = va_arg(params, short);
					break;
				}
				
				case csVNamePtr:
				{
					pbFind.ioNamePtr = va_arg(params, StringPtr);
					break;
				}
				
				case csMatchPtr:
				{
					pbFind.ioMatchPtr = va_arg(params, FSSpecPtr);
					pbFind.ioReqMatchCount = va_arg(params, long);
					break;
				}
				
				case csSearchTime:
				{
					pbFind.ioSearchTime = va_arg(params, long);
					break;
				}
				
				case csOptBuffer:
				{
					pbFind.ioOptBuffer = va_arg(params, Ptr);
					pbFind.ioOptBufSize = va_arg(params, long);
					break;
				}
				
				case csPartialName:
				{
					pbFind.ioSearchBits |= fsSBPartialName;
					pSpec1.hFileInfo.ioNamePtr = va_arg(params, StringPtr);
					break;
				}
	
				case csFullName:
				{
					pbFind.ioSearchBits |= fsSBFullName;
					pSpec1.hFileInfo.ioNamePtr = va_arg(params, StringPtr);
					break;
				}
	
				case csFlAttrib:
				{
					pbFind.ioSearchBits |= fsSBFlAttrib;
					pSpec1.hFileInfo.ioFlAttrib = va_arg(params, long);
					pSpec2.hFileInfo.ioFlAttrib = va_arg(params, long);
					break;
				}
	
				case csFlFndrInfo:
				{
					pbFind.ioSearchBits |= fsSBFlFndrInfo;
					
					pSpec1.hFileInfo.ioFlFndrInfo.fdType = va_arg(params, OSType);
					pSpec1.hFileInfo.ioFlFndrInfo.fdCreator = va_arg(params, OSType);
					pSpec1.hFileInfo.ioFlFndrInfo.fdFlags = va_arg(params, unsigned short);
					pSpec1.hFileInfo.ioFlFndrInfo.fdLocation = va_arg(params, Point);
					pSpec1.hFileInfo.ioFlFndrInfo.fdFldr = va_arg(params, short);
					
					pSpec2.hFileInfo.ioFlFndrInfo.fdType = -1;
					pSpec2.hFileInfo.ioFlFndrInfo.fdCreator = -1;
					pSpec2.hFileInfo.ioFlFndrInfo.fdFlags = -1;
					*(long*) &pSpec2.hFileInfo.ioFlFndrInfo.fdLocation = -1;
					pSpec2.hFileInfo.ioFlFndrInfo.fdFldr = -1;
				
					break;
				}
					case csFInfoFDType:
					{
						pbFind.ioSearchBits |= fsSBFlFndrInfo;
						
						pSpec1.hFileInfo.ioFlFndrInfo.fdType = va_arg(params, OSType);
						pSpec2.hFileInfo.ioFlFndrInfo.fdType = -1;
						
						break;
					}
					case csFInfoFDCreator:
					{
						pbFind.ioSearchBits |= fsSBFlFndrInfo;
						
						pSpec1.hFileInfo.ioFlFndrInfo.fdCreator = va_arg(params, OSType);
						pSpec2.hFileInfo.ioFlFndrInfo.fdCreator = -1;
						
						break;
					}
					case csFInfoFDFlags:
					{
						pbFind.ioSearchBits |= fsSBFlFndrInfo;
						
						pSpec1.hFileInfo.ioFlFndrInfo.fdFlags = va_arg(params, unsigned short);
						pSpec2.hFileInfo.ioFlFndrInfo.fdFlags = -1;
						
						break;
					}
					case csFInfoFDLocation:
					{
						pbFind.ioSearchBits |= fsSBFlFndrInfo;
						
						pSpec1.hFileInfo.ioFlFndrInfo.fdLocation = va_arg(params, Point);
						SetPt(&pSpec2.hFileInfo.ioFlFndrInfo.fdLocation, -1, -1);
						
						break;
					}
					case csFInfoFDFldr:
					{
						pbFind.ioSearchBits |= fsSBFlFndrInfo;
						
						pSpec1.hFileInfo.ioFlFndrInfo.fdFldr = va_arg(params, short);
						pSpec2.hFileInfo.ioFlFndrInfo.fdFldr = -1;
						
						break;
					}
	
				case csFlLgLen:
				{
					pbFind.ioSearchBits |= fsSBFlLgLen;
			
					pSpec1.hFileInfo.ioFlLgLen = va_arg(params, long);
					pSpec2.hFileInfo.ioFlLgLen = va_arg(params, long);
					
					break;
				}
				
				case csFlPyLen:
				{
					pbFind.ioSearchBits |= fsSBFlPyLen;
			
					pSpec1.hFileInfo.ioFlPyLen = va_arg(params, long);
					pSpec2.hFileInfo.ioFlPyLen = va_arg(params, long);
					
					break;
				}
				
				case csFlRLgLen:
				{
					pbFind.ioSearchBits |= fsSBFlRLgLen;
			
					pSpec1.hFileInfo.ioFlRLgLen = va_arg(params, long);
					pSpec2.hFileInfo.ioFlRLgLen = va_arg(params, long);
					
					break;
				}
				
				case csFlRPyLen:
				{
					pbFind.ioSearchBits |= fsSBFlRPyLen;
			
					pSpec1.hFileInfo.ioFlRPyLen = va_arg(params, long);
					pSpec2.hFileInfo.ioFlRPyLen = va_arg(params, long);
					
					break;
				}
				
				case csFlCrDat:
				{
					pbFind.ioSearchBits |= fsSBFlCrDat;
			
					pSpec1.hFileInfo.ioFlCrDat = va_arg(params, unsigned long);
					pSpec2.hFileInfo.ioFlCrDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csFlMdDat:
				{
					pbFind.ioSearchBits |= fsSBFlMdDat;
			
					pSpec1.hFileInfo.ioFlMdDat = va_arg(params, unsigned long);
					pSpec2.hFileInfo.ioFlMdDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csFlBkDat:
				{
					pbFind.ioSearchBits |= fsSBFlBkDat;
			
					pSpec1.hFileInfo.ioFlBkDat = va_arg(params, unsigned long);
					pSpec2.hFileInfo.ioFlBkDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csFlXFndrInfo:
				{
					pbFind.ioSearchBits |= fsSBFlXFndrInfo;
					
					pSpec1.hFileInfo.ioFlXFndrInfo.fdIconID = va_arg(params, short);
					pSpec1.hFileInfo.ioFlXFndrInfo.fdScript = va_arg(params, short);
					pSpec1.hFileInfo.ioFlXFndrInfo.fdXFlags = va_arg(params, short);
					pSpec1.hFileInfo.ioFlXFndrInfo.fdComment = va_arg(params, short);
					pSpec1.hFileInfo.ioFlXFndrInfo.fdPutAway = va_arg(params, long);
					
					pSpec2.hFileInfo.ioFlXFndrInfo.fdIconID = -1;
					pSpec2.hFileInfo.ioFlXFndrInfo.fdScript = -1;
					pSpec2.hFileInfo.ioFlXFndrInfo.fdXFlags = -1;
					pSpec2.hFileInfo.ioFlXFndrInfo.fdComment = -1;
					pSpec2.hFileInfo.ioFlXFndrInfo.fdPutAway = -1;
				
					break;
				}
					case csFXInfoFDIconID:
					{
						pbFind.ioSearchBits |= fsSBFlXFndrInfo;
						
						pSpec1.hFileInfo.ioFlXFndrInfo.fdIconID = va_arg(params, short);
						pSpec2.hFileInfo.ioFlXFndrInfo.fdIconID = -1;
						
						break;
					}
					case csFXInfoFDScript:
					{
						pbFind.ioSearchBits |= fsSBFlXFndrInfo;
						
						pSpec1.hFileInfo.ioFlXFndrInfo.fdScript = va_arg(params, short);
						pSpec2.hFileInfo.ioFlXFndrInfo.fdScript = -1;
						
						break;
					}
					case csFXInfoFDXFlags:
					{
						pbFind.ioSearchBits |= fsSBFlXFndrInfo;
						
						pSpec1.hFileInfo.ioFlXFndrInfo.fdXFlags = va_arg(params, short);
						pSpec2.hFileInfo.ioFlXFndrInfo.fdXFlags = -1;
						
						break;
					}
					case csFXInfoFDComment:
					{
						pbFind.ioSearchBits |= fsSBFlXFndrInfo;
						
						pSpec1.hFileInfo.ioFlXFndrInfo.fdComment = va_arg(params, short);
						pSpec2.hFileInfo.ioFlXFndrInfo.fdComment = -1;
						
						break;
					}
					case csFXInfoFDPutAway:
					{
						pbFind.ioSearchBits |= fsSBFlXFndrInfo;
						
						pSpec1.hFileInfo.ioFlXFndrInfo.fdPutAway = va_arg(params, long);
						pSpec2.hFileInfo.ioFlXFndrInfo.fdPutAway = -1;
						
						break;
					}
	
				case csFlParID:
				{
					pbFind.ioSearchBits |= fsSBFlParID;

					pSpec1.hFileInfo.ioFlParID = va_arg(params, long);
					// what does spec2 get set to?

					break;
				}
				
				case csNegate:
				{
					pbFind.ioSearchBits |= fsSBNegate;
					break;
				}
	
				case csDrUsrWds:
				{
					pbFind.ioSearchBits |= fsSBDrUsrWds;

					pSpec1.dirInfo.ioDrUsrWds.frRect = va_arg(params, Rect);
					pSpec1.dirInfo.ioDrUsrWds.frFlags = va_arg(params, short);
					pSpec1.dirInfo.ioDrUsrWds.frLocation = va_arg(params, Point);
					pSpec1.dirInfo.ioDrUsrWds.frView = va_arg(params, short);

					SetRect(&pSpec2.dirInfo.ioDrUsrWds.frRect, -1, -1, -1, -1);
					pSpec2.dirInfo.ioDrUsrWds.frFlags = -1;
					SetPt(&pSpec2.dirInfo.ioDrUsrWds.frLocation, -1, -1);
					pSpec2.dirInfo.ioDrUsrWds.frView = -1;

					break;
				}
					case csDInfoFRRect:
					{
						pbFind.ioSearchBits |= fsSBDrUsrWds;
						
						pSpec1.dirInfo.ioDrUsrWds.frRect = va_arg(params, Rect);
						SetRect(&pSpec2.dirInfo.ioDrUsrWds.frRect, -1, -1, -1, -1);
						
						break;
					}
					case csDInfoFRFlags:
					{
						pbFind.ioSearchBits |= fsSBDrUsrWds;
						
						pSpec1.dirInfo.ioDrUsrWds.frFlags = va_arg(params, short);
						pSpec2.dirInfo.ioDrUsrWds.frFlags = -1;
						
						break;
					}
					case csDInfoFRLocation:
					{
						pbFind.ioSearchBits |= fsSBDrUsrWds;
						
						pSpec1.dirInfo.ioDrUsrWds.frLocation = va_arg(params, Point);
						SetPt(&pSpec2.dirInfo.ioDrUsrWds.frLocation, -1, -1);
						
						break;
					}
					case csDInfoFRView:
					{
						pbFind.ioSearchBits |= fsSBDrUsrWds;
						
						pSpec1.dirInfo.ioDrUsrWds.frView = va_arg(params, short);
						pSpec2.dirInfo.ioDrUsrWds.frView = -1;
						
						break;
					}
	
	
				case csDrNmFls:
				{
					pbFind.ioSearchBits |= fsSBDrNmFls;

					pSpec1.dirInfo.ioDrNmFls = va_arg(params, short);
					pSpec2.dirInfo.ioDrNmFls = va_arg(params, short);

					break;
				}
	
				case csDrCrDat:
				{
					pbFind.ioSearchBits |= fsSBDrCrDat;
			
					pSpec1.dirInfo.ioDrCrDat = va_arg(params, unsigned long);
					pSpec2.dirInfo.ioDrCrDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csDrMdDat:
				{
					pbFind.ioSearchBits |= fsSBDrMdDat;
			
					pSpec1.dirInfo.ioDrMdDat = va_arg(params, unsigned long);
					pSpec2.dirInfo.ioDrMdDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csDrBkDat:
				{
					pbFind.ioSearchBits |= fsSBDrBkDat;
			
					pSpec1.dirInfo.ioDrBkDat = va_arg(params, unsigned long);
					pSpec2.dirInfo.ioDrBkDat = va_arg(params, unsigned long);
					
					break;
				}
				
				case csDrFndrInfo:
				{
					pbFind.ioSearchBits |= fsSBDrFndrInfo;

					pSpec1.dirInfo.ioDrFndrInfo.frScroll = va_arg(params, Point);
					pSpec1.dirInfo.ioDrFndrInfo.frOpenChain = va_arg(params, long);
					pSpec1.dirInfo.ioDrFndrInfo.frComment = va_arg(params, short);
					pSpec1.dirInfo.ioDrFndrInfo.frPutAway = va_arg(params, long);

					SetPt(&pSpec2.dirInfo.ioDrFndrInfo.frScroll, -1, -1);
					pSpec2.dirInfo.ioDrFndrInfo.frOpenChain = -1;
					pSpec2.dirInfo.ioDrFndrInfo.frComment = -1;
					pSpec2.dirInfo.ioDrFndrInfo.frPutAway = -1;

					break;
				}
					case csDXInfoFRScroll:
					{
						pbFind.ioSearchBits |= fsSBDrFndrInfo;
						
						pSpec1.dirInfo.ioDrFndrInfo.frScroll = va_arg(params, Point);
						SetPt(&pSpec2.dirInfo.ioDrFndrInfo.frScroll, -1, -1);
						
						break;
					}
					case csDXInfoFROpenChain:
					{
						pbFind.ioSearchBits |= fsSBDrFndrInfo;
						
						pSpec1.dirInfo.ioDrFndrInfo.frOpenChain = va_arg(params, long);
						pSpec2.dirInfo.ioDrFndrInfo.frOpenChain = -1;
						
						break;
					}
					case csDXInfoFRComment:
					{
						pbFind.ioSearchBits |= fsSBDrFndrInfo;
						
						pSpec1.dirInfo.ioDrFndrInfo.frComment = va_arg(params, short);
						pSpec2.dirInfo.ioDrFndrInfo.frComment = -1;
						
						break;
					}
					case csDXInfoFRPutAway:
					{
						pbFind.ioSearchBits |= fsSBDrFndrInfo;
						
						pSpec1.dirInfo.ioDrFndrInfo.frPutAway = va_arg(params, long);
						pSpec2.dirInfo.ioDrFndrInfo.frPutAway = -1;
						
						break;
					}
				
				case csDrParID:
				{
					pbFind.ioSearchBits |= fsSBDrParID;

					pSpec1.dirInfo.ioDrParID = va_arg(params, long);
					// what does spec2 get set to?

					break;
				}
				
				case csSearchInDirectory:
				{
					subdirectorySearch = TRUE;
					
					searchDirectory = va_arg(params, long);
					
					break;
				}
				
				case csContinue:
				{
					// csContinue found in the middle of a list of parameters.
					// It needs to be at the beginning.
					result = paramErr;
					break;
				}
				
				case csInitOnly:
				{
					initOnly = TRUE;
					break;
				}
				
				default:
				{
					DebugStr("\pCatSearch: received an unknown selector!");
					result = paramErr;
					break;
				}
			}
		}
		
		va_end(params);
	}
	
	if (result == noErr)
	{
		if (pbFind.ioNamePtr == NIL && pbFind.ioVRefNum >= 0)
			result = paramErr;
		
		if (pbFind.ioMatchPtr == NIL || pbFind.ioReqMatchCount <= 0)
			result = paramErr;
		
		if (pbFind.ioSearchBits == 0)
			result = paramErr;
	}

	if (result == noErr && !initOnly)
	{
		if (subdirectorySearch)
			result = PBCatSearchIn(&pbFind, searchDirectory);
		else
			result = PBCatSearchSync(&pbFind);

		if (result == noErr || result == eofErr)
			*found = pbFind.ioActMatchCount;
	}
	
	return result;
}

