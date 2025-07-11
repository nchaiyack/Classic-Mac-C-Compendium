
      dimension x(100), y(100), n1(400), t(100)
      integer f, vs, vg, n
      external DFOPEN,DFCLOSE, VSFATCH, VSFDTCH, VSFSFLD
      external VSFSNAM,VSFWRIT, VSFFDEF, VFATCH, VFDTCH, VFSNAM
      external VFINSRT
      integer DFOPEN
      integer VSFATCH,VSFSFLD,VSFWRIT,VSF
      integer VFATCH, VFINSRT
      integer INTTYPE
      parameter (INTTYPE=2)
      integer FTYPE
      parameter (FTYPE=3)
      integer FINTRLACE
      parameter (FINTRLACE=0)
      integer FULLACC
      parameter (FULLACC=7)     
      imax = 10
      dx = 0.1
      dy = 0.2
      do 10 i = 1, 10
      do 10 j = 1, 10
      i0j0 = i + (j-1) * imax
      i0j01 = 4 * (i0j0-1) + 1
      n1(i0j01) = i0j0
      n1(i0j01+1) = i0j0 + 1
      n1(i0j01+2) = i0j0 + imax
      n1(i0j01+3) = i0j0 + imax + 1
      x(i0j0) = (i-1) * dx
      y(i0j0) = (j-1) * dy
      t(i0j0) = 20 * i + 30 * j
  10  continue
C--------------------
      f = DFOPEN ('vseteg.hdf',FULLACC, 0)
      vg = VFATCH (f, -1, 'w')
      call VFSNAM (vg, 'spv_fortran_group')
C----------------------- x coord ------
      vs = VSFATCH (f, -1, 'w')
      call VSFSNAM (vs, 'xcoord')
      n = VSFSFLD (vs, 'PX')
      n = VSFWRIT (vs, x, 100, FINTRLACE)
      n = VFINSRT (vg, vs)
      call VSFDTCH (vs)
C----------------------- y coord ------
      vs = VSFATCH (f, -1, 'w')
      call VSFSNAM (vs, 'ycoord')
      n = VSFSFLD (vs, 'PY')
      n = VSFWRIT (vs, y, 100, FINTRLACE)
      n = VFINSRT (vg, vs)
      call VSFDTCH (vs)
C----------------------- con ------
      vs = VSFATCH (f, -1, 'w')
      call VSFSNAM (vs, 'con')
      call VSFFDEF (vs, 'PLIST', INTTYPE, 4)
      n = VSFSFLD (vs, 'PLIST')
      n = VSFWRIT (vs, n1, 100, FINTRLACE)
      n = VFINSRT (vg, vs)
      call VSFDTCH (vs)
C----------------------- temp ------
      vs = VSFATCH (f, -1, 'w')
      call VSFSNAM (vs, 'temp')
      call VSFFDEF (vs, 'SCALAR', FTYPE, 1)
      n = VSFSFLD (vs, 'SCALAR')
      n = VSFWRIT (vs, t, 100, FINTRLACE)
      n = VFINSRT (vg, vs)
      call VSFDTCH (vs)
C-----------------------
      call VFDTCH (vg)
      call DFCLOSE (f)
C
      stop
      end
      
      
  
