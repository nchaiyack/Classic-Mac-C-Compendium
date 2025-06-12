/*______________________________________________________________________

	rez.h - Resource Constants.
_____________________________________________________________________*/

#ifndef __rez__
#define __rez__

/*_____________________________________________________________________

	These definitions must exactly match the resource definitions in 
	file rez.r.
_____________________________________________________________________*/
	
#define editWindID		128		/* rsrc id of edit window template */
#define sBarID				128		/* rsrc id of scroll bar control */
#define showInfoID		129		/* rsrc id of show field info checkbox */
#define revertID			130		/* rsrc id of revert field button */
#define revertAllID		131		/* rsrc id of revert all fields button */

#define queryWindID		129		/* rsrc id of query window template */
#define queryDoitID		132		/* rsrc id of doit button */
#define querySBarID		133		/* rsrc id of scroll bar control */

#define helpWindID		130		/* rsrc id of help window template */
#define helpSBarID		134		/* rsrc id of scroll bar control */

#define mBarID				128		/* rsrc id of menu bar */
#define numMenus6			6			/* number of menus in menu bar, System 6 -
												subtract 1 for System 7! */
	
#define appleID			1			/* menu id of apple menu */
#define aboutCmd			1			/* about */

#define fileID				2			/* menu id of file menu */
#define newQueryCmd		1			/* new query window */
#define newPhCmd			2			/* new ph record */
#define openPhCmd			3			/* open ph record */
#define closeCmd			4			/* close */
#define savePhCmd			5			/* save ph record */
#define loginCmd			7			/* login  */
#define logoutCmd			8			/* logout */
#define paswdCmd			10			/* change password */
#define deletePhCmd		11			/* delete ph record */
#define updateCmd			12			/* update server site list */
#define changeDefCmd		13			/* change default server */
#define quitCmd			15			/* quit */

#define editID				3			/* menu id of edit menu */
#define undoCmd			1			/* undo */
#define cutCmd				3			/* cut */
#define copyCmd			4			/* copy */
#define pasteCmd			5			/* paste */
#define clearCmd			6			/* clear */
#define selectAllCmd		8			/* select all */

#define proxyID			4			/* menu id of proxy menu */

#define windowsID			5			/* menu id of windows menu */

#define helpID				6			/* menu id of help menu */

#define popupID			7			/* menu id of server popup menu */

#define fieldNames		128		/* rsrc id of field names STR# rsrc */
#define nameFieldName	1			/* "name" = name of name field */
#define typeFieldName	2			/* "type" = name of type field */
#define heroFieldName	3			/* "hero" = name of hero field */
#define aliasFieldName	4			/* "alias" = name of alias field */
#define proxyFieldName	5			/* "proxy" = proxy field name */
#define pswdFieldName	6			/* "password" = name of password field */
#define emailFieldName	7			/* "email" = name of email field */

#define serverCmds		129		/* rsrc id of server commands */
#define phFields			1			/* fields */
#define phLogin			2			/* login ^0 */
#define phAnswer			3			/* answer */
#define phQueryAlias		4			/* query ^0 return alias */
#define phAliasQuery		5			/* query alias=^0 return all */
#define phNameQuery		6			/* query ^0 return all */
#define phMake				7			/* change alias=^0 make ^1= */
#define phChPswdCmd		8			/* change alias=^0 make password= */
#define phProxyQuery		9			/* query proxy=^0 return alias */
#define phNewRecordCmd	10			/* add alias=^0 */
#define phDeleteCmd		11			/* delete alias=^0 */
#define phStatus			12			/* status */
#define phQuit				13			/* quit */
#define phQuery			14			/* query */
#define phHelp				15			/* help macph */
#define phHelpNative		16			/* help native */
#define phSiteQuery		17			/* query ns-servers type=serverlist return 
												text */
#define phIdQuery			18			/* query id=^0 return all */

#define fieldAttrs		130		/* rsrc id of field attributes */
#define changeAttr		1			/* "Change" field attribute */
#define encryptAttr		2			/* "Encrypt" field attribute */

#define macTCPErrors		131		/* rsrc id of MacTCP error messages */
#define msgBadCnfg		1			/* bad MacTCP config */
#define msgBadUnit		2			/* MacTCP not installed or config error */
#define msgDestDead		3			/* domain name server dead */
#define msgNoSuchName	4			/* no such domain name */
#define msgNoNameServer	5			/* no server for domain name */
#define msgOpenFailed	6			/* server not responding */
#define msgCTerminated	7			/* connection terminated */
#define msgNoAnswer		8			/* no name servers responding */
#define msgBadAddr		9			/* MacTCP couldn't get ip address */
#define msgCacheFault	10			/* MacTCP domain name server timeout */
#define msgUnexpected	11			/* unexpected error */
#define msgDnrErr			12			/* domain name server returned an error */
#define msgDnrSynErr		13			/* syntax error in domain name */
#define msgNoServers		14			/* no MacTCP domain name servers defined */

#define servErrors		132		/* rsrc id of server error messages */
#define msgIllegal		1			/* illegal value */
#define msgNoName			2			/* you must specify a name */
#define msgCodeErr		3			/* unexpected server response code */
#define msgBadLogin		4			/* alias or password bad */
#define msgNameNotUniq	5			/* login name not unique */
#define msgReadOnly		6			/* database is read only */
#define msgLockTimeout	7			/* could not obtain lock within É */
#define msgNoSuchRecord	8			/* no such record on server */
#define msgDbOff			9			/* database shut off */
#define msgFieldTooBig	10			/* This field has a maximum size of É */
#define msgNoServer		11			/* You must specify a server name */
#define msgNoAliasName	12			/* You must specify an alias or name */
#define msgNoPswd			13			/* You must specify a password */
#define msgNoAlias		14			/* You must specify an alias */
#define msgNoType			15			/* you must specify a type */
#define msgPswdNotEqual	16			/* The two passwords are not identical */
#define msgBadFieldChar	17			/* Fields may contain only standard É */
#define msgNotAuth		18			/* not authorized to make changes */
#define msgNoProxy		19			/* no proxy menu error message */
#define msgSyntaxError	20			/* unexpected syntax error */
#define msgNoHelp			21			/* help not available */
#define msgBadVal			22			/* bad field value */
#define msgBadPswd		23			/* bad password */
#define msgLoginNoAlias	24			/* login to record with no alias */
#define msgOpenNoAlias	25			/* open record with no alias */
#define msgSaveNoAlias	26			/* save record with no alias */
#define msgNoAliasIdName 27		/* You must specify an alias, id or name */

#define stringsID			133		/* rsrc id of misc strings */
#define fieldChanged		1			/* ¥ = Field Changed */
#define maxSizeStr		2			/* Max Size = */
#define pswdMsg			3			/* Message displayed in password field */
#define encryptMsg		4			/* Message displayed in encrypted fields */
#define cantChange		5			/* Â = CanÕt Change Field */
#define waitChangePswd	6			/* wait message while changing password */
#define waitGetRecord	7			/* wait message while getting record */
#define waitPutRecord	8			/* wait message while putting record */
#define waitNewRecord	9			/* wait message while creating record */
#define waitDelRecord	10			/* wait message while deleting record */
#define waitQuery			11			/* wait message while querying */
#define waitHelp			12			/* wait message while getting help text */
#define waitHelpList		13			/* wait message while getting help list */
#define waitSiteList		14			/* wait message while getting site list */
#define statusMsg			15			/* about box status message template */
#define heroStatusMsg	16			/* You are a hero! */
#define notLoggedInMsg	17			/* You are not logged in to a server. */
#define wTitleLogin		18			/* Login window title template */
#define wTitleHero		19			/* Login hero window title template */
#define wTitleRegular	20			/* Regular edit window title template */
#define msgSaveChanges	21			/* Save changes for alias "^0" before ^1? */
#define msgQuitting		22			/* quitting */
#define msgClosing		23			/* closing window */
#define whyCmd				24			/* "Why?É" command for proxy and help menus */
#define msgDelete			25			/* really delete? */
#define msgCantDelete	26			/* can't delete this record */
#define queryWindTtl		27			/* query window title template */
#define siteTag			28			/* "site" */
#define serverTag			29			/* "server" */
#define domainTag			30			/* "domain" */
#define prefsFileName	31			/* "Ph Prefs" */
#define memoryLow			32			/* memory low warning */
#define truncMsg			33			/* truncation message */
#define siteLabelMsg1	34			/* default site dialog label 1 */
#define siteLabelMsg2	35			/* default site dialog label 2 */
#define msgNeedMonaco	36			/* Ph requires the Monaco font */
#define msgNeed604		37			/* Ph requires System 6.0.4 or later */
#define helpLabelMsg1	38			/* help site dialog label 1 */
#define helpLabelMsg2	39			/* help site dialog label 2 */

#define aboutStrID		134		/* rsrc id of about box text */

#define hbStringsId				1000		/* rsrc id of Balloon Help strings */												
#define hbQueryDomainName		80			/* query window server domain name field */
#define hbQueryQuery				81			/* query window query field */
#define hbQueryResult			82			/* query window result field */
#define hbQueryPopup				83			/* query window server popup menu */
#define hbQueryDoitEnabled		84			/* query window doit button, enabled */
#define hbQueryDoitDisabled	85			/* query window doit button, disabled */
#define hbScrollEnabled			86			/* scroll bar, enabled */
#define hbScrollDisabled		87			/* scroll bar, disabled */
#define hbGrow						88			/* grow box */
#define hbHelpText				89			/* help window text field */
#define hbEditFields				90			/* edit window field display */
#define hbEditInfoUnChk			91			/* show field info, unchecked */
#define hbEditInfoChk			92			/* show field info, checked */
#define hbEditRvtEnabled		93			/* revert button, enabled */
#define hbEditRvtDisabled		94			/* revert button, disabled */
#define hbEditRvtAllEnabled	95			/* revert all button, enabled */
#define hbEditRvtAllDisabled	96			/* revert all button, disabled */

#define defServerID		128		/* rsrc id of default server string */
#define portID				129		/* rsrc id of server port number string */

#define aboutID			128		/* rsrc id of about box dialog */

#define loginDlogID		129		/* rsrc id of login dialog */
#define loginLogin		1			/* item number of login button */
#define loginCancel		2			/* item number of cancel button */
#define loginUserField	4			/* item number of alias or name field */
#define loginPswdField	6			/* item number of password field */
#define loginServField	8			/* item number of server field */
#define loginPopup		9			/* item number of popup user item */

#define pswdDlogID		130		/* rsrc id of change password dialog */
#define pswdOK				1			/* item number of ok button */
#define pswdCancel		2			/* item number of cancel button */
#define pswdField1		4			/* item number of first password field */
#define pswdField2		6			/* item number of second password field */

#define tranID				131		/* rsrc id of server transaction dialog */
#define tranAbortBtn		1			/* item number of abort button */
#define tranMsg			2			/* item number of wait message */

#define openDlogID		132		/* rsrc id of open record dialog */
#define openOK				1			/* item number of ok button */
#define openCancel		2			/* item number of cancel button */
#define openUserField	4			/* item number of alias or name field */

#define errorMsgID		133		/* rsrc id of error message dialog */

#define saveChangesID	134		/* rsrc id of save changes dialog */
#define saveSave			1			/* item number of save button */
#define saveCancel		2			/* item number of cancel button */
#define saveDont			3			/* item number of don't save button */

#define newDlogID			135		/* rsrc id of new ph record dialog */
#define newOK				1			/* item number of ok button */
#define newCancel			2			/* item number of cancel button */
#define newAliasField	4			/* item number of alias field */
#define newNameField		6			/* item number of name field */
#define newTypeField		8			/* item number of type field */
#define newPswdField1	10			/* item number of first password field */
#define newPswdField2	12			/* item number of second password field */

#define deleteID			136		/* rsrc id of delete dialog */
#define deleteDelete		1			/* item number of delete button */
#define deleteCancel		2			/* item number of cancel button */

#define siteDlogID		137		/* rsrc id of site dialog */
#define siteOK				1			/* item number of ok button */
#define siteCancel		2			/* item number of cancel button */
#define siteServField	3			/* item number of server field */
#define siteLabel			4			/* item number of label static text */
#define sitePopup			5			/* item number of popup user item */

#define noMemID			138		/* rsrc id of no memory alert */

#define beachCursorID	128		/* rsrc id of first beachball cursor */
#define numBB				4			/* number of beachball cursors */

#define phSmallIconID	128		/* rsrc id of small icon */

#endif