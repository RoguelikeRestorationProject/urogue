/*
    init.c - global variable initializaton
 
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
    Notes

        Need to add ring of maintain armor (same as ring of prot, armor only)
        Resplit file into one just for data, one just for functions
*/

#define _ALL_SOURCE /* Need to remove need for this AIXism */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "rogue.h"

static char *rainbow[] =
{
    "Red", "Blue", "Green", "Yellow",
    "Black", "Brown", "Orange", "Pink",
    "Purple", "Grey", "White", "Silver",
    "Gold", "Violet", "Clear", "Vermilion",
    "Ecru", "Turquoise", "Magenta", "Amber",
    "Topaz", "Plaid", "Tan", "Tangerine",
    "Aquamarine", "Scarlet", "Khaki", "Crimson",
    "Indigo", "Beige", "Lavender", "Saffron"
};

#define NCOLORS (sizeof rainbow / sizeof (char *))

static char *sylls[] =
{
    "a", "ab", "ag", "aks", "ala", "an", "ankh", "app", "arg", "arze",
    "ash", "ban", "bar", "bat", "bek", "bie", "bin", "bit", "bjor",
    "blu", "bot", "bu", "byt", "comp", "con", "cos", "cre", "dalf",
    "dan", "den", "do", "e", "eep", "el", "eng", "er", "ere", "erk",
    "esh", "evs", "fa", "fid", "for", "fri", "fu", "gan", "gar",
    "glen", "gop", "gre", "ha", "he", "hyd", "i", "ing", "ion", "ip",
    "ish", "it", "ite", "iv", "jo", "kho", "kli", "klis", "la", "lech",
    "man", "mar", "me", "mi", "mic", "mik", "mon", "mung", "mur",
    "nej", "nelg", "nep", "ner", "nes", "nes", "nih", "nin", "o", "od",
    "ood", "org", "orn", "ox", "oxy", "pay", "pet", "ple", "plu", "po",
    "pot", "prok", "re", "rea", "rhov", "ri", "ro", "rog", "rok", "rol",
    "sa", "san", "sat", "see", "sef", "seh", "shu", "ski", "sna",
    "sne", "snik", "sno", "so", "sol", "sri", "sta", "sun", "ta",
    "tab", "tem", "ther", "ti", "tox", "trol", "tue", "turs", "u",
    "ulk", "um", "un", "uni", "ur", "val", "viv", "vly", "vom", "wah",
    "wed", "werg", "wex", "whon", "wun", "xo", "y", "yot", "yu",
    "zant", "zap", "zeb", "zim", "zok", "zon", "zum"
};

static char *stones[] =
{
    "Agate", "Alexandrite", "Amethyst",
    "Azurite", "Carnelian", "Chrysoberyl",
    "Chrysoprase", "Citrine", "Diamond",
    "Emerald", "Garnet", "Hematite",
    "Jacinth", "Jade", "Kryptonite",
    "Lapus lazuli", "Malachite", "Moonstone",
    "Obsidian", "Olivine", "Onyx",
    "Opal", "Pearl", "Peridot",
    "Quartz", "Rhodochrosite", "Ruby",
    "Sapphire", "Sardonyx", "Serpentine",
    "Spinel", "Tiger eye", "Topaz",
    "Tourmaline", "Turquoise"
};

#define NSTONES (sizeof stones / sizeof (char *))

static char *wood[] =
{
    "Avocado wood", "Balsa", "Banyan", "Birch",
    "Cedar", "Cherry", "Cinnibar", "Dogwood",
    "Driftwood", "Ebony", "Eucalyptus", "Hemlock",
    "Ironwood", "Mahogany", "Manzanita", "Maple",
    "Oak", "Pine", "Redwood", "Rosewood",
    "Teak", "Walnut", "Zebra wood", "Persimmon wood"
};

#define NWOOD (sizeof wood / sizeof (char *))

static char *metal[] =
{
    "Aluminium", "Bone", "Brass", "Bronze",
    "Copper", "Chromium", "Iron", "Lead",
    "Magnesium", "Pewter", "Platinum", "Steel",
    "Tin", "Titanium", "Zinc", "Carbon",
	"Plastic", "Glass", "Ice", "Chocolate", 
	"Gold", "Silver", "Invisible"
};

#define NMETAL (sizeof metal / sizeof (char *))

const char *monstern = "monster";
char *spacemsg = "--Press SPACE to continue--";
char *morestr  = "--More--";
char *retstr   = "[Press RETURN to continue]";

/* 15 named levels */

const char *cnames[C_NOTSET][15] =
{
    {   "Veteran", "Warrior",
        "Swordsman", "Hero",    /* Fighter */
        "Swashbuckler", "Myrmidon",
        "Champion", "Superhero",
        "Lord", "Lord",
        "Lord", "Lord",
        "Lord", "Lord",
        "Lord"
    },

    {   "Gallant", "Keeper",
        "Protector", "Defender",    /* Paladin */
        "Warder", "Guardian",
        "Chevalier", "Justiciar",
        "Paladin", "Paladin",
        "Paladin", "Paladin",
        "Paladin", "Paladin",
        "Paladin"
    },

    {   "Runner", "Strider",
        "Scout", "Courser", /* Ranger */
        "Tracker", "Guide",
        "Pathfinder", "Ranger",
        "Ranger Knight", "Ranger Lord",
        "Ranger Lord", "Ranger Lord",
        "Ranger Lord", "Ranger Lord",
        "Ranger Lord"
    },

    {   "Acolyte", "Adept",
        "Priest", "Curate", /* Cleric */
        "Prefect", "Canon",
        "Lama", "Patriarch",
        "High Priest", "High Priest",
        "High Priest", "High Priest",
        "High Priest", "High Priest",
        "High Priest"
    },

    {   "Aspirant", "Ovate",   /* Druid */
        "Initiate of the 1st Circle", "Initiate of the 2nd Circle",
        "Initiate of the 3rd Circle", "Initiate of the 4th Circle",
        "Initiate of the 5th Circle", "Initiate of the 6th Circle",
        "Initiate of the 7th Circle", "Initiate of the 8th Circle",
        "Initiate of the 9th Circle", "Druid",
        "Archdruid", "The Great Druid",
        "The Grand Druid"
    },

    {   "Prestidigitator", "Evoker",
        "Conjurer", "Theurgist",    /* Magic User */
        "Thaumaturgist", "Magician",
        "Enchanter", "Warlock",
        "Sorcerer", "Necromancer",
        "Wizard", "Wizard",
        "Wizard", "Wizard",
        "Wizard"
    },

    {   "Prestidigitator", "Minor Trickster",
        "Trickster", "Master Trickster",    /* Illusionist */
        "Cabalist", "Visionist",
        "Phantasmist", "Apparitionist",
        "Spellbinder", "Illusionist",
        "Illusionist", "Illusionist",
        "Illusionist", "Illusionist",
        "Illusionist"
    },

    {   "Rogue", "Footpad",
        "Cutpurse", "Robber",   /* Thief */
        "Burglar", "Filcher",
        "Sharper", "Magsman",
        "Thief", "Master Thief",
        "Master Thief", "Master Thief",
        "Master Thief", "Master Thief",
        "Master Thief"
    },

    {   "Bravo", "Rutterkin",
        "Waghalter", "Murderer",    /* Assasin */
        "Thug", "Killer",
        "Cutthroat", "Executioner",
        "Assassin", "Expert Assassin",
        "Senior Assassin", "Chief Assassin",
        "Prime Assassin", "Guildmaster Assassin",
        "Grandfather of Assassins"
    },

    {   "Ninja", "Ninja",
        "Ninja", "Ninja",   /* Ninja */
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja", "Ninja",
        "Ninja"
    }
};

const struct h_list helpstr[] =
{
    { '?',      "  prints help"                         },
    { '/',      "  identify object"                     },
    { 'h',      "  left"                                },
    { 'j',      "  down"                                },
    { 'k',      "  up"                                  },
    { 'l',      "  right"                               },
    { 'y',      "  up & left"                           },
    { 'u',      "  up & right"                          },
    { 'b',      "  down & left"                         },
    { 'n',      "  down & right"                        },
    { '<',      "SHIFT><dir> run that way"              },
    { 'm',      "<dir> move onto without picking up"    },
    { 't',       "<dir> throw something"                },
    { 'z',      "<dir> zap a wand or staff"             },
    { '>',      "  go down a staircase"                 },
    { 's',      "  search for trap/secret door"         },
    { '.',      "  rest for a while"                    },
    { ',',      "  pick up an object"                   },
    { 'i',      "  inventory all items"                 },
    { 'I',      "  inventory type of item"              },
    { 'q',      "  quaff potion"                        },
    { 'r',      "  read paper"                          },
    { 'e',      "  eat food"                            },
    { 'w',      "  wield a weapon"                      },
    { 'W',      "  wear armor"                          },
    { 'T',      "  take armor off"                      },
    { 'P',      "  put on ring"                         },
    { 'R',      "  remove ring"                         },
    { 'A',      "  activate/apply an artifact"          },
    { 'd',      "  drop object"                         },
    { 'C',      "  call object (generic)"               },
    { 'M',      "  mark object (specific)"              },
    { 'o',      "  examine/set options"                 },
    { 'c',      "  cast a spell/say a prayer"           },
    { 'p',      "  pray for help (risky)"               },
    { 'a',      "  affect the undead"                   },
    { '^',      "  set a trap"                          },
    { 'D',      "  dip something (into a pool)"         },
    { 20,       "<dir>  take (steal) from (direction)"  }, /* ctrl-t */
    { 18,       "   redraw screen"                      }, /* ctrl-r */
    { 16,       "   back up to 10 previous messages"    }, /* ctrl-p */
    { ESCAPE,   "   cancel command"                     },
    { 'v',      "  print program version number"        },
    { 'S',      "  save game"                           },
    { 'Q',      "  quit"                                },
    { '=',      "  listen for monsters"                 },
    { 'f',      "<dir> fight monster"                   },
    { 'F',      "<dir> fight monster to the death"      },

    /* Wizard commands.  Identified by (h_ch != 0 && h_desc == 0). */

    {'-',       0                                       },
    { 23,       "   enter wizard mode"                  }, /* ctrl-w */
    { 23,       "v  toggle wizard verbose mode"         },
    { 23,       "e  exit wizard mode"                   },
    { 23,       "r  random number check"                },
    { 23,       "s  system statistics"                  },
    { 23,       "F  food statistics"                    },
    { 23,       "f  floor map"                          },
    { 23,       "m  see monster"                        },
    { 23,       "M  create monster"                     },
    { 23,       "c  create item"                        },
    { 23,       "i  inventory level"                    },
    { 23,       "I  identify item"                      },
    { 23,       "t  random teleport"                    },
    { 23,       "g  goto level"                         },
    { 23,       "C  charge item"                        },
    { 23,       "w  print worth of object"              },
    { 23,       "o  improve stats and pack"             },
    { 0,        0                                       }
};

struct magic_item things[] =
{
    {"potion",  "POTION",   250,    5}, /* potion           */
    {"scroll",  "SCROLL",   260,    30},/* scroll           */
    {"ring",    "RING",     70,     5}, /* ring             */
    {"stick",   "STICK",    60,     0}, /* stick            */
    {"food",    "FOOD",     210,    7}, /* food             */
    {"weapon",  "WEAPON",   60,     0}, /* weapon           */
    {"armor",   "ARMOR",    90,     0}, /* armor            */
    {"artifact","ARTIFACT", 0,      0}  /* special artifacts*/
};

int numthings = NUMTHINGS;

struct magic_item s_magic[] =
{
    {"monster confusion",   "CON",      50, 125,    0,  0   },
    {"magic mapping",       "MAP",      45, 150,    20, 10  },
    {"light",               "WATT",     0,  0,      0,  0   },
    {"hold monster",        "HOLD",     25, 200,    33, 10  },
    {"sleep",               "SNOOZE",   23, 50,     20, 10  },
    {"enchantment",         "ENCHANT",  110,400,    15, 10  },
    {"identify",            "ID",       150,50,     0,  15  },
    {"scare monster",       "SCARE",    35, 250,    27, 21  },
    {"detect gold",         "GOLD",     0,  0,      0,  0   },
    {"teleportation",       "TELEP",    50, 165,    10, 20  },
    {"create monster",      "CREATE",   25, 75,     30, 0   },
    {"remove curse",        "REM",      75, 220,    10, 15  },
    {"petrification",       "PET",      25, 285,    0,  0   },
    {"genocide",            "GEN",      10, 1200,   0,  0   },
    {"cure disease",        "CURE",     70, 80,     0,  0   },
    {"acquirement",         "MAKE",     5,  2500,   50, 15  },
    {"protection",          "PROT",     50, 1150,   0,  0   },
    {"nothing",             "NOTHING",  75, 50,     50, 50  },
    {"magic hitting",       "SILVER",   25, 1875,   45, 10  },
    {"ownership",           "OWN",      15, 1550,   45, 10  },
    {"detect food",         "FOOD",     0,  0,      0,  0   },
    {"electrification",     "ELECTRIFY",20, 1450,   0,  0   },
    {"charm monster",       "CHARM",    26, 1500,   25, 15  },
    {"summon monster",      "SUMMON",   26, 1500,   25, 15  },
    {"gaze reflection",     "REFLECT",  25, 400,    25, 15  },
    {"summon familiar",     "SUMFAM",   0,  0,      0,  0   },
    {"fear",                "FEAR",     20, 200,    20, 10  },
    {"missile protection",  "MSHIELD",  20, 300,    20, 10  }
};

int maxscrolls = MAXSCROLLS;

struct magic_item p_magic[] =
{
    {"clear thought",       "CLEAR",    90, 380,    27, 15  },
    {"gain ability",        "GAINABIL", 40, 1250,   15, 15  },
    {"see invisible",       "SEE",      0,  0,      0,  0   },
    {"healing",             "HEAL",     120,330,    27, 27  },
    {"detect monster",      "MON",      0,  0,      0,  0   },
    {"detect magic",        "MAG",      0,  0,      0,  0   },
    {"raise level",         "RAISE",    1,  1900,   11, 10  },
    {"haste self",          "HASTE",    140,300,    30, 5   },
    {"restore abilities",   "RESTORE",  130,120,    0,  0   },
    {"phasing",             "PHASE",    45, 340,    21, 20  },
    {"invisibility",        "INVIS",    30, 300,    0,  15  },
    {"acute scent",         "SMELL",    30, 100,    20, 15  },
    {"acute hearing",       "HEAR",     30, 100,    20, 15  },
    {"super heroism",       "SUPER",    10, 800,    20, 15  },
    {"disguise",            "DISGUISE", 30, 500,    0,  15  },
    {"fire resistance",     "NOFIRE",   40, 350,    20, 15  },
    {"cold resistance",     "NOCOLD",   40, 300,    20, 15  },
    {"continuous breathing","BREATHE",  10, 200,    20, 15  },
    {"flying",              "FLY",      30, 300,    20, 15  },
    {"regeneration",        "REGEN",    20, 500,    20, 15  },
    {"shield",              "SHIELD",   100,200,    20, 10  },
    {"true sight",          "TRUESEE",  64, 570,    25, 15  }
};

int maxpotions = MAXPOTIONS;

struct magic_item r_magic[] =
{
    {"protection",              "", 70, 500,    33, 25  },
    {"add strength",            "", 65, 300,    33, 25  },
    {"sustain ability",         "", 40, 380,    10, 0   },
    {"searching",               "", 65, 250,    10, 0   },
    {"see invisible",           "", 30, 175,    10, 0   },
    {"alertness",               "", 40, 190,    10, 0   },
    {"aggravate monster",       "", 35, 100,    100,0   },
    {"dexterity",               "", 65, 220,    33, 25  },
    {"increase damage",         "", 65, 320,    33, 25  },
    {"regeneration",            "", 35, 860,    10, 0   },
    {"slow digestion",          "", 40, 340,    15, 10  },
    {"teleportation",           "", 35, 100,    100,0   },
    {"stealth",                 "", 50, 700,    10, 0   },
    {"add intelligence",        "", 60, 540,    33, 25  },
    {"increase wisdom",         "", 60, 540,    33, 25  },
    {"sustain health",          "", 60, 250,    10, 0   },
    {"vampiric regeneration",   "", 20, 900,    25, 10  },
    {"illumination",            "", 20, 300,    10, 0   },
    {"delusion",                "", 20, 100,    75, 0   },
    {"carrying",                "", 20, 400,    30, 30  },
    {"adornment",               "", 15, 10000,  10, 0   },
    {"levitation",              "", 20, 450,    30, 0   },
    {"fire resistance",         "", 10, 750,    10, 0   },
    {"cold resistance",         "", 10, 650,    10, 0   },
    {"lightning resistance",    "", 10, 750,    10, 0   },
    {"resurrection",            "", 1,  8000,   10, 0   },
    {"breathing",               "", 10, 250,    10, 0   },
    {"free action",             "", 10, 225,    10, 0   },
    {"wizardry",                "", 2,  1950,   10, 0   },
    {"piety",                   "", 2,  1950,   10, 0   },
    {"teleport control",        "", 5,  450,    10, 0   },
    {"true sight",              "", 10, 775,    10, 0   }
};

int maxrings = MAXRINGS;

struct magic_item  ws_magic[] =
{
    {"light",           "LIGHT",        90, 150,    20, 20  },
    {"striking",        "HIT",          58, 400,    0,  0   },
    {"lightning",       "BOLT",         25, 800,    0,  0   },
    {"fire",            "FIRE",         25, 600,    0,  0   },
    {"cold",            "COLD",         30, 600,    0,  0   },
    {"polymorph",       "POLY",         90, 210,    0,  0   },
    {"magic missile",   "MLE",          90, 500,    0,  0   },
    {"slow monster",    "SLOW",         76, 320,    25, 20  },
    {"drain life",      "DRAIN",        90, 310,    20, 0   },
    {"charging",        "CHARGE",       70, 1100,   0,  0   },
    {"teleport monster","RANDOM",       90, 240,    25, 20  },
    {"cancellation",    "CANCEL",       38, 230,    0,  0   },
    {"confuse monster", "CONFMON",      50, 200,    0,  0   },
    {"disintegration",  "KILL-O-ZAP",   10, 1550,   33, 0   },
    {"anti-matter",     "BLACKHOLE",    30, 980,    0,  0   },
    {"paralyze monster","PARAL",        38, 200,    0,  0   },
    {"heal monster",    "XENOHEAL",     30, 200,    40, 10  },
    {"nothing",         "NOTHING",      30, 100,    0,  0   },
    {"invisibility",    "WS_INVIS",     30, 150,    30, 5   },
    {"blasting",        "BLAST",        10, 220,    0,  0   },
    {"webbing",         "WEB",          0,  0,      0,  0   },
    {"door opening",    "KNOCK",        0,  0,      0,  0   },
    {"hold portal",     "CLOSE",        0,  0,      0,  0   }
};

int maxsticks = MAXSTICKS;

struct magic_item   fd_data[] =
{
    {"food ration", "RATION",   400,    20,     20, 20  },
    {"random fruit","FRUIT", 300, 10, 0, 0},
    {"cram",        "CRAM",     120,    30,     0,  0   },
    {"honey cake",  "CAKES",    80,     10,     0,  0   },
    {"lemba",       "LEMBA",    50,     80,     0,  0   },
    {"miruvor",     "MIRUVOR",  50,     200,    0,  0   }
};

int maxfoods = MAXFOODS;

/*
 * weapons and their attributes
 * Average Damage = (min_damage + max_damage) / 2)
 * AD of 2D5+3 = (5 + 13) / 2 = 9
 * AD of 3D6   = (3 + 18) / 2 = 10.5
 */

#define ISSHARPMETAL (ISSHARP | ISMETAL)
#define ISCRYSKNIFE (ISSHARP | ISPOISON | ISMANY | ISLITTLE)

struct init_weps weaps[] =
{
    /* Missile weapons */
    {"sling",           "0d0", "0d0",   NONE,       5,  1,
        ISLAUNCHER | ISLITTLE,                              },
    {"rock",            "1d2", "1d4",   SLING,      5,  1,
        ISMANY | ISMISL | ISLITTLE                          },
    {"sling bullet",    "1d1", "1d8",   SLING,      3,  1,
        ISSHARP | ISMANY | ISMISL | ISMETAL | ISLITTLE      },
    {"short bow",       "1d1", "1d1",   NONE,       40, 75,
        ISLAUNCHER                                          },
    {"arrow",           "1d1", "2d3",   BOW,        5,  1,
        ISSHARP | ISMANY | ISMISL | ISLITTLE                },
    {"arrow",           "1d2", "2d8",   BOW,        10, 5,
        ISSHARP | ISSILVER | ISMANY | ISMISL | ISLITTLE     },
    {"fire arrow",      "1d2", "2d8",   BOW,        10, 3,
        ISSHARP | CANBURN | ISMANY | ISMISL | ISLITTLE      },
    {"footbow",         "1d1", "1d1",   NONE,       90, 125,
        ISLAUNCHER                                          },
    {"footbow bolt",    "1d2", "1d10",  FOOTBOW,    5,  1,
        ISSHARP | ISMANY | ISMISL | ISLITTLE                },
    {"crossbow",        "1d1", "1d1",   NONE,       100,175,
        ISLAUNCHER                                          },
    {"crossbow bolt",   "1d2", "2d5",  CROSSBOW,   7,  3,
        ISSHARP | ISMANY | ISMISL | ISLITTLE                },

    /* Useful throwing weapons */
    {"dart",        "1d1", "1d3",       NONE,   5,  1,
        ISSHARP | ISMANY | ISMISL | ISLITTLE            },
    {"dagger",      "1d6", "1d4",       NONE,   10, 2,
        ISSHARP | ISMETAL | ISMANY | ISMISL | ISLITTLE  },
    {"hammer",      "1d3", "1d5",       NONE,   50, 3,
        ISMETAL | ISMISL                                },
    {"leuku",       "1d6", "1d5",       NONE,   40, 4,
        ISSHARP | ISMETAL | ISTWOH                      },
    {"javelin",     "1d4", "1d6",       NONE,   10, 5,
        ISSHARP | ISMISL | ISTWOH                       },
    {"tomahawk",    "1d6", "1d6",       NONE,   45, 7,
        ISSHARP | ISMISL                                },
    {"machete",     "1d7", "1d6",       NONE,   45, 4,
        ISSHARP | ISMETAL | ISMISL                      },
    {"throwing axe","1d3", "1d6+2",     NONE,   50, 8,
        ISSHARP | ISMETAL | ISMISL                      },
    {"spear",       "2d3", "1d6",       NONE,   50, 2,
        ISSHARP | ISMETAL | ISMISL                      },
    {"boomerang",   "1d1", "1d8",       NONE,   10, 13,
        CANRETURN | ISMANY | ISMISL | ISLITTLE          },
    {"long spear",  "1d8", "1d10",      NONE,   50, 20,
        ISSHARP | ISMETAL | ISMISL | ISTWOH             },
    {"shuriken",    "1d1", "2d5",       NONE,   4,  20,
        ISSHARP | ISMETAL | ISMANY | ISMISL | ISLITTLE  },
    {"burning oil", "0d0", "2d10+5",    NONE,   20, 30,
        CANBURN | ISMANY | ISMISL | ISLITTLE            },
    {"grenade",       "1d1", "1d2/4d8", NONE,   10, 50,
        ISMANY | ISSMALL                                },

    /* other weapons */
    {"club",          "1d4",   "1d2", NONE, 30,  2,   0                     },
    {"pitchfork",     "1d5",   "2d2", NONE, 15,  5,   ISSHARPMETAL          },
    {"short sword",   "1d6",   "1d2", NONE, 50,  10,  ISSHARPMETAL          },
    {"hand axe",      "1d6",   "1d2", NONE, 40,  15,  ISSHARPMETAL          },
    {"partisan",      "1d6",   "1d2", NONE, 75,  4,   ISSHARPMETAL | ISTWOH },
    {"grain flail",   "1d6",   "1d4", NONE, 100, 2,   ISSHARPMETAL          },
    {"singlestick",   "1d4+2", "1d2", NONE, 30,  20,  0                     },
    {"rapier",        "1d6+1", "1d2", NONE, 7,   75,  ISSHARPMETAL          },
    {"sickle",        "1d6+1", "1d2", NONE, 30,  15,  ISSHARPMETAL          },
    {"hatchet",       "1d6+1", "1d4", NONE, 50,  10,  ISSHARPMETAL          },
    {"scimitar",      "1d8",   "1d2", NONE, 40,  10,  ISSHARPMETAL          },
    {"mace",          "2d4",   "1d3", NONE, 100, 40,  0                     },
    {"morning star",  "2d4",   "1d3", NONE, 125, 35,  ISMETAL               },
    {"broad sword",   "2d4",   "1d3", NONE, 75,  50,  ISSHARPMETAL          },
    {"miner's pick",  "2d4",   "1d2", NONE, 85,  40,  ISSHARPMETAL          },
    {"guisarme",      "2d4",   "1d3", NONE, 100, 25,  ISSHARPMETAL | ISTWOH },
    {"war flail",     "1d6+2", "1d4", NONE, 150, 50,  ISSHARPMETAL | ISTWOH },
    {"crysknife",     "3d3",   "1d3", NONE, 12,  100, ISCRYSKNIFE           },
    {"battle axe",    "1d8+2", "1d3", NONE, 80,  100, ISSHARPMETAL          },
    {"cutlass",       "1d10",  "1d2", NONE, 55,  120, ISSHARPMETAL          },
    {"glaive",        "1d10",  "1d3", NONE, 80,  80,  ISSHARPMETAL | ISTWOH },
    {"pertuska",      "2d5",   "1d3", NONE, 130, 100, ISSHARPMETAL | ISTWOH },
    {"long sword",    "3d4",   "1d2", NONE, 100, 150, ISSHARPMETAL          },
    {"lance",         "1d12",  "1d8", NONE, 80,  140, ISSHARP | ISTWOH      },
    {"ranseur",       "1d12",  "1d8", NONE, 100, 130, ISSHARPMETAL | ISTWOH },
    {"sabre",         "2d6",   "1d3", NONE, 50,  200, ISSHARPMETAL          },
    {"spetum",        "2d6",   "1d3", NONE, 50,  180, ISSHARPMETAL | ISTWOH },
    {"halberd",       "2d6",   "1d3", NONE, 150, 125, ISSHARPMETAL | ISTWOH },
    {"trident",       "3d4",   "1d4", NONE, 50,  200, ISSHARPMETAL | ISTWOH },
    {"war pick",      "3d4",   "1d2", NONE, 75,  175, ISSHARPMETAL | ISTWOH },
    {"bardiche",      "3d4",   "1d2", NONE, 125, 125, ISSHARPMETAL | ISTWOH },
    {"heavy mace",    "3d4",   "1d3", NONE, 200, 50,  ISTWOH                },
    {"great scythe",  "2d6+2", "1d2", NONE, 100, 200, ISSHARP | ISTWOH      },
    {"quarter staff", "3d5",   "1d2", NONE, 70,  250, 0                     },
    {"bastard sword", "2d8",   "1d2", NONE, 150, 300, ISSHARPMETAL          },
    {"pike",          "2d8",   "2d6", NONE, 200, 275, ISSHARPMETAL | ISTWOH },
    {"great flail",   "2d6+2", "1d4", NONE, 200, 275, ISSHARPMETAL | ISTWOH },
    {"great maul",    "4d4",   "1d3", NONE, 400, 250, ISTWOH                },
    {"great pick",    "2d9",   "1d2", NONE, 175, 330, ISSHARPMETAL | ISTWOH },
    {"two handed sword","4d4", "1d2", NONE, 250, 300, ISSHARPMETAL | ISTWOH },
    {"claymore",      "3d7",   "1d2", NONE, 200, 500, ISSHARPMETAL | ISTWOH }
};

int maxweapons = MAXWEAPONS;

struct init_armor   armors[] =
{
    { "soft leather",         75, 20,  9,  50 },
    { "cuirboilli",          150, 30,  8, 130 },
    { "leather armor",       175, 40,  8, 100 },
    { "ring mail",           350, 49,  7, 250 },
    { "studded leather armor",400,58,  7, 200 },
    { "scale mail",          500, 66,  6, 250 },
    { "padded armor",        550, 72,  6, 150 },
    { "chain mail",          750, 78,  5, 300 },
    { "brigandine",          800, 84,  5, 280 },
    { "splint mail",        1000, 88,  4, 350 },
    { "banded mail",        1250, 90,  4, 300 },
    { "superior chain",     1500, 93,  3, 350 },
    { "plate mail",         1400, 96,  3, 400 },
    { "plate armor",        1650, 98,  2, 450 },
    { "mithril",           30000, 99,  2, 200 },
    { "crystalline armor", 15000, 100, 0, 300 }
};

int     maxarmors = MAXARMORS;

struct init_artifact    arts[] = {
    { "Magic Purse of Yendor", 15, 1, 1, 1,  1,  4600L, 50 },
    { "Phial of Galadriel",    20, 2, 2, 2,  1, 12500L, 10 },
    { "Amulet of Yendor",      25, 4, 1, 1,  2, 16000L, 10 },
    { "Palantir of Might",     30, 1, 4, 1,  2, 18500L, 70 },
    { "Crown of Might",        35, 6, 2, 1,  1, 23500L, 50 },
    { "Sceptre of Might",      40, 2, 2, 1,  6, 38000L, 50 },
    { "Silmaril of Ea",        45, 4, 2, 5,  1, 50000L, 50 },
    { "Wand of Yendor",        50, 4, 2, 3, 10, 80000L, 50 }
};

int     maxartifact = MAXARTIFACT;

/*
    init_player()
        roll up the rogue
*/

void
init_player(void)
{
    int special = rnd(100) < 20;
    struct linked_list  *item;
    struct object   *obj;
    int which_armor = 0, which_weapon = 0;
    int other_weapon_flags = 0;

    pstats.s_lvl = 1;
    pstats.s_exp = 0L;
    pstats.s_arm = 10;

    if (!geta_player())
    {
        do_getplayer(); /* get character class */
        pstats.s_str = 8 + rnd(5);
        pstats.s_intel = 8 + rnd(5);
        pstats.s_wisdom = 8 + rnd(5);
        pstats.s_dext = 8 + rnd(5);
        pstats.s_const = 10 + rnd(8);
        pstats.s_charisma = 8 + rnd(4);
        pstats.s_power = 5 + rnd(5);
        pstats.s_hpt = 15 + rnd(5);

        switch (char_type)      /* Class-specific abilities */
        {
            case C_FIGHTER:
                pstats.s_str = 17;
                pstats.s_const += rnd(4) + 1;
                if (special)
                {
                    pstats.s_str += rnd(3);
                    pstats.s_dext += rnd(4);
                }
                pstats.s_const = max(pstats.s_const, 16);
                break;

            case C_PALADIN:
                pstats.s_charisma = 17;

                if (special)
                {
                    pstats.s_charisma += rnd(3);
                    pstats.s_wisdom += rnd(4);
                    pstats.s_str += rnd(5);
                }

                pstats.s_str = max(pstats.s_str, 16);
                pstats.s_wisdom = max(pstats.s_wisdom, 16);
                break;

            case C_RANGER:
                if (special)
                {
                    pstats.s_wisdom += rnd(4);
                    pstats.s_intel += rnd(4);
                    pstats.s_str += rnd(5);
                }

                pstats.s_str = max(pstats.s_str, 16);
                pstats.s_wisdom = max(pstats.s_wisdom, 16);
                pstats.s_const = max(pstats.s_const, 14);
                break;

            case C_MAGICIAN:
                pstats.s_intel = (special) ? 18 : 16;
                pstats.s_power += 5 + rnd(10);
                break;

            case C_ILLUSION:
                pstats.s_intel = (special) ? 18 : 16;
                pstats.s_dext = (special) ? 18 : 14;
                pstats.s_power += 5 + rnd(10);
                break;

            case C_CLERIC:
                pstats.s_wisdom = (special) ? 18 : 16;
                pstats.s_str += rnd(4);
                pstats.s_power += 5 + rnd(10);
                break;

            case C_DRUID:
                if (special)
                {
                    pstats.s_wisdom += rnd(5);
                    pstats.s_charisma += rnd(4);
                }
                pstats.s_str += rnd(4);
                pstats.s_power += 5 + rnd(10);
                pstats.s_wisdom = max(pstats.s_wisdom, 16);
                break;

            case C_THIEF:
                pstats.s_dext = 18;
                if (special)
                    pstats.s_const += rnd(4) + 1;
                break;

            case C_ASSASIN:
                pstats.s_dext = (special) ? 18 : 16;
                pstats.s_intel = (special) ? 18 : 16;
                break;

            case C_NINJA:
                if (special)
                {
                    pstats.s_dext += rnd(5);
                    pstats.s_str += rnd(4);
                    pstats.s_intel += rnd(3);
                    pstats.s_wisdom += rnd(3);
                }
                pstats.s_dext = max(pstats.s_dext, 16);
                pstats.s_str = max(pstats.s_str, 15);
                pstats.s_wisdom = max(pstats.s_wisdom, 15);
                pstats.s_const = max(pstats.s_const, 15);
                pstats.s_charisma = max(pstats.s_charisma, 11);
        }

        puta_player();
    }

    if (char_type == C_ASSASIN || char_type == C_NINJA
        || char_type == C_FIGHTER)
        pstats.s_dmg = "2d6";
    else
        pstats.s_dmg = "1d4";

    if (char_type == C_NINJA || char_type == C_FIGHTER)
        pstats.s_arm = 8;

    if (pstats.s_const > 15)
        pstats.s_hpt += pstats.s_const - 15;

    max_stats = pstats;
    player.t_rest_hpt = player.t_rest_pow = 0;
    player.t_praycnt = 0;
    pstats.s_carry = totalenc();
    pack = NULL;

    switch (player.t_ctype)    /* now outfit pack */
    {
        case C_PALADIN:
            purse        = roll(20, 60);
            which_armor  = CHAIN_MAIL;
            which_weapon = LONG_SWORD;
            break;

        case C_FIGHTER:
            purse = roll(50, 60);
            which_armor = SCALE_MAIL;
            which_weapon = BROAD_SWORD;
            break;

        case C_RANGER:
            purse        = roll(50, 60);
            which_armor  = PADDED_ARMOR;
            which_weapon = LONG_SPEAR;
            other_weapon_flags |= ISOWNED | CANRETURN;
            break;

        case C_CLERIC:
            purse        = roll(30, 80);
            which_armor  = RING_MAIL;
            which_weapon = MORNINGSTAR;
            break;

        case C_DRUID:
            purse        = roll(30, 80);
            which_armor  = STUDDED_LEATHER;
            which_weapon = LIGHT_MACE;
            break;

        case C_THIEF:
            purse        = roll(40, 80);
            which_armor  = HEAVY_LEATHER;
            which_weapon = CUTLASS;
            break;

        case C_ASSASIN:
            purse        = roll(20, 80);
            which_armor  = CUIRBOLILLI;
            which_weapon = SABRE;
            other_weapon_flags |= ISPOISON;
            break;

        case C_NINJA:
            purse        = roll(20, 80);
            which_armor  = CUIRBOLILLI;
            which_weapon = CRYSKNIFE;
            other_weapon_flags |= ISPOISON;
            item = spec_item(WEAPON, SHURIKEN, 1, 1);
            obj = OBJPTR(item);
            obj->o_count = 1;
            obj->o_flags |= ISKNOW | ISPOISON | ISOWNED | CANRETURN;
            add_pack(item, NOMESSAGE);
            break;

        case C_MAGICIAN:
        case C_ILLUSION:
            purse        = roll(20, 60);
            which_armor  = SOFT_LEATHER;
            which_weapon = SINGLESTICK;
            break;

        default:
            break;
    }

    /* Add weapon to pack */

    item = spec_item(WEAPON, which_weapon, 1, 1);
    obj  = OBJPTR(item);

    obj->o_flags |= ISKNOW;
    obj->o_flags |= other_weapon_flags;
    obj->o_count = 1;
    add_pack(item, NOMESSAGE);
    cur_weapon = obj;

    /* Add armor to pack */

    item = spec_item(ARMOR, which_armor, 0, 0);
    obj = OBJPTR(item);

    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[which_armor].a_wght;
    add_pack(item, NOMESSAGE);
    cur_armor = obj;

    /* Add some food to pack */

    item = spec_item(FOOD, FD_CRAM, 0, 0);
    obj = OBJPTR(item);

    obj->o_weight = things[TYP_FOOD].mi_wght;
    obj->o_count = 3;
    add_pack(item, NOMESSAGE);
}

/*
    init_flags()
        Initialize flags on startup
*/

void
init_flags(void)
{
    switch (player.t_ctype)
    {
        case C_MAGICIAN:
        case C_ILLUSION:
        case C_CLERIC:
        case C_DRUID:
        case C_RANGER:
        case C_PALADIN:
            turn_on(player, CANSUMMON);
            break;

        default:
            break;
    }

    turn_on(player, CANCAST);
    turn_on(player, CANWIELD);
}

/*
 * Contains definitions and functions for dealing with things like potions
 * and scrolls
 */

/*
    init_things()
        Initialize the probabilities for types of things
*/

void
init_things(void)
{
    struct magic_item *mp;

    for (mp = &things[1]; mp < &things[numthings]; mp++)
        mp->mi_prob += (mp - 1)->mi_prob;

    badcheck("things", things, numthings);
}

/*
    init_fd()
        Initialize the probabilities for types of food
*/

void
init_fd(void)
{
    struct magic_item  *mp;

    for (mp = &fd_data[1]; mp < &fd_data[maxfoods]; mp++)
        mp->mi_prob += (mp - 1)->mi_prob;

    badcheck("food", fd_data, maxfoods);
}

/*
    init_colors()
        Initialize the potion color scheme for this time
*/

void
init_colors(void)
{
    int   i;
    char *str;

    for (i = 0; i < maxpotions; i++)
    {
        do
            str = rainbow[rnd(NCOLORS)];
        while( !isupper(*str) );

        p_colors[i]    = md_strdup(str);
        p_colors[i][0] = (char) tolower(p_colors[i][0]);

        know_items[TYP_POTION][i]  = FALSE;
        guess_items[TYP_POTION][i] = NULL;

        if (i > 0)
            p_magic[i].mi_prob += p_magic[i - 1].mi_prob;
    }

    badcheck("potions", p_magic, maxpotions);
}

/*
    init_names()
        Generate the names of the various scrolls
*/

void
init_names(void)
{
    int    nsyl;
    char   *cp, *sp;
    int    i, nwords;

    for (i = 0; i < maxscrolls; i++)
    {
        cp = prbuf;
        nwords = rnd(COLS / 20) + 1 + (COLS > 40 ? 1 : 0);

        while (nwords--)
        {
            nsyl = rnd(3) + 1;

            while (nsyl--)
            {
                sp = sylls[rnd((sizeof sylls) /
                    (sizeof(char *)))];

                while (*sp)
                    *cp++ = *sp++;
            }
            *cp++ = ' ';
        }

        *--cp = '\0';
        s_names[i] = (char *) new_alloc(strlen(prbuf) + 1);

        know_items[TYP_SCROLL][i] = FALSE;
        guess_items[TYP_SCROLL][i] = 0;

        strcpy(s_names[i], prbuf);

        if (i > 0)
            s_magic[i].mi_prob += s_magic[i - 1].mi_prob;
    }

    badcheck("scrolls", s_magic, maxscrolls);
}

/*
    init_stones()
        Initialize the ring stone setting scheme for this time
*/

void
init_stones(void)
{
    int   i;
    char *str;

    for (i = 0; i < maxrings; i++)
    {
        do
            str = stones[rnd(NSTONES)];
        while(!isupper(*str));

        r_stones[i]    = md_strdup(str);
        r_stones[i][0] = (char) tolower( r_stones[i][0] );

        know_items[TYP_RING][i]  = FALSE;
        guess_items[TYP_RING][i] = NULL;

        if (i > 0)
            r_magic[i].mi_prob += r_magic[i - 1].mi_prob;
    }

    badcheck("rings", r_magic, maxrings);
}

/*
    init_materials()
        Initialize the construction materials for wands and staffs
*/

void
init_materials(void)
{
    int   i;
    char *str;

    for (i = 0; i < maxsticks; i++)
    {
        do
        {
            if (rnd(100) > 50)
            {
                str = metal[rnd(NMETAL)];

                if (isupper(*str))
                    ws_type[i] = "wand";
            }
            else
            {
                str = wood[rnd(NWOOD)];

                if (isupper(*str))
                    ws_type[i] = "staff";
            }
        }
        while(!isupper(*str));

        ws_made[i] = md_strdup(str);
        ws_made[i][0] = (char) tolower( ws_made[i][0] );

        know_items[TYP_STICK][i]  = FALSE;
        guess_items[TYP_STICK][i] = 0;

        if (i > 0)
            ws_magic[i].mi_prob += ws_magic[i - 1].mi_prob;
    }

    badcheck("sticks", ws_magic, maxsticks);
}

void
badcheck(char *name, struct magic_item *magic, int bound)
{
    struct magic_item  *end;

    if (magic[bound - 1].mi_prob == 1000)
        return;

    printf("\nBad percentages for %s:\n", name);

    for (end = &magic[bound]; magic < end; magic++)
        printf("%3d%% %s\n", magic->mi_prob, magic->mi_name);

    printf("[Hit RETURN to continue]");
    fflush(stdout);

    while ((readchar() & 0177) != '\n')
        continue;
}
