/*
    newlvl.c - Dig and draw a new level

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

        Add treasure room code from Rogue 5.2,
        put in #ifdef 0/#endif bracket at end of code
*/

#include "rogue.h"

/*
    new_level()
        Dig and draw a new level
*/

void
new_level(LEVTYPE ltype, int special)
{
    int rm, i, cnt;
    struct linked_list  *item, *nitem;
    struct thing    *tp;
    struct linked_list  *fpack = NULL;
    int     going_down = TRUE;
    coord   stairs;

    /* Start player off right */

    turn_off(player, ISHELD);
    turn_off(player, ISFLEE);
    extinguish_fuse(FUSE_SUFFOCATE);
    hold_count = 0;
    trap_tries = 0;
    no_food++;

    if (level >= max_level)
        max_level = level;
    else
        going_down = FALSE;

    /* Free up the monsters on the last level */

    if (fam_ptr != NULL)    /* save what familiar is carrying */
    {
        fpack = (THINGPTR(fam_ptr))->t_pack;
        (THINGPTR(fam_ptr))->t_pack = NULL;
        fam_ptr = NULL; /* just in case */
    }

    for (i = 1; i <= mons_summoned; i++)
        extinguish_fuse(FUSE_UNSUMMON);

    mons_summoned = 0;

    for (item = mlist; item != NULL; item = nitem)
    {
        tp = THINGPTR(item);
        nitem = next(item);

        if (on(*tp, ISUNIQUE))  /* Put alive UNIQUE on next level */
            monsters[tp->t_index].m_normal = TRUE;

        killed(NULL, item, NOMESSAGE, NOPOINTS);
    }

    free_list(lvl_obj); /* Free up previous objects (if any) */

    wclear(cw);
    wclear(mw);
    clear();
    refresh();
    levtype = ltype;

    switch (ltype)
    {
        case THRONE:
            do_throne(special);    /* do monster throne stuff */
            break;

        case MAZELEV:
            do_maze();
            break;

        case POSTLEV:
			do_post();
			levtype = ltype = NORMLEV;
			level++;
			
        default:
            do_rooms(); /* Draw rooms */
            do_passages();  /* Draw passages */
			break;
    }

    /* Place the staircase down. */

    cnt = 0;

    do
    {
        rm = rnd_room();
        rnd_pos(&rooms[rm], &stairs);
    }
    while (!(mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000));

    addch(STAIRS);

    put_things(ltype);  /* Place objects (if any) */

    if (has_artifact && level == 1)
        create_lucifer(&stairs);

    /* Place the traps */

    ntraps = 0;     /* No traps yet */

    if (levtype == NORMLEV)
    {
        if (rnd(10) < level)
        {
            char    ch = 0;

            i = ntraps = min(MAXTRAPS, rnd(level / 2) + 1);

            /* maybe a lair */

            if (level > 35 && ltype == NORMLEV && rnd(wizard ? 3 : 10) == 0)
            {
                cnt = 0;

                do
                {
                    rm = rnd_room();

                    if (rooms[rm].r_flags & ISTREAS)
                        continue;

                    rnd_pos(&rooms[rm], &stairs);
                }
                while (!(mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000));

                addch(LAIR);
                i--;
                traps[i].tr_flags = 0;
                traps[i].tr_type = LAIR;
                traps[i].tr_show = FLOOR;
                traps[i].tr_pos = stairs;
            }

            while (i--)
            {
                cnt = 0;

                do
                {
                    rm = rnd_room();

                    if (rooms[rm].r_flags & ISTREAS)
                        continue;

                    rnd_pos(&rooms[rm], &stairs);
                }
                while (!(mvinch(stairs.y, stairs.x) == FLOOR || cnt++ > 5000));

                traps[i].tr_flags = 0;

                switch (rnd(11))
                {
                    case 0:
                        ch = TRAPDOOR;
                        break;

                    case 1:
                        ch = BEARTRAP;
                        break;

                    case 2:
                        ch = SLEEPTRAP;
                        break;

                    case 3:
                        ch = ARROWTRAP;
                        break;

                    case 4:
                        ch = TELTRAP;
                        break;

                    case 5:
                        ch = DARTTRAP;
                        break;

                    case 6:
                        ch = POOL;

                        if (rnd(10))
                            traps[i].tr_flags = ISFOUND;

                        break;

                    case 7:
                        ch = MAZETRAP;
                        break;

                    case 8:
                        ch = FIRETRAP;
                        break;

                    case 9:
                        ch = POISONTRAP;
                        break;

                    case 10:
                        ch = RUSTTRAP;
                        break;
                }

                addch(ch);
                traps[i].tr_type = ch;
                traps[i].tr_show = FLOOR;
                traps[i].tr_pos = stairs;
            }
        }
    }

    do              /* Position hero */
    {
        rm = rnd_room();

        if (levtype != THRONE && (rooms[rm].r_flags & ISTREAS))
            continue;

        rnd_pos(&rooms[rm], &hero);
    }
    while(!(winat(hero.y, hero.x) == FLOOR &&
           DISTANCE(hero, stairs) > 16));

    oldrp = &rooms[rm]; /* Set the current room */
    player.t_oldpos = player.t_pos; /* Set the current position */

    if (levtype != POSTLEV && levtype != THRONE)
    {
        if (on(player, BLESSMAP) && rnd(5) == 0)
        {
            read_scroll(&player, S_MAP, ISNORMAL);

            if (rnd(3) == 0)
                turn_off(player, BLESSMAP);
        }

        if (player.t_ctype == C_THIEF || on(player, BLESSGOLD) && rnd(5) == 0)
        {
            read_scroll(&player, S_GFIND, ISNORMAL);

            if (rnd(3) == 0)
                turn_off(player, BLESSGOLD);
        }

        if (player.t_ctype == C_RANGER || on(player, BLESSFOOD) && rnd(5) == 0)
        {
            read_scroll(&player, S_FOODDET, ISNORMAL);

            if (rnd(3) == 0)
                turn_off(player, BLESSFOOD);
        }

        if (player.t_ctype == C_MAGICIAN || player.t_ctype == C_ILLUSION ||
            on(player, BLESSMAGIC) && rnd(5) == 0)
        {
            quaff(&player, P_TREASDET, ISNORMAL);

            if (rnd(3) == 0)
                turn_off(player, BLESSMAGIC);
        }

        if (player.t_ctype == C_DRUID || on(player, BLESSMONS) && rnd(5) == 0)
        {
            quaff(&player, P_MONSTDET, ISNORMAL);

            if (rnd(3) == 0)
                turn_off(player, BLESSMONS);
        }
        else if (player.t_ctype == C_CLERIC ||
            player.t_ctype == C_PALADIN || is_wearing(R_PIETY))
            undead_sense();
    }

    if (is_wearing(R_AGGR))
        aggravate();

    if (is_wearing(R_ADORNMENT) ||
        cur_armor != NULL && cur_armor->o_which == MITHRIL)
    {
        int greed = FALSE;

        for (item = mlist; item != NULL; item = next(item))
        {
            tp = THINGPTR(item);

            if (on(*tp, ISGREED))
            {
                turn_on(*tp, ISRUN);
                turn_on(*tp, ISMEAN);
                tp->t_ischasing = TRUE;
                tp->t_chasee = &player;
                greed = TRUE;
            }
        }

        if (greed)
            msg("An uneasy feeling comes over you.");
    }

    if (is_carrying(TR_PALANTIR))   /* Palantir shows all */
    {
        msg("The Palantir reveals all!");

        overlay(stdscr, cw);    /* Wizard mode 'f' command */
        overlay(mw, cw);        /* followed by 'm' command */
    }

    if (is_carrying(TR_PHIAL))  /* Phial lights your way */
    {
        if (!is_carrying(TR_PALANTIR))
            msg("The Phial banishes the darkness!");

        for (i = 0; i < MAXROOMS; i++)
            rooms[i].r_flags &= ~ISDARK;
    }

    if (is_carrying(TR_AMULET)) /* Amulet describes the level */
    {
        level_eval();
    }


    wmove(cw, hero.y, hero.x);
    waddch(cw, PLAYER);
    light(&hero);

    /* Summon familiar if player has one */

    if (on(player, HASFAMILIAR))
    {
        summon_monster((short) fam_type, FAMILIAR, MESSAGE);

        if (fam_ptr != NULL)    /* add old pack to new */
        {
            tp = THINGPTR(fam_ptr);

            if (tp->t_pack == NULL)
                tp->t_pack = fpack;
            else if (fpack != NULL)
            {
                for (item = tp->t_pack; item->l_next != NULL;item = next(item))
                    ;

                item->l_next = fpack;
                debug("new_level: l_prev = %p",item);
                fpack->l_prev = item;
            }
        }
        else
            free_list(fpack);
    }

    mem_check(__FILE__,__LINE__);
    status(TRUE);
}

/*
    put_things()
        put potions and scrolls on this level
*/

void
put_things(LEVTYPE ltype)
{
    int i, rm, cnt;
    struct linked_list  *item;
    struct object   *cur;
    int    got_unique = FALSE;
    int length, width, maxobjects;
    coord   tp;

    /*
     * Once you have found an artifact, the only way to get new stuff is
     * go down into the dungeon.
     */

    if (has_artifact && level < max_level && ltype != THRONE)
        return;

    /*
     * There is a chance that there is a treasure room on this level
     * Increasing chance after level 10
     */

    if (ltype != MAZELEV && rnd(50) < level - 10)
    {
        int n, j;
        struct room *rp;

        /* Count the number of free spaces */
        n = 0;      /* 0 tries */

        do
        {
            rp = &rooms[rnd_room()];
            width = rp->r_max.y - 2;
            length = rp->r_max.x - 2;
        }
        while(!((width * length <= MAXTREAS) || (n++ > MAXROOMS * 4)));

        /* Mark the room as a treasure room */

        rp->r_flags |= ISTREAS;

        /* Make all the doors secret doors */

        for (n = 0; n < rp->r_nexits; n++)
        {
            move(rp->r_exit[n].y, rp->r_exit[n].x);
            addch(SECRETDOOR);
        }

        /* Put in the monsters and treasures */

        for (j = 1; j < rp->r_max.y - 1; j++)
            for (n = 1; n < rp->r_max.x - 1; n++)
            {
                coord   trp;

                trp.y = rp->r_pos.y + j;
                trp.x = rp->r_pos.x + n;

                /* Monsters */

                if ((rnd(100) < (MAXTREAS * 100) /
                    (width * length)) &&
                    (mvwinch(mw, rp->r_pos.y + j,
                    rp->r_pos.x + n) == ' '))
                {
                    struct thing    *th;

                    /* Make a monster */

                    item = new_item(sizeof *th);
                    th = THINGPTR(item);

                    /*
                     * Put it there and aggravate it
                     * (unless it can escape) only put
                     * one UNIQUE per treasure room at
                     * most
                     */

                    if (got_unique)
                        new_monster(item, randmonster(NOWANDER, GRAB), &trp,
                            NOMAXSTATS);
                    else
                    {
                        new_monster(item, randmonster(NOWANDER, NOGRAB), &trp,
                            NOMAXSTATS);

                        if (on(*th, ISUNIQUE))
                            got_unique = TRUE;
                    }

                    turn_off(*th, ISFRIENDLY);
                    turn_on(*th, ISMEAN);

                    if (off(*th, CANINWALL))
                    {
                        th->t_ischasing = TRUE;
                        th->t_chasee = &player;
                        turn_on(*th, ISRUN);
                    }
                }

                /* Treasures */

                if ((rnd(100) < (MAXTREAS * 100) /
                    (width * length)) &&
                    (mvinch(rp->r_pos.y + j,
                    rp->r_pos.x + n) == FLOOR))
                {
                    item = new_thing();
                    cur = OBJPTR(item);
                    cur->o_pos = trp;
                    add_obj(item, trp.y, trp.x);
                }
            }
    }

    /* Do MAXOBJ attempts to put things on a level, maybe  */

    maxobjects = (ltype == THRONE) ? rnd(3 * MAXOBJ) + 35 : MAXOBJ;

    for (i = 0; i < maxobjects; i++)
        if (rnd(100) < 40 || ltype == THRONE)
        {
            /* Pick a new object and link it in the list */

            item = new_thing();
            cur = OBJPTR(item);

            /* Put it somewhere */

            cnt = 0;

            do
            {
                rm = rnd_room();
                rnd_pos(&rooms[rm], &tp);
            }
            while(!(winat(tp.y, tp.x) == FLOOR || cnt++ > 500));

            cur->o_pos = tp;
            add_obj(item, tp.y, tp.x);
        }

    /*
     * If he is really deep in the dungeon and he hasn't found an
     * artifact yet, put it somewhere on the ground
     */

    if (make_artifact())
    {
        item = new_item(sizeof *cur);
        cur = OBJPTR(item);
        new_artifact(-1, cur);
        cnt = 0;

        do
        {
            rm = rnd_room();
            rnd_pos(&rooms[rm], &tp);
        }
        while(!(winat(tp.y, tp.x) == FLOOR || cnt++ > 500));

        cur->o_pos = tp;
        add_obj(item, tp.y, tp.x);
    }
}

/*
    do_throne()
        Put a monster's throne room and monsters on the screen
*/

void
do_throne(int special)
{
    coord   mp;
    int save_level;
    int i;
    struct room *rp;
    struct thing    *tp;
    struct linked_list  *item;
    int throne_monster;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
    {
        rp->r_nexits = 0;   /* no exits */
        rp->r_flags = ISGONE;   /* kill all rooms */
    }

    rp = &rooms[0];     /* point to only room */
    rp->r_flags = 0;    /* this room NOT gone */
    rp->r_max.x = 40;
    rp->r_max.y = 10;   /* 10 * 40 room */
    rp->r_pos.x = (COLS - rp->r_max.x) / 2; /* center horizontal */
    rp->r_pos.y = 3;    /* 2nd line */
    draw_room(rp);      /* draw the only room */

    save_level = level;
    level = max(2 * level, level + roll(4, 6));

    if (special == 0)    /* Who has he offended? */
        do
            throne_monster = nummonst - roll(1, NUMSUMMON);
        while(!monsters[throne_monster].m_normal);
    else
        throne_monster = special;

    /* Create summoning monster */

    item = new_item(sizeof *tp);

    tp = THINGPTR(item);

    do
    {
        rnd_pos(rp, &mp);
    }
    while(mvwinch(stdscr, mp.y, mp.x) != FLOOR);

    new_monster(item, throne_monster, &mp, MAXSTATS);
    turn_on(*tp, CANSEE);
    turn_off(*tp, ISFRIENDLY);

    if (on(*tp, CANSUMMON)) /* summon his helpers */
        summon_help(tp, FORCE);
    else
    {
        for (i = roll(4, 10); i >= 0; i--)
        {
            item = new_item(sizeof *tp);
            tp = THINGPTR(item);

            do
            {
                rnd_pos(rp, &mp);
            }
            while(mvwinch(stdscr, mp.y, mp.x) != FLOOR);

            new_monster(item, randmonster(NOWANDER, NOGRAB), &mp, MAXSTATS);
            turn_on(*tp, CANSEE);
            turn_off(*tp, ISFRIENDLY);
        }
    }

    level = save_level + roll(2, 3);    /* send the hero down */
    aggravate();
}

/*
    create_lucifer()
        special surprise on the way back up create Lucifer
        with more than the usual god abilities
*/

void
create_lucifer(coord *stairs)
{
    struct linked_list  *item = new_item(sizeof(struct thing));
    struct thing    *tp = THINGPTR(item);

    new_monster(item, nummonst + 1, stairs, MAXSTATS);
    turn_on(*tp, CANINWALL);
    turn_on(*tp, CANHUH);
    turn_on(*tp, CANBLINK);
    turn_on(*tp, CANSNORE);
    turn_on(*tp, CANDISEASE);
    turn_on(*tp, NOCOLD);
    turn_on(*tp, TOUCHFEAR);
    turn_on(*tp, BMAGICHIT);
    turn_on(*tp, NOFIRE);
    turn_on(*tp, NOBOLT);
    turn_on(*tp, CANBLIND);
    turn_on(*tp, CANINFEST);
    turn_on(*tp, CANSMELL);
    turn_on(*tp, CANPARALYZE);
    turn_on(*tp, CANSTINK);
    turn_on(*tp, CANCHILL);
    turn_on(*tp, CANFRIGHTEN);
    turn_on(*tp, CANHOLD);
    turn_on(*tp, CANBRANDOM);
}
