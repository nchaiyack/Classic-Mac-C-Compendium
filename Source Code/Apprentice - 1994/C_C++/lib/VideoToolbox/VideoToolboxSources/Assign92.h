/*
Assign92.h
Including this header will make older programs that use Assign.c compatible with the
new version dated 8/12/93 of Assign.c, in which most of the routines were
renamed.
*/
#pragma once
#define Variable Description
#define FindVariable FindDescription
#define FindVariableDim FindDescribedDim
#define FreeAVariable FreeADescribedVar
#define FreePtrVariables FreeDescribedPtrVars
#define FreeVariables FreeDescribedVars
#define InitializeAVariable InitializeADescribedVar
#define InitializeVariables InitializeDescribedVars
#define KeepAVariable KeepADescribedVar
#define KeepVariables KeepDescribedVars
#define SetVariable Describe
#define SetVariableArray DescribeArray
#define SetVariableFirstLast DescribeFirstLast
#define UnequalVariablePair UnequalDescribedVarPair
#define UnequalVariables UnequalDescribedVars
Variable SetAVariable(short type,void *ptr,char *name,unsigned long dim
	,const char *comment);	/* old style */


