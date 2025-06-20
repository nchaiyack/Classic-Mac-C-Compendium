/*
 * SoftKiss - circular text buffer
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1992
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#include "sfk_core.h"
#include "sfk_core_private.h"

#define CU_WRAP(xx_cu,xx_ptr) \
 do { \
 	if(xx_ptr>=(xx_cu)->cu_size) \
 		xx_ptr-=(xx_cu)->cu_size; \
 } while(0)

/*
 * uninitialize a circular buffer, release any allocated space
 */
void sfk_cu_uninit(sfk_circular_buffer_pt cu)
{
	cu->cu_size=0;
	if(cu->cu_buf!=0)
		sfk_free(cu->cu_buf);
	cu->cu_buf=0;
	cu->cu_read=0;
	cu->cu_write=0;
}

/*
 * initialize a circular buffer
 * pass zero for the size to dispose of any allocated memory
 */
void sfk_cu_init(sfk_circular_buffer_pt cu,long max_size)
{
	if(max_size==cu->cu_size)
		return;
	sfk_cu_uninit(cu);
	cu->cu_read=0;
	cu->cu_write=0;
	cu->cu_size=0;
	if(max_size>0)
		cu->cu_buf=sfk_malloc(max_size);
	cu->cu_size=max_size;
}


/*
 * return max number of bytes that can be written
 */
long sfk_cu_write_size(sfk_circular_buffer_pt cu)
{
	register long chunk=cu->cu_read-cu->cu_write;
	if(chunk>0)
		return sfk_imax(0,chunk-1);
	return sfk_imax(0,(cu->cu_size+chunk)-1);
}

/*
 * write data
 */
long sfk_cu_write(sfk_circular_buffer_pt cu,char *put_me,long put_size)
{
	long cu_write=cu->cu_write;
	long put_might_fit=sfk_cu_write_size(cu);
	long put_fits=sfk_imax(0,sfk_imin(put_might_fit,put_size));
	long did_write=put_fits;
	long chunk_size=sfk_imin(did_write,cu->cu_size-cu_write);
	if(chunk_size>0) {
		memcpy(cu->cu_buf+cu_write,put_me,chunk_size);
		cu_write+=chunk_size;
		put_fits-=chunk_size;
		put_me+=chunk_size;
	}
	CU_WRAP(cu,cu_write);
	if(put_fits>0) {
		memcpy(cu->cu_buf+cu_write,put_me,put_fits);
		cu_write+=put_fits;
	}
	CU_WRAP(cu,cu_write);
	cu->cu_write=cu_write;
	return did_write;
}

/*
 * return max number of bytes that can be read
 */
long sfk_cu_read_size(sfk_circular_buffer_pt cu)
{
	long cu_write=cu->cu_write;
	if(cu_write<cu->cu_read)
		cu_write+=cu->cu_size;
	return cu_write-cu->cu_read;
}

/*
 * read data
 */
long sfk_cu_read(sfk_circular_buffer_pt cu,char *read_me,long read_size)
{
	long read_avail_in_driver=sfk_cu_read_size(cu);
	long read_avail=sfk_imin(read_avail_in_driver,read_size);
	long cu_read=cu->cu_read;
	long chunk_size=sfk_imin(read_avail,(cu->cu_size-cu_read));
	long did_read=read_avail;
	if(chunk_size>0) {
		memcpy(read_me,cu->cu_buf+cu_read,chunk_size);
		cu_read+=chunk_size;
		read_avail-=chunk_size;
		read_me+=chunk_size;
	}
	CU_WRAP(cu,cu_read);
	if(read_avail>0) {
		memcpy(read_me,cu->cu_buf+cu_read,read_avail);
		cu_read+=read_avail;
	}
	CU_WRAP(cu,cu_read);
	cu->cu_read=cu_read;
	return did_read;
}

/*
 * memcpy that stops after copying a control character and strips parity
 * returns a count of the characters copied
 */
static long memcpy_break(char *dst,char *src,long count)
{
	register unsigned char ch;
	register unsigned char *usrc=((unsigned char*)src);
	while(--count>=0) {
		ch= *usrc++;
		ch=sfk_MASK_PARITY(ch);	/*mask parity*/
		*dst++ = ch;
		if((ch==0x7f)||((ch<' ')&&(ch!='\t')))
			break;
	}
	return usrc-((unsigned char*)src);
}

/*
 * read data stopping on break (control) characters
 */
long sfk_cu_read_line(sfk_circular_buffer_pt cu,char *read_me,long read_size)
{
	long read_avail_in_driver=sfk_cu_read_size(cu);
	long read_avail=sfk_imin(read_avail_in_driver,read_size);
	long cu_read=cu->cu_read;
	long chunk_size;
	long did_read=0;
	long did_chunk;
	while((chunk_size=sfk_imin(read_avail,(cu->cu_size-cu_read)))>0) {
		CU_WRAP(cu,cu_read);
		did_chunk=memcpy_break(read_me,cu->cu_buf+cu_read,chunk_size);
		cu_read+=did_chunk;
		read_avail-=did_chunk;
		read_me+=did_chunk;
		did_read+=did_chunk;
		CU_WRAP(cu,cu_read);
		if(did_chunk!=chunk_size)	/*if we broke due to control char,done*/
			break;
	}
	cu->cu_read=cu_read;
	return did_read;
}
