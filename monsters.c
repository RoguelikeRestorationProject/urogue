/*
    monsters.c - File with various monster functions in it
 
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
    summon_monster()
        Summon a monster.
*/

struct linked_list *
summon_monster(int type, int familiar, int print_message)
{
    struct linked_list  *mp;
    struct thing    *tp;
    int   monster;

    if (familiar && !is_wearing(R_WIZARD) && off(player, CANSUMMON))
    {
        msg("Only spellcasters can summon familiars!");
        return(NULL);
    }

    if (type == 0)    /* Random monster modified by level */
    {
        int ndice = min(pstats.s_lvl, (nummonst - NUMSUMMON) / 8);

        monster = min(nummonst, roll(ndice, pstats.s_charisma));
        
		/*
         * if a familiar exists, and it is higher in level, make it
         * again 
         */

        if (fam_ptr != NULL) 
		{
            struct thing   *fp = THINGPTR(fam_ptr);

            monster = max(fp->t_index, monster);
        }
	}
    else
        monster = type;

    turn_on(player, SUMMONING);

    mp = creat_mons(&player, monster, NOMESSAGE);

    if (!mp)
    {
        msg("Summon failed.");
        turn_off(player, SUMMONING);
        return(NULL);
    }

    if (print_message == MESSAGE)
    {
        msg("A %s appears out of nowhere!", monsters[monster].m_name);

        if (familiar)
            msg("I am here to serve %s.", whoami);
        else
        {
            msg("My goodness, are you Yendor?");
            ++mons_summoned;
            debug("%d monsters now summoned.", mons_summoned);
        }
    }

    tp = THINGPTR(mp);
    turn_on(*tp, ISCHARMED);    /* Summoned monsters are always charmed */

    if (familiar)
    {
        int i;
        static const unsigned long fam_on[]= {ISREGEN,CANSHOOT,CANWIELD,HASARMOR,ISFAMILIAR,0};
        static const unsigned long fam_off[]={ISMEAN, ISHUH, ISINVIS,
                    CANSURPRISE, NOMOVE,
                    ISSLOW, ISSHADOW, ISGREED, ISFAST,
                    CANFLY, ISFLEE, 0};

        for (i = 0; fam_on[i]; i++)
            turn_on(*tp, fam_on[i]);

        for (i = 0; fam_off[i]; i++)
            turn_off(*tp, fam_off[i]);

        if (fam_ptr != NULL)    /* Get rid of old familiar */
        {
            struct thing    *fp = THINGPTR(fam_ptr);
            struct linked_list  *fpack = fp->t_pack;
            struct linked_list  *item;

            if (fpack != NULL)  /* Transfer pack */
            {
                if (tp->t_pack == NULL)
                    tp->t_pack = fpack;
                else
                {
                    for(item=tp->t_pack; item->l_next != NULL; item=next(item))
                        ;   /* find last item in list */

                    item->l_next = fpack;
                    fpack->l_prev = item;
                }
            }

            fpack = NULL;
            killed(NULL, fam_ptr, NOMESSAGE, NOPOINTS);
        }

        fam_ptr = mp;
        fam_type = monster;

        /* improve their abilities a bit */

        tp->t_stats.s_hpt += roll(2, pstats.s_lvl);
        tp->t_stats.s_lvl += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_arm -= roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_str += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_intel += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_wisdom += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_dext += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_const += roll(2, (pstats.s_lvl / 4) + 1);
        tp->t_stats.s_charisma += roll(2, (pstats.s_lvl / 4) + 1);
		
		/* some monsters do no damage by default */
		
		if (strcmp(tp->t_stats.s_dmg, "0d0") == 0)
            tp->t_stats.s_dmg = "1d8";
			
        tp->maxstats = tp->t_stats; /* structure assignment */
    }

    turn_off(player, SUMMONING);

    return(mp);
}

/*
     randmonster()
        wander - wandering monster allowed
        grab - a throne room monster allowed
*/

int
randmonster(int wander, int grab)
{
    int mons_number, cur_level, range, i;

    /* Do we want a merchant? */

    if (wander == WANDER && monsters[nummonst].m_wander && rnd(5000) < 3)
        return(nummonst);

    cur_level = level;
    range = 4 * NLEVMONS;
    i = 0;

    do
    {
        if (i++ > range * 10)   /* just in case all have be genocided */
        {
            i = 0;

            if (--cur_level <= 0)
                fatal("Rogue could not find a monster to make");
        }

        mons_number = NLEVMONS * (cur_level - 1) +
            (rnd(range) - (range - 1 - NLEVMONS));

        if (mons_number < 1)
            mons_number = rnd(NLEVMONS) + 1;
        else if (mons_number > nummonst - NUMSUMMON - 1)
        {
            if (grab == GRAB)
                mons_number = rnd(range + NUMSUMMON) +
                    (nummonst - 1) -
                    (range + NUMSUMMON - 1);
            else if (mons_number > nummonst - 1)
                mons_number = rnd(range) +
                (nummonst - NUMSUMMON - 1) -
                (range - 1);
        }
    }
    while (wander == WANDER ? !monsters[mons_number].m_wander ||
        !monsters[mons_number].m_normal :
        !monsters[mons_number].m_normal);

    return((short)mons_number);
}

/*
    new_monster()
        Pick a new monster and add it to the list
*/

void
new_monster(struct linked_list *item, int type, coord *cp, int max_monster)
{
    struct thing    *tp;
    struct monster  *mp;
    char    *ip, *hitp;
    int   i, min_intel, max_intel;
    int   num_dice, num_sides = 8, num_extra = 0;
    int eff_charisma = pstats.s_charisma;
    int eff_intel = pstats.s_intel;

    attach(mlist, item);
    tp = THINGPTR(item);
    tp->t_index = type;
    tp->t_wasshot = FALSE;
    tp->t_type = monsters[type].m_appear;
    tp->t_ctype = C_MONSTER;
    tp->t_no_move = 0;
    tp->t_doorgoal = -1;
    tp->t_pos = *cp;
    tp->t_oldpos = *cp;
    tp->t_oldch = CCHAR( mvwinch(cw, cp->y, cp->x) );
    mvwaddch(mw, cp->y, cp->x, tp->t_type);
    mp = &monsters[tp->t_index];

    /* Figure out monster's hit points */

    hitp = mp->m_stats.s_hpt;
    num_dice = atoi(hitp);

    if ((hitp = strchr(hitp, 'd')) != NULL)
    {
        num_sides = atoi(++hitp);

        if ((hitp = strchr(hitp, '+')) != NULL)
            num_extra = atoi(++hitp);
    }

    if (max_monster == MAXSTATS)
        tp->t_stats.s_hpt = num_dice * num_sides + num_extra;
    else
        tp->t_stats.s_hpt = roll(num_dice, num_sides) + num_extra;

    tp->t_stats.s_lvl = mp->m_stats.s_lvl;
    tp->t_stats.s_arm = mp->m_stats.s_arm;
    tp->t_stats.s_dmg = mp->m_stats.s_dmg;
    tp->t_stats.s_exp = mp->m_stats.s_exp + mp->m_add_exp * tp->t_stats.s_hpt;
    tp->t_stats.s_str = mp->m_stats.s_str;

    if (max_level > 30)
    {
        tp->t_stats.s_hpt += roll(4, (max_level - 60) * 2);
        tp->t_stats.s_lvl += roll(4, (max_level - 60) / 8);
        tp->t_stats.s_arm -= roll(2, (max_level - 60) / 8);
        tp->t_stats.s_str += roll(2, (max_level - 60) / 12);
        tp->t_stats.s_exp += roll(4, (max_level - 60) * 2) * mp->m_add_exp;
    }

    /*
     * just initailize others values to something reasonable for now
     * maybe someday will *really* put these in monster table
     */

    tp->t_stats.s_wisdom = 8 + rnd(4);
    tp->t_stats.s_dext = 8 + rnd(4);
    tp->t_stats.s_const = 8 + rnd(4);
    tp->t_stats.s_charisma = 8 + rnd(4);

    if (max_level > 45)
        tp->t_stats.s_dext += roll(2, (max_level - 50) / 8);

    /* Set the initial flags */

    for (i = 0; i < 16; i++)
        tp->t_flags[i] = 0;

    for (i = 0; i < 16; i++)
        turn_on(*tp, mp->m_flags[i]);

    /* suprising monsters don't always surprise you */

    if (!max_monster && on(*tp, CANSURPRISE) && rnd(100) < 20)
        turn_off(*tp, CANSURPRISE);

    /* If this monster is unique, genocide it */

    if (on(*tp, ISUNIQUE))
        mp->m_normal = FALSE;

    /* gods automatically get special abilities */

    if (on(*tp, ISGOD))
    {
        turn_on(*tp, CANFRIGHTEN);
        turn_on(*tp, CANCAST);
        turn_on(*tp, CANFLY);
        turn_on(*tp, CANBARGAIN);
        turn_on(*tp, ISLARGE);
        turn_on(*tp, CANTELEPORT);
        turn_on(*tp, CANSPEAK);
        turn_on(*tp, CANDARKEN);
        turn_on(*tp, CANSEE);
        turn_on(*tp, CANLIGHT);
        turn_on(*tp, BMAGICHIT);
    }

    tp->t_turn = TRUE;
    tp->t_pack = NULL;

    /* Figure intelligence */

    min_intel = atoi(mp->m_intel);

    if ((ip = (char *) strchr(mp->m_intel, '-')) == NULL)
        tp->t_stats.s_intel = min_intel;
    else
    {
        max_intel = atoi(++ip);

        if (max_monster)
            tp->t_stats.s_intel = max_intel;
        else
            tp->t_stats.s_intel = min_intel + rnd(max_intel - min_intel);
    }

    tp->t_stats.s_power = (rnd(tp->t_stats.s_lvl / 5) + 1) * tp->t_stats.s_intel;

    tp->maxstats = tp->t_stats; /* structure assignment */

    /* If the monster can shoot, it may have a weapon */

    if (on(*tp, CANSHOOT) && (max_monster || rnd(9) < 6))
    {
        struct linked_list  *thrower_item, *missile_item;
        struct object *thrower, *a_missile;

        thrower_item = new_item(sizeof *thrower);
        thrower = OBJPTR(thrower_item);
        carried_weapon(tp, thrower);

        missile_item = new_item(sizeof *a_missile);
        a_missile = OBJPTR(missile_item);
        carried_weapon(tp, a_missile);

        /* The monster may use a crossbow, sling, footbow, or an arrow */
        /* Take racial preferences into account */

        if ((strcmp(mp->m_name, "elf") == 0) ||
            (strcmp(mp->m_name, "noldor elf") == 0))
        {
            thrower->o_which = BOW;

            if (rnd(5) == 0)
                a_missile->o_which = SILVERARROW;
            else
                a_missile->o_which = ARROW;
        }
        else if ((strcmp(mp->m_name, "dwarf") == 0) ||
                (strcmp(mp->m_name, "kazad dwarf") == 0))
        {
            thrower->o_which = CROSSBOW;
            a_missile->o_which = BOLT;
        }
        else if (on(*tp, ISSMALL))
        {
            switch (rnd(3))
            {
                case 0:
                    thrower->o_which = SLING;
                    a_missile->o_which = BULLET;
                    break;
                default:
                    thrower->o_which = SLING;
                    a_missile->o_which = ROCK;
            }
        }
        else if (on(*tp, ISLARGE))
        {
            switch (rnd(4))
            {
                case 0:
                    thrower->o_which = CROSSBOW;
                    a_missile->o_which = BOLT;
                    break;

                case 1:
                    thrower->o_which = FOOTBOW;
                    a_missile->o_which = FBBOLT;
                    break;

                default:
                    thrower->o_which = BOW;

                    if (rnd(5) == 0)
                        a_missile->o_which = FLAMEARROW;
                    else
                        a_missile->o_which = ARROW;

                    break;
            }
        }
        else
        {
            switch (rnd(6))
            {
                case 1:
                    thrower->o_which = SLING;
                    a_missile->o_which = ROCK;
                    break;

                case 2:
                    thrower->o_which = CROSSBOW;
                    a_missile->o_which = BOLT;
                    break;

                case 3:
                    thrower->o_which = FOOTBOW;
                    a_missile->o_which = FBBOLT;
                    break;

                case 4:
                    thrower->o_which = BOW;
                    a_missile->o_which = ARROW;
                    break;

                default:
                    thrower->o_which = SLING;
                    a_missile->o_which = BULLET;
                    break;
            }
        }

        init_weapon(thrower, thrower->o_which);
        init_weapon(a_missile, a_missile->o_which);

        attach(tp->t_pack, thrower_item);
        attach(tp->t_pack, missile_item);
    }

    /* monsters that wield weapons */

    if (on(*tp, CANWIELD))
    {
        if (max_monster || rnd(3))
        {
            struct linked_list  *wield_item;
            struct object   *wielded;

            wield_item = new_item(sizeof *wielded);
            wielded = OBJPTR(wield_item);
            carried_weapon(tp, wielded);

            i = rnd(CLAYMORE - CLUB) + rnd(2 * tp->t_stats.s_lvl);
            i = min(i, CLAYMORE);
            wielded->o_which = i;
            init_weapon(wielded, wielded->o_which);

            /* Is it too heavy? */

            if (itemweight(wielded) > 8 * tp->t_stats.s_str)
                discard(wield_item);
            else
                attach(tp->t_pack, wield_item);
        }
    }

    if (is_wearing(R_AGGR))
        chase_it(cp, &player);
    else
    {
        turn_off(*tp, ISRUN);

        if (on(*tp, ISFLEE) && (rnd(4) == 0))
            turn_off(*tp, ISFLEE);

        if (rnd(luck) == 0)
            switch (player.t_ctype)
            {
                case C_MAGICIAN:
                case C_ILLUSION:
                    eff_intel = 2 * pstats.s_intel;
                    break;
                case C_DRUID:
                    eff_intel = 2 * pstats.s_intel;
                case C_RANGER:
                    eff_charisma = 2 * pstats.s_charisma;
                    break;
                case C_ASSASIN:
                case C_THIEF:
                case C_NINJA:
                    eff_charisma = pstats.s_charisma / 2;
                    break;
            }

        /* LOWFRIENDLY monsters might be friendly */

        i = roll(1,100);

        if (i == 0 || (on(*tp, LOWFRIENDLY) && i < eff_charisma) ||
            (on(*tp, MEDFRIENDLY) && i < 3 * eff_charisma) ||
            (on(*tp, HIGHFRIENDLY) && i < 5 * eff_charisma))
        {
            turn_on(*tp, ISFRIENDLY);
            turn_off(*tp, ISMEAN);
        }

        i = roll(1,100);

        if (i == 0 || (on(*tp, LOWCAST) && i < eff_intel) ||
            (on(*tp, MEDCAST) && i < 3 * eff_intel) ||
            (on(*tp, HIGHCAST) && i < 5 * eff_intel))
        {
            turn_on(*tp, CANCAST);
        }

        if (on(*tp, ISDISGUISE))
        {
            char    mch = 0;

            if (tp->t_pack != NULL)
                mch = (OBJPTR(tp->t_pack))->o_type;
            else
                switch (rnd(level > arts[0].ar_level ? 10 : 9))
                {
                    case 0: mch = GOLD;     break;
                    case 1: mch = POTION;   break;
                    case 2: mch = SCROLL;   break;
                    case 3: mch = FOOD;     break;
                    case 4: mch = WEAPON;   break;
                    case 5: mch = ARMOR;    break;
                    case 6: mch = RING;     break;
                    case 7: mch = STICK;    break;
                    case 8: mch = monsters[randmonster(NOWANDER, NOGRAB)].m_appear;
                                break;
                    case 9: mch = ARTIFACT; break;
                }

            tp->t_disguise = mch;
        }
    }
}

/*
    wanderer()
        A wandering monster has awakened and is headed for the player
*/

void
wanderer(void)
{
    int i, cnt = 0;
    struct room *hr = roomin(hero);
    struct linked_list  *item;
    struct thing    *tp;
    coord   cp;
    char    *loc;
    int which;

    /* Find a place for it -- avoid the player's room */

    do
    {
        do
        {
            cnt++;
            i = rnd_room();
        }
        while (!(hr != &rooms[i] || levtype == MAZELEV
               || levtype == THRONE || cnt > 5000));

        rnd_pos(&rooms[i], &cp);
    }
    while(!step_ok(cp.y, cp.x, NOMONST, NULL));

    /* Create a new wandering monster */

    item = new_item(sizeof *tp);
    which = randmonster(TRUE, FALSE);
    new_monster(item, which, &cp, FALSE);

    tp = THINGPTR(item);
    tp->t_pos = cp;     /* Assign the position to the monster */

    chase_it(&tp->t_pos, &player);

    i = rnd(7);

    if (on(*tp, ISSWARM) && i < 5)
        cnt = roll(2, 4);
    else if (on(*tp, ISFLOCK) && i < 5)
        cnt = roll(1, 4);
    else
        cnt = 0;

    for (i = 1; i <= cnt; i++)
    {
        struct linked_list  *ip = creat_mons(tp, which, NOMESSAGE);

        if (ip != NULL)
        {
            struct thing    *mp = THINGPTR(ip);

            if (on(*tp, ISFRIENDLY))
                turn_on(*mp, ISFRIENDLY);
            else
                turn_off(*mp, ISFRIENDLY);
        }
    }

    if (cnt > 0)
    {
        if (on(*tp, LOWCAST) || on(*tp, MEDCAST) || on(*tp, HIGHCAST))
            turn_on(*tp, CANCAST);

        tp->t_stats.s_hpt += roll(2, 8);
        tp->t_stats.s_lvl += roll(2, 3);
        tp->t_stats.s_arm -= roll(1, 6);
        tp->t_stats.s_str += roll(2, 3);
        tp->t_stats.s_intel += roll(2, 3);
        tp->t_stats.s_exp += roll(2, 8) * monsters[which].m_add_exp;
    }

    i = DISTANCE(cp, hero);

    if (i < 20)
        loc = "very close to you";
    else if (i < 400)
        loc = "nearby";
    else
        loc = "in the distance";

    if (wizard)
        msg("Started a wandering %s.", monsters[tp->t_index].m_name);
    else if (on(*tp, ISUNDEAD) && (player.t_ctype == C_CLERIC ||
            player.t_ctype == C_PALADIN || is_wearing(R_PIETY)))
        msg("You sense a new ungodly monster %s.", loc);
    else if (on(player, CANHEAR) || (player.t_ctype == C_THIEF &&
            rnd(20) == 0))
        msg("You hear a new %s moving %s.",
            monsters[tp->t_index].m_name, loc);
    else if (on(player, CANSCENT) || (player.t_ctype == C_THIEF &&
            rnd(20) == 0))
        msg("You smell a new %s %s.", monsters[tp->t_index].m_name,
            loc);
}

/*
    wake_monster
	
        what to do when the hero steps next to a monster
*/

struct linked_list *
wake_monster(int y, int x)
{
    struct thing    *tp;
    struct linked_list  *it;
    struct room *trp;
    char    *mname;

    if ((it = find_mons(y, x)) == NULL)
    {
        debug("Can't find monster in show.");
        return(NULL);
    }

    tp = THINGPTR(it);

    if ((good_monster(*tp)) || on(player, SUMMONING))
    {
        chase_it(&tp->t_pos, &player);
        turn_off(*tp, ISINVIS);
        turn_off(*tp, CANSURPRISE);
        return(it);
    }

    trp = roomin(tp->t_pos);   /* Current room for monster */
    mname = monsters[tp->t_index].m_name;

    /* Let greedy ones guard gold */

    if (on(*tp, ISGREED) && off(*tp, ISRUN))
        if ((trp != NULL) && (lvl_obj != NULL))
        {
            struct linked_list  *item;
            struct object   *cur;

            for (item = lvl_obj; item != NULL; item = next(item))
            {
                cur = OBJPTR(item);

                if ((cur->o_type == GOLD) &&
                    (roomin(cur->o_pos) == trp))
                {
                    /* Run to the gold */
                    tp->t_horde = cur;
                    turn_on(*tp, ISRUN);
                    turn_off(*tp, ISDISGUISE);
                    tp->t_ischasing = FALSE;
                    /* Make it worth protecting */
                    cur->o_count += roll(2, 3) * GOLDCALC;
                    break;
                }
            }
        }

    /*
     * Every time he sees mean monster, it might start chasing him unique
     * monsters always do
     */

    if (  (on(*tp, ISUNIQUE)) ||
          ( (rnd(100) > 33) &&
            on(*tp, ISMEAN) &&
            off(*tp, ISHELD) &&
            off(*tp, ISRUN) &&
            !is_stealth(&player) &&
            (off(player, ISINVIS) || on(*tp, CANSEE))
          )
       )
    {
        chase_it(&tp->t_pos, &player);
    }

    /* Handle gaze attacks */

    if (on(*tp, ISRUN) && cansee(tp->t_pos.y, tp->t_pos.x) &&
            off(player, ISINVIS))
    {
        if (on(*tp, CANHUH))    /* Confusion */
        {
            if (on(player, CANREFLECT))
            {
                msg("You reflect the bewildering stare of the %s.", mname);

                if (save_throw(VS_MAGIC, tp))
                {
                    msg("The %s is confused!", mname);
                    turn_on(*tp, ISHUH);
                }
                else
                    msg("The %s staggers for a moment.", mname);
            }
            else if (save(VS_MAGIC))
            {
                msg("You feel dizzy for a moment, but it quickly passes.");

                if (rnd(100) < 67)
                    turn_off(*tp, CANHUH);
            }
            else if (off(player, ISCLEAR))
            {
                if (off(player, ISHUH))
                {
                    light_fuse(FUSE_UNCONFUSE, 0, rnd(20) + HUHDURATION, AFTER);
                    msg("The %s's gaze has confused you.", mname);
                    turn_on(player, ISHUH);
                }
                else
                    lengthen_fuse(FUSE_UNCONFUSE, rnd(20) + HUHDURATION);
            }
        }

        if (on(*tp, CANSNORE))      /* Sleep */
        {
            if (on(player, CANREFLECT))
            {
                msg("You reflect the lethargic glance of the %s", mname);

                if (save_throw(VS_PARALYZATION, tp))
                {
                    msg("The %s falls asleep!", mname);
                    tp->t_no_move += SLEEPTIME;
                }
            }
            else if (no_command == 0 && !save(VS_PARALYZATION))
            {
                if (is_wearing(R_ALERT))
                    msg("You feel slightly drowsy for a moment.");
                else
                {
                    msg("The %s's gaze puts you to sleep.", mname);
                    no_command = SLEEPTIME;

                    if (rnd(100) < 50)
                        turn_off(*tp, CANSNORE);
                }
            }
        }

        if (on(*tp, CANFRIGHTEN))   /* Fear */
        {
            turn_off(*tp, CANFRIGHTEN);

            if (on(player, CANREFLECT))
            {
                msg("The %s sees its reflection. ", mname);

                if (save_throw(VS_MAGIC,tp))
                {
                    msg("The %s is terrified by its reflection!", mname);
                    turn_on(*tp, ISFLEE);
                }
            }
            else
            {
                if (!save(VS_WAND) && !(on(player, ISFLEE) &&
                       (player.t_chasee==tp)))
                {
                    if ((player.t_ctype != C_PALADIN) &&
                        off(player, SUPERHERO))
                    {
                        turn_on(player, ISFLEE);
                        player.t_ischasing = FALSE;
                        player.t_chasee    = tp;
                        msg("The sight of the %s terrifies you.", mname);
                    }
                    else
                        msg("My, the %s looks ugly.", mname);
                }
            }
        }

        if (on(*tp, LOOKSLOW))     /* Slow */
        {
            turn_off(*tp, LOOKSLOW);

            if (on(player, CANREFLECT))
            {
                msg("You reflect the mournful glare of the %s.", mname);

                if (save_throw(VS_MAGIC,tp))
                {
                    msg("The %s is slowing down!", mname);
                    turn_on(*tp, ISSLOW);
                }
            }
            else if (is_wearing(R_FREEDOM) || save(VS_MAGIC))
                msg("You feel run-down for a moment.");
            else
            {
                if (on(player, ISHASTE))    /* Already sped up */
                {
                    extinguish_fuse(FUSE_NOHASTE);
                    nohaste(NULL);
                }
                else
                {
                    msg("You feel yourself moving %sslower.",
                     on(player, ISSLOW) ? "even " : "");

                    if (on(player, ISSLOW))
                        lengthen_fuse(FUSE_NOSLOW, rnd(4) + 4);
                    else
                    {
                        turn_on(player, ISSLOW);
                        player.t_turn = TRUE;
                        light_fuse(FUSE_NOSLOW, 0, rnd(4) + 4, AFTER);
                    }
                }
            }
        }

        if (on(*tp, CANBLIND))  /* Blinding */
        {
            turn_off(*tp, CANBLIND);

            if (on(player, CANREFLECT))
            {
                msg("You reflect the blinding stare of the %s.", mname);

                if (save_throw(VS_WAND, tp))
                {
                    msg("The %s is blinded!", mname);
                    turn_on(*tp, ISHUH);
                }
            }
            else if (off(player, ISBLIND))
                if (save(VS_WAND) || is_wearing(R_TRUESEE) || is_wearing(R_SEEINVIS))
                    msg("Your eyes film over for a moment.");
                else
                {
                    msg("The gaze of the %s blinds you.", mname);
                    turn_on(player, ISBLIND);
                    light_fuse(FUSE_SIGHT, 0, rnd(30) + 20, AFTER);
                    look(FALSE);
                }
        }

        if (on(*tp, LOOKSTONE))  /* Stoning */
        {
            turn_off(*tp, LOOKSTONE);

            if (on(player, CANREFLECT))
            {
                msg("You reflect the flinty look of the %s.", mname);

                if (save_throw(VS_PETRIFICATION,tp))
                {
                    msg("The %s suddenly stiffens", mname);
                    tp->t_no_move += STONETIME;
                }
                else
                {
                    msg("The %s is turned to stone!", mname);
                    killed(&player, it, NOMESSAGE, POINTS);
                }
            }
            else
            {
                if (on(player, CANINWALL))
                    msg("The %s cannot focus on you.", mname);
                else
                {
                    msg("The gaze of the %s stiffens your limbs.", mname);

                    if (save(VS_PETRIFICATION))
                        no_command = STONETIME;
                    else if (rnd(100))
                        no_command = STONETIME * 3;
                    else
                    {
                        msg("The gaze of the %s petrifies you.", mname);
                        msg("You are turned to stone!!! --More--");
                        wait_for(' ');
                        death(D_PETRIFY);
                        return(it);
                    }
                }
            }
        }
    }

    /*
     * True Sight sees all Never see ISINWALL or CANSURPRISE See ISSHADOW
     * 80% See ISINVIS with See Invisibilty
     */

    if (off(player, CANTRUESEE) &&
        on(*tp, ISINWALL) || on(*tp, CANSURPRISE) ||
        (on(*tp, ISSHADOW) && rnd(100) < 80) ||
        (on(*tp, ISINVIS) && off(player, CANSEE)))
	{
	    /* 
	    TODO: incomplete - need to finish logic
	    int ch = mvwinch(stdscr, y, x); 
	    */
	}
	

    /* hero might be able to hear or smell monster if he can't see it */

    if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 ||
            on(player, CANHEAR)) && !cansee(tp->t_pos.y, tp->t_pos.x))
        msg("You hear a %s nearby.", mname);
    else if ((rnd(player.t_ctype == C_THIEF ? 40 : 200) == 0 ||
            on(player, CANSCENT)) && !cansee(tp->t_pos.y, tp->t_pos.x))
        msg("You smell a %s nearby.", mname);

    return(it);
}

/*
    genocide()
        wipe out hated monsters flags:    ISBLESSED, ISCURSED
*/

void
genocide(int flags)
{
    struct linked_list  *ip;
    struct thing    *mp;
    struct linked_list  *nip;
    int    which_monst;
    int    blessed = flags & ISBLESSED;
    int    cursed = flags & ISCURSED;

    while ((which_monst = get_monster_number("genocide")) == 0)
        ;

    if (cursed)    /* oops... */
    {
        new_level(THRONE, which_monst);
        msg("What's this I hear about you trying to wipe me out?");
        fighting = running = after = FALSE;
        return;
    }

    /* Remove this monster from the present level */

    for (ip = mlist; ip; ip = nip)
    {
        mp = THINGPTR(ip);
        nip = next(ip);

        if (mp->t_index == which_monst)
        {
            check_residue(mp);  /* Check for special features before removing */
            remove_monster(&mp->t_pos, ip);
        }
    }

    /* Remove from available monsters */

    monsters[which_monst].m_normal = FALSE;
    monsters[which_monst].m_wander = FALSE;
    mpos = 0;
    msg("You have wiped out the %s.", monsters[which_monst].m_name);

    if (blessed)
        genocide(ISNORMAL);
}


/*
    id_monst()
        lists the monsters with the displayed by the character unless
        there is only one in which case it is returned as the string
*/

void
id_monst(int monster)
{
    int i;

    for (i = 1; i <= nummonst + 2; i++)
        if (monsters[i].m_appear == monster)
            add_line("A %s ", monsters[i].m_name);

    end_line();
}


/*
    check_residue()
        takes care of any effect of the monster
*/

void
check_residue(struct thing *tp)
{
    /* Take care of special abilities */

    if (on(*tp, DIDHOLD) && (--hold_count == 0))
        turn_off(player, ISHELD);

    /* If it has lowered player, give him back a level, maybe */

    if (on(*tp, DIDDRAIN) && rnd(3) == 0)
        raise_level();

    /* If frightened of this monster, stop */

    if (on(player, ISFLEE) && (player.t_chasee==tp))
        turn_off(player, ISFLEE);

    /* If monster was suffocating player, stop it */
    if (on(*tp, DIDSUFFOCATE))
        extinguish_fuse(FUSE_SUFFOCATE);

    /* If something with fire, may darken */
    if (on(*tp, HASFIRE))
    {
        struct room *rp = roomin(tp->t_pos);

        if (rp && (--(rp->r_fires) <= 0))
        {
            rp->r_flags &= ~HASFIRE;
            light(&tp->t_pos);
        }
    }
}

/*
    sell()
        displays a menu of goods from which the player may choose to
        purchase something.
*/

#define SELL_ITEMS 10       /* How many things 'q' might carry */

void
sell(struct thing *tp)
{
    struct linked_list  *item;
    int i, j, min_worth, nitems, chance, which_item, w;
    char goods;
    struct object   *obj;
    char    buffer[2 * LINELEN];
    char    dbuf[2 * LINELEN];

    struct
    {
        int which;
        int plus1, plus2;
        int count;
        int worth;
        int flags;
        char    *name;
    }
    selection[SELL_ITEMS];

    int effective_purse = ((player.t_ctype == C_PALADIN) ?
                   (9 * purse / 10) : purse);

    min_worth = -1;     /* hope item is never worth less than this */
    item = find_mons(tp->t_pos.y, tp->t_pos.x); /* Get pointer to monster */

    /* Select the items */

    nitems = rnd(6) + 5;

    switch (rnd(6))
    {
        /* Armor */
        case 0:
        case 1:
            goods = ARMOR;
            for (i = 0; i < nitems; i++)
            {
                chance = rnd(100);

                for (j = 0; j < maxarmors; j++)
                    if (chance < armors[j].a_prob)
                        break;

                if (j == maxarmors)
                {
                    debug("Picked a bad armor %d", chance);
                    j = 0;
                }

                selection[i].which = j;
                selection[i].count = 1;

                if (rnd(100) < 40)
                    selection[i].plus1 = rnd(5) + 1;
                else
                    selection[i].plus1 = 0;

                selection[i].name = armors[j].a_name;

                switch (luck)
                {
                    case 0: break;
                    case 1:
                        if (rnd(3) == 0)
                        {
                            selection[i].flags |=  ISCURSED;
                            selection[i].plus1 =  -1 - rnd(5);
                        }
                        break;

                    default:
                        if (rnd(luck))
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 =  -1 - rnd(5);
                        }
                        break;
                }

                /* Calculate price */

                w = armors[j].a_worth;
                w *= (1 + luck + (10 * selection[i].plus1));
                w = (w / 2) + (roll(6, w) / 6);
                selection[i].worth = max(w, 25);

                if (min_worth > selection[i].worth || i == 1)
                    min_worth = selection[i].worth;
            }
            break;

            /* Weapon */
        case 2:
        case 3:
            goods = WEAPON;
            for (i = 0; i < nitems; i++)
            {
                selection[i].which = rnd(maxweapons);
                selection[i].count = 1;

                if (rnd(100) < 35)
                {
                    selection[i].plus1 = rnd(3);
                    selection[i].plus2 = rnd(3);
                }
                else
                {
                    selection[i].plus1 = 0;
                    selection[i].plus2 = 0;
                }

                if (weaps[selection[i].which].w_flags & ISMANY)
                    selection[i].count = rnd(15) + 8;

                selection[i].name = weaps[selection[i].which].w_name;

                switch (luck)
                {
                    case 0: break;
                    case 1:
                        if (rnd(3) == 0)
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 =  -rnd(3);
                            selection[i].plus2 =  -rnd(3);
                        }
                        break;

                    default:
                        if (rnd(luck))
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 =  -rnd(3);
                            selection[i].plus2 =  -rnd(3);
                        }
                        break;
                }

                w = weaps[selection[i].which].w_worth * selection[i].count;
                w *= (1 + luck + (10 * selection[i].plus1 +
                          10 * selection[i].plus2));
                w = (w / 2) + (roll(6, w) / 6);
                selection[i].worth = max(w, 25);

                if (min_worth > selection[i].worth || i == 1)
                    min_worth = selection[i].worth;
            }
            break;

            /* Staff or wand */
        case 4:
            goods = STICK;

            for (i = 0; i < nitems; i++)
            {
                selection[i].which = pick_one(ws_magic, maxsticks);
                selection[i].plus1 = rnd(11) + 5;
                selection[i].count = 1;
                selection[i].name = ws_magic[selection[i].which].mi_name;

                switch (luck)
                {
                    case 0: break;
                    case 1:
                        if (rnd(3) == 0)
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 = 1;
                        }
                        break;

                    default:
                        if (rnd(luck))
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 = 1;
                        }
                }

                w = ws_magic[selection[i].which].mi_worth;
                w += (luck + 1) * 20 * selection[i].plus1;
                w = (w / 2) + (roll(6, w) / 6);
                selection[i].worth = max(w, 25);

                if (min_worth > selection[i].worth || i == 1)
                    min_worth = selection[i].worth;
            }
            break;

            /* Ring */

        case 5:
            goods = RING;
            for (i = 0; i < nitems; i++)
            {
                selection[i].which = pick_one(r_magic, maxrings);
                selection[i].plus1 = rnd(2) + 1;
                selection[i].count = 1;

                if (rnd(100) < r_magic[selection[i].which].mi_bless + 10)
                    selection[i].plus1 += rnd(2) + 1;

                selection[i].name = r_magic[selection[i].which].mi_name;

                switch (luck)
                {
                    case 0: break;
                    case 1:
                        if (rnd(3) == 0)
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 =  -1 - rnd(2);
                        }
                        break;

                    default:
                        if (rnd(luck))
                        {
                            selection[i].flags |= ISCURSED;
                            selection[i].plus1 =  -1 - rnd(2);
                        }
                }

                w = r_magic[selection[i].which].mi_worth;

                switch(selection[i].which)
                {
                    case R_DIGEST:
                        if (selection[i].plus1 > 2)
                            selection[i].plus1 = 2;
                        else if (selection[i].plus1 < 1)
                            selection[i].plus1 = 1;
                    /* fall thru here to other cases */
                    case R_ADDSTR:
                    case R_ADDDAM:
                    case R_PROTECT:
                    case R_ADDHIT:
                    case R_ADDINTEL:
                    case R_ADDWISDOM:
                        if (selection[i].plus1 > 0)
                            w += selection[i].plus1 * 50;
                }

                w *= (1 + luck);
                w = (w / 2) + (roll(6, w) / 6);
                selection[i].worth = max(w, 25);

                if (min_worth > selection[i].worth * selection[i].count)
                    min_worth = selection[i].worth;
            }
    }

    /* See if player can afford an item */

    if (min_worth > effective_purse)
    {
        msg("The %s eyes your small purse and departs.",
            monsters[nummonst].m_name);

        /* Get rid of the monster */

        killed(NULL, item, NOMESSAGE, NOPOINTS);

        return;
    }

    /* Display the goods */

    msg("The %s shows you his wares.", monsters[nummonst].m_name);
    wstandout(cw);
    mvwaddstr(cw, 0, mpos, morestr);
    wstandend(cw);
    wrefresh(cw);
    wait_for(' ');
    msg("");
    clearok(cw, TRUE);
    touchwin(cw);

    wclear(hw);
    touchwin(hw);

    for (i = 0; i < nitems; i++)
    {
        if (selection[i].worth > effective_purse)
            continue;

        wmove(hw, i + 2, 0);
        sprintf(dbuf, "[%c] ", ('a' + i));

        switch(goods)
        {
            case ARMOR:
                strcat(dbuf, "Some ");
                break;
            case WEAPON:
                if (selection[i].count == 1)
                    strcat(dbuf, "A ");
                else
                {
                    sprintf(buffer, "%2d ", selection[i].count);
                    strcat(dbuf, buffer);
                }
                break;

            case STICK:
                strcat(dbuf, "A ");
                strcat(dbuf, ws_type[selection[i].which]);
                strcat(dbuf, " of ");
                break;

            case RING:
                strcat(dbuf, "A ring of ");
                break;
        }

        strcat(dbuf, selection[i].name);

        if (selection[i].count > 1)
            strcat(dbuf, "s");

        sprintf(buffer, "%-50s Price:  %d", dbuf, selection[i].worth);
        waddstr(hw, buffer);
    }

    sprintf(buffer, "Purse:  %d", purse);
    mvwaddstr(hw, nitems + 3, 0, buffer);
    mvwaddstr(hw, 0, 0, "How about one of the following goods? ");
    wrefresh(hw);

    /* Get rid of the monster */

    killed(NULL, item, NOMESSAGE, NOPOINTS);

    which_item = (short) ((readchar() & 0177) - 'a');

    while (which_item < 0 || which_item >= nitems ||
        selection[which_item].worth > effective_purse)
    {
        if (which_item == (short) ESCAPE - (short) 'a')
            return;

        mvwaddstr(hw, 0, 0, "Please enter one of the listed items: ");
        wrefresh(hw);
        which_item = (short) ((readchar() & 0177) - 'a');
    }

    if (purse > selection[which_item].worth)
         purse -= selection[which_item].worth;
    else
         purse = 0L;

    item = spec_item(goods, selection[which_item].which,
          selection[which_item].plus1, selection[which_item].plus2);

    obj = OBJPTR(item);

    if (selection[which_item].count > 1)
    {
        obj->o_count = selection[which_item].count;
        obj->o_group = ++group;
    }

    /* If a stick or ring, let player know the type */

    switch (goods)
    {
        case STICK: know_items[TYP_STICK][selection[which_item].which] = TRUE;
                    break;
        case RING:  know_items[TYP_RING][selection[which_item].which] = TRUE;
                    break;
    }

    if (add_pack(item, MESSAGE) == FALSE)
    {
        obj->o_pos = hero;
        fall(&player, item, TRUE, FALSE);
    }
}

void
carried_weapon(struct thing *owner, struct object *weapon)
{
    weapon->o_hplus = (rnd(4) < 3) ? 0 : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
    weapon->o_dplus = (rnd(4) < 3) ? 0 : (rnd(3) + 1) * ((rnd(3) < 2) ? 1 : -1);
    weapon->o_hplus += rnd(owner->t_stats.s_lvl / 3 + 1);
    weapon->o_hplus += rnd(owner->t_stats.s_lvl / 3 + 1);
    weapon->o_damage = weapon->o_hurldmg = "0d0";
    weapon->o_ac = 11;
    weapon->o_count = 1;
    weapon->o_group = 0;

    if ((weapon->o_hplus <= 0) && (weapon->o_dplus <= 0))
        weapon->o_flags = ISCURSED;

    weapon->o_flags = 0;
    weapon->o_type = WEAPON;
    weapon->o_mark[0] = '\0';
}
