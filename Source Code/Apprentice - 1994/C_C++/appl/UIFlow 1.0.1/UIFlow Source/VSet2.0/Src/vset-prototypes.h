/* These are the current functions as of Vset 2.1 
*/


/* These prototypes have been moved to df-prototypes.h */


DF* DFopen(char *name, int access, int ndds);
int DFsetfind(DF *dfile, uint16 tag, uint16 ref);
int DFclose( DF *dfile);
int DFIseedDDs(DF *dfile);
int DFIcheck( DF *dfile);
int DFdescriptors(DF *dfile, DFdesc ptr[], int begin, int num);
int DFnumber(DF *dfile, uint16 tag);
int DFsetfind( DF *dfile, uint16 tag, uint16 ref);
int DFfind(DF *dfile, DFdesc *ptr);
int DFIfind( DF *dfile,uint16 tag,uint16 ref, int isfirst,
	uint16 ltag,uint16 lref, DFdle **cDLEp, int *cddp);
DFdd *DFIemptyDD(DF *dfile);
int DFaccess(DF *dfile,uint16 tag,uint16 ref, char *access);
int32 DFread( DF *dfile, char *ptr, int32 len);
int32 DFIread( DF *dfile, char *ptr, int32 len);
int32 DFseek( DF *dfile, int32 offset);
int32 DFwrite( DF *dfile, char *ptr, int32 len);
int DFupdate( DF *dfile);
int DFstat(DF *dfile, struct DFdata *dfinfo);
int32 DFgetelement( DF *dfile, uint16 tag, uint16 ref, char *ptr);
int DFputelement( DF *dfile,uint16 tag,uint16 ref, char *ptr,int32 len);
int DFdup(DF *dfile , uint16 itag,uint16 iref,uint16 otag,uint16 oref);
int DFdel(DF *dfile, uint16 tag,uint16 ref);
uint16 DFnewref(DF *dfile);
DFishdf(char *filename);
int DFerrno(void);
int DFIerr(DF *dfile);




/* From VCONV.c */
/*int nolongfn(void);*/
int DIRECTtoIEEE (unsigned char * machfp,unsigned char * ieeefp,
	int mstep, int istep,int  n);
int DIRECTfromIEEE (unsigned char *ieeefp,unsigned char * machfp, 
	int istep, int mstep,int n)	;	
int bytetoIEEE (unsigned char *machfp,unsigned char * ieeefp, 
	int mstep,int  istep,int  n);
int bytefromIEEE (unsigned char *ieeefp,unsigned char * machfp, 
	int istep,int  mstep,int  n);
int inttoIEEE (unsigned char *src,unsigned char *dst,
	int sdelta,int ddelta,int n);			

int intfromIEEE (unsigned char *src,unsigned char *dst,
	int sdelta,int ddelta,int n);	
int longtoIEEE (unsigned char *src,unsigned char *dst,
	int sdelta,int ddelta,int n);
int longfromIEEE (unsigned char *src,unsigned char *dst,
	int sdelta,int ddelta,int n);
/* Skipping the cray...*/	
int vaxFtoIEEE (unsigned char *vaxfp, unsigned char *ieeefp, 
	int vstep, int istep, int n);	
int vaxFfromIEEE (unsigned char *ieeefp, unsigned char *vaxfp, 
	int istep,int vstep,int n);	
int floattoIEEE (unsigned char *machfp, unsigned char *ieeefp,
	int mstep,int  istep,int  n);	
int floatfromIEEE (unsigned char *ieeefp,unsigned char * machfp, 
	int istep, int mstep,int n);	

int movebytes (unsigned char * ss, unsigned char * dd, int n);

int vicheckcompat(DF * f);
int vimakecompat(DF *f);
int vcheckcompat(char * fs);
int vmakecompat(char * fs);
oldunpackvg (VGROUP* vg,unsigned char 	buf[],int *size); 
oldunpackvs (VDATA *vs,unsigned char buf[],int *size);

/* From VG.c */
int vnewref (DF  *f);		
int VSelts (VDATA *vs);
int VSgetinterlace (VDATA *vs);
int VSsetinterlace (VDATA *vs, int interlace);
int VSgetfields (VDATA *vs,char *fields);
int VSfexist (VDATA *vs, char * fields) ; 
int VSsizeof (VDATA *vs, char *fields);  
int matchnocase (char * strx,char * stry) ;   
void VSdump (VDATA *vs) ; 
void VSsetname (VDATA *vs, char	*vsname); 
void VSsetclass (VDATA *vs, char *vsclass) ; 
void VSgetname (VDATA *vs,char * vsname) ;  
void VSgetclass (VDATA *vs,char * vsclass);
int VSinquire (VDATA 	*vs, int *nelt, int *interlace, 
	char *fields,int * eltsize,char * vsname);
int VSlone(DF * f,int idarray[],int asize); 
int Vlone(DF * f, int idarray[], int asize); 
int Vfind (DF * f,char * vgname);
int VSfind (DF * f,char *  vsname);


/* From VGP.c */
void setjj(void);
void setnojj(void);
int Load_vfile (DF *f); 
void Remove_vfile (DF *f); 
vfile_t * Get_vfile(DF *f);
vginstance_t * vginstance (DF  *f,int vgid);
int vshowvgdir(DF * f,char* text); 
vginstance_t * vginstance (DF  *f,int vgid); 
int vexistvg (DF  *f,int vgid); 
void vpackvg (VGROUP *vg,unsigned char 	buf[],int *size);    
void vunpackvg (VGROUP *vg,unsigned char 	buf[],int *size);    
VGROUP *Vattach (DF *f, int vgid, char *accesstype); 
void Vdetach (VGROUP *vg);
int Vinsert (VGROUP	*vg,VDATA 	*velt); /* 2nd arg can also be (VGROUP *) */
int Vflocate (VGROUP *vg, char * field);
int Vinqtagref(VGROUP *vg, int tag, int ref);
int Vntagrefs(VGROUP * vg);
int Vgettagrefs(VGROUP * vg, int tagarray[], int refarray[], int n);
int Vgettagref(VGROUP * vg, int which, int * tag, int * ref);

int Vaddtagref(VGROUP * vg, int tag, int ref);
int vinsertpair (VGROUP	* vg, int tag,int ref);
int Ventries (DF *f,int vgid);  
void Vsetname (VGROUP	*vg,char * vgname);    
void Vsetclass (VGROUP *vg,char * vgclass);
int Visvg (VGROUP	*vg,int id); 
int Vgetid (DF* f,int vgid) ;  
int Vgetnext (VGROUP *vg,int id);
void Vgetname (VGROUP *vg, char *vgname);   
void Vgetclass (VGROUP *vg,char * vgclass);   
int Vinquire (VGROUP *vg,int * nentries,char * vgname); 
void Vinitialize(DF *f);

/* From vparse.c */
int scanattrs (char	*attrs,int	*attrc,char	***attrv);

/* From VHI.c */
int VHstoredata (DF * f, char * field, unsigned char buf[], int32 n, 
	int datatype,char * vsname, char *vsclass);
int VHstoredatam (DF  *f, char 	*field,unsigned char buf[], int32 n,int datatype,
	char * vsname, char * vsclass,int order);
int VHmakegroup (DF * f, int tagarray[],int refarray[] ,
	int n,char * vgname,char * vgclass);


/* From VIO.c */
int vshowvsdir(DF * f,char * text);
void vshowvg (VGROUP *vg); 
void vshowvs (VDATA *vs) ; 
vsinstance_t * vsinstance (DF  *f,int vsid);
int DFvsetclose(DF * f);
DF* DFvsetopen (char * fname,int  access, int defDDs); 
int vexistvs (DF  *f,int vsid);
void vpackvs (VDATA *vs,unsigned char buf[],int   *size); 
void vunpackvs (VDATA *vs,unsigned char buf[],int   *size);       
VDATA * VSattach (DF *f,int  vsid,char *accesstype);  
void VSdetach (VDATA  *vs);  
int Visvs (VGROUP  *vg,int id); 
int VSgetid(DF  *f,int vsid); 
zerofill(unsigned char* x,int n) ; 


 

/* From VSETF.c*/
int SIZEOF(int x);
int VSsetfields (VDATA *vs,char *fields); 
int VSfdefine (VDATA *vs, char *field,int localtype,int order);  


/* From VRW.c */
int VSseek (VDATA *vs,int32 eltpos) ; 
int VSread (VDATA    *vs,unsigned char	* buf,int32 nelt,int interlace);    
int VSwrite (VDATA *vs,unsigned char * buf,int32 nelt,int interlace);
