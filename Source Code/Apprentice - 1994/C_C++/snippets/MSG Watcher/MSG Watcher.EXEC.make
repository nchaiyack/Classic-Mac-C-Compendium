
# Makefile for MSG Watcher EXEC

MWS			= ::Support:	# Directory containing MWS support files
XECID		= 200			# Change this number for your own purposes
XECName		= "MSG Watcher"	# Change this name to match the name of your code
MWSLibName	= "{MWS}MWS Library"
COptions	= -i {MWS} -b {qDebug}

XECSupport	= {MWS}EXEC.a.o {MWS}MWS_EXEC.p.o {MWSLibName}

{XECName}.EXEC		Ä	{XECSupport} {XECName}.c.o
	Link -rt MWSX={XECID} ¶
		-sg {XECName} ¶
		{XECSupport} ¶
		{XECName}.c.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		"{PLibraries}"Paslib.o ¶
		-o {Targ} -w -t 'rsrc' -c 'RSED'
# Uncomment the next two lines for automatic installation
	Make -f {XECName}.EXEC.make Install > Install.out && ¶
 		( Install.out ; delete -i Install.out)

{XECName}.c.o	Ä	{MWS}MWS_EXEC.p

# The following commands are used to install the EXEC.

Install	Ä
	set exit 0
	unset file
	Set file "`GetFileName -t APPL -t MWS3 -b 'Install' ¶
				-m 'Select file to install {XECName} into'`"
	if {file} != ""
		Echo "include ¶"{XECName}.EXEC¶";" |
			Rez  -a -o {file}
	end
	unset file
	Set Exit 1

# End of Makefile
