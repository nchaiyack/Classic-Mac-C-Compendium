c	==================================================================	
c
c	EGHI.F
c	HDF VSET Sample Program
c
c	Uses High-Level routines
c	Creates a vset of 1 vgroup and 3 vdatas into the file 'eghi.hdf'.
c
c	Jason NG NCSA 12-MAY-91
c
c	==================================================================

	program SAMPLE

	integer	Zarray(200), i, j, n
	real		rarray(200)
	integer	conval(3,200)
	integer	tagarray(10), refarray(10)

	external DFOPEN, DFCLOSE
	external VHFSD, VHFSDM, VHFMKGP

	integer	DFOPEN
	integer	VHFSD, VHFSDM, VHFMKGP

	integer f
	integer  vs1, vs2, vs3, vg

c	--- some defined constants. see "vg.h"

	integer INTTYPE				
	parameter (INTTYPE=2)
	integer REALTYPE				
	parameter (REALTYPE=3)
	integer	VDATATAG
	parameter (VDATATAG=1962)
	integer FULLACC	
	parameter (FULLACC=7)

c	------ generate data -------

	do 111 i=1,200
		rarray(i) = i *1.001 + 500
		zarray(i) = i
111	continue

	do 112 i=1,100
		do 114 j=1,3
		  conval(j,i) = i *j
114	continue
112	continue

c	------- open hdf file ------

	f = DFOPEN ('eghi.hdf', FULLACC, 0)

c	------- store 100 floats as one field in one vdata  ------
	vs1 = VHFSD (f, 'MP', rarray, 100, REALTYPE,  
	1					'melting-points-vdata', 'test')

c	------- store 120 integers as one field in one vdata  ------
	vs2 = VHFSD (f, 'AGE', zarray, 100, INTTYPE,
	1					'age-of-specimens-vdata', 'test')

c	------- store 100*3 values as one field (of order 3) in one vdata  ------
	vs3 = VHFSDM (f, 'PLIST', conval, 100, INTTYPE, 
	1					'connectivity triplets','test',3)

c ------ make a vgroup that has links to all the above vdatas ----  

	tagarray(1) = VDATATAG
	refarray(1) = vs1
	tagarray(2) = VDATATAG
	refarray(2) = vs2
	tagarray(3) = VDATATAG
	refarray(3) = vs3

	vg = VHFMKGP(f,tagarray,refarray,3,
	1							'vgroup with 3 vdatas (fortran)', 'test')

c	--- all done. close the file ---
	call DFCLOSE (f)

	end

