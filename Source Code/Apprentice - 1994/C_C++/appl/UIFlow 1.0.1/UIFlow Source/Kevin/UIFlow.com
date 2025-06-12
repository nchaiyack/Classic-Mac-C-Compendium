      common/index/ kcomp, klam, knorth, kplax, ksolve(20),
     1              kswrl, kvisc, model, ksc, kadj, kgrav,
     2              kfuel, kpgrid
      common/input/ relx(11), tolr(5), nswp(11), iprint(12), nitn(5),
     1              nbeg(5) , imax(5), jmax(5), nsc(11) , ncelx  ,
     2              ncely   , ngrid  , niter  , nitr(5) , niterv ,
     3              nitke   , maxitn , rin
      common/const/ wrkunt, rdfctr, beta, bta
      common/xbndry/nsxm, nsym, nsxp, nsyp,
     1              kbxm(5), kbxp(5), kbym(5), kbyp(5), 
     2              jfxm(5,5), jlxm(5,5), jfxp(5,5), jlxp(5,5),
     3              ubxm(5), vbxm(5), wbxm(5), tkxm(5), tdxm(5),rhxm(5),
     4              txm (5), fxm (5), gxm (5) ,fuxm(5), sxm(5), pbxm(5),
     5              ubxp(5), vbxp(5), wbxp(5), tkxp(5), tdxp(5),rhxp(5),
     6              txp (5), fxp (5), gxp (5) ,fuxp(5), sxp(5) ,pbxp(5),
     7              co2xm(5), h2oxm(5), o2xm(5),
     8              co2xp(5), h2oxp(5), o2xp(5) 
      common/ybndry/ifym(5,5), ilym(5,5), ifyp(5,5), ilyp(5,5),
     1              ubym(5), vbym(5), wbym(5), tkym(5), tdym(5),rhym(5),
     2              tym (5), fym (5), gym (5) ,fuym(5), sym(5), pbym(5),        
     3              ubyp(5), vbyp(5), wbyp(5), tkyp(5), tdyp(5),rhyp(5),
     4              typ (5), fyp (5), gyp (5) ,fuyp(5), syp(5) ,pbyp(5),
     7              co2ym(5), h2oym(5), o2ym(5), 
     8              co2yp(5), h2oyp(5), o2yp(5),
     5              wmym(5), wmyp(5), wmxm(5), wmxp(5)
      common/ref/   refu, refv, refw, refc, pref
      common/guess/ ugs , vgs , wgs  , rhgs , tkgs, tdgs , tgs  , fgs,
     1              ggs , fugs, sgs  , o2gs , co2gs, h2ogs
      common/geom/  a11(3500) , a12(3500)  , a21(3500), a22(3500),
     1              q11(3500) , q12(3500)  , q21(3500), q22(3500),
     2              x (3500)  , y (3500)   , r (3500) , ajb(3500),
     3              fx (3500) , fx1(3500)  , fy (3500), fy1(3500),
     4              dux(3500) , duy(3500)  , dvx(3500), dvy(3500),
     5              xxic(3500), yetac(3500)
      common/prop/  gascon, airt, airv, airs, airsum, vscty,
     1              flxin, gamma, wmol(3500) ,amu(3500),amut(3500), 
     2              cph(3500)
      common/coef/  ae (3500)  , aw (3500), an (3500), as (3500),
     1              ap (3500)  , apm(3500), apu(3500), apv(3500),
     2              app(3500)
      common/sorce/ su (3500)  , sp (3500), sfu(3500)
      common/var1/  u (3500) , v (3500)  , w (3500)   , p (3500) ,
     1              pp (3500), h (3500)  , t (3500)   , rho(3500),
     2              cu (3500), cv (3500) , cx (3500)  , cy (3500),
     3              tke(3500), tde(3500) , f (3500)   , yfu(3500), 
     5              yh2o(3500),g  (3500), yo2(3500),    yn2(3500),
     6              yco2(3500)
      common/var2/  gam(3500) , dx(3500), dy(3500), dpdx(3500),
     1              dpdy(3500), c (3500), iru(3500), x1(3500),
     2              x2(3500)  , y1(3500), y2(3500) , x3(3500)
      common/res/   resu(3500) , resv(3500)  , rs(3500) ,
     1              error(7,11), resids(7,11), residu(7), residv(7)  ,
     2              residw(7)  , reside(7)   , residk(7), resux(3500),
     3              resvx(3500)
      common/turb/  cd , cdqtr, cdtqtr, ee , ak , prt(11), prl(11),
     1              ce1, ce2  , cg1   , cg2, prod(3500)
      common/cpht/  acpox , bcpox , ccpox , acpn2 , bcpn2 , ccpn2 ,
     1              acpco2, bcpco2, ccpco2, acph2o, bcph2o, ccph2o,
     2              acpfu , bcpfu , ccpfu , acpair, bcpair, ccpair,
     3              acpprd, bcpprd, ccpprd
      common/molw/  wmox  , wmfu  , wmn2  , wmh2o , wmair , cxx   ,
     1              hyy   , wmco2 , wmpr
      common/react/ rat(5)  , prexp(1), aa(1)   , bb(1) , ab(1) , cr ,
     1              hfu     , hox     , hco2    , hn2   , hh2o  ,
     2              acten(1), rox     , hffu    , hfco2 , hfh2o ,
     3              hreact  , enthfu  , enthox  , phifma, phia  ,
     4              stoic   , grav    , tfuel   , tair  , fstoic,
     5              ysub    , alfrho  , alft    , tprd  , rprd
      common/log/   prolng, restrc, relax, upleg
      common/dummy/ rad(21), trbke(21), axvel(21), wvel(21), ind1,
     1              ind2, ind3, ind4
      common/mine/  dvar, nxbaf, nybaf, nobs
      logical  prolng, restrc, relax, upleg
