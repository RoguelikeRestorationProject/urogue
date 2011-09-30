/*
    rogue.h - A very large header file
 
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1984, 1991, 1997 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    Based on "Rogue: Exploring the Dungeons of Doom"
    Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <signal.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef LINT
#include <curses.h>
#else
#include "lint-curses.h"
#endif

#define SHOTPENALTY 2       /* In line of sight of missile */
#define DOORPENALTY 1       /* Moving out of current room */

/* Maximum number of different things */
#define MAXROOMS    9   /* max rooms per normal level */
#define MAXDOORS    4   /* max doors to a room */
#define MAXOBJ      6   /* max number of items to find on a level */
#define MAXTREAS    30  /* max number monsters/treasure in treasure room */
#define MAXTRAPS    80  /* max traps per level */
#define MAXTRPTRY   16  /* max attempts/level allowed for setting traps */
#define MAXPURCH    8   /* max purchases per trading post visit */
#define NUMMONST    (sizeof(monsters) / sizeof(struct monster) - 2)
#define NUMSUMMON   48  /* number of creatures that can summon hero */
#define NLEVMONS    8   /* number of new monsters per level */
#define LINELEN     512 /* characters in a buffer */

/* The character types */
#define C_FIGHTER    0
#define C_PALADIN    1
#define C_RANGER     2
#define C_CLERIC     3
#define C_DRUID      4
#define C_MAGICIAN   5
#define C_ILLUSION   6
#define C_THIEF      7
#define C_ASSASIN    8
#define C_NINJA      9
#define C_MONSTER   10
#define C_NOTSET    11  /* Must not be a value from above */

/* used for ring stuff */
#define LEFT_1  0
#define LEFT_2  1
#define LEFT_3  2
#define LEFT_4  3
#define LEFT_5  4
#define RIGHT_1 5
#define RIGHT_2 6
#define RIGHT_3 7
#define RIGHT_4 8
#define RIGHT_5 9

/* All the fun defines */
#define next(ptr) ((ptr)?(ptr->l_next):NULL)
#define prev(ptr) ((ptr)?(ptr->l_prev):NULL)
#define identifier(ptr) (ptr->o_ident)
#define winat(y,x) ( (char) \
 ((mvwinch(mw,y,x) == ' ') ? mvwinch(stdscr,y,x) : winch(mw))  )

#define debug if (wizard && wiz_verbose) msg
#define verify(b) if (b) verify_function(__FILE__, __LINE__);
#define DISTANCE(c1, c2)  ( ((c2).x - (c1).x)*((c2).x - (c1).x) + \
                            ((c2).y - (c1).y)*((c2).y - (c1).y) )

#define xyDISTANCE(y1, x1, y2, x2) ((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1))
#define OBJPTR(what)    ((*what).data.obj)
#define THINGPTR(what)  ((*what).data.th)
#define ce(a, b) ((a).x == (b).x && (a).y == (b).y)
#define hero player.t_pos
#define pstats player.t_stats
#define max_stats player.maxstats
#define pack player.t_pack
#define attach(a, b) _attach(&a, b)
#define detach(a, b) _detach(&a, b)
#define free_list(a) _free_list(&a)
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#define GOLDCALC (rnd(50 + 30 * level) + 2)
#define o_charges o_ac
#define mi_wght mi_worth

/* Things that appear on the screens */
#define WALL        ' '
#define PASSAGE     '#'
#define DOOR        '+'
#define FLOOR       '.'
#define VPLAYER     '@'
#define IPLAYER     '_'
#define POST        '^'
#define LAIR        '('
#define RUSTTRAP    ';'
#define TRAPDOOR    '>'
#define ARROWTRAP   '{'
#define SLEEPTRAP   '$'
#define BEARTRAP    '}'
#define TELTRAP     '~'
#define DARTTRAP    '`'
#define POOL        '"'
#define MAZETRAP    '\\'
#define FIRETRAP    '<'
#define POISONTRAP  '['
#define ARTIFACT    ','
#define SECRETDOOR  '&'
#define STAIRS      '%'
#define GOLD        '*'
#define POTION      '!'
#define SCROLL      '?'
#define MAGIC       '$'
#define BMAGIC      '>' /* Blessed magic */
#define CMAGIC      '<' /* Cursed  magic */
#define FOOD        ':'
#define WEAPON      ')'
#define ARMOR       ']'
#define RING        '='
#define STICK       '/'

/* Various constants */
#define HOLDTIME      2
#define BEARTIME      3
#define SLEEPTIME     4
#define FREEZETIME    6
#define STINKTIME     6
#define CHILLTIME     (roll(2, 4))
#define STONETIME     8
#define SICKTIME     10
#define CLRDURATION  15
#define HUHDURATION  20
#define SMELLTIME    20
#define HEROTIME     20
#define HEALTIME     30
#define WANDERTIME   140
#define GONETIME    200
#define PHASEDURATION   300
#define SEEDURATION 850

#define STPOS         0
#define BEFORE        1
#define AFTER         2

#define MORETIME     150
#define HUNGERTIME  1300
#define STOMACHSIZE 2000

#define BOLT_LENGTH 10
#define MARKLEN     20

#define LINEFEED    10
#define CARRIAGE_RETURN 13
#define ESCAPE      27

/* Adjustments for save against things */
#define VS_POISON       0
#define VS_PARALYZATION     0
#define VS_DEATH        0
#define VS_PETRIFICATION    1
#define VS_WAND         2
#define VS_BREATH       3
#define VS_MAGIC        4

/*attributes for treasures in dungeon */
#define ISNORMAL    0x00000000UL /* Neither blessed nor cursed */
#define ISCURSED    0x00000001UL /* cursed */
#define ISKNOW      0x00000002UL /* has been identified */
#define ISPOST      0x00000004UL /* object is in a trading post */
#define ISMETAL     0x00000008UL /* is metallic */
#define ISPROT      0x00000010UL /* object is protected */
#define ISBLESSED   0x00000020UL /* blessed */
#define ISZAPPED    0x00000040UL /* weapon has been charged by dragon */
#define ISVORPED    0x00000080UL /* vorpalized weapon */
#define ISSILVER    0x00000100UL /* silver weapon */
#define ISPOISON    0x00000200UL /* poisoned weapon */
#define CANRETURN   0x00000400UL /* weapon returns if misses */
#define ISOWNED     0x00000800UL /* weapon returns always */
#define ISLOST      0x00001000UL /* weapon always disappears */
#define ISMISL      0x00002000UL /* missile weapon */
#define ISMANY      0x00004000UL /* show up in a group */
#define CANBURN     0x00008000UL /* burns monsters */
#define ISSHARP     0x00010000UL /* cutting edge */
#define ISTWOH      0x00020000UL /* needs two hands to wield */
#define ISLITTLE    0x00040000UL /* small weapon */
#define ISLAUNCHER  0x00080000UL /* used to throw other weapons */
#define TYP_MAGIC_MASK  0x0f000000UL
#define POT_MAGIC   0x01000000UL
#define SCR_MAGIC   0x02000000UL
#define ZAP_MAGIC   0x04000000UL
#define SP_WIZARD   0x10000000UL /* only wizards */
#define SP_ILLUSION 0x20000000UL /* only illusionists */
#define SP_CLERIC   0x40000000UL /* only clerics/paladins */
#define SP_DRUID    0x80000000UL /* only druids/rangers */
#define SP_MAGIC    0x30000000UL /* wizard or illusionist */
#define SP_PRAYER   0xc0000000UL /* cleric or druid */
#define SP_ALL      0xf0000000UL /* all special classes */
#define _TWO_       ISBLESSED   /* more powerful spell */

/* Various flag bits */
#define ISDARK      0x00000001UL
#define ISGONE      0x00000002UL
#define ISTREAS     0x00000004UL
#define ISFOUND     0x00000008UL
#define ISTHIEFSET  0x00000010UL
#define WASDARK     0x00000020UL

/* struct thing t_flags (might include player) for monster attributes */
#define ISBLIND     0x00000001UL
#define ISINWALL    0x00000002UL
#define ISRUN       0x00000004UL
#define ISFLEE      0x00000008UL
#define ISINVIS     0x00000010UL
#define ISMEAN      0x00000020UL
#define ISGREED     0x00000040UL
#define CANSHOOT    0x00000080UL
#define ISHELD      0x00000100UL
#define ISHUH       0x00000200UL
#define ISREGEN     0x00000400UL
#define CANHUH      0x00000800UL
#define CANSEE      0x00001000UL
#define HASFIRE     0x00002000UL
#define ISSLOW      0x00004000UL
#define ISHASTE     0x00008000UL
#define ISCLEAR     0x00010000UL
#define CANINWALL   0x00020000UL
#define ISDISGUISE  0x00040000UL
#define CANBLINK    0x00080000UL
#define CANSNORE    0x00100000UL
#define HALFDAMAGE  0x00200000UL
#define CANSUCK     0x00400000UL
#define CANRUST     0x00800000UL
#define CANPOISON   0x01000000UL
#define CANDRAIN    0x02000000UL
#define ISUNIQUE    0x04000000UL
#define STEALGOLD   0x08000000UL
#define STEALMAGIC  0x10000001UL
#define CANDISEASE  0x10000002UL
#define HASDISEASE  0x10000004UL
#define CANSUFFOCATE    0x10000008UL
#define DIDSUFFOCATE    0x10000010UL
#define BOLTDIVIDE  0x10000020UL
#define BLOWDIVIDE  0x10000040UL
#define NOCOLD      0x10000080UL
#define TOUCHFEAR   0x10000100UL
#define BMAGICHIT   0x10000200UL
#define NOFIRE      0x10000400UL
#define NOBOLT      0x10000800UL
#define CARRYGOLD   0x10001000UL
#define CANITCH     0x10002000UL
#define HASITCH     0x10004000UL
#define DIDDRAIN    0x10008000UL
#define WASTURNED   0x10010000UL
#define CANSELL     0x10020000UL
#define CANBLIND    0x10040000UL
#define CANBBURN    0x10080000UL
#define ISCHARMED   0x10100000UL
#define CANSPEAK    0x10200000UL
#define CANFLY      0x10400000UL
#define ISFRIENDLY  0x10800000UL
#define CANHEAR     0x11000000UL
#define ISDEAF      0x12000000UL
#define CANSCENT    0x14000000UL
#define ISUNSMELL   0x18000000UL
#define WILLRUST    0x20000001UL
#define WILLROT     0x20000002UL
#define SUPEREAT    0x20000004UL
#define PERMBLIND   0x20000008UL
#define MAGICHIT    0x20000010UL
#define CANINFEST   0x20000020UL
#define HASINFEST   0x20000040UL
#define NOMOVE      0x20000080UL
#define CANSHRIEK   0x20000100UL
#define CANDRAW     0x20000200UL
#define CANSMELL    0x20000400UL
#define CANPARALYZE 0x20000800UL
#define CANROT      0x20001000UL
#define ISSCAVENGE  0x20002000UL
#define DOROT       0x20004000UL
#define CANSTINK    0x20008000UL
#define HASSTINK    0x20010000UL
#define ISSHADOW    0x20020000UL
#define CANCHILL    0x20040000UL
#define CANHUG      0x20080000UL
#define CANSURPRISE 0x20100000UL
#define CANFRIGHTEN 0x20200000UL
#define CANSUMMON   0x20400000UL
#define TOUCHSTONE  0x20800000UL
#define LOOKSTONE   0x21000000UL
#define CANHOLD     0x22000000UL
#define DIDHOLD     0x24000000UL
#define DOUBLEDRAIN 0x28000000UL
#define ISUNDEAD    0x30000001UL
#define BLESSMAP    0x30000002UL
#define BLESSGOLD   0x30000004UL
#define BLESSMONS   0x30000008UL
#define BLESSMAGIC  0x30000010UL
#define BLESSFOOD   0x30000020UL
#define CANBRANDOM  0x30000040UL /* Types of breath */
#define CANBACID    0x30000080UL
#define CANBFIRE    0x30000100UL
#define CANBBOLT    0x30000200UL
#define CANBGAS     0x30000400UL
#define CANBICE     0x30000800UL
#define CANBPGAS    0x30001000UL /* Paralyze gas */
#define CANBSGAS    0x30002000UL /* Sleeping gas */
#define CANBSLGAS   0x30004000UL /* Slow gas */
#define CANBFGAS    0x30008000UL /* Fear gas */
#define CANBREATHE  0x3000ffc0UL /* Can it breathe at all? */
#define STUMBLER    0x30010000UL
#define POWEREAT    0x30020000UL
#define ISELECTRIC  0x30040000UL
#define HASOXYGEN   0x30080000UL /* Doesn't need to breath air */
#define POWERDEXT   0x30100000UL
#define POWERSTR    0x30200000UL
#define POWERWISDOM 0x30400000UL
#define POWERINTEL  0x30800000UL
#define POWERCONST  0x31000000UL
#define SUPERHERO   0x32000000UL
#define ISUNHERO    0x34000000UL
#define CANCAST     0x38000000UL
#define CANTRAMPLE  0x40000001UL
#define CANSWIM     0x40000002UL
#define LOOKSLOW    0x40000004UL
#define CANWIELD    0x40000008UL
#define CANDARKEN   0x40000010UL
#define ISFAST      0x40000020UL
#define CANBARGAIN  0x40000040UL
#define NOMETAL     0x40000080UL
#define CANSPORE    0x40000100UL
#define NOSHARP     0x40000200UL
#define DRAINWISDOM 0x40000400UL
#define DRAINBRAIN  0x40000800UL
#define ISLARGE     0x40001000UL
#define ISSMALL     0x40002000UL
#define CANSTAB     0x40004000UL
#define ISFLOCK     0x40008000UL
#define ISSWARM     0x40010000UL
#define CANSTICK    0x40020000UL
#define CANTANGLE   0x40040000UL
#define DRAINMAGIC  0x40080000UL
#define SHOOTNEEDLE 0x40100000UL
#define CANZAP      0x40200000UL
#define HASARMOR    0x40400000UL
#define CANTELEPORT 0x40800000UL
#define ISBERSERK   0x41000000UL
#define ISFAMILIAR  0x42000000UL
#define HASFAMILIAR 0x44000000UL
#define SUMMONING   0x48000000UL
#define CANREFLECT  0x50000001UL
#define LOWFRIENDLY 0x50000002UL
#define MEDFRIENDLY 0x50000004UL
#define HIGHFRIENDLY    0x50000008UL
#define MAGICATTRACT    0x50000010UL
#define ISGOD       0x50000020UL
#define CANLIGHT    0x50000040UL
#define HASSHIELD   0x50000080UL
#define HASMSHIELD  0x50000100UL
#define LOWCAST     0x50000200UL
#define MEDCAST     0x50000400UL
#define HIGHCAST    0x50000800UL
#define WASSUMMONED 0x50001000UL
#define HASSUMMONED 0x50002000UL
#define CANTRUESEE  0x50004000UL


#define FLAGSHIFT       28UL
#define FLAGINDEX       0x0000000fL
#define FLAGMASK        0x0fffffffL

/* on - check if a monster flag is on */
#define on(th, flag) \
        ((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag & FLAGMASK))

/* off - check if a monster flag is off */
#define off(th, flag) \
        (!((th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] & (flag & FLAGMASK)))

/* turn_on - turn on a monster flag */
#define turn_on(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] |= (flag & FLAGMASK))

/* turn_off - turn off a monster flag */
#define turn_off(th, flag) \
        ( (th).t_flags[(flag >> FLAGSHIFT) & FLAGINDEX] &= ~(flag & FLAGMASK))

#define SAME_POS(c1,c2) ( (c1.x == c2.x) && (c1.y == c2.y) )

/* types of things */
/* All magic spells duplicate a potion, scroll, or stick effect */

#define TYP_POTION  0
#define TYP_SCROLL  1
#define TYP_RING    2
#define TYP_STICK   3
#define MAXMAGICTYPES   4   /* max number of items in magic class */
#define MAXMAGICITEMS   50  /* max number of items in magic class */
#define TYP_FOOD    4
#define TYP_WEAPON  5
#define TYP_ARMOR   6
#define TYP_ARTIFACT    7
#define NUMTHINGS   (sizeof(things) / sizeof(struct magic_item))

/* Artifact types */
#define TR_PURSE    0
#define TR_PHIAL    1
#define TR_AMULET   2
#define TR_PALANTIR 3
#define TR_CROWN    4
#define TR_SCEPTRE  5
#define TR_SILMARIL 6
#define TR_WAND     7
#define MAXARTIFACT (sizeof(arts) / sizeof(struct init_artifact))

/* Artifact flags */
#define ISUSED      01
#define ISACTIVE    02

/* Potion types - add also to magic_item.c and potions.c */
#define P_CLEAR     0
#define P_GAINABIL  1
#define P_SEEINVIS  2
#define P_HEALING   3
#define P_MONSTDET  4
#define P_TREASDET  5
#define P_RAISELEVEL    6
#define P_HASTE     7
#define P_RESTORE   8
#define P_PHASE     9
#define P_INVIS     10
#define P_SMELL     11
#define P_HEAR      12
#define P_SHERO     13
#define P_DISGUISE  14
#define P_FIRERESIST    15
#define P_COLDRESIST    16
#define P_HASOXYGEN 17
#define P_LEVITATION    18
#define P_REGENERATE    19
#define P_SHIELD    20
#define P_TRUESEE   21
#define MAXPOTIONS  22

/* Scroll types - add also to magic_item.c and scrolls.c */
#define S_CONFUSE   0
#define S_MAP       1
#define S_LIGHT     2
#define S_HOLD      3
#define S_SLEEP     4
#define S_ENCHANT   5
#define S_IDENTIFY  6
#define S_SCARE     7
#define S_GFIND     8
#define S_SELFTELEP 9
#define S_CREATE    10
#define S_REMOVECURSE   11
#define S_PETRIFY   12
#define S_GENOCIDE  13
#define S_CURING    14
#define S_MAKEITEMEM    15
#define S_PROTECT   16
#define S_NOTHING   17
#define S_SILVER    18
#define S_OWNERSHIP 19
#define S_FOODDET   20
#define S_ELECTRIFY 21
#define S_CHARM     22
#define S_SUMMON    23
#define S_REFLECT   24
#define S_SUMFAMILIAR   25
#define S_FEAR      26
#define S_MSHIELD   27
#define MAXSCROLLS  28

/* Rod/Wand/Staff types - add also to magic_item.c and sticks.c */
#define WS_LIGHT    0
#define WS_HIT      1
#define WS_ELECT    2
#define WS_FIRE     3
#define WS_COLD     4
#define WS_POLYMORPH    5
#define WS_MISSILE  6
#define WS_SLOW_M   7
#define WS_DRAIN    8
#define WS_CHARGE   9
#define WS_MONSTELEP    10
#define WS_CANCEL   11
#define WS_CONFMON  12
#define WS_DISINTEGRATE 13
#define WS_ANTIMATTER   14
#define WS_PARALYZE 15
#define WS_XENOHEALING  16
#define WS_NOTHING  17
#define WS_INVIS    18
#define WS_BLAST    19
#define WS_WEB      20
#define WS_KNOCK    21
#define WS_CLOSE    22
#define MAXSTICKS   23

/* Ring types */
#define R_PROTECT   0
#define R_ADDSTR    1
#define R_SUSABILITY    2
#define R_SEARCH    3
#define R_SEEINVIS  4
#define R_ALERT     5
#define R_AGGR      6
#define R_ADDHIT    7
#define R_ADDDAM    8
#define R_REGEN     9
#define R_DIGEST    10
#define R_TELEPORT  11
#define R_STEALTH   12
#define R_ADDINTEL  13
#define R_ADDWISDOM 14
#define R_HEALTH    15
#define R_VREGEN    16
#define R_LIGHT     17
#define R_DELUSION  18
#define R_CARRYING  19
#define R_ADORNMENT 20
#define R_LEVITATION    21
#define R_FIRERESIST    22
#define R_COLDRESIST    23
#define R_ELECTRESIST   24
#define R_RESURRECT 25
#define R_BREATHE   26
#define R_FREEDOM   27
#define R_WIZARD    28
#define R_PIETY     29
#define R_TELCONTROL    30
#define R_TRUESEE   31
#define MAXRINGS    32

/* Weapon types */
#define SLING       0   /* sling */
#define ROCK        1   /* rocks */
#define BULLET      2   /* sling bullet */
#define BOW     3   /* short bow */
#define ARROW       4   /* arrow */
#define SILVERARROW 5   /* silver arrows */
#define FLAMEARROW  6   /* flaming arrows */
#define FOOTBOW     7   /* footbow */
#define FBBOLT      8   /* footbow bolt */
#define CROSSBOW    9   /* crossbow */
#define BOLT        10  /* crossbow bolt */

#define DART        11  /* darts */
#define DAGGER      12  /* dagger */
#define HAMMER      13  /* hammer */
#define LEUKU       14  /* leuku */
#define JAVELIN     15  /* javelin */
#define TOMAHAWK    16  /* tomahawk */
#define MACHETE     17  /* machete */
#define THROW_AXE   18  /* throwing axe */
#define SHORT_SPEAR 19  /* spear */
#define BOOMERANG   20  /* boomerangs */
#define LONG_SPEAR  21  /* spear */
#define SHURIKEN    22  /* shurikens */
#define MOLOTOV     23  /* molotov cocktails */
#define GRENADE     24  /* grenade for explosions */
#define CLUB        25  /* club */
#define PITCHFORK   26  /* pitchfork */
#define SHORT_SWORD 27  /* short sword */
#define HAND_AXE    28  /* hand axe */
#define PARTISAN    29  /* partisan */
#define GRAIN_FLAIL 30  /* grain flail */
#define SINGLESTICK 31  /* singlestick */
#define RAPIER      32  /* rapier */
#define SICKLE      33  /* sickle */
#define HATCHET     34  /* hatchet */
#define SCIMITAR    35  /* scimitar */
#define LIGHT_MACE  36  /* mace */
#define MORNINGSTAR 37  /* morning star */
#define BROAD_SWORD 38  /* broad sword */
#define MINER_PICK  39  /* miner's pick */
#define GUISARME    40  /* guisarme */
#define WAR_FLAIL   41  /* war flail */
#define CRYSKNIFE   42  /* crysknife */
#define BATTLE_AXE  43  /* battle axe */
#define CUTLASS     44  /* cutlass sword */
#define GLAIVE      45  /* glaive */
#define PERTUSKA    46  /* pertuska */
#define LONG_SWORD  47  /* long sword */
#define LANCE       48  /* lance */
#define RANSEUR     49  /* ranseur */
#define SABRE       50  /* sabre */
#define SPETUM      51  /* spetum */
#define HALBERD     52  /* halberd */
#define TRIDENT     53  /* trident */
#define WAR_PICK    54  /* war pick */
#define BARDICHE    55  /* bardiche */
#define HEAVY_MACE  56  /* mace */
#define SCYTHE      57  /* great scythe */
#define QUARTERSTAFF    58  /* quarter staff */
#define BAST_SWORD  59  /* bastard sword */
#define PIKE        60  /* pike */
#define TWO_FLAIL   61  /* two-handed flail */
#define TWO_MAUL    62  /* two-handed maul */
#define TWO_PICK    63  /* two-handed pick */
#define TWO_SWORD   64  /* two-handed sword */
#define CLAYMORE    65  /* claymore sword */
#define MAXWEAPONS  (sizeof(weaps) / sizeof(struct init_weps))
#define NONE        100 /* no weapon */

/* Armor types */
#define SOFT_LEATHER    0
#define CUIRBOLILLI 1
#define HEAVY_LEATHER   2
#define RING_MAIL   3
#define STUDDED_LEATHER 4
#define SCALE_MAIL  5
#define PADDED_ARMOR    6
#define CHAIN_MAIL  7
#define BRIGANDINE  8
#define SPLINT_MAIL 9
#define BANDED_MAIL 10
#define GOOD_CHAIN  11
#define PLATE_MAIL  12
#define PLATE_ARMOR 13
#define MITHRIL     14
#define CRYSTAL_ARMOR   15
#define MAXARMORS   (sizeof(armors) / sizeof(struct init_armor))

/* Food types */
#define FD_RATION   0
#define FD_FRUIT    1
#define FD_CRAM     2
#define FD_CAKES    3
#define FD_LEMBA    4
#define FD_MIRUVOR  5
#define MAXFOODS    (sizeof(fd_data) / sizeof(struct magic_item))

/* stuff to do with encumberance */
#define F_OK        0   /* have plenty of food in stomach */
#define F_HUNGRY    1   /* player is hungry */
#define F_WEAK      2   /* weak from lack of food */
#define F_FAINT     3   /* fainting from lack of food */

/* return values for get functions */
#define NORM    0       /* normal exit */
#define QUIT    1       /* quit option setting */
#define MINUS   2       /* back up one option */

/* These are the types of inventory styles. */
#define INV_SLOW    0
#define INV_OVER    1
#define INV_CLEAR   2

/* These will eventually become enumerations */
#define MESSAGE     TRUE
#define NOMESSAGE   FALSE
#define POINTS      TRUE
#define NOPOINTS    FALSE
#define LOWERCASE   TRUE
#define UPPERCASE   FALSE
#define WANDER      TRUE
#define NOWANDER    FALSE
#define GRAB        TRUE
#define NOGRAB      FALSE
#define FAMILIAR    TRUE
#define NOFAMILIAR  FALSE
#define MAXSTATS    TRUE
#define NOMAXSTATS  FALSE
#define FORCE       TRUE
#define NOFORCE     FALSE
#define THROWN      TRUE
#define NOTHROWN    FALSE

/* Ways to die */
#define D_PETRIFY   -1
#define D_ARROW     -2
#define D_DART      -3
#define D_POISON    -4
#define D_BOLT      -5
#define D_SUFFOCATION   -6
#define D_POTION    -7
#define D_INFESTATION   -8
#define D_DROWN     -9
#define D_FALL      -10
#define D_FIRE      -11
#define D_SPELLFUMBLE   -12
#define D_DRAINLIFE -13
#define D_ARTIFACT  -14
#define D_GODWRATH  -15
#define D_CLUMSY    -16

/* values for games end */
#define SCOREIT -1
#define KILLED   0
#define CHICKEN  1
#define WINNER   2
#define TOTAL    3

/*
 * definitions for function step_ok: MONSTOK indicates it is OK to step on a
 * monster -- it is only OK when stepping diagonally AROUND a monster
 */

#define MONSTOK 1
#define NOMONST 2



#define good_monster(m) (on(m, ISCHARMED) || \
            on(m, ISFRIENDLY) || \
            on(m, ISFAMILIAR))

/* Now we define the structures and types */

/* level types */
typedef enum
{
    NORMLEV,        /* normal level */
    POSTLEV,        /* trading post level */
    MAZELEV,        /* maze level */
    THRONE          /* unique monster's throne room */
}   LEVTYPE;

/* Help list */

struct h_list
{
    char        h_ch;
    char        *h_desc;
};

/* Coordinate data type */
typedef struct
{
    int     x;
    int     y;
} coord;

/* Linked list data type */
typedef struct linked_list
{
    struct linked_list  *l_next;
    struct linked_list  *l_prev;

    union
    {
        struct object *obj;
        struct thing  *th;
        void *l_data;
    } data;

} linked_list;

/* Stuff about magic items */

struct magic_item
{
    char   *mi_name;
    char   *mi_abrev;
    int     mi_prob;
    long    mi_worth;
    int     mi_curse;
    int     mi_bless;
};

/* Room structure */
struct room
{
    coord   r_pos;      /* Upper left corner */
    coord   r_max;      /* Size of room */
    coord   r_exit[MAXDOORS];   /* Where the exits are */
    int     r_flags;    /* Info about the room */
    int     r_nexits;   /* Number of exits */
    short   r_fires;    /* Number of fires in room */
};

/* Initial artifact stats */
struct init_artifact
{
    char   *ar_name;   /* name of the artifact */
    int     ar_level;   /* first level where it appears */
    int     ar_rings;   /* number of ring effects */
    int     ar_potions; /* number of potion effects */
    int     ar_scrolls; /* number of scroll effects */
    int     ar_wands;   /* number of wand effects */
    int     ar_worth;   /* gold pieces */
    int     ar_weight;  /* weight of object */
};

/* Array of all traps on this level */

struct trap
{
    coord       tr_pos;     /* Where trap is */
    long        tr_flags;   /* Info about trap (i.e. ISFOUND) */
    char        tr_type;    /* What kind of trap */
    char        tr_show;    /* What disguised trap looks like */
};

/* Structure describing a fighting being */

struct stats
{
    char   *s_dmg;     /* String describing damage done */
    long    s_exp;      /* Experience */
    long    s_hpt;      /* Hit points */
    int     s_pack;     /* current weight of his pack */
    int     s_carry;    /* max weight he can carry */
    int     s_lvl;      /* Level of mastery */
    int     s_arm;      /* Armor class */
    int     s_acmod;    /* Armor clss modifier */
    int     s_power;    /* Spell points */
    int     s_str;      /* Strength */
    int     s_intel;    /* Intelligence */
    int     s_wisdom;   /* Wisdom */
    int     s_dext;     /* Dexterity */
    int     s_const;    /* Constitution */
    int     s_charisma; /* Charisma */
};

/* Structure describing a fighting being (monster at initialization) */

struct mstats
{
    short s_str;        /* Strength */
    long  s_exp;        /* Experience */
    int   s_lvl;        /* Level of mastery */
    int   s_arm;    /* Armor class */
    char *s_hpt;        /* Hit points */
    char *s_dmg;        /* String describing damage done */
};

/* Structure for monsters and player */

/*
    NOTE: In initial v1.04 code the t_chasee variable will be
          reset during the save/restore process. Eventually
          need to implement a "thing" manager where I can
          refer to "things" by number such that references
          to them can be saved. Problems exist with regard
          to removing references to "things" that have been
          killed.
*/

typedef struct thing
{
    long   chasee_index, horde_index; /* Used in save/rest process */
    struct linked_list  *t_pack;    /* What the thing is carrying */
    struct stats    t_stats;    /* Physical description */
    struct stats    maxstats;   /* maximum(or initial) stats */
    int           t_ischasing;  /* Are we chasing someone? */
    struct thing *t_chasee;     /* Who are we chasing */
    struct object *t_horde;     /* What are we hordeing */
    coord      t_pos;      /* Cuurent Position */
    coord      t_oldpos;   /* Last Position    */
    coord      t_nxtpos;   /* Next Position    */
    long       t_flags[16];    /* State word */
    int        t_praycnt;  /* Times prayed... */
    int        t_trans;    /* # of transactions at post */
    int        t_turn;     /* If slowed, is it a turn to move */
    int        t_wasshot;  /* Was character shot last round? */
    int        t_ctype;    /* Character type */
    int        t_index;    /* Index into monster table */
    int        t_no_move;  /* How long the thing can't move */
    int        t_rest_hpt; /* used in hit point regeneration */
    int        t_rest_pow; /* used in spell point regeneration */
    int        t_doorgoal; /* What door are we heading to? */
    char       t_type;     /* What it is */
    char       t_disguise; /* What mimic looks like */
    char       t_oldch;    /* Character that was where it was */
} thing;

/* Array containing information on all the various types of monsters */

struct monster
{
    char         *m_name;    /* What to call the monster */
    short         m_carry;    /* Probability of carrying something */
    int           m_normal;   /* Does monster exist? */
    int           m_wander;   /* Does monster wander? */
    char          m_appear;   /* What does monster look like? */
    char         *m_intel;   /* Intelligence range */
    unsigned long          m_flags[16];    /* Things about the monster */
    char         *m_typesum; /* type of creature can he summon */
    short         m_numsum;   /* how many creatures can he summon */
    short         m_add_exp;  /* Added experience per hit point */
    struct mstats m_stats;    /* Initial stats */
};

/* Structure for a thing that the rogue can carry */

typedef struct object
{
    coord       o_pos;      /* Where it lives on the screen */
    struct linked_list  *next_obj;  /* The next obj. for stacked objects */
    struct linked_list  *o_bag; /* bag linked list pointer */
    char        *o_text;    /* What it says if you read it */
    char        *o_damage;  /* Damage if used like sword */
    char        *o_hurldmg; /* Damage if thrown */
    long        o_flags;    /* Information about objects */
    long        ar_flags;   /* general flags */
    char     o_type;     /* What kind of object it is */
    int     o_ident;    /* identifier for object */
    unsigned int     o_count;    /* Count for plural objects */
    int     o_which;    /* Which object of a type it is */
    int     o_hplus;    /* Plusses to hit */
    int     o_dplus;    /* Plusses to damage */
    int     o_ac;       /* Armor class */
    int     o_group;    /* Group number for this object */
    int     o_weight;   /* weight of this object */
    char        o_launch;   /* What you need to launch it */
    char        o_mark[MARKLEN];    /* Mark the specific object */
    long o_worth;
}       object;

/* weapon structure */

struct init_weps
{
    char        *w_name;    /* name of weapon */
    char        *w_dam;     /* hit damage */
    char        *w_hrl;     /* hurl damage */
    char        w_launch;   /* need to launch it */
    int         w_wght;     /* weight of weapon */
    long        w_worth;    /* worth of this weapon */
    long        w_flags;    /* flags */
};

/* armor structure */

struct init_armor
{
    char        *a_name;    /* name of armor */
    long        a_worth;    /* worth of armor */
    int     a_prob;     /* chance of getting armor */
    int     a_class;    /* normal armor class */
    int     a_wght;     /* weight of armor */
};

struct matrix
{
    int     base;   /* Base to-hit value (AC 10) */
    int     max_lvl;/* Maximum level for changing value */
    int     factor; /* Amount base changes each time */
    int     offset; /* What to offset level */
    int     range;  /* Range of levels for each offset */
};

/*
    ANSI C Prototype Additions
*/

/* armor.c */
extern void wear(void);
extern void take_off(void);
extern void waste_time(void);
extern int  wear_ok(struct thing *th, struct object *obj, int print_message);

/* artifact.c */
extern void apply(void);
extern int possessed(int artifact);
extern int is_carrying(int artifact);
extern int make_artifact(void);
extern struct object *new_artifact(int which, struct object *cur);
extern void do_minor(struct object *obj);
extern void do_major(void);
extern void do_phial(void);
extern void do_palantir(void);
extern void do_sceptre(void);
extern void do_silmaril(void);
extern void do_amulet(void);
extern void do_bag(struct object *obj);
extern void do_wand(void);
extern void do_crown(void);
extern void level_eval(void);

/* bag.c */

typedef union
{
    void *varg;
    int  *iarg;
    struct object *obj;
} bag_arg;


extern struct object *apply_to_bag(struct linked_list *bag_p, int type,
                    int (*bff_p)(struct object *obj, bag_arg *arg),
                    int (*baf_p)(struct object *obj, bag_arg *arg, int id),
                    void *user_arg);
extern int count_bag(linked_list *bag_p, int type,
                    int (*bff_p)(struct object *obj, bag_arg *arg));
extern void del_bag(linked_list *bag_p, object *obj_p);
extern struct object *pop_bag(linked_list **bag_pp, object *obj_p);
extern void push_bag(linked_list **bag_pp, object *obj_p);
extern struct object *scan_bag(linked_list *bag_p, int type, int id);
extern struct object *select_bag(linked_list *bag_p, int type,
                    int (*bff_p)(struct object *obj, bag_arg *arg), int index);

/* chase.c */

extern void do_chase(struct thing *th, int flee);
extern void chase_it(coord *runner, struct thing *th);
extern void runto(void);/* coord *runner, coord *spot); */
extern int chase(struct thing *tp, coord *ee, int flee);
extern struct room *roomin(coord cp);
extern struct linked_list *find_mons(int y, int x);
extern struct linked_list *f_mons_a(int y, int x, int hit_bad);
extern int diag_ok(coord *sp, coord *ep, struct thing *flgptr);
extern int cansee(int y, int x);
extern coord *find_shoot(struct thing *tp, coord *dir);
extern coord *can_shoot(coord *er, coord *ee, coord *dir);
extern int straight_shot(int ery, int erx, int eey, int eex, coord *dir);
extern struct linked_list *get_hurl(struct thing *tp);
extern struct object *pick_weap(struct thing *tp);
extern int can_blink(struct thing *tp);

/* command.c */

extern char  fight_ch;
extern char  countch;
extern void command(void);
extern void do_after_effects(void);
extern void make_omnipotent(void);
extern void quit_handler(int sig);
extern void quit(void);
extern void search(int is_thief);
extern void help(void);
extern void identify(void);
extern void d_level(void);
extern void u_level(void);
extern void call(int mark);
extern int  att_bonus(void);

/* daemon.c */

extern int demoncnt;

struct delayed_action
{
    int d_type;
    int d_when;
    int d_id;
    void *d_arg;
    int d_time;
};

#define EMPTY  0
#define DAEMON 1
#define FUSE   2

typedef void fuse;
typedef void daemon;

typedef union
{
    void *varg;
    int  *iarg;
    struct linked_list *ll;
} fuse_arg;

typedef union
{
    void *varg;
    int  *iarg;
    struct thing *thingptr;
} daemon_arg;

struct fuse
{
    int index;
    fuse (*func)(fuse_arg *arg);
};

struct daemon
{
    int index;
    daemon (*func)(daemon_arg *arg);
};

#define MAXDAEMONS 60

#define FUSE_NULL          0
#define FUSE_SWANDER       1
#define FUSE_UNCONFUSE     2
#define FUSE_UNSCENT       3
#define FUSE_SCENT         4
#define FUSE_UNHEAR        5
#define FUSE_HEAR          6
#define FUSE_UNSEE         7
#define FUSE_UNSTINK       8
#define FUSE_UNCLRHEAD     9
#define FUSE_UNPHASE      10
#define FUSE_SIGHT        11
#define FUSE_RES_STRENGTH 12
#define FUSE_NOHASTE      13
#define FUSE_NOSLOW       14
#define FUSE_SUFFOCATE    15
#define FUSE_CURE_DISEASE 16
#define FUSE_UNITCH       17
#define FUSE_APPEAR       18
#define FUSE_UNELECTRIFY  19
#define FUSE_UNBHERO      20
#define FUSE_UNSHERO      21
#define FUSE_UNXRAY       22
#define FUSE_UNDISGUISE   23
#define FUSE_SHERO        24
#define FUSE_WGHTCHK      25
#define FUSE_UNSUMMON     26
#define FUSE_UNGAZE       27
#define FUSE_UNCOLD       28
#define FUSE_UNHOT        29
#define FUSE_UNFLY        30
#define FUSE_UNBREATHE    31
#define FUSE_UNREGEN      32
#define FUSE_UNSUPEREAT   33
#define FUSE_UNSHIELD     34
#define FUSE_UNMSHIELD    35
#define FUSE_UNTRUESEE    36
#define FUSE_MAX          37

#define DAEMON_NULL       0
#define DAEMON_DOCTOR     1
#define DAEMON_ROLLWAND   2
#define DAEMON_STOMACH    3
#define DAEMON_RUNNERS    4
#define DAEMON_MAX        5

extern struct delayed_action d_list[MAXDAEMONS];
extern struct daemon daemons[DAEMON_MAX];
extern struct fuse fuses[FUSE_MAX];

extern struct delayed_action *d_slot(void);
extern struct delayed_action *find_slot(int id, int type);
extern void start_daemon(int id, void *arg, int whendo);
extern void kill_daemon(int id);
extern void do_daemons(int now);
extern void light_fuse(int id, void *arg, int time, int whendo);
extern void lengthen_fuse(int id, int xtime);
extern void extinguish_fuse(int id);
extern void do_fuses(int flag);
extern void activity(void);

/* daemons.c */

extern void doctor_spell_points(struct thing *tp);
extern daemon runners(daemon_arg *arg);
extern daemon doctor(daemon_arg *tp);
extern daemon rollwand(daemon_arg *arg);
extern daemon stomach(daemon_arg *arg);
extern fuse swander(fuse_arg *arg);
extern fuse unconfuse(fuse_arg *arg);
extern fuse unscent(fuse_arg *arg);
extern fuse scent(fuse_arg *arg);
extern fuse unhear(fuse_arg *arg);
extern fuse hear(fuse_arg *arg);
extern fuse unsee(fuse_arg *arg);
extern fuse unstink(fuse_arg *arg);
extern fuse unclrhead(fuse_arg *arg);
extern fuse unphase(fuse_arg *arg);
extern fuse sight(fuse_arg *arg);
extern fuse res_strength(fuse_arg *arg);
extern fuse nohaste(fuse_arg *arg);
extern fuse noslow(fuse_arg *arg);
extern fuse suffocate(fuse_arg *arg);
extern fuse cure_disease(fuse_arg *arg);
extern fuse un_itch(fuse_arg *arg);
extern fuse appear(fuse_arg *arg);
extern fuse unelectrify(fuse_arg *arg);
extern fuse unshero(fuse_arg *arg);
extern fuse unbhero(fuse_arg *arg);
extern fuse undisguise(fuse_arg *arg);
extern fuse unsummon(fuse_arg *monst);
extern fuse ungaze(fuse_arg *arg);
extern fuse shero(fuse_arg *arg);
extern fuse uncold(fuse_arg *arg);
extern fuse unhot(fuse_arg *arg);
extern fuse unfly(fuse_arg *arg);
extern fuse unbreathe(fuse_arg *arg);
extern fuse unregen(fuse_arg *arg);
extern fuse unsupereat(fuse_arg *arg);
extern fuse unshield(fuse_arg *arg);
extern fuse unmshield(fuse_arg *arg);
extern fuse untruesee(fuse_arg *arg);
extern fuse wghtchk(fuse_arg *arg);

/* encumb.h */

extern void updpack(void);
extern int  packweight(void);
extern int  itemweight(struct object *wh);
extern int  playenc(void);
extern int  totalenc(void);
extern int  hitweight(void);

/* fight.c */

extern void do_fight(coord dir, int tothedeath);
extern int  fight(coord *mp, struct object *weap, int thrown);
extern int  attack(struct thing *mp, struct object *weapon, int thrown);
extern int  mon_mon_attack(struct thing *attacker, struct linked_list *mon,
                           struct object *weapon, int thrown);
extern int  swing(int class, int at_lvl, int op_arm, int wplus);
extern void init_exp(void);
extern int  next_exp_level(int print_message);
extern void check_level(void);
extern int  roll_em(struct thing *att_er, struct thing *def_er,
                    struct object *weap, int thrown, struct object *cur_weapon);
extern const char *prname(char *who);
extern void  hit(char *ee);
extern void  miss(char *ee);
extern int   save_throw(int which, struct thing *tp);
extern int   save(int which);
extern int   dext_plus(int dexterity);
extern int   dext_prot(int dexterity);
extern int   str_plus(int str);
extern int   add_dam(int str);
extern int   hung_dam(void);
extern void  raise_level(void);
extern void  thunk(struct object *weap, char *mname);
extern void  m_thunk(struct object *weap, char *mname);
extern void  bounce(struct object *weap, char *mname);
extern void  m_bounce(struct object *weap, char *mname);
extern void  remove_monster(coord *mp, struct linked_list *item);
extern int   is_magic(struct object *obj);
extern void  killed(struct thing *killer,struct linked_list *item,
                    int print_message, int give_points);
extern struct object *wield_weap(struct object *weapon, struct thing *mp);
extern void  summon_help(struct thing *mons, int force);
extern int   maxdamage(char *cp);

/* getplay.c */

extern int   geta_player(void);
extern void  puta_player(void);
extern void  do_getplayer(void);
extern void  print_stored(void);
extern char *which_class(int c_class);

/* ident.c */

extern char    print_letters[];
extern int     get_ident(struct object *obj_p);
extern void    free_ident(struct object *obj_p);
extern int     unprint_id(char *print_id);
extern int     max_print(void);

/* init.c */

extern void init_materials(void);
extern void init_player(void);
extern void init_flags(void);
extern void init_things(void);
extern void init_fd(void);
extern void init_colors(void);
extern void init_names(void);
extern void init_stones(void);
extern void badcheck(char *name, struct magic_item *magic, int bound);

/* io.c */

extern int get_string(char *buffer, WINDOW *win);
extern void msg(const char *fmt, ...);
extern void addmsg(const char *fmt, ...);
extern void endmsg(void);
extern void doadd(const char *fmt, va_list ap);
extern char readchar(void);
extern char readcharw(WINDOW *scr);
extern void status(int display);
extern void wait_for(int ch);
extern void show_win(WINDOW *scr, char *message);
extern void restscr(WINDOW *scr);
extern void add_line(const char *fmt, ...);
extern void end_line(void);
extern void hearmsg(const char *fmt, ...);
extern void seemsg(const char *fmt, ...);

/* list.c */

extern void *ur_alloc(size_t size);
extern void  ur_free(void *buf_p);
extern void  _detach(struct linked_list **list, struct linked_list *item);
extern void  _attach(struct linked_list **list, struct linked_list *item);
extern void  _attach_after(linked_list **list_pp, linked_list *list_p, linked_list *new_p);
extern void  _free_list(struct linked_list **ptr);
extern void  discard(struct linked_list *item);
extern void  throw_away(struct object *ptr);
extern struct linked_list *new_item(int size);
extern void *new_alloc(size_t size);
extern struct linked_list *new_list(void);

/* magic.c */

/* for printing out messages */

#define CAST_NORMAL       0x000   /* cast normal version  */
#define CAST_CURSED       0x001   /* cast cursed version  */
#define CAST_BLESSED      0x002   /* cast blessed version */
#define CAST_CROWN        0x010   /* crown helped out     */
#define CAST_SEPTRE       0x020   /* septre helped out    */

#define MAX_SPELLS          100   /* Max # sorted_spells  */
#define MIN_FUMBLE_CHANCE     5
#define MAX_FUMBLE_CHANCE    95

/* Spells that a monster can cast */

#define M_SELFTELEP    0
#define M_HLNG2        1
#define M_REGENERATE   2
#define M_HLNG         3
#define NUM_RUN        4
#define M_HASTE        4
#define M_SEEINVIS     5
#define M_SHERO        6
#define M_PHASE        7
#define M_INVIS        8
#define M_CANCEL       9
#define M_OFFENSE     10


struct spells
{
    int sp_level;   /* level of casting spell */
    int sp_which;   /* which scroll or potion */
    unsigned long sp_flags;   /* scroll, blessed, known */
    int sp_cost;    /* generated in incant()  */
};


extern void incant(struct thing *caster, coord shoot_dir);
extern char *spell_name(struct spells *sp, char *buf);
extern char *spell_abrev(struct spells *sp, char *buf);
extern void fumble_spell(struct thing *caster, int num_fumbles);
extern void learn_new_spells(void);
extern struct spells *pick_monster_spell(struct thing *caster);
extern int sort_spells(const void *a, const void *b);

/* magicitm.c */

extern struct magic_item    things[];   /* Chances for each type of item*/
extern struct magic_item    s_magic[];  /* Names and chances for scrolls*/
extern struct magic_item    p_magic[];  /* Names and chances for potions*/
extern struct magic_item    r_magic[];  /* Names and chances for rings  */
extern struct magic_item    ws_magic[]; /* Names and chances for sticks */
extern struct magic_item    fd_data[];  /* Names and chances for food   */
extern struct init_weps     weaps[];    /* weapons and attributes   */
extern struct init_armor    armors[];   /* armors and attributes    */
extern struct init_artifact arts[];     /* artifacts and attributes     */
extern int  maxarmors;
extern int  maxartifact;
extern int  maxfoods;
extern int  maxpotions;
extern int  maxrings;
extern int  maxscrolls;
extern int  maxsticks;
extern int  maxweapons;
extern int  numthings;
extern char *s_names[];         /* Names of the scrolls */
extern char *p_colors[];        /* Colors of the potions */
extern char *r_stones[];        /* Stone settings of the rings */
extern char *guess_items[MAXMAGICTYPES][MAXMAGICITEMS];
                /* Players guess at what magic is */
extern int know_items[MAXMAGICTYPES][MAXMAGICITEMS];
                /* Does he know what a magic item does */
extern char *ws_type[];     /* Is it a wand or a staff */
extern char *ws_made[];     /* What sticks are made of */

/* main.c */

extern FILE *fd_score;
extern int   summoned;
extern coord dta;
extern int   main(int argc, char *argv[]);
extern void  fatal(char *s);
extern int rnd(int range);
extern unsigned char ucrnd(unsigned char range);
extern short srnd(short range);
extern unsigned long ulrnd(unsigned long range);
extern int   roll(int number, int sides);

/* maze.c */

extern void do_maze(void);
extern void draw_maze(void);
extern char *moffset(int y, int x);
extern char *foffset(int y, int x);
extern int findcells(int y, int x);
extern void rmwall(int newy, int newx, int oldy, int oldx);
extern void crankout(void);

/* memory.c */

extern void  mem_debug(const int level);
extern void  mem_tracking(int flag);
extern int   mem_check(char *fname, int line);
extern void *mem_malloc(const size_t bytes);
extern void  mem_free(const void *ptr);
extern void *mem_realloc(const void *ptr, const size_t new_size);
extern int   mem_validate(const void *ptr);

/* misc.c */

extern char *tr_name(char ch, char *buf);
extern void  look(int wakeup);
extern char  secretdoor(int y, int x);
extern struct linked_list *find_obj(int y, int x);
extern void  eat(void);
extern void  chg_str(int amt, int both, int lost);
extern void  chg_dext(int amt, int both, int lost);
extern void  add_haste(int blessed);
extern void  aggravate(void);
extern char  *vowelstr(char *str);
extern int is_current(struct object *obj);
extern int get_dir(void);
extern int is_wearing(int type);
extern int maze_view(int y, int x);
extern void listen(void);
extern void nothing_message(int flags);
extern void feel_message(void);
extern int const_bonus(void);
extern int int_wis_bonus(void);
extern void electrificate(void);
extern void feed_me(int hungry_state);
extern int get_monster_number(char *message);

/* monsdata.c */

extern struct monster monsters[];
extern int nummonst;

/* monsters.c */

extern struct linked_list *summon_monster(int type, int familiar, int print_message);
extern int randmonster(int wander, int grab);
extern void new_monster(struct linked_list *item, int type, coord *cp, int max_monster);
extern void wanderer(void);
extern struct linked_list *wake_monster(int y, int x);
extern void genocide(int flags);
extern void id_monst(int monster);
extern void check_residue(struct thing *tp);
extern void sell(struct thing *tp);
extern void carried_weapon(struct thing *owner, struct object *weapon);

/* move.c */

extern coord nh;
extern void do_run(char ch);
extern int  step_ok(int y, int x, int can_on_monst, struct thing *flgptr);
extern void corr_move(int dy, int dx);
extern void do_move(int dy, int dx);
extern void light(coord *cp);
extern int  blue_light(int flags);
extern char  show(int y, int x);
extern char be_trapped(struct thing *th, coord tc);
extern void dip_it(void);
extern struct trap *trap_at(int y, int x);
extern void set_trap(struct thing *tp, int y, int x);
extern coord rndmove(struct thing *who);
extern int isatrap(int ch);

/* newlvl.c */

extern void new_level(LEVTYPE ltype, int special);
extern void put_things(LEVTYPE ltype);
extern int throne_monster;
extern void do_throne(int special);
extern void create_lucifer(coord *stairs);

/* options.c */

typedef union
{
    void *varg;
    char *str;
    int  *iarg;
} opt_arg;

struct optstruct
{
    char    *o_name;    /* option name */
    char    *o_prompt;  /* prompt for interactive entry */
    opt_arg o_opt;     /* pointer to thing to set */
    void    (*o_putfunc)(opt_arg *arg,WINDOW *win);/* func to print value */
    int     (*o_getfunc)(opt_arg *arg,WINDOW *win);/* func to get value  */
};

typedef struct optstruct    OPTION;

extern void parse_opts(char *str);
extern void option(void);
extern void put_bool(opt_arg *iarg, WINDOW *win);
extern void put_str(opt_arg *str, WINDOW *win);
extern void put_abil(opt_arg *ability, WINDOW *win);
extern void put_inv(opt_arg *inv, WINDOW *win);
extern int  get_bool(opt_arg *bp, WINDOW *win);
extern int  get_str(opt_arg *opt, WINDOW *win);
extern int  get_abil(opt_arg *abil, WINDOW *win);
extern int  get_inv(opt_arg *inv, WINDOW *win);

/* pack.c */

extern void swap_top(struct linked_list *top, struct linked_list *node);
extern void get_all(struct linked_list *top);
extern struct linked_list *get_stack(struct linked_list *item);
extern int add_pack(struct linked_list  *item, int print_message);
extern void pack_report(object *obj, int print_message, char *message);
extern void inventory(struct linked_list *container, int type);
extern void pick_up(char ch);
extern struct object *get_object(struct linked_list *container, char *purpose, int type, int (*bff_p)(struct object *obj, bag_arg *junk) );
extern struct linked_list *get_item(char *purpose, int type);
extern void del_pack(struct linked_list *what);
extern void discard_pack(struct object *obj_p);
extern void rem_pack(struct object *obj_p);
extern void cur_null(struct object *op);
extern void idenpack(void);
extern void show_floor(void);

/* passages.c */

struct rdes
{
    int conn[MAXROOMS];     /* possible to connect to room i? */
    int isconn[MAXROOMS];   /* connection been made to room i? */
    int ingraph;            /* this room in graph already? */
};

extern void do_passages(void);
extern void conn(int r1, int r2);
extern void door(struct room *rm, coord *cp);

/* player.c */

extern void prayer(void);
extern int  gsense(void);
extern int  is_stealth(struct thing *tp);
extern void steal(void);
extern void affect(void);
extern void undead_sense(void);

/* potions.c */

extern void quaff(struct thing *quaffer, int which, int flag);
extern void lower_level(int who);
extern void res_dexterity(void);
extern void res_wisdom(void);
extern void res_intelligence(void);
extern void add_strength(int cursed);
extern void add_intelligence(int cursed);
extern void add_wisdom(int cursed);
extern void add_dexterity(int cursed);
extern void add_const(int cursed);
extern void monquaff(struct thing *quaffer, int which, int flag);

/* properti.c */

extern int baf_print_item(struct object *obj_p, bag_arg *type, int id);
extern int baf_identify(struct object *obj_p, bag_arg *junk, int id);
extern int baf_curse(struct object *obj_p, bag_arg *junk, int id);
extern int baf_decrement_test(struct object *obj_p, bag_arg *count_p, int id);
extern int baf_increment(struct object *obj_p, bag_arg *count_p, int id);
extern int bafcweapon(struct object *obj_p, bag_arg *junk, int id);
extern int bafcarmor(struct object *obj_p, bag_arg *junk, int id);
extern int bff_group(struct object *obj_p, bag_arg *new_obj_p);
extern int bff_callable(struct object *obj_p, bag_arg *junk);
extern int bff_markable(struct object *obj_p, bag_arg *junk);
extern int bffron(object *obj_p, bag_arg *junk);
extern int bff_zappable(struct object *obj_p, bag_arg *junk);

/* random.c */

extern void ur_srandom(unsigned x);
extern long ur_random(void);

/* rings.c */

extern void ring_on(void);
extern void ring_off(void);
extern int ring_eat(int hand);
extern char *ring_num(struct object *obj, char *buf);
extern int   ring_value(int type);

/* rip.c */

extern void death(int monst);
extern void score(long amount, int lvl, int flags, int monst);
extern void total_winner(void);
extern char *killname(int monst, char *buf);
extern void showpack(char *howso);
extern void byebye(void);
extern int  save_resurrect(int bonus);

/* rogue.c */

extern const char *monstern;
extern const struct h_list helpstr[];
extern const char *cnames[][15];      /* Character level names */
extern char *spacemsg;
extern char *morestr;
extern char *retstr;

/* state.c */

extern int mpos;           /* Where cursor is on top line */
extern struct trap    traps[];
extern struct room    rooms[];      /* One for each room -- A level */
extern struct thing   player;       /* The rogue */
extern struct thing  *beast;        /* The last monster attacking */
extern struct object *cur_armor;    /* What a well dresssed rogue wears */
extern struct object *cur_weapon;   /* Which weapon he is wielding */
extern struct object *cur_ring[];   /* What rings are being worn */
extern struct linked_list *fam_ptr;     /* A ptr to the familiar */
extern struct linked_list *lvl_obj;     /* List of objects on this level */
extern struct linked_list *mlist;       /* List of monsters on the level */
extern struct linked_list *curr_mons;   /* The mons. currently moving */
extern struct linked_list *next_mons;   /* The mons. after curr_mons */

extern int  prscore;    /* Print scores */
extern int  prversion;  /* Print version info */

extern WINDOW   *cw;            /* Window that the player sees */
extern WINDOW   *hw;            /* Used for the help command */
extern WINDOW   *mw;            /* Used to store mosnters */
extern LEVTYPE  levtype;
extern coord    delta;          /* Change indicated to get_dir() */

extern char *release;       /* Release number of rogue */
extern char *lastfmt;
extern char *lastarg;
extern unsigned long    total;      /* Total dynamic memory bytes */
extern long purse;         /* How much gold the rogue has */
extern int line_cnt;       /* Counter for inventory style */
extern int newpage;
extern int resurrect;      /* resurrection counter */
extern int foodlev;        /* how fast he eats food */
extern int see_dist;       /* (how far he can see)^2 */
extern int level;          /* What level rogue is on */
extern int ntraps;         /* Number of traps on this level */
extern int no_command;     /* Number of turns asleep */
extern int no_food;        /* Number of levels without food */
extern int count;          /* Number of times to repeat command */
extern int dnum;           /* Dungeon number */
extern int max_level;      /* Deepest player has gone */
extern int food_left;      /* Amount of food in hero's stomach */
extern int group;          /* Current group number */
extern int hungry_state;       /* How hungry is he */
extern int infest_dam;     /* Damage from parasites */
extern int lost_str;       /* Amount of strength lost */
extern int lost_dext;      /* amount of dexterity lost */
extern int hold_count;     /* Number of monsters holding player */
extern int trap_tries;     /* Number of attempts to set traps */
extern int has_artifact;       /* set for possesion of artifacts */
extern int picked_artifact;    /* set for any artifacts picked up */
extern int msg_index;      /* pointer to current message buffer */
extern int luck;           /* how expensive things to buy thing */
extern int fam_type;       /* Type of familiar */
extern int times_prayed;       /* The number of time prayed */
extern int mons_summoned;      /* Number of summoned monsters */
extern int char_type;      /* what type of character is player */
extern int pool_teleport;      /* just teleported from a pool */
extern int inwhgt;         /* true if from wghtchk() */
extern int running;        /* True if player is running */
extern int fighting;       /* True if player is fighting */
extern int playing;        /* True until he quits */
extern int wizard;         /* True if allows wizard commands */
extern int wiz_verbose;        /* True if show debug messages */
extern int after;          /* True if we want after daemons */
extern int fight_flush;        /* True if toilet input */
extern int terse;          /* True if we should be short */
extern int doorstop;       /* Stop running when we pass a door */
extern int jump;           /* Show running as series of jumps */
extern int door_stop;      /* Current status of doorstop */
extern int firstmove;      /* First move after setting door_stop */
extern int waswizard;      /* Was a wizard sometime */
extern int canwizard;      /* Will be permitted to do this */
extern int askme;          /* Ask about unidentified things */
extern int moving;         /* move using 'm' command */

extern int inv_type;       /* Inven style. Bool so options works */
extern char take;           /* Thing the rogue is taking */
extern char PLAYER;         /* what the player looks like */
extern char prbuf[];        /* Buffer for sprintfs */
extern char runch;          /* Direction player is running */
extern char whoami[];       /* Name of player */
extern char fruit[];        /* Favorite fruit */
extern char msgbuf[10][2 * LINELEN];    /* message buffer */
extern char file_name[];        /* Save file name */
extern char score_file[];       /* Score file name */

extern struct linked_list   *arrow, *bolt, *rock, *silverarrow, *fbbolt;
extern struct linked_list   *bullet, *firearrow, *dart, *dagger, *shuriken;
extern struct linked_list   *oil, *grenade;
extern struct room  *oldrp;     /* Roomin(&oldpos) */

extern void ur_write_thing(FILE *savef, struct thing *t);
extern struct thing *ur_read_thing(FILE *savef);
extern void ur_write_object_stack(FILE *savef, struct linked_list *l);
extern void ur_write_bag(FILE *savef, struct linked_list *l);
extern struct linked_list *ur_read_bag(FILE *savef);
extern struct linked_list *ur_read_object_stack(FILE *savef);
extern int restore_file(FILE *savef);

/* rooms.c */

extern void do_rooms(void);
extern void draw_room(struct room *rp);
extern void horiz(int cnt);
extern void vert(int cnt);
extern void rnd_pos(struct room *rp, coord *cp);
extern int  rnd_room(void);

/* save.c */

extern int restore(char *file);
extern int save_game(void);
extern void save_file(FILE *savefd);

/* scrolls.c */

extern void read_scroll(struct thing *reader, int which, int flags);
extern struct linked_list *creat_mons(struct thing *person,int monster,int message);
extern int place_mons(int y, int x, coord *pos);
extern int is_r_on(struct object *obj);
extern void monread(struct thing *reader, int which, int flags);

/* status.c */

extern int has_defensive_spell(struct thing th);

/* sticks.c */

extern void fix_stick(struct object *cur);
extern void do_zap(struct thing *zapper, int which, unsigned long flags);
extern void drain(int ymin, int ymax, int xmin, int xmax);
extern char *charge_str(struct object *obj, char *buf);
extern void shoot_bolt(struct thing *shooter, coord start, coord dir, int get_points, int reason, char *name, int damage);
extern void monster_do_zap(struct thing *zapper, int which, int flags);
extern void cancel_player(int not_unique);

/* things.c */

extern char *inv_name(struct object *obj, int lowercase);
extern void rem_obj(struct linked_list *item, int dis);
extern void add_obj(struct linked_list *item, int y, int x);
extern int  drop(struct linked_list *item);
extern int dropcheck(struct object *op);
extern struct linked_list *new_thing(void);
extern struct linked_list *spec_item(char type, int which, int hit, int damage);
extern int pick_one(struct magic_item *magic, int nitems);
extern char *blesscurse(long flags);
extern int extras(void);
extern char *name_type(int type);
extern linked_list *make_item(object *obj_p);
extern int is_member(char *list_p, int member);

/* trader.c */

extern void do_post(void);
extern void buy_it(char itemtype, int flags);
extern void sell_it(void);
extern void describe_it(struct object *obj);
extern int  open_market(void);
extern int  get_worth(struct object *obj);
extern void trans_line(void);

/* verify.c */

extern void  verify_function(const char *file, const int line);

/* vers.c */

extern char *save_format;
extern char *version;
extern char *release;

/* weapons.c */

extern coord do_motion(int ob, int ydelta, int xdelta, struct thing *tp);
extern void fall(struct thing *tp, struct linked_list *item, int pr, int
player_owned);
extern int hit_monster(int y, int x, struct object *weapon, struct thing *thrower);
extern int wield_ok(struct thing *wieldee, struct object *obj, int print_message);
extern int shoot_ok(int ch);
extern int fallpos(coord pos, coord *newpos);
extern void wield(void);
extern char *num(int n1, int n2, char *buf);
extern void init_weapon(struct object *weap, int type);
extern void missile(int ydelta, int xdelta, struct linked_list *item, struct thing *tp);

/* wizard.c */

extern void whatis(struct linked_list *what);
extern void teleport(void);
extern int passwd(void);

/* mdport.c */
char *md_strdup(const char *s);
long md_random(void);
void md_srandom(long seed);
int md_readchar(WINDOW *win);

#define NOOP(x) (x += 0)
#define CCHAR(x) ( (char) (x & A_CHARTEXT) )