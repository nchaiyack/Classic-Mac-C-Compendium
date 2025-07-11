c  ==================================================================	
c
c	EG1.F
c	HDF VSET Sample Program
c
c	Multiple files access
c	creates 2 vsets in 2 files simultaneously
c
c	compile and link
c	compile to get the object file 'eg1.o'
c	link 'eg1.o' with the libraries 'libvg.a' and 'libdf.a'
c
c	==================================================================
c	 
c				  NCSA HDF Vset release 2.0
c					December, 1990
c	            Jason NG NCSA 15-DEC-90
c	
c	 NCSA HDF Vset release 2.0 source code and documentation are in the public
c	 domain.  Specifically, we give to the public domain all rights for future
c	 licensing of the source code, all resale rights, and all publishing rights.
c	 
c	 We ask, but do not require, that the following message be included in all
c	 derived works:
c	 
c	 Portions developed at the National Center for Supercomputing Applications at
c	 the University of Illinois at Urbana-Champaign.
c	 
c	 THE UNIVERSITY OF ILLINOIS GIVES NO WARRANTY, EXPRESSED OR IMPLIED, FOR THE
c	 SOFTWARE AND/OR DOCUMENTATION PROVIDED, INCLUDING, WITHOUT LIMITATION,
c	 WARRANTY OF MERCHANTABILITY AND WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE
c	 
c	==================================================================

	program SAMPLE

	integer	buf(100), sbuf(100), i,n
	integer f1,f2

	external DFOPEN, DFCLOSE
	external VSFATCH, VSFDTCH, VSFSFLD, VSFSNAM, VSFWRIT 
	external VSFFDEF
	external VFATCH, VFDTCH, VFSNAM, VFINSRT 
	integer	DFOPEN
	integer  VSFATCH, VSFSFLD, VSFWRIT, VSFFDEF
	integer  VFATCH, VFINSRT

	integer  vs1, vg1
	integer  vs2, vg2

c	some defined constants. see "vg.h"

	integer INTTYPE				
	parameter (INTTYPE=2)
	integer	FINTRLACE
	parameter (FINTRLACE=0)

	integer FULLACC	
	parameter (FULLACC=7)

c	------ generate data -------
	do 111 i=1,100
		buf(i) = i+500
		sbuf(i) = i+7000
111	continue


c	================================================
c	====== begin creating 2 vsets in 2 files =======
c	================================================

c	--- open 2 files, with file pointers f1 and f2

	f1 = DFOPEN ('for1.hdf', FULLACC, 0)
	f2 = DFOPEN ('for2.hdf', FULLACC, 0)

c	--- attach a new vgroup in each file, give each a name

	vg1 = VFATCH (f1, -1,'w')
	vg2 = VFATCH (f2, -1,'w')
	call VFSNAM(vg1, 'my_fortran_group_one')
	call VFSNAM(vg2, 'my_fortran_group_two')

c	--- attach a new vdata to file f1, write 70 integers,
c	--- link the vdata to vg1, then detach it

	vs1 = VSFATCH (f1, -1,'w')
	call VSFSNAM(vs1, 'dataset-of-70-integers')
	n = VSFSFLD (vs1, 'IY')
	n = VSFWRIT (vs1, buf, 70, FINTRLACE)
	n = VFINSRT (vg1, vs1)
	call VSFDTCH (vs1)

c	--- attach a new vdata to file f2, write 30 integers,
c	--- link the vdata to vg2, then detach it
	vs2 = VSFATCH (f2, -1,'w')
	n = VSFSFLD (vs2, 'IY')
	n = VSFWRIT (vs2, buf, 30, FINTRLACE)
	call VSFSNAM(vs2, 'dataset-of-thirty-integers')
	n = VFINSRT (vg2, vs2)
	call VSFDTCH (vs2)

c	--- detach each vgroup in each file
	call VFDTCH (vg1)
	call VFDTCH (vg2)

c	--- close each file
	call DFCLOSE (f1)
	call DFCLOSE (f2)

	print *,'done creating vsets in 2 files '

	end

