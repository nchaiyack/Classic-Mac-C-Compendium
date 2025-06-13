//FEHLER:
//- WENN DER FILENAME NICHT RICHTIG IST KOMMT ZWAR DIE MESSAGE, ABER ES WIRD TROTZDEM GEMACHT
//- EOF WIRD GESCHRIEBEN!
//- Beide behoben, NOCHMAL AN ALLEN FILES TESTEN!!!!!
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/** Modul-Name			: sorry, that«s impossible.C	   					 **/
/** Autor				: A. Amoroso					     				 **/
/** Funktion			: New 2 Old Style Comments							 **/
/** Datum				: 6.12.1993					    				 	 **/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

#include <ctype.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <size_t.h>
#include <stat.h>
#include <stdarg.h>
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GestaltEqu.h>
#include <Files.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <types.h>
#include <pascal.h>
#include <Events.h>
#include <EPPC.h>
#include <AppleEvents.h>
#include <OSEvents.h>


#include "Apple.h"
#include "Help.h"

#include "//<->/**/.h"

#ifdef testme
#include <console.h>
#endif

/******************************************************************************/
/* Hauptprogramm							      */
/******************************************************************************/

int main(int argc, char *argv[])

{
	char 	NName[MaxFullPathName+1],
			OName[MaxFullPathName+1],
			*occur = NULL,
			option = 0,
			stop = 0,			
			c1 = 0,
			c2 = 0,
			c = 0,
			cbuf = 0,
			q1 = 0,
			q2 = 0;
	int 	i = 0,
			len = 0;
	long int
			filepos = 0L;
	unsigned long
			typ = 0,
			cre = 0;
	FILE	*Optr = NULL,
			*Nptr = NULL;
	

#ifdef apple
#ifndef testme
	argv = GetProgParams(&argc);
	occur = strrchr(*argv, ':');
	option = (NULL != strstr((const char *)occur, "o"));
//	printf("Occur: %s\n", occur);
//	if (NULL != strstr((const char *)occur, "o"))
//		printf("Ergebnis Suche nach o: %s\n", strstr((const char *)occur, "o"));
//	printf ("Option: %i\n ARGV: %s", option, *argv);


//	Nptr = fopen("Modifiers.txt","w");
//	fputs(occur,Nptr);
//	fclose(Nptr);
#endif
#endif /* apple */
#ifdef testme
	argc = ccommand(&argv);
	option = 0;
#endif
	
	if (option){
		for(i=1; i < argc; i++){
			NName[0] = OName[0] = stop = c1 = c2 = c = q1 = q2 = 0;
			len = 0;
			Optr = NULL;
			Nptr = NULL;
			stop = 0;
			c = 0;
			cbuf = 0;
			q1 = q2 = 0;
			filepos = 0L;
			strcpy((char *)NName, (const char *)*(argv+i));
			strcpy((char *)OName, (const char *)*(argv+i));
			len = (int)strlen((const char *)NName);
			if ((c2 = *(OName + len - 2)) != '.')
				HandleError(NoCFile); 
			if ((c1 = *(OName + len - 1)) == 'c')
				*(NName + len - 1) = 'b';
				else if (c1 == 'C')
					*(NName + len - 1) = 'B';
					else if (c1 == 'h')
						*(NName + len - 1) = 'g';
						else if (c1 == 'H')
							*(NName + len - 1) = 'G';
			if ((c1 != 'c') && (c1 != 'C') && (c1 != 'h') && (c1 != 'H'))
				HandleError(NoCFile); 
			Optr = fopen(OName, "r");
			if (Optr == NULL)
				HandleError(FileIO);
			Nptr = fopen(NName, "w");
			if (Nptr == NULL)
				HandleError(FileIO);
	/* Folgende while Schleifen sind kommentar erkennender automat, 4 zustaende, 4 Eingaben */
			if (EOF != (c = fgetc(Optr)))
				do{
					switch(q1){
						case 0: switch (q2){
									case 0:	switch (c){
												case SL:	q1 = 1;
															q2 = 0;
															break;
												case EOF:	stop = 1;
												case CR:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 0;
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}
											break;
									case 1:	switch (c){
												case EOF:	stop = 1;
												case CR:	q1 = 0;
															q2 = 0;
															if (EOF == fputs("*/\n",Nptr))
																HandleError(FileIO);
															break;
												case SL:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 1;
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}
											break;
								}
								break;
						case 1: switch (q2){
									case 0:	switch (c){
												case SL:	q1 = 0;
															q2 = 1;
															if (EOF == fputs("/*",Nptr))
																HandleError(FileIO);
															break;
												case EOF:	stop = 1;
												case CR:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 0;
															if (EOF == fputs("/",Nptr))
																HandleError(FileIO);
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}	
											break;
									case 1:	switch (c){
												case EOF:	stop = 1;
												case SL:	/*break, the same...*/;
												case CR:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 0;
															HandleError(IllState);
															break;
											}
											break;
								}
								break;
					}
					if (!stop)
						c = fgetc(Optr);
				}while (!stop);
		if (EOF == fclose(Optr))
			HandleError(FileIO);
		if (EOF == fclose(Nptr))
			HandleError(FileIO);
			GetFileInfo(OName,&typ,&cre);
			SetFileInfo(NName,typ,cre);
			GetFileInfo(OName,&typ,&cre); // Soll den Finder veranlassen das neue Icon zu zeigen...
	//		SetFileInfo(NName,'TEXT','KAHL');
		}
	}
	
	
	else if (!option){
		for(i=1; i < argc; i++){
			NName[0] = OName[0] = stop = c1 = c2 = c = q1 = q2 = 0;
			len = 0;
			Optr = NULL;
			Nptr = NULL;
			stop = 0;
			c = 0;
			cbuf = 0;
			filepos = 0L;
			q1 = q2 = 0;
			strcpy((char *)NName, (const char *)*(argv+i));
			strcpy((char *)OName, (const char *)*(argv+i));
			len = (int)strlen((const char *)NName);
			if ((c2 = *(OName + len - 2)) != '.')
				HandleError(NoCFile); 
			if ((c1 = *(OName + len - 1)) == 'c')
				*(NName + len - 1) = 'd';
				else if (c1 == 'C')
					*(NName + len - 1) = 'D';
					else if (c1 == 'h')
						*(NName + len - 1) = 'i';
						else if (c1 == 'H')
							*(NName + len - 1) = 'I';
			if ((c1 != 'c') && (c1 != 'C') && (c1 != 'h') && (c1 != 'H'))
				HandleError(NoCFile); 
			Optr = fopen(OName, "r");
			if (Optr == NULL)
				HandleError(FileIO);
			Nptr = fopen(NName, "w");
			if (Nptr == NULL)
				HandleError(FileIO);
	/* Folgende while Schleifen sind kommentar erkennender automat, 4 zustaende, 4 Eingaben */
			if (EOF != (c = fgetc(Optr)))
				do{
					switch(q1){
						case 0: switch (q2){
									case 0:	switch (c){
												case SL:	q1 = 1;
															q2 = 0;
															break;
												case EOF:	stop = 1;
												case CR:	/*break, the same...*/;
												case ST:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 0;
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}
											break;
									case 1:	switch (c){
												case CR:	q1 = 0;
															q2 = 1;
															if (EOF == fputs("\n//",Nptr))
																HandleError(FileIO);
															break;
												case ST:	q1 = 1;
															q2 = 1;
															break;			
												case EOF:	stop = 1;
												case SL:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 1;
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}
											break;
								}
								break;
						case 1: switch (q2){
									case 0:	switch (c){
												case ST:	q1 = 0;
															q2 = 1;
															if (EOF == fputs("//",Nptr))
																HandleError(FileIO);
															break;
												case EOF:	stop = 1;
												case CR:	/*break, the same...*/;
												case SL:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 0;
															if (EOF == fputs("/",Nptr))
																HandleError(FileIO);
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}	
											break;
									case 1:	switch (c){
												case SL:	q1 = 0;
															q2 = 0;
															{
																filepos = ftell(Optr);
																while (CR != (c = fgetc(Optr))){
																	if (isalpha(c))
																		cbuf = 1; /* Jawoll, Return ausgeben! */
																}
																if(fseek(Optr, filepos, SEEK_SET))
																	HandleError(FileIO);
																if (cbuf){
																	if (EOF == fputs("\n",Nptr))
																		HandleError(FileIO);
																	cbuf = 0;
																}
															}
															break;
												case ST:	q1 = 1;
															q2 = 1;
															if (EOF == fputs("*",Nptr))
																HandleError(FileIO);
															break;
												case EOF:	stop = 1;
												case CR:	/*break, the same...*/;
												default:	q1 = 0;
															q2 = 1;
															if (EOF == fputs("*",Nptr))
																HandleError(FileIO);
															if (c != EOF)
																if (EOF == fputc(c,Nptr))
																	HandleError(FileIO);
															break;
											}
											break;
								}
								break;
					}
					if (!stop)
						c = fgetc(Optr);
				}while (!stop);
		if (EOF == fclose(Optr))
			HandleError(FileIO);
		if (EOF == fclose(Nptr))
			HandleError(FileIO);
			GetFileInfo(OName,&typ,&cre);
			SetFileInfo(NName,typ,cre);
			GetFileInfo(OName,&typ,&cre); // Soll den Finder veranlassen das neue Icon zu zeigen...
	//		SetFileInfo(NName,'TEXT','KAHL');
		}
	}
	GenExit(0);
}

void HandleError(int error)
{
	short itemHit = 0;
	
	switch(error){
		case	1:	do
					{
						itemHit = StopAlert(rNoCFile,nil);
					}while(itemHit != dOKButton);
	  				break;
	  	case	2:	do
					{
						itemHit = StopAlert(rFileIO,nil);
					}while(itemHit != dOKButton);
	  				break;
	  	case	3:	do
					{
						itemHit = StopAlert(rIllState,nil);
					}while(itemHit != dOKButton);
	  				break;
	  	default:	GenExit(0);
	}
	GenExit(0);
}