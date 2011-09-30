/*
    state.c - Portable Rogue Save State Code

    Copyright (C) 1993, 1995 Nicholas J. Kisseberth

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name(s) of the author(s) nor the names of other contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

/*
    Notes

        Should move all game variables into one place
        Should move save/restore code into save.c or some such
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "rogue.h"

/*
     Variables for global game state.

     All variables that need to get saved when saving a game
     are defined in this file. Long term goal is to move many
     of these variables into a "struct level" data type of some
     kind... perhaps not, maybe struct game...

     Other global variables that don't need to get saved are
     kept in main.c.

     Other global variables that don't change during the course
     of a game are kept in urogue.c, monsdata.c, data.c.
*/

#define _X_ { 0, 0, 0, 0, 0 }

struct delayed_action
d_list[MAXDAEMONS] =        /* daemon/fuse list                         */
{
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
        _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_, _X_,
};

#undef _X_

char *s_names[MAXSCROLLS];   /* Names of the scrolls         */
char *p_colors[MAXPOTIONS];  /* Colors of the potions        */
char *r_stones[MAXRINGS];    /* Stone settings of the rings  */
char *ws_made[MAXSTICKS];    /* What sticks are made of      */
char *ws_type[MAXSTICKS];    /* Is it a wand or a staff      */

char *guess_items[MAXMAGICTYPES][MAXMAGICITEMS]; /* guess magic is          */
int   know_items[MAXMAGICTYPES][MAXMAGICITEMS];  /* flag knowlede of magic  */
                                                 /* object data             */

struct trap          traps[2 * MAXTRAPS];   /* 2x for special effects       */
struct room          rooms[MAXROOMS];       /* One for each room -- A level */
struct room         *oldrp = NULL;          /* Roomin(&player.t_oldpos)     */
struct thing         player;                /* The rogue                    */
struct linked_list  *lvl_obj = NULL;        /* Treasure on this level       */
struct linked_list  *fam_ptr = NULL;        /* A ptr to the familiar        */
struct linked_list  *mlist = NULL;          /* Monsters on this level       */
struct thing        *beast;                 /* The last beast that attacked */
struct object       *cur_armor  = NULL;     /* what rogue  wears            */
struct object       *cur_weapon = NULL;     /* ... and wields               */
struct object       *cur_ring[10];          /* His rings                    */
struct linked_list  *curr_mons  = NULL;     /* The mons. currently moving   */
struct linked_list  *next_mons = NULL;      /* The mons. after curr_mons    */

/* Misc. game state info */
char dummybuf1[50000];
char dummybuf2[50000];
char msgbuf[10][2*LINELEN]; /* message buffer history                   */
int  msg_index = 0;         /* index in msg history buffer for nxt msg  */
int  foodlev         = 1;           /* how fast he eats food            */
int  ntraps          = 0;           /* Number of traps on this level    */
int  dnum            = 0;           /* Dungeon number                   */
int  max_level       = 0;           /* Deepest player has gone          */
int  lost_dext       = 0;           /* amount of lost dexterity         */
int  no_command      = 0;
int  level           = 0;
int  see_dist        = 3;
int  no_food         = 0;
int  count           = 0;
int  food_left       = HUNGERTIME;
int  group           = 1;
int  hungry_state    = F_OK;
int  infest_dam      = 0;
int  lost_str        = 0;
int  hold_count      = 0;
int  trap_tries      = 0;
int  has_artifact    = 0;
int  picked_artifact = 0;
int  luck            = 0;
int  resurrect       = 0;
int  fam_type        = 0;           /* The type of familiar             */
int  mons_summoned   = 0;           /* Number of summoned monsters      */
char PLAYER          = VPLAYER;     /* what the player looks like       */
char take            = 0;           /* Thing the rogue is taking        */
char runch           = 0;           /* Direction player is running      */
int  char_type       = C_NOTSET;    /* what type of character is player */
int  inv_type        = INV_CLEAR;   /* Overwrite style of inventory     */
int  pool_teleport   = FALSE;       /* just teleported from a pool      */
int  inwhgt          = FALSE;       /* true if from wghtchk()           */
int  after           = 0;           /* True if we want after daemons    */
int  waswizard       = 0;           /* Was a wizard sometime            */
int  canwizard       = 1;           /* Will be permitted to do this     */
int  playing         = TRUE;
int  running         = FALSE;
int  fighting        = FALSE;
int  wizard          = FALSE;
int  wiz_verbose     = TRUE;
int  moving          = FALSE;
coord    delta;                     /* Change indicated to get_dir()    */
LEVTYPE  levtype;                   /* type of level i'm on             */
long purse  = 0;
unsigned long total  = 0;

WINDOW  *cw;                        /* Window that the player sees      */
WINDOW  *hw;                        /* Used for the help command        */
WINDOW  *mw;                        /* Used to store mosnters           */

/* options.o    */
/* game options */

int  terse       = FALSE;
int  door_stop   = FALSE;
int  jump        = TRUE;
int  doorstop    = TRUE;
int  firstmove   = FALSE;
int  askme       = FALSE;
char whoami[2 * LINELEN];           /* Name of player  */
char fruit[2 * LINELEN];            /* Favorite fruit  */
char file_name[2 * LINELEN];        /* Save file name  */
char score_file[2 * LINELEN];       /* Score file name */

/****************************************************************************/
/* Portable Save State Code                                                 */
/*                                                                          */
/*    UltraRogue v1.04                                                      */
/****************************************************************************/

#define URS_STATS        0xABCD0001
#define URS_THING        0xABCD0002
#define URS_OBJECT       0xABCD0003
#define URS_MAGIC        0xABCD0004
#define URS_KNOWS        0xABCD0005
#define URS_GUESSES      0xABCD0006
#define URS_STACKOBJECT  0xABCD0007
#define URS_BAGOBJECT    0xABCD0008
#define URS_MONSTERLIST  0xABCD0009
#define URS_MONSTERSTATS 0xABCD000A
#define URS_MONSTER      0xABCD000B
#define URS_TRAP         0xABCD000C
#define URS_WINDOW       0xABCD000D
#define URS_DAEMONS      0xABCD000E

void
ur_write(FILE *savef, void *ptr, size_t size)
{
    if (size == 0)
        return;

    fwrite(ptr,size,1,savef);
}

void
ur_read(FILE *savef, void *ptr, size_t size)
{
    if (size == 0)
        return;

    fread(ptr,size,1,savef);
}

void
ur_write_int(FILE *savef, int c)
{
    ur_write(savef,&c,sizeof(int));
}

int
ur_read_int(FILE *savef)
{
    int i;

    ur_read(savef, &i, sizeof(int));

    return(i);
}

void
ur_write_short(FILE *savef, short c)
{
    ur_write(savef,&c,sizeof(short));
}

short
ur_read_short(FILE *savef)
{
    short s;

    ur_read(savef, &s, sizeof(short));

    return(s);
}

void
ur_write_long(FILE *savef, long c)
{
    ur_write(savef,&c,sizeof(long));
}

long
ur_read_long(FILE *savef)
{
    long l;

    ur_read(savef, &l, sizeof(long));

    return(l);
}

void
ur_write_ulong(FILE *savef, unsigned long c)
{
    ur_write(savef,&c,sizeof(unsigned long));
}

unsigned long
ur_read_ulong(FILE *savef)
{
    long l;

    ur_read(savef, &l, sizeof(unsigned long));

    return(l);
}

void
ur_unread_long(FILE *savef)
{
    fseek(savef, -(long)sizeof(long), SEEK_CUR);
}

void
ur_write_char(FILE *savef, char c)
{
    ur_write(savef,&c,sizeof(char));
}

char
ur_read_char(FILE *savef)
{
    char c;

    ur_read(savef, &c, sizeof(char));

    return(c);
}

void
ur_write_string(FILE *savef, char *s)
{
    size_t len;

    len = (s == NULL) ? 0L : strlen(s) + 1 ;

    ur_write_long(savef, (long) len);
    ur_write(savef,s,len);
}


char *
ur_read_string(FILE *savef)
{
    size_t  len;
    char   *buf;

    len = ur_read_long(savef);

    if (len == 0)
        return(NULL);

    buf = ur_alloc(len);

    if (buf == NULL)     /* Should flag a global error condition... */
        return(NULL);

    ur_read(savef,buf,len);

    return(buf);
}

void
ur_write_coord(FILE *savef, coord c)
{
    ur_write_int(savef, c.x);
    ur_write_int(savef, c.y);
}

coord
ur_read_coord(FILE *savef)
{
    coord c;

    c.x = ur_read_int(savef);
    c.y = ur_read_int(savef);

    return(c);
}

void
ur_write_room(FILE *savef, struct room *r)
{
    int i;

    ur_write_coord(savef, r->r_pos);
    ur_write_coord(savef, r->r_max);

    for(i=0; i<MAXDOORS; i++)
        ur_write_coord(savef, r->r_exit[i]);

    ur_write_int(savef, r->r_flags);
    ur_write_int(savef, r->r_nexits);
    ur_write_int(savef, r->r_flags);
}

struct room *
ur_read_room(FILE *savef)
{
    struct room *r;
    int i;

    r = ur_alloc( sizeof(struct room) );

    r->r_pos = ur_read_coord(savef);
    r->r_max = ur_read_coord(savef);

    for(i=0; i<MAXDOORS; i++)
        r->r_exit[i] = ur_read_coord(savef);

    r->r_flags = ur_read_int(savef);
    r->r_nexits = ur_read_int(savef);
    r->r_flags = ur_read_short(savef);

    return(r);
}

void
ur_write_object(FILE *savef, struct object *o)
{
    int other;

    ur_write_long(savef,      URS_OBJECT);
    ur_write_coord(savef,     o->o_pos);
    ur_write_string(savef,    o->o_text);
    ur_write_string(savef,    o->o_damage);
    ur_write_string(savef,    o->o_hurldmg);
    ur_write_long(savef,      o->o_flags);
    ur_write_long(savef,      o->ar_flags);
    ur_write_char(savef,       o->o_type);
    ur_write_int(savef,       o->o_ident);
    ur_write_int(savef,       o->o_count);
    ur_write_int(savef,       o->o_which);
    ur_write_int(savef,       o->o_hplus);
    ur_write_int(savef,       o->o_dplus);
    ur_write_int(savef,       o->o_ac);
    ur_write_int(savef,       o->o_group);
    ur_write_int(savef,       o->o_weight);
    ur_write_char(savef,      o->o_launch);
    ur_write(savef,          &o->o_mark[0], MARKLEN);
    ur_write_long(savef,      o->o_worth);

    other = 0;

    if (o->o_bag)
        other = 1;
    else if (o->next_obj)
        other |= 2;

    ur_write_int(savef,other);

    if (o->o_bag)
        ur_write_bag(savef,o->o_bag);
    if (o->next_obj && (o->next_obj->l_prev == NULL) )
        ur_write_object_stack(savef, o->next_obj);
}

struct object *
ur_read_object(FILE *savef)
{
    struct object *o;
    long id;
    int other;

    o = ur_alloc(sizeof(struct object));

    if (o == NULL)
        return(NULL);

    memset(o,0,sizeof(struct object));

    id = ur_read_long(savef);

    assert(id == URS_OBJECT);

    o->o_pos  = ur_read_coord(savef);
    o->o_text = ur_read_string(savef);
    o->o_damage = ur_read_string(savef);
    o->o_hurldmg = ur_read_string(savef);
    o->o_flags = ur_read_long(savef);
    o->ar_flags = ur_read_long(savef);
    o->o_type = ur_read_char(savef);
    o->o_ident = ur_read_int(savef);
    o->o_count = ur_read_int(savef);
    o->o_which = ur_read_int(savef);
    o->o_hplus = ur_read_int(savef);
    o->o_dplus = ur_read_int(savef);
    o->o_ac = ur_read_int(savef);
    o->o_group = ur_read_int(savef);
    o->o_weight = ur_read_int(savef);
    o->o_launch = ur_read_char(savef);
    ur_read(savef, &o->o_mark[0], MARKLEN);
    o->o_worth = ur_read_long(savef);

    other = ur_read_int(savef);

    if (other & 1)
        o->o_bag = ur_read_bag(savef);
    if (other & 2)
        o->next_obj = ur_read_object_stack(savef);

    return(o);
}

int
list_size(struct linked_list *l)
{
    int cnt=0;

    if (l == NULL)
        return(0);

    while(l != NULL)
    {
        cnt++;
        l = l->l_next;
    }

    return(cnt);
}

int
find_thing_index(struct linked_list *l, struct thing *item)
{
    int cnt=0;

    if (l == NULL)
        return(-1);

    while(l != NULL)
    {
        if (item == l->data.th)
            return(cnt+1);

        cnt++;
        l = l->l_next;
    }

    return(0);
}


int
find_list_index(struct linked_list *l, struct object *item)
{
    int cnt=0;

    if (l == NULL)
        return(-1);

    while(l != NULL)
    {
        if (item == l->data.obj)
            return(cnt+1);

        cnt++;
        l = l->l_next;
    }

    return(0);
}

struct object *
find_object(struct linked_list *list, int num)
{
    int cnt = 0;
    struct linked_list *l = list;
	
    if ( (num < 1) || (list == NULL) )
        return(NULL);
    
	num--;

    for(cnt = 0; cnt < num; cnt++)
    {
        if ( l == NULL )
            return(NULL);

        l = l->l_next;
    }

    return(l->data.obj);
}

struct thing *
find_thing(struct linked_list *list, int num)
{
    int cnt = 0;
    struct linked_list *l = list;

    if ( (num < 1) || (list == NULL) )
        return(NULL);
    num--;

    for(cnt = 0; cnt < num; cnt++)
    {
        if (l == NULL)
            return(NULL);

        l = l->l_next;
    }

    return(l->data.th);
}


void
ur_write_object_stack(FILE *savef, struct linked_list *l)
{
    int cnt;

    ur_write_long(savef, URS_STACKOBJECT);

    ur_write_int(savef, cnt = list_size(l) );

    if (cnt == 0)
        return;

    while(l != NULL)
    {
        ur_write_object(savef, l->data.obj);
        l = l->l_next;
    }
}

void
ur_write_bag(FILE *savef, struct linked_list *l)
{
    int cnt;

    ur_write_long(savef, URS_BAGOBJECT);

    ur_write_int(savef, cnt = list_size(l) );

    if (cnt == 0)
        return;

    while(l != NULL)
    {
        ur_write_object(savef, l->data.obj);
        l = l->l_next;
    }
}

struct linked_list *
ur_read_object_stack(FILE *savef)
{
    long id;
    int i,cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    id = ur_read_long(savef);

    assert(id == URS_STACKOBJECT);

    cnt = ur_read_int(savef);

    for(i = 0; i < cnt; i++)
    {
        l         = new_list();
        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        l->data.obj = ur_read_object(savef);

        if (previous == NULL)
            head = l;

        previous = l;
    }

    if (l != NULL)
        l->l_next = NULL;

    return(head);
}


struct linked_list *
ur_read_bag(FILE *savef)
{
    long id;
    int i,cnt;
    struct linked_list *l = NULL, *previous = NULL, *head = NULL;

    id = ur_read_long(savef);

    assert( id == URS_BAGOBJECT );

    cnt = ur_read_int(savef);

    for(i = 0; i < cnt; i++)
    {
        l         = new_list();
        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        l->data.obj =  ur_read_object(savef);

        if (previous == NULL)
            head = l;

        previous = l;
    }

    if (l != NULL)
        l->l_next = NULL;

    return(head);
}

void
ur_fixup_monsters(struct linked_list *l)
{
    while(l != NULL)
    {
        if (l->data.th->t_chasee == (void *) -1L)
            l->data.th->t_chasee = &player;
        else
            l->data.th->t_chasee = find_thing(mlist, l->data.th->chasee_index);

        l->data.th->t_horde = find_object(lvl_obj, l->data.th->horde_index);

        l = l->l_next;
    }
}

void
ur_write_monsters(FILE *savef, struct linked_list *l)
{
    int cnt;

    ur_write_long(savef, URS_MONSTERLIST);

    cnt = list_size(l);

    ur_write_int(savef, cnt);

    if (cnt < 1)
        return;

    while(l != NULL)
    {
        ur_write_thing(savef, l->data.th);
        l = l->l_next;
    }
}

struct linked_list *
ur_read_monsters(FILE *savef)
{
    long id;
    int i,cnt;
    struct linked_list *l=NULL, *previous = NULL, *head = NULL;

    id = ur_read_long(savef);

    assert(id == URS_MONSTERLIST);

    cnt = ur_read_int(savef);

    if (cnt == 0)
        return(NULL);

    for(i = 0; i < cnt; i++)
    {
        l = new_list();

        l->l_prev = previous;

        if (previous != NULL)
            previous->l_next = l;

        l->data.th = ur_read_thing(savef);

        if (previous == NULL)
            head = l;

        previous = l;
    }

    if (l != NULL)
        l->l_next = NULL;

    return(head);
}

void
ur_write_monster_stats(FILE *savef, struct mstats *m)
{
    ur_write_long(savef, URS_MONSTERSTATS);
    ur_write_short(savef, m->s_str);
    ur_write_long(savef, m->s_exp);
    ur_write_int(savef, m->s_lvl);
    ur_write_int(savef, m->s_arm);
    ur_write_string(savef, m->s_hpt);
    ur_write_string(savef, m->s_dmg);
}

struct mstats *
ur_read_monster_stats(FILE *savef)
{
    long id;
    struct mstats *m;

    id = ur_read_long(savef);

    assert(id == URS_MONSTERSTATS);

    m = ur_alloc( sizeof(struct mstats) );

    m->s_str = ur_read_short(savef);
    m->s_exp = ur_read_long(savef);
    m->s_lvl = ur_read_int(savef);
    m->s_arm = ur_read_int(savef);
    m->s_hpt = ur_read_string(savef);
    m->s_dmg = ur_read_string(savef);

    return(m);
}

void
ur_write_monster(FILE *savef, struct monster *m)
{
    int i;

    ur_write_long(savef, URS_MONSTER);
    ur_write_string(savef, m->m_name);
    ur_write_short(savef, m->m_carry);
    ur_write_int(savef, m->m_normal);
    ur_write_int(savef, m->m_wander);
    ur_write_char(savef, m->m_appear);
    ur_write_string(savef, m->m_intel);

    for(i = 0; i < 10; i++)
        ur_write_long(savef, m->m_flags[i]);

    ur_write_string(savef, m->m_typesum);
    ur_write_short(savef, m->m_numsum);
    ur_write_short(savef, m->m_add_exp);
    ur_write_monster_stats(savef, &m->m_stats);
}

struct monster *
ur_read_monster(FILE *savef)
{
    struct monster *m;
    struct mstats *mstats;

    m = ur_alloc( sizeof(struct monster) );

    m->m_name = ur_read_string(savef);
    m->m_carry = ur_read_short(savef);
    m->m_normal = ur_read_int(savef);
    m->m_wander = ur_read_int(savef);
    m->m_appear = ur_read_char(savef);
    m->m_intel = ur_read_string(savef);
    ur_read(savef, &m->m_flags[0], 10*sizeof(long));
    m->m_typesum = ur_read_string(savef);
    m->m_numsum = ur_read_short(savef);
    m->m_add_exp = ur_read_short(savef);

    mstats = ur_read_monster_stats(savef);

    m->m_stats = *mstats;
    ur_free(mstats);

    return(m);
}

void
ur_write_trap(FILE *savef, struct trap *t)
{
    ur_write_long(savef, URS_TRAP);
    ur_write_coord(savef, t->tr_pos);
    ur_write_long(savef, t->tr_flags);
    ur_write_char(savef, t->tr_type);
    ur_write_char(savef, t->tr_show);
}

struct trap *
ur_read_trap(FILE *savef)
{
    struct trap *t;
    long id;

    id = ur_read_long(savef);

    assert(id == URS_TRAP);

    t = ur_alloc( sizeof(struct trap));

    t->tr_pos = ur_read_coord(savef);
    t->tr_flags = ur_read_long(savef);
    t->tr_type = ur_read_char(savef);
    t->tr_show = ur_read_char(savef);

    return(t);
}

void
ur_write_stats(FILE *savef, struct stats *s)
{
    ur_write_long(savef, URS_STATS);
    ur_write_string(savef, s->s_dmg);
    ur_write_long(savef, s->s_exp);
    ur_write_int(savef, s->s_hpt);
    ur_write_int(savef, s->s_pack);
    ur_write_int(savef, s->s_carry);
    ur_write_int(savef, s->s_lvl);
    ur_write_int(savef, s->s_arm);
    ur_write_int(savef, s->s_acmod);
    ur_write_int(savef, s->s_power);
    ur_write_int(savef, s->s_str);
    ur_write_int(savef, s->s_intel);
    ur_write_int(savef, s->s_wisdom);
    ur_write_int(savef, s->s_dext);
    ur_write_int(savef, s->s_const);
    ur_write_int(savef, s->s_charisma);
}

struct stats *
ur_read_stats(FILE *savef)
{
    struct stats *s;
    long id;

    id = ur_read_long(savef);

    assert(id == URS_STATS);

    s = ur_alloc(sizeof(struct stats));

    s->s_dmg = ur_read_string(savef);
    s->s_exp = ur_read_long(savef);
    s->s_hpt = ur_read_int(savef);
    s->s_pack = ur_read_int(savef);
    s->s_carry = ur_read_int(savef);
    s->s_lvl = ur_read_int(savef);
    s->s_arm = ur_read_int(savef);
    s->s_acmod = ur_read_int(savef);
    s->s_power = ur_read_int(savef);
    s->s_str = ur_read_int(savef);
    s->s_intel = ur_read_int(savef);
    s->s_wisdom = ur_read_int(savef);
    s->s_dext = ur_read_int(savef);
    s->s_const = ur_read_int(savef);
    s->s_charisma = ur_read_int(savef);

    return(s);
}

void
ur_write_thing(FILE *savef, struct thing *t)
{
    int i;

    ur_write_long(savef, URS_THING);
    ur_write_bag(savef, t->t_pack);
    ur_write_stats(savef, &t->t_stats);
    ur_write_stats(savef, &t->maxstats);
    ur_write_int(savef, t->t_ischasing);

    if (t->t_chasee == &player)
        ur_write_long(savef, -1L);
    else if (t->t_chasee == NULL)
         ur_write_long(savef, 0L);
    else
    {
        long m;
		
        m = find_thing_index(mlist, t->t_chasee);
		
        ur_write_long(savef,m);
    }

    ur_write_long(savef, find_list_index(lvl_obj, t->t_horde));
    ur_write_coord(savef, t->t_pos);
    ur_write_coord(savef, t->t_oldpos);
    ur_write_coord(savef, t->t_nxtpos);

    for(i = 0; i < 16; i++)
        ur_write_long(savef, t->t_flags[i]);

    ur_write_int(savef, t->t_praycnt);
    ur_write_int(savef, t->t_trans);
    ur_write_int(savef, t->t_turn);
    ur_write_int(savef, t->t_wasshot);
    ur_write_int(savef, t->t_ctype);
    ur_write_int(savef, t->t_index);
    ur_write_int(savef, t->t_no_move);
    ur_write_int(savef, t->t_rest_hpt);
    ur_write_int(savef, t->t_rest_pow);
    ur_write_int(savef, t->t_doorgoal);
    ur_write_char(savef, t->t_type);
    ur_write_char(savef, t->t_disguise);
    ur_write_char(savef, t->t_oldch);
}

struct thing *
ur_read_thing(FILE *savef)
{
    long id;
    int i;
    struct thing *t;
    struct stats *s;

    id = ur_read_long(savef);

    assert(id == URS_THING);

    t = ur_alloc( sizeof(struct thing) );

    t->t_pack = ur_read_bag(savef);

    s = ur_read_stats(savef);
    t->t_stats = *s;
    ur_free(s);

    s = ur_read_stats(savef);
    t->maxstats = *s;
    ur_free(s);

    t->t_ischasing = ur_read_int(savef);
    t->chasee_index = ur_read_long(savef);
    t->horde_index = ur_read_long(savef);
    t->t_pos = ur_read_coord(savef);
    t->t_oldpos = ur_read_coord(savef);
    t->t_nxtpos = ur_read_coord(savef);

    for(i = 0; i < 16; i++)
        t->t_flags[i] = ur_read_long(savef);

    t->t_praycnt = ur_read_int(savef);
    t->t_trans = ur_read_int(savef);
    t->t_turn = ur_read_int(savef);
    t->t_wasshot = ur_read_int(savef);
    t->t_ctype = ur_read_int(savef);
    t->t_index = ur_read_int(savef);
    t->t_no_move = ur_read_int(savef);
    t->t_rest_hpt = ur_read_int(savef);
    t->t_rest_pow = ur_read_int(savef);
    t->t_doorgoal = ur_read_int(savef);
    t->t_type = ur_read_char(savef);
    t->t_disguise = ur_read_char(savef);
    t->t_oldch = ur_read_char(savef);

    return(t);
}

void
ur_write_window(FILE *savef, WINDOW *win)
{
    int i,j;

    ur_write_long(savef, URS_WINDOW);

    ur_write_int(savef, win->_maxy);
    ur_write_int(savef, win->_maxx);

    for(i=0; i < win->_maxy; i++)
        for(j = 0; j < win->_maxx; j++)
            ur_write_ulong(savef, mvwinch(win,i,j));
}

void
ur_read_window(FILE *savef, WINDOW *win)
{
    int i,j;
    int maxy, maxx;
    long id;

    id = ur_read_long(savef);

    assert(id == URS_WINDOW);

    maxy = ur_read_int(savef);
    maxx = ur_read_int(savef);

    for(i=0; i < maxy; i++)
        for(j = 0; j < maxx; j++)
            mvwaddch(win,i,j,ur_read_long(savef));
}

void
ur_write_daemons(FILE *savef)
{
    int i;
    int id=0;

    ur_write_long(savef, URS_DAEMONS);

    for(i = 0; i < MAXDAEMONS; i++)
    {
        ur_write_int(savef, d_list[i].d_type );
        ur_write_int(savef, d_list[i].d_when );
        ur_write_int(savef, d_list[i].d_id);

        if (d_list[i].d_id == FUSE_UNSUMMON)
            id = find_thing_index(mlist, d_list[i].d_arg);

        if (d_list[i].d_id == DAEMON_DOCTOR)
            id = find_thing_index(mlist, d_list[i].d_arg);

        ur_write_int(savef, id);
        ur_write_int(savef, d_list[i].d_time );
    }
}

void
ur_read_daemons(FILE *savef)
{
    long id;
    int i;
    demoncnt = 0;
	
    id = ur_read_long(savef);

    assert(id == URS_DAEMONS);

    for(i = 0; i < MAXDAEMONS; i++)
    {
        d_list[i].d_type = ur_read_int(savef);
        d_list[i].d_when = ur_read_int(savef);
        d_list[i].d_id = ur_read_int(savef);
        id = ur_read_int(savef);
        d_list[i].d_time = ur_read_int(savef);

        if ((d_list[i].d_type != EMPTY) && (d_list[i].d_id == FUSE_UNSUMMON))
        {
            d_list[i].d_arg = find_thing(mlist,id);

            if (d_list[i].d_arg == NULL)
                d_list[i].d_type = EMPTY;
        }

        if ((d_list[i].d_type != EMPTY) &&  (d_list[i].d_id == DAEMON_DOCTOR) )
        {
            if (id == 0)
                d_list[i].d_arg = &player;
            else
                d_list[i].d_arg = find_thing(mlist, id);

            if (d_list[i].d_arg == NULL)
                d_list[i].d_type = EMPTY;
        }

        if (d_list[i].d_type != EMPTY)
            demoncnt++;
    }
}

void
save_file(FILE *savef)
{
    int i,weapon,armor,ring=0,room= -1,monster;

    ur_write_string(savef, save_format);

    ur_write_string(savef,"\nScroll Names\n");
    for(i = 0; i < MAXSCROLLS; i++)
        ur_write_string(savef,s_names[i]);

    ur_write_string(savef,"\nPotion Colors\n");
    for(i = 0; i < MAXPOTIONS; i++)
        ur_write_string(savef,p_colors[i]);

    ur_write_string(savef,"\nRing Stones\n");
    for(i = 0; i < MAXRINGS; i++)
        ur_write_string(savef,r_stones[i]);

    ur_write_string(savef,"\nStick types\n");
    for(i = 0; i < MAXSTICKS; i++)
        ur_write_string(savef,ws_made[i]);

    ur_write_string(savef,"\nStick types\n");
    for(i = 0; i < MAXSTICKS; i++)
        ur_write_string(savef,ws_type[i]);

    ur_write_string(savef, "\nTraps on this level\n");
    ur_write_int(savef, MAXTRAPS);
    for(i = 0; i < MAXTRAPS; i++)
        ur_write_trap(savef, &traps[i]);

    ur_write_string(savef,"\nRooms on this level\n");
    ur_write_int(savef, MAXROOMS);
    for(i = 0; i < MAXROOMS; i++)
    {
        ur_write_room(savef, &rooms[i]);

        if (&rooms[i] == oldrp)
            room = i;
    }
    ur_write_int(savef,room); /* save for recovery of oldrp */

    ur_write_string(savef,"\nThe Rogue\n");
    ur_write_thing(savef, &player);

    ur_write_string(savef,"\nObjects on this level\n");
    ur_write_bag(savef, lvl_obj);

    ur_write_string(savef,"\nRogue's Familiar, if any \n");
    ur_write_monsters(savef, fam_ptr);

    ur_write_string(savef,"\nMonsters on this level\n");
    ur_write_monsters(savef, mlist);

    monster = find_thing_index(mlist, beast);
    ur_write_int(savef, monster);

    ur_write_string(savef,"\nItems in use by rogue\n");
    weapon = find_list_index(player.t_pack, cur_weapon);
    armor  = find_list_index(player.t_pack, cur_armor);
    ur_write_int(savef, weapon);
    ur_write_int(savef, armor);

    for(i=0; i < 10; i++)
    {
        if (cur_ring[i] == NULL)
            ring = find_list_index(player.t_pack, cur_ring[i]);

        ur_write_int(savef, ring);
    }

    ur_write_string(savef,"\nActive Daemons and Fuses\n");
    ur_write_daemons(savef);

    ur_write_string(savef, "\nMisc\n");

    for(i = 0; i < 10; i++)
        ur_write_string(savef, msgbuf[i]);

    ur_write_int(savef, msg_index);
    ur_write_int(savef, foodlev);
    ur_write_int(savef, ntraps);
    ur_write_int(savef, dnum);
    ur_write_int(savef, max_level);
    ur_write_int(savef, lost_dext);
    ur_write_int(savef, no_command);
    ur_write_int(savef, level);
    ur_write_int(savef, see_dist);
    ur_write_int(savef, no_food);
    ur_write_int(savef, count);
    ur_write_int(savef, food_left);
    ur_write_int(savef, group);
    ur_write_int(savef, hungry_state);
    ur_write_int(savef, infest_dam);
    ur_write_int(savef, lost_str);
    ur_write_int(savef, hold_count);
    ur_write_int(savef, trap_tries);
    ur_write_int(savef, has_artifact);
    ur_write_int(savef, picked_artifact);
    ur_write_int(savef, luck);
    ur_write_int(savef, resurrect);
    ur_write_int(savef, fam_type);
    ur_write_int(savef, mons_summoned);
    ur_write_char(savef, PLAYER);
    ur_write_char(savef, take);
    ur_write_char(savef, runch);
    ur_write_int(savef, char_type);
    ur_write_int(savef, inv_type);
    ur_write_int(savef,  pool_teleport);
    ur_write_int(savef, inwhgt);
    ur_write_int(savef, after);
    ur_write_int(savef, waswizard);
    ur_write_int(savef, canwizard);
    ur_write_int(savef, playing);
    ur_write_int(savef, running);
    ur_write_int(savef, fighting);
    ur_write_int(savef, wizard);
    ur_write_int(savef, wiz_verbose);
    ur_write_int(savef, moving);
    ur_write_coord(savef, delta);
    ur_write_int(savef, levtype);
    ur_write_int(savef, purse);
    ur_write_int(savef, total);
    ur_write_window(savef, cw);
    ur_write_window(savef, hw);
    ur_write_window(savef, mw);
    ur_write_window(savef, stdscr);

    ur_write_string(savef,"\nGame Options\n");
    ur_write_int(savef, terse);
    ur_write_int(savef, door_stop);
    ur_write_int(savef, doorstop);
    ur_write_int(savef, jump);
    ur_write_int(savef, firstmove);
    ur_write_int(savef, askme);
    ur_write_string(savef,whoami);
    ur_write_string(savef,fruit);
    ur_write_string(savef,file_name);
    ur_write_string(savef,score_file);

    ur_write_string(savef,"\nEnd of UltraRogue Game State\n");
}

#define DUMPSTRING { str = ur_read_string(savef); /*printf("%s",str);*/ ur_free(str); }

int
restore_file(FILE *savef)
{
    int i,j;
    char *str;
    struct trap *t;
    struct room *r;
    struct thing *p;

    str = ur_read_string(savef);

    if (strcmp(str, save_format) != 0)
    {
        printf("Save Game Version: %s\n", str);
        printf("Real Game Version: %s\n", save_format);
        printf("Sorry, versions don't match.\n");
        return(FALSE);
    }

    DUMPSTRING
    for(i=0; i < MAXSCROLLS; i++)
        s_names[i] = ur_read_string(savef);

    DUMPSTRING
    for(i=0; i < MAXPOTIONS; i++)
        p_colors[i] = ur_read_string(savef);

    DUMPSTRING
    for(i=0; i < MAXRINGS; i++)
        r_stones[i] = ur_read_string(savef);

    DUMPSTRING
    for(i=0; i < MAXSTICKS; i++)
        ws_made[i] = ur_read_string(savef);

    DUMPSTRING
    for(i=0; i < MAXSTICKS; i++)
        ws_type[i] = ur_read_string(savef);

    DUMPSTRING
    i = ur_read_int(savef);
    assert(i == MAXTRAPS);

    for(i=0;i<MAXTRAPS;i++)
    {
        t = ur_read_trap(savef);
        traps[i] = *t;
        ur_free(t);
    }

    DUMPSTRING
    i = ur_read_int(savef);
    assert(i == MAXROOMS);

    for(i=0;i<MAXROOMS;i++)
    {
        r = ur_read_room(savef);
        rooms[i] = *r;
        ur_free(r);
    }
    i = ur_read_int(savef);
    oldrp = &rooms[i];

    DUMPSTRING
    p = ur_read_thing(savef);
    player = *p;
    ur_free(p);

    DUMPSTRING
    lvl_obj = ur_read_bag(savef);

    DUMPSTRING
    fam_ptr = ur_read_monsters(savef);

    DUMPSTRING
    mlist = ur_read_monsters(savef);
    i = ur_read_int(savef);
    beast = find_thing(mlist, i);

    ur_fixup_monsters(fam_ptr);
    ur_fixup_monsters(fam_ptr);

    DUMPSTRING
    i = ur_read_int(savef);
    cur_weapon = find_object(player.t_pack, i);

    i = ur_read_int(savef);
    cur_armor = find_object(player.t_pack, i);

    for(j=0; j < 10; j++)
    {
        i = ur_read_int(savef);
        if (i == -1)
            cur_ring[j] = NULL;
        else
            cur_ring[j] = find_object(player.t_pack, i);
    }

    DUMPSTRING
    ur_read_daemons(savef);

    DUMPSTRING
    for(i = 0; i < 10; i++)
    {
        str = ur_read_string(savef);
        strcpy(&msgbuf[i][0],str);
        ur_free(str);
    }

    msg_index = ur_read_int(savef);

    foodlev  = ur_read_int(savef);
    ntraps = ur_read_int(savef);
    dnum = ur_read_int(savef);
    max_level = ur_read_int(savef);
    lost_dext = ur_read_int(savef);
    no_command = ur_read_int(savef);
    level = ur_read_int(savef);
    see_dist  = ur_read_int(savef);
    no_food = ur_read_int(savef);
    count = ur_read_int(savef);
    food_left = ur_read_int(savef);
    group = ur_read_int(savef);
    hungry_state = ur_read_int(savef);
    infest_dam = ur_read_int(savef);
    lost_str = ur_read_int(savef);
    hold_count = ur_read_int(savef);
    trap_tries = ur_read_int(savef);
    has_artifact  = ur_read_int(savef);
    picked_artifact  = ur_read_int(savef);
    luck = ur_read_int(savef);
    resurrect = ur_read_int(savef);
    fam_type = ur_read_int(savef);
    mons_summoned = ur_read_int(savef);
    PLAYER = ur_read_char(savef);
    take = ur_read_char(savef);
    runch = ur_read_char(savef);
    char_type = ur_read_int(savef);
    inv_type = ur_read_int(savef);
    pool_teleport = ur_read_int(savef);
    inwhgt = ur_read_int(savef);
    after = ur_read_int(savef);
    waswizard = ur_read_int(savef);
    canwizard = ur_read_int(savef);
    playing = ur_read_int(savef);
    running = ur_read_int(savef);
    fighting = ur_read_int(savef);
    wizard = ur_read_int(savef);
    wiz_verbose = ur_read_int(savef);
    moving = ur_read_int(savef);
    delta = ur_read_coord(savef);
    levtype = ur_read_int(savef);
    purse = ur_read_int(savef);
    total = ur_read_int(savef);
    ur_read_window(savef, cw);
    ur_read_window(savef, hw);
    ur_read_window(savef, mw);
    ur_read_window(savef, stdscr);

    DUMPSTRING
    terse = ur_read_int(savef);
    door_stop = ur_read_int(savef);
    doorstop = ur_read_int(savef);
    jump = ur_read_int(savef);
    firstmove = ur_read_int(savef);
    askme = ur_read_int(savef);
    str = ur_read_string(savef);
    strcpy(whoami,str);
    ur_free(str);
    str = ur_read_string(savef);
    strcpy(fruit,str);
    ur_free(str);
    str = ur_read_string(savef);
    strcpy(file_name,str);
    ur_free(str);
    str = ur_read_string(savef);
    strcpy(score_file,str);
    ur_free(str);

    DUMPSTRING
    return(TRUE);
}


