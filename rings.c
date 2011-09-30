/*
    rings.c - Routines dealing specificaly with rings
        
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

#include <string.h>
#include <stdlib.h>
#include "rogue.h"

void
ring_on(void)
{
    struct object   *obj;
    struct linked_list  *item;
    int ring;
    char buf[2 * LINELEN];

    if ((item = get_item("put on", RING)) == NULL)
        return;

    obj = OBJPTR(item);

    if (obj->o_type != RING)
    {
        msg("You can't put that on!");
        return;
    }

    /* find out which hand to put it on */

    if (is_current(obj))
    {
        msg("Already wearing that!");
        return;
    }

    if (cur_ring[LEFT_1] == NULL)
        ring = LEFT_1;
    else if (cur_ring[LEFT_2] == NULL)
        ring = LEFT_2;
    else if (cur_ring[LEFT_3] == NULL)
        ring = LEFT_3;
    else if (cur_ring[LEFT_4] == NULL)
        ring = LEFT_4;
    else if (cur_ring[LEFT_5] == NULL)
        ring = LEFT_5;
    else if (cur_ring[RIGHT_1] == NULL)
        ring = RIGHT_1;
    else if (cur_ring[RIGHT_2] == NULL)
        ring = RIGHT_2;
    else if (cur_ring[RIGHT_3] == NULL)
        ring = RIGHT_3;
    else if (cur_ring[RIGHT_4] == NULL)
        ring = RIGHT_4;
    else if (cur_ring[RIGHT_5] == NULL)
        ring = RIGHT_5;
    else
    {
        msg("You already have on ten rings.");
        return;
    }

    cur_ring[ring] = obj;

    /* Calculate the effect it has on the poor guy. */

    switch (obj->o_which)
    {
        case R_ADDSTR:
            pstats.s_str += obj->o_ac;
            break;
        case R_ADDHIT:
            pstats.s_dext += obj->o_ac;
            break;
        case R_ADDINTEL:
            pstats.s_intel += obj->o_ac;
            break;
        case R_ADDWISDOM:
            pstats.s_wisdom += obj->o_ac;
            break;
        case R_FREEDOM:
            turn_off(player, ISHELD);
            hold_count = 0;
            break;
        case R_TRUESEE:
            if (off(player, PERMBLIND))
            {
                turn_on(player, CANTRUESEE);
                msg("You become more aware of your surroundings.");
                sight(NULL);
                light(&hero);
                mvwaddch(cw, hero.y, hero.x, PLAYER);
            }
            break;

        case R_SEEINVIS:
            if (off(player, PERMBLIND))
            {
                turn_on(player, CANTRUESEE);
                msg("Your eyes begin to tingle.");
                sight(NULL);
                light(&hero);
                mvwaddch(cw, hero.y, hero.x, PLAYER);
            }
            break;

        case R_AGGR:
            aggravate();
            break;

        case R_CARRYING:
            updpack();
            break;

        case R_LEVITATION:
            msg("You begin to float in the air!");
            break;

        case R_LIGHT:
            if (roomin(hero) != NULL)
            {
                light(&hero);
                mvwaddch(cw, hero.y, hero.x, PLAYER);
            }
    }

    status(FALSE);

    if (know_items[TYP_RING][obj->o_which] &&
        guess_items[TYP_RING][obj->o_which])
    {
        mem_free(guess_items[TYP_RING][obj->o_which]);
        guess_items[TYP_RING][obj->o_which] = NULL;
    }
    else if (!know_items[TYP_RING][obj->o_which] &&
         askme &&
         (obj->o_flags & ISKNOW) == 0 &&
         guess_items[TYP_RING][obj->o_which] == NULL)
    {
        mpos = 0;
        msg("What do you want to call it? ");

        if (get_string(buf, cw) == NORM)
        {
            guess_items[TYP_RING][obj->o_which] =
                new_alloc(strlen(buf) + 1);
            strcpy(guess_items[TYP_RING][obj->o_which], buf);
        }
        msg("");
    }
}

void
ring_off(void)
{
    struct object   *obj;
    struct linked_list  *item;

    if (cur_ring[LEFT_1] == NULL && cur_ring[LEFT_2] == NULL &&
        cur_ring[LEFT_3] == NULL && cur_ring[LEFT_4] == NULL &&
        cur_ring[LEFT_5] == NULL &&
        cur_ring[RIGHT_1] == NULL && cur_ring[RIGHT_2] == NULL &&
        cur_ring[RIGHT_3] == NULL && cur_ring[RIGHT_4] == NULL &&
        cur_ring[RIGHT_5] == NULL)
    {
        msg("You aren't wearing any rings.");
        return;
    }
    else if ((item = get_item("remove", RING)) == NULL)
        return;

    mpos = 0;
    obj = OBJPTR(item);

    if ((obj = OBJPTR(item)) == NULL)
        msg("You are not wearing that!");

    if (dropcheck(obj))
    {
        switch (obj->o_which)
        {
            case R_SEEINVIS:
                msg("Your eyes stop tingling.");
                break;

            case R_CARRYING:
                updpack();
                break;

            case R_LEVITATION:
                msg("You float gently to the ground.");
                break;

            case R_LIGHT:
                if (roomin(hero) != NULL)
                {
                    light(&hero);
                    mvwaddch(cw, hero.y, hero.x, PLAYER);
                }
                break;

            case R_TRUESEE:
                msg("Your sensory perceptions return to normal.");
                break;
        }

        msg("Was wearing %s.", inv_name(obj, LOWERCASE));
    }
}

/*
    ring_eat()
        how much food does this ring use up?
*/

int
ring_eat(int hand)
{
    int ret_val = 0;
    int ac;

    if (cur_ring[hand] != NULL)
    {
        switch (cur_ring[hand]->o_which)
        {
            case R_REGEN:
            case R_VREGEN:
                ret_val = rnd(pstats.s_lvl > 10 ? 10 : pstats.s_lvl);

            case R_DIGEST:

                ac = cur_ring[hand]->o_ac;

                if (ac < 0 && rnd(1 - (ac / 3)) == 0)
                    ret_val = -ac + 1;
                else if (rnd((ac / 2) + 2) == 0)
                    ret_val = -1 - ac;
                break;

            case R_SEARCH:
                ret_val = rnd(100) < 33;
                break;
				
			default:
			    ret_val = 1;
        }
    }

    ret_val += rnd(luck);

    return(ret_val);
}

/*
    ring_num()
        print ring bonuses
*/

char *
ring_num(struct object *obj, char *buf)
{
    char buffer[1024];

    if (buf == NULL)
        return("A bug in UltraRogue #101");

    buf[0] = 0;

    if (obj->o_flags & ISKNOW)
    {
        switch (obj->o_which)
        {
            case R_SEARCH:
            case R_PROTECT:
            case R_ADDSTR:
            case R_ADDDAM:
            case R_ADDHIT:
            case R_ADDINTEL:
            case R_ADDWISDOM:
            case R_CARRYING:
            case R_VREGEN:
            case R_RESURRECT:
            case R_TELCONTROL:
            case R_REGEN:
            case R_PIETY:
            case R_WIZARD:
                buf[0] = ' ';
                strcpy(&buf[1], num(obj->o_ac, 0,buffer));
                break;

            case R_DIGEST:
                buf[0] = ' ';
                strcpy(&buf[1], num(obj->o_ac < 0 ?
                obj->o_ac : obj->o_ac - 1, 0, buffer));
                break;

            default:
                if (obj->o_flags & ISCURSED)
                    strcpy(buf, " cursed");
                break;
        }
    }

    return(buf);
}

/*
    ring_value()
        Return the effect of the specified ring
*/

#define ISRING(h, r) (cur_ring[h] != NULL && cur_ring[h]->o_which == r)

int
ring_value(int type)
{
    int result = 0;

    if (ISRING(LEFT_1, type))
        result += cur_ring[LEFT_1]->o_ac;
    if (ISRING(LEFT_2, type))
        result += cur_ring[LEFT_2]->o_ac;
    if (ISRING(LEFT_3, type))
        result += cur_ring[LEFT_3]->o_ac;
    if (ISRING(LEFT_4, type))
        result += cur_ring[LEFT_4]->o_ac;
    if (ISRING(LEFT_5, type))
        result += cur_ring[LEFT_5]->o_ac;
    if (ISRING(RIGHT_1, type))
        result += cur_ring[RIGHT_1]->o_ac;
    if (ISRING(RIGHT_2, type))
        result += cur_ring[RIGHT_2]->o_ac;
    if (ISRING(RIGHT_3, type))
        result += cur_ring[RIGHT_3]->o_ac;
    if (ISRING(RIGHT_4, type))
        result += cur_ring[RIGHT_4]->o_ac;
    if (ISRING(RIGHT_5, type))
        result += cur_ring[RIGHT_5]->o_ac;

    return(result);
}
