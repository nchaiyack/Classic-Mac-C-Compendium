#   File:       CALCPIN.make
#   Target:     CALCPIN (a.k.a. Calc Plug-in)
#   Sources:    CALCPIN.C XPINOP.C

#
# NOTE: Change InputName to the name of your plug-in's filename (minus the 
# .C suffix). Change OutputPlugin to whatever you want to call it out the
# other end... Change the ApplicationType to match your app's creator.
# PluginType is the filetype of the plugin. It should match the type field
# in XPINInit (if it is to be found).
#
InputName=CALCPIN
OutputPlugin='Calc Plug-in'
ApplicationType='rp&A'
PluginType=xPIN

#
# Don't change these unless you change the source code
#
ResourceType=pin!
ResourceID=128
MainRoutine=MAIN
SegmentName={InputName}

{InputName} ÄÄ  {InputName}.C.o XPINOP.c.o SAGlobals.o
	Link -w -t {PluginType} -c {ApplicationType} -rt {ResourceType}={ResourceID} ¶
		 -m {MainRoutine} -sg {SegmentName} ¶
		{InputName}.C.o XPINOP.C.o ¶
		"{CLibraries}"CSANELib.o ¶
		"{CLibraries}"Math.o ¶
		#"{CLibraries}"Complex.o ¶
		"{CLibraries}"StdClib.o ¶
		"{Libraries}"Runtime.o ¶
		"{Libraries}"Interface.o ¶
		SAGlobals.o ¶
		-o {InputName}

{InputName}.C.o Ä {InputName}.C XPIN.h
	 C -r -b -warnings off {InputName}.c

XPINOP.C.o Ä XPINOP.c XPIN.h
	C -r -b -warnings off XPINOP.c

{InputName}		ÄÄ {InputName}.r
		Rez -rd -o {InputName} {InputName}.r -append
		rename -y {InputName} {OutputPlugin}
