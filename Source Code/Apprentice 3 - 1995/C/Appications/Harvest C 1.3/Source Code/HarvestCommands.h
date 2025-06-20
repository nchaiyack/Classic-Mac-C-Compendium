/*****

	Commands used for Harvest C
	
*****/




#define cmdNewProject	10000
#define cmdOpenProject	10001
#define cmdCloseProject	10002
#define cmdSetProjectInfo	10003
#define cmdClean	10004
#define cmdBuildApplication	10005
#define cmdDebugger		10006
#define cmdBringUpToDate 10007
#define cmdCheckLink	10008

#define cmdAddCFile		11000
#define cmdRemove		11001
#define cmdGetInfo		11002
#define cmdCheckSyntax	11003
#define cmdPreprocess	11004
#define cmdCompile		11005
#define cmdAddRsrcFile	11006
#define cmdAddLibrary	11007
#define cmdRun			11008

#define cmdOptions 12000
#define cmdWarnings 12001
#define cmdRegistration 13000

#define cmdOpenErrorLine 15000
#define cmdOpenSourceFile 15001

#define cmdToggleWarning 20000
#define cmdAllWarningsOn 20001
#define cmdAllWarningsOff 20002
#define cmdIndividuallySet 20003

#define cmdSwitchToAlpha 40000
#define cmdTclShell 40001

#define MENUproject 1000
#define MENUsources 1001
