# Generated automatically from Makefile.in by configure.
# Makefile for gnuplot LaTeX tutorial
# To make the manual from scratch, we run latex two times

.NOEXPORT:
SHELL = /bin/sh

srcdir = .
VPATH = .

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datadir = $(prefix)/lib
mandir = $(prefix)/man

all: tutorial.dvi done

done:
	latex tutorial
	echo > done

# To touch it up after changes:
remake: tutorial.dvi

# Always runs latex, e.g., to get labels right
force:
	latex tutorial

tutorial.dvi: eg1.tex eg2.tex eg3.tex eg4.tex eg5.tex eg6.tex linepoin.tex \
	tutorial.tex header.tex
	latex tutorial
	rm -f done

.SUFFIXES: .tex .plt

.plt.tex:
	gnuplot $<

clean:
	rm -f *.log eg?.tex linepoin.tex

mostlyclean: clean

distclean: clean
	rm -f Makefile

realclean: distclean
