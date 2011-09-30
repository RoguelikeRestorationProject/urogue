/*
    weapons.c - Functions for dealing with problems brought about by weapons
         
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

#include <ctype.h>
#include <string.h>
#include "rogue.h"

/*
    missile()
        Fire a missile in a given direction
*/

void
missile(int ydelta, int xdelta, struct linked_list *item, struct thing *tp)
{
    struct object   *obj;
    struct linked_list  *nitem;

    if (item == NULL)   /* Get which thing we are hurling */
        return;

    obj = OBJPTR(item);

    if (!dropcheck(obj) || is_current(obj))
        return;

    /*
     * Get rid of the thing. If it is a non-multiple item object, or if
     * it is the last thing, just drop it. Otherwise, create a new item
     * with a count of one.
    */

    if (obj->o_count < 2)
    {
        if (tp->t_pack == pack)
            rem_pack(obj);
        else
            detach(tp->t_pack, item);
    }
    else
    {
        obj->o_count--;
        nitem = (struct linked_list *) new_item(sizeof *obj);
        obj = OBJPTR(nitem);
        *obj = *(OBJPTR(item));
        obj->o_count = 1;
        item = nitem;
    }

    switch (obj->o_type)
    {
        case ARTIFACT:
            has_artifact &= ~(1 << obj->o_which);
            break;

        case SCROLL:
            if (obj->o_which == S_SCARE && obj->o_flags & ISBLESSED)
                obj->o_flags &= ~ISBLESSED;
            else
                obj->o_flags |= ISCURSED;
    }

    updpack();
    obj->o_pos = do_motion(obj->o_type, ydelta, xdelta, tp);

    /*
     * AHA! Here it has hit something. If it is a wall or a door, or if
     * it misses (combat) the monster, put it on the floor
     */

    if (!hit_monster(obj->o_pos.y, obj->o_pos.x, obj, tp))
    {
        if (obj->o_type == WEAPON && obj->o_which == GRENADE)
        {
            hearmsg("BOOOM!");
            aggravate();

            if (ntraps + 1 < 2 * MAXTRAPS &&
                fallpos(obj->o_pos, &traps[ntraps].tr_pos))
            {
                mvaddch(traps[ntraps].tr_pos.y, traps[ntraps].tr_pos.x,
                    TRAPDOOR);
                traps[ntraps].tr_type = TRAPDOOR;
                traps[ntraps].tr_flags = ISFOUND;
                traps[ntraps].tr_show = TRAPDOOR;
                ntraps++;
                light(&hero);
            }
            discard(item);
        }
        else if (obj->o_flags & ISLOST)
        {
            if (obj->o_type == WEAPON)
                addmsg("The %s", weaps[obj->o_which].w_name);
            else
                addmsg(inv_name(obj, LOWERCASE));

            msg(" vanishes in a puff of greasy smoke.");
            discard(item);
        }
        else
        {
            fall(&player, item, TRUE, TRUE);

            if (obj->o_flags & CANRETURN)
                msg("You have %s.", inv_name(obj, LOWERCASE));
        }
    }
    else if (obj->o_flags & ISOWNED)
    {
        add_pack(item, NOMESSAGE);
        msg("You have %s.", inv_name(obj, LOWERCASE));
    }

    mvwaddch(cw, hero.y, hero.x, PLAYER);
}

/*
    do_motion()
        do the actual motion on the screen done by an object
        traveling across the room
*/

coord
do_motion(int ob, int ydelta, int xdelta, struct thing *tp)
{
    coord pos;
    /* Come fly with us ... */

    pos = tp->t_pos;

    for (;;)
    {
        int ch;

        /* Erase the old one */

        if (!ce(pos, tp->t_pos) &&
            cansee(pos.y, pos.x) &&
            mvwinch(cw, pos.y, pos.x) != ' ')
        {
            mvwaddch(cw, pos.y, pos.x, show(pos.y, pos.x));
        }

        /* Get the new position */

        pos.y += ydelta;
        pos.x += xdelta;

        if (shoot_ok(ch = winat(pos.y, pos.x)) &&
            ch != DOOR && !ce(pos, hero))
        {
            /* It hasn't hit anything yet, so display it if it alright. */

            if (cansee(pos.y, pos.x) &&
                mvwinch(cw, pos.y, pos.x) != ' ')
            {
                mvwaddch(cw, pos.y, pos.x, ob);
                wrefresh(cw);
            }
			
            continue;

        }
        break;
    }

    return(pos);
}

/*
    fall()
        Drop an item someplace around here.
*/

void
fall(struct thing *tp, struct linked_list *item, int pr, int player_owned)
{
    struct object *obj;
    struct room   *rp;
    coord   fpos;

    obj = OBJPTR(item);
    rp = roomin(tp->t_pos);

    if (player_owned && obj->o_flags & CANRETURN)
    {
        add_pack(item, NOMESSAGE);
        msg("You have %s.", inv_name(obj, LOWERCASE));
        return;
    }
    else if (fallpos(obj->o_pos, &fpos))
    {
        if (obj->o_flags & CANBURN && obj->o_type == WEAPON
            && obj->o_which == MOLOTOV
            && ntraps + 1 < 2 * MAXTRAPS)
        {
            mvaddch(fpos.y, fpos.x, FIRETRAP);
            traps[ntraps].tr_type  = FIRETRAP;
            traps[ntraps].tr_flags = ISFOUND;
            traps[ntraps].tr_show  = FIRETRAP;
            traps[ntraps].tr_pos   = fpos;
            ntraps++;

            if (rp != NULL)
                rp->r_flags &= ~ISDARK;
        }
        else
        {
            obj->o_pos = fpos;
            add_obj(item, fpos.y, fpos.x);
        }

        if (rp != NULL &&
            (!(rp->r_flags & ISDARK) ||
             (rp->r_flags & HASFIRE)))
        {
            light(&hero);
            mvwaddch(cw, hero.y, hero.x, PLAYER);
        }
        return;
    }

    /* get here only if there isn't a place to put it */
	
    if (pr)
    {
        if (cansee(obj->o_pos.y, obj->o_pos.x))
        {
            if (obj->o_type == WEAPON)
                addmsg("The %s", weaps[obj->o_which].w_name);
            else
                addmsg(inv_name(obj, LOWERCASE));

            msg(" vanishes as it hits the ground.");
        }
    }
    discard(item);
}

/*
    init_weapon()
        Set up the initial goodies for a weapon
*/

void
init_weapon(struct object *weap, int type)
{
    struct init_weps *iwp = &weaps[type];

    weap->o_damage  = iwp->w_dam;
    weap->o_hurldmg = iwp->w_hrl;
    weap->o_launch  = iwp->w_launch;
    weap->o_flags   = iwp->w_flags;
    weap->o_weight  = iwp->w_wght;

    if (weap->o_flags & ISMANY)
    {
        weap->o_count = rnd(8) + 8; 
        weap->o_group = ++group;
    }
    else
        weap->o_count = 1;
}

/*
    hit_monster()
        does the missile hit the target?
*/

int
hit_monster(int y, int x, struct object *weapon, struct thing *thrower)
{
    struct linked_list *mon;
    coord target;

    target.y = y;
    target.x = x;

    if (thrower == &player)
        return(fight(&target, weapon, THROWN));

    if (ce(target, hero))
    {
        if (good_monster(*thrower))
        {
            if (on(*thrower, ISFAMILIAR))
                msg("Please get out of the way, Master!  I nearly hit you.");
            else
                msg("Get out of the way %s!", whoami);

            return(FALSE);
        }

        return(attack(thrower, weapon, THROWN));
    }

    if ((mon = find_mons(y, x)) != NULL)
        return(mon_mon_attack(thrower, mon, weapon, THROWN));
    else
        return(FALSE);
}


/*
    num()
        Figure out the plus number for armor/weapons
*/

char *
num(int n1, int n2, char *buf)
{
    if (buf == NULL)
        return("UltraRogue Error #104");

    if (n1 == 0 && n2 == 0)
    {
        strcpy(buf,"+0");
        return(buf);
    }

    if (n2 == 0)
        sprintf(buf, "%s%d", n1 < 0 ? "" : "+", n1);
    else
        sprintf(buf, "%s%d, %s%d", n1 < 0 ? "" : "+",
            n1, n2 < 0 ? "" : "+", n2);

    return(buf);
}

/*
    wield()
        Pull out a certain weapon
*/

void
wield(void)
{
    struct linked_list *item;
    struct object *obj, *oweapon;

    oweapon = cur_weapon;

    if (!dropcheck(cur_weapon))
    {
        cur_weapon = oweapon;
        return;
    }

    cur_weapon = oweapon;

    if ((item = get_item("wield", WEAPON)) == NULL)
    {
        after = FALSE;
        return;
    }

    obj = OBJPTR(item);

    if (is_current(obj))
    {
        after = FALSE;
        return;
    }

    wield_ok(&player, obj, TRUE);

    msg("You are now wielding %s.", inv_name(obj, LOWERCASE));

    cur_weapon = obj;
}

/*
    fallpos()
        pick a random position around the given (y, x) coordinates
*/

int
fallpos(coord pos, coord *newpos) /*ARGSUSED*/
{
    int   y, x, cnt;
	coord places[9];

    cnt = 0;

    /* look for all the places that qualify */

    for (y = pos.y - 1; y <= pos.y + 1; y++)
	{
        for (x = pos.x - 1; x <= pos.x + 1; x++)
        {
            switch(CCHAR(mvwinch(stdscr,y,x)))
            {
                case GOLD:
                case POTION:
                case SCROLL:
                case FOOD:
                case WEAPON:
                case ARMOR:
                case RING:
                case STICK:
                case FLOOR:
                case PASSAGE:
                case ARTIFACT:
                    places[cnt].y = y;
                    places[cnt].x = x;
                    cnt++;
            }
        }
    }
	
	/* now, pick one of the places, if there are any */

    if (cnt > 0) 
	{
        int which = rnd(cnt);

        newpos->y = places[which].y;
        newpos->x = places[which].x;

        debug("Dropping object at %d, %d", newpos->y, newpos->x);
    }
	
    return(cnt);
}

/*
    wield_ok()
        enforce player class weapons restrictions
*/

int
wield_ok(struct thing *wieldee, struct object *obj, int print_message)
{
    int ret_val = TRUE;
    int class_type = wieldee->t_ctype;

    if (obj->o_type != WEAPON)
    {
        ret_val = FALSE;
        return(ret_val);
    }
    else
        switch (class_type)
        {
            case C_MAGICIAN: /* need one hand free */
            case C_ILLUSION:
                if (obj->o_flags & ISTWOH)
                    ret_val = FALSE;
                break;

            case C_THIEF:    /* need portable weapon  */
            case C_ASSASIN:
            case C_NINJA:
                if (obj->o_flags & ISTWOH)
                    ret_val = FALSE;
                break;

            case C_CLERIC:   /* No sharp weapons */
                if (obj->o_flags & ISSHARP)
                    ret_val = FALSE;
                break;

            case C_DRUID:    /* No non-silver metal weapons */
                if (obj->o_flags & ISMETAL && !(obj->o_flags & ISSILVER))
                    ret_val = FALSE;
                break;

            case C_PALADIN:  /* must wield sharp stuff */
                if ((obj->o_flags & ISSHARP) == FALSE)
                    ret_val = FALSE;
                break;

            case C_FIGHTER:  /* wield anything */
            case C_RANGER:
            case C_MONSTER:
                break;

            default:  /* Unknown class */
                debug("Unknown class %d.", class_type);
                break;
        }

    if (itemweight(obj) > 18 * pstats.s_str)
    {
        if (wieldee == &player && print_message == TRUE)
            msg("That is too heavy for you to swing effectively!");

        ret_val = FALSE;
        return(ret_val);
    }

    if (ret_val == FALSE && print_message == MESSAGE)
        switch (class_type)
        {
            case C_MAGICIAN:
            case C_ILLUSION:
                msg("You'll find it hard to cast spells while wielding that!");
                break;

            case C_THIEF:
            case C_ASSASIN:
            case C_NINJA:
                msg("Don't expect to backstab anyone while wielding that!");
                break;

            case C_CLERIC:
            case C_DRUID:
            case C_PALADIN:
                msg("Your god strongly disapproves of your wielding that!");
                break;

            case C_FIGHTER:
            case C_RANGER:
            case C_MONSTER:
                break;
        }

    return(ret_val);
}

/*
    shoot_ok()
        returns true if it is ok for type to shoot over ch
*/

int
shoot_ok(int ch)
{
    switch(ch)
    {
        case ' ':
        case '|':
        case '-':
        case SECRETDOOR:
            return(FALSE);

        default:
            return(!isalpha(ch));
    }
}
