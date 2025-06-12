/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	calc	Recursive calculation routine.  Tries to preserve memory by
			not expanding constants until the last minute.
			Heuristically traverses to the right first to minimize
			memory usage.  Allocates left and right buffers as autos,
			but forces malloc to allocate all arrays which are used for
			calculations.  When temporary space is done with, frees the
			memory required to do the calcs.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
calc(n, a)
		node		*n;
		WORKING 	*a;
{
		WORKING 	d,	e;
		void		fn_one(),	fn_two(),	ErrorAlert();
	
		if (calcidle())				/*  check for user abort */
		   {a->stype = SSTOP;
		 	return;
		   }
	
		e.stype		= SBLANK;
		e.talloc	= 0;
		e.dat		= NULL;
	
		d.stype		= SBLANK;
		d.talloc	= 0;
		d.dat		= NULL;

		a->talloc	= 0;
		a->dat		= NULL;
	
		if (n->right)	calc(n->right,&e);
		if (n->left)	calc(n->left, &d);
		
		if 		(e.stype == SSTOP || d.stype == SSTOP)	a->stype = SSTOP;
		else if (e.stype == SERR  || d.stype == SERR)	a->stype = SERR;
		else if (n->token == TFLOAT)
				{a->stype = SCONST;
				 a->cval  = n->constv;
				}
		else if ( n->token == TIDENT )
				{struct Mwin *tw,*findvar();
			 	 if (NULL != (tw = findvar(n->var)))
			    	{a->stype	= SARRAY;
					 a->dat		= tw->dat;	/* borrow record from window */
				 	 a->dimx	= a->dat->xdim;
				 	 a->dimy	= a->dat->ydim;
				 	 a->talloc 	= 0;
					}
			 	 else
			    	  {a->stype	= SERR;		/* indicate error condition */
				 	   a->dat	= NULL;
					   ErrorAlert(GetResource('STR ',1015));
					  }
				}
		else if ( n->token == TFN)			 fn_one (n->var, &d,a);
		else if ( n->token == UMINUS)		 fn_one ("umin", &d,a);
		else if ( n->token == TFN2)			 fn_two (n->var, &d,&e,a);
		else
			{if		 ( n->token == TPLUS)	 fn_two ("plus", &d,&e,a);
			 else if ( n->token == TMINUS)	 fn_two ("minus",&d,&e,a);
			 else if ( n->token == TSTAR)	 fn_two ("star", &d,&e,a);
			 else if ( n->token == TSLASH)	 fn_two ("slash",&d,&e,a);
			 else
				{a->stype = SERR;
				 ErrorAlert(GetResource('STR ',1014));
				}
			}
		if (d.talloc && d.stype == SARRAY) 	losedat(d.dat);
		if (e.talloc && e.stype == SARRAY)	losedat(e.dat);
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_one		handle computational, single input, single output functions
				s = pointer to character string
				d = pointer to source data
				a = pointer to output data
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_one(s,d,a)		char	*s;
						WORKING	*d,*a;
{
		register float		*p,*q,*t;
		register extended	z,y,*m;
		register int		n;

float	fn_sdev(),	fn_mean(),	fn_max(),	fn_min(),
		fn_stpsd();
		
void	fn_dx1c(),	fn_dxv(),	fn_dy1c(),	fn_dyv(),
		fn_dx2c(),	fn_dy2c(),	fn_lap5c(),	fn_lap9c(),
		fn_coef1(),	fn_coef2(),	fn_medn9(), fn_transpose(),
		fn_shd(),	fn_shl(),	fn_shr(),	fn_shu(),
		unknown(),	ErrorAlert();
		

		z = asin(.5) / 30.;
		y = log(10.);

		if (d->stype == SCONST)
			
			{a->stype = SCONST;
			 if		 (!ncstrcmp(s,"sin"))
			 		 a->cval = (float)(sin((extended)(d->cval)));
			 else if (!ncstrcmp(s,"cos"))
			 		 a->cval = (float)(cos((extended)(d->cval)));
			 else if (!ncstrcmp(s,"tan"))
			 		 a->cval = (float)(tan((extended)(d->cval)));
			 else if (!ncstrcmp(s,"asin"))
			 		 a->cval = (float)(asin((extended)(d->cval)));
			 else if (!ncstrcmp(s,"acos"))
			 		 a->cval = (float)(acos((extended)(d->cval)));
			 else if (!ncstrcmp(s,"atan"))
			 		 a->cval = (float)(atan((extended)(d->cval)));
			 else if (!ncstrcmp(s,"sinh"))
			 		 a->cval = (float)(sinh((extended)(d->cval)));
			 else if (!ncstrcmp(s,"cosh"))
			 		 a->cval = (float)(cosh((extended)(d->cval)));
			 else if (!ncstrcmp(s,"tanh"))
			 		 a->cval = (float)(tanh((extended)(d->cval)));
			 
			 else if (!ncstrcmp(s,"dtor"))
			 		 a->cval = (float)((extended)(d->cval) * z);
			 else if (!ncstrcmp(s,"rtod"))
			 		 a->cval = (float)((extended)(d->cval) / z);
			 
			 else if (!ncstrcmp(s,"dsin"))
			 		 a->cval = (float) (sin((extended)(d->cval) * z));
			 else if (!ncstrcmp(s,"dcos"))
			 		 a->cval = (float) (cos((extended)(d->cval) * z));
			 else if (!ncstrcmp(s,"dtan"))
			 		 a->cval = (float) (tan((extended)(d->cval) * z));
			 else if (!ncstrcmp(s,"dasin"))
			 		 a->cval = (float)(asin((extended)(d->cval)) / z);
			 else if (!ncstrcmp(s,"dacos"))
			 		 a->cval = (float)(acos((extended)(d->cval)) / z);
			 else if (!ncstrcmp(s,"datan"))
			 		 a->cval = (float)(atan((extended)(d->cval)) / z);
			 else if (!ncstrcmp(s,"dsinh"))
			 		 a->cval = (float)(sinh((extended)(d->cval) * z));
			 else if (!ncstrcmp(s,"dcosh"))
			 		 a->cval = (float)(cosh((extended)(d->cval) * z));
			 else if (!ncstrcmp(s,"dtanh"))
			 		 a->cval = (float)(tanh((extended)(d->cval) * z));
			 
			 else if (!ncstrcmp(s,"sqrt" ))
			 		 a->cval = (float)(sqrt((extended)(d->cval)));
			 else if (!ncstrcmp(s,"log"  ))
			 		 a->cval = (float)(log((extended)(d->cval)));
			 else if (!ncstrcmp(s,"log10"))
			 		 a->cval = (float)(log((extended)(d->cval)) / y);
			 else if (!ncstrcmp(s,"exp"  ))
			 		 a->cval = (float)(exp((extended)(d->cval)));
			 else if (!ncstrcmp(s,"colrange"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"rowrange"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"pts"  ))		a->cval = 1.;
			 else if (!ncstrcmp(s,"cols" ))		a->cval = 1.;
			 else if (!ncstrcmp(s,"rows" ))		a->cval = 1.;
			 else if (!ncstrcmp(s,"abs"  ))
			 		 {if (d->cval > 0)		a->cval =	d->cval;
					  else					a->cval = - d->cval;
					 }
			 
			 else if (!ncstrcmp(s,"mean"))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"max" ))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"min" ))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"umin"))	a->cval = - d->cval;
			 else if (!ncstrcmp(s,"shl"))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"shr"))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"shu"))	a->cval = d->cval;
			 else if (!ncstrcmp(s,"shd"))	a->cval = d->cval;
			 
			 else if (!ncstrcmp(s,"dx1c"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"ddx"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"dy1c"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"ddy"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"dx2c"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"d2dx"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"dy2c"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"d2dy"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"lap"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"lap5"))	a->cval = 0.;
			 else if (!ncstrcmp(s,"lap9"))	a->cval = 0.;
			 else							unknown(s,d,NULL,a);
			}
			
		else if (!ncstrcmp(s,"max"))
				{a->stype = SCONST;
				 a->cval = fn_max (d->dat->vals,d->dimx * d->dimy);
				}
		else if (!ncstrcmp(s,"min"))
				{a->stype = SCONST;
				 a->cval = fn_min (d->dat->vals,d->dimx * d->dimy);
				}
		else if (!ncstrcmp(s,"mean"))
				{a->stype = SCONST;
				 a->cval = fn_mean(d->dat->vals,d->dimx * d->dimy);
				}
		else if (!ncstrcmp(s,"sdev"))
				{if	  (d->dimx * d->dimy < 2)
					  {a->stype = SERR;
					   ErrorAlert(GetResource('STR ',1013));
					  }
				 else
				 	  {a->stype = SCONST;
				 	   a->cval = fn_sdev(d->dat->vals,d->dimx * d->dimy);
					  }
				}
		else if (!ncstrcmp(s,"colsdev"))
				{if (d->dimx < 3)
					{a->stype = SERR;
					 ErrorAlert(GetResource('STR ',1013));
					}
				 else
				 	  {a->stype = SCONST;
				 	   a->cval = fn_stpsd(d->dat->xvals,d->dimx);
					  }
				}
		else if (!ncstrcmp(s,"colsmean"))
				{a->stype = SCONST;
				 a->cval = (float)(((extended)(*(d->dat->xvals + d->dimx - 1)) -
				 			(extended)(*(d->dat->xvals))) / (extended)(d->dimx - 1));
				}
		else if (!ncstrcmp(s,"rowsdev"))
				{if (d->dimy < 3)
					{a->stype = SERR;
					 ErrorAlert(GetResource('STR ',1013));
					}
				 else
				 	  {a->stype = SCONST;
				 	   a->cval = fn_stpsd(d->dat->yvals,d->dimy);
					  }
				}
		else if (!ncstrcmp(s,"rowsmean"))
				{a->stype = SCONST;
				 a->cval = (float)(((extended)(*(d->dat->yvals + d->dimy - 1)) -
				 			(extended)(*(d->dat->yvals))) / (extended)(d->dimy - 1));
				}
		else if (!ncstrcmp(s,"cols"))
				{a->stype = SCONST;
				 a->cval = (float)(d->dimx);
				}
		else if (!ncstrcmp(s,"rows"))
				{a->stype = SCONST;
				 a->cval = (float)(d->dimy);
				}
		else if (!ncstrcmp(s,"pts"))
				{a->stype = SCONST;
				 a->cval = (float)(d->dimx * d->dimy);
				}
		else if (!ncstrcmp(s,"colrange"))
				{a->stype = SCONST;
				 a->cval = (float)((extended)(*(d->dat->xvals + d->dimx - 1)) -
				 				   (extended)(*(d->dat->xvals)));
				}
		else if (!ncstrcmp(s,"rowrange"))
				{a->stype = SCONST;
				 a->cval = (float)((extended)(*(d->dat->yvals + d->dimy - 1)) -
				 				   (extended)(*(d->dat->yvals)));
				}
		else
				{if	(0 > useormalloc(d,a))	return;
				 else
				 	{n = d->dimx * d->dimy;
				 	 p = d->dat->vals;
				 	 q = p + n - 1;
				 	 t = a->dat->vals;
					 if			(!ncstrcmp(s,"sin"))
				 		 		{while (p < q)
									   *t++ = (float)(sin((extended)(*p++)));
						  		 *t = (float)(sin((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"cos"))
				 		 		{while (p < q)
									   *t++ = (float)(cos((extended)(*p++)));
						 		 *t = (float)(cos((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"tan"))
				 		 		{while (p < q)
									   *t++ = (float)(tan((extended)(*p++)));
						 		 *t = (float)(tan((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"asin"))
				 		 		{while (p < q)
									   *t++ = (float)(asin((extended)(*p++)));
						  		 *t = (float)(asin((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"acos"))
				 		 		{while (p < q)
									   *t++ = (float)(acos((extended)(*p++)));
						 		 *t = (float)(acos((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"atan"))
				 		 		{while (p < q)
									   *t++ = (float)(atan((extended)(*p++)));
						 		 *t = (float)(atan((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"sinh"))
				 		 		{while (p < q)
									   *t++ = (float)(sinh((extended)(*p++)));
						  		 *t = (float)(sinh((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"cosh"))
				 		 		{while (p < q)
									   *t++ = (float)(cosh((extended)(*p++)));
						 		 *t = (float)(cosh((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"tanh"))
				 		 		{while (p < q)
									   *t++ = (float)(tanh((extended)(*p++)));
						 		 *t = (float)(tanh((extended)(*p)));
						 		}
				 
					 else if	(!ncstrcmp(s,"dsin"))
				 		 		{while (p < q)
									   *t++ = (float)(sin((extended)(*p++) * z));
						  		 *t = (float)(sin((extended)(*p) * z));
						 		}
					 else if	(!ncstrcmp(s,"dcos"))
				 		 		{while (p < q)
									   *t++ = (float)(cos((extended)(*p++) * z));
						 		 *t = (float)(cos((extended)(*p) * z));
						 		}
					 else if	(!ncstrcmp(s,"dtan"))
				 		 		{while (p < q)
									   *t++ = (float)(tan((extended)(*p++) * z));
						 		 *t = (float)(tan((extended)(*p) * z));
						 		}
					 else if	(!ncstrcmp(s,"dasin"))
				 		 		{while (p < q)
									   *t++ = (float)(asin((extended)(*p++)) / z);
						  		 *t = (float)(asin((extended)(*p)) / z);
						 		}
					 else if	(!ncstrcmp(s,"dacos"))
				 		 		{while (p < q)
									   *t++ = (float)(acos((extended)(*p++)) / z);
						 		 *t = (float)(acos((extended)(*p)) / z);
						 		}
					 else if	(!ncstrcmp(s,"datan"))
				 		 		{while (p < q)
									   *t++ = (float)(atan((extended)(*p++)) / z);
						 		 *t = (float)(atan((extended)(*p)) / z);
						 		}
					 else if	(!ncstrcmp(s,"dsinh"))
				 		 		{while (p < q)
									   *t++ = (float)(sinh((extended)(*p++) * z));
						  		 *t = (float)(sinh((extended)(*p) * z));
						 		}
					 else if	(!ncstrcmp(s,"dcosh"))
				 		 		{while (p < q)
									   *t++ = (float)(cosh((extended)(*p++) * z));
						 		 *t = (float)(cosh((extended)(*p) * z));
						 		}
					 else if	(!ncstrcmp(s,"dtanh"))
				 		 		{while (p < q)
									   *t++ = (float)(tanh((extended)(*p++) * z));
						 		 *t = (float)(tanh((extended)(*p) * z));
						 		}
			
					 else if	(!ncstrcmp(s,"dtor"))
				 		 		{while (p < q)
									   *t++ = (float)((extended)(*p++) * z);
						 		 *t = (float)((extended)(*p) * z);
						 		}
					 else if	(!ncstrcmp(s,"rtod"))
				 		 		{while (p < q)
									   *t++ = (float)((extended)(*p++) / z);
						 		 *t = (float)((extended)(*p) / z);
						 		}
								
					 else if	(!ncstrcmp(s,"sqrt"))
				 		 		{while (p < q)
									   *t++ = (float)(sqrt((extended)(*p++)));
						 		 *t = (float)(sqrt((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"log"))
				 		 		{while (p < q)
									   *t++ = (float)(log((extended)(*p++)));
						 		 *t = (float)(log((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"log10"))
				 		 		{while (p < q)
									   *t++ = (float)(log((extended)(*p++)) / y);
						 		 *t = (float)(log((extended)(*p)) / y);
						 		}
					 else if	(!ncstrcmp(s,"exp"))
				 		 		{while (p < q)
									   *t++ = (float)(exp((extended)(*p++)));
						 		 *t = (float)(exp((extended)(*p)));
						 		}
					 else if	(!ncstrcmp(s,"abs"))
				 		 		{while (p < q)
									   {if (*p < 0) *t++ = - *p++;
									    else		*t++ =   *p++;
									   }
								 if (*p < 0) *t = - *p;
								 else		 *t =   *p;
						 		}
					 else if	(!ncstrcmp(s,"umin"))
				 		 		{while (p < q) *t++ = - *p++;
						 		 *t = - *p;
						 		}
			 
					else if (!ncstrcmp(s,"shl"))	 fn_shl(p,q,t,d->dimx);
					else if (!ncstrcmp(s,"shr"))	 fn_shr(p,q,t,d->dimx);
					else if (!ncstrcmp(s,"shu"))	 fn_shu(p,q,t,d->dimx);
					else if (!ncstrcmp(s,"shd"))	 fn_shd(p,q,t,d->dimx);
					
					else if (!ncstrcmp(s,"transpose"))	
							{									/* copy data anew */
							 if	(0 > cloneornot(d,a))	return;
							  fn_transpose(d->dat->vals,
												 a->dat->vals,
												 d->dimx,
												 d->dimy);
							p = a->dat->xvals;					/* swap x and y */
							a->dat->xvals = a->dat->yvals;
							a->dat->yvals = p;
							n = a->dat->xdim;					/* swap dimensions */
							a->dimx = a->dat->xdim = a->dat->ydim;
							a->dimy = a->dat->ydim = n;
							
							}
					
					else if (!ncstrcmp(s,"medn9"))
							{if (d->dimx < 3 || d->dimy < 3)
							    {a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							     {
								 TickCount();
							 	  if	(0 > cloneornot(d,a))	return;
								   fn_medn9(d->dat->vals,
							 					  a->dat->vals,
												  d->dimx,
												  d->dimx * d->dimy);
							     }
							}
					
					else if (!ncstrcmp(s,"dx1c"))
							{if (d->dimx < 3)
							    {a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = .5 * (extended)(d->dimx - 1) /
								  	   ((extended)(*(d->dat->xvals + d->dimx - 1)) -
									   (extended)(*(d->dat->xvals)));
								   if (*(d->dat->xvals + d->dimx - 1) <
								   	   *(d->dat->xvals))	z = - z;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_dx1c (d->dat->vals,
								   				   a->dat->vals,
												   d->dimx,
												   d->dimx * d->dimy,
												   z);
								  }
							}
					else if (!ncstrcmp(s,"dx2c"))
							{if (d->dimx < 3)
							    {a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = (extended)(d->dimx - 1) /
								  	   ((extended)(*(d->dat->xvals + d->dimx - 1)) -
									   (extended)(*(d->dat->xvals)));
								   z *= z;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_dx2c (d->dat->vals,
								   				   a->dat->vals,
												   d->dimx,
												   d->dimx * d->dimy,
												   z);
								  }
							}
					else if (!ncstrcmp(s,"dy1c"))
							{if (d->dimy < 3)
							    {a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = .5 * (extended)(d->dimy - 1) /
								  	   ((extended)(*(d->dat->yvals + d->dimy - 1)) -
									   (extended)(*(d->dat->yvals)));
								   if (*(d->dat->yvals + d->dimy - 1) <
								   	   *(d->dat->yvals))	z = - z;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_dy1c (d->dat->vals,
								   				   a->dat->vals,
												   d->dimx,
												   d->dimx * d->dimy,
												   z);
								  }
							}
					else if (!ncstrcmp(s,"dy2c"))
							{if (d->dimy < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = (extended)(d->dimy - 1) /
								  	   ((extended)(*(d->dat->yvals + d->dimy - 1)) -
									   (extended)(*(d->dat->yvals)));
								   z *= z;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_dy2c (d->dat->vals,
								   				   a->dat->vals,
												   d->dimx,
												   d->dimx * d->dimy,
												   z);
								  }
							}
					else if (!ncstrcmp(s,"ddx"))
							{if (d->dimx < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								 return;
								}
							 if (checkmem(2 * d->dimx * sizeof(extended)))
								return;
							 if (NULL == (m = (extended *)
							 			  NewPtr(2 * d->dimx * sizeof(extended))))
		   						{nomem();
		   						 return;
		   						}
							 if	(0 > cloneornot(d,a))	return;
							 	fn_coef1(d->dat->xvals,m,d->dimx);
							  fn_dxv(d->dat->vals,
							 			   a->dat->vals,
										   d->dimx,
										   d->dimy,
										   m);
							 DisposPtr	( (Ptr) m);
							}

					else if (!ncstrcmp(s,"d2dx"))
							{if (d->dimx < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								 return;
								}
							 if (checkmem(2 * d->dimx * sizeof(extended)))
								return;
							 if (NULL == (m = (extended *)
							 			  NewPtr(2 * d->dimx * sizeof(extended))))
		   						{nomem();
		   						 return;
		   						}
							 if	(0 > cloneornot(d,a))	return;
							 	fn_coef2(d->dat->xvals,m,d->dimx);
							  fn_dxv(d->dat->vals,
							 			   a->dat->vals,
										   d->dimx,
										   d->dimy,
										   m);
							 DisposPtr	((Ptr) m);
							}

					else if (!ncstrcmp(s,"ddy"))
							{if (d->dimy < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								 return;
								}
							 if (checkmem(2 * d->dimy * sizeof(extended)))
								return;
							 if (NULL == (m = (extended *)
							 			  NewPtr(2 * d->dimy * sizeof(extended))))
		   						{nomem();
		   						 return;
		   						}
							 if	(0 > cloneornot(d,a))	return;
							 	fn_coef1(d->dat->yvals,m,d->dimy);
							  fn_dyv(d->dat->vals,
							 			   a->dat->vals,
										   d->dimx,
										   d->dimy,
										   m);
							 DisposPtr	((Ptr) m);
							}

					else if (!ncstrcmp(s,"d2dy"))
							{if (d->dimy < 3)
								{a->stype = SERR;
					 			 ErrorAlert(GetResource('STR ',1013));
								 return;
								}
							 if (checkmem(2 * d->dimy * sizeof(extended)))
								return;
							 if (NULL == (m = (extended *)
							 			  NewPtr(2 * d->dimy * sizeof(extended))))
		   						{nomem();
		   						 return;
		   						}
							 if	(0 > cloneornot(d,a))	return;
							 	fn_coef2(d->dat->yvals,m,d->dimy);
							  fn_dyv(d->dat->vals,
							 			   a->dat->vals,
										   d->dimx,
										   d->dimy,
										   m);
							 DisposPtr	((Ptr) m);
							}

					else if (!ncstrcmp(s,"lap5") || !ncstrcmp(s,"lap"))
							{if (d->dimy < 3 || d->dimx < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = (extended)(d->dimx - 1) /
								  	   ((extended)(*(d->dat->xvals + d->dimx - 1)) -
									   (extended)(*(d->dat->xvals)));
								   z *= z;
							 	   y = (extended)(d->dimy - 1) /
								  	   ((extended)(*(d->dat->yvals + d->dimy - 1)) -
									   (extended)(*(d->dat->yvals)));
								   y *= y;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_lap5c	(d->dat->vals,
								   					 a->dat->vals,
													 d->dimx,
													 d->dimx * d->dimy,
													 z,
													 y);
								  }
							}
					else if (!ncstrcmp(s,"lap9"))
							{if (d->dimy < 3 || d->dimx < 3)
								{a->stype = SERR;
								 ErrorAlert(GetResource('STR ',1013));
								}
							 else
							 	  {z = (extended)(d->dimx - 1) /
								  	   ((extended)(*(d->dat->xvals + d->dimx - 1)) -
									   (extended)(*(d->dat->xvals)));
								   z *= z;
							 	   y = (extended)(d->dimy - 1) /
								  	   ((extended)(*(d->dat->yvals + d->dimy - 1)) -
									   (extended)(*(d->dat->yvals)));
								   y *= y;
								   
								   z = .5 * (z + y);
								   z *= .25;
							 	   if	(0 > cloneornot(d,a))	return;
								    fn_lap9c	(d->dat->vals,
								   					 a->dat->vals,
													 d->dimx,
													 d->dimx * d->dimy,
													 z);
								  }
							}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	We don't have a function by this name.  Look for an external function.
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
					else	unknown(s,d,NULL,a);
				   }
		   }
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_two		handle computational, double input, single output functions
				s = pointer to character string code
				d = pointer to left  source data
				e = pointer to right source data
				a = pointer to output data
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_two(s,d,e,a)	char		*s;
						WORKING		*d,*e,*a;
{
		register float		*p,*q,*t,*u;
		register extended	z,y;
		register int		n;
		extended			theK[7 * 7];
		void				DoKernelOperation(),
							ErrorAlert(),
							unknown();
		
		z = asin(.5) / 30.;

	/*
	*******************************
	kernel operations
	******************************* 
	*/
	if (!ncstrcmp(s,"kernel"))
	   {if (d->stype == SCONST || e->stype == SCONST)
		   {a->stype = SERR;
			ErrorAlert(GetResource('STR ',1012));
		   }
	    else if (e->dimx == 3 && e->dimy == 4)
				{if (d->dimx < 3 ||
					 d->dimy < 3 ||
					 *(e->dat->vals) == 0.)
					{a->stype = SERR;
					 ErrorAlert(GetResource('STR ',1011));
					}
				 else
				 	 {if (useormalloc(d,a) < 0)	return;
					  	     DoKernelOperation(d->dat->vals,
					  				  		   a->dat->vals,
									  		   d->dimx,
									 		   d->dimx * d->dimy,
									  		   e->dat->vals,
											   theK,
											   3);
					 }
				}
	    else if (e->dimx == 5 && e->dimy == 6)
				{if (d->dimx < 5 ||
					 d->dimy < 5 ||
					 *(e->dat->vals) == 0.)
					{a->stype = SERR;
					 ErrorAlert(GetResource('STR ',1011));
					}
				 else
				 	 {if (useormalloc(d,a) < 0)	return;
					  		 DoKernelOperation(d->dat->vals,
					  				  		   a->dat->vals,
									  		   d->dimx,
									  		   d->dimx * d->dimy,
									  		   e->dat->vals,
											   theK,
											   5);
					 }
				}
	    else if (e->dimx == 7 && e->dimy == 8)
				{if (d->dimx < 7 ||
					 d->dimy < 7 ||
					 *(e->dat->vals) == 0.)
					{a->stype = SERR;
					 ErrorAlert(GetResource('STR ',1011));
					}
				 else
				 	 {if (useormalloc(d,a) < 0)	return;
					  		 DoKernelOperation(d->dat->vals,
					  				  		   a->dat->vals,
									  		   d->dimx,
									  		   d->dimx * d->dimy,
									 		   e->dat->vals,
											   theK,
											   7);
					 }
				}
	    else
			{a->stype = SERR;
			 ErrorAlert(GetResource('STR ',1010));
			}
	   }

	/*
	*******************************
	constant input, constant output
	******************************* 
	*/
	else if ((d->stype == SCONST) && (e->stype == SCONST))
	   {a->stype = SCONST;
		if		(!ncstrcmp(s,"pow"))
				a->cval = (float)(pow((extended)(d->cval),(extended)(e->cval)));
		else if (!ncstrcmp(s,"atan2"))
				a->cval = (float)(atan2((extended)(d->cval),(extended)(e->cval)));
		else if (!ncstrcmp(s,"datan2"))
				a->cval = (float)
						  (atan2((extended)(d->cval),(extended)(e->cval)) / z);
		else if (!ncstrcmp(s,"plus"))
				a->cval = (float)((extended)(d->cval) + (extended)(e->cval));
		else if (!ncstrcmp(s,"minus"))
				a->cval = (float)((extended)(d->cval) - (extended)(e->cval));
		else if (!ncstrcmp(s,"star"))
				a->cval = (float)((extended)(d->cval) * (extended)(e->cval));
		else if (!ncstrcmp(s,"slash"))
				a->cval = (float)((extended)(d->cval) / (extended)(e->cval));
		else	unknown(s,d,e,a);		
	   }
	/*
	*****************************************************
	d is matrix input, e is constant input, matrix output
	*****************************************************
	*/
	else if  (e->stype == SCONST)
	   {if (0 > useormalloc(d,a)) return;
	    n = d->dimx * d->dimy;
		p = d->dat->vals;
		t = a->dat->vals;
		q = p + n - 1;
		y = (extended)(e->cval);
			
		if (!ncstrcmp(s,"pow"))
				{if (y == 1.)
					{while (p < q)	*t++ = *p++;
					 *t = *p;
					}
				 else if (y == 2.)
					{while (p < q)
						   {*t++ = (float)((extended)(*p) * (extended)(*p));
						    p++;
						   }
					 *t = (float)((extended)(*p) * (extended)(*p));
					}
				 else if (y == 3.)
					{while (p < q)
						   {y = (extended)(*p++);
						    *t++ = (float)(y * y * y);
						   }
					 *t = (float)((extended)(*p) * (extended)(*p) * (extended)(*p));
					}
				 else if (y == 4.)
					{while (p < q)
						   {y = (extended)(*p++);
						    y *= y;
							y *= y;
						    *t++ = (float)(y);
						   }
					 y = (extended)(*p);
					 *t = (float)(y * y * y * y);
					}
				 else
					{while (p < q)	*t++ = (float)(pow((extended)(*p++),y));
					 *t = (float)(pow((extended)(*p),y));
					}
				}
		else if (!ncstrcmp(s,"atan2"))
				{while (p < q)	*t++ = (float)(atan2((extended)(*p++),y));
				 *t = (float)(atan2((extended)(*p),y));
				}
		else if (!ncstrcmp(s,"datan2"))
			    {while (p < q)	*t++ = (float)(atan2((extended)(*p++),y) / z);
				 *t = (float)(atan2((extended)(*p),y) / z);
			    }
		else if (!ncstrcmp(s,"plus"))
			    {while (p < q)	*t++ = (float)((extended)(*p++) + y);
				 *t = (float)((extended)(*p) + y);
			    }
		else if (!ncstrcmp(s,"minus"))
			    {while (p < q)	*t++ = (float)((extended)(*p++) - y);
				 *t = (float)((extended)(*p) - y);
			    }
		else if (!ncstrcmp(s,"star"))
			    {while (p < q)	*t++ = (float)((extended)(*p++) * y);
				 *t = (float)((extended)(*p) * y);
			    }
		else if (!ncstrcmp(s,"slash"))
			    {while (p < q)	*t++ = (float)((extended)(*p++) / y);
				 *t = (float)((extended)(*p) / y);
			    }
		else	unknown(s,d,e,a);
	   }
	/*
	*****************************************************
	d is constant input, e is matrix input, matrix output
	*****************************************************
	*/
	else if  (d->stype == SCONST)
	   {if (0 > useormalloc(e,a)) return;
	    n = e->dimx * e->dimy;
		p = e->dat->vals;
		t = a->dat->vals;
		q = p + n - 1;
		y = (extended)(d->cval);
			
		if (!ncstrcmp(s,"pow"))
		  		{while (p < q)	*t++ = (float)(pow(y,(extended)(*p++)));
				 *t = (float)(pow(y,(extended)(*p)));
				}
		else if (!ncstrcmp(s,"atan2"))
				{while (p < q)	*t++ = (float)(atan2(y,(extended)(*p++)));
				 *t = (float)(atan2(y,(extended)(*p)));
				}
		else if (!ncstrcmp(s,"datan2"))
			    {while (p < q)	*t++ = (float)(atan2(y,(extended)(*p++)) / z);
				 *t = (float)(atan2(y,(extended)(*p)) / z);
			    }
		else if (!ncstrcmp(s,"plus"))
			    {while (p < q)	*t++ = (float)(y + (extended)(*p++));
				 *t = (float)(y + (extended)(*p));
			    }
		else if (!ncstrcmp(s,"minus"))
			    {while (p < q)	*t++ = (float)(y - (extended)(*p++));
				 *t = (float)(y - (extended)(*p));
			    }
		else if (!ncstrcmp(s,"star"))
			    {while (p < q)	*t++ = (float)(y * (extended)(*p++));
				 *t = (float)(y * (extended)(*p));
			    }
		else if (!ncstrcmp(s,"slash"))
			    {while (p < q)	*t++ = (float)(y / (extended)(*p++));
				 *t = (float)(y / (extended)(*p));
			    }
		else	unknown(s,d,e,a);
	   }
	/*
	***************************************************
	d is matrix input, e is matrix input, matrix output
	***************************************************
	*/
	else
	   {if (d->dimx != e->dimx || d->dimy != e->dimy)
		   {a->stype = SERR;
		    ErrorAlert(GetResource('STR ',1009));
		    return;
		   }
	    if (0 > useormalloc(d,a)) return;
	    n = e->dimx * e->dimy;
		p = e->dat->vals;
		t = a->dat->vals;
		q = p + n - 1;
		u = d->dat->vals;
			
		if (!ncstrcmp(s,"pow"))
		  		{while (p < q)
					 *t++ = (float)(pow((extended)(*u++),(extended)(*p++)));
				 *t = (float)(pow((extended)(*u),(extended)(*p)));
				}
		else if (!ncstrcmp(s,"atan2"))
				{while (p < q)
					   *t++ = (float)(atan2((extended)(*u++),(extended)(*p++)));
				 *t = (float)(atan2((extended)(*u),(extended)(*p)));
				}
		else if (!ncstrcmp(s,"datan2"))
			    {while (p < q)
					   *t++ = (float)(atan2((extended)(*u++),(extended)(*p++)) / z);
				 *t = (float)(atan2((extended)(*u),(extended)(*p)) / z);
			    }
		else if (!ncstrcmp(s,"plus"))
			    {while (p < q)
					   *t++ = (float)((extended)(*u++) + (extended)(*p++));
				 *t = (float)((extended)(*u) + (extended)(*p));
			    }
		else if (!ncstrcmp(s,"minus"))
			    {while (p < q)
					   *t++ = (float)((extended)(*u++) - (extended)(*p++));
				 *t = (float)((extended)(*u) - (extended)(*p));
			    }
		else if (!ncstrcmp(s,"star"))
			    {while (p < q)
					   *t++ = (float)((extended)(*u++) * (extended)(*p++));
				 *t = (float)((extended)(*u) * (extended)(*p));
			    }
		else if (!ncstrcmp(s,"slash"))
			    {while (p < q)
					   *t++ = (float)((extended)(*u++) / (extended)(*p++));
				 *t = (float)((extended)(*u) / (extended)(*p));
			    }
		else	unknown(s,d,e,a);
	   }
	return;
}
