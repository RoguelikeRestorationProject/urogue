/*
    scrolls.c - Functions for dealing with scrolls
  
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
#include <string.h>
#include <ctype.h>
#include "rogue.h"

/*
    read_scroll - read a scroll (or effect a scroll-like spell)
        reader:  who does it
        which:   which S_SCROLL (-1 means ask from pack)
        flags:   ISBLESSED, ISCURSED
*/

void
read_scroll(struct thing *reader, int which, int flags)
{
    struct object   *obj;
    struct linked_list  *item, *nitem;
    int i, j, charm_power;
    char    ch, nch;
    int     blessed = flags & ISBLESSED;
    int     cursed = flags & ISCURSED;
    int     is_scroll = (which < 0 ? TRUE : FALSE);
    char    buf[2 * LINELEN];

    if (reader != &player)
    {
        monread(reader, which, flags);
        return;
    }

    if (is_scroll)      /* A regular scroll */
    {
        if ((item = get_item("read", SCROLL)) == NULL)
            return;

        obj = OBJPTR(item);

        if (obj->o_type != SCROLL)
        {
            msg("It says 'Made in Yugoslavia'!");
            return;
        }

        if (on(player, ISBLIND))
        {
            msg("You can't see to read anything.");
            return;
        }

        /* Calculate its effect */

        cursed = obj->o_flags & ISCURSED;
        blessed = obj->o_flags & ISBLESSED;
        flags = obj->o_flags;
        which = obj->o_which;

        /* remove it from the pack */

        rem_pack(obj);
        discard(item);
        updpack();
    }

    switch (which)
    {
        case S_CONFUSE:  /* Touch causes monster confusion.  */
            if (cursed)
                quaff(reader, P_CLEAR, ISCURSED);
            else
            {
                msg("Your hands begin to glow red.");
                turn_on(player, CANHUH);
                /* if blessed... */
            }
            break;

        case  S_CURING:   /* A cure disease spell */
            if (on(player, HASINFEST) || on(player, HASDISEASE))
            {
                if (!cursed && on(player, HASDISEASE))
                {
                    extinguish_fuse(FUSE_CURE_DISEASE);
                    cure_disease(NULL);
                }

                if (on(player, HASINFEST))
                {
                    msg("You begin to feel yourself improving again.");
                    turn_off(player, HASINFEST);
                    infest_dam = 0;
                }

                if (is_scroll)
                    know_items[TYP_SCROLL][S_CURING] = TRUE;
            }
            else
                nothing_message(flags);
            break;

        case S_LIGHT:
            if (blue_light(flags) && is_scroll)
                know_items[TYP_SCROLL][S_LIGHT] = TRUE;
            break;

        case S_HOLD:
            if (cursed)
            {
                /*
                 * This scroll aggravates all the monsters on the
                 * current level and sets them running towards the
                 * hero
                 */
                aggravate();
                hearmsg("You hear a high pitched humming noise.");
            }
            else if (blessed)   /* Hold all monsters on level */
            {
                if (mlist == NULL)
                    nothing_message(flags);
                else
                {
                    struct linked_list  *mon;
                    struct thing    *th;

                    for (mon = mlist; mon != NULL; mon = next(mon))
                    {
                        th = THINGPTR(mon);
                        turn_off(*th, ISRUN);
                        turn_on(*th, ISHELD);
                    }
                    msg("A sudden peace comes over the dungeon.");
                }
            }
            else
            {
                /*
                 * Hold monster scroll.  Stop all monsters within two
                 * spaces from chasing after the hero.
                 */
                int x, y;
                struct linked_list  *mon;
                int gotone = FALSE;

                for (x = hero.x - 2; x <= hero.x + 2; x++)
                {
                    for (y = hero.y - 2; y <= hero.y + 2; y++)
                    {
                        if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x)))
                        {
                            if ((mon = find_mons(y, x)) != NULL)
                            {
                                struct thing    *th;

                                gotone = TRUE;
                                th = THINGPTR(mon);
                                turn_off(*th, ISRUN);
                                turn_on(*th, ISHELD);
                            }
                        }
                    }
                }

                if (gotone)
                    msg("A sudden peace surrounds you.");
                else
                    nothing_message(flags);
            }
            break;

        case S_SLEEP:

            /* if cursed, you fall asleep */

            if (cursed)
            {
                if (is_wearing(R_ALERT))
                    msg("You feel drowsy for a moment.");
                else
                {
                    msg("You fall asleep.");
                    no_command += 4 + rnd(SLEEPTIME);
                }
            }
            else
            {
                /*
                 * sleep monster scroll.  puts all monsters within 2
                 * spaces asleep
                 */
                int x, y;
                struct linked_list  *mon;
                int gotone = FALSE;

                for (x = hero.x - 2; x <= hero.x + 2; x++)
                {
                    for (y = hero.y - 2; y <= hero.y + 2; y++)
                    {
                        if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x)))
                        {
                            if ((mon = find_mons(y, x)) != NULL)
                            {
                                struct thing    *th;
                                th = THINGPTR(mon);

                                if (on(*th, ISUNDEAD))
                                    continue;

                                gotone = TRUE;
                                th->t_no_move += SLEEPTIME;
                            }
                        }
                    }
                }

                if (gotone)
                    msg("The monster(s) around you seem to have fallen asleep.");
                else
                    nothing_message(flags);
            }
            break;

        case S_CREATE:
        {
            /*
             * Create a monster. First look in a circle around
             * him, next try his room otherwise give up
             */

            struct thing    *tp;
            struct linked_list  *ip;

            if (blessed)
                summon_monster((short) 0, NOFAMILIAR, MESSAGE);
            else if (cursed)
            {
                i = rnd(4) + 3;
                for (j = 0; j < i; j++)
                {
                    if ((ip = creat_mons(&player, (short) 0, MESSAGE)) != NULL)
                    {
                        tp = THINGPTR(ip);
                        turn_off(*tp, ISFRIENDLY);
                    }
                }
            }
            else if ((ip = creat_mons(&player, (short) 0, MESSAGE)) != NULL)
            {
                tp = THINGPTR(ip);
                turn_off(*tp, ISFRIENDLY);
            }
        }
        break;

        case S_IDENTIFY:
            if (cursed)
                msg("You identify this scroll as an identify scroll");
            else if (blessed)   /* identify everything in the pack */
            {
                msg("You feel more Knowledgeable!");
                idenpack();
            }
            else
            {
                /* Identify, let the rogue figure something out  */

                if (is_scroll && know_items[TYP_SCROLL][S_IDENTIFY] != TRUE)
                {
                    msg("This scroll is an identify scroll.");
                    know_items[TYP_SCROLL][S_IDENTIFY] = TRUE;
                }
                whatis(NULL);
            }
            break;

        case S_MAP:

            /* Scroll of magic mapping. */

            if (cursed)
            {
                msg("Your mind goes blank for a moment.");
                wclear(cw);
                light(&hero);
                status(TRUE);
                break;
            }

            if (is_scroll && know_items[TYP_SCROLL][S_MAP] != TRUE)
            {
                msg("Oh! This scroll has a map on it!!");
                know_items[TYP_SCROLL][S_MAP] = TRUE;
            }

            if (blessed)
                turn_on(player, BLESSMAP);

            overwrite(stdscr, hw);

            /* Take all the things we want to keep hidden out of the window */

            for (i = 0; i < LINES; i++)
                for (j = 0; j < COLS; j++)
                {
                    switch (nch = ch = CCHAR(mvwinch(hw, i, j)))
                    {
                        case SECRETDOOR:
                            nch = DOOR;
                            mvaddch(i, j, nch);
                            break;

                        case '-':
                        case '|':
                        case DOOR:
                        case PASSAGE:
                        case ' ':
                        case STAIRS:
                            if (mvwinch(mw, i, j) != ' ')
                            {
                                struct thing    *it;
                                struct linked_list  *lit;

                                lit = find_mons(i, j);

				if (lit) {
				    it = THINGPTR(lit);

				    if (it && it->t_oldch == ' ')
					it->t_oldch = nch;
				}
                            }
                            break;

                        default:
                            if (!blessed || !isatrap(ch))
                                nch = ' ';
                            else
                            {
                                struct trap *tp;
                                struct room *rp;

                                tp = trap_at(i, j);
                                rp = roomin(hero);

                                if (tp->tr_type == FIRETRAP && rp != NULL)
                                {
                                    rp->r_flags &= ~ISDARK;
                                    light(&hero);
                                }

                                tp->tr_flags |= ISFOUND;
                            }
                    }
                    if (nch != ch)
                        waddch(hw, nch);
                }

            /* Copy in what he has discovered */
            overlay(cw, hw);

            /* And set up for display */
            overwrite(hw, cw);

            break;

        case S_GFIND:
            /* Scroll of gold detection */

            if (cursed)
            {
                int n = roll(3, 6);
                int k;
                struct room *rp;
                coord   pos;

                msg("You begin to feel greedy and you sense gold.");
                wclear(hw);

                for (k = 1; k < n; k++)
                {
                    rp = &rooms[rnd_room()];
                    rnd_pos(rp, &pos);
                    mvwaddch(hw, pos.y, pos.x, GOLD);
                }
                overlay(hw, cw);

                break;
            }

            if (blessed)
                turn_on(player, BLESSGOLD);

            if (gsense() && is_scroll)
                know_items[TYP_SCROLL][S_GFIND] = TRUE;

            break;

        case S_SELFTELEP:

            /* Scroll of teleportation: Make him disappear and reappear */

            if (cursed)
            {
                level += 5 + rnd(5);
                new_level(NORMLEV,0);
                mpos = 0;
                msg("You are banished to the lower regions.");
            }
            else if (blessed)
            {
                level -= rnd(3) + 1;

                if (level < 1)
                    level = 1;

                mpos = 0;
                new_level(NORMLEV,0); /* change levels */
                status(TRUE);
                msg("You are whisked away to another region.");
            }
            else
            {
                teleport();

                if (is_scroll)
                    know_items[TYP_SCROLL][S_SELFTELEP] = TRUE;
            }

            if (off(player, ISCLEAR))
            {
                if (on(player, ISHUH))
                    lengthen_fuse(FUSE_UNCONFUSE, rnd(4) + 4);
                else
                {
                    light_fuse(FUSE_UNCONFUSE, 0, rnd(4) + 4, AFTER);
                    turn_on(player, ISHUH);
                }
            }
            else
                msg("You feel dizzy for a moment, but it quickly passes.");

            break;

        case S_SCARE:

            /*
             * A blessed scroll of scare monster automatically transports
             * itself to the hero's feet
             *
             */

            if (blessed)
            {
                ch = CCHAR( mvwinch(stdscr, hero.y, hero.x) );

                if (ch != FLOOR && ch != PASSAGE)
                {
                    msg("Your feet tickle for a moment");
                    return;
                }

                item = spec_item(SCROLL, S_SCARE, 0, 0);

                obj = OBJPTR(item);
                obj->o_flags = ISCURSED;
                obj->o_pos = hero;
                add_obj(item, hero.y, hero.x);
                msg("A wave of terror sweeps throughout the room");
            }
            else
            {
                /*
                 * A monster will refuse to step on a scare monster
                 * scroll if it is dropped.  Thus reading it is a
                 * mistake and produces laughter at the poor rogue's
                 * boo boo.
                 */

                msg("You hear maniacal laughter in the distance.");

                if (cursed) /* If cursed, monsters get mad */
                    aggravate();
            }
            break;

        case S_REMOVECURSE:
            if (cursed)     /* curse a player's possession */
            {
                for (nitem = pack; nitem != NULL; nitem = next(nitem))
                {
                    obj = OBJPTR(nitem);

                    if (rnd(5) == 0)
                        if (obj->o_flags & ISBLESSED)
                            obj->o_flags &= ~ISBLESSED;
                        else
                            obj->o_flags |= ISCURSED;
                }
                msg("The smell of fire and brimstone comes from your pack.");
            }
            else if (blessed)
            {
                for (nitem = pack; nitem != NULL; nitem = next(nitem))
                    (OBJPTR(nitem))->o_flags &= ~ISCURSED;

                msg("Your pack glistens brightly.");
            }
            else
            {
                if ((nitem = get_item("remove the curse on", 0)) != NULL)
                {
                    obj = OBJPTR(nitem);
                    msg("Removed the curse from %s.", inv_name(obj, LOWERCASE));
                    obj->o_flags &= ~ISCURSED;

                    if (is_scroll)
                        know_items[TYP_SCROLL][S_REMOVECURSE] = TRUE;
                }
            }
            break;

        case S_PETRIFY:
            switch(CCHAR(mvinch(hero.y, hero.x)))
            {
                case TRAPDOOR:
                case DARTTRAP:
                case TELTRAP:
                case ARROWTRAP:
                case SLEEPTRAP:
                case BEARTRAP:
                case FIRETRAP:
                {
                    int n;

                    /* Find the right trap */
                    for (n = 0; n < ntraps && !ce(traps[n].tr_pos, hero); n++)
                        ;

                    ntraps--;

                    if (!ce(traps[n].tr_pos, hero))
                        msg("What a strange trap!");
                    else
                    {
                        while (n < ntraps)
                        {
                            traps[n] = traps[n + 1];
                            n++;
                        }
                    }

                    msg("The dungeon begins to rumble and shake!");
                    addch(WALL);

                    if (on(player, CANINWALL))
                    {
                        extinguish_fuse(FUSE_UNPHASE);
                        turn_off(player, CANINWALL);
                        msg("Your dizzy feeling leaves you.");
                    }

                    turn_on(player, ISINWALL);
                }
                break;

                case DOOR:
                case SECRETDOOR:
                {
                    struct room *rp = roomin(hero);
                    short   n;

                    /* Find the right door */

                    for (n=0; n<rp->r_nexits && !ce(rp->r_exit[n], hero); n++)
                        /* do nothing */ ;

                    rp->r_nexits--;

                    if (!ce(rp->r_exit[n], hero))
                        msg("What a strange door!");
                    else
                    {
                        while (n < rp->r_nexits)
                        {
                            rp->r_exit[n] = rp->r_exit[n + 1];
                            n++;
                        }
                    }
                    /* No break - fall through */
                }
                case FLOOR:
                case PASSAGE:
                    msg("The dungeon begins to rumble and shake!");
                    addch(WALL);

                    if (on(player, CANINWALL))
                    {
                        extinguish_fuse(FUSE_UNPHASE);
                        turn_off(player, CANINWALL);
                        msg("Your dizzy feeling leaves you.");
                    }

                    turn_on(player, ISINWALL);
                    break;

                default:
                    nothing_message(flags);
                    break;
            }
            break;

        case  S_GENOCIDE:
            msg("You have been granted the boon of genocide!--More--");

            wait_for(' ');
            msg("");

            genocide(flags);

            if (is_scroll)
                know_items[TYP_SCROLL][S_GENOCIDE] = TRUE;

            break;

        case S_PROTECT:
            if (is_scroll && know_items[TYP_SCROLL][S_PROTECT] == FALSE)
            {
                msg("You can protect something from rusting or theft.");
                know_items[TYP_SCROLL][S_PROTECT] = TRUE;
            }

            if ((item = get_item("protect", 0)) != NULL)
            {
                struct object   *lb = OBJPTR(item);

                if (cursed)
                {
                    lb->o_flags &= ~ISPROT;
                    mpos = 0;
                    msg("Unprotected %s.", inv_name(lb, LOWERCASE));
                }
                else
                {
                    lb->o_flags |= ISPROT;
                    mpos = 0;
                    msg("Protected %s.", inv_name(lb, LOWERCASE));
                }
            }
            break;

        case S_MAKEITEMEM:
            if (!is_scroll || rnd(luck))
                feel_message();
            else
            {
                char itemtype;

                if (is_scroll)
                    know_items[TYP_SCROLL][S_MAKEITEMEM] = TRUE;

                msg("You have been endowed with the power of creation.");

                if (blessed)
                    itemtype = '\0';
                else
                    switch (rnd(6))
                    {
                        case 0: itemtype = RING;    break;
                        case 1: itemtype = POTION;  break;
                        case 2: itemtype = SCROLL;  break;
                        case 3: itemtype = ARMOR;   break;
                        case 4: itemtype = WEAPON;  break;
                        case 5: itemtype = STICK;   break;
                    }

                flags |= SCR_MAGIC;
                buy_it(itemtype, flags);
            }
            break;

        case S_ENCHANT:
        {
            struct linked_list  *ll;
            struct object   *lb;
            int howmuch, flg=0;

            if (is_scroll && know_items[TYP_SCROLL][S_ENCHANT] == FALSE)
            {
                msg("You are granted the power of enchantment.");
                msg("You may enchant anything(weapon,ring,armor,scroll,potion)");
                know_items[TYP_SCROLL][S_ENCHANT] = TRUE;
            }

            if ((ll = get_item("enchant", 0)) != NULL)
            {
                lb = OBJPTR(ll);
                lb->o_flags &= ~ISCURSED;

                if (blessed)
                    howmuch = 2;
                else if (cursed)
                    howmuch = -1;
                else
                {
                    howmuch = 1;
                    flg |= ISBLESSED;
                }

                switch (lb->o_type)
                {
                    case RING:
                        lb->o_ac += howmuch;

                        if (lb->o_ac > 5 && rnd(5) == 0)
                        {
                            msg("Your ring explodes in a cloud of smoke.");
                            lb->o_flags &= ~ISCURSED;
                            dropcheck(lb);

                            switch (lb->o_which)
                            {
                                case R_ADDSTR:
                                    chg_str(-2, TRUE, FALSE);
                                    break;
                                case R_ADDHIT:
                                    chg_dext(-2, TRUE, FALSE);
                                    break;
                                case R_ADDINTEL:
                                    pstats.s_intel -= 2;
                                    max_stats.s_intel -= 2;
                                    break;
                                case R_ADDWISDOM:
                                    pstats.s_wisdom -= 2;
                                    max_stats.s_wisdom -= 2;
                                    break;
                            }

                            del_pack(ll);
                            lb = NULL;
                        }
                        else if (is_r_on(lb))
                            switch (lb->o_which)
                            {
                                case R_ADDSTR:
                                    pstats.s_str += howmuch;
                                    break;
                                case R_ADDHIT:
                                    pstats.s_dext += howmuch;
                                    break;
                                case R_ADDINTEL:
                                    pstats.s_intel += howmuch;
                                    break;
                                case R_ADDWISDOM:
                                    pstats.s_wisdom += howmuch;
                                    break;
                                case R_CARRYING:
                                    updpack();
                                    break;
                            }

                        break;

                    case ARMOR:
                        lb->o_ac -= howmuch;

                        if (armors[lb->o_which].a_class - lb->o_ac > 5 && rnd(5) == 0)
                        {
                            msg("Your %s explodes in a cloud of dust.",
                                inv_name(lb, LOWERCASE));

                            lb->o_flags &= ~ISCURSED;

                            if (lb == cur_armor)
                                pstats.s_hpt /= 2;

                            dropcheck(lb);
                            del_pack(ll);
                            lb = NULL;
                        }
                        break;

                    case STICK:
                        if (wizard || howmuch != 1 && rnd(5) == 0)
                            lb->o_flags |= flg;

                        lb->o_charges += howmuch + 10;

                        if (lb->o_charges < 0)
                            lb->o_charges = 0;

                        if (lb->o_charges > 50 && rnd(5) == 0)
                        {
                            msg("Your %s explodes into splinters.",
                                inv_name(lb, LOWERCASE));

                            lb->o_flags &= ~ISCURSED;
                            dropcheck(lb);
                            del_pack(ll);
                            lb = NULL;
                        }
                        break;

                    case WEAPON:
                        if (rnd(100) < 50)
                            lb->o_hplus += howmuch;
                        else
                            lb->o_dplus += howmuch;

                        if (lb->o_hplus + lb->o_dplus > 10 && rnd(5) == 0)
                        {
                            msg("Your %s explodes in a cloud of shrapnel",
                                inv_name(lb, LOWERCASE));

                            lb->o_flags &= ~ISCURSED;

                            if (lb == cur_weapon)
                                chg_dext(-2, FALSE, TRUE);

                            dropcheck(lb);
                            del_pack(ll);
                            lb = NULL;

                        }
                        break;

                    case POTION:
                    case SCROLL:
                    default:
                        lb->o_flags |= flg;
                        break;
                }

                mpos = 0;

                if (lb != NULL)
                    msg("Enchanted %s.", inv_name(lb, LOWERCASE));
            }
        }
        break;

        case S_NOTHING:
            nothing_message(flags);
            break;

        case S_SILVER:
        {
            struct object   *lb;

            if (is_scroll && know_items[TYP_SCROLL][S_SILVER] == FALSE)
            {
                msg("You are granted the power of magic hitting.");
                know_items[TYP_SCROLL][S_SILVER] = TRUE;
            }

            if ((item = get_item("annoint", WEAPON)) != NULL)
            {
                lb = OBJPTR(item);

                if (blessed && !(lb->o_flags & ISSILVER))
                {
                    lb->o_hplus += rnd(3) + 1;
                    lb->o_flags |= ISSILVER;
                    lb->o_flags |= ISMETAL;
                    msg("Your weapon has turned to silver!");
                }
                else if (cursed && (lb->o_flags & ISSILVER))
                {
                    lb->o_hplus -= (rnd(3) + 1);
                    lb->o_flags &= ~ISSILVER;
                    msg("Your silver weapon has turned to steel.");
                }
                else if (lb->o_flags & ISSILVER)
                {
                    msg("Your silver weapon glitters briefly.");
                    lb->o_hplus += rnd(2);
                }
                else
                {
                    lb->o_hplus += rnd(3);
                    lb->o_flags |= ISSILVER;
                    lb->o_flags |= ISMETAL;
                    msg("Your weapon has turned to silver.");
                }
            }
        }
        break;
        case S_OWNERSHIP:
        {
            struct linked_list  *ll;
            struct object   *lb;

            if (is_scroll && know_items[TYP_SCROLL][S_OWNERSHIP] == FALSE)
            {
                msg("You are granted the power of ownership.");
                know_items[TYP_SCROLL][S_OWNERSHIP] = TRUE;
            }

            if ((ll = get_item("claim", 0)) != NULL)
            {
                lb = OBJPTR(ll);

                if (cursed && lb->o_flags & (ISOWNED | CANRETURN))
                {
                    lb->o_flags &= ~(ISOWNED | CANRETURN);
                    msg("The gods seem to have forgotten you.");
                }
                else if (cursed && !(lb->o_flags & ISLOST))
                {
                    lb->o_flags |= ISLOST;
                    msg("The gods look the other way.");
                }
                else if (blessed && lb->o_flags & ISLOST)
                {
                    lb->o_flags |= CANRETURN;
                    msg("The gods seem to have remembered you.");
                }
                else if (blessed && !(lb->o_flags & ISOWNED))
                {
                    lb->o_flags |= (ISOWNED | CANRETURN);
                    msg("The gods smile upon you.");
                }
                else if (blessed | cursed)
                {
                    nothing_message(flags);
                }
                else
                {
                    lb->o_flags |= CANRETURN;
                    msg("The gods look upon you.");
                }
            }
        }
        break;

        case S_FOODDET:

            /* Scroll of food detection */

            if (cursed)
            {
                int n = roll(3, 6);
                int k;
                struct room *rp;
                coord   pos;

                msg("You begin to feel hungry and you smell food.");
                wclear(hw);

                for (k = 1; k < n; k++)
                {
                    rp = &rooms[rnd_room()];
                    rnd_pos(rp, &pos);
                    mvwaddch(hw, pos.y, pos.x, FOOD);
                }

                overlay(hw, cw);

                if (is_scroll)
                    know_items[TYP_SCROLL][S_FOODDET] = TRUE;

                break;
            }

            if (blessed)
                turn_on(player, BLESSFOOD);

            if (off(player, ISUNSMELL) && lvl_obj != NULL)
            {
                struct linked_list  *itm;
                struct object   *cur;
                struct thing    *th;
                int fcount = 0;
                int  same_room = FALSE;
                struct room *rp = roomin(hero);

                wclear(hw);

                for (itm = lvl_obj; itm != NULL; itm = next(itm))
                {
                    cur = OBJPTR(itm);

                    if (cur->o_type == FOOD)
                    {
                        fcount += cur->o_count;
                        mvwaddch(hw, cur->o_pos.y, cur->o_pos.x, FOOD);

                        if (roomin(cur->o_pos) == rp)
                            same_room = TRUE;
                    }
                }

                for (itm = mlist; itm != NULL; itm = next(itm))
                {
                    struct linked_list  *pitem;

                    th = THINGPTR(itm);

                    for (pitem = th->t_pack; pitem != NULL; pitem = next(pitem))
                    {
                        cur = OBJPTR(pitem);

                        if (cur->o_type == FOOD)
                        {
                            fcount += cur->o_count;
                            mvwaddch(hw, th->t_pos.y, th->t_pos.x, FOOD);

                            if (roomin(th->t_pos) == rp)
                                same_room = TRUE;
                        }
                    }
                }

                if (fcount)
                {
                    if (is_scroll)
                        know_items[TYP_SCROLL][S_FOODDET] = TRUE;

                    if (same_room)
                        msg("FOOOOD!!");
                    else
                        msg("You begin to feel hungry and you smell food.");

                    overlay(hw, cw);
                    break;
                }
            }

            if (off(player, ISUNSMELL))
                msg("You can't smell anything.");
            else
                nothing_message(flags);

            break;

        case S_ELECTRIFY:
            if (on(player, ISELECTRIC))
            {
                msg("Your violet glow brightens for an instant.");
                lengthen_fuse(FUSE_UNELECTRIFY, 4 + rnd(8));
            }
            else
            {
                msg("Your body begins to glow violet and shoot sparks.");
                turn_on(player, ISELECTRIC);
                light_fuse(FUSE_UNELECTRIFY,0,(blessed?3:1)*WANDERTIME, AFTER);
                light(&hero);
            }

            if (is_scroll)
                know_items[TYP_SCROLL][S_ELECTRIFY] = TRUE;

            break;

        case S_CHARM:

            /*
             * Beauty, brains and experience make a person charming.
             * Unique monsters can never be charmed.
             */

            charm_power = pstats.s_charisma / 2 + pstats.s_lvl / 3 + max(0, pstats.s_intel - 15);

            if (cursed)
            {
                msg("You hear harsh, dissonant twanging throughout the dungeon.");
                aggravate();
            }
            else if (blessed)  /* Charm entire level */
            {
                struct linked_list  *mon;

                msg("You hear ringingly meliflous music all around you.");

                for (mon = mlist; mon != NULL; mon = next(mon))
                {
                    struct thing    *th = THINGPTR(mon);

                    if (th->t_stats.s_intel < charm_power && off(*th, ISUNIQUE))
                    {
                        turn_on(*th, ISCHARMED);
                        chase_it(&th->t_pos, &player);
                    }
                }
            }
            else        /* Charm all monsters within two spaces of the hero */
            {
                int x, y;
                struct linked_list  *mon;

                msg("You hear soft, lyrical music all around you.");

                for (x = hero.x - 2; x <= hero.x + 2; x++)
                    for (y = hero.y - 2; y <= hero.y + 2; y++)
                        if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x)))
                        {
                            if ((mon = find_mons(y, x)) != NULL)
                            {
                                struct thing    *th;

                                th = THINGPTR(mon);

                                if (th->t_stats.s_intel < charm_power && off(*th, ISUNIQUE))
                                {
                                    turn_on(*th, ISCHARMED);
                                    chase_it(&th->t_pos, &player);
                                }
                            }
                        }
            }
            break;

        case S_SUMMON:
        {
            struct linked_list  *llp;
            struct thing    *tp;
            int mon_type;

            if (on(player, HASSUMMONED))
            {
                nothing_message(flags);
                break;
            }

            if (cursed)
            {
                creat_mons(&player, (short) 0, MESSAGE);
                break;
            }

            if (blessed)    /* Summon a possibly very high monster */
            {
                int nsides = max(2, max(pstats.s_lvl, 12) - luck);

                mon_type = roll(nsides, rnd(pstats.s_charisma + 15) + 8);
                mon_type = min(mon_type, nummonst);
            }
            else
                mon_type = 0;

	    llp = summon_monster((short) mon_type, NOFAMILIAR, NOMESSAGE);

            if (llp)
            {
                tp = THINGPTR(llp);
                turn_on(*tp, WASSUMMONED);
                turn_on(player, HASSUMMONED);
                msg("You have summoned a %s.", monsters[tp->t_index].m_name);
                light_fuse(FUSE_UNSUMMON, llp, WANDERTIME + rnd(pstats.s_lvl), AFTER);
            }
        }
        break;

        case S_REFLECT:
            if (on(player, CANREFLECT))
            {
                msg("The sparkling around you brightens momentarily.");
                lengthen_fuse(FUSE_UNGAZE, 5 + rnd(10));
            }
            else
            {
                msg("Shiny particles sparkle all around you.");
                turn_on(player, CANREFLECT);
                light_fuse(FUSE_UNGAZE, 0, (blessed ? 3 : 1) * WANDERTIME, AFTER);
            }
            break;

        case S_SUMFAMILIAR:
        {
            int type = 0;

            if (on(player, HASFAMILIAR))
            {
                msg("But you already have a familiar - somewhere...");
                return;
            }

            if (wizard)
                type = get_monster_number("be your familiar");
            else if (blessed)  /* Summon a possibly very high monster */
            {
                int nsides = max(2, max(pstats.s_lvl, 12) - luck);

                type = roll(nsides, rnd(pstats.s_charisma + 15) + 8);
                type = min(type, nummonst);
            }
            else if (cursed)    /* Summon a bat, maggot, eye, etc */
            {
                type = rnd(20) + 1; 
				
                if (summon_monster(type, FAMILIAR, MESSAGE))
                    turn_on(player, HASFAMILIAR);
            }
        }
        break;

        case S_FEAR:

            /* if cursed, you get frightened */

            if (cursed)
            {
                if (off(player, SUPERHERO) && (player.t_ctype != C_PALADIN) && !save(VS_DEATH))
                    msg("A momentary wave of panic sweeps over you.");
                else
                {
                    msg("Panicstricken, you fall into a coma.");
                    no_command += roll(2, SLEEPTIME);
                }
            }
            else
            {
                /*
                 * terrify monster scroll.  frightens all monsters
                 * within 2 spaces
                 */
                int x, y;
                struct linked_list  *mon;
                int  gotone = FALSE;

                for (x = hero.x - 2; x <= hero.x + 2; x++)
                {
                    for (y = hero.y - 2; y <= hero.y + 2; y++)
                    {
                        if (y > 0 && x > 0 && isalpha(mvwinch(mw, y, x)))
                        {
                            if ((mon = find_mons(y, x)) != NULL)
                            {
                                struct thing    *th;

                                th = THINGPTR(mon);

                                if (on(*th, ISUNDEAD) || on(*th, ISUNIQUE))
                                    continue;

                                gotone = TRUE;
                                turn_on(*th, ISFLEE);
                                th->t_chasee = &player;
                                th->t_ischasing = FALSE;
                                th->t_horde = NULL;
                            }
                        }
                    }
                }

                if (gotone)
                    seemsg("The monster(s) around you recoil in horror.");
                else
                    nothing_message(flags);
            }
            break;

        case  S_MSHIELD:  /* deal with blessed/cursed later */
            if (on(player, HASMSHIELD))
            {
                seemsg("The fog around you thickens.");
                lengthen_fuse(FUSE_UNMSHIELD, (blessed ? 3 : 1) * HEROTIME);
            }
            else
            {
                seemsg("A fog forms around you.");
                turn_on(player, HASMSHIELD);
                light_fuse(FUSE_UNMSHIELD, 0, (blessed ? 3 : 1) * HEROTIME, AFTER);
            }

            if (is_scroll)
                know_items[TYP_SCROLL][S_MSHIELD] = TRUE;

            break;

        default:
            msg("What a puzzling scroll!");
            return;
    }

    look(TRUE);     /* put the result of the scroll on the screen */
    status(FALSE);

    if (is_scroll)
    {
        if (know_items[TYP_SCROLL][which] && guess_items[TYP_SCROLL][which])
        {
            ur_free(guess_items[TYP_SCROLL][which]);
            guess_items[TYP_SCROLL][which] = NULL;
        }
        else if (askme && !know_items[TYP_SCROLL][which] && guess_items[TYP_SCROLL][which] == NULL)
        {
            msg("What do you want to call it? ");

            if (get_string(buf, cw) == NORM)
            {
                guess_items[TYP_SCROLL][which] = new_alloc(strlen(buf) + 1);
                strcpy(guess_items[TYP_SCROLL][which], buf);
            }
        }
    }
}

/*
    creat_mons()
        creates the specified monster -- any if 0
*/

struct linked_list  *
creat_mons(struct thing *person, int monster, int message)
{
    coord   mp;

    /* Search for an open place */

    debug("Creator @(%d, %d) ", person->t_pos.y, person->t_pos.x);

    if ((place_mons(person->t_pos.y, person->t_pos.x, &mp)) != FALSE)
    {
        struct linked_list *nitem;

        nitem = new_item(sizeof(struct thing));
        new_monster(nitem, monster == 0 ? randmonster(NOWANDER, NOGRAB)
                : monster, &mp, MAXSTATS);
        chase_it(&mp, &player);

        /* If the monster is on a trap, trap it */

        if (isatrap(mvinch(mp.y, mp.x)))
        {
            debug("Monster trapped during creat_mons.");
            be_trapped(THINGPTR(nitem), mp);
        }

        light(&hero);
        return(nitem);
    }

    if (message)
        hearmsg("You hear a faint cry of anguish in the distance.");

    return(NULL);
}

/*
    place_mons()
        finds a place to put the monster
*/

int
place_mons(int y, int x, coord *pos)
{
    int distance, xx, yy, appears;

    for (distance = 1; distance <= 10; distance++)
    {
        appears = 0;

        for (yy = y - distance; yy <= y + distance; yy++)
            for (xx = x - distance; xx <= x + distance; xx++)
            {
                /* Don't put a monster in top of the creator or player */

                if (xx < 0 || yy < 0)
                    continue;

                if (yy == y && xx == x)
                    continue;

                if (yy == hero.y && xx == hero.x)
                    continue;

                /* Or anything else nasty */

                if (step_ok(yy, xx, NOMONST, FALSE))
                {
                    if (rnd(max(1, (10 * distance - ++appears))) == 0)
                    {
                        pos->y = yy;
                        pos->x = xx;
                        debug("Make monster dist %d appear %d @(%d, %d) ",
                              distance, appears, pos->y, pos->x);
                        return(TRUE);
                    }
                }
            }
    }
    return(FALSE);
}

/*
    is_t_on()
        This subroutine determines if an object that is a ring is being
        worn by the hero  by Bruce Dautrich 4/3/84
 */

int
is_r_on(struct object *obj)
{
    if (obj == cur_ring[LEFT_1] || obj == cur_ring[LEFT_2] ||
        obj == cur_ring[LEFT_3] || obj == cur_ring[LEFT_4] ||
        obj == cur_ring[LEFT_5] ||
        obj == cur_ring[RIGHT_1] || obj == cur_ring[RIGHT_2] ||
        obj == cur_ring[RIGHT_3] || obj == cur_ring[RIGHT_4] ||
        obj == cur_ring[RIGHT_5])
    {
        return(TRUE);
    }

    return(FALSE);
}

/*
    monread()
        monster gets the effect
*/

void
monread(struct thing *reader, int which, int flags)
{
    struct stats    *curp = &(reader->t_stats);
    struct stats    *maxp = &(reader->maxstats);
    int blessed = flags & ISBLESSED;
    int cursed = flags & ISCURSED;

    switch (which)
    {
        case S_SELFTELEP:
            /* If monster was suffocating, stop it */
            if (on(*reader, DIDSUFFOCATE))
            {
                turn_off(*reader, DIDSUFFOCATE);
                extinguish_fuse(FUSE_SUFFOCATE);
            }

            /* If monster held us, stop it */

            if (on(*reader, DIDHOLD) && (hold_count == 0))
                turn_off(player, ISHELD);

            turn_off(*reader, DIDHOLD);

            if (cursed)
                reader->t_no_move++;
            else
            {
                int rm;

                if (blessed)    /* Give him his hit points */
                    curp->s_hpt = maxp->s_hpt;

                /* Erase the monster from the old position */

                if (isalpha(mvwinch(cw, reader->t_pos.y, reader->t_pos.x)))
                    mvwaddch(cw, reader->t_pos.y, reader->t_pos.x, reader->t_oldch);

                mvwaddch(mw, reader->t_pos.y, reader->t_pos.x, ' ');

                /* Get a new position */

                do
                {
                    rm = rnd_room();
                    rnd_pos(&rooms[rm], &reader->t_pos);
                }
                while (winat(reader->t_pos.y, reader->t_pos.x) != FLOOR);

                /* Put it there */

                mvwaddch(mw, reader->t_pos.y, reader->t_pos.x, reader->t_type);
                reader->t_oldch = CCHAR( mvwinch(cw, reader->t_pos.y, reader->t_pos.x) );
            }
            break;

        default:
            debug("'%s' is a strange scroll for a monster to read!",
                  r_magic[which].mi_name);
            break;
    }
}
