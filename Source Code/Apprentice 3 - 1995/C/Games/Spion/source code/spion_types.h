typedef	struct enemy
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed;
	Fixed	direction;
	short	type;
	short	level;
	Boolean	use_air_resistance;	/* TRUE if the enemy is subject to air resistance */
	short	throttle;			/* Current throttle */
	long	energy;				/* Current energy */
	short	rotation;			/* Rotation of the ship */
	Boolean	has_rotating_turret;/* TRUE if turret can rotate */
	short	turret;				/* Direction turret is facing (0=fore) */
	short	turret_rotation;	/* direction, if any that the turret is rotating */
	short	fire_countdown;
	short	silo_frame;
	struct enemy	**tracer_lock_enemy;
	struct enemy	**missile_lock_enemy;
	struct enemy	**smart_bomb_lock_enemy;
	short	hit_points;
	Boolean	phased;
	Boolean	dead;
	long	phase_mask_lw;
	short	phase_mask_word;
	char	phase_mask_byte;
	char	*radar_position;
	short	exploding;		/* enemy has been hit and is going down if > 0; frame # of fireball */ 
	short	launching;		/* enemy being launched if > 0; time remaining in launch sequence */
	short	launched_from;	/* base this enemy was launched from; 0 if none */
	} enemy, *enemy_pointer, **enemy_handle;

typedef	struct goodie
	{
	Fixed	h;
	Fixed	v;
	short	type;
	} goodie, *goodie_pointer, **goodie_handle;

typedef	struct base
	{
	Fixed	h;
	Fixed	v;
	short	type;
	Boolean	dead;
	short	hit_points;
	char	*radar_position;
	short	exploding;		/* base is exploding if > 0; frame # of explosion */ 
	Boolean	launching;		/* TRUE if this base is currently launching an enemy */
	} base, *base_pointer, **base_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	short	fuse;		/* frames left before bullet disappears */
	Boolean	ours;		/* TRUE of this bullet was fired from our gun */
	Boolean	phased;
	short	phase_mask;	
	} bullet, *bullet_pointer, **bullet_handle;

typedef	struct
	{
	Fixed	start_h;
	Fixed	start_v;
	Fixed	end_h;
	Fixed	end_v;
	Fixed	speed_h;
	Fixed	speed_v;
	short	fuse;		/* frames left before blaster disappears */
	Boolean	ours;		/* TRUE of this blaster was fired from our gun */
	Boolean	phased;
	} blaster, *blaster_pointer, **blaster_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	Fixed	radius;
	short	fuse;		/* frames left before circle disappears */
	Boolean	ours;		/* TRUE of this circle was fired by us */
	Boolean	phased;
	long	phase_mask_lw;
	short	phase_mask_word;
	char	phase_mask_byte;
	} circle, *circle_pointer, **circle_handle;

typedef	struct
	{
	Fixed	start_h;
	Fixed	start_v;
	Fixed	end_h;
	Fixed	end_v;
	Boolean	ours;		/* TRUE of this laser beam was fired from our gun */
	enemy_handle	source_enemy;
	Boolean	phased;
	} laser_beam, *laser_beam_pointer, **laser_beam_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	Boolean	ours;		/* TRUE of this bomb was dropped by us */
	short	fuse;		/* frames left before bomb hits ground */
	Boolean	phased;
	Boolean	smart;
	Fixed	smart_speed_h;
	Fixed	smart_speed_v;
	enemy_handle smart_lock;
	long	phase_mask;	
	} bomb, *bomb_pointer, **bomb_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	Boolean	ours;		/* TRUE of this large bomb was dropped by us */
	short	fuse;		/* frames left before large bomb hits ground */
	Boolean	phased;
	long	phase_mask;	
	} large_bomb, *large_bomb_pointer, **large_bomb_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	short	frame;
	Boolean	phased;
	long	phase_mask_lw;
	short	phase_mask_word;
	char	phase_mask_byte;
	char	*radar_position;
	} explosion, *explosion_pointer, **explosion_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	short	fuse;		/* frames left before shell explodes */
	Boolean	ours;		/* TRUE of this shell was fired from our gun */
	Boolean	phased;
	long	phase_mask;	
	} shell, *shell_pointer, **shell_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	speed_h;
	Fixed	speed_v;
	short	direction;
	enemy_handle	enemy_lock;
	short	fuse;		/* frames left before missile crashes */
	Boolean	ours;		/* TRUE of this missile was fired by us */
	Boolean	phased;
	long	phase_mask;	
	char	*radar_position;
	} missile, *missile_pointer, **missile_handle;

typedef	struct
	{
	Fixed	h;
	Fixed	v;
	Fixed	base_speed_h;
	Fixed	base_speed_v;
	short	direction;
	enemy_handle	locked_enemy;
	short	frame;
	short	fuse;		/* frames left before missile crashes */
	Boolean	ours;		/* TRUE of this tracer was fired from our gun */
	Boolean	phased;
	long	phase_mask;	
	} tracer, *tracer_pointer, **tracer_handle;

typedef enum
	{
	ship,
	biplane,
	ground_gun,
	smart_ground_gun,
	cannon,
	silo,
	fireball
	};

typedef enum
	{
	none = 0,
	died,
	completed
	};

typedef enum
	{
	air,
	ground,
	ground_fireball
	};

typedef enum
	{
	crystal_goodie,
	weapons_goodie,
	ability_goodie
	};

typedef enum
	{
	phase_ability = 1,
	water_energy_ability,
	low_energy_ability,
	medium_energy_ability,
	high_energy_ability,
	auto_aim_ability
	};

typedef enum
	{
	single_fire_gun = 1,
	rapid_fire_gun,
	tracer_gun,
	laser_gun,
	bomb_dropper,
	large_bomb_dropper,
	smart_bomb_dropper,
	circle_weapon,
	blaster_weapon,
	missile_launcher,
	cannon_gun
	};

typedef
struct
	{
	short	width;
	short	height;
	} **world_size_resource;
