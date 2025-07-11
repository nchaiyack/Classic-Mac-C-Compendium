/*
 * sfk_dyntext.c - dynamicly loaded text
 */

/*
 * routine to free memory allocated by dyntext
 */
typedef void (*free_rtn)(void *);

/*
 * routine to allocate memory
 */
typedef void *(*alloc_rtn)(unsigned long alloc_size);

struct dyn_text_R {
	char dispose_thand;			/*thand has been detached so free it*/
	unsigned short nstrs;		/*number of strings*/
	Handle thand;				/*handle to text*/
	unsigned char *tptr;		/*pointer to text*/
	unsigned short *tstrs;		/*pointers to strings*/
	free_rtn fr;				/*routine to free tstrs when done*/
};
typedef struct dyn_text_R dyn_text,*dyn_text_pt;

/*
 * read in a dynamic text block
 * return TRUE if successfull
 */
int sfk_dt_load(short res_id,dyn_text_pt dt,alloc_rtn ar,free_rtn fr);

/*
 * clean up a dt text block
 */
void sfk_dt_free(dyn_text_pt dt);
