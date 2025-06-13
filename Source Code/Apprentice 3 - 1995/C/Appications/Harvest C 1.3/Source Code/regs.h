
/* Register definitions */

/* movec registers */
#define SFC     0x000
#define DFC     0x001
#define CACR    0x002
#define USP     0x800
#define VBR     0x801
#define CAAR    0x802
#define MSP     0x803
#define ISP     0x804

/* floating point */
#define FPCR    (1<<12)
#define FPSR    (1<<11)
#define FPIAR   (1<<10)

/* memory management */
#define TC      0x4000
#define DRP     0x4400
#define SRP     0x4800
#define CRP     0x4C00
#define CAL     0x5000
#define VAL     0x5400
#define SCC     0x5800
#define AC      0x5C00
#define BAD0    0x7000
#define BAD1    0x7004
#define BAD2    0x7008
#define BAD3    0x700C
#define BAD4    0x7010
#define BAD5    0x7014
#define BAD6    0x7018
#define BAD7    0x701C
#define BAC0    0x7400
#define BAC1    0x7404
#define BAC2    0x7408
#define BAC3    0x740C
#define BAC4    0x7410
#define BAC5    0x7414
#define BAC6    0x7418
#define BAC7    0x741C
#define PSR     0x6000
#define PCSR    0x6400
#define TT0     0x0800		/* 68030 only */
#define TT1     0x0C00		/* 68030 only */

struct regs {
    char                           *r_name;
    char                            r_type;
    int                             r_value;
};

extern struct regs              iregs[];

#define NIREGS  (sizeof(iregs)/sizeof(iregs[0]))
