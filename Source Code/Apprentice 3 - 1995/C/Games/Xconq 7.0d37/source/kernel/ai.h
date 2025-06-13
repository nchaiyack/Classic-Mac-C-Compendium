/* Definitions common to all AIs.
   Copyright (C) 1992, 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

typedef enum {
    nobrains = 0,
    mplayertype = 1
    /* plus other types as defined */
#ifdef MPW_C
    /* Works around an MPW C bug where byte-sized enums lose - symptom
       is that AIs never do anything. */
    , aitype_intifier = 1000000
#endif /* MPW_C */
} AIType;

/* Definition common to all ai type. (?) */

typedef struct a_ai {
  AIType type;
} AI;

#define side_ai_type(s) (((AI *) (s)->ai)->type)

/* Add declaration of AI hooks here. */

extern char *aitypenames[];

extern void init_ai PROTO ((Side *side));
extern void set_side_ai PROTO ((Side *side, char *typename));
extern void ai_init_turn PROTO ((Side *side));
extern void ai_decide_plan PROTO ((Side *side, Unit *unit));
extern void ai_react_to_unit_loss PROTO ((Side *side, Unit *unit));
extern void ai_react_to_action_result PROTO ((Side *side, Unit *unit,
					      int rslt));
extern void ai_react_to_task_result PROTO ((Side *side, Unit *unit,
					    Task *task, int rslt));
extern int ai_guide_explorer PROTO ((Side *side, Unit *unit));
extern int ai_preferred_build_type PROTO ((Side *side, Unit *unit,
					   int plantype));
extern void ai_finish_movement PROTO ((Side *side));
extern void ai_receive_message PROTO ((Side *side, Side *sender, char *str));
extern void ai_write_state PROTO ((FILE *fp, Side *side));
extern void ai_read_state PROTO ((Side *side, Obj *data));
extern int ai_region_at PROTO ((Side *side, int x, int y));
extern char *ai_at_desig PROTO ((Side *side, int x, int y));
