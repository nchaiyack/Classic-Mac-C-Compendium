/* This LuminanceRecord1.h file is a description of a monitor produced by the CalibrateLuminance program. */
/* This file may be #included in any C program. It simply fills in a luminanceRecord data structure. */
/* The theory is described by Pelli & Zhang (1991). The data structure is defined in Luminance.h.  */
/* CalibrateLuminance and Luminance.h are part of the VideoToolbox software. */
/* Pelli & Zhang (1991) Accurate control of contrast on microcomputer displays. */
/* Vision Research, 31:1337-1360. */
/* The VideoToolbox software is available, free for research purposes, from Denis Pelli. */
/* Institute for Sensory Research, Syracuse University, Syracuse, NY 13244-5290. */
LR.screen=1;	/* device=GetScreenDevice(LR.screen); */
/* Caution: the screen number used here and in GetScreen Device is NOT the same as */
/* displayed by the Monitors cdev in the Control Panel. Sorry. The most obvious difference */
/* is that GetScreenDevice always assigns 0 to the main screen, the one with the menu bar. */
LR.date="3:09 PM Friday, October 16, 1992";
LR.id="5111767";
LR.name="signal";
LR.notes="manoj, lights off, photometer 1 inch from screen";
LR.dpi=76.0;	/* pixels per inch */
LR.Hz=66.67;	/* frames per second */
LR.units="cd/m^2";
/* coefficients of polynomial fit */
LR.coefficients=9;	/* # of coefficients in polynomial fit */
/* L(V)=p[0]+p[1]*V+p[2]*V*V+ . . . �polynomialError */
LR.p[0]=1.28201e-14;
LR.p[1]=8.25776e-13;
LR.p[2]=5.00064e-11;
LR.p[3]=2.51138e-09;
LR.p[4]=7.98055e-08;
LR.p[5]=-2.04184e-09;
LR.p[6]=1.79181e-11;
LR.p[7]=-6.3791e-14;
LR.p[8]=8.10888e-17;
LR.polynomialError=  0.2263;	/* RMS error of fit */
/* coefficients of quadratic fit */
/* L(V)=q[0]+q[1]*V+q[2]*V*V�quadraticError */
LR.q[0]=4.86285;
LR.q[1]=-0.201266;
LR.q[2]=0.00125998;
LR.quadraticError=  2.5493;	/* RMS error of fit */
/* coefficients of power law fit */
/* L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]�powerError */
/* where Rectify(x)=x if x�0, and Rectify(x)=0 otherwise */
/* Pelli & Zhang (1991) Eqs.9&10 use symbols v=V/255, alpha=power[0], beta=power[1], kappa=power[2]*255, gamma=power[3] */
LR.power[0]=-0.0615421;
LR.power[1]=-3.38554;
LR.power[2]=0.0305017;
LR.power[3]=2.48866;
LR.powerError=  0.2616;	/* RMS error of fit */
/* coefficients of power law fit, with fixed exponent */
/* L(V)=fixedPower[0]+Rectify(fixedPower[1]+fixedPower[2]*V)^fixedPower[3]�fixedPowerError */
LR.fixedPower[0]=-0.764058;
LR.fixedPower[1]=-2.96785;
LR.fixedPower[2]=0.0310164;
LR.fixedPower[3]=  2.28;
LR.fixedPowerError=  1.1986;	/* RMS error of fit */
LR.r=0.0282406;
LR.g=0.149883;
LR.b=0.821877;
LR.gainAccuracy=-0.0337905;
LR.gm=3.85543;	/* The monitor's contrast gain. */
LR.VMin=  0;	/* minimum value that can be loaded into DAC */
LR.VMax=255;	/* maximum value that can be loaded into DAC */
LR.LMin=   -0.06;	/* luminance at VMin */
LR.LMax=   39.70;	/* luminance at VMax */
LR.LBackground=   9.731;	/* background luminance during calibration */
LR.VBackground=193;	/* background number used during calibration */
LR.rangeSet=0;	/* indicate that range parameters have yet to be set */
LR.L.exists=0;	/* indicate that luminance table has yet to be initialized */
