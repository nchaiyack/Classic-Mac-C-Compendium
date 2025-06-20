c  ==================================================================	
c	FORTRAN EXAMPLE
c
c	HDF VSET 2.1 
c	Jason Ng NCSA MAY 1991
c	
c	This program creates a Vset to store an array of pressure values,
c	and an array of color-triplets (red-green-blue). It demonstrate
c	the use of the Vset high-level write routines.

c	The pressure data is stored in one vdata (pid), and the color-triplets 
c	are stored in another vdata (cid). These ids are then stored together
c	in a vgroup, thereby logically grouping them as one vset.
c
c	Note that pressure is a simple variable. It is stored using the
c	function VHFSD. But color-triplet data is a compound variable, with
c	3 components (red,green,blue). The function VHFSDM is used instead
c	so that the order (the number of components, ie 3) can be specified.
c	
c	The HDF file that is created,"eh.hdf", can be looked at with the 
c	Vset utility "vshow"

c  ==================================================================	
	program SAMPLE

	real 		pbuf(100)
	integer 	cbuf(60,3)
	integer 	i,j, npres, ncolor
	integer f
	integer  pid, cid, vgid
	character*10  class
	integer tagnums(10), refnums(10), ntagref

c	--- routines and functions used
	external DFOPEN, DFCLOSE
	integer	DFOPEN
	external VHFSD, VHFSDM, VHFMKGP
	integer	VHFSD, VHFSDM, VHFMKGP

c	--- The parameters below are defined constants from "vg.h"
c	-- float and integer types
	integer 		T_INT, T_FLOAT 
	parameter 	(T_INT=2)
	parameter 	(T_FLOAT=3)

c	--- HDF tag for vgroup and vdata.
	integer 		VDATTAG, VGPTAG
	parameter 	(VGPTAG=1965)
	parameter 	(VDATTAG=1962)

c	--- full file access
	integer FULLACC
	parameter (FULLACC=7)

c	------ generate pressure data -------------------
		npres = 100
		do 111 i=1,npres
			pbuf(i) = 0.01 * i + 500
111	continue
c	------ generate color-triplet (rgb)  data -------
		ncolor = 60
		do 222 i=1,ncolor
			do 225 j=1,3
				cbuf(i,j) = i + j * 100
225		continue
222	continue

	class = 'example'

c	--- open the HDF file
	f = DFOPEN ('eh.hdf', FULLACC, 0)

c	--- store pressure values in a new vdata
	pid = VHFSD (f, 'PRES', pbuf, npres, T_FLOAT, 
	1				'pressure values', class)
	print *, 'Pressure vdata id is ', pid

c	--- store color-triplet values in a new vdata
c      	Note the argument 3 (for order=3 for a triplet)

	cid = VHFSDM (f, 'RGB', cbuf, ncolor, T_INT,
	1				'a set of rgb values', class, 3)
	print *, 'Color-triplet vdata id is ', cid

c 	--- create a new vgroup and then group the 2 vdatas into it

	tagnums(1) = VDATTAG
	tagnums(2) = VDATTAG
	refnums(1) = pid
	refnums(2) = cid
	ntagref    = 2

	vgid = VHFMKGP (f,tagnums ,refnums, ntagref,
	1					'vgroup with 2 vdatas', 
	1					class)
	print *, 'Vgroup id is ', vgid

c	--- close the HDF file
	call DFCLOSE (f)
	print *,'HDF VSet file eh.hdf created'

	end

