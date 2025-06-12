/* tags.h */

#define forever for(;;)

/* a tag is a very simple data structure */

struct tag
   {
   unsigned long type;
   VALUE data;
   };


#define TAG_END 0
#define TAG_IGNORE 1    /* ignore this tag */
#define TAG_SKIP 3      /* skip <n> tags in the list */
#define TAG_SUB 2       /* sub to <taglist> */
#define TAG_JUMP 4      /* jump to <taglist> */

unsigned long tag_length P((struct tag *t));
struct tag *tags_copy P((struct tag *t));
struct tag *alloc_tags P((unsigned long l));
struct tag *get_tag P((struct tag *t));

#ifdef KLUDGE_TAG

/* in case the stack is right, we can do this */
#define TAG(function)   \
struct tag *function##taglist(struct tag *list); \
struct tag *function(unsigned long arg1, ...) \
   { \
   return function##taglist(&arg1); \
   }\
struct tag *function##taglist
#else

/* in other cases, we have to copy the tags to a taglist... */
#include <stdarg.h>
#define TAG(function)   \
struct tag *function##taglist(struct tag *list); \
struct tag *function(unsigned long type, ...) \
   {  \
   va_list ap; \
   struct tag big[50]; \
   int i = 0; \
   va_start(ap, type); \
   while (type != TAG_END) \
      { \
      big[i].type = type; \
      big[i].data = va_arg(ap, VALUE); \
      i++; \
      type = va_arg(ap, unsigned long); \
      } \
   return function##taglist(big); \
   } \
struct tag *function##taglist
#endif
