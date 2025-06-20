/*/
     Project Arashi/STORM: RightWrite.c
     Major release: 9/9/92

     Last modification: Tuesday, August 25, 1992, 22:38
     Created: Tuesday, August 25, 1992, 14:58

     Copyright � 1992, Juri Munkki
/*/

#include <Packages.h>

Handle	StartText,EndText,ContentText;
int		StartLen,EndLen;

void	DoMungersToComment(FilePB,Content)
FileParam	*FilePB;
Handle		Content;
{
	char			Keyword[256];
	char			TimeString[256];
	DateTimeRec		CreateDate,ModifyDate;
	
	GetIndString(Keyword,128,1);
	Munger(Content,0,Keyword+1,*Keyword,FilePB->ioNamePtr+1,*(FilePB->ioNamePtr));
	Secs2Date(FilePB->ioFlCrDat,&CreateDate);
	Secs2Date(FilePB->ioFlMdDat,&ModifyDate);
	
	NumToString(CreateDate.year,TimeString);
	GetIndString(Keyword,128,3);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	if(CreateDate.year != ModifyDate.year)
	{	int	firstlen;
	
		firstlen = TimeString[0];
		NumToString(ModifyDate.year,firstlen+1+TimeString);
		TimeString[0]+= 1+TimeString[firstlen+1];
		TimeString[firstlen+1] = '-';
	}

	GetIndString(Keyword,128,2);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	NumToString(ModifyDate.year,TimeString);
	GetIndString(Keyword,128,4);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	IUDateString(FilePB->ioFlCrDat,longDate,(void *)TimeString);
	GetIndString(Keyword,128,5);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	IUDateString(FilePB->ioFlMdDat,longDate,(void *)TimeString);
	GetIndString(Keyword,128,6);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	IUTimeString(FilePB->ioFlCrDat,FALSE,(void *)TimeString);
	GetIndString(Keyword,128,7);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);

	IUTimeString(FilePB->ioFlMdDat,FALSE,(void *)TimeString);
	GetIndString(Keyword,128,8);
	Munger(Content,0,Keyword+1,*Keyword,TimeString+1,*TimeString);
}

void	DoFileOperations()
{
	int			message,count,i;
	AppFile		thefile;
	FileParam	block;
	
	CountAppFiles(&message,&count);
	if(message==1)
	{	return;
	}
	else
	{	for(i=1;i<=count;i++)
		{	int			ref;
		
			GetAppFiles(i,&thefile);

			block.ioCompletion=0;
			block.ioNamePtr=thefile.fName;
			block.ioVRefNum=thefile.vRefNum;
			block.ioFVersNum=thefile.versNum;
			block.ioFDirIndex=0;
			PBGetFInfo(&block,0);

			if(FSOpen(thefile.fName,thefile.vRefNum,&ref)==0)
			{	Handle		FileContent;
				long		FileSize;

				GetEOF(ref,&FileSize);
				FileContent = NewHandle(FileSize);
				if(FileContent)
				{	HLock(FileContent);
					if(FSRead(ref,&FileSize,*FileContent)==0)
					{	long	firstPlace,secondPlace;
						
						HUnlock(FileContent);
						firstPlace = Munger(FileContent,0,*StartText,StartLen,0,0);
						if(firstPlace>=0)
						{	secondPlace = Munger(FileContent,
													firstPlace+StartLen,
													*EndText,EndLen,0,0);
							if(secondPlace>=0)
							{	int		Failure = FALSE;
								Handle	NewContent;
								long	WriteLen;
							
								NewContent = ContentText;
								HandToHand(&NewContent);
								DoMungersToComment(&block,NewContent);
								HLock(NewContent);
								
								SetEOF(ref,0);
								
								WriteLen = firstPlace;
								Failure |= 0 != FSWrite(ref,&WriteLen,*FileContent);

								WriteLen = GetHandleSize(NewContent);
								Failure |= 0 != FSWrite(ref,&WriteLen,*NewContent);

								WriteLen = FileSize-secondPlace-EndLen;
								Failure |= 0 != FSWrite(ref,&WriteLen,
														*FileContent+secondPlace+EndLen);
								
								if(Failure)
								{	SysBeep(10);
									SetEOF(ref,0);
									FSWrite(ref,&FileSize,*FileContent);
								}
								
								HUnlock(NewContent);
								DisposHandle(NewContent);		
							}
						}
						
					}
					else
					{	HUnlock(FileContent);
					}
					DisposHandle(FileContent);
				}			
				FSClose(ref);

			}

			PBSetFInfo(&block,0);
			
		}
	}
}

pascal	int		keyfilter(theDialog,theEvent,itemHit)
DialogPtr	theDialog;
EventRecord	*theEvent;
int			*itemHit;
{
	if(theEvent->what==keyDown && (theEvent->modifiers & cmdKey))
	{	switch((char)theEvent->message)
		{	case 'Q':
			case 'q':
			case '.':
				*itemHit=2;
				break;
			case 'D':
			case 'd':
			case ' ':
			case 13:
			case 3:
				*itemHit=1;
				break;
			default:
				*itemHit=0;
		}
		return -1;
	}
	return 0;
}

void	main()
{
	DialogPtr	myDialog;
	short		item;
	short		itemType;
	Rect		itemBox;

	DoInits();
	myDialog = GetNewDialog(128,0,(WindowPtr)-1);
	
	do
	{	ModalDialog(keyfilter,&item);
	}	while(item != 1 && item != 2);
	
	if(item == 1)
	{	GetDItem(myDialog,6,&itemType,&StartText,&itemBox);
		HandToHand(&StartText);
		HLock(StartText);
		
		StartLen = GetHandleSize(StartText);
		GetDItem(myDialog,7,&itemType,&EndText,&itemBox);
		HandToHand(&EndText);
		HLock(EndText);
		
		EndLen = GetHandleSize(EndText);
		GetDItem(myDialog,8,&itemType,&ContentText,&itemBox);
		HandToHand(&ContentText);
		
		DoFileOperations();
	}
	DisposDialog(myDialog);
}