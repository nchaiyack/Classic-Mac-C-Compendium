
all: xdiff xfix

xdiff : xdiff.h xdiff.c
	cc -DUNIX xdiff.c -o xdiff

xfix : xfixup.c xdiff.h
	cc -DUNIX xfixup.c -o xfix
