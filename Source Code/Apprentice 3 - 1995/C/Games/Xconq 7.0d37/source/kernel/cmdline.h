/* Command-line parsing definitions for Xconq.
   Copyright (C) 1993, 1994 Stanley T. Shebs.

Xconq is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.  See the file COPYING.  */

enum parsestage {
    general_options,
    variant_options,
    player_options,
    interface_options,
    leftover_options
};

extern void init_options PROTO ((void));
extern void parse_command_line PROTO ((int argc, char **argv, int spec));
extern void add_a_module PROTO ((char *name, char *filename));
extern void general_usage_info PROTO ((void));
extern void game_usage_info PROTO ((void));
extern void unixify_variant_name PROTO ((char *buf, char *varname));
extern void player_usage_info PROTO ((void));
extern void load_all_modules PROTO ((void));
extern void set_variants_from_options PROTO ((void));
extern void set_players_from_options PROTO ((void));
extern void parse_player_spec PROTO ((Player *player, char *spec));
extern void print_instructions PROTO ((void));
