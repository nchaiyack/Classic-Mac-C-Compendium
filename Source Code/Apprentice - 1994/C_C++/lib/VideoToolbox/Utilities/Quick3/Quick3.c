/*
Quick3.c 
Copyright (c) 1990-1993 Denis G. Pelli 
Quick3.c is a C program that makes maximum likelihood fits of simple models to
psychometric data. This is a replacement for, and to some extent is derived from
the old QUICK.FOR program written by A.B.Watson. See A.B.Watson (1979)
Probability summation over time. Vision Research 19, 515-522.

The principal difference between this program and the original QUICK, is in how
the goodness of fit is calculated. Both programs assess the goodness of fit by
comparing the log likelihoods of the model fit (usually Weibull) with a null
hypothesis. Minus two times this log likelihood ratio is approximately chi
square, with a number of degrees of freedom equal to the difference in degrees
of freedom of the two hypotheses. QUICK used an unconstrained psychometric
function, which adopted the actual proportion correct at each contrast. Quick3
uses a monotone hypothesis that is constrained to be monotonically increasing
as a function of contrast. The virtue of the latter hypothesis is that it is
easier to estimate its degrees of freedom when there are unequal numbers of data
points at each contrast. (However, this estimate is seat-of-my-pants.) Finally,
this program actually computes the significance for you, from the chi square
value and the estimate of its degrees of freedom. This significance value is of
some use in choosing a small fraction of your fits to discard (e.g. if
significance is less than 0.05). This significance is misleadingly low when both
hypotheses have nearly the same number of degrees of freedom (i.e. you only have
a few contrasts). Looking at the significance is helpful, but not a substitute
for thinking about whether your results are really reasonable.

Note that Quick3.c is just a front end, dealing with the user interface and
reading and writing files. All the work is done by one call to
PsychometricFit(). In many cases it will be convenient to call that routine
directly from your program that collects (or generates) the data, rather than
saving the data in a file for subsequent analysis by Quick3.c. If you want to
use a psychometric model other than the Weibull function all you need to do is
write a small function to implement it, based on Weibull.c. It should be easy.
PsychometricFit() uses whatever psychometric function is supplied in the call.

Quick3 reads in your data from a text file and analyzes them. The results are
shown on the screen, and are saved in two kinds of output file. The *.fit file is
in Excel format and has just a minimal one-line summary for each condition,
giving the parameters and goodness of fit. The *.plot file is in text format and
is suitable for plotting (e.g. by CricketGraph) of the psychometric data along
with the Weibull and Monotonic fits. For Macintosh users, the output files are
TEXT files, but have the appropriate creator so that double-clicking them opens
the data file in the appropriate application, either Excel or Cricket Graph.
For maximum convenience, change the definition of PLOT_CREATOR to match that of 
your favorite graphing program.

You should also have received a CricketGraph format files called "CricketGraph
Quick3" (for the original CricketGraph) and "CA-CricketGraphIII Quick3" (for the new
CricketGraphIII). Put the appropriate file in the same folder as the
CricketGraph application. When CricketGraph starts up it will read in the format
file, and later you'll be able to select that format from CricketGraph's Format
menu when you plot your data.

Quick3's input file format is simple and flexible. It is a text file. Lines
beginning with "#" are treated as comments. Comments are either copied directly
to the .fit file, or used as a name for the next "condition" (i.e. block of
data). The file must begin with at least one comment line. The last of several
contiguous comment lines will be taken to be the name for the following
condition. All the non-comment lines until the next comment will be interpreted
as the data for that condition.

An experiment usually consists of several conditions, or blocks of trials.
Within the condition the observer will be tested repeatedly at various
contrasts. The results consist of the number of trials correct at each contrast.
The results may be described by a set of "contrast records". Each contrast
record consists of a contrast (e.g. 0.012), the number of trials at that
contrast (e.g. 1 or 100), and the number of correct responses (e.g. 0 or 79).
Quick3 expects a contrast record to look like this:
0.012 100 79
The three numbers are all on one line, and separated by white space, so sscanf
can parse them. Anything on the line after the third number is ignored. All
contrast records until the next comment (line beginning with "#") or the end of
file are assumed to be from the same condition and are analyzed together. The
contrast records may be in any order. The records will be sorted into order of
ascending contrast. You may have any number of trials at each contrast, up to
about two billion (i.e. LONG_MAX in limits.h). It's okay to have multiple
records with equal contrasts. The data will be merged, adding up the trials at
that contrast. You can even be really lazy and just write out each trial as a
separate contrast record, in whatever order, without keeping track of how many,
etc.

You may add records with zero trials (and zero correct) so as to have fitted
values computed at those contrasts in the .plot file.

Here's an example of a data file, suitable for analysis by Quick3:
#This is a sample file. 
#The first condition is called "monocular" and has 60 trials at 6 contrasts.
#monocular
0.1 10 4
0.2 10 5
0.3 10 7
0.4 10 6 Anything after the specified data is ignored.
0.5 10 8
0.6 10 10
# The "binocular" condition is bigger: 1000 trials.
#binocular
 0.056     100    53
 0.064     100    53
 0.073     100    65
 0.083     100    77
 0.094     100    81
 0.107     100    84
 0.121     100    96
 0.138     100    99
 0.156     100   100
 0.178     100   100
#all done

Here's the resulting .fit file (tabs will space properly in Excel, but not here):
Condition	logAlpha	beta	gamma	delta	free params	signif.	Chi sq.	d.f.	trials	contrasts
#This is a sample file. 
#The first condition is called "monocular" and contains 60 trials at 6 contrasts.
monocular	-0.305	8.269	0.524	0	4	0.183	1.773	1	60	6
# The "binocular" condition is bigger: 1000 trials.
binocular	-1.015	3.811	0.482	0	4	0.479	5.521	6	1000	10
#all done										

SOURCES:

Quick3.h
LogLikelihood.c
MonotonicFit.c
PsychometricFit.c
Quick3.c
SortAndMergeContrasts.c
Weibull.c
#From Denis Pelli's VideoToolbox:
VideoToolbox.h
Binomial.c
ChiSquare.c
Normal.c
SetFileInfo.c		# Used only on the Macintosh
#From Numerical Recipes in C:
nr.h
NRUTIL.h
BRENT.C
F1DIM.C
LINMIN.C
MNBRAK.C
NRUTIL.C
POWELL.C

HISTORY:
4/7/90		dgp	wrote it.
4/10/90		dgp	changed .fit format from %.3f to %.4f. Accept only printing characters
				in the condition name, ignoring everything after the first non-printing
				character. This deals with the fact that if an Excel file is used for
				input, there are lots of trailing tabs that should be ignored. Checked
				that files were actually open before closing 'em. Jeesh!
10/29/90	dgp	tidied up comments.
1/20/91		dgp	updated calls to BinomialUpperBound & BinomialUpperBound to
				conform to new definition.
8/24/91		dgp	Made compatible with THINK C 5.0.
11/17/92	dgp Now SetVol() after calling SFGetFile, so that file can be in any folder,
				not just the same folder that Quick3 is in.
1/19/93		dgp	put #ifs around the Mac-dependent code.
2/20/93		dgp	added call to Require().
8/5/93		dgp	moved call to Require() into a separate main(), since application
				failed on Mac Plus before it got to the call to Require(). Thanks
				to Robert Friedman friedman@cortex.health.ufl.edu for reporting the
				problem.
10/13/93	dgp	added comments about CricketGraphIII. Allow compile-time choice of
				Mac file's creator.
3/13/94	dgp	replaced MyFGets by standard fgets. Edited the code slightly, making
				it a bit more resilient of errors in data file format.
*/
//#define PLOT_CREATOR	'CGRF'	/* Create Macintosh Cricket Graph file */
#define PLOT_CREATOR	'CRGR'	/* Create Macintosh CA-Cricket Graph III file */
#include "VideoToolbox.h"
#include "Quick3.h"
#include <nr.h>				/* Numerical Recipes in C*/
#include <assert.h>
#if MACINTOSH
	#include <QuickDraw.h>
	#include <StandardFile.h>
#endif
void Quick3(void);

void main(void)
{
	#if MACINTOSH
		Require(0);	// check for any required cpu and fpu.
	#endif
	Quick3();
}
void Quick3(void)
{
	static dataRecord data,monotonicData;
	contrastRecord *cPtr;
	static paramRecord params, initParams;
	double *paramPtr=&params.logAlpha;	/* a generic way of accessing the parameters */
	int i;
	double chiSquare,modelLL,monotonicLL;	/* log likelihood */
	int chiSquareDF,modelDF,monotonicDF;	/* degrees of freedom */
	double significance;
	FILE *dataFile=NULL,*fitFile=NULL,*plotFile=NULL;
	char dataFileName[50],fitFileName[50],plotFileName[50],string[100];
	char conditionName[100];
	long trials;
	unsigned int a;
	PsychometricFunctionPtr ModelFunction=&Weibull;	/* a psychometric function */
	static const char modelName[]="Weibull";
	static const char paramName[PARAMS][16]={"logAlpha","beta","gamma","delta"};
	#if MACINTOSH
		Point where;
		static SFTypeList typeList;
		static SFReply reply;
		assert(StackSpace()>4000);
	#endif

	/* initial values for the parameters of the psychometric function */
	params.logAlpha=0.0;
	params.beta=3.5;
	params.gamma=0.5;
	params.delta=0.01;
	printf("\n");	/* ask THINK C to init QuickDraw */
	
	#if MACINTOSH
		where.h=100;
		where.v=100;
		typeList[0]='TEXT';
		reply.version=0;
		SFGetFile(where,"\p",NULL,1,typeList,NULL,&reply);
		if(!reply.good)PrintfExit("Couldn't open file.\n");
		SetVol(NULL,reply.vRefNum);	/* look in that folder */
		sprintf(dataFileName,"%#s",reply.fName);
	#else
		// Insert code here to get data file name into dataFileName.
	#endif
	dataFile=fopen(dataFileName,"r");
	if(dataFile==NULL)
		PrintfExit("\007Sorry, can't find file \"%s\".\007\n",dataFileName);
	printf("Reading \"%s\"\n",dataFileName);
	strcpy(string,dataFileName);
	if(strstr(string,".data"))strcpy(strstr(string,".data"),"");
	else if(strstr(string,".yes"))strcpy(strstr(string,".yes"),"");
	if(strlen(string)>0){
		sprintf(fitFileName,"%s.fit",string);
		printf("Creating output files:\n%s\n%s.<condition name>.plot\n"
			,fitFileName,string);
		fitFile=fopen(fitFileName,"w");
		if(fitFile==NULL)
			PrintfExit("Sorry, I can't create file \"%s\".\007\n",fitFileName);
		#if MACINTOSH
			SetFileInfo(fitFileName,'TEXT','XCEL');		/* Excel file */
		#endif
	}
	modelDF=PARAMS;	/* number of parameters to be adjusted in fitting */
	printf("How many parameters shall be adjustable? 0 to %d (%d):",PARAMS,modelDF);
	gets(string);
	sscanf(string,"%d",&modelDF);
	for(i=1;i<modelDF;i++){
		printf("Initial value for %s? (%.2f):",paramName[i],paramPtr[i]);
		gets(string);
		sscanf(string,"%lf",&paramPtr[i]);
	}
	for(i=modelDF;i<PARAMS;i++){
		printf("Fixed value for %s? (%.2f):",paramName[i],paramPtr[i]);
		gets(string);
		sscanf(string,"%lf",&paramPtr[i]);
	}
	initParams=params;
	
	printf("Reading %s\n",dataFileName);
	if(fitFile)fprintf(fitFile,"Condition\t%s\t%s\t%s\t%s\tfree params"
		"\tsignif.\tChi sq.\td.f.\ttrials\tcontrasts\n"
		,paramName[0],paramName[1],paramName[2],paramName[3]);
	while(!feof(dataFile)){
		// last comment line before data is used as a name for the condition
		fgets(string,sizeof(string),dataFile);
		printf("\n");
		while(a=fgetc(dataFile),ungetc(a,dataFile),a=='#' || a==EOF){
			printf("%s",string);								/* echo comment */
			if(fitFile)fprintf(fitFile,"%s",string);
			fgets(string,sizeof(string),dataFile);
			if(feof(dataFile))break;
		}
		/* get condition name. Strip leading # and any trailing junk */
		strcpy(conditionName,&string[1]);
		for(i=0;i<strlen(conditionName);i++)
			if(!isprint(conditionName[i]))conditionName[i]=0;
		printf("\n");
		data.contrasts=0;
		while(a=fgetc(dataFile),ungetc(a,dataFile),a!='#' && a!=EOF){	/* read data */
			if(data.contrasts>=MAX_CONTRASTS){
				SortAndMergeContrasts(&data);
				if(data.contrasts>=MAX_CONTRASTS)
					PrintfExit("Quick3: can't handle more than %d different contrasts.\n"
						,MAX_CONTRASTS);
			}
			fgets(string,sizeof(string),dataFile);
			i=data.contrasts;
			sscanf(string,"%lf\t%ld\t%ld"
				,&data.c[i].contrast,&data.c[i].trials,&data.c[i].correct);
			data.contrasts++;
		}
		if(data.contrasts==0){
			// No data. Skip to next condition.
			continue;
		}
		SortAndMergeContrasts(&data);
		trials=0;
		for(i=0;i<data.contrasts;i++)trials+=data.c[i].trials;	/* total trials */
		params=initParams;	/* initial guess & fixed values */
		if(modelDF>0)
			params.logAlpha=log(data.c[data.contrasts/2].contrast)/log(10.0);	/* median contrast */
		printf("%s\n",conditionName);
		printf("\t%s   %s   %s   %s  contrasts trials signif. Chi sq.   d.f.\n",
			paramName[0],paramName[1],paramName[2],paramName[3]);
		printf("Guess:");
		printf("\t%7.2f%8.1f%8.2f%8.2f\n",
			paramPtr[0],paramPtr[1],paramPtr[2],paramPtr[3]);
		significance=PsychometricFit(&params,ModelFunction,&data,&modelLL,modelDF,
			&chiSquare,&chiSquareDF);
		printf("Fit:");
		printf("\t%7.2f%8.1f%8.2f%8.2f",
			paramPtr[0],paramPtr[1],paramPtr[2],paramPtr[3]);
		printf("%8d%8ld",data.contrasts,trials);
		printf("%8.2f%8.1f%8d\n",significance,chiSquare,chiSquareDF);
		if(fitFile)fprintf(fitFile,"%s"
				"\t%7.4f\t%7.4f\t%7.4f\t%7.4f\t%7d"
				"\t%7.4f\t%7.4f\t%7d"
				"\t%7ld\t%7d\n",
				conditionName,
				paramPtr[0],paramPtr[1],paramPtr[2],paramPtr[3],modelDF,
				significance,chiSquare,chiSquareDF,
				trials,data.contrasts);
	
		/* Now create plot file */
		if(fitFile){
			strcpy(plotFileName,fitFileName);
			plotFileName[strlen(plotFileName)-strlen(".fit")]=0;	/* strip off the ".fit" */
			sprintf(plotFileName,"%s.%s.plot",plotFileName,conditionName);
			plotFile=fopen(plotFileName,"w");
			if(plotFile==NULL){
				PrintfExit("Sorry, I can't create file \"%s\".\n\007",plotFileName);
			}
			#if MACINTOSH
				SetFileInfo(plotFileName,'TEXT',PLOT_CREATOR);	/* specify graphing program */
				if(PLOT_CREATOR=='CGRF')fprintf(plotFile,"*\n");
			#endif
			monotonicData=data;
			MonotonicFit(&monotonicData,&monotonicLL,&monotonicDF);	/* overwrites data with fit */
			fprintf(plotFile,"Contrast\t%s\tLower bound\tUpper bound\t%s fit\tMonotone fit\tTrials\tCorrect\tParameters\n",
				conditionName,modelName);
			for(i=0;i<data.contrasts;i++){
				cPtr=&data.c[i];
				fprintf(plotFile,"%.3f",cPtr->contrast);
				if(cPtr->trials>0){
					fprintf(plotFile,"\t%.3f",cPtr->correct/(double)cPtr->trials);
					fprintf(plotFile,"\t%.3f\t%.3f",
						BinomialLowerBound(0.95,cPtr->correct,cPtr->trials),
						BinomialUpperBound(0.95,cPtr->correct,cPtr->trials));
				}
				else
					fprintf(plotFile,"\t\t\t");
				fprintf(plotFile,"\t%.3f",(*ModelFunction)(cPtr->contrast,&params));
				if(cPtr->trials>0)
					fprintf(plotFile,"\t%.3f\t%5ld\t%5ld",
						monotonicData.c[i].correct/(double)monotonicData.c[i].trials,
						cPtr->trials,cPtr->correct);
				else fprintf(plotFile,"\t\t\t");
				if(i<PARAMS)fprintf(plotFile,"\t%s=%.3f",paramName[i],paramPtr[i]);
				if(i==0)fprintf(plotFile,"\talpha=%.4f",pow(10.0,paramPtr[0]));
				if(i==PARAMS)fprintf(plotFile,"\tsignificance=%.3f",significance);
				fprintf(plotFile,"\n");
			}
			if(plotFile)fclose(plotFile);
		}
	}
	if(fitFile)fclose(fitFile);
	if(dataFile)fclose(dataFile);
}

