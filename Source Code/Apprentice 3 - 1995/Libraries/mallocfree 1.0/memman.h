void *fmalloc(size_t nbytes);
void ffree(void *ap);
void *frealloc(void *p, size_t s);
size_t fgetallocsize(const void *p);
void *fcalloc(size_t, size_t);
void *ffcalloc(size_t s);
