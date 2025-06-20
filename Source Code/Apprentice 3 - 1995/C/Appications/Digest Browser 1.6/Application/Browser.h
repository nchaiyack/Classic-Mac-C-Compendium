/* Browser.h */

#pragma once

#include <stdlib.h>
#include <stdio.h>

// Macros to make life easy
#define Allocate(aType) (aType *) NewPtr(sizeof(aType))
#define Deallocate(Var) if (Var) { DisposPtr(Var); Var = NULL;}

#define MAX_STRING 200

typedef struct BrowserItem {
	struct BrowserItem *next;
	void	*owner;		//really a struct BrowserDirPtr owner;
	FILE	*fp;

	char	 date[MAX_STRING];
	char	 from[MAX_STRING];
	char	 subject[MAX_STRING];
	char	 composite[MAX_STRING];	// JRB support composite index
	long	 startAt;
	long	 endAt;
	Boolean  marked;
} BrowserItem, *BrowserItemPtr;


typedef struct BrowserDir {
	char	fname[64];		// file name
	short	vRefNum;		// volume ref.
	FILE	*fp;			// file pointer
	long	numArticles;	// count of items
	long	markArticles;	// count of marked items

	BrowserItemPtr topItem;
} BrowserDir, *BrowserDirPtr;

Boolean equalstr(register char *s, register char *t, int n);
Boolean BuildBrowserIndex(BrowserDir	*dir);

void	brInitDir(BrowserDirPtr dir);
long	brItemCount(BrowserDirPtr dir);
long	brMarkCount(BrowserDirPtr dir);

void	brInitItem(BrowserItemPtr item);

void	brSetOwner(BrowserItemPtr item, BrowserDirPtr dir);
BrowserDirPtr	brGetOwner(BrowserItemPtr item);

void	brSetFP(BrowserItemPtr item, FILE *file);
FILE	*brGetFP(BrowserItemPtr item);

void	brSetStart(BrowserItemPtr item, long start);
long	brGetStart(BrowserItemPtr item);

void	brSetEnd(BrowserItemPtr item, long end);
long	brGetEnd(BrowserItemPtr item);

void	brToggleMark(BrowserItemPtr item);
void	brSetMark(BrowserItemPtr item, Boolean mark);
Boolean	brGetMark(BrowserItemPtr item);

void	brSetNext(BrowserItemPtr item, BrowserItemPtr nxt);
BrowserItemPtr	brGetNext(BrowserItemPtr item);

