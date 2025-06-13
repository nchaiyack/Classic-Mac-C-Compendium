/* Main include file for Xconq.
   Copyright (C) 1991, 1992, 1993, 1994, 1995 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

/* This is the include file that nearly all Xconq source files
   should include (exceptions would be very low-level or generic
   sources).  While it might be possible to find a source file
   or two that does not all the definitions here, in general a
   source file will need all of these includes. */

#include "config.h"
#include "misc.h"
#include "dir.h"
#include "lisp.h"
#include "module.h"
#include "game.h"
#include "player.h"
#include "side.h"
#include "unit.h"
#include "world.h"
#include "history.h"
#include "score.h"
#include "help.h"
#include "ai.h"
#include "version.h"

/* (should be in a movies.h?) */

enum movie_type {
  movie_null,
  movie_miss,
  movie_hit,
  movie_death,
  movie_nuke,
  movie_extra_0
};

typedef struct a_movie_type {
  enum movie_type type;
  char *name;
} MovieType;

/* Declarations of globally visible globals. */

extern int typesdefined;
extern int gamedefined;
extern int compromised;
extern int cursequence;
extern char *version;
extern char *xconqlib;
extern int beforestart;
extern int endofgame;
extern int any_post_action_scores;
extern int any_people_side_changes;
extern int max_zoc_range;
extern time_t game_start_in_real_time;
extern time_t turn_play_start_in_real_time;

/* Declarations of functions that must be supplied by an interface. */

extern void announce_read_progress PROTO ((void));
extern void announce_lengthy_process PROTO ((char *msg));
extern void announce_progress PROTO ((int pctdone));
extern void finish_lengthy_process PROTO ((void));

extern Player *add_default_player PROTO ((void));

extern void init_ui PROTO ((Side *side));

extern int active_display PROTO ((Side *side));

extern void update_turn_display PROTO ((Side *side, int rightnow));
extern void update_action_display PROTO ((Side *side, int rightnow));
extern void update_action_result_display PROTO ((Side *side, Unit *unit, int rslt, int rightnow));
extern void update_event_display PROTO ((Side *side, HistEvent *hevt, int rightnow));
extern void update_fire_at_display PROTO ((Side *side, Unit *unit, Unit *unit2, int m, int rightnow));
extern void update_fire_into_display PROTO ((Side *side, Unit *unit, int x, int y, int z, int m, int rightnow));
extern void update_clock_display PROTO ((Side *side, int rightnow));
extern void update_side_display PROTO ((Side *side, Side *side2, int rightnow));
extern void update_unit_display PROTO ((Side *side, Unit *unit, int rightnow));
extern void update_unit_acp_display PROTO ((Side *side, Unit *unit, int rightnow));
extern void update_message_display PROTO ((Side *side, Side *side2, char *str, int rightnow));
extern void update_cell_display PROTO ((Side *side, int x, int y, int rightnow));
extern void update_all_progress_displays PROTO ((char *str, int s));
extern void update_everything PROTO ((void));
extern void flush_display_buffers PROTO ((Side *side));

extern int schedule_movie PROTO ((Side *side, enum movie_type movie, ...));
extern void play_movies PROTO ((SideMask sidemask));

extern void notify PROTO ((Side *side, char *str, ...));

extern void init_warning PROTO ((char *str, ...));
extern void low_init_warning PROTO ((char *str));
extern void init_error PROTO ((char *str, ...));
extern void low_init_error PROTO ((char *str));
extern void run_warning PROTO ((char *str, ...));
extern void low_run_warning PROTO ((char *str));
extern void run_error PROTO ((char *str, ...));
extern void low_run_error PROTO ((char *str));

extern void printlisp PROTO ((Obj *));

extern void close_displays PROTO ((void));

/* Declarations of functions. */

extern void init_library_path PROTO ((char *path));
extern void init_data_structures PROTO ((void));
extern void check_game_validity PROTO ((void));
extern void calculate_globals PROTO ((void));
extern void patch_object_references PROTO ((void));
extern void make_trial_assignments PROTO ((void));
extern void make_up_a_side PROTO ((void));
extern void init_side_advantage PROTO ((Side *side));
extern int add_side_and_player PROTO ((void));
extern int exchange_players PROTO ((int n, int n2));
extern int remove_side_and_player PROTO ((void));
extern void run_synth_methods PROTO ((void));
extern int make_random_date PROTO ((void));
extern int make_weather PROTO ((void));
extern void final_init PROTO ((void));
extern void configure_sides PROTO ((void));
extern void init_all_views PROTO ((void));
extern void init_area_views PROTO ((void));
extern int make_initial_materials PROTO ((void));
extern void init_supply PROTO ((Unit *unit));
extern int saved_game PROTO ((void));
extern void init_side_balance PROTO ((void));
extern void check_consistency PROTO ((void));
extern void assign_players_to_sides PROTO ((void));
extern char *version_string PROTO ((void));
extern char *copyright_string PROTO ((void));
extern char *license_string PROTO ((void));

extern void syntax_error PROTO ((Obj *x, char *msg));
extern void type_error PROTO ((Obj *x, char *msg));
extern void interp_form PROTO ((Module *module, Obj *form));
extern void interp_game_module PROTO ((Obj *form, Module *module));
extern void do_module_variants PROTO ((Module *module, Obj *lis));
extern void fill_in_side PROTO ((Side *side, Obj *props, int userdata));
extern void read_warning PROTO ((char *str, ...));

extern int eval_boolean_expression PROTO ((Obj *expr, int (*fn)(Obj *), int dflt));

extern void init_actions PROTO ((void));
extern int can_move_via_conn PROTO ((Unit *unit, int nx, int ny));
extern int can_occupy_conn PROTO ((Unit *unit, int nx, int ny, int nz));
extern int move_unit PROTO ((Unit *unit, int nx, int ny));
extern int can_move_at_all PROTO ((Unit *unit));
extern int in_blocking_zoc PROTO ((Unit *unit, int x, int y, int z));
extern int unit_blockable_by PROTO ((Unit *unit, Unit *unit2));
extern int total_move_cost PROTO ((int u, int u2, int x1, int y1, int z1, int x2, int y2, int z2));
extern int zoc_move_cost PROTO ((Unit *unit, int ox, int oy, int oz));
extern int maybe_react_to_move PROTO ((Unit *unit, int ox, int oy));
extern void consume_move_supplies PROTO ((Unit *unit));
extern int sides_allow_entry PROTO ((Unit *unit, Unit *transport));
extern int total_entry_cost PROTO ((int u1, int u3, int x1, int y1, int z1, int u2, int x2, int y2, int z2));
extern int transfer_supply PROTO ((Unit *from, Unit *to, int m, int amount));
extern void adjust_tech_crossover PROTO ((Side *side, int u));
extern void adjust_tooling_crossover PROTO ((Unit *unit, int u2));
extern void garrison_unit PROTO ((Unit *newunit, Unit *unit2));
extern void make_unit_complete PROTO ((Unit *unit));
extern void distribute_material PROTO ((Unit *unit, int m, int amt));
extern int execute_action PROTO ((Unit *unit, Action *action));
extern int can_have_enough_acp PROTO ((Unit *unit, int acp));
extern int has_enough_acp PROTO ((Unit *unit, int acp));
extern int has_supply_to_act PROTO ((Unit *unit));
extern void use_up_acp PROTO ((Unit *unit, int acp));
extern int people_surrender_chance PROTO ((int u, int x, int y));
extern void detonate_on_approach_around PROTO ((Unit *unit));
extern int unit_speed PROTO ((Unit *unit, int nx, int ny));
extern int zoc_range PROTO ((Unit *unit, int u2));
extern int damaged_value PROTO ((Unit *unit, Obj *effect, int maxval));

extern int can_research PROTO ((Unit *unit));
extern int type_can_research PROTO ((int u));
extern int can_toolup PROTO ((Unit *unit));
extern int type_can_toolup PROTO ((int u));
extern int can_create PROTO ((Unit *unit));
extern int type_can_create PROTO ((int u));
extern int can_complete PROTO ((Unit *unit));
extern int type_can_complete PROTO ((int u));
extern int can_repair PROTO ((Unit *unit));
extern int type_can_repair PROTO ((int u));
extern int can_change_type PROTO ((Unit *unit));
extern int type_can_change_type PROTO ((int u));
extern int can_disband_at_all PROTO ((Side *side, Unit *unit));
extern int any_construction_possible PROTO ((void));
extern char *action_desig PROTO ((Action *act));

extern int can_attack PROTO ((Unit *unit));
extern int type_can_attack PROTO ((int u));
extern int can_fire PROTO ((Unit *unit));
extern int type_can_fire PROTO ((int u));
extern int type_can_capture PROTO ((int u));
extern int can_detonate PROTO ((Unit *unit));

extern int one_attack PROTO ((Unit *atker, Unit *defender));
extern int found_blocking_elevation PROTO ((int u, int ux, int uy, int uz, int u2, int u2x, int u2y, int u2z));
extern void fire_on_unit PROTO ((Unit *atker, Unit *other));
extern int enough_ammo PROTO ((Unit *unit, Unit *other));
extern void attack_unit PROTO ((Unit *atker, Unit *other));
extern void maybe_hit_unit PROTO ((Unit *atker, Unit *other, int fallsoff));
extern void hit_unit PROTO ((Unit *unit, int hit, Unit *atker));
extern void reckon_damage PROTO ((void));
extern void reckon_damage_around PROTO ((int x, int y, int r));
extern void damage_unit PROTO ((Unit *unit));
extern int retreat_unit PROTO ((Unit *unit, Unit *atker));
extern int retreat_in_dir PROTO ((Unit *unit, int dir));
extern void attempt_to_capture_unit PROTO ((Unit *atker, Unit *other));
extern int capture_chance PROTO ((int u, int u2, Side *side2));
extern void capture_unit PROTO ((Unit *unit, Unit *pris));
extern void capture_occupant PROTO ((Unit *unit, Unit *pris, Unit *occ));
extern int detonate_unit PROTO ((Unit *unit, int x, int y, int z));

extern int multiply_dice PROTO ((int dice, int mult));
extern void damage_terrain PROTO ((int u, int x, int y));

extern int run_game PROTO ((int maxactions));
extern void test_for_game_start PROTO ((void));
extern void test_for_game_end PROTO ((void));
extern int damaged_acp PROTO ((Unit *unit, Obj *effect));
extern int all_sides_finished PROTO ((void));
extern void finish_turn PROTO ((Side *side));
extern void check_realtime PROTO ((void));
extern int exceeded_rt_for_game PROTO ((void));
extern int exceeded_rt_per_turn PROTO ((void));
extern void compute_acp PROTO ((Unit *unit));
extern int units_still_acting PROTO ((Side *side));
extern void resign_game PROTO ((Side *side, Side *side2));
extern int realtime_game PROTO ((void));
extern int all_others_willing_to_save PROTO ((Side *side));
extern int all_others_willing_to_quit PROTO ((Side *side));
extern void end_the_game PROTO ((void));
extern void change_people_side_around PROTO ((int x, int y, int u, Side *side));
extern void init_random_events PROTO ((void));
extern void maybe_starve PROTO ((Unit *unit, int partial));
extern int people_surrender_chance PROTO ((int u, int x, int y));
extern void compose_actionvector PROTO ((void));
extern int unit_priority PROTO ((Unit *unit));

extern int make_fractal_terrain PROTO ((void));
extern int make_random_terrain PROTO ((void));
extern int make_earthlike_terrain PROTO ((void));
extern int make_maze_terrain PROTO ((void));
extern int name_geographical_features PROTO ((void));
extern void add_edge_terrain PROTO ((void));

extern int make_countries PROTO ((void));
extern int make_independent_units PROTO ((void));

extern int make_rivers PROTO ((void));

extern int make_roads PROTO ((void));

extern Obj *make_namer PROTO ((Obj *sym, Obj *meth));

extern void init_namers PROTO ((void));
extern void make_up_side_name PROTO ((Side *side));
extern int name_in_use PROTO ((Side *side, char *str));
extern int name_units_randomly PROTO ((void));
extern char *unit_namer PROTO ((Unit *unit));
extern char *propose_unit_name PROTO ((Unit *unit));
extern void make_up_unit_name PROTO ((Unit *unit));
extern void assign_unit_number PROTO ((Unit *unit));
extern char *run_namer PROTO ((Obj *namer));
extern char *name_from_grammar PROTO ((Obj *grammar));
extern void gen_name PROTO ((Obj *nonterm, Obj *rules, int depth, char *rslt));
extern void gen_from_rule PROTO ((Obj *rule, Obj *rules, int depth, char *rslt));

extern void pad_out PROTO ((char *buf, int n));
extern char *short_side_title PROTO ((Side *side));
extern char *shortest_side_title PROTO ((Side *side2, char *buf));
extern char *rel_short_side_title PROTO ((Side *side, Side *side2, int n));
extern char *rel_long_side_title PROTO ((Side *side, Side *side2));
extern char *narrative_side_desc PROTO ((Side *side, Side *side2));
extern char *narrative_side_desc_first PROTO ((Side *side, Side *side2));
extern char *long_player_title PROTO ((char *buf, Player *player, char *thisdisplayname));
extern char *short_player_title PROTO ((char *buf, Player *player, char *thisdisplayname));
extern void side_and_type_name PROTO ((char *buf, Side *side, int u, Side *side2));
extern char *unit_handle PROTO ((Side *side, Unit *unit));
extern char *short_unit_handle PROTO ((Unit *unit));
extern void name_or_number PROTO ((Unit *unit, char *buf));
extern void construction_desc PROTO ((char *buf, Unit *unit, int u));
extern void constructible_desc PROTO ((char *buf, Side *side, int u, Unit *unit));
extern int est_completion_time PROTO ((Unit *unit, int u2));
extern void historical_event_date_desc PROTO ((HistEvent *hevt, char *buf));
extern void historical_event_desc PROTO ((Side *side, HistEvent *hevt, char *buf));
extern void linear_desc PROTO ((char *buf, int x, int y));
extern void elevation_desc PROTO ((char *buf, int x, int y));
extern char *feature_desc PROTO ((Feature *feature, char *buf));
extern char *feature_name_at PROTO ((int x, int y));
extern void temperature_desc PROTO ((char *buf, int x, int y));
extern void hp_desc PROTO ((char *buf, Unit *unit, int label));
extern void acp_desc PROTO ((char *buf, Unit *unit, int label));
extern void plan_desc PROTO ((char *buf, Unit *unit));
extern void task_desc PROTO ((char *buf, Task *task));
extern void time_desc PROTO ((char *buf, int time, int maxtime));
extern char *summarize_units PROTO ((char *buf, int *ucnts));
extern char *exit_commentary PROTO ((Side *side));
extern char *ordinal_suffix PROTO ((int n));
extern char *plural_form PROTO ((char *word));
extern char *make_text PROTO ((char *buf, Obj *maker, long a1, long a2, long a3, long a4));
extern char *absolute_date_string PROTO ((long date));
extern char *relative_date_string PROTO ((long date, long base));
extern void parse_usual_initial_date PROTO ((void));
extern void write_unit_record PROTO ((FILE *fp, Side *side));
extern void write_side_results PROTO ((FILE *fp, Side *side));
extern void write_combat_results PROTO ((FILE *fp, Side *side));
extern void dice_desc PROTO ((char *buf, int dice));
extern char *past_unit_handle PROTO ((Side *side, PastUnit *past_unit));
extern char *short_past_unit_handle PROTO ((PastUnit *past_unit));
extern void past_name_or_number PROTO ((PastUnit *past_unit, char *buf));
extern void record_missing_image PROTO ((int typtyp, char *str));
extern int missing_images PROTO ((char *buf));

extern void describe_command PROTO ((int ch, char *name, char *help, int onechar, char *buf));

extern FILE *open_module_library_file PROTO ((Module *module));
extern FILE *open_module_explicit_file PROTO ((Module *module));
extern FILE *open_library_file PROTO ((char *filename));
extern char *default_library_filename PROTO ((void));
extern char *news_filename PROTO ((void));
extern char *saved_game_filename PROTO ((void));
extern char *checkpoint_filename PROTO ((void));
extern char *error_save_filename PROTO ((void));
extern char *statistics_filename PROTO ((void));
extern void remove_saved_game PROTO ((void));
extern void init_signal_handlers PROTO ((void));
extern int n_seconds_elapsed PROTO ((int n));

extern void doublecheck_state PROTO ((Side *side));
extern long extension_value PROTO ((Obj *extensions, char *key, long dflt));
extern void set_u_internal_name PROTO ((int u, char *s));
extern void set_u_type_name PROTO ((int u, char *s));
extern void set_m_type_name PROTO ((int m, char *s));
extern void set_t_type_name PROTO ((int t, char *s));
extern int coerce_to_side_id PROTO ((Obj *x));
extern Side *coerce_to_side PROTO ((Obj *x));
extern int coerce_to_unit_id PROTO ((Obj *x));
extern Unit *coerce_to_unit PROTO ((Obj *x));

extern int write_entire_game_state PROTO ((char *fname));
extern int write_game_module PROTO ((Module *module));

int total_gain PROTO ((Side *side, int u));
int total_loss PROTO ((Side *side, int u));
