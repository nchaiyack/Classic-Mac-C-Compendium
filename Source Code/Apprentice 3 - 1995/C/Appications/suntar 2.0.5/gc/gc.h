/* E_parser input values */
#define Normal 8
#define commaIsNotOp 020
#define externalCall 0x8000

/* associativity codes */
#define prefix 2
#define postfix 1
#define leftass 5
#define rightass 7
#define noass 6
#define ternary 4

#define ass 0
#define dyn 1
#define const 2
#define string 3

#define F  0
#define T  1
#define EQ 2
#define NE 3
#define LE 4
#define GT 5
#define LT 6
#define GE 7

/* lazy code generator classes */
#define immediate   0
#define simplevar   2
#define relocatable 4
#define based       5
#define generated   6
#define flags       7

/* features: */
#define lval1op 2
#define lval2op 1
#define unaryToo 4
#define asgnOpToo 8
#define assignOp 16

#define TOLONG(x) (char*)(&symbolTable[-1])+x
#define TOWORD(x) (char *)x-(char*)(&symbolTable[-1])


typedef unsigned char byte;
typedef unsigned short word;
typedef struct tabel{
      short typ;
      char name[10];
      long val;
}tabel;

/* code generator struct: it's horribly tricky with all those overlays,
but you haven't yet seen what happens when the "right" and "this" structs
are really overlayed, occupying the same memory, and the fields of "this" 
are computed from the fields of "right", doing the most tricky things to
avoid reading a field after it was overwritten (and to avoid assigning
values since they are already there): a very pretty set of tricks !
*/


typedef struct localmem{
   byte op,opfea;
   word counter;
   struct GCI{
      byte Lval;
      byte Class;
      word Displacement,Symplus;
      union bb{
         struct cc{
            byte Auxternary,IsCExpr;
         }cc;
         word AuxLname;
      }bb;
      union aa{
         long Eval;
         tabel *STable;
         word Label[2];
      }aa;
   } gci;
#define auxlabel counter
#define auxternary gci.bb.cc.Auxternary
#define isCExpr gci.bb.cc.IsCExpr
#define auxLname gci.bb.AuxLname
#define lval gci.Lval
#define class gci.Class
#define displacement gci.Displacement
#define condition displacement
#define symminus displacement
#define symplus gci.Symplus
#define place symplus
#define eval gci.aa.Eval
#define sTable gci.aa.STable
#define label gci.aa.Label
} localmem;

extern struct t{
   byte asc,index;
   tabel *symTable;
   word priority,features;
}token;
extern FILE *out;
extern localmem currentExpr;

extern tabel symbolTable[];
