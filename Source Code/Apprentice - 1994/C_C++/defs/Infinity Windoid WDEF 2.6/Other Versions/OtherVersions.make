#========================================================================
#
#	Infinity Windoid -- Other Versions
#
#------------------------------------------------------------------------
#			  © 1991-94 Troy Gaul
#				Infinity Systems
#				All Rights Reserved
#========================================================================

Output		=	Infinity Windoid
ResName		=	Infinity Windoid 2.6

#------------------------------------------------------------------------

Type		=	rsrc
Creator		=	RSED
ResType		=	WDEF
ResID		=	128

#========================================================================
#	Directories and Options
#------------------------------------------------------------------------

ObjDir		=	::obj:

#------------------------------------------------------------------------

COptions	=	-warnings off											∂
				-b				# PC-relative strings					∂
				-mbg off 		# off, full								∂
				-o "{ObjDir}"											∂
				-opt full		# off, (empty), full, size, speed 		∂
				-i ::
				
#========================================================================
#	Object Files
#------------------------------------------------------------------------

OBJECTS		=	"{ObjDir}"InfinityWindoid.c.o							∂
				"{ObjDir}"WindoidUtil.c.o

#========================================================================
#	Various Versions
#------------------------------------------------------------------------

OtherVersions ƒƒ OtherVersions.make
	Duplicate -y ::WindoidDefines.h ::WindoidDefines.h-bkup
	∂
	∂
	∂
	set Extension "close only"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "MacApp vers"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "thin tbar"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "THINK vers"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "title/grow"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "with grow"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	set Extension "small grow"
	∂
	Duplicate -y "WindoidDefines.h—{Extension}" ::WindoidDefines.h
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o "{Output}—{Extension}" {OBJECTS}
	∂
	∂
	∂
	∂
	Duplicate -y ::WindoidDefines.h-bkup ::WindoidDefines.h
	∂
	C {COptions} ::InfinityWindoid.c
	C {COptions} ::WindoidUtil.c
	Link	-t {Type} -c {Creator} -rt {ResType}={ResID} -m MAIN 		∂
			-sg "{ResName}" -o ::"{ResName}" {OBJECTS}
