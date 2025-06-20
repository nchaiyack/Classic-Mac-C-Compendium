#define RIGHT_TREE_PATH(depth) \
(hit_global.data[0].tree.path[(depth) SHR 8] BIT_AND \
 (1 SHL ((depth) BIT_AND 7)))

#define CSG_NODES_MAX (224)

typedef
struct
{
  unsigned short int depth;
  unsigned char   path[(CSG_NODES_MAX + 7) / 8];
} tree_struct;
typedef
tree_struct    *tree_ptr;

#define PATH_LEFT  (0)
#define PATH_RIGHT (1)

#define SAVE_TREE_PATH(side)\
do {\
  if ((side) == PATH_LEFT)\
    tree.path[tree.depth SHR 8] &= BIT_NOT(1 SHL (tree.depth BIT_AND 7));\
  if ((side) == PATH_RIGHT)\
    tree.path[tree.depth SHR 8] |= 1 SHL (tree.depth BIT_AND 7);\
} while (0)

typedef
struct
{
  object_ptr      object;
  real            distance;
  xyz_struct      vector;
  unsigned short int enter;
  tree_struct     tree;
} hit_node_struct;

#define ENTERING (2)

typedef
struct
{
  short int       nodes;
  hit_node_struct data[CSG_NODES_MAX];
} hit_struct;
typedef
hit_struct     *hit_ptr;

#define CSG_SET_ENTER(hit, flag) (hit)->data[0].enter = (flag) + 1

/* Prototypes */

real
#ifdef __STDC__
object_intersect(object_ptr, xyz_ptr, xyz_ptr, hit_ptr, real);
#else
object_intersect();
#endif

real csg_sec_intersect(object_ptr, xyz_ptr, xyz_ptr, hit_ptr, real);

boolean csg_intersection(xyz_ptr, xyz_ptr, hit_ptr, hit_ptr, real, real,
                 hit_ptr *, real*);
                 
boolean csg_subtraction(xyz_ptr, xyz_ptr, hit_ptr, hit_ptr, real, real,
                hit_ptr *, real *);

boolean csg_union(xyz_ptr, xyz_ptr, hit_ptr, hit_ptr, real, real,
                hit_ptr *, real *);

void csg_copy_hit(hit_ptr, hit_ptr);
