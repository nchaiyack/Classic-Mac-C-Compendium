extern Ptr		sillymalloc( Size howmuch );
extern void		sillyfree( void );
extern void		silly_get_MF_mem_status( void );

typedef enum {
	sm_unknown,
	sm_no_temp_mem,
	sm_temp_mem,
	sm_temp_mem_is_real
} sm_temp_mem_status;

extern sm_temp_mem_status	MF_mem_status;