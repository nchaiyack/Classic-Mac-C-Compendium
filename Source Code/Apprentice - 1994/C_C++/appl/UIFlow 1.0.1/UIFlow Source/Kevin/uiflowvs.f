C======================================================================C
C                                                                      C
C                      UIFLOW - 2 D                                    C
C                    -----------------                                 C
C                                                                      C
C                                                                      C
C     A computer program for TWO-dimensional analysis of               C
C     turbulent elliptic flows in general coordinates.                 C
C                                                                      C
C     UIFLOW2D uses a decoupled multigrid procedure                    C
C     and solves for cell-centered velocities.                         C
C                                                                      C
C     Changes made to this code:                                       C
C     -------------------------                                        C
C                                                                      C
C     (1)  Inclusion of subroutine vset.                               C
C     (2)  Specification of o2 mass fractions based on fuel fractions. C
C     (3)  Error corrected in sub. gammod (-imaxl).                    C
C     (4)  Adaptive cycle removed from sub. moment.                    C
C     (5)  Correct placement of u and v residual calculations.         C
C                                                                      C
C======================================================================C
      SUBROUTINE UIFLOW
      include 'UIFlow.com'
C
      CEXTERNAL SHOWLINE
      CEXTERNAL STOPKEY
      CEXTERNAL ABORTC
	  INTEGER*2 STOPKEY
	  CHARACTER*255 buffer
	  CHARACTER*1 NULL
C
      open(4, file='UIFlow.plt' ,access='sequential',status='unknown')
      open(5, file='UIFlow.In'  ,access='sequential',status='unknown')
      open(8, file='UIFlow.out' ,access='sequential',status='unknown')
      open(11,file='UIFlow.Grid',access='sequential',status='unknown')
C
C.... Read input data and initialise flowfields
C
      NULL = char(0)
      call SHOWLINE('Beginning Computation')
	  call  param
      call  header
      call SHOWLINE('Reading Input File')
      call  input
      if ( model .eq. 2  ) call search
      call  outp
      call  const
      if ( model .ne. 0  ) call constr
      call  zero
      if ( kpgrid .eq. 1 ) call gridp
      if ( kpgrid .eq. 0 ) call grid
      call  geomm
      call  init
C
      write (8,1001) 
      write (8,1002)
C
C.... Set multigrid cycle
C
      igrf = 1
      wrkunt = 0.0
      call SHOWLINE('Writing History')
      write (8, 1006) 'Convergence History'
      write (8, 1007)
      write (8, 1008) igrf
      write (8, 1013)
      write (8, 1005)
C
      write (buffer, 2013) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer, 2008) igrf, NULL
      call SHOWLINE(%ref(buffer))
      write (buffer, 2013) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer, 2005) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer, 3006) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer, 3007) NULL
      call SHOWLINE(%ref(buffer))
C
      xxx = 0
10    continue
C
      nitn(igrf) = nitn(igrf) + 1
      if ( model .ne. 0 ) call lamvis(igrf)
      if ( klam  .eq. 0 ) call tvis  (igrf)
C
C.... Solve momentum and continuity equations
C
      call  moment(igrf)
C
C.... Solve scalar equations including k and epsilon
C
      call  scalrs(igrf)
C
C.... Update thermodynamic properties.
C
      if ( (model .eq. 1) .or. (model .eq. 2) ) then
       call props (igrf)
      endif
C
C.... Enforce boundary conditions.
C
      call  extrpl(igrf)
C
C.... Check convergence on fine grid
C
      write(8,1009)  igrf, nitn(igrf), (error(igrf,nv), nv=3,8)
      write(buffer,2009)  igrf, nitn(igrf), (error(igrf,nv),nv=3,8), NULL
      call SHOWLINE(%ref(buffer))
      xxx = STOPKEY()
	  if (xxx .eq. 1) goto 9999
C
      if ( error(igrf,3) .lt. tolr(igrf) ) goto 20
      if ( nitn(igrf)    .ge. maxitn     ) goto 9000
      if ( error(igrf,3) .gt. 1.0E+5     ) goto 9000
      go to 10
C
20    continue
      write (8,1010) igrf
      write (8,1012)
C
      write (buffer,2012) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2010) igrf, NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2012) NULL
      call SHOWLINE(%ref(buffer))
C
C.... Terminate on finegrid or prolongate
C
      if(igrf .eq. ngrid) go to 50    
C
C.... Extrapolate Solution and increment grid
C
      call  prlong(igrf)
      igrf = igrf + 1
      call  extrpl(igrf)
      write (8,1006) 'Convergence History Continued'
      write (8,1007)
      write (8,1008) igrf
      write (8,1013)
      write (8,1005)
C
      write (buffer,2008) igrf, NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2013) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2005) NULL
      call SHOWLINE(%ref(buffer))
      go to 10
C
50    continue
C
C.... Converged on the desired finest grid
C
      write (8,1015) 'The Problem Has Converged !!'
      write (8,1020)
C
      write (buffer,2020) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2015) 'The Problem Has Converged !!  Wait For Final
     1 Results to be Printed to Output File!', NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2020) NULL
      call SHOWLINE(%ref(buffer))
      go to 9001
C
9000  continue
      write (8,1015) ' The Program is Not Converging Well,
     1 Check The Input Data'
C
      write (buffer,2015) ' The Program is Not Converging Well,
     1 Check The Input Data', NULL
      call SHOWLINE(%ref(buffer))
	  call ABORTC()
	  goto 9999
C
9001  continue
      call  extrpl(ngrid)
      write (8,1006) 'Converged Flow Fields On Finest Grid'
      write (8,1007)
C
      write (buffer,2007) NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2006) 'Converged Flow Fields On Finest Grid',NULL
      call SHOWLINE(%ref(buffer))
      write (buffer,2007) NULL
      call SHOWLINE(%ref(buffer))
C
      call  output(ngrid)
      call  vset
C      call  ftout (ngrid)
C
990   format (5x,'Reading Input Data')
1001  format (72('*')/15x,'Initial Fields For Coarsest Grid')
1002  format (72('*'))
1005  format (/3x,'Grid',2x,'Iter',3x,'Mass',5x,8('-'),
     1  'Norm. Residuals in Scalars ',8('-')/3x,'Numb',
     2  2x,'Numb',3x,'Residual',3x,'Swirl',4x,'Enthalpy',4x,
     3  'k',4x,'eps',4x,'Mix. Fraction'/3x,'----',2x,'----',3x,
     4  8('-'),3x,5('-'),4x,8('-'),2(3x,3('-')),4x,13('-'))
2005  format (3x,'Grid',2x,'Iter',3x,'Mass',5x,8('-'),
     1  'Norm. Residuals in Scalars ',8('-'),a1)
3006  format (3x,'Numb',2x,'Numb',3x,'Residual',3x,'Swirl',4x,'Enthalpy',
     1  4x,'k',4x,'eps',4x,'Mix. Fraction',a1)
3007  format (3x,'----',2x,'----',3x,8('-'),3x,5('-'),4x,8('-'),
     1  2(3x,3('-')),4x,13('-'),a1)
1006  format (/72('*')/25x,72a)
2006  format (25x,72a,1a)
1007  format (72('*'))
2007  format (5x,72('*'),1a)
1008  format (72('-')/15x,'Starting Grid Number',i5)
2008  format (15x,'Starting Grid Number',i5,a1)
1009  format (5x,i2,3x,i4,3x,6(1pe12.5,1x))
2009  format (5x,i2,3x,i4,3x,6(1pe12.5,1x),a1)
1010  format (/5x,30('+')/6x,'Converged on Grid Number',i4)
2010  format (6x,'Converged on Grid Number',i4,a1)
1012  format (5x,30('+'))
2012  format (5x,30('+'),a1)
1013  format (72('-'))
2013  format (72('-'),a1)
1015  format (/5x,81('*')/5x,81a)
2015  format (5x,81a,a1)
1020  format (5x,81('*'))
2020  format (5x,81('*'),a1)
1028  format(i4,a1,1pe10.3)
C
9999  continue
      close(UNIT = 4, status='keep')
      close(UNIT = 5, status='keep')
      close(UNIT = 8, status='keep')
      close(UNIT = 11,status='keep')
C
      return
      end
C======================================================================C
       subroutine adjstx (igrl)
C                                                                      C
C      Purpose:  Perform integral mass adjustments to satisfy overall  C
C                mass balance at every zi station.                     C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      delp  = 0.0
      sumin = 0.0
C
      do 10 j = 2, jmax1
           ij = 1 + (j-1) * imaxl + ibeg
       sumin  = sumin + cx(ij)
10    continue
C
      do 30 i = 2, imax2
        ijf   = i + ibeg
        ijl   = i + (jmax1-1) * imaxl + ibeg
       sumin  = sumin + cy(ijf) - cy(ijl)
C
C.... Calculate actual mass flow rate.
C
       sumflx = 0.0
       sumcf  = 0.0
C
       do 20 j = 2, jmax1
            ij = i + (j-1) * imaxl + ibeg
        sumflx = sumflx + cx(ij)
        sumcf  = sumcf  + ae(ij)
20     continue
       if (i .eq. imax1) sumcf = 1.0
C
C.... Scale fluxes to satisfy mass continuity, and change pressure to
C     correspond to change in mass flux.
C
       delp    = delp + (sumin - sumflx) / sumcf
       if (i .eq. imax1) delp = 0.0
       do 25 j  = 2, jmax1
            ij  = i + (j-1) * imaxl + ibeg
        cx(ij)  = cx (ij) * sumin / sumflx
        cu(ij)  = cu (ij) * sumin / sumflx
        p(ij+1) = p(ij+1) - delp
25     continue
30    continue
C
      return
      end
C======================================================================C
      subroutine apcalc (igrl)
C                                                                      C
C     Purpose:  Calculate apu and apv for use in computing             C
C               the mass fluxes.                                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 10 i  = 2, imax1
             ij = i + ioff
           ijp1 = ij + 1
           ijm1 = ij - 1
            ijp = ij + imaxl
            ijm = ij - imaxl
        apu(ij) = (ae(ij)  * u(ijp1) + aw(ij) * u(ijm1) +
     1             an(ij)  * u(ijp)  + as(ij) * u(ijm)  +
     2             apu(ij) ) / app(ij)
        apv(ij) = (ae(ij)  * v(ijp1) + aw(ij) * v(ijm1) +
     1             an(ij)  * v(ijp)  + as(ij) * v(ijm)  +
     2             apv(ij) ) / ap(ij)
        apm(ij) =  ap(ij)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine bbndry
C                                                                      C
C     Purpose:  Prescribe boundary conditions on the eta minus         C
C               (bottom) boundary of the calculation domain.           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      j = 1
      do 12 n = 1, nsym
C
       if (model .eq. 0) rhym(n) = rhgs
       if (model .eq. 1) rhym(n) = pref * wmair / (gascon * tym(n))
       if (model .eq. 2) then
        yn2ym   = 1.0 - fuym(n) - o2ym(n) - h2oym(n) - co2ym(n)
        rwmym   = fuym(n)/wmfu  + o2ym(n)/wmox + yn2ym/wmn2
     1          + co2ym(n)/wmco2 + h2oym(n)/wmh2o
        wmym(n) = 1.0 / rwmym
        rhym(n) = pref * wmym(n) / (gascon * tym(n))
       endif
C
          ifl = ifym (n,ngrid)
	  ill = ilym (n,ngrid)
       do 10 i = ifl, ill
         ijf   = i + (j-1) * imaxl + ibeg
        u   (ijf)  = ubym(n)
        v   (ijf)  = vbym(n)
        w   (ijf)  = wbym(n)
        p   (ijf)  = 0.0
        t   (ijf)  = tym (n)
        tke (ijf)  = tkym(n)
        tde (ijf)  = tdym(n)
        f   (ijf)  = fym(n)
        g   (ijf)  = gym(n)
        yfu (ijf)  = fuym(n)
        yo2 (ijf)  = o2ym(n)
        yh2o(ijf)  = h2oym(n)
        yco2(ijf)  = co2ym(n)
        yn2 (ijf)  = yn2ym
        gam (ijf)  = vscty
        amu (ijf)  = vscty
        amut(ijf)  = cd * rhym(n) * tkym(n) * tkym(n) /
     1               ( tdym(n) + 1.0e-30 )
        wmol(ijf)  = wmym(n)
        rho (ijf)  = rhym(n)
        cv  (ijf)  = a21(ijf)*u(ijf) + a22(ijf)*v(ijf)
        cy  (ijf)  = cv (ijf)*rho(ijf)
10     continue
12    continue
      return
      end
C======================================================================C
      subroutine bcor (igrl, q1)
C                                                                      C
C     Purpose:  Extrapolate interior values of the given quantity to   C
C               the wall.  Consistent with enforcing a zero normal     C
C               derivative of the given variable.                      C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q1(*)
      include 'UIFlow.indx'
C
      do 10 j = 1, jmaxl
         ijf  = (j-1) * imaxl + ibeg
       q1(ijf + 1)   = q1 (ijf  + 2)
       q1(ijf+imaxl) = q1 (ijf+imax1)
10    continue
C
      do 20 i = 1, imaxl
         ijf  = i + ibeg
         ij1  = ijf + jmax1 * imaxl
       q1(ijf) = q1 (ijf+imaxl)
       q1(ij1) = q1 (ij1 - imaxl)
20    continue
C
      return
      end
C======================================================================C
      block data param
C======================================================================C
      include 'UIFlow.com'
      data cd, cdqtr, cdtqtr, ee, ak/ 0.09,0.5477,0.1643,9.025,0.4/
      data ce1, ce2, cg1, cg2, cr/ 1.44, 1.92, 2.8, 2.0, 3.0/
      data acpox, bcpox, ccpox/938.5681,0.0972,-1.7167e-5/
      data acpn2, bcpn2, ccpn2/842.5130,0.2364,-6.1891e-5/
      data acpco2,bcpco2,ccpco2/842.7291,0.2927,-7.8040e-5/
      data acph2o,bcph2o,ccph2o/1215.6587,0.7182,-1.3888e-4/
      data acpfu, bcpfu, ccpfu/1420.5564,1.7792,-3.9418e-4/
      data wmh2o, wmco2/ 18.01520, 44.00980/
      data wmn2, wmox/ 28.161 , 31.99880/
      data hox , hn2 / 2.8802e5, 2.7057e5 /
      data hco2, hh2o/ 2.7521e5, 4.2261e5 /
      data rox, wmair, gascon/0.2331, 28.967, 8314.3/
      data airt, airv, airs, airsum/273.11, 1.716e-5, 110.56, 383.67/
      data hfu, wmfu /5.7125e5, 44.09620/
      data hffu, hfco2, hfh2o / -2.35510e6, -8.94114e6, -1.34228e7/
      data gamma/ 1.40 /
      data acpair, bcpair, ccpair/868.3073, 0.2054, -51.846e-6/
      data grav, kgrav/9.80665,0/
      data beta, rdfctr/0.85, 0.1/
      data bta/0.1024/
      data refu, refv, refw, refc/1.0,1.0,1.0,1.0/
      data nitr/1,1,1,1,1/
      data tolr/0.01,0.001,0.01,0.01,0.01/
      data nitke/3/
      data knorth/0/
      data ind1,ind2,ind3,ind4/1,1,1,1/
      data alft, alfrho/0.95,0.95/
      end
C======================================================================C
      subroutine cflux (igrl)
C                                                                      C
C     Purpose:  Calculate values of the mass fluxes and contravariant  C
C               velocities at the cell faces.                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Calculate flux perpendicular to eta coordinate.
C
      call grad (igrl, p)
      relxm = 1.0 - relx(1)
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax2
           ij  = i + ioff
          ij1  = ij + 1
        cu(ij) = a11(ij) *
     1         ( fx (ij) * ( apu(ij1)+duy(ij1)*dpdy(ij1)/app(ij1) )
     2         + fx1(ij) * ( apu(ij) +duy(ij) *dpdy(ij) /app(ij) ) )
     3         + a12(ij) *
     4         ( fx (ij) * ( apv(ij1)+dvy(ij1)*dpdy(ij1)/ap(ij1))
     5         + fx1(ij) * ( apv(ij) +dvy(ij) *dpdy(ij) /ap(ij)  ) )
     6         + ( a11(ij)*a11(ij)*(fx(ij)/app(ij1) + fx1(ij)/app(ij))
     7         +   a12(ij)*a12(ij)*(fx(ij)/ap(ij1)  + fx1(ij)/ap(ij)))
     8         * ( p(ij) - p(ij1) ) + relxm  * cu(ij)
        cx(ij) = cu(ij) * ( amax1( sign( rho(ij) ,  cu(ij) ), 0.)
     1                  +   amax1( sign( rho(ij1), -cu(ij) ), 0.) )
10     continue
11    continue
C
C.... Calculate flux perpendicular to zi coordinate.
C
      do 21 j  = 2, jmax2
         ioff  = (j-1) * imaxl + ibeg
	ioffp  = ioff + imaxl
       do 20 i = 2, imax1
           ij  = i + ioff
	  ijp  = i + ioffp
        cv(ij) = a21(ij) *
     1         ( fy (ij) * ( apu(ijp) + dux(ijp)*dpdx(ijp)/app(ijp) )
     2         + fy1(ij) * ( apu(ij)  + dux(ij) *dpdx(ij) /app(ij)) )
     3         + a22(ij) *
     4         ( fy(ij)  * ( apv(ijp) + dvx(ijp)*dpdx(ijp)/ap(ijp) )
     5         + fy1(ij) * ( apv(ij)  + dvx(ij) *dpdx(ij) /ap(ij)  ) )
     6         + ( a21(ij)*a21(ij)*(fy(ij)/app(ijp) + fy1(ij)/app(ij))
     7         +   a22(ij)*a22(ij)*(fy(ij)/ap(ijp)  + fy1(ij)/ap(ij)))
     8         * ( p(ij) - p(ijp) ) + relxm  * cv(ij)
        cy(ij) = cv(ij) * ( amax1( sign( rho(ij) ,  cv(ij) ), 0.)
     1                  +   amax1( sign( rho(ijp), -cv(ij) ), 0.) )
20     continue
21    continue
      return
      end
C======================================================================C
      subroutine cfluxc (igrl)
C                                                                      C
C     Purpose:  Make corrections to contravariant velocities and mass  C
C               fluxes.  For use when iterations are being performed   C
C               on the coarse grids.                                   C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      imaxc  =  imax(igrl)
      jmaxc  =  jmax(igrl)
      imaxf  =  imax(igrl+1)
      jmaxf  =  jmax(igrl+1)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      imaxc2 =  imaxc - 2
      jmaxc2 =  jmaxc - 2
      ibegc  =  nbeg(igrl)
C
      do 6  jc  = 1, jmaxc
       do 5  ic = 1, imaxc
            ijc = ic + (jc-1) * imaxc + ibegc
        dx(ijc) = 0.0
        dy(ijc) = 0.0
        x3(ijc) = 0.0
        su(ijc) = 0.0
5      continue
6     continue
C
C.... Evaluate corrections to finer grid.
C
      do 11 jc   = 2, jmaxc1
       do 10 ic  = 2, imaxc1
            ijc  = ic + (jc - 1) * imaxc + ibegc
	    ijf  = iru (ijc)
           ijf1  = ijf - 1
           ijfm  = ijf - imaxf
          ijfm1  = ijf - imaxf - 1
        apm(ijc) = ap(ijc)
        dx(ijc)  = u(ijc) - 0.25 * (u(ijf) + u(ijf1) + u(ijfm) +
     1             u(ijfm1))
        dy(ijc)  = v(ijc) - 0.25 * (v(ijf) + v(ijf1) + v(ijfm) +
     1             v(ijfm1))
        x3(ijc)  = p(ijc) - 0.25 * (p(ijf) + p(ijf1) + p(ijfm) +
     1             p(ijfm1))
10     continue
11    continue
C
      call bcor (igrl, x3)
C
      call trsrc (igrl, dx)
C
      do 21 j   = 2, jmaxc1
          ioff  = (j-1) * imaxc + ibegc
       do 20 i  = 2, imaxc1
            ij  = i + ioff
        apu(ij) = (ae(ij) * dx(ij+1)      + aw(ij) * dx(ij-1) +
     1             an(ij) * dx(ij+imaxc)  + as(ij) * dx(ij-imaxc) 
     2          +  resux(ij) + su(ij))/app(ij)
20     continue
21    continue
C
      call trsrc (igrl, dy)
C
      do 23 j   = 2, jmaxc1
          ioff  = (j-1) * imaxc + ibegc
       do 22 i  = 2, imaxc1
            ij  = i + ioff
        apv(ij) = (ae(ij) * dy(ij+1)      + aw(ij) * dy(ij-1) +
     1             an(ij) * dy(ij+imaxc)  + as(ij) * dy(ij-imaxc) 
     2          +  resvx(ij) + su(ij))/ap(ij)
22     continue
23    continue
C
C.... Restrict fluxes again from fine grid.
C
      call restfl (igrl+1)
      call grad (igrl, x3)
C
      do 31 j  = 2, jmaxc1
         ioff  = (j-1) * imaxc + ibegc
        ioffp  = ioff + 1
       do 30 i = 2, imaxc2
           ij  = i + ioff
          ij1  = i + ioffp
        cu(ij) = a11(ij) *
     1         ( fx (ij) * ( apu(ij1)+duy(ij1)*dpdy(ij1)/app(ij1) )
     2         + fx1(ij) * ( apu(ij) +duy(ij) *dpdy(ij) /app(ij) ) )
     3         + a12(ij) *
     4         ( fx (ij) * ( apv(ij1)+dvy(ij1)*dpdy(ij1)/ap(ij1))
     5         + fx1(ij) * ( apv(ij) +dvy(ij) *dpdy(ij) /ap(ij)  ) )
     6         + ( a11(ij)*a11(ij)*(fx(ij)/app(ij1) + fx1(ij)/app(ij))
     7         +   a12(ij)*a12(ij)*(fx(ij)/ap(ij1)  + fx1(ij)/ap(ij)))
     8         * ( x3(ij) - x3(ij1) ) + cu(ij)
        cx(ij) = cu(ij)  * (amax1 (sign (rho(ij) ,  cx(ij)), 0.0)
     1                   +  amax1 (sign (rho(ij1), -cx(ij)), 0.0))
30     continue
31    continue
C
      do 41 j  = 2, jmaxc2
          ioff = (j-1) * imaxc + ibegc
	 ioffp = ioff + imaxc
       do 40 i = 2, imaxc1
           ij  = i + ioff
	  ijp  = i + ioffp
        cv(ij) = a21(ij) *
     1         ( fy (ij) * ( apu(ijp) + dux(ijp)*dpdx(ijp)/app(ijp) )
     2         + fy1(ij) * ( apu(ij)  + dux(ij) *dpdx(ij) /app(ij)) )
     3         + a22(ij) *
     4         ( fy(ij)  * ( apv(ijp) + dvx(ijp)*dpdx(ijp)/ap(ijp) )
     5         + fy1(ij) * ( apv(ij)  + dvx(ij) *dpdx(ij) /ap(ij)  ) )
     6         + ( a21(ij)*a21(ij)*(fy(ij)/app(ijp) + fy1(ij)/app(ij))
     7         +   a22(ij)*a22(ij)*(fy(ij)/ap(ijp)  + fy1(ij)/ap(ij)))
     8         * ( x3(ij) - x3(ijp) ) + cv(ij) 
        cy(ij) = cv(ij)  * (amax1 (sign (rho(ij)  ,  cy(ij)), 0.0)
     1                   +  amax1 (sign (rho(ijp), - cy(ij)), 0.0))
40     continue
41    continue
      return
      end    
C======================================================================C
      subroutine cfpmod (igrl)
C                                                                      C
C     Purpose:  Modify coefficients of the pressure correction         C
C               equation to account for boundary condition.            C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      if (kcomp .eq. 1) call pcoefc (igrl)
C
      do 10 j  = 2, jmax1
          ioff = (j-1) * imaxl + ibeg
	  ij1  = ioff + 2
	  ij2  = ioff + imax1
       ap(ij1) = ap(ij1) - aw(ij1)
       ap(ij2) = ap(ij2) - ae(ij2)
       aw(ij1) = 0.0
       ae(ij2) = 0.0
       dx(ij2) = 0.0
10    continue
C
      do 20 i  = 2, imax1
          ioff = i + ibeg
	  ij1  = ioff + imaxl
	  ij2  = ioff + (jmax1-1) * imaxl
       ap(ij1) = ap(ij1) - as(ij1)
       ap(ij2) = ap(ij2) - an(ij2)
       as(ij1) = 0.0
       an(ij2) = 0.0
       dy(ij2) = 0.0
20    continue
C
      return
      end
C======================================================================C
      subroutine coeff(igrl)
C                                                                      C
C     Purpose:  Calculate coefficients of the discretized equation     C
C               for any flow variable.  The hybrid discretization      C
C               scheme is presently being used.                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j = 2, jmax1
         ioff = (j-1) * imaxl + ibeg
        ioff1 = ioff - 1
        ioffm = ioff - imaxl
       do 10 i = 2, imax1
           ij  = i + ioff
          ij1  = i + ioff1
          ijm  = i + ioffm
        ae(ij)  = amax1( 0., -cx(ij) , -cx(ij)  * fx (ij)  + dx(ij)  )
        aw(ij)  = amax1( 0.,  cx(ij1),  cx(ij1) * fx1(ij1) + dx(ij1) )
        an(ij)  = amax1( 0., -cy(ij) , -cy(ij)  * fy (ij)  + dy(ij)  )
        as(ij)  = amax1( 0.,  cy(ijm),  cy(ijm) * fy1(ijm) + dy(ijm) )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine coeffp (igrl)
C                                                                      C
C     Purpose:  Calculate coefficients of the pressure correction      C
C               equation.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
          ioff = (j-1) * imaxl + ibeg
         ioff1 = ioff + 1
         ioffp = ioff + imaxl
       do 10 i = 2, imax1
           ij  = i + ioff
          ij1  = i + ioff1
          ijp  = i + ioffp
        ae(ij) = ( a11(ij) * a11(ij)  + a12(ij) * a12(ij) )
     1         * ( fx(ij)  / apm(ij1) + fx1(ij) / apm(ij)  )
        ae(ij) = ae(ij)  * ( amax1( sign( rho(ij)  ,  cx(ij) ), 0.)
     1                   +   amax1( sign( rho(ij1), -cx(ij) ), 0.) )
        dx(ij) = ae(ij)
        an(ij) = ( a21(ij) * a21(ij) + a22(ij) *  a22(ij) )
     1         * ( fy(ij)  / apm(ijp) + fy1(ij) / apm(ij) )
        an(ij) = an(ij)  * ( amax1( sign( rho(ij) ,  cy(ij) ), 0.)
     1                   +   amax1( sign( rho(ijp), -cy(ij) ), 0.) )
        dy(ij) = an(ij)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine const
C                                                                      C
C     Purpose:  Compute indices on coarse grids.                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      imax(ngrid) = ncelx + 2
      jmax(ngrid) = ncely + 2
      nbeg (1)    = 0
      if( ngrid .eq. 1 ) return
C
      ngrid1   = ngrid - 1
      do 10 nr = 1, ngrid1
            n  = ngrid - nr
       imax(n) = ncelx / (2 ** nr) + 2
       jmax(n) = ncely / (2 ** nr) + 2
10    continue
C
      do 15 n  = 2, ngrid
       nbeg(n) = nbeg(n-1) + imax(n-1)*jmax(n-1)
15    continue
C
C.... Indices for prolongation.
C
      do 25 igr = 1, ngrid1
       imaxc  = imax (igr)
       jmaxc  = jmax (igr)
       ibegc  = nbeg (igr)
       imaxf  = imax(igr+1)
       jmaxf  = jmax(igr+1)
       ibegf  = nbeg(igr+1)
       do 18 j = 1, jmaxc
        do 18 i = 1, imaxc
         ijc     = i + (j-1) * imaxc + ibegc
         ijf     = 2*i-1 + (2*j-2)*imaxf + ibegf
         iru(ijc) = ijf
18      continue
C
C.... Modify at boundaries.
C
        j = jmaxc
        do 20 i   = 2, imaxc-1
          ijc     = i + (j-1) * imaxc + ibegc
          ijf     = 2*i-1 + (2*j-3)*imaxf + ibegf
         iru(ijc) = ijf
20      continue
        i = imaxc
        do 21 j   = 2, jmaxc-1
          ijc     = i + (j-1) * imaxc + ibegc
          ijf     = 2*(i-1) + (2*j-2)*imaxf + ibegf
         iru(ijc) = ijf
21     continue
25    continue
C
C.... Restrict segment indices.
C
      do 40 nr = 1, ngrid1
            n  = ngrid - nr
C
       do 31 i = 1, nsxm
        jfxm(i,n) = (jfxm (i,n+1)-2)/2 + 2
        jlxm(i,n) = (jlxm (i,n+1)-1)/2 + 1
31     continue
C
       do 32 i = 1, nsxp
        jfxp(i,n) = (jfxp (i,n+1)-2)/2 + 2
        jlxp(i,n) = (jlxp (i,n+1)-1)/2 + 1
32     continue
C
       do 33 i = 1, nsym
        ifym(i,n) = (ifym (i,n+1)-2)/2 + 2
        ilym(i,n) = (ilym (i,n+1)-1)/2 + 1
33     continue
C
       do 34 i = 1, nsyp
        ifyp(i,n) = (ifyp (i,n+1)-2)/2 + 2
        ilyp(i,n) = (ilyp (i,n+1)-1)/2 + 1
34     continue
C
40    continue
C
      return
      end
C======================================================================C
      subroutine constr
C                                                                      C
C     Purpose:  Compute the constants required for the combustion      C
C               models.                                                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
C.... Values for PROPANE ( C3H8 ).
C
      if ( kfuel .eq. 0 ) then
       wmfu   = 44.0962
       wmpr   = 28.3238
       hreact = 46.353e6
       stoic  = 15.5715
       tstoic = 2396.0
       acpfu  = 1420.56
       bcpfu  = 1.7792
       ccpfu  = -3.94184e-4
       acpprd = 882.544
       bcpprd = 1.0537
       ccpprd = -2.22175e-4
       cxx    = 3.0
       hyy    = 8.0
       aa(1)  = 0.10
       bb(1)  = 1.65
       acten(1) = 30000.0
       prexp(1) = 8.6e11
      endif
C
C.... Values for METHANE ( CH4 ).
C
      if ( kfuel .eq. 1 ) then
       wmfu   = 16.04260
       wmpr   = 27.6339
       hreact = 44.164e6
       stoic  = 17.1206
       tstoic = 2329.0
       acpfu  = 1126.59
       bcpfu  = 2.3305
       ccpfu  = -481.2e-6
       acpprd = 891.9962
       bcpprd = 0.3055
       ccpprd = -74.111e-6
       cxx    = 1.0
       hyy    = 4.0
       aa(1)  = -0.3
       bb(1)  = 1.3
       acten(1) = 48400
       prexp(1) = 1.3e8
      endif
C
C.... Values for TOWN GAS ( mixture of methane and ethane ).
C
      if ( kfuel .eq. 2 ) then
       wmfu   = 18.4349
       wmpr   = 27.7509
       hreact = 37.0227e6
       stoic  = 14.376
       tstoic = 2510.0
       acpfu  = 1066.49
       bcpfu  = 1.9408
       ccpfu  = -403.83e-6
       acpprd = 888.335
       bcpprd = 0.3022
       ccpprd = -73.406e-6
       cxx    = 1.0
       hyy    = 4.0
       aa(1)  = -0.3
       bb(1)  = 1.3
       acten(1) = 48400
       prexp(1) = 1.3e8
      endif
C
C.... Calculate constants used for diffusion flame model.
C
      fstoic = 1.0 / ( 1.0 + stoic  )
      rstoic = pref * wmpr / gascon / tstoic
      ysub   = 1.0 / ( 1.0 - fstoic )
      rfuel  = pref * wmfu / gascon / tfuel
      rair   = pref * wmair / gascon / tair
      denom  = fstoic * ( 1.0 - fstoic )
      tprd   = alft * ( tstoic -  ( (1.0 - fstoic) * tair
     1       + fstoic * tfuel ) ) / denom
      rprd   = alfrho * ( rstoic -  ( (1.0 - fstoic) * rair
     1       + fstoic * rfuel ) ) / denom
      phia   = - 1.0 / stoic
      phifma = 1.0 +  ( 1.0 /  stoic )
C
      cpf    = acpfu  + bcpfu * tfuel + ccpfu  * tfuel * tfuel
      cpa    = acpair + bcpair * tair + ccpair * tair  * tair
      enthfu = cpf * tfuel + hreact
      enthox = cpa * tair
C
C.... Calculate constants used for premixed flame model.
C
      rat(1)   = cxx * wmco2 / wmfu
      rat(2)   = ( hyy * wmh2o ) / ( 2.0 * wmfu )
      rat(3)   = cxx * wmox / wmfu + ( hyy * wmox ) / ( 4.0 * wmfu )
      rat(4)   = 3.0 * wmco2 / wmfu
      rat(5)   = 4.0 * wmh2o / wmfu
C
      ab(1)    = aa(1) + bb(1)
      prexp(1) = 1000.0 * wmfu * prexp(1) * ( 0.001 )**ab(1)
     1         * ( 1.0 / wmfu )**aa(1) * ( 1.0 / wmox )**bb(1)
      acten(1) = acten(1) * 4.1868 / 8.3143
C
C....Ensure that initial estimate of mixture fraction is LARGER than
C    the fuel mass fraction.
C
      if ( fugs .gt. fgs ) then
       temp = fgs
       fgs  = fugs
       fugs = temp
      endif
C
C....Specify initial estimates of mass fractions based on the given
C    estimates of mixture fraction and fuel mass fraction.
C
      o2gs  = rox * ( 1.0 - fgs ) - rat(3) * ( fgs - fugs )
      co2gs = rat(1) * ( fgs - fugs )
      h2ogs = rat(2) * ( fgs - fugs )
C
      return
      end
C======================================================================C
      subroutine cpenth (igrl)
C                                                                      C
C     Purpose:  Calculate the specific heat appropriate for            C
C               determination of the fluid temperature.                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Cp for air only.
C
      if (model .eq. 1) then
       do 11 j   = 1, jmaxl
          ioff   = (j-1) * imaxl + ibeg
        do 10 i  = 1, imaxl
             ij  = i + ioff
         cph(ij) = acpair + bcpair * t(ij) + ccpair * t(ij) * t(ij)
10      continue
11     continue
C
      elseif (model .eq. 2) then
C
C.... Cp for mixture of fuel and air.
C
       do 21 j   = 1, jmaxl
           ioff  = (j-1) * imaxl + ibeg
        do 20 i  = 1, imaxl
             ij  = i + ioff
         cpfu    = acpfu  + bcpfu  * t(ij) + ccpfu  * t(ij) * t(ij)
         cpox    = acpox  + bcpox  * t(ij) + ccpox  * t(ij) * t(ij)
         cpn2    = acpn2  + bcpn2  * t(ij) + ccpn2  * t(ij) * t(ij)
         cpco2   = acpco2 + bcpco2 * t(ij) + ccpco2 * t(ij) * t(ij)
         cph2o   = acph2o + bcph2o * t(ij) + ccph2o * t(ij) * t(ij)
         cph(ij) = yfu(ij) * cpfu  +  yo2(ij)  * cpox
     1           + yn2(ij) * cpn2  +  yco2(ij) * cpco2
     2           + yh2o(ij)* cph2o
20      continue
21     continue
      endif
C
      return
      end
C======================================================================C
      subroutine cpgrad (igrl)
C                                                                      C
C     Purpose:  Compute the cross derivative terms in the pressure     C
C               correction equation.                                   C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 6 j  = 1, jmaxl
        ioff  = (j-1) * imaxl + ibeg
       do 5 i = 1, imaxl
           ij = i + ioff
        x1(ij) = 0.0
        x2(ij) = 0.0
5      continue
6     continue
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
        ioffp  = ioff + 1
       do 10 i = 2, imax2
           ij  = i + ioff
          ij1  = i + ioffp
        x1(ij) = a11(ij) * (fx(ij) * duy(ij1) * dpdy(ij1)/apm(ij1) +
     1                      fx1(ij)* duy(ij)  * dpdy(ij) /apm(ij)) +
     2           a12(ij) * (fx(ij) * dvy(ij1) * dpdy(ij1)/apm(ij1) +
     3                      fx1(ij)* dvy(ij)  * dpdy(ij) /apm(ij))
10     continue
11    continue
C
      do 21 j  = 2, jmax2
         ioff  = (j-1) * imaxl + ibeg
	ioffp  = ioff + imaxl
       do 20 i = 2, imax1
           ij  = i + ioff
	  ijp  = i + ioffp
        x2(ij) = a21(ij) * (fy(ij) * dux(ijp) * dpdx(ijp)/apm(ijp) +
     1                      fy1(ij)* dux(ij)  * dpdx(ij) /apm(ij)) +
     4           a22(ij) * (fy(ij) * dvx(ijp) * dpdx(ijp)/apm(ijp) +
     5                      fy1(ij)* dvx(ij)  * dpdx(ij) /apm(ij))
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine dens (igrl)
C                                                                      C
C     Purpose:  Calculate the fluid density based on the assumption    C
C               of ideal gas behavior.  Allocation is also made for    C
C               low mach number flames in which the base pressure of   C
C               the system determines the thermodynamic state.         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      relxm = 1.0 - relx(11)
C
      if ( kcomp .eq. 0 ) then
C
C.... Low mach number case ( Charles' law is applicable ).
C
       do 11 j   = 2, jmax1
          ioff   = (j-1) * imaxl + ibeg
        do 10 i  = 2, imax1
             ij  = i + ioff
         rhl     = pref * wmol(ij) / ( gascon * t(ij) )
         rho(ij) = relx(11) * rhl + relxm * rho(ij)
10      continue
11     continue
C
      elseif ( kcomp .eq. 1 ) then
C
C.... Density determination for a compressible flow.
C
       do 21 j   = 2, jmax1
          ioff   = (j-1) * imaxl + ibeg
        do 20 i  = 2, imax1
             ij  = i + ioff
         rhl     = (p(ij) + pref) * wmol(ij) / ( gascon * t(ij) )
         rho(ij) = relx(11) * rhl + relxm * rho(ij)
20      continue
21     continue
      endif
C
      return
      end
C======================================================================C
      subroutine dflux (igrl)
C                                                                      C
C     Purpose:  Compute the diffusion contributions to the             C
C               coefficients in the discretized equations.             C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 1, jmax1
          ioff = (j-1) * imaxl + ibeg
         ioffp = ioff + imaxl
       do 10 i = 1, imax1
           ij  = i + ioff
          ijp  = i + ioffp
        dx(ij) = (fx(ij) * gam(ij+1) + fx1(ij) * gam(ij)) * q11(ij)
        dy(ij) = (fy(ij) * gam(ijp)  + fy1(ij) * gam(ij)) * q22(ij)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine enth (igrl)
C                                                                      C
C     Purpose:  Calculate initial sensible enthalpy field.             C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      if (model .eq. 1) then
C
C.... For air only.
C
      hmix = rox * hox  +  (1.0 - rox) * hn2
C
       do 11 j  = 1, jmaxl
          ioff  = (j-1) * imaxl + ibeg
        do 10 i = 1, imaxl
             ij = i + ioff
         h(ij)  = cph(ij) * t(ij) - hmix
10      continue
11     continue
C
      elseif (model .eq. 2) then
C
C.... For constituents of the one step chemical reaction.
C
       do 21 j  = 1, jmaxl
          ioff  = (j-1) * imaxl + ibeg
        do 20 i = 1, imaxl
             ij = i + ioff
         hmix   =  yfu(ij)  * hfu  +  yo2(ij)  * hox 
     1          +  yn2(ij)  * hn2  +  yco2(ij) * hco2
     2          +  yh2o(ij) * hh2o
         h(ij)  =  cph(ij)  * t(ij) - hmix
20      continue
21     continue
      endif
C
      return
      end
C======================================================================C
      subroutine extrpl(igrl)
C                                                                      C
C     Purpose:  Enforce boundary conditions by extrapolating interior  C
C               values.                                                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      call xmextr (igrl)
      call xpextr (igrl)
      call ymextr (igrl)
      call ypextr (igrl)
      if ( kadj .eq. 0 ) call mssout (igrl)
C
      return
      end
C======================================================================C
      subroutine fstchm (igrl)
C                                                                      C
C     Purpose:  Calculate quantities pertinent to the diffusion flame  C
C               model.                                                 C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Properties for a LAMINAR reacting flow assuming a mixing limited
C     reaction.
C
      call scalar (igrl, 8, f)
C
      relxm  = 1.0 - relx(5)
      rwmfu  = 1.0 / wmfu
      rwmair = 1.0 / wmair
      rwmpr  = 1.0 / wmpr
C
      do 11 j     = 2, jmax1
          ioff    = (j-1) * imaxl + ibeg
       do 10 i    = 2, imax1
             ij   = i + ioff
        phi       = f(ij) * phifma + phia
        yfu  (ij) = amax1( 0.0, phi )
        yo2  (ij) = ( yfu(ij) - phi ) * stoic
        yco2 (ij) = 1.0 - yfu(ij) - yo2(ij)
        h    (ij) = f(ij) * enthfu + (1.0 - f(ij)) * enthox
        cpf       = acpfu  + bcpfu  * t(ij) + ccpfu  * t(ij) * t(ij)
        cpa       = acpair + bcpair * t(ij) + ccpair * t(ij) * t(ij)
        cprd      = acpprd + bcpprd * t(ij) + ccpprd * t(ij) * t(ij)
        cph  (ij) = yfu(ij)  * cpf + yo2(ij) * cpa
     1            + yco2(ij) * cprd
        tl        = ( h(ij) - yfu(ij) * hreact ) / cph(ij)
        t(ij)     = relx(5) * tl + relxm * t(ij)
        rwmol     = yfu(ij)*rwmfu + yo2(ij)*rwmair + yco2(ij)*rwmpr
        wmol(ij)  = 1.0 / rwmol
10     continue
11    continue
      call dens (igrl)
C
      if (klam .eq. 0) then
C
C.... Properties for a TURBULENT reacting flow assuming a mixing
C     limited reaction.
C
       call scalar (igrl, 10, g)
C
       relxmr = 1.0 - relx(11)
       relxmt = 1.0 - relx(5)
       stcp1  = 1.0 + stoic
C
       do 31 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
        do 30 i = 2, imax1
            ij  = i + ioff
         fprme    =  sqrt( g(ij) )
         zsubs    = abs( fstoic - f(ij) ) / fprme
         unmxd    = 0.45 * exp ( - zsubs )
         unprd    = ysub * fprme * unmxd
         yco2(ij) = yco2(ij) - stcp1 * unprd
         if ( yco2(ij) .lt. 0 ) then
          unmxd = 0.5 * unmxd
          unprd = 0.5 * unprd
         endif
         yfu(ij)  = yfu(ij) + unprd
         yo2(ij)  = yo2(ij) + stoic * unprd
         yco2(ij) = 1.0 - yfu(ij) - yo2(ij)
         tl       = t(ij) - tprd * fprme * unmxd
         t(ij)    = relx(5) * tl + relxmt * t(ij)
         rrho     = 1.0 / rho(ij) - rprd * fprme * unmxd
         rhl      = 1.0 / rrho
         rho(ij)  = relx(11) * rhl + relxmr * rho(ij)
         rwmol    = yfu(ij)*rwmfu + yo2(ij)*rwmair + yco2(ij)*rwmpr
         wmol(ij) = 1.0 / rwmol
30      continue
31     continue
      endif
C
      return
      end
C======================================================================C
      subroutine ftout(igrl)
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      i = ind1
      do 20 j = 2, jmax1
           ij = i + (j-1) * imaxl + ibeg
          ij1 = ij - imaxl
       rav = 0.25 * (r(ij) + r(ij-1) + r(ij1) + r(ij1-1))/0.0762
       write (22,999) rav, char(9), u(ij), char(9), tke(ij)
20    continue
C
      i = ind2
      do 21 j = 2, jmax1
          ij  = i + (j-1) * imaxl + ibeg
          ij1 = ij - imaxl
       rav = 0.25 * (r(ij) + r(ij-1) + r(ij1) + r(ij1-1))/0.0762
       write (23,999) rav, char(9), u(ij), char(9), tke(ij)
21    continue
C
      i = ind3 
      do 22 j = 2, jmax1
          ij  = i + (j-1) * imaxl + ibeg
          ij1 = ij - imaxl
       rav = 0.25 * (r(ij) + r(ij-1) + r(ij1) + r(ij1-1))/0.0762
       write (24,999) rav, char(9), u(ij), char(9), tke(ij)
22    continue
C
      i = ind4
      do 23 j = 2, jmax1
          ij  = i + (j-1) * imaxl + ibeg
          ij1 = ij - imaxl
       rav = 0.25 * (r(ij) + r(ij-1) + r(ij1) + r(ij1-1))/0.0762
       write (25,999) rav, char(9), u(ij), char(9), tke(ij)
23    continue
C
999   format(e12.5,2(a1,e12.5))
      return
      end
C======================================================================C
      subroutine fxx (igrl)
C                                                                      C
C     Purpose:  Compute the interpolation factor in zi coordinate      C
C               direction.                                             C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 2, jmax1
         ioff   = (j-1) * imaxl + ibeg
       do 10 i  = 2, imax2
            ij  = ioff + i
        fx (ij) = xxic(ij) / (xxic(ij) + xxic(ij+1))
        fx1(ij) = 1.0 - fx(ij)
10     continue
11    continue
C
      call bcor (igrl, fx)
      call bcor (igrl, fx1)
C
      do 12 j  = 2, jmax1
         ijf   = (j-1) * imaxl + ibeg
       fx1 (1 + ijf)     = 1.0
       fx1 (imax1 + ijf) = 0.0
       fx (1 + ijf)      = 0.0
       fx (imax1 + ijf)  = 1.0
12    continue
C
      return
      end
C======================================================================C
      subroutine fyy (igrl)
C                                                                      C
C     Purpose:  Compute the interpolation factor in eta coordinate     C
C               direction.                                             C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 i   = 2, imax1
          ioff  = i + ibeg
       do 10 j  = 2, jmax2
            ij  = ioff + (j-1) * imaxl 
        fy (ij) = yetac(ij) / (yetac(ij) + yetac(ij+imaxl))
        fy1(ij) = 1.0 - fy(ij)
10     continue
11    continue
C
      call bcor (igrl,fy)
      call bcor (igrl,fy1)
C
      do 12 i = 2, imax1
          ijf = i + ibeg
       fy1(ijf)                 = 1.0
       fy1(jmax2 * imaxl + ijf) = 0.0
       fy (ijf)                 = 0.0
       fy (jmax2 * imaxl + ijf) = 1.0
12    continue
C
      return
      end
C======================================================================C
      subroutine gammod (igrl)
C                                                                      C
C     Purpose:  Modify the viscosities at the walls so as to impose    C
C               wall functions on the mean velocity field.             C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      i = 2
      do 10 n = 1, nsxm
       if ( kbxm(n) .ne. 1 ) goto 10
         jfl  = jfxm(n,igrl)
	 jll  = jlxm(n,igrl)
        call  wallg (igrl,i,i,jfl,jll,-1,xxic)
10    continue
C
      i = imax1
      do 20 n = 1, nsxp
       if ( kbxp(n) .ne. 1 ) goto 20
          jfl = jfxp(n,igrl)
	  jll = jlxp(n,igrl)
       call  wallg (igrl,i,i,jfl,jll,1,xxic)
20    continue
C
      j = 2
      do 30 n = 1, nsym
       if ( kbym(n) .ne. 1 ) goto 30
         ifl  = ifym(n,igrl)
	 ill  = ilym(n,igrl)
       call  wallg (igrl,ifl,ill,j,j,-imaxl,yetac)
30    continue
C
      j = jmax1
      do 40 n = 1, nsyp
       if ( kbyp(n) .ne. 1 ) goto 40
         ifl  = ifyp(n,igrl)
	 ill  = ilyp(n,igrl)
       call  wallg (igrl,ifl,ill,j,j,imaxl,yetac)
40    continue
C
      return
      end
C======================================================================C
      subroutine geomc1 (igrl)
C                                                                      C
C     Purpose:  Calculate geometry terms associated with the           C
C               transformation to general curvilinear coordinates.     C
C               Geometry terms stored at cell centers are              C
C               calculated here.                                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 2, jmax1
         ioff   = (j-1) * imaxl + ibeg
        ioff1   = ioff - imaxl
       do 10 i  = 2, imax1
            ij  = i + ioff
           ij1  = i + ioff1
        xzi     = 0.5 * ( x(ij)  + x(ij1)  -  x(ij-1) - x(ij1-1) )
        yzi     = 0.5 * ( y(ij)  + y(ij1)  -  y(ij-1) - y(ij1-1) )
        xeta    = 0.5 * ( x(ij)  + x(ij-1) -  x(ij1)  - x(ij1-1) )
        yeta    = 0.5 * ( y(ij)  + y(ij-1) -  y(ij1)  - y(ij1-1) )
        ajb(ij) =   xzi * yeta - xeta * yzi
        q12(ij) = - yzi * yeta - xzi  * xeta
        xxic (ij) = sqrt( xzi  * xzi  + yzi  * yzi  )
        yetac(ij) = sqrt( yeta * yeta + xeta * xeta )
        dux(ij) =   yeta
        duy(ij) = - yzi
        dvx(ij) = - xeta
        dvy(ij) =   xzi
        q12(ij) =   q12(ij) / ajb(ij)
        q21(ij) =   q12(ij)
10     continue
11    continue
C
C.... Change geometry terms if the grid describes an AXISYMMETRIC domain.
C
      if ( kplax .eq. 1 ) return
C
      do 21 j   = 2, jmax1
         ioff   = (j-1) * imaxl + ibeg
        ioff1   = ioff - imaxl
       do 20 i  = 2, imax1
            ij  = i + ioff
           ij1  = i + ioff1
        rav     = 0.25 * ( r(ij) + r(ij-1) + r(ij1) + r(ij1-1) )
        dux(ij) = dux(ij) * rav
        duy(ij) = duy(ij) * rav
        dvx(ij) = dvx(ij) * rav
        dvy(ij) = dvy(ij) * rav
        q12(ij) = q12(ij) * rav
        q21(ij) = q12(ij)
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine geomc2 (igrl)
C                                                                      C
C     Purpose:  Modify the Jacobian for axisymmetric systems.          C
C               This modification must NOT be done in Geomc1.          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      if (kplax .eq. 1) return
C
      do 11 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
         ioff1  = ioff - imaxl
       do 10 i  = 2, imax1
          ij    = i + ioff
          ij1   = i + ioff1
        rav     =  0.25 * ( r(ij) + r(ij-1) + r(ij1) + r(ij1-1) )
        ajb(ij) =  ajb(ij) * rav
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine geomm
C                                                                      C
C     Purpose:  Compute all transformation metrics and interpolation   C
C               factors.                                               C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      do 200 igr = 1,ngrid 
       call geomc1 (igr)
       call geomx  (igr)
       call geomy  (igr)
       call geomc2 (igr)
       call fxx    (igr)
       call fyy    (igr)
200   continue
C
      return
      end
C======================================================================C
      subroutine geomx (igrl)
C                                                                      C
C     Purpose:  Calculate geometry terms associated with the           C
C               transformation to general curvilinear coordinates.     C
C               Geometry terms stored at cell faces of constant zi     C
C               are calculated here.                                   C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 2, jmax1
         ioff   = (j-1) * imaxl + ibeg
        ioff1   = ioff - imaxl
       do 10 i  = 1, imax1
            ij  = i + ioff
           ij1  = i + ioff1
        xeta    = ( x(ij) - x(ij1) )
        yeta    = ( y(ij) - y(ij1) )
        a11(ij) =   yeta
        a12(ij) = - xeta
        q11(ij) = a11(ij) * a11(ij)  +  a12(ij) * a12(ij)
        q11(ij) = 2.0 * q11(ij) / ( ajb(ij) + ajb(ij+1) )
10     continue
11    continue
C
C.... Change geometry terms if the grid describes an AXISYMMETRIC domain.
C
      if ( kplax .eq. 1 ) return
C
      do 21 j   = 2, jmax1
         ioff   = (j-1) * imaxl + ibeg
        ioff1   = ioff - imaxl
       do 20 i  = 1, imax1
            ij  = i + ioff
           ij1  = i + ioff1
        rav     = 0.5 * ( r(ij) + r(ij1) )
        a11(ij) = a11(ij) * rav
        a12(ij) = a12(ij) * rav
        q11(ij) = q11(ij) * rav
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine geomy (igrl)
C                                                                      C
C     Purpose:  Calculate geometry terms associated with the           C
C               transformation to general curvilinear coordinates.     C
C               Geometry terms stored at cell faces of constant eta    C
C               are calculated here.                                   C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 1, jmax1
         ioff   = (j-1) * imaxl + ibeg
       do 10 i  = 2, imax1
            ij  = i + ioff
        xzi     = ( x(ij) - x(ij-1) )
        yzi     = ( y(ij) - y(ij-1) )
        a21(ij) = - yzi
        a22(ij) =   xzi
        q22(ij) = a21(ij) * a21(ij)  +  a22(ij) * a22(ij)
        q22(ij) = 2.0 * q22(ij) / ( ajb(ij) + ajb(ij+imaxl) )
10     continue
11    continue
C
C.... Change geometry terms if the grid describes an AXISYMMETRIC domain.
C
      if ( kplax .eq. 1 ) return
C
      do 21 j   = 1, jmax1
         ioff   = (j-1) * imaxl + ibeg
       do 20 i  = 2, imax1
            ij  = i + ioff
        rav     = 0.5 * ( r(ij) + r(ij-1) )
        a21(ij) = a21(ij) * rav
        a22(ij) = a22(ij) * rav
        q22(ij) = q22(ij) * rav
20     continue
21    continue
      return
      end
C======================================================================C
      subroutine grad (igrl,q)
C                                                                      C
C     Purpose:  Calculate the NEGATIVE of the zi and eta gradients     C
C               for the given variable.                                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
      include 'UIFlow.indx'
C
      do 12 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 11 i   = 2, imax1
            ij   = i + ioff
           ij1   = ij - 1
           ijm   = ij - imaxl
        dpdx(ij) = fx(ij1) * q(ij)   + fx1(ij1) * q(ij1)
     1           - fx(ij)  * q(ij+1) - fx1(ij)  * q(ij)
        dpdy(ij) = fy(ijm) * q(ij)   + fy1(ijm) * q(ijm)
     1           - fy1(ij) * q(ij)   - fy (ij)  * q(ij+imaxl)
11     continue
12    continue
C
      return
      end
C======================================================================C
      subroutine grid
C                                                                      C
C     Purpose:  Read in the x,y locations of the finest grid and then  C
C               calculate the coarser grids.                           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      imaxl  =  imax  (ngrid)
      jmaxl  =  jmax  (ngrid)
      ibeg   =  nbeg  (ngrid)
      imax1  =  imaxl - 1
      jmax1  =  jmaxl - 1
C
      do 10 i  = 1, imax1
       do 11 j = 1, jmax1
            ij = i + (j-1) * imaxl + ibeg
        read (11,*) x(ij), y(ij)
11     continue
10    continue
C
      do 20 i= 1, imax1
       do 21 j = 1, jmax1
           ij  = i + (j-1) * imaxl + ibeg
        r(ij)  = y(ij) + rin
21     continue
20    continue
C
C.... Calculate coarse grids based on the user prescribed finest grid.
C
      ngrid1 = ngrid - 1
      if (ngrid .eq. 1) return
      do 50 n = 1, ngrid1
         nrev    = ngrid - n
         imaxc   = imax (nrev)
         jmaxc   = jmax (nrev)
         imaxf   = imax (nrev + 1)
         jmaxf   = jmax (nrev + 1)
         imaxc1  = imaxc - 1
         jmaxc1  = jmaxc - 1
         ibegc   = nbeg (nrev)
         ibegf   = nbeg (nrev + 1)
       do 30 ic  = 1, imaxc1
        do 31 jc = 1, jmaxc1
             if  = 2 * ic - 1
	     jf  = 2 * jc - 1
            ijf  = if + (jf - 1) * imaxf + ibegf 
	    ijc  = ic + (jc - 1) * imaxc + ibegc 
         x(ijc)  = x(ijf)
         y(ijc)  = y(ijf)
         r(ijc)  = r(ijf)
31      continue
30     continue
50    continue
C
      return
      end
C======================================================================C
      subroutine gridp
C                                                                      C
C     Purpose:  Read in x,y locations of the finest grid and calculate C
C               the x,y locations for the coarser grids.  This routine C
C               is used in conjunction with a grid generated by the    C
C               pre-processor.                                         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      imaxl  =  imax  (1)
      jmaxl  =  jmax  (1)
      ibeg   =  nbeg  (1)
      imax1  =  imaxl - 1
      jmax1  =  jmaxl - 1
C
      do 10 i  = 1, imax1
       do 11 j = 1, jmax1
           ij  = i + (j-1) * imaxl + ibeg
        read (11,*) x(ij), y(ij)
11     continue
10    continue
C
      do 20 i  = 1, imax1
       do 21 j = 1, jmax1
           ij  = i + (j-1) * imaxl + ibeg
        r(ij)  = y(ij) + rin
21     continue
20    continue
C
C.... Calculate coarse grids based on the user prescribed finest grid.
C
      if (ngrid .eq. 1) return
      do 50 n = 2, ngrid
       imaxc  = imax (n-1)
       jmaxc  = jmax (n-1)
       imaxf  = imax (n)
       jmaxf  = jmax (n)
       imaxc1 = imaxc - 1
       jmaxc1 = jmaxc - 1
       ibegc  = nbeg (n-1)
       ibegf  = nbeg (n)
       do 30 ic  = 1, imaxc1
        do 31 jc = 1, jmaxc1
             if  = 2 * ic - 1
	     jf  = 2 * jc - 1
            ijf  = if + (jf - 1) * imaxf + ibegf
	    ijc  = ic + (jc - 1) * imaxc + ibegc
C
         x(ijf)   = x(ijc)
         y(ijf)   = y(ijc)
         r(ijf)   = r(ijc)
C
         x(ijf+1) = 0.5*( x(ijc) + x(ijc+1) )
         y(ijf+1) = 0.5*( y(ijc) + y(ijc+1) )
         r(ijf+1) = 0.5*( r(ijc) + r(ijc+1) )
C
         x(ijf+imaxf) = 0.5*( x(ijc) + x(ijc+imaxc) )
         y(ijf+imaxf) = 0.5*( y(ijc) + y(ijc+imaxc) )
         r(ijf+imaxf) = 0.5*( r(ijc) + r(ijc+imaxc) )
C
         x(ijf+imaxf+1) = 0.25*( x(ijc)+x(ijc+1)+x(ijc+imaxc)+
     1                           x(ijc+1+imaxc) )
         y(ijf+imaxf+1) = 0.25*( y(ijc)+y(ijc+1)+y(ijc+imaxc)+
     1                           y(ijc+1+imaxc) )
         r(ijf+imaxf+1) = 0.25*( r(ijc)+r(ijc+1)+r(ijc+imaxc)+
     1                           r(ijc+1+imaxc) )
31      continue
30     continue
50    continue
C
      return
      end
C======================================================================C
      subroutine header
C                                                                      C
C     Purpose:  Print a title for program output.                      C
C                                                                      C
C======================================================================C
       write(8,*) 
       write(8,*) ('*',i = 1,86)
       write(8,*) ('*',i = 1,86)
       do 10 j = 1,22
         write(8,*) '**',(' ',i = 1,82),'**'
 10    continue
       write(8,*) '**',(' ',i=1,34),'UIFLOW - 2D',(' ',i=1,34),'**'
       write(8,*) '**',(' ',i= 1,82),'**' 
       write(8,*) '**',(' ',i=1,35),'VERSION 2.0 ',(' ',i=1,35),'**' 
       write(8,*) '**',(' ',i= 1,82),'**' 
       write(8,*) '**',(' ',i= 1,82),'**' 
       write(8,*) '**',(' ',i= 1,82),'**' 
       write(8,*) '**',(' ',i=1,19),'UNIVERSITY of ILLINOIS at
     1 URBANA - CHAMPAIGN',(' ',i=1,19),'**'
       write(8,*) '**',(' ',i= 1,82),'**' 
       write(8,*) '**',(' ',i=1,15),'DEPARTMENT of MECHANICAL and 
     1 INDUSTRIAL ENGINEERING',(' ',i=1,15),'**'
       write(8,*) '**',(' ',i= 1,82),'**'
       write(8,*) '**',(' ',i= 1,82),'**'
       write(8,*) '**',(' ',i=1,29),'CONTACT - Dr. S.P. Vanka',
     1 (' ',i=1,29),'**'
       write(8,*) '**',(' ',i=1,33),'(217) 244 - 8388',
     1 (' ',i=1,33),'**' 
       do 20 j = 1,23
         write(8,*) '**',(' ',i= 1,82),'**'
 20    continue
       write(8,*) ('*',i = 1,86)
       write(8,*) ('*',i = 1,86)
       write(8,*)
       return
       end
C======================================================================C
      subroutine init
C                                                                      C
C     Purpose:  Specify initial estimate of flowfield variables on     C
C               all grids.  Also specify inlet conditions on all       C
C               grids.                                                 C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
C.... Specify inlet conditions and initial estimates on the finest grid.
C
      call  intern
      call  lbndry
      call  rbndry
      call  tbndry
      call  bbndry
      call  mssin
C
      call  cpenth (ngrid)
      call  enth   (ngrid)
C
C.... Restrict flow variables to coarser grids.
C
      if( ngrid .gt. 1 ) then
       ngrid1   = ngrid - 1
       do 10 nr  = 1, ngrid1
             igr = ngrid - nr + 1 
        call restv (igr)
        call restfl(igr)
C
        if (klam .eq. 0) then
         call rests (igr, tke)
         call rests (igr, tde)
        endif
C
        if (model.gt. 0) then
         call rests (igr, h)
         call rests (igr, t)
         call rests (igr, wmol)
        endif
C
        if (model .eq. 2) then
         call rests (igr, f)
         call rests (igr, yfu)
         call rests (igr, yo2)
         call rests (igr, yco2)
         call rests (igr, yh2o)
         call rests (igr, yn2)
        endif
C
        if (model .eq. 3) then
         call rests (igr, f)
         if (klam .eq. 0) call rests (igr, g)
         call rests (igr, yfu)
         call rests (igr, yo2)
         call rests (igr, yco2)
        endif
C
        call extrpl(igr-1)
10     continue
      endif
      return
      end
C======================================================================C
      subroutine input
C                                                                      C
C     Purpose:  Read in all data pertinent to describing the flow      C
C               to be solved.                                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
C.... Read general problem data.
C
      read (5, *) klam , kcomp , kswrl, kpgrid, model
      read (5, *) kfuel, knorth, kplax, kadj
      read (5, *) ngrid, ncelx, ncely
C
C.... Read segment data.
C
      read (5, *) nsxm
      do 10 n = 1,nsxm
       read (5, *) kbxm(n), jfc, jlc
       jfxm (n, ngrid) = jfc + 1 
       jlxm (n, ngrid) = jlc + 1 
       read (5, *) ubxm(n), vbxm(n) , wbxm(n) , dvar   , txm(n)  
       read (5, *) rhxm(n), fxm(n)  , gxm(n)  , tkxm(n), tdxm(n) 
       read (5, *) fuxm(n), co2xm(n), h2oxm(n), o2xm(n), wmxm(n)
10    continue
C
      read (5, *) nsxp
      do 20 n = 1,nsxp
       read (5, *) kbxp (n), jfc, jlc
       jfxp (n, ngrid) = jfc + 1 
       jlxp (n, ngrid) = jlc + 1 
       read (5, *) ubxp(n), vbxp(n) , wbxp(n) , dvar   , txp(n)	   
       read (5, *) rhxp(n), fxp(n)  , gxp(n)  , tkxp(n), tdxp(n)
       read (5, *) fuxp(n), co2xp(n), h2oxp(n), o2xp(n), wmxp(n)
20    continue
C
      read (5, *) nsym     
      do 30 n = 1,nsym
       read (5, *) kbym (n), ifc, ilc
       ifym (n, ngrid) = ifc + 1 
       ilym (n, ngrid) = ilc + 1 
       read (5, *) ubym(n), vbym(n) , wbym(n) , dvar   , tym(n)
       read (5, *) rhym(n), fym(n)  , gym(n)  , tkym(n), tdym(n)
       read (5, *) fuym(n), co2ym(n), h2oym(n), o2ym(n), wmym(n)
30    continue
C
      read (5, *) nsyp
      do 40 n = 1,nsyp
       read (5, *) kbyp(n), ifc, ilc
       ifyp (n, ngrid) = ifc + 1 
       ilyp (n, ngrid) = ilc + 1 
       read (5, *) ubyp(n), vbyp(n) , wbyp(n) , dvar   , typ(n)
       read (5, *) rhyp(n), fyp(n)  , gyp(n)  , tkyp(n), tdyp(n)
       read (5, *) fuyp(n), co2yp(n), h2oyp(n), o2yp(n), wmyp(n)
40    continue
C
      read (5, *) ugs, vgs, wgs, rhgs
      read (5, *) tgs, tkgs, tdgs, fgs, ggs, fugs
      read (5, *) tfuel, tair
      read (5, *) (prl(nv)  , nv=1, 11)
      read (5, *) (prt(nv)  , nv=1, 11)
      read (5, *) (relx(nv) , nv=1, 11)
      read (5, *) (nswp(nv) , nv=1, 11)
      read (5, *) (iprint(i), i=1 , 12)
      read (5, *) pref, vscty
      read (5, *) maxitn, tolr(ngrid)
      read (5, *) rin
      read (5, *) nxbaf, nybaf, nobs
      relx(2) =  relx(1)
C
C      call xwcommon
1001  format (80a1)
C
      return
      end
C======================================================================C
      subroutine intern
C                                                                      C
C     Purpose:  Initialize internal values of flow variables.          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      if(model .gt. 0) rhgs = pref * wmair / ( gascon * tgs )
C
      do 11 i     =  2, imax1
       do 10 j    =  2, jmax1
           ijf    =  i + (j-1) * imaxl + ibeg
        u(ijf)    =  ugs
        v(ijf)    =  vgs
        w(ijf)    =  wgs
        p(ijf)    =  0.0
        t(ijf)    =  tgs
        gam(ijf)  =  vscty
        amu(ijf)  =  vscty
        amut(ijf) =  cd * rhgs * tkgs * tkgs / ( tdgs + 1.0e-30 )
        rho(ijf)  =  rhgs
        tke(ijf)  =  tkgs
        tde(ijf)  =  tdgs
        f  (ijf)  =  fgs
        g  (ijf)  =  ggs
        yfu(ijf)  =  fugs
        yco2(ijf) =  co2gs
        yh2o(ijf) =  h2ogs
        yo2(ijf)  =  o2gs
        yn2(ijf)  =  1.0 - fugs - co2gs - h2ogs - o2gs
        wmol(ijf) =  wmair
        cu(ijf)   =  a11(ijf) * u(ijf) + a12(ijf) * v(ijf)
        cv(ijf)   =  a21(ijf) * u(ijf) + a22(ijf) * v(ijf)
        cx(ijf)   =  rho(ijf) * cu(ijf)
        cy(ijf)   =  rho(ijf) * cv(ijf)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine lamvis (igrl)
C                                                                      C
C     Purpose:  Compute the laminar viscosity based on the             C
C               Sutherland's law constants for AIR only.               C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 10 i  = 2, imax1
         ij     = i + ioff
        amu(ij) = airv * ( t(ij) / airt ) ** 1.5 * ( airsum ) /
     1                   ( t(ij) + airs )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine lbndry
C                                                                      C
C     Purpose:  Prescribe boundary conditions at the zi-minus (left)   C
C               boundary of the calculation domain.                    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      i = 1
      do 11 n = 1, nsxm
C
       wmxm(n) = wmair
       if (model .eq. 0) rhxm(n) = rhgs
       if (model .eq. 1) rhxm(n) = pref * wmair / ( gascon * txm(n) )
       if (model .eq. 2) then
        yn2xm   = 1.0 - fuxm(n)  - o2xm(n) - h2oxm(n) - co2xm(n)
        rwmxm   = fuxm(n)  / wmfu  + o2xm(n)  / wmox + yn2xm / wmn2
     1          + co2xm(n) / wmco2 + h2oxm(n) / wmh2o
        wmxm(n) = 1.0 / rwmxm
        rhxm(n) = pref * wmxm(n) / (gascon * txm(n) + 1.0e-30)
       endif
C
          jfl  = jfxm(n,ngrid)
	  jll  = jlxm(n,ngrid)
       do 10 j = jfl,jll
         ijf   = i + (j-1) * imaxl + ibeg
        rav        = 0.5 * ( r(ijf) + r(ijf-imaxl) )
        u   (ijf)  = ubxm(n)
        v   (ijf)  = vbxm(n)
        w   (ijf)  = wbxm(n)
        p   (ijf)  = 0.0
        t   (ijf)  = txm (n)
        tke (ijf)  = tkxm(n)
        tde (ijf)  = tdxm(n)
        f   (ijf)  = fxm(n)
        g   (ijf)  = gxm(n)
        yfu (ijf)  = fuxm(n)
        yo2 (ijf)  = o2xm(n)
        yh2o(ijf)  = h2oxm(n)
        yco2(ijf)  = co2xm(n)
        yn2 (ijf)  = yn2xm
        gam (ijf)  = vscty
        amu (ijf)  = vscty
        amut(ijf)  = cd * rhxm(n) * tkxm(n) * tkxm(n)
     1               / ( tdxm(n) + 1.0e-30 )
        wmol(ijf)  = wmxm(n)
        rho (ijf)  = rhxm(n)
        cu  (ijf)  = a11(ijf)*u(ijf) + a12(ijf)*v(ijf)
        cx  (ijf)  = cu (ijf)*rho(ijf)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine masfrc (igrl)
C                                                                      C
C     Purpose:  Calculate the mass fractions of species based on law   C
C               of conservation of atoms.                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      rwmfu  = 1.0 / wmfu
      rwmox  = 1.0 / wmox
      rwmco2 = 1.0 / wmco2
      rwmh2o = 1.0 / wmh2o
      rwmn2  = 1.0 / wmn2
C
      do 11 j    = 2, jmax1
          ioff   = (j-1) * imaxl + ibeg
       do 10 i   = 2, imax1
           ij    = i + ioff
        fmyfu    = f(ij) - yfu(ij)
        yo2(ij)  = rox * ( 1.0 - f(ij) ) - rat(3) * fmyfu
        yco2(ij) = rat(1) * fmyfu
        yh2o(ij) = rat(2) * fmyfu
        yo2(ij)  = amax1 ( yo2(ij) , 1.0e-20 )
        yco2(ij) = amax1 ( yco2(ij), 0.0 )
        yh2o(ij) = amax1 ( yh2o(ij), 0.0 )
        yn2(ij)  = 1.0 - yfu(ij) - yo2(ij) - yco2(ij) - yh2o(ij)
        rwmol    = yfu(ij)  * rwmfu   +  yo2(ij)  * rwmox
     1           + yco2(ij) * rwmco2  +  yh2o(ij) * rwmh2o
     2           + yn2(ij)  * rwmn2
        wmol(ij) = 1.0 / rwmol
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine moment (igrf)
C                                                                      C
C     Purpose:  Perform multi-grid V cycle on continuity and           C
C               momentum equations.                                    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrl  = igrf
      upleg = .false.
      niter = 0
C
C.... Calculate diffusive exchange coefficient.
C
      call  visc (igrl, 1)
      if ( klam .eq. 0 ) call gammod(igrf)
C
10    continue
      niter = niter + 1
      call  dflux (igrl)
      call  coeff (igrl)
C
C.... Solve u momentum equation.
C
      call  trsrc  (igrl, u)
      call  srcu   (igrl)
      call  urelax (igrl, 1, u)
      call  resid  (igrl, u, error(igrl,1))
      call  solve  (igrl, 1, u)
C
C.... Solve v momentum equation.
C
      call  trsrc  (igrl, v)
      call  srcv   (igrl)
      call  urelax (igrl, 2, v)
      call  resid  (igrl, v, error(igrl,2))
      call  solve  (igrl, 2, v)
C
C.... Calculate cell face mass fluxes.
C
      if (igrl .eq. igrf) then
       call apcalc (igrl)
       call cflux  (igrl)
      else
       call cfluxc (igrl)
      endif
C
C.... Solve pressure correction equation and update velocities and
C     pressure.
C
      call  coeffp (igrl)
      call  mresid (igrl)
      call  solve  (igrl, 3, pp)
      call  update (igrf, igrl)
C
      if ( kadj .eq. 1 ) then
       call adjstx (igrl)
      else
       call mssout (igrl)
      endif
C
      if (igrl .eq. igrf) call extrpl (igrl)
C
      ndif   =  ngrid - igrl
      wrkunt = wrkunt + 1.0 / (4 ** ndif)
C
C.... Decide next action to be performed in V-cycle.
C
      if(error(igrf,3) .le. tolr(igrf)) return
      call next (igrf, igrl)
      if (prolng)  go to 20
      if (restrc)  go to 30
      if (relax)   go to 10
      return
  20  continue
C
C.... Prolongate corrections to finer grid.
C
      if(igrl .eq. igrf) return
      call  prols (igrl, p, 1)
      call  prols (igrl, u, 0)
      call  prols (igrl, v, 0)
      call  prolcx(igrl)
      call  prolcu(igrl)
      call  prolcy(igrl)
      call  prolcv(igrl)
      igrl  = igrl + 1
      call  bcor  (igrl,p)
      if(igrl .eq. igrf) return
      niter = 0
      go to 10
C
C.... Restrict solution and residuals to coarser grid.
C
  30  continue
      call  restv (igrl)
      call  restfl(igrl)
      call  bcor  (igrl-1,p)
      call  restr (igrl)
      igrl  = igrl - 1
      niter = 0
      go to 10
C
      end
C======================================================================C
      subroutine mresid(igrl)
C                                                                      C
C     Purpose:  Determine coefficients and source term for the         C
C               pressure correction equation.                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Initialize pressure corrections to zero.
C
      do 11 j  = 1, jmaxl
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 1, imaxl
           ij  = i + ioff
        pp(ij) = 0.0
10     continue
11    continue
C
      error(igrl,3) = 0.0
C
      do 21 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 20 i  = 2, imax1
            ij  = i + ioff
        su(ij)  = cx(ij-1) - cx(ij) - cy(ij) + cy(ij-imaxl)
        sp(ij)  = 0.0
        aw(ij)  = ae(ij-1)
        as(ij)  = an(ij-imaxl)
        ap(ij)  = ae(ij) + aw(ij) + an(ij) + as(ij)
        error(igrl,3) = error(igrl,3) + abs( su(ij) )
20     continue
21    continue
C
      write (8,*) error(igrl,3),'     ',nitn(igrl),'     ',refc
      error(igrl, 3) = error(igrl,3) / refc
C
      if ( nitn(igrl) .eq. 1 ) then
       refc = error(igrl, 3)
       error(igrl, 3) = error(igrl,3) / refc
      endif
C
C.... Modify coefficients near boundaries.
C
      call cfpmod (igrl)
C
      return
      end
C======================================================================C
      subroutine mssin
C                                                                      C
C     Purpose:  Calculate the overall mass flow rate into the flow     C
C               domain.                                                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      flxin = 0.0
C
      do 100 n = 1, nsxm
       if ( kbxm(n) .eq. 2 ) then
        i = 1
        do 10 j = jfxm(n,ngrid), jlxm(n,ngrid)
            ij  = i + ibeg + (j-1) * imaxl
         flxin  = flxin + cx(ij)
10      continue
       endif
100   continue
C
      do 200 n = 1, nsxp
       if ( kbxp(n) .eq. 2 ) then
        i = imax1
        do 20 j = jfxp(n,ngrid), jlxp(n,ngrid)
            ij  = i + ibeg + (j-1) * imaxl
         flxin  = flxin + cx(ij)
20      continue
       endif
200   continue
C
      do 300 n = 1, nsym
       if ( kbym(n) .eq. 2 ) then
        j = 1
        do 30 i = ifym(n,ngrid), ilym(n,ngrid)
            ij  = i + ibeg + (j-1) * imaxl
         flxin  = flxin + cy(ij)
30      continue
       endif
300   continue
C
      do 400 n = 1, nsyp
       if ( kbyp(n) .eq. 2 ) then
        j = jmax1
        do 40 i = ifyp(n,ngrid), ilyp(n,ngrid)
            ij  = i + ibeg + (j-1) * imaxl
         flxin  = flxin + cy(ij)
40      continue
       endif
400   continue
C
      return
      end
C======================================================================C
      subroutine mssout(igrl)
C                                                                      C
C     Purpose:  Ensure that integral mass flow out of the domain is    C
C               equal to the influx of mass.                           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      floact = 1.0e-30
      flxrto = 0.0
C
      i = imax1
      do 20 j = 2, jmax1
           ij = i + ibeg + (j-1) * imaxl
       floact = floact + abs( cx(ij) )
20    continue
C
      flxrto  = flxin / floact
C
      do 21 j = 2, jmax1
           ij = i + ibeg + (j-1) * imaxl
       cx(ij) = cx(ij) * flxrto
21    continue
C
      return
      end
C======================================================================C
      subroutine next (igrf, igrl)
C                                                                      C
C     Purpose:  Determine the next step to be performed                C
C               in the V-cycle.                                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      prolng  =  .false.
      restrc  =  .false.
      relax   =  .false.
      niterv  =  nitr(igrl)
C
      if (niter .lt. niterv) then
       relax = .true.
       return
      else
       if(igrf .eq. 1) return
       if(igrl .eq. 1) upleg = .true.
       if (upleg) prolng = .true.
       if (.not. upleg) restrc = .true.
      endif
C
      return
      end
C======================================================================C
      subroutine onestp(igrl)
C                                                                      C
C     Purpose:  Calculate all quantities pertinent to the simulation   C
C               of a turbulent premixed flame.                         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      call scalar (igrl, 8, f  )
      call scalar (igrl, 9, yfu)
      call masfrc (igrl)
      call trap   (igrl)
      call scalar (igrl, 5, h  )
      call props  (igrl)
C
      return
      end
C======================================================================C
      subroutine outp
C                                                                      C
C     Purpose:  Write out input information to serve as an echo check. C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
C              Problem description
C
      write (8,1002) klam, kcomp, knorth, kplax, kswrl, kvisc, model, 
     1               ksc,  kadj
 1002 format(5x, 'Laminar Flow index (1=Laminar, 0=Turbulent) = ', i5/
     1       5x, 'Compressible Flow Index (1=Compressible )   = ', i5/
     2       5x, 'Non-Orthogonal Coordinate index (0 = Orth)  = ', i5/
     3       5x, 'Planar/Axisymm index (1 = planar)           = ', i5/
     4       5x, 'Swirl flow index (0 = no swirl)             = ', i5/
     5       5x, 'Viscosity model(0=fixed laminar viscosity)  = ', i5/
     6       5x, 'Density model                               = ', i5/
     7       5x, 'Solve for scalar variable (0 = do not solve)= ', i5/
     5       5x, 'Block-correction Index(0=no corrections)    = ', i5/)
C
C              Grid data
C
      write (8,1003) ngrid, ncelx, ncely
 1003 format(5x, 'Number of Levels of grids                   = ', i5/
     1       5x, 'Number of Cells (finest grid) in x-dirn     = ', i5/
     2       5x, 'Number of Cells (finest grid) in y-dirn     = ', i5/)
C
C            Segment data
C
      write (8,1004) nsxm, nsxp, nsym, nsyp
 1004 format(5x, 'Number of Segments on x-minus Boundary      = ', i5/
     1       5x, 'Number of Segments on x-plus  Boundary      = ', i5/
     2       5x, 'Number of Segments on y-minus Boundary      = ', i5/
     3       5x, 'Number of Segments on y-plus  Boundary      = ', i5/)
C
C            Flow variables Data
C
      do 10 n = 1, nsxm
      write (8,1005) n, jfxm(n,ngrid), jlxm(n,ngrid), kbxm(n)
      write (8,1006) ubxm(n), vbxm(n), wbxm(n), tkxm(n), tdxm(n), 
     1               rhxm(n), fxm (n), gxm (n), fuxm(n), txm (n), 
     2               sxm (n)
  10  continue
 1005 format (5x,'Segment Number = ', i5/
     1        5x,'  jfxm = ',i3,'  jlxm = ',i3/
     3        5x,'Boundary Index       = ', i5/)
 1006 format (5x,'u velocity           = ', e12.3/
     1        5x,'v velocity           = ', e12.3/
     2        5x,'w velocity           = ', e12.3/
     3        5x,'turb. kinetic energy = ', e12.3/
     4        5x,'turb dissipation rate= ', e12.3/
     5        5x,'density              = ', e12.3/
     6        5x,'mixture fraction     = ', e12.3/
     7        5x,'conc. fluctuation    = ', e12.3/
     8        5x,'fuel fraction        = ', e12.3/
     9        5x,'temperature          = ', e12.3/
     9        5x,'general scalar       = ', e12.3/)
C
      do 20 n = 1, nsxp
      write (8,1007) n, jfxp(n,ngrid), jlxp(n,ngrid), kbxp(n)
      write (8,1006) ubxp(n), vbxp(n), wbxp(n), tkxp(n), tdxp(n), 
     1               rhxp(n), fxp (n), gxp (n), fuxp(n), txp (n), 
     2               sxp (n)
  20  continue
 1007 format (5x,'Segment Number = ', i5/
     1        5x,'  jfxp = ',i3,'  jlxp = ',i3/
     3        5x,'Boundary Index       = ', i5/)
C
      do 30 n = 1, nsym
      write (8,1008) n, ifym(n,ngrid), ilym(n,ngrid), kbym(n)
      write (8,1006) ubym(n), vbym(n), wbym(n), tkym(n), tdym(n), 
     1               rhym(n), fym (n), gym (n), fuym(n), tym (n), 
     2               sym (n)
  30  continue
 1008 format (5x,'Segment Number = ', i5/
     1        5x,'  ifym = ',i3,'  ilym = ',i3/
     3        5x,'Boundary Index       = ', i5/)
C
      do 40 n = 1, nsyp
      write (8,1009) n, ifyp(n,ngrid), ilyp(n,ngrid), kbyp(n)
      write (8,1006) ubyp(n), vbyp(n), wbyp(n), tkyp(n), tdyp(n), 
     1               rhyp(n), fyp (n), gyp (n), fuyp(n), typ (n), 
     2               syp (n)
  40  continue
 1009 format (5x,'Segment Number = ', i5/
     1        5x,'  ifyp = ',i3,'  ilyp = ',i3/
     3        5x,'Boundary Index       = ', i5/)
C
C                    Guessed Values
C
      write (8,1012)
 1012 format(5x, 'Guessed Values'/5x,15(1h-)/)
      write (8,1006) ugs, vgs, wgs, tkgs, tdgs, rhgs, fgs, ggs, 
     1              fugs, tgs,sgs
C
C                 Print and Solution Parameters
C
      write (8,1013) (iprint(i), i=1,18)
 1013 format (5x, 'iprint array =', 18i5/)
      write (8,1014) (relx(n), n=1,11)
 1014 format (5x, 'relax factor array =', 1p11e8.1/)
      write (8,1015) (nswp(n), n=1,13)
 1015 format (5x, 'number of sweeps   =', 13i5/)
      write (8,1016) (prl(n), n=1,13)
      write (8,1017) (prt(n), n=1,13)
 1016 format (5x, 'Laminar Prandtl numbers =', 13f6.1)
 1017 format (5x, 'Turb.Prandtl numbers    =', 13f6.1)
      write (8,1018) (tolr(n), n=1,ngrid)
 1018 format (5x, 'error tolerances', 5e8.3)
C
C                    Reference values
C
      write(8,1019) refu, refv, refw, refc
 1019 format (5x, 'Reference values '/
     1        5x, 'u-velocity             =', e12.3/
     2        5x, 'v-velocity             =', e12.3/
     3        5x, 'w-velocity             =', e12.3/     
     4        5x, 'pressure               =', e12.3/)
       return
       end
C======================================================================C
      subroutine output(igrl)
C                                                                      C
C     Purpose:  Print out flow variables after the calculation has     C
C               terminated.                                            C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      imaxl  =  imax (igrl)
      jmaxl  =  jmax (igrl)
      ibeg   =  nbeg (igrl)
C
      if (iprint(1).eq.1) write (8, 1001)
      if (iprint(1).eq.1) call  plane (imaxl, jmaxl,ibeg, u)
C
      if (iprint(2).eq.1) write (8, 1002)
      if (iprint(2).eq.1) call  plane (imaxl, jmaxl,ibeg, v)
C
      if (iprint(3).eq.1) write (8, 1003)
      if (iprint(3).eq.1) call  plane (imaxl, jmaxl,ibeg, p)
C
      if (iprint(4).eq.1) write (8, 1004)
      if (iprint(4).eq.1) call  plane (imaxl, jmaxl,ibeg, w)
C
      if (iprint(5).eq.1) write (8, 1005)
      if (iprint(5).eq.1) call  plane (imaxl, jmaxl,ibeg, h)
C
      if (iprint(5).eq.1) write (8, 1012)
      if (iprint(5).eq.1) call  plane (imaxl, jmaxl,ibeg, t)
C
      if (iprint(6).eq.1) write (8, 1006)
      if (iprint(6).eq.1) call  plane (imaxl, jmaxl,ibeg, tke)
C
      if (iprint(6).eq.1) write (8, 1013)
      if (iprint(6).eq.1) call  plane (imaxl, jmaxl,ibeg, amut)
C
      if (iprint(7).eq.1) write (8, 1007)
      if (iprint(7).eq.1) call  plane (imaxl, jmaxl,ibeg, tde)
C
      if (iprint(8).eq.1) write (8, 1008)
      if (iprint(8).eq.1) call  plane (imaxl, jmaxl,ibeg, f)
C
      if (iprint(9).eq.1) write (8, 1009)
      if (iprint(9).eq.1) call  plane (imaxl, jmaxl,ibeg, yfu)
C
      if (iprint(10).eq.1) write (8, 1010)
      if (iprint(10).eq.1) call  plane (imaxl, jmaxl,ibeg, g)
C
      if (iprint(11).eq.1) write (8, 1011)
      if (iprint(11).eq.1) call  plane (imaxl, jmaxl,ibeg, rho)
C
      if (iprint(12).eq.1) write (8, 1014)
      if (iprint(12).eq.1) call  plane (imaxl, jmaxl,ibeg, yco2)
C
      if (iprint(12).eq.1) write (8, 1015)
      if (iprint(12).eq.1) call  plane (imaxl, jmaxl,ibeg, yh2o)
C
      if (iprint(12).eq.1) write (8, 1016)
      if (iprint(12).eq.1) call  plane (imaxl, jmaxl,ibeg, yo2)
C
      if (iprint(12).eq.1) write (8, 1017)
      if (iprint(12).eq.1) call  plane (imaxl, jmaxl,ibeg, yn2)
C
1001  format (// 5x, 20(1H-), '    u - Velocity   ', 20(1H-)/)
1002  format (// 5x, 20(1H-), '    v - Velocity   ', 20(1H-)/)
1003  format (// 5x, 20(1H-), '      Pressure     ', 20(1H-)/)
1004  format (// 5x, 20(1H-), '    w - Velocity   ', 20(1H-)/)
1005  format (// 5x, 20(1H-), '      Enthalpy     ', 20(1H-)/)
1006  format (// 5x, 20(1H-), '    Kinetic Energy ', 20(1H-)/)
1007  format (// 5x, 20(1H-), '     Dissipation   ', 20(1H-)/)
1008  format (// 5x, 20(1H-), '    Mix.Fraction   ', 20(1H-)/)
1009  format (// 5x, 20(1H-), '     Fuel Frac.    ', 20(1H-)/)
1010  format (// 5x, 20(1H-), ' Conc. Fluctuation ', 20(1H-)/)
1011  format (// 5x, 20(1H-), '      Density      ', 20(1H-)/)
1012  format (// 5x, 20(1H-), '     Temperature   ', 20(1H-)/)
1013  format (// 5x, 20(1H-), '     Turb. Visc.   ', 20(1H-)/)
1014  format (// 5x, 20(1H-), '        CO2        ', 20(1H-)/)
1015  format (// 5x, 20(1H-), '        H2O        ', 20(1H-)/)
1016  format (// 5x, 20(1H-), '      Oxygen       ', 20(1H-)/)
1017  format (// 5x, 20(1H-), '     Nitrogen      ', 20(1H-)/)
C
      return
      end
C======================================================================C
      subroutine pcoefc (igrl)
C                                                                      C
C     Purpose:  Add terms to the pressure correction equation which    C
C               account for density fluctuations.  This is only        C
C               called if the flow is compressible.                    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
        ioff   = (j-1) * imaxl + ibeg
        ioffm  = ioff - imaxl
        ioffp  = ioff + imaxl
       do 10 i = 2, imax1
          ij   = i + ioff
          ijm  = i + ioffm
          ijp  = i + ioffp
          ij1  = ij - 1
          ijp1 = ij + 1
        ae(ij) = ae(ij) + amax1( -cu(ij), 0. ) * wmol(ijp1)
     1                  / ( gamma * gascon  * t(ijp1) )
        aw(ij) = aw(ij) + amax1( cu(ij1), 0. )* wmol(ij1)
     1                  / ( gamma * gascon * t(ij-1) )
        an(ij) = an(ij) + amax1( -cv(ij), 0. )* wmol(ijp)
     1                  / ( gamma * gascon * t(ijp) )
        as(ij) = as(ij) + amax1( cv(ijm), 0. )* wmol(ijm)
     1                  / ( gamma * gascon * t(ijm) )
        ap(ij) = ap(ij) + ( amax1( cu(ij), 0.) + amax1( -cu(ij1), 0.)
     1                  +   amax1( cv(ij), 0.) + amax1( -cv(ijm), 0.) )
     2                  *   wmol(ij)/( gamma * gascon * t(ij) )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine pcoefo (igrl)
C                                                                      C
C     Purpose:  Add terms to the pressure correction source term       C
C               which account for use of a highly NON-ORTHOGONAL grid. C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      call bcor   (igrl, pp)
      call grad   (igrl, pp)
      call cpgrad (igrl)
C
      do 10 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg 
       do 11 i = 2, imax1
          ij   = i + ioff
        sp(ij) = x1(ij-1) - x1(ij) + x2(ij-imaxl) - x2(ij)
11     continue
10    continue
C
      return
      end
C======================================================================C
      subroutine plane (idim, jdim, nbg, varl)
C                                                                      C
C     Purpose:  Print out a given field variable.                      C
C                                                                      C
C======================================================================C
      dimension varl (*)
      ip = 1
      jp = 1
      istp = ip * 8
C
      do 10 ilo = 1,idim,istp
            ihi = min0 (ilo+istp-1, idim)
       write(8,1001)
       do 20 jj = 1,jdim,jp
              j = jdim - jj + 1
            ijl = ilo + (j-1) * idim + nbg  
            ijh = ihi + (j-1) * idim + nbg
        write(8, 1002) j, (varl(ij), ij = ijl,ijh,ip)
20     continue
10    continue
C
1001  format(4x, 'j')
1002  format (2x, i3, 3x, 1p8e10.3)
      return
      end
C======================================================================C
      subroutine prlong (igrf)
C                                                                      C
C     Purpose:  Call subroutines to prolongate variables from one      C
C               fine grid to the next.  Used in the nested iteration   C
C               phase of the FMG cycle.                                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      call  prolcx (igrf)
      call  prolcy (igrf)
      call  prolcu (igrf)
      call  prolcv (igrf)
      call  prols  (igrf, u, 0)
      call  prols  (igrf, v, 0)
      call  prols  (igrf, p, 1)
C
      if (klam .eq. 0) then
       call  prols (igrf, gam,  0)
       call  prols (igrf, tke,  0)
       call  prols (igrf, tde,  0)
       call  prols (igrf, amut, 0)
       if (model .eq. 3) then
        call prols (igrf, g, 0)
       endif
      endif
C
      if (kswrl.eq. 1) call  prols (igrf, w, 0)
C
      if (model .gt. 0) call  prols (igrf, h,    0)
      if (model .gt. 0) call  prols (igrf, t,    0)
      if (model .gt. 0) call  prols (igrf, rho,  0)
      if (model .gt. 1) call  prols (igrf, f,    0)
      if (model .gt. 1) call  prols (igrf, yfu,  0)
      if (model .gt. 1) call  prols (igrf, yo2,  0)
      if (model .gt. 1) call  prols (igrf, yco2, 0)
      if (model .gt. 1) call  prols (igrf, wmol, 0)
      if (model .eq. 2) call  prols (igrf, yn2,  0)
      if (model .eq. 2) call  prols (igrf, yh2o, 0)
C
      return
      end
C======================================================================C
      subroutine prodn (igrl)
C                                                                      C
C     Purpose:  Calculate the production of turbulence kinetic energy. C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
        ioffm  = ioff - imaxl
        ioffp  = ioff + imaxl
       do 10 i = 2, imax1
            ij = i + ioff
           imj = ij - 1
           ipj = ij + 1
           ijm = i + ioffm
           ijp = i + ioffp
C
C.... Compute derivatives in the zi and eta system.
C
        dudzi  = fx(ij) * u(ipj) + fx1(ij) * u(ij) - fx(imj) * u(ij)
     1         - fx1(imj) * u(imj)
        dvdzi  = fx(ij) * v(ipj) + fx1(ij) * v(ij) - fx(imj) * v(ij)
     1         - fx1(imj) * v(imj)
        dudet  = fy(ij) * u(ijp) + fy1(ij) * u(ij) - fy(ijm)
     1         * u(ij) - fy1(ijm) * u(ijm)
        dvdet  = fy(ij) * v(ijp) + fy1(ij) * v(ij) - fy(ijm)
     1         * v(ij) - fy1(ijm) * v(ijm)
C
C.... Compute derivatives in the x and y system.
C
        rajb   = 1.0 / ajb(ij)
C
        dudx   = ( dudzi * dux(ij) + dudet * duy(ij) ) * rajb
        dvdx   = ( dvdzi * dux(ij) + dvdet * duy(ij) ) * rajb
        dudy   = ( dudzi * dvx(ij) + dudet * dvy(ij) ) * rajb
        dvdy   = ( dvdzi * dvx(ij) + dvdet * dvy(ij) ) * rajb
C              
        prod(ij) = 2.0 * ( dudx * dudx  +  dvdy * dvdy )
     1           + (dudy + dvdx)**2
10     continue
11    continue
C
      if ( kplax .eq. 0 ) then
       do 21 j    = 2, jmax1
          ioff    = (j-1) * imaxl + ibeg
         ioffm    = ioff - imaxl
        do 20 i   = 2, imax1
             ij   = i + ioff
            ij1   = i + ioffm
         rav      = 0.25 * ( r(ij) + r(ij-1) + r(ij1) + r(ij1-1) )
         prod(ij) = prod(ij) + 2.0 * ( v(ij) / rav )**2
20      continue
21     continue
      endif
C
      do 31 j    = 2, jmax1
         ioff    = (j-1) * imaxl + ibeg
       do 30 i   = 2, imax1
            ij   = i + ioff
        prod(ij) = prod(ij) * amut(ij)
30     continue
31    continue
C
C.... Modify production term if calculating a swirl flow.
C
      if ( kswrl .eq. 1 ) then
       do 41 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
         ioffm  = ioff - imaxl
        do 40 i = 2, imax1
             ij = i + ioff
            imj = ij - 1
            ij1 = i + ioffm
         rav    = 0.25 * ( r(ij) + r(imj) + r(ij1) + r(ij1-1) )
         rajb   = 1.0 / ajb(ij)
         dwdzi  = fx(ij) * w(ij+1) + fx1(ij) * w(ij) - fx(imj) * w(ij)
     1          - fx1(imj) * w(imj)
         dwdet  = fy(ij) * w(ij+imaxl) + fy1(ij) * w(ij) - fy(ij1)
     1            * w(ij) - fy1(ij1) * w(ij1)
         dwdx   = ( dwdzi * dux(ij) + dwdet * duy(ij) ) * rajb
         dwdy   = ( dwdzi * dvx(ij) + dwdet * dvy(ij) ) * rajb
         prod(ij) = prod(ij) + amut(ij) * ( dwdx * dwdx
     1            + ( dwdy - w(ij)/rav )**2 )
40      continue
41     continue
      endif
C
      return
      end
C======================================================================C
      subroutine prolcu (igrl)
C                                                                      C
C     Purpose:  Prolongate corrections to contravariant velocities     C
C               perpendicular to lines of constant zi.                 C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrlp  =  igrl + 1
      imaxc  =  imax (igrl)
      jmaxc  =  jmax (igrl)
      imaxf  =  imax (igrlp)
      jmaxf  =  jmax (igrlp)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrl)
      ibegf  =  nbeg (igrlp)
C
C.... Initialise corrections.
C
      do 11 jc  = 1, jmaxc 
       do 10 ic = 1, imaxc 
           ijc  = ic + (jc - 1) * imaxc + ibegc
        c(ijc)  = 0.0
10     continue
11    continue
C
C.... Evaluate corrections on the coarse grid.
C
      do 21 jc  = 2, jmaxc1
       do 20 ic = 1, imaxc1
           ijc  = ic + (jc - 1) * imaxc + ibegc
	   ijf  = iru (ijc)
        c(ijc)  = cu(ijc) - ( cu(ijf) + cu(ijf-imaxf) )
        c(ijc)  = 0.5 * c(ijc)
20     continue
21    continue
C
C.... Prolongate corrections to the finer grid.
C
      a1 = 0.75
      a2 = 0.25
C
      do 31 ic  = 2, imaxc1
       do 30 jc = 2, jmaxc1
            ijc = ic + (jc - 1) * imaxc + ibegc
	    ijf = iru (ijc)
           ijfm = ijf - imaxf
	   ijc1 = ijc - 1
           ijcp = ijc + imaxc
           ijcm = ijc - imaxc
        cu(ijf)    = cu(ijf)  + a1 * c(ijc) + a2 * c(ijcp)
        cu(ijfm)   = cu(ijfm) + a1 * c(ijc) + a2 * c(ijcm)
        cu(ijf-1)  = cu(ijf-1)  + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2 * ( c(ijcp)+c(ijc1+imaxc) ) )
        cu(ijfm-1) = cu(ijfm-1) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2 * ( c(ijcm)+c(ijc1-imaxc) ) )
30     continue
31    continue
C
      return
      end
C======================================================================C
      subroutine prolcv (igrl)
C                                                                      C
C     Purpose:  Prolongate corrections to contravariant velocities     C
C               perpendicular to lines of constant eta.                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrlp  =  igrl + 1
      imaxc  =  imax (igrl)
      jmaxc  =  jmax (igrl)
      imaxf  =  imax (igrlp)
      jmaxf  =  jmax (igrlp)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrl)
      ibegf  =  nbeg (igrlp)
C
C.... Initialise corrections.
C
      do 10 jc  = 1, jmaxc 
       do 11 ic = 1, imaxc 
           ijc  = ic + (jc - 1) * imaxc + ibegc
        c(ijc)  = 0.0
11     continue
10    continue
C
C.... Evaluate corrections on the coarse grid.
C
      do 20 jc  = 1, jmaxc1
       do 21 ic = 2, imaxc1
           ijc  = ic + (jc - 1) * imaxc + ibegc
	   ijf  = iru (ijc)
        c(ijc)  = cv(ijc) - ( cv(ijf) + cv(ijf-1) )
        c(ijc)  = 0.5 * c(ijc)
21     continue
20    continue
C
C.... Prolongate corrections to the fine grid.
C
      a1 = 0.75
      a2 = 0.25
C
      do 30 ic  = 2, imaxc1
       do 31 jc = 2, jmaxc1
            ijc = ic + (jc - 1) * imaxc + ibegc
	   ijc1 = ijc - imaxc
           ijcm = ijc - 1
           ijcp = ijc + 1
	    ijf = iru (ijc)
           ijf1 = ijf - 1
           ijfm = ijf - imaxf
        cv(ijf)  = cv(ijf)  + a1 * c(ijc) + a2 * c(ijcp)
        cv(ijf1) = cv(ijf1) + a1 * c(ijc) + a2 * c(ijcm)
        cv(ijfm) = cv(ijfm) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1           + a2*( c(ijcp)+c(ijc1+1) ) )
        cv(ijfm-1) = cv(ijfm-1) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2*( c(ijcm)+c(ijc1-1) ) )
31     continue
30    continue
C
      return
      end
C======================================================================C
      subroutine prolcx (igrl)
C                                                                      C
C     Purpose:  Prolongate corrections to mass fluxes perpendicular    C
C               to lines of constant zi.                               C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrlp  =  igrl + 1
      imaxc  =  imax (igrl)
      jmaxc  =  jmax (igrl)
      imaxf  =  imax (igrlp)
      jmaxf  =  jmax (igrlp)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrl)
      ibegf  =  nbeg (igrlp)
C
C.... Initialise corrections.
C
      do 11 jc  = 1, jmaxc 
       do 10 ic = 1, imaxc 
           ijc  = ic + (jc - 1) * imaxc + ibegc
        c(ijc)  = 0.0
10     continue
11    continue
C
C.... Evaluate corrections on the coarse grid.
C
      do 21 jc  = 2, jmaxc1
       do 20 ic = 1, imaxc1
           ijc  = ic + (jc - 1) * imaxc + ibegc
	   ijf  = iru (ijc)
        c(ijc)  = cx(ijc) - ( cx(ijf) + cx(ijf-imaxf) )
        c(ijc)  = 0.5 * c(ijc)
20     continue
21    continue
C
C.... Prolongate corrections to the finer grid.
C
      a1 = 0.75
      a2 = 0.25
C
      do 31 ic  = 2, imaxc1
       do 30 jc = 2, jmaxc1
            ijc = ic + (jc - 1) * imaxc + ibegc
	    ijf = iru (ijc)
           ijfm = ijf - imaxf
	   ijc1 = ijc - 1
           ijcp = ijc + imaxc
           ijcm = ijc - imaxc
        cx(ijf)    = cx(ijf)  + a1 * c(ijc) + a2 * c(ijcp)
        cx(ijfm)   = cx(ijfm) + a1 * c(ijc) + a2 * c(ijcm)
        cx(ijf-1)  = cx(ijf-1)  + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2 * ( c(ijcp)+c(ijc1+imaxc) ) )
        cx(ijfm-1) = cx(ijfm-1) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2 * ( c(ijcm)+c(ijc1-imaxc) ) )
30     continue
31    continue
C
      return
      end
C======================================================================C
      subroutine prolcy (igrl)
C                                                                      C
C     Purpose:  Prolongate corrections to mass fluxes perpendicular    C
C               to lines of constant eta.                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrlp  =  igrl + 1
      imaxc  =  imax (igrl)
      jmaxc  =  jmax (igrl)
      imaxf  =  imax (igrlp)
      jmaxf  =  jmax (igrlp)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrl)
      ibegf  =  nbeg (igrlp)
C
C.... Initialise corrections.
C
      do 10 jc  = 1, jmaxc 
       do 11 ic = 1, imaxc 
           ijc  = ic + (jc - 1) * imaxc + ibegc
        c(ijc)  = 0.0
11     continue
10    continue
C
C.... Evaluate corrections on the coarse grid.
C
      do 20 jc  = 1, jmaxc1
       do 21 ic = 2, imaxc1
           ijc  = ic + (jc - 1) * imaxc + ibegc
	   ijf  = iru (ijc)
        c(ijc)  = cy(ijc) - ( cy(ijf) + cy(ijf-1) )
        c(ijc)  = 0.5 * c(ijc)
21     continue
20    continue
C
C.... Prolongate corrections to the fine grid.
C
      a1 = 0.75
      a2 = 0.25
C
      do 30 ic  = 2, imaxc1
       do 31 jc = 2, jmaxc1
            ijc = ic + (jc - 1) * imaxc + ibegc
	   ijc1 = ijc - imaxc
           ijcm = ijc - 1
           ijcp = ijc + 1
	    ijf = iru (ijc)
           ijf1 = ijf - 1
           ijfm = ijf - imaxf
        cy(ijf)  = cy(ijf)  + a1 * c(ijc) + a2 * c(ijcp)
        cy(ijf1) = cy(ijf1) + a1 * c(ijc) + a2 * c(ijcm)
        cy(ijfm) = cy(ijfm) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1           + a2*( c(ijcp)+c(ijc1+1) ) )
        cy(ijfm-1) = cy(ijfm-1) + 0.5 * ( a1*( c(ijc)+c(ijc1) )
     1             + a2*( c(ijcm)+c(ijc1-1) ) )
31     continue
30    continue
C
      return
      end
C======================================================================C
      subroutine prols (igrl,q1,indx)
C                                                                      C
C     Purpose:  Prolongate corrections to variable (q1) located        C
C               at cell center.                                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q1(*)
C
      igrlp  =  igrl + 1
      imaxc  =  imax (igrl)
      jmaxc  =  jmax (igrl)
      imaxf  =  imax (igrlp)
      jmaxf  =  jmax (igrlp)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrl)
      ibegf  =  nbeg (igrlp)
C
C.... Initialise corrections.
C
      do 10 jc  = 1, jmaxc
       do 11 ic = 1, imaxc
          ijc   = ic + (jc-1) *imaxc + ibegc
        c(ijc)  = 0.0
11     continue
10    continue
C
C.... Evaluate corrections on the coarse grid.
C
      do 20 jc = 2, jmaxc1
       do 21 ic = 2, imaxc1
           ijc = ic + (jc - 1) * imaxc + ibegc
	   ijf = iru (ijc)
        c(ijc) = q1(ijc) - 0.25 * ( q1(ijf)       + q1(ijf-1)
     1                            + q1(ijf-imaxf) + q1(ijf-imaxf-1) )
21     continue
20    continue
C
C.... Extrapolate corrections at boundaries.
C
      if(indx .eq. 1) call bcor (igrl,c)
C
C.... Prolongate corrections to the fine grid.
C
      a1 = 9./16.
      a2 = 3./16.
      a3 = 3./16.
      a4 = 1./16.
C
      do 30 ic  = 2, imaxc1
       do 31 jc = 2, jmaxc1
           ijc  = ic + (jc - 1) * imaxc + ibegc
          ijcm  = ijc - imaxc
          ijcp  = ijc + imaxc
	   ijf  = iru (ijc)
          ijf1  = ijf - 1
          ijfm  = ijf - imaxf
        q1(ijf)    = q1(ijf) + a1 * c(ijc)  + a2 * c(ijc+1)
     1                       + a3 * c(ijcp) + a4 * c(ijcp+1)
        q1(ijf1)   = q1(ijf1) + a1 * c(ijc)  + a2 * c(ijc-1)
     1                        + a3 * c(ijcp) + a4 * c(ijcp-1)
        q1(ijfm)   = q1(ijfm) + a1 * c(ijc)  + a2 * c(ijc+1)
     1                        + a3 * c(ijcm) + a4 * c(ijcm+1)
        q1(ijfm-1) = q1(ijfm-1) + a1 * c(ijc)  + a2 * c(ijc-1)
     1                          + a3 * c(ijcm) + a4 * c(ijcm-1)
31     continue
30    continue
C
      return
      end
C======================================================================C
      subroutine props (igrl)
C
C     Purpose:  Call subroutines which will calculate the temperature  C
C               and density for the compressible flow and premixed     C
C               flame models.                                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      call cpenth (igrl)
      call temp   (igrl)
      call dens   (igrl)
C
      return
      end
C======================================================================C
      subroutine rbndry
C                                                                      C
C     Purpose:  Prescribe boundary conditions for the zi plus (right)  C
C               boundary of the calculation domain.                    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      i = imaxl
C
      do 11 n = 1, nsxp
C
       wmxp(n) = wmair
       if (model .eq. 0) rhxp(n) = rhgs
       if (model .eq. 1) rhxp(n) = pref * wmair / (gascon * txp(n))
       if (model .eq. 2) then
        yn2xp   = 1.0 - fuxp(n)  - o2xp(n) - h2oxp(n) - co2xp(n)
        rwmxp   = fuxp(n)  / wmfu  + o2xp(n)  / wmox + yn2xp / wmn2
     1          + co2xp(n) / wmco2 + h2oxp(n) / wmh2o
        wmxp(n) = 1.0 / rwmxp
        rhxp(n) = pref * wmxp(n) / (gascon * txp(n) + 1.0e-30)
       endif
C
           jfl = jfxp(n,ngrid)
	   jll = jlxp(n,ngrid)
       do 10 j = jfl,jll
         ijf   = i + (j-1) * imaxl + ibeg
         ijf1  = ijf - 1
        u   (ijf)  = ubxp(n)
        v   (ijf)  = vbxp(n)
        w   (ijf)  = wbxp(n)
        p   (ijf)  = 0.0
        t   (ijf)  = txp (n)
        tke (ijf)  = tkxp(n)
        tde (ijf)  = tdxp(n)
        f   (ijf)  = fxp(n)
        g   (ijf)  = gxp(n)
        yfu (ijf)  = fuxp(n)
        yo2 (ijf)  = o2xp(n)
        yh2o(ijf)  = h2oxp(n)
        yco2(ijf)  = co2xp(n)
        yn2 (ijf)  = yn2xp
        gam (ijf)  = vscty
        amu (ijf)  = vscty
        amut(ijf)  = cd * rhxp(n) * tkxp(n) * tkxp(n)
     1               / ( tdxp(n) + 1.0e-30 )
        wmol(ijf)  = wmxp(n)
        rho (ijf)  = rhxp(n)
        cu  (ijf1) = a11(ijf1) * ubxp(n) + a12(ijf1) * vbxp(n)
        cx  (ijf1) = cu (ijf1) * rho(ijf)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine resid(igrl, q, sum)
C                                                                      C
C     Purpose:  Compute the residuals for any given variable.          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
      include 'UIFlow.indx'
C
      sum = 0.0
C
      do 11 j   = 2, jmax1
       do 10 i  = 2, imax1
           ijf  = i + (j-1) * imaxl + ibeg
        rs(ijf) = su(ijf) + aw(ijf) * q(ijf-1) + ae(ijf) * q(ijf+1) +
     1            as(ijf) * q(ijf-imaxl) + an(ijf) * q(ijf+imaxl) -
     2            ap(ijf) * q(ijf)
        sum     = sum + abs (rs(ijf))
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine restbs (igrl, q)
C                                                                      C
C     Purpose:  Restrict boundary values of the given variable.        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
C
      igrm   = igrl - 1
      imaxf  = imax (igrl)
      jmaxf  = jmax (igrl)
      ibegf  = nbeg (igrl)
      imaxc  = imax (igrm)
      jmaxc  = jmax (igrm)
      imaxc1 = imaxc - 1
      jmaxc1 = jmaxc - 1
      ibegc  = nbeg (igrm)
C
C.... Restrict x - minus boundary.
C
      i = 1
      do 10 j = 2, jmaxc1
          ijc = i + (j-1) * imaxc + ibegc
          ijf = iru(ijc)
       q(ijc) = 0.5 * ( q(ijf) + q(ijf-imaxf) )
10    continue
C
C.... Restrict x - plus boundary.
C
      i = imaxc
      do 11 j = 2, jmaxc1
          ijc = i + (j-1) * imaxc + ibegc
          ijf = iru(ijc)
       q(ijc) = 0.5 * ( q(ijf) + q(ijf-imaxf) )
11    continue
C
C.... Restrict y - minus boundary.
C
      j = 1
      do 20 i = 2, imaxc1
          ijc = i + (j-1) * imaxc + ibegc
          ijf = iru(ijc)
       q(ijc) = 0.5 * ( q(ijf) + q(ijf-1) )
20    continue
C
C.... Restrict y - plus boundary.
C
      j  =  jmaxc
      do 21 i = 2, imaxc1
          ijc = i + (j-1) * imaxc + ibegc
          ijf = iru(ijc)
       q(ijc) = 0.5 * ( q(ijf) + q(ijf-1) )
21    continue
C
      return
      end
C======================================================================C
      subroutine restfl (igrl)
C                                                                      C
C     Purpose:  Restrict the volume and mass fluxes.                   C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrlm  = igrl - 1
      imaxf  = imax (igrl)
      jmaxf  = jmax (igrl)
      ibegf  = nbeg (igrl)
      imaxc  = imax (igrlm)
      jmaxc  = jmax (igrlm)
      imaxc1 = imaxc - 1
      jmaxc1 = jmaxc - 1
      ibegc  = nbeg (igrlm)
C
C.... Restrict fluxes perpendicular to lines of constant zi.
C
      do 11 j   = 2, jmaxc1
       do 10 i  = 1, imaxc1
           ijc  = i + (j-1) * imaxc + ibegc
           ijf  = iru(ijc)
        cx(ijc) = cx(ijf) + cx(ijf-imaxf)
        cu(ijc) = cx(ijc) / (amax1 (sign (rho(ijc)  ,  cx(ijc)), 0.0)
     1                    +  amax1 (sign (rho(ijc+1), -cx(ijc)), 0.0))
10     continue
11    continue
C
C.... Restrict fluxes perpendicular to lines of constant eta.
C
      do 21 j   = 1, jmaxc1
       do 20 i  = 2, imaxc1
           ijc  = i + (j-1) * imaxc + ibegc
           ijf  = iru(ijc)
        cy(ijc) = cy(ijf) + cy(ijf-1)
        cv(ijc) = cy(ijc) / (amax1 (sign (rho(ijc),  cy(ijc)), 0.0)
     1          +  amax1 (sign (rho(ijc+imaxc), -cy(ijc)), 0.0))
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine restr (igrl)
C                                                                      C
C     Purpose:  Restrict the residuals to a coarser grid.              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      igrm   =  igrl - 1
      imaxc  =  imax (igrm)
      jmaxc  =  jmax (igrm)
      imaxf  =  imax (igrl)
      jmaxf  =  jmax (igrl)
      imaxc1 =  imaxc - 1
      jmaxc1 =  jmaxc - 1
      ibegc  =  nbeg (igrm)
      ibegf  =  nbeg (igrl)
C
C.... Calculate diffusion terms and coefficients.
C
      call  dflux (igrl)
      call  dflux (igrm)
      call  coeff (igrl)
      call  coeff (igrm)
C
C.... Calculate source terms for u equation.
C               
      call  trsrc  (igrl, u)
      call  trsrc  (igrm, u)
      call  srcu   (igrl)
      call  srcu   (igrm)
      call  urelax (igrl, 1,u)
      call  urelax (igrm, 1,u)
C
C.... Calculate residuals and restrict.
C
      call  resid (igrl,u,sum)
      call  resid (igrm,u,sum)
C
      do 11 j  = 2, jmaxc1
       do 10 i = 2, imaxc1
          ijc  = i + (j-1)*imaxc + ibegc
	  ijf  = iru(ijc)
         ijfm  = ijf - imaxf
        resux(ijc) = rs(ijf) + rs(ijf-1) + rs(ijfm) + rs(ijfm-1)
        resu (ijc) = resux(ijc) - ( rs(ijc) - resu(ijc) )
10     continue
11    continue
C
C.... Calculate source terms for v equation.
C
      call  trsrc  (igrl, v)
      call  trsrc  (igrm, v)
      call  srcv   (igrl)
      call  srcv   (igrm)
      call  urelax (igrl,2,v)
      call  urelax (igrm,2,v)
C
C.... Calculate residuals and restrict.
C
      call  resid (igrl,v,sum)
      call  resid (igrm,v,sum)
C
      do 21 j  = 2, jmaxc1
       do 20 i = 2, imaxc1
          ijc  = i + (j-1)*imaxc + ibegc
	  ijf  = iru(ijc)
         ijfm  = ijf - imaxf
        resvx(ijc) = rs(ijf) + rs(ijf-1) + rs(ijfm) + rs(ijfm-1)
        resv (ijc) = resvx(ijc) - ( rs(ijc) - resv(ijc) )
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine rests (igrl,q)
C                                                                      C
C     Purpose:  Restrict cell centered quantity.                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
C
      igrm   = igrl - 1
      imaxf  = imax (igrl)
      jmaxf  = jmax (igrl)
      ibegf  = nbeg (igrl)
      imaxc  = imax (igrm)
      jmaxc  = jmax (igrm)
      imaxc1 = imaxc - 1
      jmaxc1 = jmaxc - 1
      ibegc  = nbeg (igrm)
C
C.... Restrict internal variables.
C
      do 11 j  = 2, jmaxc1 
       do 10 i = 2, imaxc1 
          ijc  = i + (j-1) * imaxc + ibegc
          ijf1 = iru(ijc)
	  ijf2 = ijf1 - 1
	  ijf3 = ijf1 - imaxf
	  ijf4 = ijf1 - imaxf  - 1
        q(ijc) = 0.25 * ( q(ijf1) + q(ijf2) + q(ijf3) + q(ijf4) )
10     continue
11    continue
C
C.... Restrict boundary values.
C
      call restbs (igrl, q)
C
      return
      end
C======================================================================C
      subroutine restv (igrl)
C                                                                      C
C     Purpose:  Restrict values of cell centered quantities as used    C
C               in the V cycle.                                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      call rests (igrl, u)
      call rests (igrl, v)
      call rests (igrl, p)
      call rests (igrl, amu)
      call rests (igrl, gam)
      call rests (igrl, rho)
C
      if (kswrl .eq. 1) call rests(igrl, w)
      if (klam  .eq. 0) call rests(igrl, amut)
C
      return
      end
C======================================================================C
      subroutine scalar (igrf, nv, q)
C                                                                      C
C     Purpose:  Calculate coefficients and obtain solution of the      C
C               given variable.                                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
      igrl  = igrf
      include 'UIFlow.indx'
C
      call visc  (igrl, nv)
      call dflux (igrl)
      call coeff (igrl)
      call trsrc (igrl, q)
      call sorce (igrl, nv)
      call urelax(igrl, nv, q)
      call solve (igrl, nv, q)
      call resid (igrl, q, sum)
      error (igrl, nv) = sum
C
      return
      end
C======================================================================C
      subroutine scalrs (igrf)
C                                                                      C
C     Purpose:  Solve appropriate scalars depending on the problem     C
C               description.                                           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = igrf
      include 'UIFlow.indx'
C
      do 10 itnke = 1, nitke
C
       if (kswrl .eq. 1) call scalar (igrf, 4, w  )
C
       if (klam .eq. 0) then
        call prodn  (igrf)
        call scalar (igrf, 6, tke)
        call scalar (igrf, 7, tde)
       endif
C
       if (model .eq. 1) call scalar (igrf, 5, h)
       if (model .eq. 2) call onestp (igrf)
       if (model .eq. 3) call fstchm (igrf)
10    continue
C
      return
      end
C======================================================================C
      subroutine search
C                                                                      C
C     Purpose:  Calculate oxygen mass fractions at all inlets based on C
C               a prescribed fuel mass fraction.  This precludes the   C
C               specification of an already burned fuel rich mixture   C
C               at the inlet.                                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      do 10 n = 1, nsxm
       if ( kbxm(n) .eq. 2 ) then
        yair     = 1.0 - fuxm(n)
        o2xm (n) = rox * yair
        h2oxm(n) = 0.0
        co2xm(n) = 0.0
       endif
10    continue
C
      do 20 n = 1, nsxp
       if ( kbxp(n) .eq. 2 ) then
        yair     = 1.0 - fuxp(n)
        o2xp (n) = rox * yair
        h2oxp(n) = 0.0
        co2xp(n) = 0.0
       endif
20    continue
C
      do 30 n = 1, nsym
       if ( kbym(n) .eq. 2 ) then
        yair     = 1.0 - fuym(n)
        o2ym (n) = rox * yair
        h2oym(n) = 0.0
        co2ym(n) = 0.0
       endif
30    continue
C
      do 40 n = 1, nsyp
       if ( kbyp(n) .eq. 2 ) then
        yair     = 1.0 - fuyp(n)
        o2yp (n) = rox * yair
        h2oyp(n) = 0.0
        co2yp(n) = 0.0
       endif
40    continue
C
      return
      end
C======================================================================C
      subroutine solve (igrl, nv, phi)
C                                                                      C
C     Purpose:  Obtain solution to the given flow variable by use of   C
C               an ADI technique.                                      C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension phi(*), ad(200), bd(200), cc(200), dd(200)
      include 'UIFlow.indx'
C
      ifrst = 2
      ilst  = imax1
      jfrst = 2
      jlst  = jmax1
      iswps = 0
C
100   continue
C
C.... Set up tri-diagonal matrix for an EAST-WEST inversion.
C
      do 21 j  = jfrst, jlst
       do 20 i = ifrst, ilst
            ij = ibeg + i + (j-1) * imaxl
           ijp = ij + imaxl
           ijm = ij - imaxl
        bd(i) = - aw(ij)
        dd(i) =   ap(ij) - beta * an(ij)
        ad(i) = - ae(ij)
        cc(i) =   an(ij) * ( phi(ijp) - beta * phi(ij) ) +
     1            as(ij) *   phi(ijm) + su(ij) + sp(ij)
20     continue
        ijf = ifrst + ibeg + (j-1) * imaxl
        ijl = ilst  + ibeg + (j-1) * imaxl
        cc(ifrst) = cc(ifrst) + aw(ijf) * phi(ijf-1)
        cc(ilst)  = cc(ilst)  + ae(ijl) * phi(ijl+1)
C
C.... Perform forward elimination.
C
      do 22 k = ifrst + 1, ilst
           k1 = k - 1
       dd(k)  = dd(k) - ( bd(k) * ad(k1) ) / dd(k1)
       cc(k)  = cc(k) - ( bd(k) * cc(k1) ) / dd(k1)
22    continue
C
C.... Back substitute.
C
      ij = ilst  + ibeg + (j-1) * imaxl
      phi(ij) = cc(ilst) / dd(ilst)
C
      do 23 k  = 1, ilst - 2
            kk = ilst - k
           ij  = kk + ibeg + (j-1) * imaxl
       phi(ij) = ( cc(kk) - ad(kk) * phi(ij+1) ) / dd(kk)
23    continue
C
21    continue
C
C.... Call pcoefo if the grid structure is highly non-orthogonal.
C
      if ( (nv .eq. 3) .and. (knorth .eq. 1) ) call pcoefo (igrl)
C
C.... Set up tri-diagonal matrix for a NORTH-SOUTH inversion.
C
      do 31 i  = ifrst, ilst
       do 30 j = jfrst, jlst
            ij = ibeg + i + (j-1) * imaxl
        bd(j)  = - as(ij)
        dd(j)  =   ap(ij) - beta * ae(ij)
        ad(j)  = - an(ij)
        cc(j)  =   ae(ij) * ( phi(ij+1) - beta * phi(ij) ) +
     1             aw(ij) *   phi(ij-1) + su(ij) + sp(ij)
30     continue
        ijf = i + ibeg + (jfrst-1) * imaxl
        ij1 = ijf - imaxl
        ijl = i + ibeg + (jlst-1) * imaxl
        ij2 = ijl + imaxl
        cc(jfrst) = cc(jfrst) + as(ijf) * phi(ij1)
        cc(jlst)  = cc(jlst)  + an(ijl) * phi(ij2)
C
C.... Perform forward elimination.
C
      do 32 k = jfrst + 1, jlst
           k1 = k - 1
       dd(k)  = dd(k) - ( bd(k) * ad(k1) ) / dd(k1)
       cc(k)  = cc(k) - ( bd(k) * cc(k1) ) / dd(k1)
32    continue
C
C.... Back substitute.
C
      ij = i + ibeg + (jlst-1) * imaxl
      phi(ij) = cc(jlst) / dd(jlst)
C
      do 33 k  = 1, jlst - 2
           kk  = jlst - k
          ij   = i + ibeg + (kk-1) * imaxl
          ijp  = ij + imaxl
       phi(ij) = ( cc(kk) - ad(kk)*phi(ijp) ) / dd(kk)
33    continue
31    continue
C
C.... Call pcoefo if the grid structure is highly non-orthogonal.
C
      if ( (nv .eq. 3) .and. (knorth .eq. 1) ) call pcoefo (igrl)
C
      iswps = iswps + 1
C
C.... Check for convergence of algebraic solver.
C
      if (iswps .ge. nswp(nv) ) then
       goto 200
      else
       goto 100
      endif
C
200   continue
      return
      end
C======================================================================C
      subroutine sorce (igrl, nv)
C                                                                      C
C     Purpose:  Call appropriate routine for the calculation of the    C
C               given variable's source term.                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      goto (10, 20, 30, 40, 50, 60, 70, 80, 90, 100), nv
C
C.... u - momentum equation, nv = 1
C
10    continue
      call srcu (igrl)
      return
C
C.... v - momentum equation, nv = 2
C
20    continue
      call srcv (igrl)
      return
C
C.... p prime equation , nv = 3
C
30    continue
      return
C
C.... w - momentum equation, nv = 4
C
40    continue
      call srcw (igrl)
      return
C
C.... energy equation, nv = 5
C
50    continue
      call srch (igrl)
      return
C
C.... turbulence kinetic energy, nv = 6
C
60    continue
      call  srck (igrl)
      return
C
C.... turbulence dissipation rate, nv = 7
C
70    continue
      call srcd (igrl)
      return
C
C.... mixture fraction ( f ) , nv = 8
C
80    continue
      call srcf (igrl)
      return
C
C.... fuel mass fraction ( yfu ), nv = 9
C
90    continue
      call srcfu (igrl)
      return
C
C.... concentraction fluctuation ( g ), nv = 10
C
100   continue
      call srcg (igrl)
      return
      end
C======================================================================C
      subroutine srcd (igrl)
C                                                                      C
C     Purpose:  Compute the source term for the dissipation equation.  C
C               srcd also implements the wall functions by calling     C
C               walld for each wall segment.                           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Compute production and dissipation.
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
          ij   = i + ioff
        sp(ij) = - ce2 * rho (ij) * tde(ij) / tke(ij) * ajb(ij)
     1         +   amin1( su(ij)/tde(ij), 0.0 )
        su(ij) =   ce1 * prod(ij) * tde(ij) / tke(ij) * ajb(ij)
     1         +   amax1( su(ij), 0.0 )
10     continue
11    continue
C
C.... Enforce wall functions on turb. dissipation rate.
C
      i = 2
      do 20 n = 1, nsxm
       if ( kbxm(n) .ne. 1 ) goto 20
          jfl  = jfxm(n,igrl)
	  jll  = jlxm(n,igrl)
       call  walld (igrl,i,i,jfl,jll,xxic)
20    continue
C
      i = imax1
      do 30 n = 1, nsxp
       if ( kbxp(n) .ne. 1 ) goto 30
           jfl = jfxp(n,igrl)
	   jll = jlxp(n,igrl)
       call  walld (igrl,i,i,jfl,jll,xxic)
30    continue
C
      j = 2
      do 40 n = 1, nsym
       if ( kbym(n) .ne. 1 ) goto 40
          ifl  = ifym(n,igrl)
	  ill  = ilym(n,igrl)
       call  walld (igrl,ifl,ill,j,j,yetac)
40    continue
C
      j = jmax1
      do 50 n = 1, nsyp
       if ( kbyp(n) .ne. 1 ) goto 50
          ifl  = ifyp(n,igrl)
	  ill  = ilyp(n,igrl)
       call  walld (igrl,ifl,ill,j,j,yetac)
50    continue
C
      return
      end
C======================================================================C
      subroutine srcf (igrl)
C                                                                      C
C     Purpose:  Compute the source term for the mixture fraction       C
C               equation.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
           ij  = i + ioff
        sp(ij) = amin1( su(ij)/f(ij), 0.0 )
        su(ij) = amax1( su(ij), 0.0 )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine srcfu (igrl)
C                                                                      C
C     Purpose:  Compute the source terms for the fuel fraction         C
C               equation.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      if ( klam .eq. 1 ) then
C
C.... Source term determination for a LAMINAR flow.
C
       do 11 j   = 2, jmax1
          ioff   = (j-1) * imaxl + ibeg
        do 10 i  = 2, imax1
             ij  = i + ioff
         sp(ij)  = amin1( su(ij)/yfu(ij), 0.0 )
         su(ij)  = amax1( su(ij), 0.0 )
         arhn    = - prexp(1) * exp(-acten(1) / t(ij))
         sor     = arhn * ( yfu(ij)**aa(1) ) * ( yo2(ij)**bb(1) )
     1           * ( rho(ij)**ab(1) )
         sp (ij) = sp(ij) + sor * ajb(ij) / yfu(ij)
         sfu(ij) = sor * ajb(ij)
10      continue
11     continue
C
      elseif ( klam .eq. 0 ) then
C
C.... Source term determination for a TURBULENT flow.
C
       do 21 j   = 2, jmax1
          ioff   = (j-1) * imaxl + ibeg
        do 20 i  = 2, imax1
             ij  = i + ioff
         sp(ij)  = amin1( su(ij)/yfu(ij), 0.0 )
         su(ij)  = amax1( su(ij), 0.0 )
         arhn    = - prexp(1) * exp( -acten(1) / t(ij) )
         sor1    = arhn * ( yfu(ij)**aa(1) ) * ( yo2(ij)**bb(1) )
     1           * ( rho(ij) ** ab(1) )
         sor2    = - cr * yfu(ij) * rho(ij) * tde(ij) / tke(ij)
         sor     = amax1 ( sor1, sor2 ) * ajb(ij)
         sp(ij)  = sp(ij) + sor / yfu(ij)
         sfu(ij) = sor
20      continue
21     continue
      endif
C
      return
      end
C======================================================================C
      subroutine srcg (igrl)
C                                                                      C
C     Purpose:  Compute the source terms for the concentration         C
C               fluctuation equation.                                  C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
            ij = i + ioff
           ij1 = ij - 1
           ijp = ij + imaxl
           ijm = ij - imaxl
C
C.... Compute derivatives in computational domain.
C
        dfdzi  = fx(ij)  * f(ij+1) + fx1(ij)  * f(ij)
     1         - fx(ij1) * f(ij)   - fx1(ij1) * f(ij1)
        dfdeta = fy(ij)  * f(ijp)  + fy1(ij)  * f(ij)
     1         - fy(ijm) * f(ij)   - fy1(ijm) * f(ijm)
C
C.... Compute derivatives in the physical space.
C
        rajb = 1.0 / ajb(ij)
        dfdx = ( dfdzi * dux(ij) + dfdeta * duy(ij) ) * rajb
        dfdy = ( dfdzi * dvx(ij) + dfdeta * dvy(ij) ) * rajb
C
C.... Calculate source term for the scalar fluctuation equation.
C
        sp(ij) = - cg2 * rho(ij) * tde(ij) * ajb(ij) / tke(ij)
     1           + amin1( su(ij)/g(ij) , 0.0 )
        su(ij) = cg2 * amut(ij) * ajb(ij) / prt(10)
     1         * ( dfdx * dfdx + dfdy * dfdy )
     2         + amax1( su(ij) , 0.0 )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine srch (igrl)
C                                                                      C
C     Purpose:  Calculate source term for the enthalpy equation.       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      if ( model .ne. 2 ) return
C
C.... Source terms which account for specie heat flux.
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
            ij = i + ioff
        sp(ij) = amin1( su(ij)/h(ij), 0.0 )
        su(ij) = amax1( su(ij), 0.0 )
        su(ij) = su(ij) + sfu(ij) * ( rat(4)*hfco2 + rat(5)*hfh2o )
        sp(ij) = sp(ij) - sfu(ij) * hffu / h(ij)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine srck (igrl)
C                                                                      C
C     Purpose:  Compute the source terms in the turbulent kinetic      C
C               energy equation.                                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
          ij   = i + ioff
        sp(ij) = - rho(ij) * tde(ij) / tke(ij) * ajb(ij)
     1         +   amin1( su(ij)/tke(ij), 0.0 )
        su(ij) =   prod(ij) * ajb(ij) + amax1( su(ij), 0.0 )
10     continue
11    continue
C
C.... Modify coefficients and source terms at walls.
C
      i = 2
      do 20 n = 1, nsxm
       if ( kbxm(n) .eq. 1 ) then
          jfl = jfxm(n,igrl)
	  jll = jlxm(n,igrl)
        call  wallk (igrl,i,i,jfl,jll,u,v,w,0.0,0.0,0.0,xxic,aw)
       endif
20    continue
C
      i = imax1
      do 30 n = 1, nsxp
       if ( kbxp(n) .eq. 1 ) then
          jfl = jfxp(n,igrl)
	  jll = jlxp(n,igrl)
        call  wallk (igrl,i,i,jfl,jll,u,v,w,0.0,0.0,0.0,xxic,ae)
       endif
30    continue
C
      j = 2
      do 40 n = 1, nsym
       if ( kbym(n) .eq. 1 ) then
          ifl = ifym(n,igrl)
	  ill = ilym(n,igrl)
        call  wallk (igrl,ifl,ill,j,j,u,v,w,0.0,0.0,0.0,yetac,as)
       endif
40    continue
C
      j = jmax1
      do 50 n = 1, nsyp
       if ( kbyp(n) .eq. 1 ) then
          ifl = ifyp(n,igrl)
	  ill = ilyp(n,igrl)
        call  wallk (igrl,ifl,ill,j,j,u,v,w,0.0,0.0,0.0,yetac,an)
       endif
50    continue
C
      return
      end
C======================================================================C
      subroutine srcu (igrl)
C                                                                      C
C     Purpose:  Calculate source terms for the u momentum equation.    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Calculate pressure gradient terms.
C
      call grad (igrl, p)
      call bcor (igrl, dpdx)
      call bcor (igrl, dpdy)
C
C.... Calculate apu and source terms.
C
      do 11  j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 10 i  = 2, imax1
            ij  = i + ioff
        apu(ij) = su(ij)
        su(ij)  = (dux(ij) * dpdx(ij) + duy(ij) * dpdy(ij)) +
     1             resu(ij) + su(ij)
        sp(ij)  = 0.0
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine srcv (igrl)
C                                                                      C
C     Purpose:  Calculate source terms for the v momentum equation.    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
C.... Calculate source terms axisymmetric flows.
C
      if ( kplax .eq. 0 ) then
       do 11 j  = 2, jmax1
           ioff = (j-1) * imaxl + ibeg
        do 10 i = 2, imax1
            ij  = i + ioff
           ij1  = ij - imaxl
         rav    = 0.25 * ( r(ij) + r(ij-1) + r(ij1) + r(ij1-1) )
         su(ij) = su(ij) + rho(ij) * w(ij) * w(ij) / rav * ajb(ij)
         sp(ij) = - 2.0 * gam(ij) / (rav * rav) * ajb(ij)
10      continue
11     continue
      endif
C
C.... Calculate apv and source terms.
C
      do 21 j   = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
       do 20 i  = 2, imax1
            ij  = i + ioff
        apv(ij) = su(ij)
        su (ij) = (dvx(ij) * dpdx(ij) + dvy(ij) * dpdy(ij)) +
     1             resv(ij) + su(ij)
20     continue
21    continue
C
      return
      end
C======================================================================C
      subroutine srcw (igrl)
C                                                                      C
C     Purpose:  Calculate source terms for the w momentum equation.    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
            ij = i  + ioff
           ij1 = ij - 1
           ijp = ij + imaxl
           ijm = ij - imaxl
        rav    = 0.25 * ( r(ij) + r(ij1) + r(ijm) + r(ijm-1) )
        dgdzi  = fx(ij)  * gam(ij+1) + fx1(ij)  * gam(ij)
     1         - fx(ij1) * gam(ij)   - fx1(ij1) * gam(ij1)
        dgdeta = fy(ij)  * gam(ijp) + fy1(ij)  * gam(ij)
     1         - fy(ijm) * gam(ij)  - fy1(ijm) * gam(ijm)
        dgamdr = dgdzi * dvx(ij) + dgdeta * dvy(ij)
        term1  = - rho(ij) * v(ij) * w(ij) / rav * ajb(ij)
        term2  = - gam(ij) * w(ij) / ( rav * rav ) * ajb(ij)
        term3  = - w(ij) * dgamdr  / rav
C
        sp(ij) = amin1( su(ij)/w(ij), 0.0 ) + amin1( term1/w(ij), 0.0 )
     1         + amin1( term2/w(ij) , 0.0 ) + amin1( term3/w(ij), 0.0 )
        su(ij) = amax1( su(ij), 0.0 ) + amax1( term1, 0.0 )
     1         + amax1( term2 , 0.0 ) + amax1( term3, 0.0 )
C
10     continue
11    continue
      return
      end
C======================================================================C
      subroutine tbndry
C                                                                      C
C     Purpose:  Prescribe boundary conditions on the eta plus (top)    C 
C               boundary.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      igrl = ngrid
      include 'UIFlow.indx'
C
      j = jmaxl
C
      do 11 n = 1, nsyp
C
       wmyp(n) = wmair
       if (model .eq. 0) rhyp(n) = rhgs
       if (model .eq. 1) rhyp(n) = pref * wmair / ( gascon * typ(n) )
       if (model .eq. 2) then
        yn2yp   = 1.0 - fuyp(n) - o2yp(n) - h2oyp(n) - co2yp(n)
        rwmyp   = fuyp(n)  / wmfu  + o2yp(n)  / wmox + yn2yp / wmn2
     1          + co2yp(n) / wmco2 + h2oyp(n) / wmh2o
        wmyp(n) = 1.0 / rwmyp
        rhyp(n) = pref * wmyp(n) / (gascon * typ(n) + 1.0e-30)
       endif
          ifl = ifyp(n,ngrid)
	  ill = ilyp(n,ngrid)
       do 10 i = ifl,ill
         ijf   = i + (j-1) * imaxl + ibeg
	 ijf1  = ijf - imaxl
        u   (ijf)  = ubyp(n)
        v   (ijf)  = vbyp(n)
        w   (ijf)  = wbyp(n)
        p   (ijf)  = 0.0
        t   (ijf)  = typ (n)
        tke (ijf)  = tkyp(n)
        tde (ijf)  = tdyp(n)
        f   (ijf)  = fyp(n)
        g   (ijf)  = gyp(n)
        yfu (ijf)  = fuyp(n)
        yo2 (ijf)  = o2yp(n)
        yh2o(ijf)  = h2oyp(n)
        yco2(ijf)  = co2yp(n)
        yn2 (ijf)  = yn2yp
        gam (ijf)  = vscty
        amu (ijf)  = vscty
        amut(ijf)  = cd * rhyp(n) * tkyp(n) * tkyp(n)
     1               / ( tdyp(n) + 1.0e-30 )
        wmol(ijf)  = wmyp(n)
        rho (ijf)  = rhyp(n)
        cv (ijf1)  = a21(ijf1)*u(ijf) + a22(ijf1)*v(ijf)
        cy (ijf1)  = cv (ijf1)*rho(ijf)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine temp (igrl)
C                                                                      C
C     Purpose:  Calculate fluid temperature for compressible and       C
C               premixed flame models.                                 C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      relxm = 1.0 - relx(5)
C
      if ( model .eq. 1 ) then
C
C... Calculate temperature for compressible air flow problem.
C
       hmix = rox * hox  +  (1.0 - rox) * hn2
C
       do 11 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
        do 10 i = 2, imax1
             ij = i + ioff
         t1     = ( h(ij) + hmix ) / cph(ij)
         t(ij)  = relx(5) * t1 + relxm * t(ij)
10      continue
11     continue
C
      elseif ( model .eq. 2 ) then
C
       do 21 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
        do 20 i = 2, imax1
             ij = i + ioff
         hmix   =  yfu(ij)  * hfu  +  yo2(ij)  * hox
     1          +  yn2(ij)  * hn2  +  yco2(ij) * hco2
     2          +  yh2o(ij) * hh2o
         t1     = ( h(ij) + hmix ) / cph(ij)
         t(ij)  = relx(5) * t1  +  relxm * t(ij)
         t(ij)  = amin1( t(ij), 3500.0 )
20      continue
21     continue
      endif 
C
      return
      end
C======================================================================C
      subroutine trap (igrl)
C                                                                      C
C     Purpose:  Ensure that the sum of all mass fractions is equal     C
C               to unity.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j  = 2, jmax1
          ioff = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
            ij = i + ioff
        sum    = yfu(ij) + yco2(ij) + yh2o(ij) + yo2(ij) + yn2(ij)
        yfu (ij) = yfu (ij) / sum
        yco2(ij) = yco2(ij) / sum
        yh2o(ij) = yh2o(ij) / sum
        yo2 (ij) = yo2 (ij) / sum
        yn2 (ij) = yn2 (ij) / sum
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine trsrc (igrl, q)
C                                                                      C
C     Purpose:  Compute the source terms resulting from coordinate     C
C               transformation.                                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
      include 'UIFlow.indx'
C
C.... Compute the derivatives at the cell centers.
C
      do 11 j  = 2, jmax1
          ioff = (j-1) * imaxl + ibeg
         ioff1 = ioff - 1
         ioffm = ioff - imaxl
       do 10 i = 2, imax1
           ij  = i + ioff
          ij1  = i + ioff1
          ijm  = i + ioffm
        x1(ij) = fx(ij)  * q(ij+1) + fx1(ij)  * q(ij) -
     1           fx(ij1) * q(ij)   - fx1(ij1) * q(ij1)
        y1(ij) = fy(ij)  * q(ij+imaxl) + fy1(ij) * q(ij) -
     1           fy(ijm) * q(ij)   - fy1(ijm) * q(ijm)
        x1(ij) = gam(ij) * q21(ij) * x1(ij)
        y1(ij) = gam(ij) * q12(ij) * y1(ij)
10     continue
11    continue
C
C.... Calculate derivative at the x - minus boundary.
C
      do 20 j = 2, jmax1
         ij   = 1 + (j-1) * imaxl + ibeg
        ijm   = ij - imaxl
       y1(ij) = fy(ij)  * q(ij+imaxl) + fy1(ij)  * q(ij) -
     1          fy(ijm) * q(ij)       - fy1(ijm) * q(ijm)
       y1(ij) = gam(ij) * q12(ij+1) * y1(ij)
20    continue
C
C.... Calculate derivative at the y - minus boundary.
C
      do 21 i = 2, imax1
         ij   = ibeg + i
         ij1  = ij + imaxl
         ijm  = ij - 1
       x1(ij) = fx(ij)  * q(ij+1) + fx1(ij)  * q(ij) -
     1          fx(ijm) * q(ij)   - fx1(ijm) * q(ijm)
       x1(ij) = gam(ij) * q21(ij1) * x1(ij)
21    continue
C
C.... Calculate derivative at the x - plus boundary.
C
      do 22 j = 2, jmax1
          ij  = imaxl + (j-1) * imaxl + ibeg
         ij1  = ij - 1
         ijm  = ij - imaxl
       y1(ij) = fy(ij)  * q(ij+imaxl) + fy1(ij)  * q(ij) -
     1          fy(ijm) * q(ij)       - fy1(ijm) * q(ijm)
       y1(ij) = gam(ij) * q12(ij1) * y1(ij)
22    continue
C
C.... Calculate derivative at the y - plus boundary.
C
      do 23 i = 2, imax1
         ij   = i + jmax1 * imaxl + ibeg 
         ij1  = ij - imaxl
         ijm  = ij - 1
       x1(ij) = fx(ij)  * q(ij+1) + fx1(ij)  * q(ij) -
     1          fx(ijm) * q(ij)   - fx1(ijm) * q(ijm)
       x1(ij) = gam(ij)  * q21(ij1) * x1(ij)
23    continue
C
C.... Calculate transformation source term.
C
      do 51 j   = 2, jmax1
           ioff = (j-1) * imaxl + ibeg
       do 50 i  = 2, imax1
            ij  = i + ioff
            ij1 = ij - 1
            ijp = ij + imaxl
            ijm = ij - imaxl
        su(ij)  = ( fx(ij)  * y1(ij+1) + fx1(ij)  * y1(ij)   )
     1          - ( fx(ij1) * y1(ij)   + fx1(ij1) * y1(ij1) )
     2          + ( fy(ij)  * x1(ijp)  + fy1(ij)  * x1(ij)   )
     3          - ( fy(ijm) * x1(ij)   + fy1(ijm) * x1(ijm)  )
50     continue
51    continue
      return
      end
C======================================================================C
      subroutine tvis (igrl)
C                                                                      C
C     Purpose:  Calculate the turbulent viscosity.  Together with      C
C               the laminar viscosity it comprises the exchange        C
C               coefficient for the momentum equations.                C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      relxm    = 1.0 - relx(6)
C
      do 11 j  = 2, jmax1
         ioff  = ibeg + (j-1)* imaxl
       do 10 i = 2, imax1
           ij  = i + ioff
        amut(ij) = relxm * amut(ij) + 
     1           relx(6) * cd * rho(ij) * tke(ij) * tke(ij) / tde(ij)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine update (igrf, igrl)
C                                                                      C
C     Purpose:  Correct the flow variables in accordance with the use  C
C               of a decoupled solution procedure ( SIMPLEC ).         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      call bcor (igrl, pp)
      call grad (igrl, pp)
C
C.... Corrections for incompressible flows.
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
           ij  = i + ioff
        rapm   = 1.0 / apm(ij)
        p(ij)  = p(ij) +  pp(ij) * relx(3)
        u(ij)  = u(ij) + (dux(ij) * dpdx(ij) + duy(ij) * dpdy(ij))
     1                 * rapm
        v(ij)  = v(ij) + (dvx(ij) * dpdx(ij) + dvy(ij) * dpdy(ij))
     1                 * rapm
10     continue
11    continue
C
C.... Correct zi - direction fluxes.
C
      do 13 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 12 i = 2, imax2
           ij  = i + ioff
          ij1  = ij + 1
        cx(ij) = cx(ij) + dx(ij) * ( pp(ij) - pp(ij1) )
        cu(ij) = cx(ij) / ( amax1( sign( rho(ij) ,  cx(ij) ), 0.)
     1                  +   amax1( sign( rho(ij1), -cx(ij) ), 0.) )
12     continue
13    continue
C
C.... Correct eta direction fluxes.
C
      do 15 j = 2, jmax2
         ioff = (j-1) * imaxl + ibeg
        ioffp = ioff + imaxl
       do 14 i = 2, imax1
           ij = i + ioff
          ijp = i + ioffp
        cy(ij)  = cy(ij) + dy(ij) * ( pp(ij) - pp(ijp) )
        cv(ij)  = cy(ij) / ( amax1( sign( rho(ij) ,  cv(ij) ), 0.)
     1                   +   amax1( sign( rho(ijp), -cv(ij) ), 0.) )
14     continue
15    continue
C
C.... Additional corrections for compressibility.
C
      if ( kcomp .eq. 1 ) then
       do 21 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
         ioffp  = ioff + imaxl
        do 20 i = 2, imax2
            ij  = i + ioff
           ijp  = i + ioffp
           ij1  = ij + 1
         cx(ij) = cx(ij) + amax1( -cu(ij), 0.) * wmol(ij1) * pp(ij1)
     1                         / (  gamma * gascon * t(ij1) )
     2                   + amax1(  cu(ij), 0.) * wmol(ij) * pp(ij)
     3                         / (  gamma * gascon * t(ij)   )
20      continue
21     continue
C
       do 23 j  = 2, jmax2
          ioff  = (j-1) * imaxl + ibeg
         ioffp  = ioff + imaxl
        do 22 i = 2, imax1
            ij  = i + ioff
           ijp  = i + ioffp
         cy(ij) = cy(ij) + amax1( -cv(ij), 0. ) * wmol(ijp) * pp(ijp)
     1                       / (  gamma * gascon * t(ijp) )
     2                   + amax1(  cv(ij), 0.) * wmol(ij) * pp(ij)
     3                       / (  gamma * gascon * t(ij)  )
22      continue
23     continue
      endif
C
C.... Corrections to fluxes for highly non-orthogonal grid.
C
      if ( knorth .eq. 1 ) then
       do 32 j  = 2, jmax1
          ioff  = (j-1) * imaxl + ibeg
        do 31 i = 2, imax2
             ij = i + ioff
            ij1 = ij + 1
            ijp = ij + imaxl
         cu(ij) = a11(ij) * ( fx1(ij)*duy(ij) *dpdy(ij) /apm(ij)
     1                    +   fx(ij) *duy(ij1)*dpdy(ij1)/apm(ij1) )
     2          + a12(ij) * ( fx1(ij)*dvy(ij) *dpdy(ij) /apm(ij)
     3                    +   fx(ij) *dvy(ij1)*dpdy(ij1)/apm(ij1) )
     4          + cu(ij)
         cx(ij) = cu(ij) * ( amax1( sign( rho(ij) ,  cu(ij) ), 0.)
     1                   +   amax1( sign( rho(ij1), -cu(ij) ), 0.) )
31      continue
32     continue
C
       do 34 j  = 2, jmax2
          ioff  = (j-1) * imaxl + ibeg
        do 33 i = 2, imax1
            ij  = i + ioff
           ijp  = ij + imaxl
         cv(ij) = a21(ij) * ( fy1(ij)*dux(ij) *dpdx(ij) /apm(ij)
     1                    +   fy(ij) *dux(ijp)*dpdx(ijp)/apm(ijp) )
     2          + a22(ij) * ( fy1(ij)*dvx(ij) *dpdx(ij) /apm(ij)
     3                    +   fy(ij) *dvx(ijp)*dpdx(ijp)/apm(ijp) )
     4          + cv(ij)
         cy(ij) = cv(ij)  * ( amax1( sign( rho(ij) ,  cv(ij) ), 0.)
     1                    +   amax1( sign( rho(ijp), -cv(ij) ), 0.) )
33      continue
34     continue
      endif
C
      return
      end
C======================================================================C
      subroutine urelax(igrl,nv,q)
C                                                                      C
C     Purpose:  Incorporate under-relaxation into the discretized      C
C               equation.                                              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension q(*)
      include 'UIFlow.indx'
C
      relxm   = 1.0 - relx(nv)
C
      do 11 j  = 2, jmax1
         ioff  = (j-1) * imaxl + ibeg
       do 10 i = 2, imax1
           ij  = i + ioff
        ap(ij) = ae(ij) + aw(ij) + an(ij) + as(ij) - sp(ij)
        sp(ij) = 0.0
        ap(ij) = ap(ij) / relx(nv)
        su(ij) = su(ij) + relxm * ap(ij) * q(ij)
10     continue
11    continue
C
      if ( nv .eq. 1 ) then
       do 20  j  = 2, jmax1
           ioff  = (j-1) * imaxl + ibeg
        do 21 i  = 2, imax1
             ij  = i + ioff
         app(ij) = ap(ij)
21      continue
20     continue
      endif
C
      return
      end
C======================================================================C
      subroutine visc (igrl, nv)
C                                                                      C
C     Purpose:  Calculate diffusive exchange coefficient.              C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      do 11 j   = 1, jmaxl
          ioff  = (j-1) * imaxl + ibeg
       do 10 i  = 1, imaxl
            ij  = i + ioff
        gam(ij) = amu(ij) / prl(nv) + amut(ij) / prt(nv)
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine vset
C======================================================================C
      include 'UIFlow.com'
C
      dimension xloc(3500) , yloc(3500) , uf(3500)   , vf(3500)  ,
     1          wf  (3500) , tf(3500)   , ff(3500)   , gf(3500)  ,
     2          yfuf(3500) , yh2of(3500), yco2f(3500), yo2f(3500),
     3          yn2f(3500) , rhof(3500) , hf(3500)   , tkef(3500),
     4          tdef(3500) , pf(3500)   , n1(14000)
      external DFopen, DFclose, vsfatch, vsfdtch, vsfsfld
      external vsfsnam, vsfwrit, vsffdef, vfatch, vfdtch, vfsnam
      external vfinsrt
      integer  DFopen, vsfatch, vsfsfld, vsfwrit, vsffdef, vfatch,
     1         vfinsrt, ftype, fintrlace, fullacc,
     2         vg, vs
C	==================================================================
C
      external VHFSD, VHFSDM, VHFMKGP
      integer	VHFSD, VHFSDM, VHFMKGP
      integer INTTYPE	
	  character*10 strng
	  character*20 strng1
	  character*20 strng2
	  character*10 uiflow
      parameter (INTTYPE=2)
      integer REALTYPE				
      parameter (REALTYPE=3)
      integer	VDATATAG
      parameter (VDATATAG=1962)
      integer tagarray(30), refarray(30), vxsize
C	==================================================================
C	
      CEXTERNAL SHOWLINE
	  CHARACTER*255 buffer
	  CHARACTER*1 NULL
C
      parameter (ftype = 3, fintrlace = 0, fullacc = 7 )
      igrl = ngrid
      include 'UIFlow.indx'
      NULL = char(0)
C
      nsize = imaxl * jmaxl
	  nsize2 = imax1 * jmax1
C
C.... Calculate x-y coordinates at data points.
C
      do 10 j  = 2, jmax1
       do 11 i = 2, imax1
            ij = ibeg + i + (j-1) * imaxl
           ijm = ij - imaxl
          i0j0 = i + (j-1) * imaxl
        xloc(i0j0) = 0.25 * ( x(ij) + x(ij-1) + x(ijm) + x(ijm-1) )
        yloc(i0j0) = 0.25 * ( y(ij) + y(ij-1) + y(ijm) + y(ijm-1) )
11     continue
10    continue
C
C.... Calculate y-minus and y-plus x,y locations.
C
      do 12 i = 2, imax1
          ijm = ibeg + i
          ijp = ibeg + i + (jmax2) * imaxl
         i0jm = i
         i0jp = i + (jmax1) * imaxl
       xloc(i0jm) = 0.5 * ( x(ijm) + x(ijm-1) )
       xloc(i0jp) = 0.5 * ( x(ijp) + x(ijp-1) )
       yloc(i0jm) = 0.5 * ( y(ijm) + y(ijm-1) )
       yloc(i0jp) = 0.5 * ( y(ijp) + y(ijp-1) )
12    continue
C
C.... Calculate x-minus and x-plus x,y locations.
C
      do 13 j = 2, jmax1
          imj = ibeg + 1 + (j-1) * imaxl
          ipj = ibeg + imax1 + (j-1) * imaxl
         imj0 = 1 + (j-1) * imaxl
         ipj0 = imaxl + (j-1) * imaxl
       xloc(imj0) = 0.5 * ( x(imj) + x(imj-imaxl) )
       xloc(ipj0) = 0.5 * ( x(ipj) + x(ipj-imaxl) )
       yloc(imj0) = 0.5 * ( y(imj) + y(imj-imaxl) )
       yloc(ipj0) = 0.5 * ( y(ipj) + y(ipj-imaxl) )
13    continue
C
C.... Specify CORNER points.
C
      xloc(1) = x(ibeg + 1)
      yloc(1) = y(ibeg + 1)
C
      xloc(imaxl) = x(ibeg + imax1)
      yloc(imaxl) = y(ibeg + imax1)
C
      xloc(1 + jmax1*imaxl) = x(ibeg + 1 + jmax2*imaxl)
      yloc(1 + jmax1*imaxl) = y(ibeg + 1 + jmax2*imaxl)
C
      xloc(imaxl + jmax1*imaxl) = x(ibeg + imax1 + jmax2*imaxl)
      yloc(imaxl + jmax1*imaxl) = y(ibeg + imax1 + jmax2*imaxl)
C
      do 20 j   = 1, jmaxl
       do 21 i  = 1, imaxl
            ij  = ibeg + i + (j-1) * imaxl
          i0j0  = i + (j-1 ) * imaxl
        uf   (i0j0) = u   (ij)
        vf   (i0j0) = v   (ij)
        wf   (i0j0) = w   (ij)
        pf   (i0j0) = p   (ij)
        tf   (i0j0) = t   (ij)
        hf   (i0j0) = h   (ij)
        ff   (i0j0) = f   (ij)
        gf   (i0j0) = g   (ij)
        rhof (i0j0) = rho (ij)
        yfuf (i0j0) = yfu (ij)
        yh2of(i0j0) = yh2o(ij)
        yco2f(i0j0) = yco2(ij)
        yo2f (i0j0) = yo2 (ij)
        yn2f (i0j0) = yn2 (ij)
        tkef (i0j0) = tke (ij)
        tdef (i0j0) = tde (ij)
21     continue
20    continue
C
C.... Compute connectivity.
C
      do 30 j  = 1, jmax1
       do 31 i = 1, imax1
            ij = i + (j-1) * imaxl
           ncv = i + (j-1) * imax1
          indx = ( ncv - 1 ) * 4 + 1
        n1(indx)   = ij
        n1(indx+1) = ij + 1
        n1(indx+3) = ij + imaxl
        n1(indx+2) = ij + imaxl + 1
31     continue
30    continue
C
C
	  nn = DFopen('vset.out', fullacc, 0 )
	  if (nn .eq. -1) then 
	  	call SHOWLINE('Unable to Create VSet')
	    return
	  endif
C
      vxsize = 0
C
      uiflow = 'uiflow' // char(0)
      strng = 'px' // char(0)
      strng1 = 'xcoord' // char(0)
      vs = VHFSD (nn, strng, xloc, nsize, REALTYPE, strng1, uiflow)
	  vxsize = vxsize + 1
      tagarray(vxsize) = VDATATAG
      refarray(vxsize) = vs
C
      strng = 'py' // char(0)
      strng1 = 'ycoord' // char(0)
      vs = VHFSD (nn, strng, yloc, nsize, REALTYPE, strng1, uiflow)
	  vxsize = vxsize + 1
      tagarray(vxsize) = VDATATAG
      refarray(vxsize) = vs
C
      strng = 'plist4' // char(0)
      strng1 = 'CONNECTIVITY' // char(0)
      vs = VHFSDM (nn, strng, n1, nsize2, INTTYPE, strng1,uiflow,4)
	  vxsize = vxsize + 1
      tagarray(vxsize) = VDATATAG
      refarray(vxsize) = vs
C
C.... Write data.
C
      if (iprint(1) .eq. 1) then
        strng = 'u-vel' // char(0)
        strng1 = 'U-VELOCITY' // char(0)
        vs = VHFSD (nn, strng, uf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(2) .eq. 1) then
        strng = 'v-vel' // char(0)
        strng1 = 'V-VELOCITY' // char(0)
        vs = VHFSD (nn, strng, vf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(4) .eq. 1) then
        strng = 'w-vel' // char(0)
        strng1 = 'W-VELOCITY' // char(0)
        vs = VHFSD (nn, strng, wf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(5) .eq. 1) then
        strng = 'temp' // char(0)
        strng1 = 'TEMPERATURE' // char(0)  
        vs = VHFSD (nn, strng, tf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(3) .eq. 1) then
        strng = 'press' // char(0)
        strng1 = 'PRESSURE' // char(0)
        vs = VHFSD (nn, strng, pf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(11) .eq. 1) then
        strng = 'dens' // char(0)
        strng1 = 'DENSITY' // char(0)
        vs = VHFSD (nn, strng, rhof, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(5) .eq. 1) then
        strng = 'enth' // char(0)
        strng1 = 'ENTHALPY' // char(0)
        vs = VHFSD (nn, strng, hf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(9) .eq. 1) then
        strng = 'fuel' // char(0)
        strng1 = 'FUEL' // char(0)
        vs = VHFSD (nn,strng, yfuf, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(12) .eq. 1) then
        strng = 'h2o' // char(0)
        strng1 = 'WATER' // char(0)
        vs = VHFSD (nn, strng, yh2of, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(12) .eq. 1) then
        strng = 'co2' // char(0)
        strng1 = 'CARBON_DIOXIDE' // char(0)
        vs = VHFSD (nn, strng, yco2f, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(12) .eq. 1) then
        strng = 'o2' // char(0)
        strng1 = 'OXYGEN' // char(0)
        vs = VHFSD (nn, strng, yo2f, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(12) .eq. 1) then
        strng = 'n2' // char(0)
        strng1 = 'NITROGEN' // char(0)
        vs = VHFSD (nn, strng, yn2f, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(8) .eq. 1) then
        strng = 'mix.frac.' // char(0)
        strng1 = 'MIXTURE.FRAC' // char(0)
        vs = VHFSD (nn, strng, ff, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(10) .eq. 1) then
        strng = 'conc.fluc.' // char(0)
        strng1 = 'CONC.FLUC.' // char(0)
        vs = VHFSD (nn, strng, gf, nsize, REALTYPE, strng1, uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(6) .eq. 1) then
        strng = 'kin.ener.' // char(0)
        strng1 = 'KIN.ENER.' // char(0)
        vs = VHFSD (nn, strng, tkef, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      if (iprint(7) .eq. 1) then
        strng = 'turb.disp' // char(0)
        strng1 = 'TURB.DISP.' // char(0)
        vs = VHFSD (nn, strng, tdef, nsize, REALTYPE, strng1,uiflow)
	    vxsize = vxsize + 1
	    tagarray(vxsize) = VDATATAG
	    refarray(vxsize) = vs
	  endif
C
      strng2 = 'UIFlow Group' // char(0)
      vg = VHFMKGP(nn,tagarray,refarray,vxsize,strng2,uiflow)
	  if (vg .eq. -1)   call SHOWLINE('Unable to Create VGroup')
      call DFclose ( nn )
C
      return
      end
C======================================================================C
      subroutine walld (igrl,i1,i2,j1,j2,dst)
C                                                                      C
C     Purpose:  Modify the source terms at the walls to impose wall    C
C               functions on turbulent dissipation.                    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension dst(*)
      include 'UIFlow.indx'
C
C.... Turb. dissipation is fixed at near wall nodes.
C
      do 11 i  = i1, i2
       do 10 j = j1, j2
           ij  = i + (j-1) * imaxl + ibeg
        su(ij) = 2.0e10 * cdtqtr * (tke(ij) ** 1.5) / (ak * dst(ij))
        sp(ij) = - 1.0e10
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine wallg (igrl, i1, i2, j1, j2, ioff, dst)
C                                                                      C
C     Purpose:  Modify the values of the exchange coefficient at the   C
C               walls so as to impose wall functions on velocities.    C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension dst(*)
      include 'UIFlow.indx'
C
      do 11 i  = i1, i2
       do 10 j = j1, j2
           ij  = i + (j-1) * imaxl + ibeg
          ij1  = ij + ioff
        term     = 0.5  * rho(ij) * cdqtr * sqrt( tke(ij) ) * dst(ij)
        gam(ij1) = term * ak / alog (ee * term / amu(ij) )
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine wallk(igrl,i1,i2,j1,j2,v1,v2,v3,vw1,vw2,vw3,dst,cf1)
C                                                                      C
C     Purpose:  Modify the source terms at the near wall cells so as   C
C               to impose wall functions on k.                         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      dimension v1(*), v2(*), v3(*), cf1(*), dst(*)
      include 'UIFlow.indx'
C
      do 11 j   = j1, j2
       do 10 i  = i1, i2
            ij  = i + (j-1) * imaxl + ibeg
        tkhf    =   sqrt ( tke(ij) )
        vres    =   sqrt ( u(ij)*u(ij) + v(ij)*v(ij) + w(ij)*w(ij) )
        dist    =   0.5 * dst(ij)
        term    =   rho(ij) * cdqtr * tkhf
        denom   =   alog ( ee * dist * term / amu(ij) )
        tau     =   ak * term * vres / denom
        tau1    =   amu(ij) * vres  / dist
        if (tau .lt. 0.0) tau = tau1
        prdn    =   tau * vres * ajb(ij) / dist
        sp(ij)  = - rho(ij) * cdtqtr * tkhf * denom / 
     1              (ak * dist) * ajb(ij)
        su(ij)  =   prdn
        cf1(ij) =   0.0
10     continue
11    continue
C
      return
      end
C======================================================================C
      subroutine xmextr (igrl)
C                                                                      C
C     Purpose:  Extrapolate appropriate boundary conditions to the     C
C               x-minus ( zi-minus ) boundary.                         C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      i = 1
C
      do 100 n = 1, nsxm
          jfl  = jfxm(n,igrl)
	  jll  = jlxm(n,igrl)
C
C.... Enforce WALL boundary conditions.
C
      if ( kbxm(n) .eq. 1 ) then
       do 10 j   = jfl, jll
            ij   = ibeg + i + (j-1) * imaxl
           ij1   = ij + 1
        p   (ij) = p   (ij1)
        rho (ij) = rho (ij1)
        tke (ij) = tke (ij1)
        tde (ij) = tde (ij1)
        amut(ij) = amut(ij1)
        h   (ij) = h   (ij1)
        t   (ij) = t   (ij1)
        f   (ij) = f   (ij1)
        g   (ij) = g   (ij1)
        yfu (ij) = yfu (ij1)
        yo2 (ij) = yo2 (ij1)
        yco2(ij) = yco2(ij1)
        yh2o(ij) = yh2o(ij1)
        yn2 (ij) = yn2 (ij1)
10     continue
C
C.... Enforce INLET boundary condition.
C
      elseif ( kbxm(n) .eq. 2 ) then
       do 20 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij + 1
        p(ij)  = p (ij1) + 0.5 * ( p(ij1) - p(ij+2) )
20     continue
C
C.... Enforce SYMMETRY boundary condition.
C
      elseif ( kbxm(n) .eq. 3 ) then
       do 30 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij + 1
        cx   (ij) = 0.0
        cu   (ij) = 0.0
        cy   (ij) = cy (ij1)
        rho  (ij) = rho(ij1)
        cv   (ij) = cy (ij) / rho(ij)
        p    (ij) = p   (ij1)
        tke  (ij) = tke (ij1)
        tde  (ij) = tde (ij1)
        amut (ij) = amut(ij1)
        h    (ij) = h   (ij1)
        t    (ij) = t   (ij1)
        u    (ij) = u   (ij1)
        v    (ij) = v   (ij1)
        w    (ij) = w   (ij1)
        f    (ij) = f   (ij1)
        g    (ij) = g   (ij1)
        yfu  (ij) = yfu (ij1)
        yo2  (ij) = yo2 (ij1)
        yco2 (ij) = yco2(ij1)
        yh2o (ij) = yh2o(ij1)
        yn2  (ij) = yn2 (ij1)
30     continue
C
C.... Enforce OUTFLOW boundary condition.
C
      elseif ( kbxm(n) .eq. 4 ) then
       do 40 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij + 1
        cx   (ij) = cx (ij1)
        rho  (ij) = rho(ij1)
        cu   (ij) = cx(ij) / rho(ij)
        cy   (ij) = 0.0
        cv   (ij) = 0.0
        p    (ij) = p (ij1) + 0.5 * ( p(ij1) - p(ij+2) )
        tke  (ij) = tke (ij1)
        tde  (ij) = tde (ij1)
        amut (ij) = amut(ij1)
        h    (ij) = h   (ij1)
        t    (ij) = t   (ij1)
        u    (ij) = u   (ij1)
        v    (ij) = v   (ij1)
        w    (ij) = w   (ij1)
        f    (ij) = f   (ij1)
        g    (ij) = g   (ij1)
        yfu  (ij) = yfu (ij1)
        yo2  (ij) = yo2 (ij1)
        yco2 (ij) = yco2(ij1)
        yh2o (ij) = yh2o(ij1)
        yn2  (ij) = yn2 (ij1)
40     continue
      endif
C
100   continue
      return
      end
C======================================================================C
      subroutine xpextr (igrl)
C                                                                      C
C     Purpose:  Extrapolate appropriate boundary conditions to the     C
C               x-plus ( zi-plus ) boundary.                           C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      i = imaxl
C
      do 100 n = 1, nsxp
          jfl  = jfxp(n,igrl)
	  jll  = jlxp(n,igrl)
C
C.... Enforce WALL boundary conditions.
C
      if ( kbxp(n) .eq. 1 ) then
       do 10 j   = jfl, jll
            ij   = ibeg + i + (j-1) * imaxl
            ij1  = ij - 1
        p   (ij) = p   (ij1)
        rho (ij) = rho (ij1)
        tke (ij) = tke (ij1)
        tde (ij) = tde (ij1)
        amut(ij) = amut(ij1)
        h   (ij) = h   (ij1)
        t   (ij) = t   (ij1)
        f   (ij) = f   (ij1)
        g   (ij) = g   (ij1)
        yfu (ij) = yfu (ij1)
        yo2 (ij) = yo2 (ij1)
        yco2(ij) = yco2(ij1)
        yh2o(ij) = yh2o(ij1)
        yn2 (ij) = yn2 (ij1)
10     continue
C
C.... Enforce INLET boundary condition.
C
      elseif ( kbxp(n) .eq. 2 ) then
       do 20 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij - 1
        p(ij)  = p (ij1) + 0.5 * ( p(ij1) - p(ij-2) )
20     continue
C
C.... Enforce SYMMETRY boundary condition.
C
      elseif ( kbxp(n) .eq. 3 ) then
       do 30 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij - 1
        cx   (ij) = 0.0
        cu   (ij) = 0.0
        cy   (ij) = cy  (ij1)
        rho  (ij) = rho (ij1)
        p    (ij) = p   (ij1)
        tke  (ij) = tke (ij1)
        tde  (ij) = tde (ij1)
        amut (ij) = amut(ij1)
        h    (ij) = h   (ij1)
        t    (ij) = t   (ij1)
        u    (ij) = u   (ij1)
        v    (ij) = v   (ij1)
        w    (ij) = w   (ij1)
        f    (ij) = f   (ij1)
        g    (ij) = g   (ij1)
        yfu  (ij) = yfu (ij1)
        yo2  (ij) = yo2 (ij1)
        yco2 (ij) = yco2(ij1)
        yh2o (ij) = yh2o(ij1)
        yn2  (ij) = yn2 (ij1)
30     continue
C
C.... Enforce OUTFLOW boundary condition.
C
      elseif ( kbxp(n) .eq. 4 ) then
       do 40 j = jfl, jll
           ij  = ibeg + i + (j-1) * imaxl
           ij1 = ij - 1
        cx   (ij1) = cx (ij-2)
        cx   (ij)  = cx (ij1)
        rho  (ij)  = rho(ij1)
        cu   (ij)  = cx(ij) / rho(ij)
        cu   (ij1) = cx(ij1) / rho(ij)
        cy   (ij) = 0.0
        cv   (ij) = 0.0
        p    (ij) = p (ij1) + 0.5 * ( p(ij1) - p(ij-2) )
        tke  (ij) = tke (ij1)
        tde  (ij) = tde (ij1)
        amut (ij) = amut(ij1)
        h    (ij) = h   (ij1)
        t    (ij) = t   (ij1)
        u    (ij) = u   (ij1)
        v    (ij) = 0.0
        w    (ij) = w   (ij1)
        f    (ij) = f   (ij1)
        g    (ij) = g   (ij1)
        yfu  (ij) = yfu (ij1)
        yo2  (ij) = yo2 (ij1)
        yco2 (ij) = yco2(ij1)
        yh2o (ij) = yh2o(ij1)
        yn2  (ij) = yn2 (ij1)
40     continue
      endif
C
100   continue
      return
      end
C======================================================================C
      subroutine ymextr (igrl)
C                                                                      C
C     Purpose:  Extrapolate appropriate boundary conditions to the     C
C               y-minus ( eta-minus ) boundary.                        C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      j = 1
C
      do 100 n = 1, nsym
          ifl  = ifym(n,igrl)
	  ill  = ilym(n,igrl)
C
C.... Enforce WALL boundary conditions.
C
      if ( kbym(n) .eq. 1 ) then
       do 10 i   = ifl, ill
            ij   = ibeg + i + (j-1) * imaxl
           ij1   = ij + imaxl
        p   (ij) = p   (ij1)
        rho (ij) = rho (ij1)
        tke (ij) = tke (ij1)
        tde (ij) = tde (ij1)
        amut(ij) = amut(ij1)
        h   (ij) = h   (ij1)
        t   (ij) = t   (ij1)
        f   (ij) = f   (ij1)
        g   (ij) = g   (ij1)
        yfu (ij) = yfu (ij1)
        yo2 (ij) = yo2 (ij1)
        yco2(ij) = yco2(ij1)
        yh2o(ij) = yh2o(ij1)
        yn2 (ij) = yn2 (ij1)
10     continue
C
C.... Enforce INLET boundary condition.
C
      elseif ( kbym(n) .eq. 2 ) then
       do 20 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij + imaxl
          ij2  = ij1 + imaxl
        p(ij)  = p (ij1) + 0.5 * ( p(ij1) - p(ij2) )
20     continue
C
C.... Enforce SYMMETRY boundary condition.
C
      elseif ( kbym(n) .eq. 3 ) then
       do 30 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij + imaxl
        cx   (ij) = cx   (ij1)
        cy   (ij) = 0.0
        cv   (ij) = 0.0
        rho  (ij) = rho  (ij1)
        p    (ij) = p    (ij1)
        tke  (ij) = tke  (ij1)
        tde  (ij) = tde  (ij1)
        amut (ij) = amut (ij1)
        h    (ij) = h    (ij1)
        t    (ij) = t    (ij1)
        u    (ij) = u    (ij1)
        v    (ij) = 0.0 
        w    (ij) = 0.0
        f    (ij) = f    (ij1)
        g    (ij) = g    (ij1)
        yfu  (ij) = yfu  (ij1)
        yo2  (ij) = yo2  (ij1)
        yh2o (ij) = yh2o (ij1)
        yco2 (ij) = yco2 (ij1)
        yn2  (ij) = yn2  (ij1)
30     continue
C
C.... Enforce OUTFLOW boundary condition.
C
      elseif ( kbym(n) .eq. 4 ) then
       do 40 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij + imaxl
          ij2  = ij1 + imaxl
       cx (ij)   = 0.0
       cu (ij)   = 0.0
       rho(ij)   = rho(ij1)
       cy (ij)   = cy(ij1)
       cv (ij)   = cy(ij) / rho(ij)
       p    (ij) = p (ij1) + 0.5 * ( p(ij1) - p(ij2) )
       tke  (ij) = tke  (ij1)
       tde  (ij) = tde  (ij1)
       amut (ij) = amut (ij1)
       h    (ij) = h    (ij1)
       t    (ij) = t    (ij1)
       u    (ij) = u    (ij1)
       v    (ij) = v    (ij1)
       w    (ij) = w    (ij1)
       f    (ij) = f    (ij1)
       g    (ij) = g    (ij1)
       yfu  (ij) = yfu  (ij1)
       yo2  (ij) = yo2  (ij1)
       yh2o (ij) = yh2o (ij1)
       yco2 (ij) = yco2 (ij1)
       yn2  (ij) = yn2  (ij1)
40     continue
      endif
C
100   continue
      return
      end
C======================================================================C
      subroutine ypextr (igrl)
C                                                                      C
C     Purpose:  Extrapolate appropriate boundary conditions to the     C
C               y-plus ( eta-plus ) boundary.                          C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
      include 'UIFlow.indx'
C
      j = jmaxl
C
      do 100 n = 1, nsyp
          ifl  = ifyp(n,igrl)
	  ill  = ilyp(n,igrl)
C
C.... Enforce WALL boundary conditions.
C
      if ( kbyp(n) .eq. 1 ) then
       do 10 i   = ifl, ill
            ij   = ibeg + i + (j-1) * imaxl
           ij1   = ij - imaxl
        p   (ij) = p   (ij1)
        rho (ij) = rho (ij1)
        tke (ij) = tke (ij1)
        tde (ij) = tde (ij1)
        amut(ij) = amut(ij1)
        h   (ij) = h   (ij1)
        t   (ij) = t   (ij1)
        f   (ij) = f   (ij1)
        g   (ij) = g   (ij1)
        yfu (ij) = yfu (ij1)
        yo2 (ij) = yo2 (ij1)
        yco2(ij) = yco2(ij1)
        yh2o(ij) = yh2o(ij1)
        yn2 (ij) = yn2 (ij1)
10     continue
C
C.... Enforce INLET boundary condition.
C
      elseif ( kbyp(n) .eq. 2 ) then
       do 20 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij - imaxl
          ij2  = ij1 - imaxl
        p(ij)  = p (ij1) + 0.5 * ( p(ij1) - p(ij2) )
20     continue
C
C.... Enforce SYMMETRY boundary condition.
C
      elseif ( kbyp(n) .eq. 3 ) then
       do 30 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij - imaxl
        cx   (ij) = cx   (ij1)
        cy   (ij) = 0.0
        cv   (ij) = 0.0
        rho  (ij) = rho  (ij1)
        p    (ij) = p    (ij1)
        tke  (ij) = tke  (ij1)
        tde  (ij) = tde  (ij1)
        amut (ij) = amut (ij1)
        h    (ij) = h    (ij1)
        t    (ij) = t    (ij1)
        u    (ij) = u    (ij1)
        v    (ij) = 0.0 
        w    (ij) = 0.0
        f    (ij) = f    (ij1)
        g    (ij) = g    (ij1)
        yfu  (ij) = yfu  (ij1)
        yo2  (ij) = yo2  (ij1)
        yh2o (ij) = yh2o (ij1)
        yco2 (ij) = yco2 (ij1)
        yn2  (ij) = yn2  (ij1)
30     continue
C
C.... Enforce OUTFLOW boundary condition.
C
      elseif ( kbyp(n) .eq. 4 ) then
       do 40 i = ifl, ill
           ij  = ibeg + i + (j-1) * imaxl
          ij1  = ij  - imaxl
          ij2  = ij1 - imaxl
        cx   (ij) = 0.0
        cu   (ij) = 0.0
        rho  (ij) = rho(ij1)
        cy  (ij1) = cy (ij2)
        cy   (ij) = cy (ij1)
        cv  (ij1) = cy (ij1) / rho(ij)
        cv   (ij) = cv (ij1)
        p    (ij) = p (ij1) + 0.5 * ( p(ij1) - p(ij2) )
        tke  (ij) = tke  (ij1)
        tde  (ij) = tde  (ij1)
        h    (ij) = h    (ij1)
        t    (ij) = t    (ij1)
        u    (ij) = u    (ij1)
        v    (ij) = v    (ij1)
        w    (ij) = w    (ij1)
        f    (ij) = f    (ij1)
        g    (ij) = g    (ij1)
        yfu  (ij) = yfu  (ij1)
        yo2  (ij) = yo2  (ij1)
        yh2o (ij) = yh2o (ij1)
        yco2 (ij) = yco2 (ij1)
        yn2  (ij) = yn2  (ij1)
40     continue
      endif
C
100   continue
      return
      end
C======================================================================C
      subroutine zero
C                                                                      C
C     Purpose:  Set all variables equal to zero.                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      do 20 igrl = 1,ngrid
       nitn(igrl) = 0
       do 11 nv   =  1, 11
        error(igrl,nv) = 0.0
11     continue
C
       if = 1 + nbeg(igrl)
       il = nbeg(igrl) + imax(igrl) * jmax(igrl)
       do 10 i = if, il
        u    (i) = 0.0
        v    (i) = 0.0
        w    (i) = 0.0
        p    (i) = 0.0
        h    (i) = 0.0
        t    (i) = 0.0
        cph  (i) = 0.0
        pp   (i) = 0.0
        rho  (i) = 0.0
        gam  (i) = 0.0
        tke  (i) = 0.0
        tde  (i) = 0.0
        f    (i) = 0.0
        g    (i) = 0.0
        yfu  (i) = 0.0
        yo2  (i) = 0.0
        yn2  (i) = 0.0
        yco2 (i) = 0.0
        yh2o (i) = 0.0
        x    (i) = 0.0
        y    (i) = 0.0
        ajb  (i) = 0.0
        a11  (i) = 0.0
        a12  (i) = 0.0
        a21  (i) = 0.0
        a22  (i) = 0.0
        q11  (i) = 0.0
        q12  (i) = 0.0
        q21  (i) = 0.0
        q22  (i) = 0.0
        fx   (i) = 1.0
        fy   (i) = 1.0
        fx1  (i) = 1.0
        fy1  (i) = 1.0
        dux  (i) = 0.0
        duy  (i) = 0.0
        dvx  (i) = 0.0
        dvy  (i) = 0.0
        cu   (i) = 0.0
        cv   (i) = 0.0
        cx   (i) = 0.0
        cy   (i) = 0.0
        dx   (i) = 0.0
        dy   (i) = 0.0
        aw   (i) = 0.0
        ae   (i) = 0.0
        as   (i) = 0.0
        an   (i) = 0.0
        ap   (i) = 1.0
        apm  (i) = 1.0
        app  (i) = 1.0
        apu  (i) = 0.0
        apv  (i) = 0.0
        su   (i) = 0.0
        sfu  (i) = 0.0
        sp   (i) = 0.0
        resu (i) = 0.0
        resv (i) = 0.0
        resux(i) = 0.0
        resvx(i) = 0.0
        rs   (i) = 0.0
        dpdx (i) = 0.0
        dpdy (i) = 0.0
        amu  (i) = 0.0
        amut (i) = 0.0
        prod (i) = 0.0
        x1   (i) = 0.0
        x2   (i) = 0.0
        y1   (i) = 0.0
        y2   (i) = 0.0
10     continue
20    continue
C
      return
      end
C======================================================================C
      subroutine xwcommon
C                                                                      C
C     Purpose:  Set all variables equal to zero.                       C
C                                                                      C
C======================================================================C
      include 'UIFlow.com'
C
      open(99, file='TRACE.INPUT' ,access='sequential',status='unknown')
      write (99, *) 'klam +++'
      write (99, *) klam,kcomp, kswrl, kpgrid, model
      write (99, *) 'kfuel +++'
      write (99, *) kfuel, knorth, kplax, kadj
      write (99, *) 'ngrid +++'
      write (99, *) ngrid, ncelx, ncely
C	  
      write (99, *) '+++ Left Side +++'
      write (99, *) nsxm
      do 10 n = 1, nsxm
        write (99, *) kbxm(n), jfxm, jlxm
        write (99, *) ubxm(n), vbxm(n), wbxm(n), dvar, txm(n)
        write (99, *) rhxm(n), fxm(n), gxm(n), tkxm(n), tdxm(n)
        write (99, *) fuxm(n), co2xm(n), h2oxm(n), o2xm(n), wmxm(n)
10    continue
C	  
      write (99, *) '+++ Right Side +++'
      write (99, *) nsxp
      do 20 n = 1,nsxp
        write (99, *) kbxp (n), jfxp, jlxp
        write (99, *) ubxp(n), vbxp(n), wbxp(n), dvar, txp(n)
        write (99, *) rhxp(n), fxp(n), gxp(n), tkxp(n), tdxp(n)
        write (99, *) fuxp(n), co2xp(n), h2oxp(n), o2xp(n), wmxp(n)
20    continue
C	  
      write (99, *) '+++ Bottom Side +++'
      write (99, *) nsym
      do 30 n = 1,nsym
        write (99, *) kbym (n), ifym, ilym
        write (99, *) ubym(n), vbym(n), wbym(n), dvar, tym(n)
        write (99, *) rhym(n), fym(n), gym(n), tkym(n), tdym(n)
        write (99, *) fuym(n), co2ym(n), h2oym(n), o2ym(n), wmym(n)
30    continue
C	  
      write (99, *) '+++ Top Side +++'
      write (99, *) nsyp
      do 40 n = 1,nsyp
        write (99, *) kbyp(n), ifyp, ilyp
        write (99, *) ubyp(n), vbyp(n), wbyp(n), dvar, typ(n)
        write (99, *) rhyp(n), fyp(n), gyp(n), tkyp(n), tdyp(n)
        write (99, *) fuyp(n), co2yp(n), h2oyp(n), o2yp(n), wmyp(n)
40    continue
C	  
      write (99, *) '+++ Interior +++'
      write (99, *) ugs, vgs, wgs, rhgs
      write (99, *) tgs, tkgs, tdgs, fgs, ggs, fugs
      write (99, *) tfuel, tair
      write (99, *) (prl(nv), nv=1, 11)
      write (99, *) (prt(nv), nv=1, 11)
      write (99, *) (relx(nv), nv=1, 11)
      write (99, *) (nswp(nv) , nv=1, 11)
      write (99, *) (iprint(i), i=1 , 12)
      write (99, *) pref, vscty
      write (99, *) maxitn, tolr(ngrid)
      write (99, *) rin
      write (99, *) nxbaf, nybaf, nobs
C
	  write (99,*) cd,cdqtr,cdtqtr,ee,ak
	  write (99,*) refu,refv,refw,refc
	  write (99,*) ind1,ind2,ind3,ind4
      close(UNIT = 99, status='keep')
	  return
      end
	  