#ifndef bsioc
#define bsioc

//Alternativ: #pragma once

/***************************************************************************/
/*****   Datei, die alle innerhalb von SETI benštigten E/A-Routinen    *****/
/*****   bereitstellt.                                                 *****/
/*****   Autor:    Andreas Amoroso                                     *****/
/*****   Datum:    11.8.1993                                           *****/
/***************************************************************************/


//SCREENSIZE 80 * 24
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 
//80 Dots ................................................................................ 




//INCLUDES                      //  INCLUDE ALL OF THIS IN YOUR FILE!!!
                                //  ===================================
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#ifdef ibm
#include <stddef.h>
#endif // ifdef ibm
#ifdef apple
#include <size_t.h>
#include <stat.h>
#endif // ifdef apple
#include <stdarg.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include <stddef.h>
#include <math.h>

#ifdef modus_inter
#ifdef apple
#include <console.h>
#include <pascal.h>
#endif // apple
#endif // modus_inter

#include "Apple.h"
#include "Help.h"




//DEFINITIONS

void ConInit(char * modul)                              //  Sets 24 * 80 mode on mac, call before first console i/o
//      ConInit("RedEX\0");                             //  Example call
{
#ifdef modus_inter
#ifdef apple
    console_options.nrows=24;
    console_options.ncols=80;
    console_options.title = CtoPstr(modul);
#ifndef testmode
    console_options.pause_atexit = 0;
#endif //testmode
    cshow(stdin);
#endif // ifdef apple
#endif // ifdef modus_inter
}

     
int GenOutHeader(char *modul,char *version,char *funktion) 
// N.B.: Maximale LŠnge der Ÿbergebenen Zeichenketten :44 inkl. \0!
// modul = Modulname
// funktion = Funktionsbschreibung in wenigen Worten 
{
    char modmax[MaxHeadOutLen],vermax[MaxHeadOutLen],funmax[MaxHeadOutLen],timmax[MaxHeadOutLen];
    struct tm *time_now;
    time_t secs_now;
    int i=0,
        modlen=0,
        verlen=0,
        funlen=0,
        timlen=0,
        error=0;
    error = ((modul == (char *)NULL) || (version == (char *)NULL) || (funktion == (char *)NULL));
    error = (NULL == (time_now = (struct tm *) malloc(sizeof(struct tm))));
    if ((long) strlen(modul) >= (long) MaxModulNamLen) error = 1; 
    if ((long) strlen(version) >= (long) MaxHeadOutLen) error = 1;
    if ((long) strlen(funktion) >= (long) MaxHeadOutLen) error = 1;
    if (!error)
    {
        strncpy(modmax,modul,MaxModulNamLen);
        strncpy(vermax,version,MaxHeadOutLen);
        strncpy(funmax,funktion,MaxHeadOutLen);
#ifdef ibm
        tzset(); // DOS/UNIX-Kommando zum setzen der Zeit, auf Mac unnštig!!!
#endif // ifdef ibm
        time(&secs_now);
        time_now = localtime(&secs_now);
        strftime(timmax, MaxTimeDateLen,"%d.%b.%y, %H:%M:%S",time_now);
        *(modmax+MaxHeadOutLen-1) = '\0';
        *(vermax+MaxHeadOutLen-1) = '\0';
        *(funmax+MaxHeadOutLen-1) = '\0';
        *(timmax+MaxHeadOutLen-1) = '\0';
        modlen = (int) strlen(modmax);
        funlen = (int) strlen(funmax);
        verlen = (int) strlen(vermax);
        timlen = (int) strlen(timmax);
        for (i=(int)modlen; i< (MaxHeadOutLen-1); i++)
            *(modmax+i)=' ';
        for (i=(int)verlen; i< (MaxHeadOutLen-1); i++)
            *(vermax+i)=' ';
        for (i=(int)funlen; i< (MaxHeadOutLen-1); i++)
            *(funmax+i)=' ';
        for (i=(int)timlen; i< (MaxHeadOutLen-1); i++)
            *(timmax+i)=' ';
        if (EOF == printf("\n\n")) error =1;
        if (EOF == printf("\n           .........................................................")) error =1;
        if (EOF == printf("\n           .            __       ____    _______                   .")) error =1;
        if (EOF == printf("\n           .           /         |          |        |             .")) error =1;
        if (EOF == printf("\n           .           \\__       |__        |        |             .")) error =1;
        if (EOF == printf("\n           .              \\      |          |        |             .")) error =1;
        if (EOF == printf("\n           .            __/      |____      |        |             .")) error =1;
        if (EOF == printf("\n           .                                                       .")) error =1;
	if (EOF == printf("\n           . Development System for SElf TImed sequential Circuits .")) error =1;
        if (EOF == printf("\n           .                                                       .")) error =1;
        if (EOF == printf("\n           .        (c)1993  Universitaet Kaiserslautern           .")) error =1;
        if (EOF == printf("\n           .                     AG Beister                        .")) error =1;
        if (EOF == printf("\n           .........................................................")) error =1;
        if (EOF == printf("\n           . Module:   %s .",modmax)) error =1;
        if (EOF == printf("\n           . Version:  %s .",vermax)) error =1;
        if (EOF == printf("\n           . Function: %s .",funmax)) error =1;
        if (EOF == printf("\n           . Time:     %s .",timmax)) error =1;
        if (EOF == printf("\n           .........................................................")) error =1;
        if (EOF == printf("\n\n\n\a\a")) error =1;
    }
    if (EOF == GenOutAcknowledge(ModusReturn)) error =1;
    free(time_now);
	return(error ? EOF : 0);
}

#ifdef modus_inter
int GenOutTrailer(void) 
{
    int error = 0;
    
    GenOutClrScr(MaxLineCntS,(char *) NULL);
    if (EOF == printf("\n           .........................................................")) error =1;
    if (EOF == printf("\n           . Good bye...                                           .")) error =1;
    if (EOF == printf("\n           .........................................................")) error =1;
    if (EOF == printf("\n\n\n\n\a")) error =1;
    return(error ? EOF : 0);
}
#endif // ifdef modus_inter

#ifdef modus_inter
int GenOutMenu(char *menu[], char *menustring, char *modul, int PktZahl)
                                                                    // PktZahl = ArraylŠnge PLUS 1 (šberschrift!)
{
    int i = 0, space = 0, length = 0, error = 0;
    char title[MaxMenuTitleLen], modmax[MaxModulNamLen], helpstr[MaxLineS], formstr[MaxLineS],
//       *menumax[MaxMenuPointCnt];
         menumax[MaxMenuPointCnt][MaxMenuPointLen+1];
    div_t res;
    
    error = ((menustring == (char *)NULL) || (modul == (char *)NULL));
    for (i=0; i<= PktZahl; i++)
        error = ((char *)NULL == menu[i]);
    if ( (long) strlen(modul) >= (long) MaxModulNamLen) error = 1;
    if (PktZahl >= MaxMenuPointCnt) error = 1;                      // MenŸpunkte PLUS šberschrift!
    if ((long) strlen(menu[0]) >= (long) MaxMenuTitleLen) error = 1;
    for (i=1; i<= PktZahl; i++)
        if ((long) strlen(menu[i]) >= (long) MaxMenuPointLen) error = 1;
    strncpy(modmax,modul,MaxModulNamLen);
//    if (NULL != (menumax[0] = (char *) calloc(sizeof(char),MaxMenuTitleLen)))
//            strncpy(menumax[0],menu[0],MaxMenuTitleLen);
//    for (i=1; i<= PktZahl; i++)
//    {
//        menumax[i] = (char *) calloc(sizeof(char),MaxMenuPointLen);
//        if (NULL != menumax[i])
//            strncpy(menumax[i],menu[i],MaxMenuPointLen);
//        else
//            error = 1;
//    }
    strncpy(menumax[0],menu[0],MaxMenuTitleLen);
    for (i=1; i<= PktZahl; i++)
        strncpy(menumax[i],menu[i],MaxMenuPointLen);
    
    length = (int) (long) strlen(modmax);
    length += (int) (long) strlen(MenuTitle);
    length += (int) (long) strlen(menu[0]);
    strcpy(title,strcat(modmax, MenuTitle));
    strcpy(title,strcat(title, menu[0]));
    if(!error)
    {   
        space = MaxLineS - (int) strlen(title);
        res = div(space,2);
        space = res.quot;
        GenOutClrScr(MaxLineCntS,menustring);
        for (i=1; i<=space; i++)
        {
            if (!error)
                strcat(menustring," \0");
        }
        if (!error)
            strcat(menustring,title);
        if (!error)
            strcat(menustring,"\n\0");
        for (i=1; i<=space; i++)
        {
            if (!error)
                strcat(menustring," \0");
        }
        for (i=1; i<=(int)((long) strlen(title)); i++)
        {
            if (!error)
                strcat(menustring,"=\0"); 
        }
        if (!error)
            strcat(menustring,"\n\0");
        for(i=1; i<=PktZahl; i++)
        {   
            if (EOF == sprintf(helpstr,"\n  %2i.   %s", i, menumax[i])) 
                error =1;
            if (!error)
                strcat(menustring,helpstr); 
        }
        if (!error)
        {   
            strcat(menustring,"\n\n   H.   Help...\0");
            strcat(menustring,"\n   X.   eXit\0");
            for (i=1; i<(MaxLineCntS - (PktZahl +8)); i++)
                strcat(menustring,"\n\0"); 
        }
        return(error ? EOF : 0);
    }
}
#endif // ifdef modus_inter  

int GenOutLine(int outform,int outtype,int inlen,char outstr1[],char outstr2[],char *modul,char *mess,char *auxmess,char *defstr)
{
    int error = 0;
    
    error = ((outstr1 == (char *)NULL) || (outstr2 == (char *)NULL) || (modul == (char *)NULL) || (mess == (char *)NULL)
             || (auxmess == (char *)NULL) || (defstr == (char *)NULL));
    if (!error)
    {
        if (outform == 0) 
            error = GenOutLine0(outtype,inlen,outstr1,mess,defstr);
            else if ((outform == 1) || (outform == 3))
                error = GenOutLine2(outform,outtype,inlen,outstr1,outstr2,modul,mess,auxmess,defstr);
                else 
                    error=1; 
    }
    return(error ? EOF : 0);       
}


int GenOutLine0(int outtype,int inlen,char outstr1[],char *mess, char *defstr)
{
    int error = 0,i = 0,defstrlen = 0;
    
    error = ((outstr1 == (char *)NULL) || (mess == (char *)NULL) || (defstr == (char *)NULL));
    error = !(MaxMessLen >= (int) (long) strlen(mess));
    error = !(MaxDefStrLen >= (defstrlen = (int) (long) strlen(defstr)));
    if (!error)
    {   
        switch (outtype)
        {
            case 100:   
            case 101:
            case 102:
            case 103:
            case 104:
                    {                       
                        error = (EOF == sprintf(outstr1,"%s%s%s",ReqStr,mess,ReqTStr2));
                        if (!error)
                           {
                            for (i=1;i<=(inlen - defstrlen);i++)
                                strcat(outstr1," \0");                          
                            strcat(outstr1,defstr);
                            strcat(outstr1,ReqTStr3);
                            for (i=1;i<=(inlen+1);i++)
                	    	strcat(outstr1,"\b");
                            }                           
                        break;
                    }
            case 105:
                    {
                        if (defstrlen == 0)
                            error = (EOF == sprintf(outstr1,"%s%s%s",ReqStr,mess,ReqTStr1));
                        else
                            error = (EOF == sprintf(outstr1,"%s%s%s%s",ReqStr,mess,ReqTStr1,defstr));                       
                        break;
                    }

            case 200:
            case 201:
            case 202:
                    {
                        error = (EOF == sprintf(outstr1,"%s%s\n",InfStr,mess));
                        break;
                    }
            case 300:
            case 301:
            case 302:
            case 303:
            case 304:
            case 305:
                    {
                        error = (EOF == sprintf(outstr1,"%s%s\n",SnfStr,mess));
                        break;
                    }
            case 400:
            case 401:
            case 402:
            case 403:
            case 404:
            case 405:
            case 406:
                    {
                        error = (EOF == sprintf(outstr1,"%s%s\n",WrnStr,mess));
                        break;
                    }
            default:    
                    {
                        printf("\nUse Format LongScr/LongFile for Errors and Severe Errors!");
                        printf("\nIf this doesn't apply to this call of GenOutLine0");
                        printf("\nthen please check your arguments!");
                        error= GenOutAcknowledge(ModusReturn);
                        printf("\nProgramm exiting...");
                        exit(1);                     
                        break;
                    }
            }       
    }
    return(error);
}           


int GenOutLine2(int outform,int outtype,int inlen,char outstr1[],char outstr2[],char *modul,char *mess,char *auxmess,char *defstr)
{
    int error = 0,i = 0,defstrlen = 0;
//    char *helpstr,*helpstr2;
    char helpstr[MaxTimeDateLen],helpstr2[MaxErrMessLen];
    struct tm *time_now;
    time_t secs_now;
    
    error = ((outstr1 == (char *)NULL) || (outstr2 == (char *)NULL) || (modul == (char *)NULL) || (mess == (char *)NULL)
             || (auxmess == (char *)NULL) || (defstr == (char *)NULL));
//    error = (NULL == (helpstr = (char *) calloc(MaxTimeDateLen,sizeof(char))));
//    error = (NULL == (helpstr2 = (char *) calloc(MaxTimeDateLen,sizeof(char))));
    error = (NULL == (time_now = (struct tm *) malloc(sizeof(struct tm))));
    error = !(MaxModulNamLen >= (int) (long) strlen(modul));
    error = !(MaxMessLen >= (int) (long) strlen(mess));
    error = !(MaxAuxMessLen >= (int) (long) strlen(auxmess));
    error = !(MaxDefStrLen >= (defstrlen = (int) (long) strlen(defstr)));
#ifdef ibm
    tzset(); // DOS/UNIX-Kommando zum setzen der Zeit, auf Mac unnštig!!!
#endif // ifdef ibm
    time(&secs_now);
    time_now = localtime(&secs_now);
    error = (!strftime(helpstr, MaxTimeDateLen-1,"%d.%b.%y, %H:%M:%S",time_now));
    if (!error)
    {   
        switch (outtype)
        {
            case 100:   
            case 101:
            case 102:
            case 103:
            case 104:
                    {   
                        error = GenCorStr(outtype,helpstr2); 
                        if (!error)             
                            error = (EOF == sprintf(outstr1,"%s<<%s / %s / %i>> %s",LotStr,modul,helpstr,outtype,helpstr2));
                        if (!error && (outform == 1))
                            strcat(outstr1,"\n\0"); 
                        if (!error) 
                            error = (EOF == sprintf(outstr2,"%s%s%s",ReqStr,mess,ReqTStr1));
                        if (!error)
                           {
                            strcat(outstr2,ReqTStr2);
                            for (i=1;i<=(inlen - defstrlen);i++)
                                strcat(outstr2," \0");                          
                            strcat(outstr1,defstr);
                            strcat(outstr2,ReqTStr3);
                            for (i=1;i<=(inlen+1);i++)
                                strcat(outstr2,"\b");
                            }                           
                        break;
                    }
            case 105:
                    {
                        error = GenCorStr(outtype,helpstr2); 
                        if (!error)             
                            error = (EOF == sprintf(outstr1,"%s<<%s / %s / %i>> %s",LotStr,modul,helpstr,outtype,helpstr2));
                        if (!error && (outform == 1))
                            strcat(outstr1,"\n\0"); 
                        if ((!error) && (defstrlen == 0))
                            error = (EOF == sprintf(outstr2,"%s%s%s",ReqStr,mess,ReqTStr1));
                        else if(!error)
                            error = (EOF == sprintf(outstr2,"%s%s%s%s",ReqStr,mess,ReqTStr1,defstr));                       
                        break;
                    }

            case 200:
            case 201:
            case 202:
            case 300:
            case 301:
            case 302:
            case 303:
            case 304:
            case 305:
            case 400:
            case 401:
            case 402:
            case 403:
            case 404:
            case 405:
            case 406:
            case 500:
            case 501:
            case 502:
            case 503:
            case 504:
            case 505:
            case 506:
            case 507:
            case 508:
            case 509:
            case 510:
            case 600:
            case 601:
            case 602:
            case 603:
            case 604:
            case 605:
            case 606:
            case 607:
            case 608:
            case 609:
            case 610:
                    {
                        error = GenCorStr(outtype,helpstr2); 
                        if (!error)             
                            error = (EOF == sprintf(outstr1,"%s<<%s / %s / %i>> %s",LotStr,modul,helpstr,outtype,helpstr2));
                        if (!error && (outform == 1))
                            strcat(outstr1,"\n\0"); 
                        if (!error) 
                            error = (EOF == sprintf(outstr2,"%s\n",mess));
                        break;
                    }
            default:    
                    {
                        printf("\nUse Format LongScr/LongFile for Errors and Severe Errors!");
                        printf("\nIf this doesn't apply to this call of GenOutLine0");
                        printf("\nthen please check your arguments!");
                        printf("\nProgramm exiting...");
                        error= GenOutAcknowledge(ModusReturn);
                        exit(1);                     
                        break;
                    }
            }       
    }
    free(time_now);
	return(error);
}           

#ifdef modus_inter
int GenInAnswer(int intype,int inlen, bsBoolean *inbool,char *inchar, char incharmax, int *inint, int inintmin, int inintmax, 
                char *instring, char *inhelpstr, char *inerrstring,char *headstring, char *promptstring)
                                // ATTENTION: INPUT IS RETURNED IN CASE UESR JUST HITS RETURN WHENEVER
                                // PRECEEDING REQUEST STRING SHOWS A DIFFERENT FROM NONE!!!
                                // THEREFORE: BEFORE CALLING THIS ROUTINE SET VALUES APPROPRIATELY!
{
    int helpint = 0,
        error = 0,
        i = 0;
    char helpchar=' ',
         helplong[MaxFullPathName+2],
         *helpstring,
         *occur;
    bsBoolean answerok=bsFalse,
              answerquit=bsFalse,
              answerhelp=bsFalse,
              answervoid=bsFalse; 
    
    error = ((inbool == (bsBoolean *)NULL) || (inchar == (char *)NULL) || (inint == (int *)NULL) || (instring == (char *)NULL) || 
            (inhelpstr == (char *)NULL) || (inerrstring == (char *)NULL) || (headstring == (char *)NULL) || (promptstring == (char *)NULL));
    error = (NULL == (helpstring = calloc(sizeof(char),inlen+1)));
    error = (EOF == printf("%s", headstring));
    error = (EOF == printf("%s", promptstring));
    if (!error)
    {
        fflush(stdin);
        switch(intype)
        {
            case AReq:  error = GenOutAcknowledge(ModusReturn);
                        break;
            case BReq:  helpchar = getchar();
                        helpchar = (char) tolower((int)helpchar);
                        if (helpchar == (NEWLINE))   
                        {
                              answerok = bsTrue;          // DEFAULT!!!
                              answervoid = bsTrue;            
                        }
                        else if ((helpchar == KEYN) || (helpchar == KEYn) || (helpchar == KEYS) || (helpchar == KEYs))
                        {
                              *inbool = bsFalse;
                              answerok = bsTrue;
                        }
                        else if ((helpchar == KEYY) || (helpchar == KEYy) || (helpchar == KEYJ) || (helpchar == KEYj) || (helpchar == KEYL) || (helpchar == KEYl))
                        {
                              *inbool = bsTrue;
                              answerok = bsTrue;
                        }
                        else if ((helpchar == KEYH) || (helpchar == KEYh) || (helpchar == HELP))
                        {
                              printf("%s ",inhelpstr);
                              GenOutAcknowledge(ModusNoRet);
                              answerhelp = bsTrue;
                              answerok = bsTrue;
                        }
                        else if ((helpchar == KEYq) || (helpchar == KEYQ) || (helpchar == HELP))
                        {
                              answerquit = bsTrue;
                              answerok = bsTrue;
                        }
                        else
                        {
                              printf("%s ",inerrstring);
                              GenOutAcknowledge(ModusNoRet);
                        }
                        break;
            case MReq:  fgets(helplong,(inlen + 1),stdin);
                        if (helplong[0] == NEWLINE)   
                        {
                            answerok = bsTrue;          // DEFAULT!!!
                            answervoid = bsTrue;
                        }
                        else if ((char *)NULL != (occur = strpbrk((const char *)helplong,(const char *)"\n\0")))
                            *occur = '\0';                      
                        if(!answerok)
                        {
                            if ((helplong[0] == KEYH) || (helplong[0] == KEYh)) 
                            {
                                *inchar = incharmax + 1;
                                answerhelp = bsTrue;
                                answerok = bsTrue;          
                            }
                            else if ((helplong[0] == KEYX) || (helplong[0] == KEYx)) 
                                {
                                    *inchar = incharmax + 2;
                                    answerok = bsTrue;          
                                }
	                            else if ((helplong[0] == KEYQ) || (helplong[0] == KEYq)) 
	                                {
	                                    answerquit = bsTrue;
	                                    answerok = bsTrue;          
	                                }
	                                else 
	                                {
	                                    for (i=0;i<inlen;i++)
	                                        if (!isdigit(helplong[i]))
	                                            error = 1;
	                                    if (!error)
	                                    {
	                                        sscanf(helplong,"%i",&helpint);
	                                        helpchar = (char) helpint;
	                                        if ((helpchar >= 1) && (helpchar <= incharmax))
	                                        {
	                                            *inchar = helpchar;
	                                            answerok = bsTrue;
	                                        }
	                                    }
	                                    else
	                                    {
	                                        printf("%s ",inerrstring);
	                                        GenOutAcknowledge(ModusNoRet);
	                                        
	                                    }   
	                                }
	                        }
	                        helplong[0] = ' ';
                        GenOutClrScr(MaxLineS,helplong);
                        if  (EOF == printf("%s",helplong))
                        	GenExit(100);                 
                        break;
            case IReq:  fgets(helplong,(inlen + 1),stdin);
                        if (helplong[0] == NEWLINE)   
                        {
                            answerok = bsTrue;          // DEFAULT!!!
                            answervoid = bsTrue;
                        }
                        else if ((char *)NULL != (occur = strpbrk((const char *)helplong,(const char *)"\n\0")))
                            *occur = '\0';                      
                        if (!answerok)
                        {
                            if ((helplong[0] == KEYH) || (helplong[0] == KEYh)) 
                            {
                                printf("%s ",inhelpstr);
                                GenOutAcknowledge(ModusNoRet);
                                answerhelp = bsTrue;
                                answerok = bsTrue;          
                            }
                            else if ((helplong[0] == KEYQ) || (helplong[0] == KEYq)) 
                                {
                                    answerquit = bsTrue;
                                    answerok = bsTrue;          
                                }
                                else 
                                {
                                    for (i=0;i<inlen;i++)
                                        if (!isdigit(helplong[i]))
                                            error = 1;
                                    if (!error)
                                    {
                                        sscanf(helplong,"%i",&helpint);
                                        if ((helpint >= inintmin) && (helpint <= inintmax))
					{
                                            helpchar = (char)helpint;
                                            *inchar = helpchar;
                                            answerok = bsTrue;
                                        }
                                    }
                                    else
                                    {
                                        printf("%s ",inerrstring);
                                        GenOutAcknowledge(ModusNoRet);  
                                    }
                                }
                        }                   
                        break;
            case RReq:  printf("NOT IMPLEMENTED, Program exiting...");
                        error = GenOutAcknowledge(ModusNoRet);
                        exit(0);
                        break;
            case SReq:  fgets(helplong,(inlen + 1),stdin);
                        if (helplong[0] == NEWLINE)   
                        {
                            answerok = bsTrue;          // DEFAULT!!!
                            answervoid = bsTrue;
                        }
                        else if ((char *)NULL != (occur = strpbrk((const char *)helplong,(const char *)"\n\0")))
                            *occur = '\0';                      
                        if (!answerok)
                        {
                            if (((helplong[0] == KEYH) || (helplong[0] == KEYh)) && (1 == (int) strlen(helplong)))
                            {
                                printf("%s ",inhelpstr);
                                GenOutAcknowledge(ModusNoRet);
                                answerhelp = bsTrue;
                                answerok = bsTrue;          
                            }
                            else if ((helplong[0] == KEYQ) || (helplong[0] == KEYq)) 
                                {
                                    answerquit = bsTrue;
                                    answerok = bsTrue;          
                                }
                                else 
                                {
                                    for (i=0;i<(int)strlen(helplong);i++)
                                        if ((!isgraph(helplong[i]) && (!isspace(helplong[i]))))
                                            error = 1;
                                    if (!error)
                                    {
                                        helplong[inlen] = '\0';
                                        // sscanf(helplong,"%s",helpstring);
                                        // Dann kšnnen keine Blanks verwendet werden!!!
                                        for (helpint = 0; *(helplong+helpint) !=0; helpint++)
                                    		*(helpstring+helpint) = *(helplong + helpint);
                                    	*(helpstring+helpint) = 0;
                                        if (inlen >= (int) strlen(helpstring))  
                                        {
                                            for(i=0;(*(helpstring+i-1) != '\0');i++)
                                                *(instring+i) = *(helpstring+i);
                                            answerok = bsTrue;
                                        }
                                    }
                                    else
                                    {
                                        printf("%s ",inerrstring);
                                        GenOutAcknowledge(ModusNoRet);
                                        
                                    }   
                                }
                        }                   
                        break;
            default:    error=1;
        
        }   
    }
    free (helpstring);
    if (error)
        return INPUTER;
    if (!answerok)
        return INPUTIR;
    if (answerquit)
        return INPUTQI;
    if (answervoid)
        return INPUTVO;
    if (answerhelp)
        return INPUTHE;
    else 
        return INPUTOK;
    
}
#endif // modus_inter

int GenCorStr(int type,char *str)
{
    int error = 0;
    
    error = ((char *)NULL == str);
    if (!error)
    {
        switch (type)
        {
            case 100:   
                        strncpy(str,BReqStr,((size_t) MaxErrMessLen));
                        break;
            case 101:   strncpy(str,MReqStr,((size_t) MaxErrMessLen));
                        break;
            case 102:   strncpy(str,IReqStr,((size_t) MaxErrMessLen));
                        break;
            case 103:   strncpy(str,RReqStr,((size_t) MaxErrMessLen));
                        break;
            case 104:   strncpy(str,SReqStr,((size_t) MaxErrMessLen));
                        break;
            
            case 200:   strncpy(str,IInfStr,((size_t) MaxErrMessLen));
                        break;
            case 201:   strncpy(str,AInfStr,((size_t) MaxErrMessLen));
                        break;
            case 202:   strncpy(str,EInfStr,((size_t) MaxErrMessLen));
                        break;
            
            case 300:   strncpy(str,CSnfStr,((size_t) MaxErrMessLen));
                        break;
            case 301:   strncpy(str,WSnfStr,((size_t) MaxErrMessLen));
                        break;
            case 302:   strncpy(str,ASnfStr,((size_t) MaxErrMessLen));
                        break;
            case 303:   strncpy(str,ESnfStr,((size_t) MaxErrMessLen));
                        break;
            case 304:   strncpy(str,SSnfStr,((size_t) MaxErrMessLen));
                        break;
            case 305:   strncpy(str,PSnfStr,((size_t) MaxErrMessLen));
                        break;
    
            case 400:   strncpy(str,IWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 401:   strncpy(str,DWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 402:   strncpy(str,SWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 403:   strncpy(str,AWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 404:   strncpy(str,JWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 405:   strncpy(str,BWrnStr,((size_t) MaxErrMessLen));
                        break;
            case 406:   strncpy(str,KWrnStr,((size_t) MaxErrMessLen));
                        break;
            
            case 500:   strncpy(str,IErrStr,((size_t) MaxErrMessLen));
                        break;
            case 501:   strncpy(str,DErrStr,((size_t) MaxErrMessLen));
                        break;
            case 502:   strncpy(str,SErrStr,((size_t) MaxErrMessLen));
                        break;
            case 503:   strncpy(str,AErrStr,((size_t) MaxErrMessLen));
                        break;
            case 504:   strncpy(str,JErrStr,((size_t) MaxErrMessLen));
                        break;
            case 505:   strncpy(str,BErrStr,((size_t) MaxErrMessLen));
                        break;
            case 506:   strncpy(str,KErrStr,((size_t) MaxErrMessLen));
                        break;
            case 507:   strncpy(str,WErrStr,((size_t) MaxErrMessLen));
                        break;
            case 508:   strncpy(str,RErrStr,((size_t) MaxErrMessLen));
                        break;
            case 509:   strncpy(str,UErrStr,((size_t) MaxErrMessLen));
                        break;
            case 510:   strncpy(str,EErrStr,((size_t) MaxErrMessLen));
                        break;
            
            case 600:   strncpy(str,ASrrStr,((size_t) MaxErrMessLen));
                        break;
            case 601:   strncpy(str,FSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 602:   strncpy(str,NSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 603:   strncpy(str,OSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 604:   strncpy(str,QSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 605:   strncpy(str,RSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 606:   strncpy(str,WSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 607:   strncpy(str,CSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 608:   strncpy(str,DSrrStr,((size_t) MaxErrMessLen));
                        break;
            case 609:   strncpy(str,USrrStr,((size_t) MaxErrMessLen));
                        break;
            case 610:   strncpy(str,MSrrStr,((size_t) MaxErrMessLen));
                        break;
    
            default:    error =1;
         }
    }
    return(error);
}


int GenOutCentered(char *modul)     
{   
    int error = 0, i = 0, space = 0, length = 0;
    char modmax[MaxModulNamLen],title[MaxMenuTitleLen];
    div_t res;
    
    error = ((char *)NULL == modul);
    if ( (long) strlen(modul) >= (long) MaxModulNamLen) error = 1;
    if(!error)
    {   
        strncpy(modmax,modul,MaxModulNamLen);
        length = (int) (long) strlen(modmax);
        length += (int) (long) strlen(HelStr);
        strcpy(title,strcat(modmax, HelStr));
        space = MaxLineS - (int) strlen(title);
        res = div(space,2);
        space = res.quot;
        for (i=1; i<=space; i++)
            if (EOF == printf(" ")) error =1; 
        if (EOF == printf("%s\n", title)) error =1;
        for (i=1; i<=space; i++)
            if (EOF == printf(" ")) error =1; 
        for (i=1; i<=(int)((long) strlen(title)); i++)
            if (EOF == printf("=")) error =1; 
    }               
    return(error);
}


int GenOutAcknowledge(int modus)
{
    int error =0;
    
    if (modus == ModusReturn)
    {
        if (EOF == printf("\n ----> hit RETURN to continue...\a")) 
            error = 1;
    }
    else if (modus == ModusNoRet)
        {
            if (EOF == printf(" -> RETURN ...\a"))
                 error = 1;
        }
        else error = 1;
    if (!error)
    {
        if (EOF == fflush(stdin)) error = 1;
        if (EOF == getchar()) error = 1;
    }
    return(error ? EOF : 0);
}

#ifdef modus_inter
int GenOutClrScr(int count,char *string)
{
    int i=0, j=0, error=0;
    
    j = bsintmin(MaxLineCntS, count);
    if (string == (char *) NULL)
    {
//        if (MaxLineCntS == j)
        	if (EOF == printf("\f")) error = 1;
//        else
        	for (i=1; i <= j; i++)
        		if (EOF == printf("\n")) error = 1;
        return(error ? EOF : 0);
    }
    else
    {
//        if (MaxLineCntS == j)
            strcat(string,"\f\0");
//        else
	        for (i=1; i <= j; i++)
	            strcat(string,"\n\0");
        return(error ? EOF : 0);
    }

}
#endif // modus_inter

void GenInName(char *str)
{	
#ifdef apple
 	int error = 0,len = 0;
	char hlstr[MaxFileName+1];
	
	sprintf(hlstr,"%#s",CurApName);
	strncpy(str,hlstr,MaxFileName);
#endif // ifdef apple
#ifdef ibm
	strncpy(str,argv,MaxFileName);
#endif // ifdef ibm
#ifdef sun
	strncpy(str,argv,MaxFileName);
#endif // ifdef sun
}

void GenExit(unsigned errno)
{
#ifdef apple
	DSErrCode = (unsigned long) errno;
#endif // ifdef apple
	exit(errno);
}
#endif // ifndef bsioc