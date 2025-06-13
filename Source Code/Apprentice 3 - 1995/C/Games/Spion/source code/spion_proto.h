#include <Sound.h>

/**************************** Prototypes ***************************/

void do_menu(long command);
short event_loop(void);
void handle_mouse_down(EventRecord	*event);
void init_menus(void);
void init_windows(void);
void main(void);
void init_program(void);
void init_graphics_data(void);
void draw_background(void);
void draw_ship(void);
void draw_bullets(void);
void draw_bombs(void);
void scroll_background(Rect *bounds, short hscroll, short vscroll);
void init_data(void);
void draw_ship(void);
void update_window(Boolean offset);
void handle_update_event(void);
void handle_key_down(EventRecord *event);
void handle_key_up(EventRecord *event);
void draw_throttle(void);
void draw_energy_status(void);
void draw_thermometer(Rect *therm_rect, long current_value, long max_value, Boolean danger);
void init_new_level(short level);
short update_frame(void);
void draw_damage_indicator(void);
void update_radar(Boolean first_time);
pascal void set_refresh_flag(void);
void install_vbl_task(void);
void remove_vbl_task(void);
void draw_turret(void);
void draw_turret_indicator(short direction);
void create_offscreen_grafport(CGrafPtr offscreen_port, Rect *bounds_rect,
								char **pixmap_base);
void dispose_offscreen_grafport (CGrafPtr offscreen_ptr);
void draw_enemies(short level);
void update_bullets(void);
Boolean update_enemies(void);
void fire_bullet(enemy_pointer which_enemy);
void check_ship_collisions(void);
void create_enemy(short type, Fixed h, Fixed v, Fixed speed, Fixed direction,
					short launched_from);
pascal void sound_done_proc(SndChannelPtr channel, SndCommand command);
void update_enemy_rotation(register enemy_handle this_enemy_handle);
void update_enemy_position(enemy_pointer which_enemy);
void update_missile_direction(missile *this_enemy);
void update_score_bar(Boolean draw_all);
void init_new_game(void);
void read_picture_and_mask(short pict_id, long pict_size, Ptr pixmap_base, 
							Ptr *bitmap_ptr, Ptr *mask_ptr);
void read_picture(short pict_id, long pict_size, Ptr pixmap_base, Ptr *bitmap_ptr);
void update_bombs(void);
void fire_shell(enemy_pointer which_enemy, short fuse);
void update_shells(void);
void draw_shells(void);
void update_missiles(void);
void fire_missile(enemy_pointer which_enemy);
void draw_missiles(void);
void destroy_bullet(short which_bullets);
void destroy_shell(short which_shell);
void destroy_missile(short which_missile);
void destroy_bomb(short which_bomb);
void destroy_enemy(short which_enemy);
void fire_tracer(enemy_pointer which_enemy);
void update_tracers(void);
void update_tracer_direction(register tracer_pointer this_tracer);
void destroy_tracer(short which_tracer);
void draw_status_lights(Boolean draw_all);
void draw_weapon(short which_weapon);
void draw_ability(short which_ability);
void fire_weapon(enemy_handle source_enemy_handle, short weapon_type, long param, Boolean repeat);
Boolean check_tracer_lock(register enemy_pointer source_enemy, register enemy_pointer check_enemy);
void update_tracer_lock(enemy_pointer source_enemy);
void update_weapon(enemy_pointer source_enemy, short weapon_type);
void draw_tracers(void);
void create_explosion(Fixed h, Fixed v, Boolean phased);
void update_explosions(void);
void destroy_explosion(short which_explosion);
void update_large_bombs(void);
void destroy_large_bomb(short which_large_bomb);
void drop_large_bomb(enemy_pointer which_enemy);
void draw_large_bombs(void);
void draw_explosions(void);
void draw_bases(short level);
void update_smart_bomb_direction(register bomb_pointer this_bomb);
void drop_bomb(enemy_pointer which_enemy, Boolean smart);
void update_smart_bomb_lock(enemy_pointer source_enemy);
Boolean check_smart_bomb_lock(register enemy_pointer source_enemy, register enemy_pointer check_enemy);
void fire_laser(enemy_handle which_enemy_handle);
void play_explosion_sound(void);
void explode_enemy(register enemy_pointer which_enemy, Boolean ours);
void check_line_hits(Fixed x1, Fixed y1, Fixed x2, Fixed y2, Boolean line_phased,
						enemy_handle exempt_enemy, Boolean ours);
Boolean check_segment_intersection(Fixed a1x, Fixed a1y, Fixed b1x, Fixed b1y,
									Fixed a2x, Fixed a2y, Fixed b2x, Fixed b2y);
void draw_laser_beams(void);
void destroy_laser_beam(short which_laser_beam);
void update_laser_beams(void);
void destroy_circle(short which_circle);
void fire_circle(enemy_pointer which_enemy);
void update_circles(void);
void draw_circles(void);
void fire_blaster(enemy_pointer which_enemy);
void update_blasters(void);
void destroy_blaster(short which_blaster);
void draw_blasters(void);
Boolean check_enemy_collisions(Rect *check_rect, short phased, short level,
								short start_enemy, Boolean ours, Boolean *hit_ours);
Boolean check_bullet_collisions(Rect *check_rect, short phased, Boolean *ours);
Boolean check_tracer_collisions(Rect *check_rect, short phased, Boolean *ours);
Boolean explode_missile(missile_pointer which_missile, short i, Boolean ours);
short find_intercept_direction(Fixed ax, Fixed ay, Fixed bx, Fixed by, Fixed Ex, Fixed Ey,
								Fixed se, Boolean smart, Boolean *in_range);
short vector_to_direction(long x, long y);
void create_base(short type, Fixed h, Fixed v);
void launch_enemy(short which_base);
void explode_base(register base_pointer which_base, Boolean ours);
void update_bases(void);
void destroy_base(short which_base);
void create_fireball(Fixed where_h, Fixed where_v, Boolean phased);
void create_goodie(short type, Fixed h, Fixed v);
void get_random_point(Fixed *h, Fixed *v);
void draw_goodies(void);
void destroy_goodie(short which_goodie);
void play_sound(Handle sound, SndChannelPtr channel);
void update_goodies(void);
void hit_base(base_pointer which_base, short damage);
void hit_enemy(enemy_pointer which_enemy, short damage);
void drain_energy(long how_much);
void read_pictures_and_masks(short first_resource_id, short num_picts, short bitmap_size,
								Ptr *bitmaps, Ptr *masks, CGrafPtr offscreen_port,
								Ptr pixmap_base, Rect *status_rect, long *status_count,
								long total_count);
short find_color_word(RGBColor *color, Ptr pixmap_base);




