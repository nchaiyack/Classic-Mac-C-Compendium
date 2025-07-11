/*
 *
 * loan.c
 * @(#) loan.c amoritization program
 * 
 * acronym:    loan amortization program
 *             ----
 *
 * purpose:	calculates monthly payment, future value
 *		and creates an amortization schedule
 *
 * 06/27/84	Bill Gregg, Informatics General Corp.
 * 07/12/84     Revision 1
 * 07/12/84	Revision 2
 * 11/05/85	Jane Medefesser, Implementation NPSN Unix (wilbur)
 *		compile as follows: 'cc -o <outfile> loan.c -lm'
 * 12/05/85	Changes to direct output to a file.
 * 03/02/88	Implemented on Eternity 5.3.1
 *
 */

#include <stdio.h>
#include <math.h>

/*
 *
 */
 
main() 		/* loan program */
{
	float amt, term, rate, ic;
	float r, temp, pmt, fv;
	float exp, prin, x, y, mbeg, mnbr, yrint = 0;
	int month, i, k, a = 0, yr=1;
	char d, filename[9], c;
	FILE *fp;
	/*  prompt for input from terminal  */

	printf("Enter principal amount: ");
	scanf("%f", &amt);

	printf("Enter term in years: ");
	scanf("%f", &term);

	printf("Enter interest rate in percent: ");
	scanf("%f", &rate);

	printf("Enter month payments begin (ex: 8 = August): ");
	scanf("%f", &mbeg);

	/*  compute payment and future value  */

	r = rate/100.0;
	x = 1.0 + r/12.0;
	y = term*12.0;
	temp = (1.0 / pow(x,y));
	pmt = (amt*r/12.0) / (1-temp);
	k = term*12;
	fv = pmt*k;
	ic = fv - amt;

	printf("Do you want the report directed to a file or to the screen?");
	printf("\n[f = file / s = screen] : ");
	d = getchar();      /* This is only here to eat up the '\n' left over
			       from the last scanf. */
	d = getchar();
	switch(d) {
	     case 'f':
	     d = getchar();
	     printf("\nEnter a filename: ");
	     scanf("%s", filename);
	     while((c = getchar()) != '\n') {
	     filename[a] = c; a++; }
	     fp = fopen(filename, "w");
	     break;
	     default:
	     fp = stdout;
	     break;
	     }

	     /*  print header  */
  
	   fprintf(fp,"\n\t *** Amortization Schedule ***\n\n");
	   fprintf(fp,"Principal:  %.2f\n", amt);
	   fprintf(fp,"Future value:  %.2f\n", fv);
	   fprintf(fp,"Term of loan in years:  %.1f\n", term);
	   fprintf(fp,"Interest Rate:  %3.3f\n", rate);
	   fprintf(fp,"Total Interest Charge:  %.2f\n", ic);
	   fprintf(fp,"Payment:  %.2f\n", pmt);
	   fprintf(fp,"\nMONTH\tPRINCIPAL\tINTEREST\tBALANCE\n");
  
	   /* start of loop to print amortization schedule */
  
	   mnbr=mbeg;
	   for (i=1; i<=k; i++) {
	      month = i;
	      exp = amt*(r/12.0);
	      yrint=yrint+exp;
	      prin = pmt-exp;
	      amt = amt-prin;
	      mnbr++;
	      fprintf(fp,"%d\t %.2f\t\t %.2f\t\t %.2f\n", month, prin, exp, amt);
	      if (mnbr > 12 ) {
	      fprintf(fp,"\t\tInterest paid for year %d is %.2f\n\n",yr,yrint);
	      yr++;
	      yrint=0;
	      mnbr=1;
              }
}
	     fprintf(fp,"\t\tInterest paid for year %d is %.2f\n\n",yr,yrint);
	     fclose(fp);
}


