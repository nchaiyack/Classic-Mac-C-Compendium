CC=/home/ponder/grad/oleg/commands/c++
CCL=/home/ponder/grad/oleg/commands/c++l
TOUCH=/usr/att/bin/touch
.SUFFIXES: .cc
LIBMOD=image.cc image_rect.cc read_xwd.cc write_xwd.cc \
	read_pgm.cc write_pgm.cc read_tiff.cc write_tiff.cc morph_filter.cc
LIBRARY=libimage.a


#	Rules

%.o :: %.cc
	$(CC) $*.cc

% : %.o $(LIBRARY)
	$(CCL) $< $(LIBRARY) -o $@
	./$@

% :: %.cc
	$(CC) $*.cc
	$(CCL) $*.o $(LIBRARY) -o $@
	./$@

.cc.o:
	$(CC) $*.cc

.o:	$*.o libimage.a
	$(CCL) $*.o libimage.a -o $*
	./$*

.cc: 	$*.cc libimage.a
	$(CC) $*.cc
	$(CCL) $*.o libimage.a -o $*
	./$*

# Primary goal

a.out: $(MODULES)
	$(CCL) $(MODULES)
	a.out

# Library

lib:	libimage.a
.PHONY: lib

libimage.a::	image.h
# 					Make the library very obsolete
	$(TOUCH) 0101010180 $@

libimage.a::	$(LIBMOD)
# 			Compile the source files that have been changed 
	$(CC) $?
	listobj=`echo $? | sed s/.cc/.o/g` ; \
	ar rv libimage.a $$listobj &&	\
	rm $$listobj
	ar d libimage.a __.SYMDEF; ranlib libimage.a

# Verification routines
vimage:	vimage.o libimage.a
	$(CCL) vimage.o libimage.a -o vimage
	vimage

morph_filter:	morph_filter.o libimage.a
	$(CCL) morph_filter.o libimage.a -o morph_filter
	./morph_filter

# Specific dependent goals


# Dependence rules
