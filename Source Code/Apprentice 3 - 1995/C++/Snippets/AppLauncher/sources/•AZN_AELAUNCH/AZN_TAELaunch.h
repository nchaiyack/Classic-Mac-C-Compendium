#pragma once
/*
*	AZN_TAELaunch.h
*
*	Class declaration for 'TAELaunch' class
*	A class to locate and launch from the Finder
*	application(s) of the desired kind
*	HEIRARCHY:	BASE CLASS
*	( Based on Pascal code by Peter Lewis, [ peter@kagi.com ] )
*	( Converted to C++ by Gilles Dignard [ gdignard@hookup.net ] )
*	This version by Andrew Nemeth [ azn@nemeng.mpx.com.au ]
*
*	File Created:		21 Feb 95
*	File Ammended:		21 Feb;
*					4, 6�8, 13, 14 Mar 95.
*/


class TAELaunch
{
	public:
		enum EProcess {
					kenumError,
					kenumNone,
					kenumSimilar,
					kenumIdentical
					};

		TAELaunch(){};
		~TAELaunch(){};

		static  OSErr		findApplication( OSType, FSSpecPtr );

		static  Boolean	openItemInFinder( const FSSpec &, Boolean );
		static  Boolean	openDocInApp( OSType, const FSSpec &, Boolean );
		static  void		quitRemoteApp( OSType );
		
		static  Boolean	hideItemInFinder( OSType );

		static  Boolean	openControlPanel( OSType );

		static  EProcess	scanProcesses( const FSSpec &, OSType *, ProcessSerialNumber * );
		static  Boolean	docOpenable( const FSSpec & );
		static  Boolean	findProcess( OSType, OSType, ProcessSerialNumber *, FSSpecPtr );
		static  Boolean	grabCurrentFSSpec( FSSpecPtr );
		static  void		addFSSToAEList( AEDescList *, short, const FSSpec & );
};
