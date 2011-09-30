/*
    artifact.c  -  functions for dealing with artifacts

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <stdlib.h>
#include <ctype.h>
#include "rogue.h"

/*
    apply()
        apply an artifact
*/

void
apply(void)
{
    struct linked_list  *item;
    struct object   *obj;
    int which;
    int chance;

    if ((item = get_item("activate", ARTIFACT)) == NULL)
        return;

    obj = OBJPTR(item);
    which = obj->o_which;

    if (!(obj->ar_flags & ISACTIVE))
    {
        chance = rnd(100) - 10 * rnd(luck);
        debug("Rolled %d.", chance);
        if (chance < 5)
            do_major();
        else if (chance < 50)
            do_minor(obj);
        else
            obj->ar_flags |= ISACTIVE;
    }

    if (obj->ar_flags & ISACTIVE)
    {
        switch (which)
        {
            case TR_PURSE:    do_bag(obj);
                              break;
            case TR_PHIAL:    do_phial();
                              break;
            case TR_AMULET:   do_amulet();
                              break;
            case TR_PALANTIR: do_palantir();
                              break;
            case TR_CROWN:    do_crown();
                              break;
            case TR_SCEPTRE:  do_sceptre();
                              break;
            case TR_SILMARIL: do_silmaril();
                              break;
            case TR_WAND:     do_wand();
                              break;
            default:          nothing_message(ISCURSED);
                              return;
        }
    }

    if (rnd(pstats.s_lvl) < 6)
        do_minor(obj);

    turn_on(player, POWEREAT);
}

/*
    possessed(int artifact)
        was the hero carrying a particular artifact
*/

int
possessed(int artifact)
{
    return (picked_artifact >> artifact) & 1;
}

/*
    is_carrying(int artifact)
        is the hero carrying a particular artifact
*/

int
is_carrying(int artifact)
{
    return (has_artifact >> artifact) & 1;
}

/*
    make_artifact()
        is it time to make a new artifact?
*/

int
make_artifact(void)
{
    int i;

    mpos = 0;

    debug("Artifact possession and picked flags : %x %x.",
        has_artifact, picked_artifact);

    for(i = 0; i < maxartifact; i++)
    {
       if (!is_carrying(i) && arts[i].ar_level <= level)
           return TRUE;
    }

    return FALSE;
}

/*
    new_artifact(int which, struct object *cur)
        make a specified artifact
*/

struct object *
new_artifact(int which, struct object *cur)
{
    if (which >= maxartifact)
    {
        debug("Bad artifact %d.  Random one created.", which);
        which = rnd(maxartifact);
    }

    if (which < 0)
    {
        for (which = 0; which < maxartifact; which++)
            if (!is_carrying(which) && arts[which].ar_level <= level)
                break;
    }

    debug("Artifact number: %d.", which);

    cur->o_hplus = cur->o_dplus = 0;
    cur->o_damage = cur->o_hurldmg = "0d0";
    cur->o_ac = 11;
    cur->o_mark[0] = '\0';
    cur->o_type = ARTIFACT;
    cur->o_which = which;
    cur->o_weight = arts[which].ar_weight;
    cur->o_flags = 0;
    cur->o_group = 0;
    cur->o_count = 1;
    cur->o_bag = NULL;
    cur->ar_flags = 0;

    return(cur);
}

/*
    do_minor(struct object *tr)
        side effects and minor malevolent effects of artifacts
*/

void
do_minor(struct object *tr)
{
    int which;
    long loss;

    which = rnd(110);

    debug("Rolled %d.", which);

    switch (which)
    {
        case 0:
            seemsg("You develop some acne on your face.");
            break;

        case 1:
            if (on(player, CANSCENT))
            {
                msg("A sudden whiff of BO causes you to faint.");
                no_command = STONETIME;
            }
            else if (off(player, ISUNSMELL))
                msg("You begin to smell funny.");
            break;

        case 2:
            seemsg("A wart grows on the end of your nose.");
            break;

        case 3:
            hearmsg("Your hear strange noises in the distance.");
            break;

        case 4:
            hearmsg("You hear shuffling in the distance.");
            break;

        case 5:
            hearmsg("You hear clanking in the distance.");
            break;

        case 6:
            hearmsg("You hear water dripping onto the floor.");
            break;

        case 7:
            hearmsg("The dungeon goes strangely silent.");
            break;

        case 8:
            msg("You suddenly feel very warm.");
            break;

        case 9:
            msg("You feel very hot.");
            break;

        case 10:
            msg("A blast of heat hits you.");
            break;

        case 11:
            {
                 struct room *rp;

                 if (off(player, ISBLIND))
                    msg("A pillar of flame leaps up beside you.");
                 else
                    msg("You feel something very hot nearby.");

                 if (ntraps + 1 < 2 * MAXTRAPS &&
                     fallpos(hero, &traps[ntraps].tr_pos))
                 {
                    mvaddch(traps[ntraps].tr_pos.y, traps[ntraps].tr_pos.x,
                       FIRETRAP);
                    traps[ntraps].tr_type = FIRETRAP;
                    traps[ntraps].tr_flags = ISFOUND;
                    traps[ntraps].tr_show = FIRETRAP;
                    ntraps++;

                    if ((rp = roomin(hero)) != NULL)
                    {
                        rp->r_flags &= ~ISDARK;
                        light(&hero);
                        mvwaddch(cw, hero.y, hero.x, PLAYER);
                    }
                }
            }
            break;

        case 12:
            msg("You feel a blast of hot air.");
            break;

        case 13:
            msg("You feel very cold.");
            break;

        case 14:
            msg("You break out in a cold sweat.");
            break;

        case 15:
            if (off(player, ISBLIND) && cur_armor == NULL)
                msg("You are covered with frost.");
            else if (off(player, ISBLIND))
                msg("Your armor is covered with frost.");
            else if (cur_armor == NULL)
                msg("Your body feels very cold and you begin to shiver.");
            else
                msg("Your armor feels very cold.  You hear cracking ice.");
            break;

        case 16:
            msg("A cold wind whistles through the dungeon.");
            break;

        case 17:
            {
                int change;

                change = 18 - pstats.s_str;
                chg_str(change, TRUE, FALSE);
                chg_dext(-change, TRUE, FALSE);

                if (change > 0)
                    msg("You feel stronger and clumsier now.");
                else if (change < 0)
                    msg("You feel weaker and more dextrous now.");
                else
                    nothing_message(ISCURSED);
            }
            break;

        case 18:
            msg("You begin to itch all over.");
            break;

        case 19:
            msg("You begin to feel hot and itchy.");
            break;

        case 20:
            msg("You feel a burning itch.");
            chg_dext(-1, FALSE, TRUE);

            if (off(player, HASITCH))
            {
                turn_on(player, HASITCH);
                light_fuse(FUSE_UNITCH, 0, roll(4,6), AFTER);
            }
            else
                lengthen_fuse(FUSE_UNITCH, roll(4,6));
            break;

        case 21:
            if (off(player, ISBLIND))
               msg("Your skin begins to flake and peel.");
            else
               msg("You feel an urge to scratch an itch.");
            break;


        case 22:
            seemsg("Your hair begins to turn grey.");
            break;

        case 23:
            seemsg("Your hair begins to turn white.");
            break;

        case 24:
            seemsg("Some of your hair instantly turns white.");
            break;

        case 25:
            seemsg("You are covered with long white hair.");
            break;

        case 26:
            seemsg("You are covered with long red hair.");
            break;

        case 27:
            msg("You grow a beard.");
            break;

        case 28:
            msg("Your hair falls out.");
            break;

        case 29:
            msg("You feel a burning down below.");
            break;

        case 30:
            msg("Your toes fall off.");
            break;

        case 31:
            msg("You grow some extra toes.");
            break;

        case 32:
            msg("You grow some extra fingers.");
            break;

        case 33:
            msg("You grow an extra thumb.");
            break;

        case 34:
            msg("Your nose falls off.");
            break;

        case 35:
            msg("Your nose gets bigger.");
            break;

        case 36:
            msg("Your nose shrinks.");
            break;

        case 37:
            msg("An eye grows on your forehead.");
            break;

        case 38:
            seemsg("You see beady eyes watching from a distance.");
            break;

        case 39:
            msg("The dungeon rumbles for a moment.");
            break;

        case 40:
            seemsg("A flower grows on the floor next to you.");
            break;

        case 41:
            msg("You are stunned by a psionic blast.");

            if (on(player, ISHUH))
                lengthen_fuse(FUSE_UNCONFUSE, rnd(40) + (HUHDURATION * 3));
            else
            {
                light_fuse(FUSE_UNCONFUSE,0,rnd(40)+(HUHDURATION * 3), AFTER);
                turn_on(player, ISHUH);
            }
            break;

        case 42:
            msg("You are confused by thousands of voices in your head.");

            if (on(player, ISHUH))
                lengthen_fuse(FUSE_UNCONFUSE, rnd(10) + (HUHDURATION * 2));
            else
            {
                light_fuse(FUSE_UNCONFUSE,0,rnd(10)+(HUHDURATION * 2), AFTER);
                turn_on(player, ISHUH);
            }
            break;

        case 43:
            hearmsg("You hear voices in the distance.");
            break;

        case 44:
            msg("You feel a strange pull.");
            teleport();

            if (off(player, ISCLEAR))
            {
                if (on(player, ISHUH))
                    lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
                else
                {
                    light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);
                    turn_on(player, ISHUH);
                }
            }
            break;

        case 45:
            msg("You feel less healthy now.");
            pstats.s_const = max(pstats.s_const - 1, 3);
            max_stats.s_const = max(max_stats.s_const - 1, 3);
            break;

        case 46:
            msg("You feel weaker now.");
            chg_str(-1, TRUE, FALSE);
            break;

        case 47:
            msg("You feel less wise now.");
            pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
            max_stats.s_wisdom = max(max_stats.s_wisdom - 1, 3);
            break;

        case 48:
            msg("You feel less dextrous now.");
            chg_dext(-1, TRUE, FALSE);
            break;

        case 49:
            msg("You feel less intelligent now.");
            pstats.s_intel = max(pstats.s_intel - 1, 3);
            max_stats.s_intel = max(max_stats.s_intel - 1, 3);
            break;

        case 50:
            msg("A trap door opens underneath your feet.");
            mpos = 0;
            level++;
            new_level(NORMLEV,0);

            if (rnd(4) < 2)
            {
                addmsg("You are damaged by the fall");

                if ((pstats.s_hpt -= roll(1, 6)) <= 0)
                {
                    addmsg("!  The fall killed you.");
                    endmsg();
                    death(D_FALL);
                    return;
                }
            }

            addmsg("!");
            endmsg();

            if (off(player, ISCLEAR) && rnd(4) < 3)
            {
                if (on(player, ISHUH))
                    lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
                else
                    light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);

                turn_on(player, ISHUH);
            }
            else
                msg("You feel dizzy for a moment, but it quickly passes.");

            break;

        case 51:
            msg("A maze entrance opens underneath your feet.");
            mpos = 0;
            level++;
            new_level(MAZELEV,0);

            if (rnd(4) < 2)
            {
                addmsg("You are damaged by the fall");

                if ((pstats.s_hpt -= roll(1, 6)) <= 0)
                {
                    addmsg("!  The fall killed you.");
                    endmsg();
                    death(D_FALL);
                    return;
                }
            }
            addmsg("!");
            endmsg();

            if (off(player, ISCLEAR) && rnd(4) < 3)
            {
                if (on(player, ISHUH))
                    lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
                else
                    light_fuse(FUSE_UNCONFUSE,0, rnd(8) + HUHDURATION, AFTER);

                turn_on(player, ISHUH);
            }
            else
                msg("You feel dizzy for a moment, but it quickly passes.");

            break;

        case 52:
            hearmsg("You hear a wailing sound in the distance.");
            aggravate();
            break;

        case 53:
            read_scroll(&player, S_HOLD, ISCURSED);
            break;

        case 54:
            msg("You can't move.");
            no_command = 3 * HOLDTIME;
            break;

        case 55:
            hearmsg("You hear a buzzing sound.");
            aggravate();
            break;

        case 56:
            msg("Your limbs stiffen.");
            no_command = 3 * STONETIME;
            break;

        case 57:
            msg("You feel a rock in your shoe hurting your foot.");
            turn_on(player, STUMBLER);
            break;

        case 58:
            msg("You get a hollow feeling in your stomach.");
            food_left -= 500;
            break;

        case 59:
            msg("Your purse feels lighter.");

            loss  = 50L + ulrnd(purse / 2L);
            purse = (purse > loss) ? purse - loss : 0L;
            break;

        case 60:
            msg("A pixie appears and grabs gold from your purse.");

            loss = 50L + rnd(50);
            purse = (purse > loss) ? purse - loss : 0L;
            break;

        case 61:
            msg("You feel a tingling sensation all over.");
            pstats.s_hpt -= ulrnd(pstats.s_hpt / 3L);
            break;

        case 62:
            msg("You feel a pull downwards.");
            break;

        case 63:
            msg("You feel a strange pull downwards.");
            break;

        case 64:
            msg("You feel a peculiar pull downwards.");
            break;

        case 65:
            msg("You have a strange urge to go down.");
            break;

        case 66:
            msg("You feel a pull upwards.");
            break;

        case 67:
            msg("You feel a strange pull upwards.");
            break;

        case 68:
            msg("You have a strange feeling for a moment.");
            break;

        case 69:
            msg("You float in the air for a moment.");
            break;

        case 70:
            msg("You feel very heavy for a moment.");
            break;

        case 71:
            msg("You feel a strange sense of loss.");
            break;

        case 72:
            msg("You feel the earth spinning underneath your feet.");
            break;

        case 73:
            msg("You feel in touch with a Universal Oneness.");
            break;

        case 74:
            hearmsg("You hear voices in the distance.");
            break;

        case 75:
            msg("A strange feeling of power comes over you.");
            break;

        case 76:
            msg("You feel a strange sense of unease.");
            break;

        case 77:
            msg("You feel Lady Luck is looking the other way.");
            luck++;
            break;

        case 78:
            msg("You feel your pack vibrate for a moment.");
            break;

        case 79:
            msg("You feel someone is watching you.");
            break;

        case 80:
            msg("You feel your hair standing on end.");
            break;

        case 81:
            msg("Wait!  The walls are moving!");
            new_level(NORMLEV,0);
            break;

        case 82:
            msg("Wait!  Walls are appearing out of nowhere!");
            new_level(MAZELEV,0);
            break;

        case 83:
            blue_light(ISCURSED);
            break;

        case 84:
            msg("Your mind goes blank for a moment.");
            wclear(cw);
            light(&hero);
            status(TRUE);
            break;

        case 85:
            if (on(player, ISDEAF))
            {
                msg("You feel your ears burn for a moment.");
                lengthen_fuse(FUSE_HEAR, 2 * PHASEDURATION);
            }
            else
            {
                msg("You are suddenly surrounded by silence.");
                turn_on(player, ISDEAF);
                light_fuse(FUSE_HEAR, 0, 2 * PHASEDURATION, AFTER);
            }
            break;

        case 86:
            {
                apply_to_bag(pack, 0, NULL, baf_curse, NULL);

                if (off(player, ISUNSMELL))
                    msg("You smell a faint trace of burning sulfur.");
            }
            break;

        case 87:
            msg("You have contracted a parasitic infestation.");
            infest_dam++;
            turn_on(player, HASINFEST);
            break;

        case 88:
            msg("You suddenly feel a chill run up and down your spine.");
            turn_on(player, ISFLEE);
            player.t_ischasing = FALSE;
            player.t_chasee = &player;
            break;

        case 89:
            if (cur_weapon != NULL)
                msg("You feel your %s get very hot.",
                    inv_name(cur_weapon, LOWERCASE));
            break;

        case 90:
            if (cur_weapon != NULL)
                msg("Your %s glows white for an instant.",
                    inv_name(cur_weapon, LOWERCASE));
            break;

        case 91:
            if (cur_armor != NULL)
                msg("Your %s gets very hot.", inv_name(cur_armor, LOWERCASE));
            break;

        case 92:
            if (cur_weapon != NULL)
                msg("Your %s suddenly feels very cold.",
                    inv_name(cur_weapon, LOWERCASE));
            break;

        case 93:
            if (cur_armor != NULL)
                msg("Your armor is covered by an oily film.");
            break;

        case 94:
            read_scroll(&player, S_CREATE, ISNORMAL);
            break;

        case 95:
            lower_level(D_POTION);
            break;

        case 96:
            {
                int x, y;

                for (x = -1; x <= 1; x++)
                {
                    for (y = -1; y <= 1; y++)
                    {
                        if (x == 0 && y == 0)
                            continue;

                        delta.x = x;
                        delta.y = y;

                        do_zap(&player, WS_POLYMORPH, rnd(2)
                            ? ISCURSED : ISNORMAL);
                    }
                }
            }
            break;

        case 97:
            {
                int x, y;

                for (x = -1; x <= 1; x++)
                {
                    for (y = -1; y <= 1; y++)
                    {
                        if (x == 0 && y == 0)
                            continue;

                        delta.x = x;
                        delta.y = y;

                        do_zap(&player, WS_INVIS, ISNORMAL);
                    }
                }
            }
            break;

        default:
            tr->ar_flags &= ~ISACTIVE;
            hearmsg("You hear a click coming from %s.",inv_name(tr,LOWERCASE));
            break;

    }
}

/*
    do_major()

        major malevolent effects

        0.  read_scroll(S_SELFTELEPORT, ISCURSED)
        1.  PERMBLIND for twice normal duration
        2.  new_level(THRONE);
        3.  turn_on(player, SUPEREAT);
        4.  lengthen(noslow, 20 + rnd(20));
        5.  lower_level(D_POTION) * roll(1,4)
        6.  change stats
        7.  FIRETRAP
        8.  armor crumbles
        9.  weapon crumbles
       10. weapon crumbles
       11. curse weapon
*/

void
do_major(void)
{
    int which;

    which = rnd(12);

    debug("Rolled %d.", which);

    switch (which)
    {
        case 0:
            read_scroll(&player, S_SELFTELEP, ISCURSED);
            break;

        case 1:
            quaff(&player, P_TRUESEE, ISCURSED);
            quaff(&player, P_TRUESEE, ISCURSED);
            break;

        case 2:
            new_level(THRONE,0);
            break;

        case 3: /* Turn off other body-affecting spells */

            if (on(player, ISREGEN))
            {
                extinguish_fuse(FUSE_UNREGEN);
                turn_off(player, ISREGEN);
                unregen(NULL);
            }

            if (on(player, NOCOLD))
            {
                extinguish_fuse(FUSE_UNCOLD);
                turn_off(player, NOCOLD);
                uncold(NULL);
            }

            if (on(player, NOFIRE))
            {
                extinguish_fuse(FUSE_UNHOT);
                turn_off(player, NOFIRE);
                unhot(NULL);
            }

            if (on(player, SUPEREAT))
            {
                lengthen_fuse(FUSE_UNSUPEREAT, 2 * PHASEDURATION);
                msg("Your body temperature rises still further.");
            }
            else
            {
                msg("You feel very warm all over.");
                light_fuse(FUSE_UNSUPEREAT, 0, 2 * PHASEDURATION, AFTER);
                turn_on(player, SUPEREAT);
            }
            break;

        case 4:
            msg("You feel yourself moving %sslower.",
            on(player, ISSLOW) ? "even " : "");

            if (on(player, ISSLOW))
                lengthen_fuse(FUSE_NOSLOW, PHASEDURATION);
            else
            {
                turn_on(player, ISSLOW);
                player.t_turn = TRUE;
                light_fuse(FUSE_NOSLOW, 0, PHASEDURATION, AFTER);
            }
            break;

        case 5:
            {
                int i, n = roll(1, 4);

                for (i = 1; i < n; i++)
                    lower_level(D_POTION);
            }
            break;

        case 6:
            if (rnd(2))
                add_intelligence(TRUE);

            if (rnd(2))
                chg_dext(-1, TRUE, FALSE);

            if (rnd(2))
                chg_str(-1, TRUE, FALSE);

            if (rnd(2))
                add_wisdom(TRUE);

            if (rnd(2))
                add_const(TRUE);

            break;

        case 7:
            {
                struct room *rp;

                if (ntraps + 1 >= MAXTRAPS)
                {
                    msg("You feel a puff of hot air.");
                    return;
                }

                for (; ntraps < 2 * MAXTRAPS; ntraps++)
                {
                    if (!fallpos(hero, &traps[ntraps].tr_pos))
                        break;

                    mvaddch(traps[ntraps].tr_pos.y, traps[ntraps].tr_pos.x,
                        FIRETRAP);
                    traps[ntraps].tr_type   = FIRETRAP;
                    traps[ntraps].tr_flags |= ISFOUND;
                    traps[ntraps].tr_show   = FIRETRAP;

                    if ((rp = roomin(hero)) != NULL)
                        rp->r_flags &= ~ISDARK;
                }
            }
            break;

        case 8:
            {
                object  *obj;

                if (cur_weapon == NULL)
                {
                    msg("You feel your hands tingle a moment.");
                    pstats.s_dmg = "1d2";
                    return;
                }

                obj = apply_to_bag(pack, 0, NULL, bafcweapon, NULL);

                if (obj->o_flags & ISMETAL)
                    msg("Your %s melts and disappears.",
                        inv_name(obj,LOWERCASE));
                else
                    msg("Your %s crumbles in your hands.",
                        inv_name(obj, LOWERCASE));

                obj->o_flags &= ~ISCURSED;
                dropcheck(obj);
                del_bag(pack, obj);

            }
            break;

        case 9:
            {
                object  *obj;

                if (cur_armor == NULL)
                {
                    msg("Your body tingles a moment.");
                    return;
                }

                obj = apply_to_bag(pack, 0, NULL, bafcarmor, NULL);

                msg("Your %s crumbles into small black powdery dust.",
                    inv_name(obj, LOWERCASE));

                obj->o_flags &= ~ISCURSED;
                dropcheck(obj);
                del_bag(pack, obj);
            }
            break;

        default:

            if (cur_weapon == NULL)
            {
                seemsg("Your hand glows yellow for an instant.");
                pstats.s_dmg = "1d3";
                return;
            }

            seemsg("Your %s glows bright red for a moment.",
                   weaps[cur_weapon->o_which].w_name);

            if (cur_weapon->o_hplus > 0)
                cur_weapon->o_hplus = -rnd(3);
            else
                cur_weapon->o_hplus -= rnd(3);

            if (cur_weapon->o_dplus > 0)
                cur_weapon->o_dplus = -rnd(3);
            else
                cur_weapon->o_dplus -= rnd(3);

            cur_weapon->o_flags = ISCURSED | ISLOST;
            cur_weapon->o_ac = 0;

            break;
    }
}

/*
    do_phial()
        handle powers of the Phial of Galadriel
*/

void
do_phial(void)
{
    int which;

    /* Prompt for action */

    msg("How do you wish to apply the Phial of Galadriel (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    if (which < 0 || which > 1)
    {
        add_line("[a] total healing");
        add_line("[b] total monster confusion");
        end_line();
        msg("");
        msg("Which power do you wish to use? ");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > 1)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("");
            msg("Please enter one of the listed powers: ");

            which = (short) ((readchar() & 0177) - 'a');
        }
        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successsful.");

    switch (which)
    {
        case 0:
            pstats.s_hpt = max_stats.s_hpt += rnd(pstats.s_lvl) + 1;
            pstats.s_power = max_stats.s_power += rnd(pstats.s_lvl) + 1;
            break;

        case 1:
            {
                struct linked_list  *mi;
                struct thing    *tp;

                for (mi = mlist; mi != NULL; mi = next(mi))
                {
                    tp = THINGPTR(mi);

                    if (off(*tp, ISUNIQUE) || !save_throw(VS_MAGIC, tp))
                        turn_on(*tp, ISHUH);
                }
            }
            break;

        default:
            msg("What a strange thing to do!!");
            break;

    }
}

/*
    do_palantir()
        handle powers of the Palantir of Might
*/

void
do_palantir(void)
{
    int which, limit;

    /* Prompt for action */

    msg("How do you wish to apply the Palantir of Might? (* for list): ");

    limit = 3;

    if (is_carrying(TR_SCEPTRE))
        limit += 1;

    if (is_carrying(TR_CROWN))
        limit += 1;

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    if (which < 0 || which > limit)
    {
        msg("");
        add_line("[a] monster detection");
        add_line("[b] gold detection");
        add_line("[c] magic detection");
        add_line("[d] food detection");

        if (limit >= 4)
            add_line("[e] teleportation");

        if (limit >= 5)
            add_line("[f] clear thought");

        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > limit)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }

        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    switch (which)
    {
        case 0: quaff(&player, P_MONSTDET, ISNORMAL);
                break;
        case 1: read_scroll(&player, S_GFIND, ISNORMAL);
                break;
        case 2: quaff(&player, P_TREASDET, ISNORMAL);
                break;
        case 3: read_scroll(&player, S_FOODDET, ISNORMAL);
                break;
        case 4: read_scroll(&player, S_SELFTELEP, ISNORMAL);
                break;
        case 5: quaff(&player, P_CLEAR, ISNORMAL);
                break;
        default:
                msg("What a strange thing to do!!");
                break;
    }
}

/*
    do_silmaril()
        handle powers of the Silamril of Ea
*/

void
do_silmaril(void)
{
    int which;

    /* Prompt for action */
    msg("How do you wish to apply the Silamril of Ea (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    if (which < 0 || which > 2)
    {
        msg("");
        add_line("[a] magic mapping");
        add_line("[b] petrification");
        add_line("[c] stairwell downwards");
        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > 2)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }
            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }
        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    switch (which)
    {
        case 0: read_scroll(&player, S_MAP, ISNORMAL);
                break;
        case 1: read_scroll(&player, S_PETRIFY, ISNORMAL);
                break;
        case 2: msg("A stairwell opens beneath your feet and you go down.");
                level++;
                new_level(NORMLEV,0);
                break;
        default:msg("What a strange thing to do!!");
                break;
    }
}

/*
    do_amulet()
        handle powers of the Amulet of Yendor
*/

void
do_amulet(void)
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Amulet of Yendor (* for list)? ");

    limit = 0;

    if (is_carrying(TR_PURSE))
        limit += 1;

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    if (which < 0 || which > limit)
    {
        msg("");
        add_line("[a] level evaluation");

        if (limit >= 1)
            add_line("[b] invisibility");

        end_line();
        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > limit)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }

        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    switch (which)
    {
        case 0: level_eval();
                break;
        case 1: quaff(&player, P_INVIS, ISNORMAL);
                break;
        default:msg("What a strange thing to do!!");
                break;
    }
}

/*
    do_bag()
        handle powers of the Magic Purse of Yendor as a bag of holding
*/

void
do_bag(struct object *obj)
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Magic Purse of Yendor (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    limit = 2;

    if (is_carrying(TR_AMULET))
        limit += 1;

    if (which < 0 || which > limit)
    {
        msg("");
        add_line("[a] inventory");
        add_line("[b] add to bag");
        add_line("[c] remove from bag");

        if (limit >= 3)
            add_line("[d] see invisible");

        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > limit)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }

        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    switch (which)
    {
        case 0:
            inventory(obj->o_bag, 0);
            break;

        case 1:
            {
                object  *new_obj_p; /* what the user selected */

                if ((new_obj_p = get_object(pack, "add", 0, NULL)) != NULL)
                {
                    rem_pack(new_obj_p);    /* free up pack slot */
                    push_bag(&obj->o_bag, new_obj_p);
                    pack_report(new_obj_p, MESSAGE, "You just added ");
                }
            }
            break;

        case 2:
            {
                object  *obj_p;
                linked_list *item_p;

                if ((obj_p=get_object(obj->o_bag,"remove",0,NULL)) != NULL)
                {
                    item_p = make_item(obj_p);  /* attach upper structure */

                    if (add_pack(item_p, MESSAGE) != FALSE)
                        pop_bag(&obj->o_bag, obj_p);
                }
            }
            break;

        case 3:
            quaff(&player, P_TRUESEE, ISBLESSED);
            break;

        default:
            msg("What a strange thing to do!!");
    }
}

/*
    do_sceptre()
        handle powers of the Sceptre of Might
*/

void
do_sceptre(void)
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Sceptre of Might (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    limit = 5;

    if (is_carrying(TR_CROWN))
        limit += 1;

    if (is_carrying(TR_PALANTIR))
        limit += 1;

    if (which < 0 || which > limit)
    {
        msg("");
        add_line("[a] cancellation");
        add_line("[b] polymorph monster");
        add_line("[c] slow monster");
        add_line("[d] teleport monster");
        add_line("[e] monster confusion");
        add_line("[f] paralyze monster");

        if (limit >= 6)
            add_line("[g] drain life");

        if (limit >= 7)
            add_line("[h] smell monster");

        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > limit)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }

        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 7)
    {
        msg("Your finger slips.");
        which = rnd(6);
        if (wizard)
        {
            msg("What wand? (%d)", which);

            if (get_string(prbuf, cw) == NORM)
            {
                which = atoi(prbuf);
                if (which < 0 || which > 5)
                {
                    msg("Invalid selection.");
                    which = rnd(6);
                    msg("Rolled %d.", which);
                }
            }
        }
    }

    switch (which)
    {
        case 0:
            if (get_dir())
                do_zap(&player, WS_CANCEL, ISBLESSED);
            break;

        case 1:
            if (get_dir())
                do_zap(&player, WS_POLYMORPH, ISBLESSED);
            break;

        case 2:
            if (get_dir())
                do_zap(&player, WS_SLOW_M, ISBLESSED);
            break;

        case 3:
            if (get_dir())
                do_zap(&player, WS_MONSTELEP, ISBLESSED);
            break;

        case 4:
            if (get_dir())
                do_zap(&player, WS_CONFMON, ISBLESSED);
            break;

        case 5:
            if (get_dir())
                do_zap(&player, WS_PARALYZE, ISBLESSED);
            break;

        case 6:
            if (get_dir())
                do_zap(&player, WS_DRAIN, ISBLESSED);
            break;

        case 7:
            quaff(&player, P_SMELL, ISBLESSED);
            break;

        default:
            msg("What a strange thing to do!!");
            break;
    }
}

/*
    do_wand()
        handle powers of the Wand of Yendor
*/

void
do_wand(void)
{
    int which, i;

    /* Prompt for action */
    msg("How do you wish to apply the Wand of Yendor (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    if (which < 0 || which >= maxsticks)
    {
        msg("");

        for (i = 0; i < maxsticks; i++)
        {
            sprintf(prbuf, "[%c] %s", i + 'a', ws_magic[i].mi_name);
            add_line(prbuf);
        }

        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which >= maxsticks)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }

            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }
        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    if (rnd(pstats.s_lvl) < 12)
    {
        msg("Your finger slips.");
        which = rnd(maxsticks);

        if (wizard)
        {
            msg("What wand? (%d)", which);

            if (get_string(prbuf, cw) == NORM)
            {
                which = atoi(prbuf);

                if (which < 0 || which >= maxsticks)
                {
                    msg("Invalid selection.");
                    which = rnd(maxsticks);
                    msg("Rolled %d.", which);
                }
            }
        }
    }

    if (get_dir())
        do_zap(&player, which, ISBLESSED);
}

/*
    do_crown()
        handle powers of the Crown of Might
*/

void
do_crown(void)
{
    int which, limit;

    /* Prompt for action */
    msg("How do you wish to apply the Crown of Might (* for list)? ");

    which = (short) ((readchar() & 0177) - 'a');

    if (which == (short) ESCAPE - (short) 'a')
    {
        after = FALSE;
        return;
    }

    limit = 9;

    if (is_carrying(TR_PALANTIR))
        limit += 1;

    if (is_carrying(TR_SCEPTRE))
        limit += 1;

    if (which < 0 || which > limit)
    {
        msg("");
        add_line("[a] add strength");
        add_line("[b] add intelligence");
        add_line("[c] add wisdom");
        add_line("[d] add dexterity");
        add_line("[e] add constitution");
        add_line("[f] normal strength");
        add_line("[g] normal intelligence");
        add_line("[h] normal wisdom");
        add_line("[i] normal dexterity");
        add_line("[j] normal constitution");

        if (limit >= 10)
            add_line("[k] disguise");

        if (limit >= 11)
            add_line("[l] super heroism");

        end_line();

        msg("Which power do you wish to use?");

        which = (short) ((readchar() & 0177) - 'a');

        while (which < 0 || which > limit)
        {
            if (which == (short) ESCAPE - (short) 'a')
            {
                after = FALSE;
                return;
            }
            msg("");
            msg("Please enter one of the listed powers: ");
            which = (short) ((readchar() & 0177) - 'a');
        }

        msg("Your attempt is successful.");
    }
    else
        msg("Your attempt is successful.");

    switch (which)
    {
        case 0:
            if (off(player, POWERSTR))
            {
                turn_on(player, POWERSTR);
                chg_str(10, FALSE, FALSE);
                msg("You feel much stronger now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 1:
            if (off(player, POWERINTEL))
            {
                pstats.s_intel += 10;
                turn_on(player, POWERINTEL);
                msg("You feel much more intelligent now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 2:
            if (off(player, POWERWISDOM))
            {
                pstats.s_wisdom += 10;
                turn_on(player, POWERWISDOM);
                msg("Your feel much wiser know.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 3:
            if (off(player, POWERDEXT))
            {
                turn_on(player, POWERDEXT);
                chg_dext(10, FALSE, FALSE);
                msg("You feel much more dextrous now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 4:
            if (off(player, POWERCONST))
            {
                pstats.s_const += 10;
                turn_on(player, POWERCONST);
                msg("You feel much healthier now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 5:
            if (on(player, POWERSTR))
            {
                turn_off(player, POWERSTR);
                chg_str(-10, FALSE, FALSE);
                msg("Your muscles bulge less now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 6:
            if (on(player, POWERINTEL))
            {
                pstats.s_intel = max(pstats.s_intel - 10,
                             3 + ring_value(R_ADDINTEL));
                turn_off(player, POWERINTEL);
                msg("You feel less intelligent now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 7:
            if (on(player, POWERWISDOM))
            {
                pstats.s_wisdom = max(pstats.s_wisdom - 10,
                              3 + ring_value(R_ADDWISDOM));
                turn_off(player, POWERWISDOM);
                msg("You feel less wise now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 8:
            if (on(player, POWERDEXT))
            {
                turn_off(player, POWERDEXT);
                chg_dext(-10, FALSE, FALSE);
                msg("You feel less dextrous now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 9:
            if (on(player, POWERCONST))
            {
                pstats.s_const -= 10;
                turn_off(player, POWERCONST);
                msg("You feel less healthy now.");
            }
            else
                nothing_message(ISCURSED);
            break;

        case 10: quaff(&player, P_DISGUISE, ISNORMAL);
            break;

        case 11: quaff(&player, P_SHERO, ISNORMAL);
            break;

        default:
            msg("What a strange thing to do!!");
            break;

    }
}

/*
    level_eval()
        have amulet evaluate danger on this level
*/

void
level_eval(void)
{
    int cnt = 0;
    long max_nasty = 0;
    struct linked_list  *item;
    struct thing    *tp;
    char    *colour, *temp;

    for (item = mlist; item != NULL; item = next(item))
    {
        tp = THINGPTR(item);
        cnt++;
        max_nasty = max(max_nasty,(10L-tp->t_stats.s_arm) * tp->t_stats.s_hpt);
    }

    if (cnt < 3)
        colour = "black";
    else if (cnt < 6)
        colour = "red";
    else if (cnt < 9)
        colour = "orange";
    else if (cnt < 12)
        colour = "yellow";
    else if (cnt < 15)
        colour = "green";
    else if (cnt < 18)
        colour = "blue";
    else if (cnt < 25)
        colour = "violet";
    else
        colour = "pink with purple polka dots";

    if (max_nasty < 10)
        temp = "feels cold and lifeless";
    else if (max_nasty < 30)
        temp = "feels cool";
    else if (max_nasty < 200)
        temp = "feels warm and soft";
    else if (max_nasty < 1000)
        temp = "feels warm and slippery";
    else if (max_nasty < 5000)
        temp = "feels hot and dry";
    else if (max_nasty < 10000)
        temp = "feels too hot to hold";
    else if (max_nasty < 20000)
        temp = "burns your hand";
    else
        temp = "jumps up and down shrieking 'DANGER! DANGER'";

    msg("The amulet glows %s and %s.", colour, temp);

    return;
}
