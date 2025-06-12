c  ==================================================================	
c
c	EGHI.F
c	HDF VSET Sample Program
c
c	Uses High-Level routines
c	Creates a vset of 1 vgroup and 2 vdatas into the file 'eghi.hdf'.
c
c	Jason NG NCSA 12-MAY-91
c	
c	==================================================================

	program SAMPLE

	integer	buf(100), i, n
	integer f

	external DFOPEN, DFCLOSE
	external VHFSD, VHFSDM, VHMKGP

	integer	DFOPEN
	integer	VHFSD, VHFSDM, VHMKGP

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
111	continue


c	------ write to vset -------

	f = DFOPEN ('eghi.hdf', FULLACC, 0)

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

