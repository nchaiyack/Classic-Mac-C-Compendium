// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UGroupListCmds.h

#define __UGROUPLISTCMDS__

#ifndef __UGROUPVIEWCMDS__
#include "UGroupViewCmds.h"
#endif

#ifndef __UPERIODIC__
#include "UPeriodic.h"
#endif

class TGroupListDoc;
class TGroupList;
class TGroupTree;
class PNntp;

class TUnsubscribeGroupTracker : public TGroupTracker
{	
	public:
			
		TUnsubscribeGroupTracker();
		virtual pascal void Initialize();
		virtual pascal void IUnsubscribeGroupTracker(
												TGroupListView *groupListView,
												const VPoint &localMouse,
												Boolean makingCopy);
		virtual pascal void Free();
	protected:
		virtual void MakeGrayPict();
		virtual void CreateListOfDraggedGroups();
		virtual void CheckForExistingGroups(TGroupListView *hitGroupListView, Boolean makingCopy);
		virtual void GetBeforeGroupName(TGroupListView *groupListView, short beforeRow, CStr255 &name);

	private:
		TGroupListView *fGroupListView;
};


class TListTypeName : public TGroupViewTypeNameCommand
{
	public:
		TListTypeName();
		virtual pascal void Initialize();
		void IListTypeName(TGroupListView *glv, TToolboxEvent *event);
		virtual pascal void Free();
	protected:
		TGroupListView *fGroupListView;
		TGroupList *fGroupList;

		virtual void SetUp();
		virtual void SetDown();
		virtual void GetLineText(ArrayIndex line, CStr255 &text);
		virtual void DoOneLine(ArrayIndex line);
};

class TListTabKeyCommand : public TGroupViewTabKeyCommand
{
	public:
		TListTabKeyCommand();
		virtual pascal void Initialize();
		void IListTabKeyCommand(TGroupListView *glv, Boolean forward);
		virtual pascal void Free();
	protected:
		TGroupListView *fGroupListView;
		TGroupList *fGroupList;

		virtual void GetLineText(ArrayIndex line, CStr255 &text);
};

class TPeriodicCheckNewArticles : public TPeriodicAction
{
	public:
		TPeriodicCheckNewArticles();
		pascal void Initialize();
		void IPeriodicCheckNewArticles(TGroupListDoc *doc);
		pascal void Free();

	protected:
		void DoPeriodic();
		const char *GetDebugDescription();

	private:
		TGroupListDoc *fGroupListDoc;
		TGroupList *fGroupList;
		PNntp *fNntp;

		Boolean GetCurrentLastID(const CStr255 &dotName, long &lastID);
		void DoTheCheck();
		Boolean UpdateGroup(const CStr255 &dotName);
		void UpdateGroupDatabases();
};
