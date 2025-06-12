##################################
#	BuildProgram for DataScope DSIncludes
##################################
SDIR = ::DSSource:
VERS = Version 2.0.2
DATE = "`date -d -s`"
##################################
fview.h			Ä	vdevice.h
	SetFile -m . fview.h
#
macfview.h		Ä	fview.h
	SetFile -m . macfview.h
#
::newversion	Ä	{SDIR}fdlog.c		¶
					fview.h				¶
					Makefile
	echo "char *VERSION = ¶" {VERS}  {DATE}¶";" >DSversion.h
	Duplicate	-y	DSversion.h	::newversion
#
