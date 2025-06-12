/* Prototypes of functions. */

/* (should be able to generate all these automatically) */

char *savefile_name();
char *newsfile_name();
FILE *open_library_file();
FILE *open_explicit_file();

int announce_read_progress();
int announce_lengthy_process();
int announce_progress();
int finish_lengthy_process();

Player *add_default_player();

int update_turn_display();
int update_action_display();
int update_action_result_display();
int update_clock_display();
int update_side_display();
int update_unit_display();
int update_cell_display();
int update_everything();
int draw_blast();
int flush_display_buffers();

int init_warning();
int init_error();
int abort_init();
int run_warning();
int run_error();

/* function prototypes from ai.c */
Goal *create_goal();
char *goal_desig();

/* function prototypes from generic.c */
char *index_type_name();

/* function prototypes from help.c */
void create_game_help_nodes();
HelpNode *create_help_node();
HelpNode *add_help_node PROTO((char *key, int (*fn)(), int arg, HelpNode *prevnode));
char *get_help_text PROTO((HelpNode *node));
int describe_topics PROTO((int arg, char *key, char *buf));
int describe_news PROTO((int arg, char *key, char *buf));
int describe_game_design PROTO((int arg, char *key, char *buf));
int describe_game_modules PROTO((int arg, char *key, char *buf));
int describe_utype PROTO((int arg, char *key, char *buf));
int describe_mtype PROTO((int arg, char *key, char *buf));
int describe_ttype PROTO((int arg, char *key, char *buf));
int describe_scorekeepers PROTO((int arg, char *key, char *buf));
int describe_concepts PROTO((int arg, char *key, char *buf));

/* function prototypes from history.c */
HistEvent *create_historical_event();
HistEvent *record_event();

/* function prototypes from init.c */
char *version_string();
char *copyright_string();

/* function prototypes from lisp.c */
Obj *newobj();
Obj *read_form();
Obj *read_form_aux();
Obj *read_list();
Obj *new_string();
Obj *new_number();
Obj *new_utype();
Obj *new_mtype();
Obj *new_ttype();
Obj *new_method();
Obj *cons();
Obj *car();
Obj *cdr();
Obj *cadr();
Obj *cddr();
Obj *caddr();
char *c_string();
int c_number();
Obj *intern_symbol();
Obj *symbol_value();
Obj *setq();
Obj *make_namer();
Obj *elt();
Obj *eval();
Obj *eval_symbol();
Obj *eval_list();

/* function prototypes from mknames.c */
char *run_namer();
char *name_from_grammar();
char *unit_namer();
char *propose_unit_name();

/* function prototypes from mkunits.c */
Unit *find_place_for_occupant();

/* function prototypes from module.c */
void load_game_module PROTO((Module *module, int dowarn));
int open_module PROTO((Module *module, int dowarn));
Module *create_game_module();
Module *get_game_module();
Module *add_game_module();
char *module_desig();

/* function prototypes from <system>.c */
FILE *open_library_module();
FILE *open_explicit_file();
char *newsfile_name();
char *savefile_name();
char *checkpoint_name();

/* function prototypes from nlang.c */
char *short_side_title();
char *shortest_side_title();
void long_player_title PROTO((char *buf, Player *player, char *name));
void short_player_title PROTO((char *buf, Player *player, char *name));
void side_and_type_name PROTO((char *buf, Side *side, int u, Side *side2));
char *unit_handle PROTO((Side *side, Unit *unit));
char *short_unit_handle();
char *exit_commentary();
char *ordinal_suffix();
char *plural_form();
char *usual_date_string();
char *absolute_date_string();
char *relative_date_string();
char *season_name();

/* function prototypes from plan.c */
char *task_desig();
char *plan_desig();

/* function prototypes from plan2.c */
Task *make_route_step();
Task *make_route_chain();
Task *find_route_aux();
Task *find_route();
Task *find_route_aux_nearest();
Task *find_route_to_nearest();

/* function prototypes from read.c */
char *value_keyword();
char *keyword_name();
long extension_value();

/* function prototypes from run.c */
int run_game PROTO((int maxactions));
Unit *autonext_unit PROTO((Side *side, Unit *unit));

/* function prototypes from score.c */
int eval_sk_form();
Scorekeeper *create_scorekeeper();

/* function prototypes from side.c */
Side *create_side();
char *side_name();
char *side_adjective();
Side *side_n();
Unit *find_next_unit();
Unit *find_prev_unit();
Unit *find_next_actor();
Unit *find_prev_actor();
char *side_desig();
Player *add_player();
Player *find_player();
char *player_desig();
Agreement *create_agreement();
char *agreement_desig();
void see_exact PROTO((Side *side, int x, int y));
void set_side_view();

/* function prototypes from task.c */
char *task_desig PROTO((Task *task));
/* char *create_task();                  specified in unit.h */
TaskOutcome execute_task();
Task *create_moveto_task();
Task *create_movenear_task();
Task *create_movedir_task();
Task *create_build_task();

/* function prototypes from unit.c */
Unit *create_bare_unit();
Unit *create_unit PROTO((int u, int makeplan));
Unit *designer_create_unit PROTO((Side *side, int u, int s, int x, int y));
char *unit_desig PROTO((Unit *unit));
char *utype_name_n();
char *actorstate_desig();
char *summarize_units();
Unit *find_unit();
Unit *first_unit();

/* function prototypes from util.c */
char *xmalloc PROTO((int n));
char *copy_string PROTO((char *orig));

/* function prototypes from world.c */
Feature *create_feature();
Feature *find_feature();
Feature *feature_at();
char *feature_name_at();

/* function prototypes from write.c */
char *escaped_symbol();
char *escaped_string();
char *shortest_unique_name();
