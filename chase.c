/*
    chase.c  -  Code for one creature to chase another
 
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

#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include "rogue.h"

/*
    do_chase()
        Make one thing chase another.
*/

void
do_chase(struct thing *th, int flee)
{
    struct room    *rer;        /* room of chaser */
    struct room    *ree;        /* room of chasee */
    struct room    *old_room;   /* old room of monster */
    struct room    *new_room;   /* new room of monster */

    int i, mindist = INT_MAX, maxdist = INT_MIN, dist = INT_MIN;

    int last_door = -1;     /* Door we just came from */
    int stoprun = FALSE;    /* TRUE means we are there */
    int rundoor;            /* TRUE means run to a door */
    int hit_bad = FALSE;    /* TRUE means hit bad monster */
    int mon_attack;         /* TRUE means find a monster to hit */

    char    sch;
    struct linked_list *item;
    coord   this;           /* Temporary destination for chaser */

    if (!th->t_ischasing)
        return;

    /* Make sure the monster can move */

    if (th->t_no_move != 0)
    {
        th->t_no_move--;
        return;
    }

    /*
     * Bad monsters check for a good monster to hit, friendly monsters
     * check for a bad monster to hit.
     */

    mon_attack = FALSE;

    if (good_monster(*th))
    {
        hit_bad = TRUE;
        mon_attack = TRUE;
    }
    else if (on(*th, ISMEAN))
    {
        hit_bad = FALSE;
        mon_attack = TRUE;
    }

    if (mon_attack)
    {
        struct linked_list  *mon_to_hit;

	mon_to_hit = f_mons_a(th->t_pos.y, th->t_pos.x, hit_bad);

        if (mon_to_hit)
        {
            mon_mon_attack(th, mon_to_hit, pick_weap(th), NOTHROWN);
            return;
        }
    }
	
    /* no nearby monster to hit */
	
    rer = roomin(th->t_pos);            /* Find room of chaser */
    ree = roomin(th->t_chasee->t_pos);  /* Find room of chasee */

    /*
     * We don't count doors as inside rooms for this routine
     */

    if (mvwinch(stdscr, th->t_pos.y, th->t_pos.x) == DOOR)
        rer = NULL;

    this = th->t_chasee->t_pos;

    /*
     * If we are not in a corridor and not a phasing monster, then if we
     * are running after the player, we run to a door if he is not in the
     * same room. If we are fleeing, we run to a door if he IS in the
     * same room.  Note:  We don't bother with doors in mazes. Phasing 
     * monsters don't need to look for doors. There are no doors in mazes
     * and throne rooms. 
     */

    if (levtype != MAZELEV && levtype != THRONE && rer != NULL && off(*th, CANINWALL))
    {
        if (flee)
            rundoor = (rer == ree);
        else
            rundoor = (rer != ree);
    }
    else
        rundoor = FALSE;

    if (rundoor)
    {
        coord   d_exit;   /* A particular door */
        int exity, exitx;   /* Door's coordinates */

        if (th->t_doorgoal != -1)
        { /* Do we already have the goal? */
            this = rer->r_exit[th->t_doorgoal];
            dist = 0;   /* Indicate that we have our door */
        }
        else
            for (i = 0; i < rer->r_nexits; i++)
            {   /* Loop through doors */
                d_exit = rer->r_exit[i];
                exity = d_exit.y;
                exitx = d_exit.x;

                /* Avoid secret doors */
                if (mvwinch(stdscr, exity, exitx) == DOOR)
                {
                    /* Were we just on this door? */
                    if (ce(d_exit, th->t_oldpos))
                        last_door = i;
                    else
                    {
                        dist = DISTANCE(th->t_chasee->t_pos, d_exit);

                        /*
                         * If fleeing, we want to
                         * maximize distance from
                         * door to what we flee, and
                         * minimize distance from
                         * door to us.
                         */

                        if (flee)
                            dist-=DISTANCE(th->t_pos,d_exit);

                        /*
                         * Maximize distance if
                         * fleeing, otherwise
                         * minimize it
                         */

                        if ((flee && (dist > maxdist)) ||
                            (!flee && (dist < mindist)))
                        {
                            th->t_doorgoal = i; /* Use this door */
                            this = d_exit;
                            mindist = maxdist = dist;
                        }
                    }
                }
            }

        /* Could we not find a door? */
        if (dist == INT_MIN)
        {
            /* If we were on a door, go ahead and use it */
            if (last_door != -1)
            {
                th->t_doorgoal = last_door;
                this = th->t_oldpos;
                dist = 0;   /* Indicate that we found a door */
            }
        }

        /* Indicate that we do not want to flee from the door */
        if (dist != INT_MIN)
            flee = FALSE;
    }
    else
        th->t_doorgoal = -1;    /* Not going to any door */

    /*
     * this now contains what we want to run to this time so we run to
     * it.  If we hit it we either want to fight it or stop running
     */

    if (!chase(th, &this, flee))
    {
        if (ce(th->t_nxtpos, hero))
        {
            /* merchants try to sell something */

            if (on(*th, CANSELL))
            {
                sell(th);
                return;
            }
            else if (off(*th, ISFRIENDLY) && off(*th, ISCHARMED)
                    && (off(*th, CANFLY) || (on(*th, CANFLY) && rnd(2))))
                    attack(th, pick_weap(th), FALSE);
                return;
        }
        else if (on(*th, NOMOVE))
            stoprun = TRUE;
    }

    if (!curr_mons)
        return;     /* Did monster get itself killed? */

    if (on(*th, NOMOVE))
        return;

    /* If we have a scavenger, it can pick something up */

    if ((item = find_obj(th->t_nxtpos.y, th->t_nxtpos.x)) != NULL)
    {
		struct linked_list *node, *top = item;
        struct object *obt;
		
		while(top)
		{
			/* grab all objects that qualify */
			
			struct object *obj = OBJPTR(item);
			
			obt = OBJPTR(top);
			node = obt->next_obj;
			
			if (on(*th, ISSCAVENGE) ||
                ((on(*th, CANWIELD) || on(*th, CANSHOOT)) &&
                (obj->o_type == WEAPON || obj->o_type == ARMOR)) ||
                (on(*th, CANCAST) && is_magic(obj))) 
			{
                rem_obj(top, FALSE);
                attach(th->t_pack, top);
            }
			
			top = node;
		}
		
		light(&hero);
    }

    mvwaddch(cw, th->t_pos.y, th->t_pos.x, th->t_oldch);
    sch = CCHAR( mvwinch(cw, th->t_nxtpos.y, th->t_nxtpos.x) );

    /* Get old and new room of monster */
    old_room = roomin(th->t_pos);
    new_room = roomin(th->t_nxtpos);

    /* If the monster can illuminate rooms, check for a change */
    if (on(*th, HASFIRE))
    {
        /* Is monster entering a room? */
        if (old_room != new_room && new_room != NULL)
        {
            new_room->r_flags |= HASFIRE;
            new_room->r_fires++;
            if (cansee(th->t_nxtpos.y, th->t_nxtpos.x) && new_room->r_fires==1)
                light(&hero);
        }

        /* Is monster leaving a room? */
        if (old_room != new_room && old_room != NULL)
        {
            if (--(old_room->r_fires) <= 0)
            {
                old_room->r_flags &= ~HASFIRE;
                if (cansee(th->t_pos.y, th->t_pos.x))
                    light(&th->t_pos);
            }
        }
    }

    /*
     * If monster is entering player's room and player can see it, stop
     * the player's running.
     */

    if (new_room != old_room && new_room != NULL &&
        new_room == ree && cansee(th->t_nxtpos.y, th->t_nxtpos.x) &&
        (off(*th, ISINVIS) || (off(*th, ISSHADOW) || rnd(10) == 0) ||
         on(player, CANSEE)) && off(*th, CANSURPRISE))
        running = FALSE;

    if (rer != NULL && (rer->r_flags & ISDARK) &&
        !(rer->r_flags & HASFIRE) && sch == FLOOR &&
         DISTANCE(th->t_nxtpos, th->t_pos) < see_dist &&
        off(player, ISBLIND))
        th->t_oldch = ' ';
    else
        th->t_oldch = sch;

    if (cansee(th->t_nxtpos.y, th->t_nxtpos.x) &&
      off(*th, ISINWALL) &&
      ((off(*th, ISINVIS) && (off(*th, ISSHADOW) || rnd(100) < 10)) ||
      on(player, CANSEE)) &&
      off(*th, CANSURPRISE))
        mvwaddch(cw, th->t_nxtpos.y, th->t_nxtpos.x, th->t_type);

    mvwaddch(mw, th->t_pos.y, th->t_pos.x, ' ');
    mvwaddch(mw, th->t_nxtpos.y, th->t_nxtpos.x, th->t_type);

    /* Record monster's last position (if new one is different) */

    if (!ce(th->t_nxtpos, th->t_pos))
        th->t_oldpos = th->t_pos;

    th->t_pos = th->t_nxtpos; /* Mark the monster's new position */

    /* If the monster is on a trap, trap it */

    sch = CCHAR(mvinch(th->t_nxtpos.y, th->t_nxtpos.x));

    if (isatrap(sch))
    {
        debug("Monster trapped by %c.", sch);

        if (cansee(th->t_nxtpos.y, th->t_nxtpos.x))
            th->t_oldch = sch;

        be_trapped(th, th->t_nxtpos);
    }

    /* And stop running if need be */

    if (stoprun && ce(th->t_pos, th->t_chasee->t_pos))
    {
        th->t_ischasing = FALSE;
        turn_off(*th, ISRUN);
    }
}

/*
    chase_it()
        Set a monster running after something or stop it from running (for
        when it dies)
*/

void
chase_it(coord *runner, struct thing *th)
{
    struct linked_list  *item;
    struct thing    *tp;

    /* If we couldn't find him, something is funny */

    if ((item = find_mons(runner->y, runner->x)) == NULL)
    {
        debug("CHASER '%s'", unctrl(winat(runner->y, runner->x)));
        return;
    }

    tp = THINGPTR(item);

    /* Start the beastie running */

    tp->t_ischasing = TRUE;
    tp->t_chasee    = th;

    turn_on(*tp, ISRUN);
    turn_off(*tp, ISDISGUISE);

    return;
}

/*
    chase()
        Find the spot for the chaser(er) to move closer to the chasee(ee).
        Returns TRUE if we want to keep on chasing later, FALSE if we reach the
        goal.
*/

int
chase(struct thing *tp, coord *ee, int flee)
{
    int x, y;
    int dist, thisdist, monst_dist = INT_MAX;
    struct linked_list  *weapon;
    coord   *er = &tp->t_pos;
    coord shoot;
    coord *shootit_dir = NULL;
    int ch;
    char   mch;
    int    next_player = FALSE;

    /* Take care of shooting directions */

    if (on(*tp, CANBREATHE) || on(*tp, CANSHOOT) || on(*tp, CANCAST))
    {
        if (good_monster(*tp))
        {
            shootit_dir = find_shoot(tp, &shoot); /* find a mean monster */

            if (wizard && shootit_dir)
                msg("Found monster to attack towards (%d,%d).",
                    shootit_dir->x, shootit_dir->y);
        }
        else
            shootit_dir = can_shoot(er, ee, &shoot);  /* shoot hero */
    }

    /*
     * If the thing is confused, let it move randomly. Some monsters are
     * slightly confused all of the time.
     */

    if ((on(*tp, ISHUH) && rnd(10) < 8) ||
        ((on(*tp, ISINVIS) || on(*tp, ISSHADOW)) && rnd(100) < 20) ||
        (on(player, ISINVIS) && off(*tp, CANSEE)))
    {   /* Player is invisible */

        /* get a valid random move */

        tp->t_nxtpos = rndmove(tp);

        dist = DISTANCE(tp->t_nxtpos, *ee);

        if (on(*tp, ISHUH) && rnd(20) == 0) /* monster might lose confusion */
            turn_off(*tp, ISHUH);

        /*
         * check to see if random move takes creature away from
         * player if it does then turn off ISHELD
         */

        if (dist > 1 && on(*tp, DIDHOLD))
        {
            turn_off(*tp, DIDHOLD);
            turn_on(*tp, CANHOLD);

            if (--hold_count == 0)
                turn_off(player, ISHELD);
        }
    } /* If we can breathe, we may do so */
    else if (on(*tp, CANBREATHE) && (shootit_dir) && (rnd(100) < 67) &&
         (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6)) &&
         (DISTANCE(*er, *ee) < BOLT_LENGTH * BOLT_LENGTH))
    {
        int   chance;
        char    *breath;

        /* Will it breathe at random */

        if (on(*tp, CANBRANDOM))
        {
            if (rnd(level / 20) == 0 && tp->t_index != nummonst + 1
                && !(good_monster(*tp)))
                turn_off(*tp, CANBRANDOM);

            /* Select type of breath */

            chance = rnd(100);

            if (chance < 11)
                breath = "acid";
            else if (chance < 22)
                breath = "flame";
            else if (chance < 33)
                breath = "lightning bolt";
            else if (chance < 44)
                breath = "chlorine gas";
            else if (chance < 55)
                breath = "ice";
            else if (chance < 66)
                breath = "nerve gas";
            else if (chance < 77)
                breath = "sleeping gas";
            else if (chance < 88)
                breath = "slow gas";
            else
                breath = "fear gas";
        } /* Or can it breathe acid? */
        else if (on(*tp, CANBACID))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBACID);

            breath = "acid";
        } /* Or can it breathe fire */
        else if (on(*tp, CANBFIRE))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBFIRE);

            breath = "flame";
        } /* Or can it breathe electricity? */
        else if (on(*tp, CANBBOLT))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBBOLT);

            breath = "lightning bolt";
        } /* Or can it breathe gas? */
        else if (on(*tp, CANBGAS))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBGAS);

            breath = "chlorine gas";
        } /* Or can it breathe ice? */
        else if (on(*tp, CANBICE))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBICE);

            breath = "ice";
        }
        else if (on(*tp, CANBPGAS))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBPGAS);

            breath = "nerve gas";
        }
        else if (on(*tp, CANBSGAS))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBSGAS);

            breath = "sleeping gas";
        }
        else if (on(*tp, CANBSLGAS))
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBSLGAS);

            breath = "slow gas";
        }
        else
        {
            if (!good_monster(*tp) && rnd(level / 15) == 0)
                turn_off(*tp, CANBFGAS);

            breath = "fear gas";
        }

        shoot_bolt(tp, *er, *shootit_dir, (tp == THINGPTR(fam_ptr)),
               tp->t_index, breath, roll(tp->t_stats.s_lvl, 6));

        tp->t_nxtpos = *er;

        dist = DISTANCE(tp->t_nxtpos, *ee);

        if (!curr_mons)
            return (TRUE);
    }
    else if (shootit_dir && on(*tp, CANCAST) &&
         (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6)))
    {
        /*
            If we can cast spells we might do so - even if adjacent fleeing
            monsters are restricted to certain spells
        */

        incant(tp, *shootit_dir);
        tp->t_nxtpos = *er;
        dist = DISTANCE(tp->t_nxtpos, *ee);
    }
    else if (shootit_dir && on(*tp, CANSHOOT)) 
    {
	weapon = get_hurl(tp);
	
	if (weapon &&
         (off(*tp, ISFLEE) || rnd(DISTANCE(*er, *ee)) > 2) &&
         (off(player, ISDISGUISE) || (rnd(tp->t_stats.s_lvl) > 6)))
	{
	    /*
	        Should we shoot or throw something? fleeing monsters 
		may to shoot anyway if far enough away
	    */

	    missile(shootit_dir->y, shootit_dir->x, weapon, tp);
	    tp->t_nxtpos = *er;
	    dist = DISTANCE(tp->t_nxtpos, *ee);
	}
    }
    else
    {
        /*
            Otherwise, find the empty spot next to the chaser that is closest
            to the chasee.
        */
        int ey, ex;
        struct room *rer, *ree;
        int dist_to_old = INT_MIN;   /* Dist from goal to old position */

        /* Get rooms */
        rer = roomin(*er);
        ree = roomin(*ee);

        /*
         * This will eventually hold where we move to get closer. If
         * we can't find an empty spot, we stay where we are.
         */

        dist = flee ? 0 : INT_MAX;
        tp->t_nxtpos = *er;

        /* Are we at our goal already? */

        if (!flee && ce(tp->t_nxtpos, *ee))
            return (FALSE);

        ey = er->y + 1;
        ex = er->x + 1;

        for (x = er->x - 1; x <= ex; x++)
            for (y = er->y - 1; y <= ey; y++)
            {
                coord   tryp; /* test position */

                /* Don't try off the screen */

                if ((x < 0) || (x >= COLS) || (y < 1) || (y >= LINES - 2))
                    continue;

                /*
                 * Don't try the player if not going after
                 * the player or he's disguised and monster is dumb
                 */

                if (((off(*tp, ISFLEE) && !ce(hero, *ee)) ||
                     (on(player, ISDISGUISE) && (rnd(tp->t_stats.s_lvl) < 6))
                     || good_monster(*tp))
                    && x == hero.x && y == hero.y)
                    continue;

                tryp.x = x;
                tryp.y = y;

                /*
                 * Is there a monster on this spot closer to
                 * our goal? Don't look in our spot or where
                 * we were.
                 */

                if (!ce(tryp, *er) && !ce(tryp, tp->t_oldpos) &&
                    isalpha( (mch = CCHAR(mvwinch(mw, y, x))) ) )
                {
                    int test_dist;

                    test_dist = DISTANCE(tryp,*ee);
                    if (test_dist <= 25 &&  /* Let's be fairly close */
                        test_dist < monst_dist)
                    {

                        /* Could we really move there? */

                        mvwaddch(mw, y, x, ' '); /* Temp blank monst */

                        if (diag_ok(er, &tryp, tp))
                            monst_dist = test_dist;

                        mvwaddch(mw, y, x, mch);    /* Restore monster */
                    }
                }

                if (!diag_ok(er, &tryp, tp))
                    continue;

                ch = mvwinch(cw, y, x); /* Screen character */

                /*
                 * Stepping on player is NOT okay if we are
                 * fleeing
                 */

                if (on(*tp, ISFLEE) && (ch == PLAYER))
				    next_player = TRUE;
					
                if (step_ok(y, x, NOMONST, tp) &&
                    (off(*tp, ISFLEE) || ch != PLAYER))
                {

                    /*
                     * If it is a trap, an intelligent
                     * monster may not step on it (unless
                     * our hero is on top!)
                     */

                    if (isatrap(ch))
                    {
                        if (!(ch == RUSTTRAP) &&
                            !(ch == FIRETRAP && on(*tp, NOFIRE)) &&
                            rnd(10) < tp->t_stats.s_intel &&
                        (y != hero.y || x != hero.x))
                            continue;
                    }

                    /*
                     * OK -- this place counts
                     */

                    thisdist = DISTANCE(tryp, *ee);
					
                    /*
                     * Adjust distance if we are being
                     * shot at to moving out of line of sight.
                     */

                    if (tp->t_wasshot && tp->t_stats.s_intel > 5 &&
                        ce(hero, *ee))
                    {
                        /* Move out of line of sight */
                        if (straight_shot(tryp.y, tryp.x, ee->y, ee->x, NULL))
                        {
                            if (flee)
                                thisdist -= SHOTPENALTY;
                            else
                                thisdist += SHOTPENALTY;
                        }

                        /*
                         * But do we want to leave
                         * the room?
                         */
                        else if (rer && rer == ree && ch == DOOR)
                            thisdist += DOORPENALTY;
                    }

                    /*
                     * Don't move to the last position if
                     * we can help it
                     */

                    if (ce(tryp, tp->t_oldpos))
                        dist_to_old = thisdist;
                    else if ((flee && (thisdist > dist)) ||
                         (!flee && (thisdist < dist)))
                    {
                        tp->t_nxtpos = tryp;
                        dist = thisdist;
                    }
                }
            }

        /*
         * If we are running from the player and he is in our way, go
         * ahead and slug him.
         */

        if (next_player && DISTANCE(*er,*ee) < dist &&
            step_ok(tp->t_chasee->t_pos.y, tp->t_chasee->t_pos.x, NOMONST, tp))
        {
            tp->t_nxtpos = tp->t_chasee->t_pos;    /* Okay to hit player */
            return(FALSE);
        }


        /*
         * If we can't get closer to the player (if that's our goal)
         * because other monsters are in the way, just stay put
         */

        if (!flee && ce(hero, *ee) && monst_dist < INT_MAX &&
            DISTANCE(*er, hero) < dist)
            tp->t_nxtpos = *er;

        /* Do we want to go back to the last position? */
        else if (dist_to_old != INT_MIN &&   /* It is possible to move back */
             ((flee && dist == 0) ||        /* No other possible moves */
              (!flee && dist == INT_MAX)))
        {
            /* Do we move back or just stay put (default)? */

            dist = DISTANCE(*er,*ee); /* Current distance */

            if (!flee || (flee && (dist_to_old > dist)))
                tp->t_nxtpos = tp->t_oldpos;
        }
    }

    /* Make sure we have the real distance now */
    dist = DISTANCE(tp->t_nxtpos, *ee);

    /* Mark monsters in a wall */

    switch(mvinch(tp->t_nxtpos.y, tp->t_nxtpos.x))
    {
        case WALL:
        case '-':
        case '|':
            turn_on(*tp, ISINWALL);
            break;
        default:
            turn_off(*tp, ISINWALL);
    }

    if (off(*tp, ISFLEE) &&
        !(!SAME_POS((tp->t_chasee->t_pos),hero) || off(player, ISINWALL) || on(*tp, CANINWALL)))
        return(dist != 0);
    else /* May actually hit here from a confused move */
        return(!ce(tp->t_nxtpos, hero));
}

/*
    roomin(coord *cp)

        Find what room some coordinates are in.
        NULL means they aren't in any room.
*/

struct room *
roomin(coord cp)
{
    struct room *rp;
    int i;

    for (i = 0; i < MAXROOMS; i++)
    {
        rp = &rooms[i];

        if ((cp.x <= (rp->r_pos.x + (rp->r_max.x - 1))) &&
            (cp.y <= (rp->r_pos.y + (rp->r_max.y - 1))) &&
            (cp.x >= rp->r_pos.x)                       &&
            (cp.y >= rp->r_pos.y))
        {
            return(rp);
        }
    }

    return(NULL);
}

/*
 * find_mons: Find the monster from his corrdinates
 */

struct linked_list  *
find_mons(int y, int x)
{
    struct linked_list  *item;

    for (item = mlist; item != NULL; item = next(item))
    {
        struct thing *th = THINGPTR(item);

        if (th->t_pos.y == y && th->t_pos.x == x)
            return item;
    }
    return NULL;
}

/*
 * Find an unfriendly monster around us to hit
 */

struct linked_list  *
f_mons_a(int y, int x, int hit_bad)
{
    int row, col;
    struct linked_list  *item;
    struct thing    *tp;

    for (row = x - 1; row <= x + 1; row++)
        for (col = y - 1; col <= y + 1; col++)
            if (row == x && col == y)
                continue;
            else if (col > 0 && row > 0 &&
                isalpha(mvwinch(mw, col, row)) &&
                 ((item = find_mons(col, row)) != NULL))
            {
                tp = THINGPTR(item);
                if ((good_monster(*tp) && !hit_bad) ||
                    (!good_monster(*tp) && hit_bad))
                    return (item);
            }

    return (NULL);
}


/*
    diag_ok()
        Check to see if the move is legal if it is diagonal
*/

int
diag_ok(coord *sp, coord *ep, struct thing *flgptr)
{
    if (ep->x == sp->x || ep->y == sp->y)
        return TRUE;

    return (step_ok(ep->y, sp->x, MONSTOK, flgptr) &&
        step_ok(sp->y, ep->x, MONSTOK, flgptr));
}

/*
    cansee()
        returns true if the hero can see a certain coordinate.
*/

int
cansee(int y, int x)
{
    struct room *rer;
    coord   tp;

    if (on(player, ISBLIND))
        return FALSE;

    tp.y = y;
    tp.x = x;
    rer = roomin(tp);

    /*
     * We can only see if the hero in the same room as the coordinate and
     * the room is lit or if it is close.
     */

    return ((rer != NULL &&
         rer == roomin(hero) &&
         (!(rer->r_flags & ISDARK) || (rer->r_flags & HASFIRE)) &&
         (levtype != MAZELEV || /* Maze level needs direct line */
          maze_view(tp.y, tp.x))) ||
        DISTANCE(tp,hero) < see_dist);
}

coord   *
find_shoot(struct thing *tp, coord *dir)
{
    struct room *rtp;
    int ulx, uly, xmx, ymx, xmon, ymon, tpx, tpy, row, col;
    struct linked_list  *mon;
    struct thing    *ick;

    rtp = roomin(tp->t_pos);   /* Find room of chaser */

    if (rtp == NULL)
        return NULL;

    ulx = rtp->r_pos.x;
    uly = rtp->r_pos.y;
    xmx = rtp->r_max.x;
    ymx = rtp->r_max.y;

    tpx = tp->t_pos.x;
    tpy = tp->t_pos.y;

    for (col = ulx; col < (ulx + xmx); col++)
        for (row = uly; row < (uly + ymx); row++)
        {
            if (row > 0 && col > 0 && isalpha(mvwinch(mw, row, col)))
            {
		mon = find_mons(row, col);

                if (mon)
                {
                    ick = THINGPTR(mon);
                    xmon = ick->t_pos.x;
                    ymon = ick->t_pos.y;

                    if (!(good_monster(*ick)))
                    {
                        if (straight_shot(tpy, tpx, ymon, xmon, dir))
                            return(dir);
                    }
                }
            }
        }

    return(NULL);
}

/*
    can_shoot()
        determines if the monster (er) has a direct line of shot at the
        player (ee).  If so, it returns the direction in which to shoot.
*/

coord *
can_shoot(coord *er, coord *ee, coord *dir)
{
    int ery, erx, eey, eex;

    /* Make sure we are chasing the player */

    if (!ce((*ee), hero))
        return(NULL);

    /* They must be in the same room */

    if (roomin(*er) != roomin(hero))
        return(NULL);

    ery = er->y;
    erx = er->x;
    eey = ee->y;
    eex = ee->x;

    /* Will shoot unless next to player, then 80% prob will fight */

    if ((DISTANCE(*er,*ee) < 4) && (rnd(100) < 80))
        return(NULL);

    /* Do we have a straight shot? */

    if (!straight_shot(ery, erx, eey, eex, dir))
        return(NULL);
    else
        return(dir);
}

/*
    straight_shot()
        See if there is a straight line of sight between the two
        given coordinates.  If shooting is not NULL, it is a pointer to a
        structure which should be filled with the direction to shoot (if
        there is a line of sight).  If shooting, monsters get in the way.
        Otherwise, they do not.
*/

int
straight_shot(int ery, int erx, int eey, int eex, coord *dir)
{
    int dy, dx; /* Deltas */
    int ch;

    /* Does the monster have a straight shot at player */

    if ((ery != eey) && (erx != eex) &&
        (abs(ery - eey) != abs(erx - eex)))
        return (FALSE);

    /* Get the direction to shoot */

    if (eey > ery)
        dy = 1;
    else if (eey == ery)
        dy = 0;
    else
        dy = -1;

    if (eex > erx)
        dx = 1;
    else if (eex == erx)
        dx = 0;
    else
        dx = -1;

    /* Make sure we have free area all the way to the player */

    ery += dy;
    erx += dx;

    while ((ery != eey) || (erx != eex))
    {
        switch(ch = winat(ery, erx))
        {
            case '|':
            case '-':
            case WALL:
            case DOOR:
            case SECRETDOOR:
                return(FALSE);
            default:
                if (dir && isalpha(ch))
                    return(FALSE);
        }

        ery += dy;
        erx += dx;
    }

    if (dir)
    {     /* If we are shooting -- put in the directions */
        dir->y = dy;
        dir->x = dx;
    }

    return(TRUE);
}

/*
    get_hurl
        returns the weapon that the monster will "throw" if it has one
*/

struct linked_list  *
get_hurl(struct thing *tp)
{
    struct linked_list  *arrow,  *bolt,      *rock, *silverarrow, *fbbolt;
    struct linked_list  *bullet, *firearrow, *dart, *dagger,      *shuriken;
    struct linked_list  *oil,    *grenade;

    struct linked_list  *pitem;
    int bow = FALSE, crossbow = FALSE, sling = FALSE, footbow = FALSE;

    /* Don't point to anything to begin with */

    arrow = bolt = rock = silverarrow = fbbolt = NULL;
    bullet = firearrow = dart = dagger = shuriken = NULL;
    oil = grenade = NULL;

    for (pitem = tp->t_pack; pitem != NULL; pitem = next(pitem))
        if ((OBJPTR(pitem))->o_type == WEAPON)
            switch ((OBJPTR(pitem))->o_which)
            {
                case    BOW:bow = TRUE; break;
                case    CROSSBOW:crossbow = TRUE; break;
                case    SLING:sling = TRUE; break;
                case    FOOTBOW:footbow = TRUE; break;
                case    ROCK:rock = pitem; break;
                case    ARROW:arrow = pitem; break;
                case    SILVERARROW:silverarrow = pitem; break;
                case    BOLT:bolt = pitem; break;
                case    FBBOLT:fbbolt = pitem; break;
                case    BULLET:bullet = pitem; break;
                case    FLAMEARROW:firearrow = pitem; break;
                case    DART:dart = pitem; break;
                case    DAGGER:dagger = pitem; break;
                case    SHURIKEN:shuriken = pitem; break;
                case    MOLOTOV:oil = pitem; break;
                case    GRENADE:shuriken = pitem; break;
            }

    if (bow && silverarrow)
        return(silverarrow);

    if (crossbow && bolt)
        return(bolt);

    if (bow && firearrow)
        return(firearrow);

    if (off(*tp, ISCHARMED) && oil)
        return(oil);
		
	if (off(*tp, ISCHARMED) && grenade)
		return(grenade);

    if (footbow && fbbolt)
        return(fbbolt);

    if (bow && arrow)
        return(arrow);

    if (sling && bullet)
        return(bullet);

    if (sling && rock)
        return(rock);

    if (shuriken)
        return(shuriken);

    if (dagger)
        return(dagger);

    if (silverarrow)
        return(silverarrow);

    if (firearrow)
        return(firearrow);

    if (fbbolt)
        return(fbbolt);

    if (bolt)
        return(bolt);

    if (bullet)
        return(bullet);

    if (dart)
        return(dart);

    if (rock)
        return(rock);

    return(NULL);
}

/*
    pick_weap()
        returns the biggest weapon that the monster will wield if it
        has a non-launching or non-missile weapon returns NULL if no weapon, or
        bare hands is better
*/

struct object *
pick_weap(struct thing *tp)
{
    int weap_dam = maxdamage(tp->t_stats.s_dmg);
    struct object   *ret_obj = NULL;
    struct linked_list  *pitem;

    if (on(*tp, CANWIELD))
    {
        for (pitem = tp->t_pack; pitem != NULL; pitem = next(pitem))
        {
            struct object   *obj = OBJPTR(pitem);

            if (obj->o_type != WEAPON && !(obj->o_flags&(ISLAUNCHER|ISMISL)) &&
                maxdamage(obj->o_damage) > weap_dam)
            {
                weap_dam = maxdamage(obj->o_damage);
                ret_obj = obj;
            }
        }
    }

    return (ret_obj);
}

/*
    canblink()
        checks if the monster can teleport (blink).  If so, it will try
        to blink the monster next to the player.
*/

int
can_blink(struct thing *tp)
{
    int   y, x, index = 9;
    coord   tryp;       /* To hold the coordinates for use in diag_ok */
    int    spots[9], found_one = FALSE;

    /*
     * First, can the monster even blink?  And if so, there is only a 30%
     * chance that it will do so.  And it won't blink if it is running.
     */

    if (off(*tp, CANBLINK) || (on(*tp, ISHELD)) ||
        on(*tp, ISFLEE) ||
        (on(*tp, ISSLOW) && off(*tp, ISHASTE) && !(tp->t_turn)) ||
        (rnd(10) < 9))
        return (FALSE);

    /* Initialize the spots as illegal */

    do
    {
        spots[--index] = FALSE;
    }
    while (index > 0);

    /* Find a suitable spot next to the player */

    for (y = hero.y - 1; y < hero.y + 2; y++)
        for (x = hero.x - 1; x < hero.x + 2; x++, index++)
        {
            /*
             * Make sure x coordinate is in range and that we are
             * not at the player's position
             */

            if (x < 0 || x >= COLS || index == 4)
                continue;

            /* Is it OK to move there? */

            if (!step_ok(y, x, NOMONST, tp))
                spots[index] = FALSE;
           else
           {

                /*
                 * OK, we can go here.  But don't go there if
                 * monster can't get at player from there
                 */

                tryp.y = y;
                tryp.x = x;
                if (diag_ok(&tryp, &hero, tp))
                {
                    spots[index] = TRUE;
                    found_one = TRUE;
                }
            }
        }

    /* If we found one, go to it */

    if (found_one)
    {
        /* Find a legal spot */

        while (spots[index = rnd(9)] == FALSE)
            continue;

        /* Get the coordinates */

        y = hero.y + (index / 3) - 1;
        x = hero.x + (index % 3) - 1;

        /* Move the monster from the old space */

        mvwaddch(cw, tp->t_pos.y, tp->t_pos.x, tp->t_oldch);

        /* Move it to the new space */

        tp->t_oldch = CCHAR( mvwinch(cw, y, x) );

        if (cansee(y, x) &&
            off(*tp, ISINWALL) &&
            ((off(*tp, ISINVIS) &&
              (off(*tp, ISSHADOW) || rnd(100) < 10)) || on(player, CANSEE)) &&
            off(*tp, CANSURPRISE))
            mvwaddch(cw, y, x, tp->t_type);

        mvwaddch(mw, tp->t_pos.y,tp->t_pos.x,' '); /*Clear old position */
        mvwaddch(mw, y, x, tp->t_type);
        tp->t_pos.y = y;
        tp->t_pos.x = x;
    }

    return (found_one);
}
