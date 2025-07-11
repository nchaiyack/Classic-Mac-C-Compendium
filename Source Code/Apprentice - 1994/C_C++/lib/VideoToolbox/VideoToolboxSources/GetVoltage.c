/*
GetVoltage.c
� 1990,1991,1992 Denis G. Pelli
These subroutine are for measuring voltages via the Data Translation DT2211-PGL
FORERUNNER analog NuBus card. The assumed unit-gain input range is VMIN to VMAX,
e.g. 0 to 5 Volts. If you change this input range (by changing the jumpers on the card)
then these defined constants should be changed appropriately. 

The on-board FIFO holds 1024 samples, so you can use InitiateVoltageSampling() to
start the conversion, do something else, and come back later to collect the results,
by calling RetrieveVoltages(). All the other routines are built upon these two.

BUGS:
RetrieveVoltages() hangs up at high clock rates and high gains (e.g. 100 kHz at gain 1
or 10 kHz at gain 500). This is almost certainly some sort of hardware problem.
Perhaps Data Translation could advise a work-around. What happens is that
we wait forever for the Control and Status Register to indicate that there is
valid data. (It checks before getting each of the n samples.)
Since the clock is presumably still running the data ought to appear
eventually, even if the a/d can't quite keep up. Anyway, I can do all my work at
2 kHz or lower, so it's not an issue for me.***6/23/92 This problem now occurs
even at 2 kHz, but it may be an interaction with the Radius Rocket.

I don't understand why, but the THINK C profiler timing data indicates that 
RetrieveVoltages() is much too fast on the cases where a voltageOverflow occurs.
The most likely explanation for this is some sort of glitch in the Profiler, probably
an interaction with printf, so I'm ignoring it.

HISTORY:
8/8/89	dgp	written by denis,based on an early draft by Preeti
9/29/89 dgp	changed the #defines so that slot number appears in only one place.
3/28/90 dgp	call the new function CardSlot() that actually finds the ForeRunner card
			on the NuBus if present, and fail gracefully if it's absent.
9/13/90	dgp	Added standard deviation to GetVoltage(). Tidied up the comments.
9/14/90	dgp	Broke up code into separate routines that initiate the sampling, and
			later retrieve the samples.
			Improved the error detection, handling, and reporting.
			Made explicit and general the dependence on the unit-gain input voltage range.
			Elaborated the main() test driver.
			Sped up the key loop in RetrieveVoltages().
10/17/90 dgp removed unused variables from InitiateVoltageSampling() and
			RetrieveVoltages().
5/21/91	dgp	GetVoltage() now automatically tries again at lower gain if there was
			a voltage overflow. I also added the voltage limits to the diagnostic
			messages.
8/24/91		dgp	Made compatible with THINK C 5.0.
6/22/92	dgp	added a brief pause between successive interrogations of the fifo when
			fetching data, to prevent hang ups. I suspect that the problem
			is that the fifo's input and output are not totally independent and the
			excessively frequent interrogation of the output (waiting for the data
			available flag to come up) prevented the fifo from accepting data.
6/23/92	dgp	well, the last fix helped but didn't solve the problem. I now explicitly
			time out if the next data point doesn't appear after a reasonable wait
			(100000 iterations). I modified the calling structure of GetVoltages and
			RetrieveSamples to pass the number of samples by reference so that
			when there is a buffer overflow or timeout the programs can proceed
			using the reduced number of data actually retrieved.
10/23/92 dgp removed obsolete support for THINK C 4. Fine tuned RetrieveVoltages()
			to make it as fast as possible. Removed all the beeps in the error
			messages, because they annoyed me.
*/
#include "VideoToolbox.h"
#include <math.h>

#define VMAX 5.0	/* unit-gain input range of a/d assumed to be VMIN to VMAX volts */
#define VMIN 0.0

#define BASE_ADDR(slot) 	(0xf0080000+(slot)*0x1100000)
#define CS_REG 		0x00
#define	AD_DATA		0x10
#define DAC_0_DATA	0x20
#define DAC_1_DATA	0x30
#define	DIO_COUNTER 0x40
#define DIO_DATA	0x50

#define GAINS 4
static double gain[GAINS]={1.,10.,100.,500.};
#define FREQUENCIES 64
static double frequency[FREQUENCIES]={
	0.005,0.006,0.010,0.012,0.015,0.020,0.030,
	0.050,0.060,0.060,0.100,0.120,0.150,0.200,0.300,
	0.500,0.600,0.600,1.0,1.200,1.500,2.0,3.0,
	5.0,6.0,6.0,10.0,12.0,15.0,20.0,30.0,
	50.0,60.0,60.0,100.0,120.0,150.0,200.0,300.0,
	500.0,600.0,600.0,1000.0,1200.0,1500.0,2000.0,3000.0,
	5000.0,6000.0,6000.0,10000.0,12000.0,15000.0,20000.0,30000.0,
	50000.0,60000.0,60000.0,100000.0,120000.0,150000.0,200000.0,300000.0,600000.0
};
static short frequencyCode[FREQUENCIES]={
	63,15,55,47,39,31,23,62,14,7,54,46,38,30,22,
	61,6,13,53,45,37,29,21,60,12,5,52,44,36,28,20,
	59,11,4,51,43,35,27,19,58,3,10,50,42,34,26,18,
	57,9,2,49,41,33,25,17,56,1,8,48,40,32,24,16,0
};

#if 0
#include <profile.h>
void main(void);

void main()
{
	int i,k;
	short channel=1;
	long n=2000;
	static double g=1.0,f=2000.,sd,voltDelta,voltZero,mean;
	static unsigned short data[2000];
	
	Require(0);
	for(i=0;i<3;i++)printf("%6.2f mV\n",1000.*VoltsDuringFrame(20));
	InitProfile(200,3);
	for(k=0;k<3;k++){
		g=gain[k];
		mean=GetVoltage(channel,&g,&f,n,&sd);
		printf("n %ld,mean %f V,sd %f V,g %3.0f,f %.3f",n,mean,sd,g,f);
		mean=GetVoltages(channel,&g,&f,n,data,&voltDelta,&voltZero);
		printf(",range is %.3f to %.3f V\n",voltZero,voltDelta*4095+voltZero);
		for(i=0;i<10;i++)printf("%5d",data[i]);
		printf("\n");
	}
}
#endif

double VoltsDuringFrame(double frames)
/* sample channel 1 at 2 kHz for the specified number of 15 ms frames */
{
	double gain=10.;
	double frequency=2000.;
	long n;
	short channel=1;
	
	n=(long)(0.5+frequency*0.015*frames);
	return GetVoltage(channel,&gain,&frequency,n,NULL);
}

double GetVoltage(short channel,double *gainPtr,double *frequencyPtr,long nDesired
	,double *sdPtr)
/*
Samples at specified rate, gain, and number of samples and returns the mean voltage.
If sdPtr is not NULL, then the standard deviation is computed and returned in *sdPtr.
*/
{
	unsigned short *readingsPtr;
	double voltDelta,voltZero,v;
	int error,i;
	register unsigned long sum,m;
	register double sumSquares;
	long n;
	
tryAgain:
	if(nDesired >= 1L<<20){
		printf("GetVoltage: n %ld too large\n",nDesired);
		return NAN;
	}
	readingsPtr=(unsigned short *)NewPtr(nDesired*sizeof(unsigned short));
	if(readingsPtr==NULL){
		printf("GetVoltage: out of memory!\n");
		return NAN;
	}
	n=nDesired;
	error=GetVoltages(channel,gainPtr,frequencyPtr,&n,readingsPtr,&voltDelta,&voltZero);
	if(error & voltageBufferOverflow){
		printf("GetVoltage: Warning: retrieved only %4ld of the %4ld samples requested.\n",n,nDesired);
	}
	if(error & voltageOverflow){
		if(*gainPtr>gain[0]){
			printf("GetVoltage: voltageOverflow >%.2f V at gain %.0f. "
				"Trying again at lower gain.\n",VMAX/(*gainPtr),*gainPtr);
			*gainPtr /=10.0;
			goto tryAgain;
		}
		else printf("GetVoltage: voltageOverflow >%.2f V at gain %.0f.\n"
			,VMAX/(*gainPtr),*gainPtr);
	}
	if(error & voltageUnderflow)printf("GetVoltage: voltageUnderflow <%.3f V.\n"
		,VMIN/(*gainPtr));
	sum=0;
	for(i=0;i<n;i++) sum+=readingsPtr[i];
	v=(double)sum/(double)n;
	if(sdPtr!=NULL){
		sumSquares=0.0;
		for(i=0;i<n;i++){
			m=readingsPtr[i];
			sumSquares+=m*m;
		}
		*sdPtr=voltDelta*sqrt((sumSquares-n*v*v)/(n-1));
	}
	DisposPtr((Ptr)readingsPtr);
	return voltDelta*v+voltZero;
}

short GetVoltages(short channel,double *gainPtr,double *frequencyPtr,long *nPtr
	,unsigned short readings[],double *voltDeltaPtr,double *voltZeroPtr)
/*
Retrieves *nPtr samples from the a/d. The samples are placed in the array
readings[]. The returned value of *voltDeltaPtr, when multiplied by the samples,
will yield volts. The function uses a/d gain and frequency settings as close as
possible to those requested, and sets the parameters to the values actually
used. The returned value will be 0 when the operation was successful, and
nonzero if there was an error. The error value is the OR of those that occured:
voltageBufferOverflow, voltageUnderflow, voltageOverflow. In case of buffer
overflow the value of *nPtr will be reduced to the number of good samples.
*/
{
	int error=0;
	
	error |= InitiateVoltageSampling(channel,gainPtr,frequencyPtr,voltDeltaPtr,voltZeroPtr);
	error |= RetrieveVoltages(nPtr,readings);
	return error;
}


short InitiateVoltageSampling(short channel,double *gainPtr,double *frequencyPtr
	,double *voltDeltaPtr,double *voltZeroPtr)
/*
Initiates sampling by the a/d at specified gain and frequency (in Hz). The
samples accumulate in the FIFO, which will hold 1024 before overflowing. Each
sample multiplied by *voltDeltaPtr and added to *voltZeroPtr is the input
voltage. This function will use gain and frequency settings as close as possible
to those requested, and then set the parameters to the values actually used. The
returned value is always 0.
*/
{
	register volatile short *Con_Stat_Reg;
	unsigned volatile short *AD_Data_Reg;
	unsigned volatile short *DIO_Counter_Reg;
	register short i;
	short mode;
	short iGain,iFrequency;
	static int slot;
	
	slot=ForeRunnerSlot();			/* NuBus slot of ForeRunner card */
	if(slot<0){
		PrintfExit("InitiateVoltageSampling: Sorry, I need a Data Translation ForeRunner A/D card.\n");
	}

	AD_Data_Reg = (unsigned short *)(AD_DATA+BASE_ADDR(slot));
	Con_Stat_Reg = (short *)(CS_REG+BASE_ADDR(slot));
	DIO_Counter_Reg = (unsigned short *)(DIO_COUNTER+BASE_ADDR(slot));

	/* find nearest gain */
	for(i=0;i<GAINS-1;i++) if(*gainPtr <= (gain[i]+gain[i+1])/2.0) break;
	iGain=i;
	*gainPtr=gain[iGain];
	*voltDeltaPtr=(VMAX-VMIN)/4095.0/gain[iGain];
	*voltZeroPtr=VMIN/gain[iGain];
	
	/* find nearest frequency */
	for(i=0;i<FREQUENCIES-1;i++)
		if(*frequencyPtr <= (frequency[i]+frequency[i+1])/2.0) break;
	iFrequency=i;
	*frequencyPtr=frequency[iFrequency];
	
	/* This follows the steps suggested in the FORERUNNER manual pages:192,194,195. */
	*Con_Stat_Reg = (iGain<<6)+channel;				/* mode zero: stop */
	*DIO_Counter_Reg=0;								/* stop the clock */
	while(*Con_Stat_Reg<0) i=*AD_Data_Reg;			/* empty the FIFO */
	*DIO_Counter_Reg=frequencyCode[iFrequency];		/* set clock frequency */
	mode=1;											/* enable */
	*Con_Stat_Reg=(1<<12)+(mode<<8)+(iGain<<6)+channel; /* clear a/d error flag */
	*AD_Data_Reg=0;									/* start */
	return 0;
}

short RetrieveVoltages(long *nPtr,unsigned short readings[])
/*
Retrieves *nPtr samples from the a/d. The samples are placed in the array
readings[]. The returned value will be 0 when the operation was successful, and
nonzero if there was an error. The error value is the OR of those that occured:
voltageBufferOverflow, voltageUnderflow, voltageOverflow. In case of buffer 
overflow the value of *nPtr will be reduced to the number of good samples.
*/
{
	register volatile short *Con_Stat_Reg;
	register volatile unsigned short *AD_Data_Reg;
	register unsigned short *readingsPtr;
	register long i,j,nDesired=*nPtr;
	int slot;						/* NuBus slot of ForeRunner card */
	int error=0;
	register unsigned short r;
	double a;
	
	slot=ForeRunnerSlot();
	if(slot<0){
		PrintfExit("RetrieveVoltages: Sorry, I need a Data Translation ForeRunner A/D card.\n");
	}

	AD_Data_Reg = (unsigned short *)(AD_DATA+BASE_ADDR(slot));
	Con_Stat_Reg = (short *)(CS_REG+BASE_ADDR(slot));

	/* retrieve data as quickly as possible, before the FIFO overflows */
	readingsPtr=&readings[0];
	for(i=0;i<nDesired;i++) {
		j=100000;
		while(*Con_Stat_Reg>=0) if(--j==0)goto timeout;
		*readingsPtr++ = *AD_Data_Reg;
	}
	if(0){
	timeout:
//		printf("RetrieveVoltages timed out waiting for %d-th datum of %ld. ",i,nDesired);
//		printf("*Con_Stat_Reg == 0x%X\n",*Con_Stat_Reg);
		*nPtr=i;									/* use only the valid data */
		for(;i<nDesired;i++)*readingsPtr++=0;
		error|=voltageBufferOverflow;
	}
	*Con_Stat_Reg &= ~(3<<8); 						/* disable conversions */

	/* if we want more than the FIFO can hold, then check for FIFO overflow */
	r=*Con_Stat_Reg;				// force THINK C to read a word, not a byte
	if(*nPtr>1024 && r & 1<<11){
		error|=voltageBufferOverflow;
		*nPtr=1024;									/* use only the valid data */
	}
	while(*Con_Stat_Reg<0)i=*AD_Data_Reg;			/* now empty the FIFO */
	
	/* check for voltage out of range */
	for(i=0;i<*nPtr;i++){
		r=readings[i];
		if(r==0)error|=voltageUnderflow;
		if(r==4095)error|=voltageOverflow;
	}
	return error;
}

int ForeRunnerSlot(void)
/* caches the slot number to speed up subsequent calls */
{
	static gotIt=0;
	static slot=-1;
	
	if(!gotIt) {
		slot=CardSlot(".ForeRunner");
		gotIt=1;
	}
	return slot;
}

