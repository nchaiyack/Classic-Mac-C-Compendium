# ==============================================================================
#
#	Infinity Windoid 2.6
#
#				makefile for MPW
#
# ------------------------------------------------------------------------------
#			  � 1991-94 Troy Gaul
#				Infinity Systems
#				All Rights Reserved
# ------------------------------------------------------------------------------
#   File:       Makefile
#   Target:     Infinity Windoid 2.6
#   Sources:    InfinityWindoid.c
#   			WindoidUtil.c
# ==============================================================================

Output		=	Infinity Windoid 2.6
PPCOut		=	Infinity Windoid/Fat

OutRsrc		=	InfinityWDEF.rsrc			# intermediate file for Fat build

# ==============================================================================
#	Types and IDs
# ------------------------------------------------------------------------------

Type		=	rsrc
Creator		=	RSED
ResType		=	WDEF
ResID		=	128

# ==============================================================================
#	Directories
# ------------------------------------------------------------------------------

ObjDir		=	:obj:

{ObjDir}	�	:				# directory dependency

# ==============================================================================
#	68K Options
# ------------------------------------------------------------------------------

COptions	= # -r				# warnings 									�
				-b				# PC-relative strings						�
				-mbg off 		# off, full									�
				-opt full		# off, (empty)=normal, full, size, speed	�
				-o "{ObjDir}"

# ==============================================================================
#	PowerPC Options/Libs
# ------------------------------------------------------------------------------

PPCCOptions	=	-w conformance -appleext on

PPCPefLibs	=	-l InterfaceLib.xcoff=InterfaceLib							�
				-l StdCLib.xcoff=StdCLib									�
				-l MathLib.xcoff=MathLib

PPCObjects	=	"{ObjDir}"InfinityWindoid.o									�
				"{ObjDir}"WindoidUtil.o

PPCLinkLibs	=	"{PPCLibraries}"InterfaceLib.xcoff 							�
				"{PPCLibraries}"StdCLib.xcoff 								�
				"{PPCLibraries}"StdCRuntime.o 								�
				"{PPCLibraries}"PPCCRuntime.o

# ==============================================================================
#	68K Compilation
# ------------------------------------------------------------------------------

"{ObjDir}"InfinityWindoid.c.o	�	Makefile 								�
									InfinityWindoid.c	WindoidDefines.h	�
									WindoidTypes.h		WindoidUtil.h
	C {COptions} InfinityWindoid.c

# ------------------------------------------------------------------------------

"{ObjDir}"WindoidUtil.c.o		�	Makefile 								�
									WindoidUtil.c		WindoidDefines.h	�
									WindoidTypes.h		WindoidUtil.h
	C {COptions} WindoidUtil.c

# ==============================================================================
#	PowerPC Compilation
# ------------------------------------------------------------------------------

"{ObjDir}"InfinityWindoid.o		�	Makefile 								�
									InfinityWindoid.c	WindoidDefines.h	�
									WindoidTypes.h		WindoidUtil.h
	PPCC {PPCCOptions} InfinityWindoid.c	-o "{ObjDir}"InfinityWindoid.o

# ------------------------------------------------------------------------------

"{ObjDir}"WindoidUtil.o			�	Makefile 								�
									WindoidUtil.c		WindoidDefines.h	�
									WindoidTypes.h		WindoidUtil.h
	PPCC {PPCCOptions} WindoidUtil.c		-o "{ObjDir}"WindoidUtil.o

# ==============================================================================
#	Object Files
# ------------------------------------------------------------------------------

Objects		=	"{ObjDir}"InfinityWindoid.c.o								�
				"{ObjDir}"WindoidUtil.c.o

# ==============================================================================
#	Linking
# ------------------------------------------------------------------------------

LinkOpts	=	-m MAIN �
				-sg "{Output}"

# ------------------------------------------------------------------------------
#	68K version

"{Output}"	��	Makefile {Objects}
	Link		-t {Type} -c {Creator} -rt {ResType}={ResID}				�
				{Objects} {LinkOpts}										�
				-o {Targ}

# ------------------------------------------------------------------------------
#	Fat version - 68K

"{OutRsrc}"	��	Makefile {Objects}
	Link		-t rsrc -c RSED -rt oCod={ResID}							�
				{Objects} {LinkOpts} 										�
				-o {Targ}

# ------------------------------------------------------------------------------
#	Fat version - PowerPC

"{OutRsrc}"	��	Makefile {PPCObjects}
	PPCLink -warn -main main 												�
			{PPCObjects} {PPCLinkLibs}										�
			-o "{ObjDir}"InfinityWDEF.xcoff
	makepef -b "{ObjDir}"InfinityWDEF.xcoff -o InfinityWDEF.pef {PPCPefLibs}
	echo "read �'pCod�' (128) �"InfinityWDEF.pef�";" | rez -a -o "{OutRsrc}"

# ------------------------------------------------------------------------------
#	Put the versions together in a fat binary WDEF

"{PPCOut}"	�	Makefile {OutRsrc}
	rez InfinityWDEF.r -a -o {Targ}
	setfile -t {Type} -c {Creator} {Targ}
#	Delete "{OutRsrc}"		# you might want to delete the intermediate file

# ------------------------------------------------------------------------------
