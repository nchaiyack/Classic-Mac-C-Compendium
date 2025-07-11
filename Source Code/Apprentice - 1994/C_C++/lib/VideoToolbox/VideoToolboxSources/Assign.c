/*
Assign.c

Assign is a portable runtime C interpreter that reads and executes any text
"assignment" file that contains only C assignments and comments, e.g.
	viewingDistance=57.0;	// inches
	
See Assign.note for documentation.

(It would be nice to add a flag for WriteAssignmentsToFile() that would request
a read-after-write check, ala WriteLuminanceRecord in ReadLuminanceRecord.c.)

HISTORY
7/30/91 dgp	wrote it as "ReadAssignments.c".
8/4/91	dgp added new routines and renamed the old ones. Everything
			seems to work, but has not been thoroughly tested.
8/5/91	dgp	MPW C 3.2 now compiles it without error messages. The MPW C macro processor,
			contrary to Standard C, finds comment symbols inside strings and finds 
			preprocessor directives even when # is not the first nonblank character.
8/24/91	dgp	Made compatible with THINK C 5.0.
			Changed ReadAssignment file to return an error code instead of aborting
			if it can't open the file.
8/26/91	dgp	Added SetVariable() and noted, above, that the Variable array
			is terminated by an element with its "type" field set to zero. (Thanks
			to Evan Relkin for pointing out the omission.)
4/1/92	dgp	Tidied up the documentation above.
4/2/92	dgp Introduced ReadAssignmentBlock() and AssignmentLineWasBlank(). 
			Deleted ReadAssignments().
4/5/92	dgp	ReadAssignmentFile() now closes the file before returning.
4/17/92	dgp	Expanded the explanation of flags, as requested by Evan Relkin.
5/14/92	dgp	Expanded the explanation of stringType, "".
8/4/92	dgp	Added PrintAnAssignment(), which prints out the value of a variable,
			as an assignment statement, suitable for reading by the ReadAssignment
			routines.
10/24/92 dgp Eliminated double spacing that occurred after //-style comment when 
			assignEchoComment was requested.
3/4/93	dgp	changed the definition of emptyString slightly so that this file could be
			compiled as a code resource.
5/24/93	dgp	Assignment lines may now be continued by putting a backslash at the end of
			the line.
			Strings may include literal linefeeds, rather than just "\n".
			Printed string assignments translate linefeeds to "\n".
			Skip unknown variables unless flags&assignReportUnknown is true.
5/27/93	dgp	Added "dim" field to Variable structure, and added support for
			it in all the routines.
			Added floatType.
			Input line length is still limited to BUFFER_SIZE (presently 512) but
			strings (broken up into lines of legal length and automatically 
			concatenated during reading) can be of any length that will fit
			in memory.
5/28/93	dgp	Added support for hex-encoded strings.
			Created new bottleneck procedure ReadLine that does all the reading.
5/31/93	dgp	assignEchoAssignments now calls PrintAnAssignment. Removed "\n" from
			PrintAnAssignment. Made all Print routines return assignment count,
			which is now redefined as the number of array elements or scalars
			that were assigned or printed. Now parse names so that
			known and unknown identifiers are treated uniformly; these new
			naming rules allow only simple C identifiers joined by "." and "->".
6/1/93	dgp	introduced assignNoPrintfExit flag that causes all routines to handle
			errors discreetely, simply returning an error number to the caller.
6/2/93	dgp PrintAnAssignment now prints multiple assignments per 80 character line, 
			adding as many continuation lines as necessary. Quoted strings (and hex
			data strings) are now broken up into 80-character lines; continuation
			lines are indented four spaces. Added comment field to Variable structure
			and added support for it in PrintAnAssignment. Renamed all published
			enum values to begin with "assign". Renamed this file "Assign.c". Added
			UnequalVariables(). CheckAVariable checks that variable name is legal.
6/5/93	dgp	Removed all Macintosh dependencies, leaving pure Standard C plus a few
			C++ style comments.
6/16/93	dgp	Removed prohibition: arrays of stringType are now ok.
			InitializeAVariable now calls CheckAVariable with "flags" unmodified.
			CheckAVariable now checks for any illegal suffix in name.
			Subscripted unknown variables are now correctly reported as "UNKNOWN" 
			rather than "OUT OF BOUNDS".
6/29/93	dgp	added shortDoubleType.
7/10/93	dgp	restored Standard C compatibility by using "short double" only if
			SHORT_DOUBLE_OK, since Standard C doesn't allow that type.
7/19/93	dgp	noted that file should be opened in text mode, not binary mode, so that
			\r characters will be translated properly.
7/29/93	dhb & dgp Found and fixed the newline problem that appears in the MATLAB
			environment, as documented above.
7/31/93	dgp	Added support for multidimensional arrays, with up to ASSIGN_DIMS
			dimensions. Added SetVariableArray(), changed SetVariable() and
			SetVariableFirstLast(), and deleted SetAVariableFirstLast().
8/2/93	dgp	Added dynamic allocation of ptrType arrays, based on scanning the current
			gulp of the assign file.
8/12/93	dgp	Renamed "Variable" to "Description". Renamed most of the routines. 
			The header file Assign92.h provides for backward compatibility.
8/21/93	dhb,dgp	For compatibility with MATLAB we avoid using int arguments in stdio 
			calls because when MATLAB is true we'll be using MPW's stdio
			library, and its int is 4 bytes whereas the THINK C int is 2 bytes.
		dhb,jms	Changed "wa" in fopen in PrintAssignmentsToFile to "a".
		dhb,jms,dgp Moved the NL and NEWLINE macros into VideoToolbox.h.
9/2/93	dgp	Expanded documentation of FreePtrVariables. Fixed minor bugs
			in PrintAVariable for hex encoded Ptr variables so that dimensionality
			of what was written is recovered when it's read.
9/7/93	dgp	Introduced STDIO_INT.
9/7/93	dhb,jms,dgp	Added OpenCalFileWrite, OpenCalFileRead, OpenCalFileReadAndCheck,
			AppendDescriptions, CopyDescriptions, AllocateDescriptions,
			FreeDescriptions, NumberOfDescriptions, NullDescription, and 
			IsNullDescription.
9/9/93	dhb	Added ReadAssignmentStream, to read the rest of a file stream as a single
			gulp.
9/11/93	dgp	rewrote ReadAssignmentFile to just open the file and call 
			ReadAssignmentStream.
9/12/93	dhb	Changed STDIO_INT to PRINTF_INT, added inclusion of VideoToolbox
            when MATLAB is on.
9/15/93	dgp	Moved documentation to Assign.note.
9/16/93	dgp	Merged Assign.h into VideoToolbox.h.
3/4/94	dgp	Increased number of float and double digits produced by PrintAnAssignment 
			up to that specified in float.h. This should eliminate spurious failure
			of the read-back checks.
			Enhanced PrintQuotedString to correctly handle strings containing the double
			quote character.
3/28/94	dgp Inserted space before comments (i.e. before "/*") and now pad the inside
			of the comment delimiters with option-space instead of space for more 
			pleasing word wrapping when the assignment file is viewed in that way.
*/
#include "VideoToolbox.h"
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
IsNan and IsFinite--see VideoToolbox.h--should work on most Intel and Motorola
processors. If IsNan and IsFinite don't work on your computer, don't fret. They
are used solely by UnequalDescribedVarPair. A quick fix is to use the following
definitions, the only bad effect of which will be that comparisons of NANs may
appear spuriously unequal.
*/
#ifndef IsFinite
	#define IsNan(x) 0
	#define IsFinite(x) 1
#endif
Description SetAVariable(short type,void *ptr,char *name,unsigned long dim
	,const char *comment);	/* old style */

/* the following are private, not intended for use outside this file */
static int lineWasBlank=1;	/* used in ReadAssignmentLine(),	*/
			/* NextTokenInThisOrNextLine(), and AssignmentLineWasBlank().*/
#define IsIntegral(type) ((type)>=charType&&(type)<=unsignedLongType || (type)>=charPtrType&&(type)<=unsignedLongPtrType)
#define IsFloating(type) ((type)>=floatType&&(type)<=doubleType || (type)>=floatPtrType&&(type)<=doublePtrType)
#define IsString(type) (type==stringType)
#define IsPtr(type) ((type)>=charPtrType&&(type)<=doublePtrType)
long CheckADescription(Description *d,const char *here,short flags);
long CheckDescriptions(Description d[],const char *here,short flags);
void FreeAPtrDescription(Description *d,short flags);
void FreeAStringDescription(Description *d,short flags);
char *GetName(char **sPtr,short flags);
char *GetQuotedString(FILE *stream,char *lineBuffer,char *s,char **sPtr,short flags);
long HexAssignment(FILE *stream,char *lineBuffer,char **sPtr,Description *d
	,int subscriptDims,long index,long *bytesPtr,short flags);
char *NextToken(FILE *stream,char *lineBuffer,char *s,short flags);
char *NextTokenInLine(FILE *stream,char *lineBuffer,char *s,short flags);
char *NextTokenInThisOrNextLine(FILE *stream,char *lineBuffer,char *s,short flags);
int ParseName(char **sPtr);
void ParsingError(char *s,char *format,...);
long PrintAnAssignmentOfElement(FILE *stream,Description *d,long index,short flags);
void PrintQuotedString(FILE *stream,short *lineLengthPtr,const char *s);
char *ReadLine(char *lineBuffer,size_t bufferSize,FILE *stream,short flags);
double strtodN(char *s,char **sPtr);
long strtolN(char *s,char **sPtr,short flags);
unsigned long strtoulN(char *s,char **sPtr,short flags);
char strtoc(char *s,char **sPtr);
#define streq2(s1,s2) (strncmp(s1,s2,strlen(s2))==0)
#define streq(s1,s2) (strcmp(s1,s2)==0)
long Dims(Description *d);
char *ElementName(char *s,Description *d,long index);
void *ElementPtr(Description *d,long index);
long Elements(Description *d);
void ElementSubscript(Description *d,long index,long sub[ASSIGN_DIMS]);
long ElementIndex(Description *d,long sub[ASSIGN_DIMS]);
int GetDimensions(Description *d,int subscriptDims,long subscript[ASSIGN_DIMS],short flags);
int AllocateDescribedPtrVars(Description description[],const char *here,short flags);

#ifndef TRUE
	#define FALSE	0
	#define TRUE	1
#endif
#define TOLERANCE	1e-6	/* fractional difference between two "equal" floats */
#define BUFFER_SIZE 512
#define ECHO_ASSIGNMENTS	(flags&assignEchoAssignments && !(flags&assignScan))
#define ECHO_COMMENTS		(flags&assignEchoComments && !(flags&assignScan))
#define ECHO_FILE			(flags&assignEchoFile && !(flags&assignScan))
#define PRINTF_EXIT			!(flags&assignNoPrintfExit)
short typeSize[]={0
	,sizeof(char),sizeof(unsigned char)
	,sizeof(short),sizeof(unsigned short)
	,sizeof(long),sizeof(unsigned long)
	,sizeof(float)
	#if SHORT_DOUBLE_OK
		,sizeof(short double)
	#endif
	,sizeof(double)
	,sizeof(char),sizeof(unsigned char)
	,sizeof(short),sizeof(unsigned short)
	,sizeof(long),sizeof(unsigned long)
	,sizeof(float)
	#if SHORT_DOUBLE_OK
		,sizeof(short double)
	#endif
	,sizeof(double)
	,sizeof(char *)
	,0};
char typeName[][20]={"none"
	,"char","unsigned char","short","unsigned short"
	,"long","unsigned long","float"
	#if SHORT_DOUBLE_OK
		,"short double"
	#endif
	,"double"
	,"char *","unsigned char *","short *","unsigned short *"
	,"long *","unsigned long *","float *"
	#if SHORT_DOUBLE_OK
		,"short double *"
	#endif
	,"double *"
	,"string"
	,"unknown"};
enum{assignNeedMore=0x4000,assignAlreadyChecked=0x2000
,assignScan=0x100,assignLocalCall=0x200
};/*flags used only in this file*/

/*
ROUTINE: OpenCalFileReadAndCheck
PURPOSE:
	Open up a calibration file for reading.
	
	If the file exists in the current directory, it is used.
	If not, then if it exists in the preferences folder, it used.
*/

FILE *OpenCalFileReadAndCheck(char *filename)
{
	FILE *stream;
	
	stream = OpenCalFileRead(filename);
	if (stream == NULL)
		PrintfExit("OpenCalFileReadAndCheck: cannot open file \"%s\"",filename);
	return(stream);	
}

/*
ROUTINE: OpenCalFileRead
PURPOSE:
	Open up a calibration file for reading.  Don't abort on error.
	
	If the file exists in the current directory, it is used.
	If not, then if it exists in the preferences folder, it used.
*/

FILE *OpenCalFileRead(char *filename)
{
	FILE *stream;
	
	/* Try the current directory */
	stream = fopen(filename,"r");
	if (stream != NULL) return(stream);
	
	/* Try the preferences folder */
	OpenPreferencesFolder();
	stream = fopen(filename,"r");
	ClosePreferencesFolder();
	return(stream);	
}

/*
ROUTINE: OpenCalFileWrite
PURPOSE:
	Open up a calibration file for appending.
	
	If the file exists in the current directory, it is used.
	If not, then if it exists in the Preferences folder, it used.
	If not, the file is created in the Preferences folder.
*/

FILE *OpenCalFileWrite(char *filename)
{
	FILE *stream;
	
	/* Try in the current directory */
	stream = fopen(filename,"r");
	if (stream != NULL) {
		fclose(stream);
		stream = fopen(filename,"a");
		if (stream == NULL)
			PrintfExit("OpenCalFileWrite: cannot reopen file \"%s\"",filename);
		return(stream);
	}
	
	/* Try the Preferences folder */
	OpenPreferencesFolder();
	stream = fopen(filename,"r");
	ClosePreferencesFolder();
	if (stream != NULL) {
		fclose(stream);
		OpenPreferencesFolder();
		stream = fopen(filename,"a");
		ClosePreferencesFolder();
		if (stream == NULL)
			PrintfExit("OpenCalFileWrite: cannot reopen file \"%s\" in Preferences folder"
				,filename);
		return(stream);
	}
	
	/* Create it in the Preferences folder */
	OpenPreferencesFolder();
	stream = fopen(filename,"a");
	ClosePreferencesFolder();
	if (stream == NULL)
		PrintfExit("OpenCalFileWrite: cannot create file \"%s\" in Preferences folder.",filename);
	return(stream);
}

/*
ROUTINE: AppendDescriptions
PURPOSE:
	Appends the second descriptions array onto the end of the first,
	which is reallocated with more space.
	
	The source array is not freed; the caller should do that.
*/
void AppendDescriptions(Description **d,Description *s)
{
	Description *dTemp;
	long n1, n2, n;
	
	n1 = NumberOfDescriptions(*d);
	n2 = NumberOfDescriptions(s);
	n = n1+n2;
	dTemp = AllocateDescriptions(n);
	CopyDescriptions(dTemp,*d);
	CopyDescriptions(dTemp+n1,s);
	FreeDescriptions(*d);
	*d=dTemp;
}

/*
ROUTINE: CopyDescriptions
PURPOSE:
	Copy one null-terminated array of descriptions to another, which is assumed to
	be big enough.
*/
void CopyDescriptions(Description *d,Description *s)
{
	long i = 0;
	
	while ( !IsNullDescription(s[i]) ) {
		d[i]=s[i];
		i++;
	}
	d[i]=NullDescription();
}

/*
ROUTINE: AllocateDescriptions
PURPOSE:
	Allocate space for variable descriptions.
	Adds one to the passed size to hold the null descriptor.
	Nulls the first element.
*/
Description *AllocateDescriptions(long n)
{
	Description *d;

	d = calloc(n+1,sizeof(Description));
	if (d == NULL) {
		PrintfExit("AllocateDescription: memory allocation failure.");
	}
	d[0]=NullDescription();
	return(d);
}


/*
ROUTINE: FreeDescriptions
PURPOSE:
	Free the description space.  Does not affect the described variables
	themselves.
*/
void FreeDescriptions(Description *d)
{
	free(d);	
}

/*
ROUTINE: NumberOfDescriptions
PURPOSE:
	Find the size of an array of descriptions.
	
	Does not count the trailing null description.
*/
long NumberOfDescriptions(Description *d)
{
	long i=0;
	
	while ( !IsNullDescription(d[i]) ) i++;
	return(i);
}

/*
ROUTINE: NullDescription
PURPOSE:
	Returns a null description.
	Standard C specifies that static variables are initialized to zero.
*/
Description NullDescription(void)
{
	static Description d;
	return d;
}

Description Describe(short type,void *ptr,char *name,const char *comment)
{
	static Description var;

	var=DescribeArray(type,ptr,name,comment,0L);
	if(IsPtr(type)) var.sizedOnce=var.sized=0;
	return var;
}

Description DescribeArray(short type,void *ptr,char *name,const char *comment,...)
/* WARNING: the dimensions must be (long) and the last argument must be 0L. */
{
	va_list args;
	static Description var;
	int i;
	long dim;
	
	var.name=name;
	var.ptr=ptr;
	var.type=type;
	var.firstElement=0;
	for(i=0;i<ASSIGN_DIMS;i++)var.dim[i]=0;
	var.comment=comment;
	va_start(args,comment);
	for(i=0;;i++){
		dim=va_arg(args,long);
		if(dim==0)break;
		if(i>=ASSIGN_DIMS)PrintfExit("Describe/Array/FirstLast: "
			"too many dimensions (or missing final 0L) for \"%s\".\n",var.name);
		var.dim[i]=dim;
	}
	va_end(args);
	if(var.type!=0 && var.ptr==NULL)
		PrintfExit("Describe/Array/FirstLast: \"%s\" ptr is NULL.\n",var.name);
	/* Note: we're clearing the malloced flag, which is the only safe assumption. */
	var.malloced=0;
	var.sizedOnce=var.sized=1;
	return var;
}
Description DescribeFirstLast(short type,void *ptr,char *name
	,const char *comment,long firstElement,long lastElement)
{
	static Description var;
	long dim;
	
	dim=1+lastElement-firstElement;
	var=DescribeArray(type,ptr,name,comment,dim,0L);
	var.firstElement=firstElement;
	return var;
}
Description SetAVariable(short type,void *ptr,char *name,unsigned long dim
	,const char *comment)
/* Old style, retained solely for compatibility */
{
	if(dim==0)return Describe(type,ptr,name,comment);
	else return DescribeArray(type,ptr,name,comment,dim,0L);
}
int AllocateDescribedPtrVars(Description description[],const char *here,short flags)
{
	register Description *d;
	
	for(d=description;d->type!=0;d++){
		if(IsPtr(d->type) && d->sizedOnce && !d->malloced){
			assert(d->ptr!=NULL);
			*(void **)d->ptr=malloc(Elements(d)*typeSize[d->type]);
			if(*(void **)d->ptr==NULL){
				if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes for \"%s\".\n"
					,here,Elements(d)*typeSize[d->type],d->name);
				else return assignMemoryError;
			}
			d->sized=d->malloced=1;
			InitializeADescribedVar(d,flags);
		}
	}
	return 0;
}

long CheckDescriptions(Description d[],const char *here,short flags)
{
	long error;
	
	if(flags&assignAlreadyChecked)return 0;
	for(;d->type!=0;d++){
		error=CheckADescription(d,here,flags);
		if(error)return error;
	}
	return 0;
}

long CheckADescription(Description *d,const char *here,short flags)
{
	char *s,*sOk;
	
	if(flags&assignAlreadyChecked)return 0;
	if(d->type<=0 || d->type>=unknownType){
		if(PRINTF_EXIT)PrintfExit("%s: \"%s\" has unknown type %ld.\n"
			,here,d->name,(long)d->type);
		else return assignVariableError;
	}
	if(!IsPtr(d->type) && d->ptr==NULL){
		if(PRINTF_EXIT)PrintfExit("%s: \"%s\" ptr is NULL.\n"
			,here,d->name);
		else return assignVariableError;
	}
	/* Squeeze any space out of the name */
	for(s=sOk=d->name;*s!=0;s++)if(!isspace(*s))*sOk++=*s;
	*sOk=0;
	/* Check name's syntax */
	s=d->name;
	if(ParseName(&s) || *s!=0){
		if(PRINTF_EXIT)PrintfExit("%s: illegal name \"%s\".\n"
			,here,d->name);
		else return assignVariableError;
	}
	return 0;
}

int UnequalDescribedVars(Description d1[],Description d2[],short flags)
/*
Compares the data pointed to by the two Description arrays and returns
true if the structs are legal and the data are equal, false otherwise.
*/
{
	int error;
	
	for(;d1->type!=0 && d2->type!=0;d1++,d2++){
		error=UnequalDescribedVarPair(d1,d2,flags);
		if(error)return error;
	}
	return 0;
}

long Elements(Description *d)
{
	long elements=1;
	int i;

	for(i=0;i<ASSIGN_DIMS && d->dim[i]>0;i++)elements*=d->dim[i];
	return elements;
}
long Dims(Description *d)
{
	int i;

	for(i=0;i<ASSIGN_DIMS && d->dim[i]>0;i++);
	return i;
}
void *ElementPtr(Description *d,long index)
/* Returns NULL if PtrType array not allocated. */
{
	char *ptr;
	
	if(IsPtr(d->type)){
		assert(d->ptr!=NULL);
		ptr=*(void **)d->ptr;
	}else ptr=d->ptr;
	if(ptr!=NULL && d->dim[0]>0){
		index+=d->firstElement;
		ptr+=index*typeSize[d->type];
	}
	return ptr;
}
char *ElementName(char *s,Description *d,long index)
{
	long sub[ASSIGN_DIMS];
	int i;
	
	ElementSubscript(d,index,sub);
	sprintf(s,"%s",d->name);
	for(i=0;i<Dims(d);i++)sprintf(s,"%s[%ld]",s,sub[i]);
	return s;
}
char *DescriptionNameDimensions(Description *d);
char *DescriptionNameDimensions(Description *d)
{
	long sub[ASSIGN_DIMS];
	int i;
	static char s[64];
	
	sprintf(s,"%s",d->name);
	for(i=0;d->dim[i]>0;i++)sprintf(s,"%s[%ld]",s,d->dim[i]);
	assert(strlen(s)<sizeof(s));
	return s;
}
void ElementSubscript(Description *d,long index,long sub[ASSIGN_DIMS])
{
	int i;

	for(i=0;i<ASSIGN_DIMS;i++)sub[i]=0;
	if(Dims(d)){
		for(i=Dims(d)-1;i>=0;i--){
			sub[i]=index%d->dim[i];
			index/=d->dim[i];
		}
		sub[Dims(d)-1]+=d->firstElement;
	}
}
long ElementIndex(Description *d,long sub[ASSIGN_DIMS])
{
	int i;
	long index=0;

	if(Dims(d)){
	sub[Dims(d)-1]-=d->firstElement;
	for(i=0;i<Dims(d);i++){
		index*=d->dim[i];
		index+=sub[i];
	}
	sub[Dims(d)-1]+=d->firstElement;
	}
	return index;
}
int GetDimensions(Description *d,int subscriptDims,long subscript[ASSIGN_DIMS],short flags)
{
	short i;
	
	if(d->sizedOnce && Dims(d)!=subscriptDims)
		return assignInconsistentDimensionsError;
	if(!d->sized){
		for(i=0;i<subscriptDims;i++)
			if(d->dim[i]<subscript[i]+1)d->dim[i]=subscript[i]+1;
		d->sizedOnce=1;
	}
	return 0;
}

int UnequalDescribedVarPair(Description *d1,Description *d2,short flags)
/*
Compares the data pointed to by the two Description structs and returns true if the
structs are legal and the data are equal, false otherwise. Comparison of floats
and doubles allows a tolerance of +/- one part in a million--because converting
to and from decimal may lose some precision--and ignores any NANs' indices (NAN04
vs NANFF)--because they're not preserved.
*/
{
	unsigned char *p1,*p2;
	long i,size,elements;
	int error;
	double a,b,e;
	static const char here[]="UnequalDescribedVarPair";
	char name[32+16*ASSIGN_DIMS];
	
	/* Squeeze space out of names */
	error=CheckADescription(d1,here,flags);
	if(error<0)return error;
	error=CheckADescription(d2,here,flags);
	if(error<0)return error;
	if(d1->type!=d2->type
		|| !streq(d1->name,d2->name) 
		|| d1->firstElement!=d2->firstElement
		|| !streq(d1->comment,d2->comment))
			{error=assignInconsistentDescriptionsError;goto done;}
	for(i=0;i<ASSIGN_DIMS;i++)if(d1->dim[i]!=d2->dim[i])
		{error=assignInconsistentDescriptionsError;goto done;}
	p1=ElementPtr(d1,0);
	p2=ElementPtr(d2,0);
	size=typeSize[d1->type];
	elements=Elements(d1);
	if(!IsString(d1->type)){
		if(memcmp(p1,p2,size*elements)!=0){
			if(!IsFloating(d1->type)){error=assignUnequalDataError;goto done;}
			for(i=0;i<elements;i++){
				switch(d1->type){
				case floatType:
					a=*(float *)p1;
					b=*(float *)p2;
					break;
				#if SHORT_DOUBLE_OK
					case shortDoubleType:
						a=*(short double *)p1;
						b=*(short double *)p2;
						break;
				#endif
				case doubleType:
					a=*(double *)p1;
					b=*(double *)p2;
					break;
				}
				/*  compare doubles */
				/*  ignore NaN type, since it's not preserved */
				if(IsNan(a)==0 || IsNan(b)==0){
					if(!IsFinite(a) || !IsFinite(b))
						{error=assignUnequalDataError;goto done;}
					e=a/b-1.0;
					if(e>TOLERANCE || e<-TOLERANCE)
						{error=assignUnequalDataError;goto done;}
				}
				p1+=size;
				p2+=size;
			}
		}
	}else for(i=0;i<elements;i++){							/* compare strings */
		if(*(char **)p1!=*(char **)p2){
			if((*(char **)p1==NULL) || (*(char **)p2==NULL)
				|| !streq(*(char **)p1,*(char **)p2))
					{error=assignUnequalDataError;goto done;}
		}
		p1+=size;
		p2+=size;
	}
	return 0;
done:
	if(!(flags&assignNoPrintfExit))switch(error){
	case assignInconsistentDescriptionsError:
		PrintfExit("%s: the two Descriptions of \"%s\" are inconsistent.\n"
			,here,d1->name);
	case assignUnequalDataError:
		PrintfExit("%s: the two instances of \"%s\" have significantly different values.\n"
			,here,ElementName(name,d1,i));
	}else return error;
}

void FreeAPtrDescription(Description *d,short flags)
{
	int i;
	
	if(IsPtr(d->type)){
		if(d->malloced)free(*(void **)d->ptr);
		*(void **)d->ptr=NULL;
		for(i=0;i<ASSIGN_DIMS;i++)d->dim[i]=0;
		d->sizedOnce=d->sized=d->malloced=0;
	}
}
void FreeAStringDescription(Description *d,short flags)
{
	static const char emptyString[]="";
	int i;
	
	if(IsString(d->type)){
		if(d->malloced && !d->dim[0])free(*(char **)d->ptr);
		*(const char **)d->ptr=emptyString;
		d->malloced=0;
	}
}
void FreeDescribedPtrVars(Description d[],short flags)
{
	for(;d->type!=0;d++)FreeAPtrDescription(d,flags);
}
void FreeADescribedVar(Description *d,short flags)
{
	FreeAPtrDescription(d,flags);
	FreeAStringDescription(d,flags);
}
void FreeDescribedVars(Description d[],short flags)
{
	for(;d->type!=0;d++)FreeADescribedVar(d,flags);
}
void KeepDescribedVars(Description d[],short flags)
{
	for(;d->type!=0;d++)d->malloced=0;
}
void KeepADescribedVar(Description *d,short flags)
{
	d->malloced=0;
}

long FindDescription(Description d[],void *ptr,short flags)
{
	long i;
	
	for(i=0;d[i].type!=0;i++)if(ptr==d[i].ptr)return i;
	if(PRINTF_EXIT)PrintfExit("FindDescription: couldn't find your variable.\n");
	else return assignCouldntFindDescription;
}
long FindDescribedDim(Description d[],void *ptr,int i,short flags)
{
	long n;
	
	n=FindDescription(d,ptr,flags);
	if(n<0)return n;
	if(i<0 || i>=ASSIGN_DIMS)return 0;
	return d[n].dim[i];
}

long InitializeDescribedVars(Description d[],short flags)
{
	long j,n=0;
	
	for(;d->type!=0;d++){
		j=InitializeADescribedVar(d,flags);
		if(j<0)return j;
		else n+=j;
	}
	return n;
}
long InitializeADescribedVar(Description *d,short flags)
{
	static const char here[]="InitializeADescribedVar";
	long error;
	
	error=CheckADescription(d,here,flags);
	if(error)return error;
	if(IsPtr(d->type) && !d->malloced) *(void **)d->ptr=NULL;
	if(IsPtr(d->type) || d->dim[0]){
		Description w;
		long i,j,n=0,elements;
		
		if(ElementPtr(d,0)==NULL)return n;
		w=*d;
		if(IsPtr(w.type))w.type+=charType-charPtrType;
		for(i=0;i<ASSIGN_DIMS;i++)w.dim[i]=0;
		w.sizedOnce=w.sized=1;
		if(IsString(w.type))w.malloced=0;
		w.firstElement=0;
		elements=Elements(d);
		for(i=0;i<elements;i++){
			w.ptr=ElementPtr(d,i);
			j=InitializeADescribedVar(&w,flags);
			if(j<0)return j;
			n+=j;
		}
		return n;
	}
	switch(d->type){
	case charType:
	case unsignedCharType:
		*(char *)d->ptr=0;
		break;
	case shortType:
	case unsignedShortType:
		*(short *)d->ptr=0;
		break;
	case longType:
	case unsignedLongType:
		*(long *)d->ptr=0;
		break;
	case floatType:
		*(float *)d->ptr=NAN;
		break;
	#if SHORT_DOUBLE_OK
		case shortDoubleType:
			*(short double *)d->ptr=NAN;
			break;
	#endif
	case doubleType:
		*(double *)d->ptr=NAN;
		break;
	case stringType:
		FreeAStringDescription(d,flags);
		break;
	default:
		/*  it shouldn't be possible to arrive here */
		return assignVariableError;
	}
	return 1;
}

long PrintAssignmentsToFile(const char *filename,Description d[],short flags)
{
	long n=0;
	FILE *stream;
	
	stream=fopen(filename,"a");
	if(stream==NULL)return assignFileError;
	n=PrintAssignments(stream,d,flags);
	fprintf(stream,NL);	/*  Add blank line to separate blocks */
	fclose(stream);
	return n;
}

long PrintAssignments(FILE *stream,Description d[],short flags)
{
	Description *vStart=d;
	long n=0,i;
	
	while(d->type!=0){
		i=PrintAnAssignment(stream,d,flags);
		if(i>0)fprintf(stream,NL);	/* new line */
		if(i<0)return i;
		n+=i;
		d++;
	}
	return n;
}

long PrintAnAssignmentOfElement(FILE *stream,Description *d,long index,short flags)
{
	Description w;
	long n,i;
	static const char here[]="PrintAnAssignmentOfElement";
	
	if(index<0 || index>=Elements(d))return assignSubscriptBoundsError;
	w=*d;
	for(i=0;i<ASSIGN_DIMS;i++)w.dim[i]=0;
	w.firstElement=0;
	w.comment=NULL;
	w.name=(char *)malloc(strlen(w.name)+16*Dims(d));
	if(w.name==NULL){
		if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
			,here,(long)strlen(w.name)+16*Dims(d));
		else return assignMemoryError;
	}
	ElementName(w.name,d,index);
	w.ptr=ElementPtr(d,index);
	if(IsPtr(w.type))w.type+=charType-charPtrType;
	if(IsString(w.type))w.malloced=0;
	/*  Turn off checking, 'cause subscripted name is illegal. */
	n=PrintAnAssignment(stream,&w,flags|assignAlreadyChecked);
	free(w.name);
	return n;
}

long PrintAnAssignment(FILE *stream,Description *d,short flags)
{
	short hexEncode;
	char *string,*s,name[32+16*ASSIGN_DIMS];
	static const char here[]="PrintAnAssignment";
	long index,n=0,error,row;
	static short lineLength;/* zeroed before printing array or string and after \n. */
	Description w;
	
	error=CheckADescription(d,here,flags);
	if(error<0)return error;
	if(ElementPtr(d,0)==NULL)return n;
	if(IsPtr(d->type) || d->dim[0]){
		if(Dims(d))row=d->dim[Dims(d)-1];
		else row=1;
		hexEncode=IsIntegral(d->type) && !(flags&assignNoHexInts)
			&&(row>2+typeSize[d->type]);
		hexEncode|=IsFloating(d->type)&&(flags&assignHexFloats);
		if(hexEncode){
			for(index=0;index<Elements(d);index+=row){
				lineLength=0;
				w=*d;
				if(IsString(w.type))w.malloced=0;
				if(row>1)w.dim[Dims(d)-1]=0;
				ElementName(name,&w,index/row);
				lineLength+=fprintf(stream,"%s=",name);
				string=BinaryToHex(row*typeSize[d->type]
					,ElementPtr(d,index),NULL);
				if(string==NULL){
					if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
						,here,2L*row*typeSize[d->type]);
					else return assignMemoryError;
				}
				PrintQuotedString(stream,&lineLength,string);
				free(string);
				lineLength+=fprintf(stream,";");
				if(index<Elements(d)-row)lineLength+=fprintf(stream,"\\" NL);
			}
			n=Elements(d);
		}else{
			long i,assignLength,oldLineLength,elements;
			
			lineLength=0;
			elements=Elements(d);
			for(i=0;i<elements;i++){
				oldLineLength=lineLength;
				n+=PrintAnAssignmentOfElement(stream,d,i,flags);
				assignLength=lineLength-oldLineLength;
				if(lineLength+assignLength+1>=80 && i<elements-1){
					fprintf(stream,"\\" NL);
					lineLength=0;
					lineLength+=fprintf(stream,"    ");
				}
			}
		}
		if(d->comment!=NULL && lineLength+strcspn(d->comment,NL)+6>=80){
			fprintf(stream,"\\" NL);
			lineLength=0;
			lineLength+=fprintf(stream,"    ");
		}
		goto comment;
	}
	assert(d->dim[0]==0);
	if(IsString(d->type))lineLength=0;
fflush(stream);
	lineLength+=fprintf(stream,"%s=",d->name);
fflush(stream);
	n=1;
	switch(d->type){
	case charType:
		lineLength+=fprintf(stream,"%ld;",(long)*(char *)d->ptr);
		break;
	case unsignedCharType:
		lineLength+=fprintf(stream,"%lu;",(unsigned long)*(unsigned char *)d->ptr);
		break;
	case shortType:
		lineLength+=fprintf(stream,"%ld;",(long)*(short *)d->ptr);
		break;
	case unsignedShortType:
		lineLength+=fprintf(stream,"%lu;",(unsigned long)*(unsigned short *)d->ptr);
		break;
	case longType:
		lineLength+=fprintf(stream,"%ld;",*(long *)d->ptr);
		break;
	case unsignedLongType:
		lineLength+=fprintf(stream,"%lu;",*(unsigned long *)d->ptr);
		break;
	case floatType:
	#if SHORT_DOUBLE_OK
	case shortDoubleType:
	#endif
	case doubleType:
		if(flags&assignHexFloats){
			string=BinaryToHex(typeSize[d->type],d->ptr,NULL);
			if(string==NULL){
				if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
					,here,2L*typeSize[d->type]);
				else return assignMemoryError;
			}
			lineLength+=fprintf(stream,"\"%s\";",string);
			free(string);
		}else switch(d->type){
			case floatType: 
				lineLength+=fprintf(stream,"%.*g;",FLT_DIG,*(float *)d->ptr);
				break;
			#if SHORT_DOUBLE_OK
				case shortDoubleType: 
					lineLength+=fprintf(stream,"%.*g;",DBL_DIG,*(short double *)d->ptr);
					break;
			#endif
			case doubleType: 
				lineLength+=fprintf(stream,"%.*g;",DBL_DIG,*(double *)d->ptr);
				break;
		}
		break;
	case stringType:
		PrintQuotedString(stream,&lineLength,*(const char **)d->ptr);
		lineLength+=fprintf(stream,";");
		break;
	default:
		/*  It shouldn't be possible to arrive here. */
		PrintfExit("PrintAnAssignment: oops. \"%s\" of type (%s) fell through a crack.\n"
			,d->name,typeName[d->type]);
		n=0;
	}
comment:
	if(d->comment!=NULL && !(flags&assignNoComment))
		lineLength+=fprintf(stream," /*�%s�*/",d->comment);
	return n;
}

int AssignmentLineWasBlank(void)
{
	extern int lineWasBlank;

	return lineWasBlank;
}

long ReadAssignmentFile(const char *filename,Description d[],short flags)
{
	long n=0,error;
	FILE *stream;
	static const char here[]="ReadAssignmentFile";
	
	stream=fopen(filename,"r");
	if(stream==NULL)return assignFileError;
	n=ReadAssignmentStream(stream,d,flags);
	fclose(stream);
	return n;
	
	#if 0
	if(!(flags&assignLocalCall)){
			/* First pass: scan for dimensions of unallocated Ptr variables.*/
			ReadAssignmentFile(filename,d,flags|assignScan|assignLocalCall);
			flags&=~assignScan;
			flags|=assignLocalCall;
			error=AllocateDescribedPtrVars(d,here,flags);
			if(error)return error;
			/* Second pass: read data again, including the newly allocated Ptr variables */
		}
		stream=fopen(filename,"r");
		if(stream==NULL)return assignFileError;
		do{
			n+=ReadAssignmentLine(stream,d,flags);
		}while(!feof(stream));
		fclose(stream);
		return n;
	#endif
}

long ReadAssignmentStream(FILE *stream,Description d[],short flags)
{
	long n=0,error;
	static const char here[]="ReadAssignmentStream";
	
	if(!(flags&assignLocalCall)){
		/* First pass: scan for dimensions of unallocated Ptr variables.*/
		long position=ftell(stream);
		ReadAssignmentStream(stream,d,flags|assignScan|assignLocalCall);
		flags&=~assignScan;
		flags|=assignLocalCall;
		fseek(stream,position,SEEK_SET);
		error=AllocateDescribedPtrVars(d,here,flags);
		if(error)return error;
		/* Second pass: read data again, including the newly allocated Ptr variables */
	}
	do{
		n+=ReadAssignmentLine(stream,d,flags);
	}while(!feof(stream));
	return n;
}

long ReadAssignmentBlock(FILE *stream,Description d[],short flags)
{
	long n=0,error;
	static const char here[]="ReadAssignmentBlock";
	
	if(!(flags&assignLocalCall)){
		/* First pass: scan for dimensions of unallocated Ptr variables.*/
		long position=ftell(stream);
		ReadAssignmentBlock(stream,d,flags|assignScan|assignLocalCall);
		flags&=~assignScan;
		flags|=assignLocalCall;
		fseek(stream,position,SEEK_SET);
		error=AllocateDescribedPtrVars(d,here,flags);
		if(error)return error;
		/* Second pass: read data again, including the newly allocated Ptr variables */
	}
	do{
		n+=ReadAssignmentLine(stream,d,flags);
	}while(!AssignmentLineWasBlank());
	return n;
}

static char *lineBuffer;	/* global because it would be messy to pass this address to
							all the routines below that need to call ParsingError */
long ReadAssignmentLine(FILE *stream,Description description[],short flags)
{
	register Description *d;
	Description *vMatch;
	long j,n=0,nAssign;
	int i;
	char *s,*sOld;
	long ftellOld;
	void *ptr;
	extern int lineWasBlank;
	int hexAllowed,subscriptDims,outOfBounds,unknown;
	long subscript[ASSIGN_DIMS],index,error;
	Description scratchDescription,*dScratch;
	double scratchDouble;
	long scratchLong;
	void *scratchPtr;
	static const char here[]="ReadAssignmentLine";
	
	if(!(flags&assignLocalCall)){
		/* First pass: scan for dimensions of unallocated Ptr variables.*/
		long position=ftell(stream);
		ReadAssignmentLine(stream,description,flags|assignScan|assignLocalCall);
		flags&=~assignScan;
		flags|=assignLocalCall;
		fseek(stream,position,SEEK_SET);
		error=AllocateDescribedPtrVars(description,here,flags);
		if(error)return error;
		/* Second pass: read data again, including the newly allocated Ptr variables */
	}
	error=CheckDescriptions(description,here,flags);
	if(error)return error;
	lineWasBlank=TRUE;
	lineBuffer=(char *)malloc(BUFFER_SIZE);
	if(lineBuffer==NULL){
		if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
			,here,(long)BUFFER_SIZE);
		else return assignMemoryError;
	}
	if(stream==NULL)return 0;
	dScratch=&scratchDescription;
	s=NULL;
	do{
		/* parse the name */
		if(s==NULL)
			s=NextTokenInThisOrNextLine(stream,lineBuffer,s,flags&~assignNeedMore);
		else s=NextTokenInLine(stream,lineBuffer,s,flags);
		if(s==NULL){
			free(lineBuffer);
			return n;
		}
		*dScratch=Describe(unknownType,&scratchPtr,GetName(&s,flags),NULL);
		if(dScratch->name==NULL){
			free(lineBuffer);
			return assignMemoryError;
		}
		dScratch->sizedOnce=dScratch->sized=dScratch->malloced=0;
		d=dScratch;
		unknown=1;
		for(vMatch=&description[0];vMatch->type!=0;vMatch++){
			if(streq(dScratch->name,vMatch->name)){
				d=vMatch;
				unknown=0;
				break;
			}
		}
		if(unknown && (flags&assignReportUnknown)){
			if(PRINTF_EXIT)ParsingError(s,"unknown variable");
			else {
				free(lineBuffer);
				return assignUnknownVariableError;
			}
		}
		hexAllowed=IsIntegral(d->type)&&!(flags&assignNoHexInts)
			|| IsFloating(d->type)&&(flags&assignHexFloats);
		s=NextToken(stream,lineBuffer,s,flags);
		subscriptDims=0;
		for(i=0;i<ASSIGN_DIMS;i++)subscript[i]=0;
		outOfBounds=0;
		if(IsPtr(d->type) && !d->malloced)outOfBounds=1;

		/* parse subscripts, e.g. [123][3] */
		for(i=0;i<Dims(d) || !d->sized;i++){
			/* parse a subscript, e.g. [123] */
			if(*s != '['){
				if(hexAllowed && subscriptDims==Dims(d)-1)break;
				if(!d->sized)break;
				if(PRINTF_EXIT)ParsingError(s,"expected \"[\"");
				else {
					free(lineBuffer);
					return assignSubscriptError;
				}
			}
			if(i==ASSIGN_DIMS){
				if(PRINTF_EXIT)ParsingError(s,"too many dimensions");
				else {
					free(lineBuffer);
					return assignSubscriptError;
				}
			}
			s++;
			s=NextToken(stream,lineBuffer,s,flags);
			sOld=s;
			ftellOld=ftell(stream);
			subscript[i]=strtolN(s,&s,0);
			subscriptDims=i+1;
			if(s==sOld && ftellOld==ftell(stream)){
				if(PRINTF_EXIT)ParsingError(s,"expected subscript");
				else {
					free(lineBuffer);
					return assignSubscriptError;
				}
			}
			j=subscript[i];
			if(i==Dims(d)-1)j-=d->firstElement;
			if(j<0 || d->sized && j>=d->dim[i]){
				if(flags&assignReportUnknown){
					if(PRINTF_EXIT)ParsingError(s,"subscript out of bounds: %s"
						,DescriptionNameDimensions(d));
					else {
						free(lineBuffer);
						return assignSubscriptBoundsError;
					}
				}
				outOfBounds=1;
			}
			s=NextToken(stream,lineBuffer,s,flags);
			if(*s != ']'){
				if(PRINTF_EXIT)ParsingError(s,"expected \"]\"");
				else {
					free(lineBuffer);
					return assignSubscriptError;
				}
			}
			s++;
			s=NextToken(stream,lineBuffer,s,flags);
		}

		/* parse the equal sign */
		if(*s != '='){
			if(PRINTF_EXIT)ParsingError(s,"expected \"=\"");
			else {
				free(lineBuffer);
				return assignEqualsError;
			}
		}
		s++;
		
		/* parse the value */
		s=NextToken(stream,lineBuffer,s,flags);
		if(d->sized && Dims(d)!=subscriptDims 
			&& !(*s=='"'  && Dims(d)-1==subscriptDims)){
			if(PRINTF_EXIT)
				ParsingError(s,"\"%s\": wrong number of subscripts",DescriptionNameDimensions(d));
			else {
				free(lineBuffer);
				return assignSubscriptError;
			}
		}
		if(d->type==unknownType){
			/*  choose type that can accept the data, so we can parse and discard it. */
			if(*s=='"')d->type=stringType;
			else if(streq2(s,"0x") || streq2(s,"0X") || *s=='\''
				|| !(flags&assignNoHexInts) && *s=='"')d->type=longType;
			else d->type=doubleType;
		}
		if(unknown || outOfBounds){
			/* use scratch scalar to receive the assignment */
			short type=d->type;
			if(IsPtr(type))type+=charType-charPtrType;
			if(IsString(type))
				*dScratch=Describe(type,&scratchPtr,d->name,NULL);
			else if(IsIntegral(type))
				*dScratch=Describe(type,&scratchLong,d->name,NULL);
			else if(IsFloating(type))
				*dScratch=Describe(type,&scratchDouble,d->name,NULL);
			d=dScratch;
			index=0;
			/* Note: d now inconsistent with subscript and subscriptDims */
		}else index=ElementIndex(d,subscript);
		ptr=ElementPtr(d,index);
		sOld=s;
		ftellOld=ftell(stream);
		nAssign=1;
		error=0;
		if(*s=='"' && !IsString(d->type) && hexAllowed){
			long bytes,elements;
			if(unknown || outOfBounds)
				nAssign=HexAssignment(stream,lineBuffer,&s,d,0,index,&bytes,flags);
			else nAssign=HexAssignment(stream,lineBuffer,&s,d,subscriptDims,index
				,&bytes,flags);
			if(!unknown){
				elements=bytes/typeSize[vMatch->type];
				if(elements>1){
					subscript[subscriptDims++]=elements-1;
					error=GetDimensions(vMatch,subscriptDims,subscript,flags);
					subscript[--subscriptDims]=0;
				}else error=GetDimensions(vMatch,subscriptDims,subscript,flags);
			}
		}else{
			if(!unknown)error=GetDimensions(vMatch,subscriptDims,subscript,flags);
			else error=0;
			if(!error)switch(d->type){
			default:
				assert(0 /* It shouldn't be possible to arrive here. */);
				nAssign=0;
				break;
			case charType:
			case charPtrType:
				*(char *)ptr=strtolN(s,&s,0);
				break;
			case unsignedCharType:
			case unsignedCharPtrType:
				*(unsigned char *)ptr=strtoulN(s,&s,0);
				break;
			case shortType:
			case shortPtrType:
				*(short *)ptr=strtolN(s,&s,0);
				break;
			case unsignedShortType:
			case unsignedShortPtrType:
				*(unsigned short *)ptr=strtoulN(s,&s,0);
				break;
			case longType:
			case longPtrType:
				*(long *)ptr=strtolN(s,&s,0);
				break;
			case unsignedLongType:
			case unsignedLongPtrType:
				*(unsigned long *)ptr=strtoulN(s,&s,0);
				break;
			case floatType:
			case floatPtrType:
				*(float *)ptr=strtodN(s,&s);
				break;
			#if SHORT_DOUBLE_OK
				case shortDoubleType:
				case shortDoublePtrType:
					*(short double *)ptr=strtodN(s,&s);
					break;
			#endif
			case doubleType:
			case doublePtrType:
				*(double *)ptr=strtodN(s,&s);
				break;
			case stringType:
				if(d->malloced && !d->dim[0])free(*(char **)ptr);
				*(char **)ptr=GetQuotedString(stream,lineBuffer,s,&s,flags);
				d->malloced=1;
				break;
			}
		}
		if(error==assignInconsistentDimensionsError){
			if(PRINTF_EXIT)ParsingError(s,"inconsistent dimensionality of \"%s\""
				,DescriptionNameDimensions(d));
			else {
				free(lineBuffer);
				return error;
			}
		}
		if(s==sOld && ftellOld==ftell(stream)){
			if(PRINTF_EXIT)ParsingError(s,"expected %s",typeName[d->type]);
			else {
				free(lineBuffer);
				return assignConstantError;
			}
		}

		/* parse the semicolon */
		s=NextToken(stream,lineBuffer,s,flags);
		if(*s!=';'){
			if(PRINTF_EXIT)ParsingError(s,"expected \";\"");
			else {
				free(lineBuffer);
				return assignSemicolonError;
			}
		}
		s++;
		
		if(ECHO_ASSIGNMENTS){
			if(unknown)printf("/*UNKNOWN: ");
			else if(outOfBounds)printf("/*OUT OF BOUNDS: ");
			if(!subscriptDims)PrintAnAssignment(stdout,d
				,(flags&~assignHexFloats)|assignNoComment);
			else PrintAnAssignmentOfElement(stdout,d,index
				,(flags&~assignHexFloats)|assignNoComment);
			if(outOfBounds || unknown)printf("*/ ");
		}
		dScratch->name=NULL;
		FreeADescribedVar(dScratch,flags);
		if(d!=dScratch)n+=nAssign;
	} while(1);
}

char *GetName(char **sPtr,short flags)
/*  Gets a variable's name.  */
{
	char *s,*sEnd,*name;
	long bytes=0,i;
	static const char here[]="ReadAssignmentLine/CheckADescription:GetName";
	
	s=*sPtr;
	if(ParseName(&s))if(PRINTF_EXIT)ParsingError(s,"expected name");
	sEnd=s;
	bytes=0;
	for(s=*sPtr;!isspace(*s) && s!=sEnd;s++)bytes++;
	name=(char *)malloc(bytes+1);
	if(name==NULL){
		if(PRINTF_EXIT)ParsingError(s,"%s: no room for %ld bytes.\n"
			,here,bytes+1);
		else return NULL;
	}
	for(i=0;i<bytes;i++){
		while(isspace((*sPtr)[i]))(*sPtr)++;
		name[i]=(*sPtr)[i];
	}
	name[bytes]=0;
	*sPtr=s;
	return name;
}

int ParseName(char **sPtr)
/*
Parse a variable's name. It may be just a C identifier: alphabetic (or _)
followed by any number of alphanumeric (or _). Or it may be made up of multiple
identifiers joined by "." or "->" infix operators, e.g. "LP->p". 
*/
{
	char *s;
	int error=0;
	
	s=*sPtr;
	while(*s!=0){
		for(;isspace(*s);s++);
		if(!(isalpha(*s)||*s=='_')){
			error=1;
			break;
		}
		for(;isalnum(*s)||*s=='_';s++);
		for(;isspace(*s);s++);
		if(*s=='.'){
			s++;
			continue;
		}
		if(streq2(s,"->")){
			s+=2;
			continue;
		}
		break;
	}
	*sPtr=s;
	return error;
}

long HexAssignment(FILE *stream,char *lineBuffer,char **sPtr,Description *d
	,int subscriptDims,long index,long *bytesPtr,short flags)
{
	char *string,*s;
	long bytes,excess,n;
	short error;
	void *ptr;
	
	if(!(flags&assignScan) 
		&& (subscriptDims<Dims(d)-1 || subscriptDims>Dims(d)))return assignSubscriptError;
	ptr=ElementPtr(d,index);
	/*
	Read in hex data.
	For scalar insist on exactly the right number of bytes.
	For array, if flags&assignReportUnknown then insist on exactly the right number
	of bytes; otherwise merely insist that the binary object size be a 
	multiple of the data element size.
	*/
	s=*sPtr;
	string=GetQuotedString(stream,lineBuffer,s,&s,flags);
	*bytesPtr=strlen(string)/2;
	bytes=typeSize[d->type];
	if(subscriptDims==Dims(d)-1)bytes*=d->dim[subscriptDims];
	excess=strlen(string)-2*bytes;
	if(subscriptDims==Dims(d)-1 && !(excess>0 && (flags&assignReportUnknown)))
		excess=strlen(string)%(2*typeSize[d->type]);
	if(excess!=0 && !(flags&assignScan)){
		free(string);
		if(PRINTF_EXIT){
			if(excess>0)ParsingError(s,"\"%s=\", %ld extra hex digits",d->name,excess);
			else ParsingError(s,"\"%s=\", %ld too few hex digits",d->name,-excess);
		}else return assignHexError;
	}
	string[strlen(string)-strlen(string)%(2*typeSize[d->type])]=0;
	if(strlen(string)>2*bytes)string[2*bytes]=0;
	if(ptr!=NULL){
		error=HexToBinary(string,ptr);
		if(error){
			if(PRINTF_EXIT)ParsingError(s,"\"%s=\", hex string contains non hex char:\n\"%s\"\n"
				,d->name,string);
			else {
				free(string);
				return assignHexError;
			}
		}
		n=strlen(string)/(2*typeSize[d->type]);	/*  number of assigments */
	}else n=0;
	free(string);
	*sPtr=s;
	return n;
}

char *GetQuotedString(FILE *stream,char *lineBuffer,char *s,char **sPtr,short flags)
/*
Returns a newly malloced string copied from the lineBuffer (starting at s),
supplemented if necessary, by reading in further lines from the stream. The
translation from input to output mimics what would be performed by a C compiler.
Each line of input must fit in the BUFFER_SIZE byte lineBuffer, but the
resulting string (concatenated across continuations and adjacent strings) can be
of any length, limited only by how much space can be obtained by malloc. Returns
NULL if an error occurs (e.g. can't malloc needed space).
*/
{
	char *newS,*oldString,*newString;
	size_t stringSize;
	long j;
	static const char here[]="GetQuotedString";
	
	/* parse the opening quote mark */
	if(*s!='"'){
		if(PRINTF_EXIT)ParsingError(s,"expected '\"'");
		else return NULL;
	}
	s++;
	stringSize=0;
	newS=newString=NULL;
	do{
		/* copy string, translating any backslash escapes */
		while(1){
			j=strcspn(s,"\\\"");
			if(stringSize==0 || strlen(newString)+j+2>stringSize){
				/*  allocate a string that's big enough */
				stringSize+=j;
				stringSize*=2;
				if(stringSize<BUFFER_SIZE)stringSize=BUFFER_SIZE;
				oldString=newString;
				newString=malloc(stringSize);
				if(newString==NULL){
					if(oldString!=NULL)free(oldString);
					if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
						,here,(long)stringSize);
					else return NULL;
				}
				if(oldString!=NULL){
					strcpy(newString,oldString);
					free(oldString);
				}
				newS+=newString-oldString;
			}
			strncpy(newS,s,j);
			s+=j;
			newS+=j;
			*newS=0;
			if(streq2(s,"\\" NL)){
				s+=2;
				continue;
			}
			if(*s=='\\'){
				*newS++=strtoc(s,&s);
				*newS=0;
			}else break;
		};
		if(strlen(s)==0){
			s=ReadLine(lineBuffer,BUFFER_SIZE,stream,flags|assignNeedMore);
			continue;
		}
		/* parse the closing quote mark */
		if(*s!='"'){
			if(PRINTF_EXIT)ParsingError(s,"expected '\"'");
			else{
				free(newString);
				return NULL;
			}
		}
		s++;
		
		/* look for another opening quote mark. Concatenate adjacent strings */
		s=NextToken(stream,lineBuffer,s,flags);
		if(*s!='"')break;
		s++;
	}while(1);
	/* shrink string allocation down to what we actually used */
	stringSize=1+strlen(newString);
	newString=realloc(newString,stringSize);
	if(newString==NULL){
		if(PRINTF_EXIT)PrintfExit("%s: no room for %ld bytes.\n"
			,here,(long)stringSize);
		else return NULL;
	}
	*sPtr=s;
	return newString;
}

void ParsingError(char *s,char *format,...)
{
	va_list args;
	long i;

	printf("\007\nReadAssignmentLine: ");
	va_start(args,format);
	vfprintf(stdout,format,args);
	va_end(args);
	if(s==NULL)PrintfExit(".\n");
	printf(". Quitting at ^.\n");
	printf("%s",lineBuffer);
	if(lineBuffer[strlen(lineBuffer)-1]!=NEWLINE)printf("...\n");
	s[0]='^';
	s[1]=0;
	s=lineBuffer;
	for(i=0;i<strlen(s)-1;i++)if(isgraph(s[i]))s[i]=' ';
	PrintfExit("%s\n",lineBuffer);
}

char *ReadLine(char *lineBuffer,size_t bufferSize,FILE *stream,short flags)
/*  The Boolean global lineWasBlank is initialized to true in ReadAssignmentLine(),  */
/*  falsified (if appropriate) here, and returned by AssignmentLineWasBlank(). */
{
	char *s;
	static const char whiteSpace[]=" \t\n\r\v\f";
	extern int lineWasBlank;
	
	s=fgets(lineBuffer,bufferSize,stream);
	if(s==NULL){
		if(flags&assignNeedMore){
			if(PRINTF_EXIT)ParsingError(s,"premature end of stream");
			else return NULL;
		}else return NULL;
	}
	if(strlen(s)!=strspn(s,whiteSpace))lineWasBlank=FALSE;
	if(strlen(s)>=bufferSize-1 && s[strlen(s)-1]!=NEWLINE){
		if(PRINTF_EXIT){
			s+=strlen(s);
			ParsingError(s,"line exceeds buffer size");
		}else return NULL;
	}
	if(ECHO_FILE)printf("%s",s);
	return s;
}

char *NextToken(FILE *stream,char *lineBuffer,char *s,short flags)
/*
Skip past white space and comments to the beginning of the next token,
reading as many lines as necessary.
*/
{
	do{
		s=NextTokenInThisOrNextLine(stream,lineBuffer,s,flags|assignNeedMore);
		if(s!=NULL || feof(stream))return s;
	}while(1);
}

char *NextTokenInThisOrNextLine(FILE *stream,char *lineBuffer,char *s,short flags)
/*
Skip past white space and comments to the beginning of the next token,
reading one line if necessary. 
*/
{
	s=NextTokenInLine(stream,lineBuffer,s,flags);
	if(s!=NULL)return s;
	s=ReadLine(lineBuffer,BUFFER_SIZE,stream,flags);
	s=NextTokenInLine(stream,lineBuffer,s,flags);
	return s;
}

char *NextTokenInLine(FILE *stream,char *lineBuffer,char *s,short flags)
/*
Skip past white space and comments to the beginning of the next token. NextTokenInLine
will read in new lines only if they are continuations of the current line (i.e.
it ends in "\") or if new lines are needed to find the end of an unfinished
comment. Input from stream is buffered one line at a time in lineBuffer, and s
points to the next unparsed character in that buffer.
*/
{
	static char whiteSpace[]=" \t\n\r\v\f";
	char *sTemp;

	do{
		/* skip white space and comments */
		do {
			if(s==NULL || strlen(s)==0)goto endOfLine;
			s+=strspn(s,whiteSpace);
			if(strlen(s)==0)goto endOfLine;
			if(streq2(s,"/*")){
				do{
					sTemp=s;
					s=strstr(s,"*/");
					if(s!=NULL)break;
					if(ECHO_COMMENTS)printf("%s",sTemp);
					s=ReadLine(lineBuffer,BUFFER_SIZE,stream,flags|assignNeedMore);
				} while(1);
				s+=2;
				if(ECHO_COMMENTS){
					s[-1]=0;
					printf("%s/",sTemp);
				}
				continue;
			}
			if(streq2(s,"//")){
				if(ECHO_COMMENTS){
					if(s[strlen(s)-1]==NEWLINE)s[strlen(s)-1]=0;
					printf("%s",s);
				}
				goto endOfLine;
			}
			if(streq2(s,"\\" NL)){
				if(ECHO_COMMENTS || ECHO_ASSIGNMENTS)printf(" \\\n");
				s=ReadLine(lineBuffer,BUFFER_SIZE,stream,flags&~assignNeedMore);
				continue;
			}
			break;
		} while(1);
		return s;	/* normal return */
	}while(1);
endOfLine:
	if((ECHO_ASSIGNMENTS || ECHO_COMMENTS) && s!=NULL)printf("\n");
	return NULL;
}

double strtodN(char *s,char **sPtr)
/*
Supplement standard C routine strtod() by also handling NAN and INF. Any specification
of the NAN's type (1 to 255) is ignored, whether in THINK C style, e.g. NANFF, or in
MPW C style, e.g. Nan[255].
*/
{
	if(streq2(s,"NAN")||streq2(s,"NaN")||streq2(s,"Nan")||streq2(s,"nan")){
		s+=3;
		s+=strspn(s,"0123456789abcdefABCDEF[]");
		*sPtr=s;
		return NAN;
	}
	if(strncmp(s,"INF",3)==0||strncmp(s,"Inf",3)==0||strncmp(s,"inf",3)==0){
		s+=3;
		*sPtr=s;
		return INF;
	}
	if(strncmp(s,"-INF",4)==0||strncmp(s,"-Inf",4)==0||strncmp(s,"-inf",4)==0){
		s+=4;
		*sPtr=s;
		return -INF;
	}
	return strtod(s,sPtr);
}

long strtolN(char *s,char **sPtr,short flags)
/* Supplement standard C routine strtol() by handling quoted char, e.g. 'e' */
{
	long j;
	
	if(*s!='\'') return strtol(s,sPtr,flags);
	s++;
	j=strtoc(s,&s);
	if(*s!='\''){
		if(PRINTF_EXIT)ParsingError(s,"expected \"'\"");
		else return 0;
	}
	*sPtr=s+1;
	return j;
}	

unsigned long strtoulN(char *s,char **sPtr,short flags)
/* Supplement standard C routine by handling quoted char, e.g. 'e' */
{
	unsigned long j;
	
	if(*s!='\'') return strtoul(s,sPtr,flags);
	s++;
	j=strtoc(s,&s);
	j &= 255L;		/* strip off any sign extension, since char was signed */
	if(*s!='\''){
		if(PRINTF_EXIT)ParsingError(s,"expected \"'\"");
		else return 0;
	}
	*sPtr=s+1;
	return j;
}	

char strtoc(char *s,char **sPtr)
/* Extract a character from a string, translating backslash escapes */
{
	char c;
	
	startAgain:
	if(*s!='\\'){
		if(*s==0){
			*sPtr=s;
			return -1;
		}
		*sPtr=s+1;
		return *s;
	}
	s++;
	switch(s++[0]){
	case 'n': c='\n'; break;
	case 'r': c='\r'; break;
	case 't': c='\t'; break;
	case 'b': c='\b'; break;
	case 'v': c='\v'; break;
	case 'f': c='\f'; break;
	case 'a': c='\a'; break;
	case 'x': c=strtol(s,&s,16); break;
	case '0': case '1':case '2': case '3':case '4': case '5':case '6': case '7':
		c=strtol(s-1,&s,8); break;
	case NEWLINE: s++; goto startAgain;
	default: c=s[-1]; break;
	}
	*sPtr=s;
	return c;
}

void PrintQuotedString(FILE *stream,short *lineLengthPtr,const char *s)
{
	*lineLengthPtr+=fprintf(stream,"\"");
	do{
		for(;*s!=0 && *lineLengthPtr+1+(isprint(*s)?1:(isspace(*s)?2:4))<=80;s++){
			if(isprint(*s) && *s!='"' && *s!='\\')
				*lineLengthPtr+=fprintf(stream,"%c",(PRINTF_INT)*s);
			else switch(*s){
			case '"': *lineLengthPtr+=fprintf(stream,"\\\""); break;
			case '\\': *lineLengthPtr+=fprintf(stream,"\\\\"); break;
			case '\n': *lineLengthPtr+=fprintf(stream,"\\n"); break;
			case '\t': *lineLengthPtr+=fprintf(stream,"\\t"); break;
			case '\b': *lineLengthPtr+=fprintf(stream,"\\b"); break;
			case '\r': *lineLengthPtr+=fprintf(stream,"\\r"); break;
			case '\v': *lineLengthPtr+=fprintf(stream,"\\v"); break;
			case '\f': *lineLengthPtr+=fprintf(stream,"\\f"); break;
			default: *lineLengthPtr+=fprintf(stream,"\\%03lo"
				,(unsigned long)*(unsigned char *)s); break;
			}
		}
		if(*s!=0){
			fprintf(stream,"\"" NL);
			*lineLengthPtr=0;
			*lineLengthPtr+=fprintf(stream,"    \"");
		}
	}while(*s!=0);
	*lineLengthPtr+=fprintf(stream,"\"");
}

#if 0
	/*  A longer, Macintosh-savvy, version of this routine, PrintfExit.c, */
	/*  is part of the VideoToolbox. */
	int PrintfExit(const char *format,...)
	{
		va_list args;
		int i;
	  
		va_start(args,format);
		i=vfprintf(stdout,format,args);
		va_end(args);
		exit(1);
	}
#endif
