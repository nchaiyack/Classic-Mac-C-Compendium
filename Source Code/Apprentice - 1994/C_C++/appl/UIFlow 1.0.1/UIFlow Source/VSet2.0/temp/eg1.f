c  ==================================================================	
c
c	EG1.F
c	HDF VSET Sample Program
c
c	Creates a vset of 1 vgroup and 2 vdatas into the file 'eg1.hdf'.
c	The companion program VSETR.F reads from this file.
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
	integer f

	external DFOPEN, DFCLOSE
	external VSFATCH, VSFDTCH, VSFSFLD, VSFSNAM, VSFWRIT 
	external VSFFDEF
	external VFATCH, VFDTCH, VFSNAM, VFINSRT 
	integer	DFOPEN
	integer  VSFATCH, VSFSFLD, VSFWRIT, VSFFDEF
	integer  VFATCH, VFINSRT

	integer  vs, vg

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


c	------ write to vset -------

c	--- (A) ----
c	--- open HDF file, attach to new vgroup
c	--- name the vgroup 'my_fortran_vgroup'

c	--- (B) ----
c	--- attach to new (1st) vdata, name it 'the_fortune_500_vdata'
c	--- write out 20 integers from buf as the predefined field 'IY'
c	--- tell VSFWRIT your data is fully-interlaced.
c	--- insert the vdata into the above vgroup
c	--- when done, detach the vdata.

c	--- (C) ---
c	--- attach to new (2nd)  vdata, name it 'the-famous-sevens'
c	--- define your own new field 'SIEBEN' type integer.
c	--- write out 70 integers from buf as the field 'SIEBEN'
c	--- tell VSFWRIT your data is fully-interlaced.
c	--- insert the vdata into the above vgroup
c	--- when done, detach the vdata.

c	--- (D) ---
c	--- finally detach the vgroup and close the file

c	--- (A) ----
	f = DFOPEN ('eg1.hdf', FULLACC, 0)
	vg = VFATCH (f, -1,'w')
	call VFSNAM(vg, 'my_fortran_group')

c	--- (B) ----
	vs = VSFATCH (f, -1,'w')
	call VSFSNAM(vs, 'the_fortune_500_vdata')
	n = VSFSFLD (vs, 'IY')
	n = VSFWRIT (vs, buf,20, FINTRLACE)
	n = VFINSRT (vg, vs)
	call VSFDTCH (vs)

c	--- (C) ---
	vs = VSFATCH (f, -1,'w')
	call VSFSNAM(vs, 'the-famous-sevens')
	n = VSFFDEF (vs, 'SIEBEN',INTTYPE,1)
	n = VSFSFLD (vs, 'SIEBEN')
	n = VSFWRIT (vs, sbuf,70, FINTRLACE)
	n = VFINSRT (vg, vs)
	call VSFDTCH (vs)

c	--- (D) ---
	call VFDTCH (vg)
	call DFCLOSE (f)

	end

