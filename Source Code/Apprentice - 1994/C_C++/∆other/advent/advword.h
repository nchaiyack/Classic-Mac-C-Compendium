
/*	header ADVWORD.H					*/


/*
  Adventure vocabulary & encryption
*/
struct wac wc[] = {
		   "\"spelunker\"", 1016,
		   "22", 2053,
		   "22", 3012,
		   "34", 2053,
		   "34", 3013,
		   "4-leafed", 5034,
		   "7", 2053,
		   "7", 3011,
		   "?", 3051,
		   "above", 29,
		   "abra", 3050,
		   "abracd", 3050,
		   "across", 42,
		   "alacaz", 3050,
		   "all", 1109,
		   "altar", 90,
		   "and", 6001,
		   "answer", 2035,
		   "anvil", 1091,
		   "ascend", 29,
		   "at", 4009,
		   "attack", 2012,
		   "awaken", 2029,
		   "awkward", 26,
		   "axe", 1028,
		   "back", 8,
		   "ball", 1120,
		   "barren", 40,
		   "bat", 1104,
		   "bats", 1104,
		   "batshit", 1104,
		   "batteries", 1039,
		   "beans", 1024,
		   "bear", 1035,
		   "bed", 16,
		   "bedquilt", 70,
		   "bee", 1087,
		   "beehive", 1097,
		   "bees", 1087,
		   "billboard", 1116,
		   "bird", 1101,
		   "bitch", 2048,
		   "black", 5006,
		   "blast", 2023,
		   "blow", 2036,
		   "blowup", 2023,
		   "boat", 1048,
		   "book", 1110,
		   "booth", 1093,
		   "bottle", 1020,
		   "box", 1055,
		   "brass", 5004,
		   "break", 2028,
		   "bridge", 89,
		   "brief", 2026,
		   "broken", 54,
		   "broom", 1114,
		   "brush", 1114,
		   "brush", 2054,
		   "building", 12,
		   "bumble", 1087,
		   "burn", 2047,
		   "cage", 1004,
		   "cake", 1107,	/* value must be mushrooms + 1 */
		   "cakes", 1107,
		   "call", 2038,
		   "calm", 2010,
		   "canister", 1118,
		   "canyon", 25,
		   "cape", 1047,
		   "capture", 2001,
		   "carpet", 1040,
		   "carry", 2001,
		   "carving", 1115,
		   "cask", 1071,
		   "catch", 2001,
		   "cave", 67,
		   "cavern", 73,
		   "chain", 1064,
		   "chalice", 1070,
		   "chant", 2003,
		   "chasm", 1021,	/* troll bridge */
		   "chest", 1055,
		   "chimney", 78,
		   "clam", 1014,
		   "click", 85,
		   "climb", 56,
		   "cloak", 1047,
		   "close", 2006,
		   "clover", 1073,
		   "cobble", 18,
		   "coins", 1054,
		   "comb", 1096,
		   "complain", 2048,
		   "continue", 7,
		   "continue", 2011,
		   "crack", 33,
		   "crap", 3106,
		   "crap!", 3106,
		   "crawl", 17,
		   "cross", 69,
		   "crown", 1066,
		   "crystal", 5033,
		   "cup", 1070,
		   "cupcakes", 1107,
		   "d", 30,
		   "dark", 22,
		   "debris", 51,
		   "defile", 23,
		   "depression", 63,
		   "descend", 30,
		   "describe", 2052,
		   "detonate", 2023,
		   "devour", 2014,
		   "diagnose", 2051,
		   "dial", 2039,
		   "diamond", 1051,
		   "diamonds", 1051,
		   "dig", 3066,
		   "discard", 2002,
		   "disturb", 2029,
		   "doff", 2002,
		   "dog", 1098,
		   "dome", 35,
		   "don", 2033,
		   "door", 1041,	/* giant door */
		   "down", 30,
		   "down", 4008,
		   "downstream", 5,
		   "downward", 30,
		   "dragon", 1031,
		   "drawing", 1029,
		   "drink", 2015,
		   "drop", 2002,
		   "droplet", 1075,
		   "dump", 2002,
		   "dust", 2054,
		   "dwarf", 1017,
		   "dwarves", 1017,
		   "e", 43,
		   "east", 43,
		   "eat", 2014,
		   "egg", 1056,
		   "eggs", 1056,
		   "elfin", 5019,
		   "emerald", 1059,
		   "empty", 2013,
		   "enter", 3,
		   "entrance", 64,
		   "everything", 1109,
		   "examine", 2052,
		   "excavate", 3066,
		   "exit", 11,
		   "explore", 2011,
		   "extinguish", 2008,
		   "fee", 2025,
		   "fee", 3001,
		   "feed", 2021,
		   "fie", 2025,
		   "fie", 3002,
		   "fight", 2012,
		   "figure", 1027,
		   "fill", 2022,
		   "find", 2019,
		   "fissure", 1012,
		   "fling", 2017,
		   "floor", 58,
		   "flower", 1046,
		   "flowers", 1046,
		   "foe", 2025,
		   "foe", 3003,
		   "follow", 2011,
		   "foo", 2025,
		   "foo", 3004,
		   "food", 1019,
		   "forcd", 1,
		   "forest", 6,
		   "fork", 77,
		   "forward", 7,
		   "fountain", 1103,
		   "four-leafed", 5034,
		   "free", 2002,
		   "fresh", 5010,
		   "from", 4005,
		   "fuck", 3079,
		   "fuck!", 3079,
		   "fum", 2025,
		   "fum", 3005,
		   "gate", 2058,
		   "get", 2044,
		   "geyser", 1037,	/* same as volcano */
		   "giant", 27,
		   "giant", 5029,
		   "glowing", 5031,
		   "gnome", 1105,
		   "go", 2011,
		   "gold", 1050,
		   "golden", 5001,
		   "goto", 2011,
		   "grab", 2032,
		   "grail", 1070,
		   "grate", 1003,
		   "green", 5032,
		   "grey", 5032,
		   "gripe", 2048,
		   "grotto", 91,
		   "guano", 1104,
		   "gully", 13,
		   "h20", 1081,
		   "hall", 38,
		   "headlamp", 1002,
		   "health", 2051,
		   "heave", 2017,
		   "heels", 1067,
		   "help", 3051,
		   "hike", 2011,
		   "hill", 2,
		   "hit", 2034,
		   "hive", 1097,
		   "hocus", 3050,
		   "hole", 52,
		   "holy", 5021,
		   "honey", 1096,
		   "honeycomb", 1096,
		   "horn", 1052,
		   "hound", 1098,
		   "house", 12,
		   "hurl", 2017,
		   "i", 2020,
		   "ice", 88,
		   "ignite", 2023,
		   "in", 19,
		   "in", 4001,
		   "insert", 2045,
		   "inside", 19,
		   "inside", 4001,
		   "into", 4001,
		   "inventory", 2020,
		   "inward", 19,
		   "iron", 5011,
		   "issue", 1016,
		   "jar", 1020,
		   "jerk", 2032,
		   "jewelry", 1053,
		   "jewels", 1053,
		   "jump", 39,
		   "keep", 2001,
		   "keg", 1071,
		   "key", 1090,
		   "keys", 1102,
		   "kick", 2034,
		   "kill", 2012,
		   "knapsack", 1108,
		   "knife", 1018,
		   "knives", 1018,
		   "knoll", 81,
		   "l", 2052,
		   "lamp", 1002,
		   "lantern", 1002,
		   "lead", 5023,
		   "leaden", 5023,
		   "leap", 39,
		   "leather", 5024,
		   "leave", 11,
		   "leave", 2037,
		   "ledge", 83,
		   "left", 36,
		   "light", 1002,
		   "little", 5012,
		   "lock", 2049,
		   "look", 2052,
		   "lost", 3068,
		   "low", 24,
		   "lyre", 1068,
		   "machine", 1038,
		   "magazine", 1016,
		   "main", 76,
		   "map", 2057,
		   "message", 1036,
		   "metal", 5035,
		   "ming", 5016,
		   "mirror", 1023,
		   "mist", 3069,
		   "moss", 1040,
		   "mumble", 2003,
		   "mushroom", 1106,
		   "mushrooms", 1106,
		   "n", 45,
		   "ne", 47,
		   "nest", 1056,
		   "north", 45,
		   "northeast", 47,
		   "northwest", 50,
		   "nothing", 2005,
		   "nowhere", 21,
		   "nugget", 1050,
		   "null", 21,
		   "nw", 50,
		   "oak", 5022,
		   "oaken", 5022,
		   "off", 4006,
		   "office", 76,
		   "oil", 1083,		/* in bottle */
		   "on", 4002,
		   "onto", 4002,
		   "onward", 7,
		   "open", 2004,
		   "opensesame", 3050,
		   "oriental", 72,
		   "out", 11,
		   "outdoors", 32,
		   "outside", 11,
		   "over", 41,
		   "oyster", 1015,
		   "pantry", 57,
		   "passage", 23,
		   "pause", 2030,
		   "pearl", 1061,
		   "persian", 5002,
		   "peruse", 2027,
		   "peyote", 1106,
		   "phone", 1094,
		   "phonebooth", 1094,
		   "phuce", 82,
		   "pick", 2041,
		   "pillow", 1010,
		   "pirate", 1030,
		   "pirloc", 2059,
		   "piss", 3107,
		   "piss!", 3107,
		   "pit", 31,
		   "placate", 2010,
		   "plant", 1024,
		   "platinum", 5017,
		   "play", 2040,
		   "plover", 71,
		   "plugh", 65,
		   "pocus", 3050,
		   "pole", 1009,
		   "pool", 80,
		   "poster", 1113,
		   "pottery", 1058,
		   "pound", 2034,
		   "pour", 2013,
		   "pray", 92,
		   "prayer", 92,
		   "proceed", 2011,
		   "pull", 2032,
		   "punch", 2034,
		   "put", 2042,
		   "pyramid", 1060,
		   "q", 2018,
		   "quartz", 5036,
		   "quit", 2018,
		   "radium", 1119,
		   "rare", 5018,
		   "ration", 1019,
		   "read", 2027,
		   "refill", 2022,
		   "release", 2002,
		   "remove", 2046,
		   "reply", 2035,
		   "report", 2048,
		   "reservoir", 75,
		   "restore", 2031,
		   "retreat", 8,
		   "return", 8,
		   "right", 37,
		   "ring", 1072,
		   "road", 2,
		   "rock", 1119,
		   "rock", 15,
		   "rocks", 1092,
		   "rocks", 1115,
		   "rod", 1005,
		   "room", 59,
		   "rowboat", 1048,
		   "rub", 2016,
		   "ruby", 5020,
		   "rug", 1062,
		   "run", 2011,
		   "rusty", 5028,
		   "s", 46,
		   "sack", 1108,
		   "safe", 1112,
		   "saint-michel", 93,
		   "sandwich", 1019,
		   "sapphire", 1069,
		   "save", 2030,
		   "say", 2003,
		   "score", 2024,
		   "se", 48,
		   "secret", 66,
		   "sesame", 3050,
		   "shadowy", 5027,
		   "shake", 2009,
		   "shards", 1058,
		   "shatter", 2028,
		   "shazam", 3050,
		   "shelf", 83,
		   "shell", 74,
		   "shield", 1118,
		   "ship", 1048,
		   "shit", 3106,
		   "shit!", 3106,
		   "shoes", 1067,
		   "shut", 2006,
		   "silk", 5013,
		   "silken", 5013,
		   "silver", 5014,
		   "sing", 2003,
		   "slab", 61,
		   "slabroom", 61,
		   "slay", 2012,
		   "slide", 79,
		   "slippers", 1067,
		   "slit", 60,
		   "slugs", 1095,
		   "small", 5012,
		   "smash", 2028,
		   "snake", 1011,
		   "south", 46,
		   "southeast", 48,
		   "southwest", 49,
		   "spelunker", 1016,
		   "sphere", 1120,
		   "spices", 1063,
		   "stair", 10,
		   "stairs", 10,
		   "stalagmite", 1026,
		   "star", 5026,
		   "steal", 2001,
		   "steel", 5025,
		   "steps", 1007,
		   "steps", 34,
		   "stick", 1049,
		   "sticks", 1049,
		   "stone", 1119,
		   "stop", 3139,
		   "stream", 14,
		   "strike", 2034,
		   "strum", 2040,
		   "suggest", 2048,
		   "surface", 20,
		   "suspend", 2030,
		   "sw", 49,
		   "sweep", 2054,
		   "swim", 3147,
		   "swing", 2009,
		   "sword", 1065,
		   "tablet", 1013,
		   "take", 2001,
		   "tame", 2010,
		   "tasty", 5030,
		   "telephone", 1094,
		   "terse", 2055,
		   "then", 6002,
		   "throw", 2017,
		   "thunder", 84,
		   "tiny", 5012,
		   "to", 4004,
		   "tome", 1110,
		   "toss", 2017,
		   "tote", 2001,
		   "travel", 2011,
		   "treasure", 5015,
		   "tree", 1074,
		   "tree", 3064,
		   "trees", 3064,
		   "trident", 1057,
		   "troll", 1033,
		   "tube", 1118,
		   "tunnel", 23,
		   "turn", 2043,
		   "u", 29,
		   "unbrief", 2026,
		   "unlock", 2050,
		   "unterse", 2055,
		   "up", 29,
		   "up", 4007,
		   "upon", 4002,
		   "upstream", 4,
		   "upward", 29,
		   "used", 5009,
		   "utter", 2003,
		   "valley", 9,
		   "vase", 1058,
		   "velvet", 5007,
		   "vending", 5008,
		   "view", 28,
		   "volcano", 1037,
		   "volume", 1110,
		   "w", 44,
		   "wake", 2029,
		   "waken", 2029,
		   "walk", 2011,
		   "wall", 53,
		   "wall", 1088,	/* in blue grotto */
		   "wand", 1005,
		   "water", 1081,	/* in bottle */
		   "wave", 2009,
		   "wear", 2033,
		   "west", 44,
		   "whack", 2034,
		   "where", 2019,
		   "whirl", 80,
		   "whirlpool", 80,
		   "whisk", 1114,
		   "whiskbroom", 1114,
		   "wicker", 5005,
		   "wine", 1085,	/* in bottle */
		   "with", 4003,
		   "wiz", 2056,
		   "wolf", 1098,
		   "wooden", 5003,
		   "worn", 5009,
		   "worn-out", 5009,
		   "wornout", 5009,
		   "wumpus", 1099,
		   "xyzzy", 62,
		   "y2", 55,
		   "yank", 2032
};
