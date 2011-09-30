/*
    potions.c - Functions for dealing with potions
      
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

/*
    quaff - drink a potion (or effect a potion-like spell)

    quaffer: who does it
    which:   which P_POTION (-1 means ask from pack)
    flags:   ISBLESSED, ISCURSED
*/

void
quaff(struct thing *quaffer, int which, int flags)
{
    struct object   *obj;
    struct thing    *th;
    struct stats    *curp = &(quaffer->t_stats);
    struct stats    *maxp = &(quaffer->maxstats);
    int    blessed = flags & ISBLESSED;
    int    cursed = flags & ISCURSED;
    int    is_potion = (which < 0 ? TRUE : FALSE);

    struct linked_list  *item, *titem;
    char    buf[2 * LINELEN];

    if (quaffer != &player)
    {
        monquaff(quaffer, which, flags);
        return;
    }

    if (is_potion)      /* A regular potion */
    {
        if ((item = get_item("quaff", POTION)) == NULL)
            return;

        obj = OBJPTR(item);

        if (obj->o_type != POTION)
        {
            msg("You can't drink that!");
            return;
        }

        /* Calculate its effect */

        flags = obj->o_flags;
        cursed = obj->o_flags & ISCURSED;
        blessed = obj->o_flags & ISBLESSED;
        which = obj->o_which;

        /* remove it from the pack */

        rem_pack(obj);
        discard(item);
        updpack();
    }

    switch(which)
    {
        case P_CLEAR:
            if (cursed)
            {
                if (off(player, ISCLEAR))
                {
                    msg("Wait, what's going on here. Huh? What? Who?");

                    if (on(player, ISHUH))
                        lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
                    else
                        light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);

                    turn_on(player, ISHUH);
                }
                else
                    msg("You feel dizzy for a moment, but it passes.");
            }
            else
            {
                if (blessed) /* Make player immune for the whole game */
                {
                    extinguish_fuse(FUSE_UNCLRHEAD);  /* If we have a fuse, put it out */
                    msg("A strong blue aura surrounds your head.");
                }
                else  /* Just light a fuse for how long player is safe */
                {
                    if (off(player, ISCLEAR))
                    {
                        light_fuse(FUSE_UNCLRHEAD, 0, CLRDURATION, AFTER);
                        msg("A faint blue aura surrounds your head.");
                    }
                    else  /* If have fuse lengthen, else permanently clear */
                    {
                        if (find_slot(FUSE_UNCLRHEAD,FUSE) == NULL)
                            msg("Your blue aura continues to glow strongly.");
                        else
                        {
                            lengthen_fuse(FUSE_UNCLRHEAD, CLRDURATION);
                            msg("Your blue aura brightens for a moment.");
                        }
                    }
                }

                turn_on(player, ISCLEAR);

                /* If player is confused, unconfuse him */

                if (on(player, ISHUH))
                {
                    extinguish_fuse(FUSE_UNCONFUSE);
                    unconfuse(NULL);
                }
            }
            break;

        case P_HEALING:
            if (cursed)
            {
                if (player.t_ctype != C_PALADIN
                    && !(player.t_ctype == C_NINJA
                    && curp->s_lvl > 12)
                    && !save(VS_POISON))
                {
                    feel_message();
                    curp->s_hpt /= 2;
                    curp->s_power /= 2;

                    if ((curp->s_hpt -= 1) <= 0)
                    {
                        death(D_POISON);
                        return;
                    }
                }
                else
                    msg("You feel momentarily sick.");
            }
            else
            {
                int nsides = (blessed ? 8 : 4);
                int hpt_gain = roll(curp->s_lvl, nsides);
                int power_gain = roll(curp->s_lvl, nsides);

                if (blessed && on(player, ISHUH))
                {
                    extinguish_fuse(FUSE_UNCONFUSE);
                    unconfuse(NULL);
                }

                curp->s_hpt = min(curp->s_hpt + hpt_gain, maxp->s_hpt);

                if (is_potion)  /* Do not bump power or maximums if spell */
                {
                    know_items[TYP_POTION][P_HEALING] = TRUE;
                    curp->s_power = min(curp->s_power + power_gain, maxp->s_power);

                    if (maxp->s_hpt == curp->s_hpt)
                        maxp->s_hpt = curp->s_hpt += roll(1, nsides);

                    if (maxp->s_power == curp->s_power)
                        maxp->s_power = curp->s_power += roll(1, nsides);
                }

                msg("You begin to feel %sbetter.", blessed ? "much " : "");

                if (off(player, PERMBLIND))
                    sight(NULL);
            }
            break;

        case P_GAINABIL:
        {
            int   ctype;

            if (!is_potion || pstats.s_arm <= 0)
                feel_message();
            else
            {
                if (blessed)    /* add to all attributes */
                {
                    add_intelligence(FALSE);
                    add_dexterity(FALSE);
                    add_strength(FALSE);
                    add_wisdom(FALSE);
                    add_const(FALSE);
                }
                else
                {
                    if (rnd(100) < 70)
                    /* probably change own ability */
                        ctype = player.t_ctype;
                    else
                        switch(rnd(4))
                        {
                            case 0: ctype = C_FIGHTER;  break;
                            case 1: ctype = C_MAGICIAN; break;
                            case 2: ctype = C_CLERIC;   break;
                            case 3: ctype = C_THIEF;    break;
                        }
                        switch (ctype)
                        {
                            case C_FIGHTER:add_strength(cursed);        break;
                            case C_PALADIN:add_strength(cursed);        break;
                            case C_RANGER:add_strength(cursed);         break;
                            case C_MAGICIAN:add_intelligence(cursed);   break;
                            case C_ILLUSION:add_intelligence(cursed);   break;
                            case C_CLERIC:add_wisdom(cursed);           break;
                            case C_DRUID:add_wisdom(cursed);            break;
                            case C_THIEF:add_dexterity(cursed);         break;
                            case C_ASSASIN:add_dexterity(cursed);       break;
                            case C_NINJA:add_dexterity(cursed);         break;
                            default: msg("You're a strange type!");     break;
                        }
                    }

                    if (rnd(100) < 10)
                        add_const(cursed);

                    if (rnd(100) < 60)
                        curp->s_arm += (cursed ? 1 : -1);

                    if (!cursed)
                        know_items[TYP_POTION][P_GAINABIL] = TRUE;
                }
            }
            break;

        case P_MONSTDET:

            /*
             * Potion of monster detection, if there are monsters,
             * detect them
             */

            if (is_potion)
                know_items[TYP_POTION][P_MONSTDET] = TRUE;

            if (cursed)
            {
                int nm = roll(3, 6);
                int i;
                char    ch;
                struct room *rp;
                coord   pos;

                msg("You begin to sense the presence of monsters.");
                wclear(hw);

                for (i = 1; i < nm; i++)
                {
                    rp = &rooms[rnd_room()];
                    rnd_pos(rp, &pos);

                    if (rnd(2))
                        ch = 'a' + ucrnd(26);
                    else
                        ch = 'A' + ucrnd(26);

                    mvwaddch(hw, pos.y, pos.x, ch);
                }
                waddstr(cw, morestr);
                overlay(hw, cw);
                wrefresh(cw);
                wait_for(' ');
                msg("");
            }
            else if (mlist != NULL)
            {
                msg("You begin to sense the presence of monsters.");
                waddstr(cw, morestr);
                overlay(mw, cw);
                wrefresh(cw);
                wait_for(' ');
                msg("");

                if (blessed)
                    turn_on(player, BLESSMONS);
            }
            else
                nothing_message(flags);
            break;

        case P_TREASDET:

            /* Potion of magic detection.  Show the potions and scrolls */

            if (cursed)
            {
                int nm = roll(3, 3);
                int i;
                char    ch;
                struct room *rp;
                coord   pos;

                msg("You sense the presence of magic on this level.");
                wclear(hw);

                for (i = 1; i < nm; i++)
                {
                    rp = &rooms[rnd_room()];
                    rnd_pos(rp, &pos);

                    if (rnd(9) == 0)
                        ch = BMAGIC;
                    else if (rnd(9) == 0)
                        ch = CMAGIC;
                    else
                        ch = MAGIC;

                    mvwaddch(hw, pos.y, pos.x, ch);
                }
                waddstr(cw, morestr);

                overlay(hw, cw);
                wrefresh(cw);
                wait_for(' ');
                msg("");

                if (is_potion)
                    know_items[TYP_POTION][P_TREASDET] = TRUE;

                break;
            }

            if (blessed)
                turn_on(player, BLESSMAGIC);

            if (lvl_obj != NULL)
            {
                struct linked_list  *mobj;
                struct object   *tp;
                int showit;

                showit = FALSE;
                wclear(hw);

                for (mobj = lvl_obj; mobj != NULL; mobj = next(mobj))
                {
                    tp = OBJPTR(mobj);

                    if (is_magic(tp))
                    {
                        char    mag_type = MAGIC;

                        if (blessed)
                            if (tp->o_flags & ISCURSED)
                                mag_type = CMAGIC;
                            else if (tp->o_flags & ISBLESSED)
                                mag_type = BMAGIC;

                        showit = TRUE;
                        mvwaddch(hw, tp->o_pos.y, tp->o_pos.x, mag_type);
                    }
                }

                for (titem = mlist; titem != NULL; titem = next(titem))
                {
                    struct linked_list  *pitem;

                    th = THINGPTR(titem);

                    for (pitem = th->t_pack; pitem != NULL; pitem = next(pitem))
                    {
                        if (is_magic(OBJPTR(pitem)))
                        {
                            showit = TRUE;
                            mvwaddch(hw, th->t_pos.y, th->t_pos.x,MAGIC);
                        }
                    }
                }

                if (showit)
                {
                    msg("You sense the presence of magic on this level.");

                    if (is_potion)
                        know_items[TYP_POTION][P_TREASDET] = TRUE;

                    waddstr(cw, morestr);
                    overlay(hw, cw);
                    wrefresh(cw);
                    wait_for(' ');
                    msg("");
                    break;
                }
            }
            nothing_message(flags);
            break;

        case P_SEEINVIS:
            if (cursed)
            {
                if (off(player, ISBLIND) && !is_wearing(R_SEEINVIS))
                {
                    msg("A cloak of darkness falls around you.");
                    turn_on(player, ISBLIND);
                    light_fuse(FUSE_SIGHT, 0, SEEDURATION, AFTER);
                    look(FALSE);
                }
                else
                    msg("Your eyes stop tingling for a moment.");
            }
            else if (off(player, PERMBLIND))
            {
                if (is_potion)
                    know_items[TYP_POTION][P_SEEINVIS] = TRUE;

                if (off(player, CANSEE))
                {
                    turn_on(player, CANSEE);
                    msg("Your eyes begin to tingle.");
                    light_fuse(FUSE_UNSEE, 0, blessed ? SEEDURATION * 3 : SEEDURATION, AFTER);
                    light(&hero);
                }
                else if (find_slot(FUSE_UNSEE,FUSE) != NULL)
                {
                    nothing_message(ISNORMAL);
                    lengthen_fuse(FUSE_UNSEE, blessed ? SEEDURATION * 3 : SEEDURATION);
                }
                sight(NULL);
            }
            break;

        case P_PHASE:

            if (cursed)
            {
                msg("You can't move.");
                no_command = HOLDTIME;
            }
            else
            {
                short   duration = (blessed ? 3 : 1);

                if (is_potion)
                    know_items[TYP_POTION][P_PHASE] = TRUE;

                if (on(player, CANINWALL))
                    lengthen_fuse(FUSE_UNPHASE, duration * PHASEDURATION);
                else
                {
                    light_fuse(FUSE_UNPHASE, 0, duration * PHASEDURATION, AFTER);
                    turn_on(player, CANINWALL);
                }
                msg("You feel %slight-headed!", blessed ? "very " : "");
            }
            break;

        case P_RAISELEVEL:

            if (cursed || (!is_potion && pstats.s_lvl > 20))
                lower_level(D_POTION);
            else
            {
                msg("You suddenly feel %smore skillful.", blessed ? "much " : "");
                know_items[TYP_POTION][P_RAISELEVEL] = TRUE;
                raise_level();

                if (blessed)
                    raise_level();
            }
            break;

        case P_HASTE:
            if (cursed)     /* Slow player down */
            {
                if (on(player, ISHASTE))
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
                    else if (!is_wearing(R_FREEDOM))
                    {
                        turn_on(player, ISSLOW);
                        player.t_turn = TRUE;
                        light_fuse(FUSE_NOSLOW, 0, rnd(4) + 4, AFTER);
                    }
                }
            }
            else
            {
                if (off(player, ISSLOW))
                    msg("You feel yourself moving %sfaster.",
                        blessed ? "much " : "");

                add_haste(blessed);

                if (is_potion)
                    know_items[TYP_POTION][P_HASTE] = TRUE;
            }
            break;

        case P_RESTORE:
        {
            int i;

            msg("You are surrounded by an orange mist.");

            if (is_potion)
                know_items[TYP_POTION][P_RESTORE] = TRUE;

            if (lost_str)
            {
                for (i = 0; i < lost_str; i++)
                    extinguish_fuse(FUSE_RES_STRENGTH);

                lost_str = 0;
            }
            res_strength(NULL);

            if (lost_dext)
            {
                for (i = 0; i < lost_dext; i++)
                    extinguish_fuse(FUSE_UNITCH);

                lost_dext = 0;
            }

            res_dexterity();
            res_wisdom();
            res_intelligence();
            curp->s_const = maxp->s_const;
        }
        break;

        case P_INVIS:

            if (cursed)
            {
                msg("You feel very noticable.");
                quaff(&player, P_SHIELD, ISCURSED);
            }
            else if (off(player, ISINVIS))
            {
                turn_on(player, ISINVIS);

                if (on(player, ISDISGUISE))
                {
                    turn_off(player, ISDISGUISE);
                    extinguish_fuse(FUSE_UNDISGUISE);
                    msg("Your skin feels itchy for a moment.");
                }
                msg("You have a tingling feeling all over your body.");
                light_fuse(FUSE_APPEAR, 0, blessed ? WANDERTIME * 3 : WANDERTIME, AFTER);
                PLAYER = IPLAYER;
                light(&hero);

                if (is_potion)
                    know_items[TYP_POTION][P_INVIS] = TRUE;
            }
            else
                lengthen_fuse(FUSE_APPEAR, blessed ? WANDERTIME * 3 : WANDERTIME);

            break;

        case P_SMELL:

            if (cursed)
            {
                if (on(player, CANSCENT))
                {
                    turn_off(player, CANSCENT);
                    extinguish_fuse(FUSE_UNSCENT);
                    msg("You no longer smell monsters around you.");
                }
                else if (on(player, ISUNSMELL))
                {
                    lengthen_fuse(FUSE_UNSCENT, PHASEDURATION);
                    msg("You feel your nose tingle.");
                }
                else
                {
                    turn_on(player, ISUNSMELL);
                    light_fuse(FUSE_SCENT, 0, PHASEDURATION, AFTER);
                    msg("You can't smell anything now.");
                }
            }
            else
            {
                short   duration = (blessed ? 3 : 1);

                if (is_potion)
                    know_items[TYP_POTION][P_SMELL] = TRUE;

                if (on(player, CANSCENT))
                    lengthen_fuse(FUSE_UNSCENT, duration * PHASEDURATION);
                else
                {
                    light_fuse(FUSE_UNSCENT, 0, duration * PHASEDURATION, AFTER);
                    turn_on(player, CANSCENT);
                }
                msg("You begin to smell monsters all around you.");
            }
            break;

        case P_HEAR:

            if (cursed)
            {
                if (on(player, CANHEAR))
                {
                    turn_off(player, CANHEAR);
                    extinguish_fuse(FUSE_HEAR);
                    msg("You no longer hear monsters around you.");
                }
                else if (on(player, ISDEAF))
                {
                    lengthen_fuse(FUSE_HEAR, PHASEDURATION);
                    msg("You feel your ears burn.");
                }
                else
                {
                    light_fuse(FUSE_HEAR, 0, PHASEDURATION, AFTER);
                    turn_on(player, ISDEAF);
                    msg("You are surrounded by a sudden silence.");
                }
            }
            else
            {
                short   duration = (blessed ? 3 : 1);

                if (is_potion)
                    know_items[TYP_POTION][P_HEAR] = TRUE;

                if (on(player, CANHEAR))
                    lengthen_fuse(FUSE_UNHEAR, duration * PHASEDURATION);
                else
                {
                    light_fuse(FUSE_UNHEAR, 0, duration * PHASEDURATION, AFTER);
                    turn_on(player, CANHEAR);
                }
                msg("You begin to hear monsters all around you.");
            }
            break;

        case P_SHERO:
            if (cursed)
            {
                if (on(player, SUPERHERO))
                {
                    msg("You feel ordinary again.");
                    turn_off(player, SUPERHERO);
                    extinguish_fuse(FUSE_UNSHERO);
                    extinguish_fuse(FUSE_UNBHERO);
                }
                else if (on(player, ISUNHERO))
                {
                    msg("Your feeling of vulnerability increases.");
                    lengthen_fuse(FUSE_SHERO, 5 + rnd(5));
                }
                else
                {
                    msg("You feel suddenly vulnerable.");

                    if (curp->s_hpt == 1)
                    {
                        death(D_POTION);
                        return;
                    }

                    curp->s_hpt /= 2;
                    chg_str(-2, FALSE, TRUE);
                    chg_dext(-2, FALSE, TRUE);
                    no_command = 3 + rnd(HEROTIME);
                    turn_on(player, ISUNHERO);
                    light_fuse(FUSE_SHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
                }
            }
            else
            {
                if (on(player, ISFLEE))
                {
                    turn_off(player, ISFLEE);
                    msg("You regain your composure.");
                }

                if (on(player, ISUNHERO))
                {
                    extinguish_fuse(FUSE_SHERO);
                    shero(NULL);
                }
                else if (on(player, SUPERHERO))
                {
                    if (find_slot(FUSE_UNBHERO,FUSE))
                        lengthen_fuse(FUSE_UNBHERO, HEROTIME+2*rnd(HEROTIME));
                    else if (find_slot(FUSE_UNSHERO,FUSE) && !blessed)
                        lengthen_fuse(FUSE_UNSHERO,HEROTIME+2*rnd(HEROTIME));
                    else
                    {
                        extinguish_fuse(FUSE_UNSHERO);
                        unshero(NULL);
                        light_fuse(FUSE_UNBHERO,0, 2 * (HEROTIME + rnd(HEROTIME)), AFTER);
                    }
                    msg("Your feeling of invulnerablity grows stronger.");
                }
                else
                {
                    turn_on(player, SUPERHERO);
                    chg_str(10, FALSE, FALSE);
                    chg_dext(5, FALSE, FALSE);
                    quaff(quaffer, P_HASTE, ISBLESSED);
                    quaff(quaffer, P_CLEAR, ISNORMAL);

                    if (blessed)
                    {
                        light_fuse(FUSE_UNBHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
                        msg("You suddenly feel invincible.");
                    }
                    else
                    {
                        light_fuse(FUSE_UNSHERO, 0, HEROTIME + rnd(HEROTIME), AFTER);
                        msg("You suddenly feel invulnerable.");
                    }

                    if (is_potion)
                        know_items[TYP_POTION][P_SHERO] = TRUE;
                }
            }
            break;

        case P_DISGUISE:
            if (off(player, ISDISGUISE) && off(player, ISINVIS))
            {
                turn_on(player, ISDISGUISE);
                msg("Your body shimmers a moment and then changes.");
                light_fuse(FUSE_UNDISGUISE, 0, blessed ? GONETIME * 3 : GONETIME, AFTER);

                if (rnd(2))
                    PLAYER = 'a' + ucrnd(26);
                else
                    PLAYER = 'A' + ucrnd(26);

                light(&hero);

                if (is_potion)
                    know_items[TYP_POTION][P_DISGUISE] = TRUE;
            }
            else if (off(player, ISINVIS))
                lengthen_fuse(FUSE_UNDISGUISE,blessed?GONETIME * 3 : GONETIME);
            else
                msg("You have an itchy feeling under your skin.");

            break;

        case P_FIRERESIST:
            if (cursed)
            {
                if (!is_wearing(R_FIRERESIST))
                {
                    msg("Your teeth start clattering.");

                    if (on(player, ISHASTE))
                    {
                        extinguish_fuse(FUSE_NOHASTE);
                        nohaste(NULL);
                    }
                    else
                    {
                        msg("You feel yourself moving %sslower.",
                            on(player, ISSLOW) ? "even "  : "");

                        if (on(player, ISSLOW))
                            lengthen_fuse(FUSE_NOSLOW, rnd(4) + 4);
                        else if (!is_wearing(R_FREEDOM))
                        {
                            turn_on(player, ISSLOW);
                            player.t_turn = TRUE;
                            light_fuse(FUSE_NOSLOW, 0, rnd(4) + 4, AFTER);
                        }
                    }
                }
                else
                    msg("You feel a brief chill.");
            }
            else
            {
                if (is_potion)
                    know_items[TYP_POTION][P_FIRERESIST] = TRUE;

                if (blessed)
                {
                    extinguish_fuse(FUSE_UNHOT);
                    msg("You feel a strong continuous warm glow.");
                }
                else
                {
                    if (off(player, NOFIRE))
                    {
                        light_fuse(FUSE_UNHOT, 0, PHASEDURATION, AFTER);
                        msg("You feel a warm glow.");
                    }
                    else
                    {
                        if (find_slot(FUSE_UNHOT,FUSE) == NULL)
                            msg("Your warm glow continues.");
                        else
                        {
                            lengthen_fuse(FUSE_UNHOT, PHASEDURATION);
                            msg("Your feel a hot flush.");
                        }
                    }
                }

                turn_on(player, NOFIRE);

                if (on(player, NOCOLD))
                {
                    turn_off(player, NOCOLD);
                    extinguish_fuse(FUSE_UNCOLD);
                }
            }
            break;

        case P_COLDRESIST:
            if (cursed)
            {
                if (!is_wearing(R_COLDRESIST))
                {
                    msg("Your feel feverishly hot.");

                    if (on(player, ISHASTE))
                    {
                        extinguish_fuse(FUSE_NOHASTE);
                        nohaste(NULL);
                    }
                    else
                    {
                        msg("You feel yourself moving %sslower.",
                            on(player, ISSLOW)  ? "even " : "");

                        if (on(player, ISSLOW))
                            lengthen_fuse(FUSE_NOSLOW, rnd(4) + 4);
                        else if (!is_wearing(R_FREEDOM))
                        {
                            turn_on(player, ISSLOW);
                            player.t_turn = TRUE;
                            light_fuse(FUSE_NOSLOW, 0, rnd(4) + 4, AFTER);
                        }
                    }
                }
                else
                    msg("You feel a brief touch of heat rash.");
            }
            else
            {
                if (is_potion)
                    know_items[TYP_POTION][P_COLDRESIST] = TRUE;

                if (blessed)
                {
                    extinguish_fuse(FUSE_UNCOLD);
                    msg("You feel a strong continuous cool breeze.");
                }
                else
                {
                    if (off(player, NOCOLD))
                    {
                        light_fuse(FUSE_UNCOLD, 0, PHASEDURATION, AFTER);
                        msg("You feel a cool breeze.");
                    }
                    else
                    {
                        if (find_slot(FUSE_UNCOLD,FUSE) == NULL)
                            msg("Your cool feeling continues.");
                        else
                        {
                            lengthen_fuse(FUSE_UNCOLD, PHASEDURATION);
                            msg("The cool breeze blows more strongly.");
                        }
                    }
                }

                turn_on(player, NOCOLD);

                if (on(player, NOFIRE))
                {
                    extinguish_fuse(FUSE_UNHOT);
                    turn_off(player, NOFIRE);
                }
            }
            break;

        case P_HASOXYGEN:
            if (cursed)
            {
                if (!is_wearing(R_BREATHE))
                {
                    msg("You can't breathe.");
                    no_command = HOLDTIME;
                }
                else
                {
                    msg("You feel a momentary shortness of breath.");
                }
            }
            else
            {
                short   duration = (blessed ? 3 : 1);

                if (is_potion)
                    know_items[TYP_POTION][P_HASOXYGEN] = TRUE;

                if (on(player, HASOXYGEN))
                    lengthen_fuse(FUSE_UNBREATHE, duration * PHASEDURATION);
                else
                {
                    light_fuse(FUSE_UNBREATHE, 0, duration * PHASEDURATION, AFTER);
                    turn_on(player, HASOXYGEN);
                }

                if (!is_wearing(R_BREATHE))
                    msg("The air seems %sless polluted.",
                        blessed ? "much " : "");
            }
            break;

        case P_LEVITATION:
            if (cursed)
            {
                msg("You can't move.");
                no_command = HOLDTIME;
            }
            else
            {
                short   duration = (blessed ? 3 : 1);

                if (is_potion)
                    know_items[TYP_POTION][P_LEVITATION] = TRUE;

                if (on(player, CANFLY))
                    lengthen_fuse(FUSE_UNFLY, duration * WANDERTIME);
                else
                {
                    light_fuse(FUSE_UNFLY, 0, duration * WANDERTIME, AFTER);
                    turn_on(player, CANFLY);
                }

                if (!is_wearing(R_LEVITATION))
                    msg("You %sbegin to float in the air!",
                        blessed ? "quickly " : "");
            }
            break;

        case P_REGENERATE:
            if (cursed)
            {
                quaff(quaffer, P_HEALING, ISCURSED);
                quaff(quaffer, P_HASTE, ISCURSED);
            }
            else
            {
                short   duration = (blessed ? 3 : 1) * HUHDURATION;

                if (is_potion)
                    know_items[TYP_POTION][P_REGENERATE] = TRUE;

                if (on(player, SUPEREAT))
                    lengthen_fuse(FUSE_UNSUPEREAT, duration);
                else
                {
                    light_fuse(FUSE_UNSUPEREAT, 0, duration, AFTER);
                    turn_on(player, SUPEREAT);
                }

                if (on(player, ISREGEN))
                    lengthen_fuse(FUSE_UNREGEN, duration);
                else
                {
                    light_fuse(FUSE_UNREGEN, 0, duration, AFTER);
                    turn_on(player, ISREGEN);
                }

                if (!is_wearing(R_REGEN))
                    msg("You feel %shealthier!", blessed ? "much " : "");
            }

        case P_SHIELD:
        {
            int adjustment = 0;

            if (on(player, HASSHIELD))      /* cancel old spell */
            {
                extinguish_fuse(FUSE_UNSHIELD);
                unshield(NULL);
            }

            if (cursed)
                adjustment = 2;
            else if (blessed)
            {
                msg("Your skin feels very rigid.");

                switch (player.t_ctype)
                {
                    case C_FIGHTER:
                    case C_PALADIN:
                    case C_RANGER:
                        adjustment = -3;
                        break;
                    default:
                        adjustment = -5;
                }
            }
            else
            {
                msg("Your skin hardens.");
                adjustment = -2;
            }

            pstats.s_arm += adjustment;
            pstats.s_acmod += adjustment;
            turn_on(player, HASSHIELD);
            light_fuse(FUSE_UNSHIELD,0,(blessed ? 3 : 1) * SEEDURATION, AFTER);

            if (is_potion)
                know_items[TYP_POTION][P_SHIELD] = TRUE;
        }
        break;

        case P_TRUESEE:
            if (cursed)
            {
                turn_on(player, PERMBLIND);

                if (on(player, ISBLIND))
                {
                    msg("The gloom around you thickens.");
                    lengthen_fuse(FUSE_SIGHT, SEEDURATION);
                }
                else
                {
                    msg("A mantle of darkness falls around you.");
                    turn_on(player, ISBLIND);
                    light_fuse(FUSE_SIGHT, 0, SEEDURATION, AFTER);
                    look(FALSE);
                }
                look(FALSE);
            }
            else if (on(player, PERMBLIND))
            {
                if (blessed || is_potion)
                {
                    turn_off(player, PERMBLIND);
                    sight(NULL);
                    goto let_there_be_light;
                }
                else
                    nothing_message(ISBLESSED);
            }
            else
    let_there_be_light:
                if (off(player, CANSEE))
                {
                    turn_on(player, CANSEE);
                    msg("You feel especially perceptive.");
                    light_fuse(FUSE_UNTRUESEE, 0, blessed ? SEEDURATION * 3
                        : SEEDURATION, AFTER);
                    light(&hero);
                }
                else if (find_slot(FUSE_UNSEE,FUSE) != NULL)
                {
                    nothing_message(ISNORMAL);
                    lengthen_fuse(FUSE_UNTRUESEE, blessed ? SEEDURATION * 3
                        : SEEDURATION);
                }

            break;

        default:
            msg("What an odd tasting potion!");
            return;
    }

    status(FALSE);

    if (is_potion)
    {
        if (!cursed && know_items[TYP_POTION][which] &&
            guess_items[TYP_POTION][which])
        {
            ur_free(guess_items[TYP_POTION][which]);
            guess_items[TYP_POTION][which] = NULL;
        }
        else if (askme && !know_items[TYP_POTION][which] &&
            guess_items[TYP_POTION][which] == NULL)
        {
            msg(terse ? "Call it: " :  "What do you want to call it? ");

            if (get_string(buf, cw) == NORM)
            {
                guess_items[TYP_POTION][which] =
                    new_alloc(strlen(buf) + 1);
                strcpy(guess_items[TYP_POTION][which], buf);
            }
        }
        food_left += (blessed ? rnd(100) : (cursed ? -rnd(100) : rnd(50)));
    }
}

/*
    lower_level()
        Lower a level of experience
*/

void
lower_level(int who)
{
    int fewer, nsides = 0, i;

    if (--pstats.s_lvl == 0)
    {
        death(who); /* All levels gone */
        return;
    }

    msg("You suddenly feel less skillful.");
    pstats.s_exp = 1L;
    init_exp();

    for (i = 2; i <= pstats.s_lvl; i++)
    {
        if (max_stats.s_exp < 0x3fffffffL)  /* 2^30 - 1 */
            max_stats.s_exp *= 2L;  /* twice as many for next */
    }

    switch (player.t_ctype)
    {
        case C_FIGHTER: nsides = 12;break;
        case C_PALADIN: nsides = 12;break;
        case C_RANGER: nsides = 12; break;
        case C_MAGICIAN: nsides = 4;break;
        case C_ILLUSION: nsides = 4;break;
        case C_CLERIC: nsides = 8;  break;
        case C_DRUID: nsides = 8;   break;
        case C_THIEF: nsides = 6;   break;
        case C_ASSASIN: nsides = 6; break;
        case C_NINJA: nsides = 6;   break;
    }

    fewer = max(1, roll(1, 16 - nsides) + int_wis_bonus());
    pstats.s_power -= fewer;
    max_stats.s_power -= fewer;

    fewer = max(1, roll(1, nsides) + const_bonus());
    pstats.s_hpt -= fewer;
    max_stats.s_hpt -= fewer;

    if (pstats.s_hpt < 1)
        pstats.s_hpt = 1;

    if (max_stats.s_hpt < 1)
    {
        death(who);
        return;
    }
}

void
res_dexterity(void)
{
    if (lost_dext)
    {
        chg_dext(lost_dext, FALSE, FALSE);
        lost_dext = 0;
    }
    else
    {
        pstats.s_dext = max_stats.s_dext + ring_value(R_ADDHIT) +
            (on(player, POWERDEXT) ? 10 : 0) +
            (on(player, SUPERHERO) ? 5 : 0);
    }

}


/*
    res_wisdom()
        Restore player's wisdom
*/

void
res_wisdom(void)
{
    int ring_str;

    /* Discount the ring value */

    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    if (pstats.s_wisdom < max_stats.s_wisdom)
        pstats.s_wisdom = max_stats.s_wisdom;

    /* Redo the rings */

    pstats.s_wisdom += ring_str;
}

/*
    res_intelligence()
        Restore player's intelligence
*/

void
res_intelligence(void)
{
    int ring_str;

    /* Discount the ring value */

    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    if (pstats.s_intel < max_stats.s_intel)
        pstats.s_intel = max_stats.s_intel;

    /* Redo the rings */

    pstats.s_intel += ring_str;
}


/*
    add_strength()
        Increase player's strength
*/

void
add_strength(int cursed)
{

    if (cursed)
    {
        msg("You feel slightly weaker now.");
        chg_str(-1, FALSE, FALSE);
    }
    else
    {
        msg("You feel stronger now.  What bulging muscles!");

        if (lost_str != 0)
        {
            lost_str--;
            chg_str(1, FALSE, FALSE);
        }
        else
            chg_str(1, TRUE, FALSE);
    }
}


/*
    add_intelligence()
        Increase player's intelligence
*/

void
add_intelligence(int cursed)
{
    int ring_str;   /* Value of ring strengths */

    /* Undo any ring changes */

    ring_str = ring_value(R_ADDINTEL) + (on(player, POWERINTEL) ? 10 : 0);
    pstats.s_intel -= ring_str;

    /* Now do the potion */

    if (cursed)
    {
        msg("You feel slightly less intelligent now.");
        pstats.s_intel = max(pstats.s_intel - 1, 3);
    }
    else
    {
        msg("You feel more intelligent now.  What a mind!");
        pstats.s_intel = min(pstats.s_intel + 1, 25);
    }

    /* Adjust the maximum */

    if (max_stats.s_intel < pstats.s_intel)
        max_stats.s_intel = pstats.s_intel;

    /* Now put back the ring changes */
    pstats.s_intel += ring_str;
}


/*
    add_wisdom()
        Increase player's wisdom
*/

void
add_wisdom(int cursed)
{
    int ring_str;   /* Value of ring strengths */

    /* Undo any ring changes */

    ring_str = ring_value(R_ADDWISDOM) + (on(player, POWERWISDOM) ? 10 : 0);
    pstats.s_wisdom -= ring_str;

    /* Now do the potion */

    if (cursed)
    {
        msg("You feel slightly less wise now.");
        pstats.s_wisdom = max(pstats.s_wisdom - 1, 3);
    }
    else
    {
        msg("You feel wiser now.  What a sage!");
        pstats.s_wisdom = min(pstats.s_wisdom + 1, 25);
    }

    /* Adjust the maximum */

    if (max_stats.s_wisdom < pstats.s_wisdom)
        max_stats.s_wisdom = pstats.s_wisdom;

    /* Now put back the ring changes */
    pstats.s_wisdom += ring_str;
}


/*
    add_dexterity()
        Increase player's dexterity
*/

void
add_dexterity(int cursed)
{
    /* Now do the potion */

    if (cursed)
    {
        msg("You feel less dextrous now.");
        chg_dext(-1, FALSE, TRUE);
    }
    else
    {
        msg("You feel more dextrous now.  Watch those hands!");

        if (lost_dext != 0)
        {
            lost_dext--;
            chg_dext(1, FALSE, FALSE);
        }
        else
            chg_dext(1, TRUE, FALSE);
    }
}


/*
    Increase player's constitution
*/

void
add_const(int cursed)
{
    /* Do the potion */

    if (cursed)
    {
        msg("You feel slightly less healthy now.");
        pstats.s_const = max(pstats.s_const - 1, 3) +
            (on(player, POWERCONST) ? 10 : 0);
    }
    else
    {
        msg("You feel healthier now.");
        pstats.s_const = min(pstats.s_const + 1, 25) +
            (on(player, POWERCONST) ? 10 : 0);
    }

    /* Adjust the maximum */

    if (max_stats.s_const < pstats.s_const - (on(player, POWERCONST) ? 10 : 0))
        max_stats.s_const = pstats.s_const;
}

/*
    monquaff()
        monster gets the effect
*/

void
monquaff(struct thing *quaffer, int which, int flags)
{
    struct stats *curp = &(quaffer->t_stats);
    struct stats *maxp = &(quaffer->maxstats);
    int blessed = flags & ISBLESSED;
    int cursed = flags & ISCURSED;

    switch(which)
    {
        case P_SEEINVIS:
            if (cursed)
                turn_on(*quaffer, ISHUH);
            else
                turn_on(*quaffer, CANSEE);
            break;

        case P_GAINABIL:
            if (cursed)
                curp->s_intel /= 2;
            else
                curp->s_power = maxp->s_power;
            break;

        case P_CLEAR:
            if (cursed)
                turn_on(*quaffer, ISHUH);
            else
                turn_on(*quaffer, ISHUH);
            break;

        case P_HEALING:
            if (cursed)
            {
                curp->s_hpt /= 2;
                curp->s_power /= 2;
            }
            else
            {
                int nsides = (blessed ? 8 : 4);
                int hpt_gain = roll(curp->s_lvl, nsides);
                int power_gain = roll(curp->s_lvl, nsides);

                curp->s_hpt = min(curp->s_hpt + hpt_gain, maxp->s_hpt);
                curp->s_power = min(curp->s_power + power_gain, maxp->s_power);
            }
            break;

        case  P_HASTE:
            if (cursed)
            {
                if (on(*quaffer, ISHASTE))
                    turn_off(*quaffer, ISHASTE);
                else
                    turn_on(*quaffer, ISSLOW);
            }
            else
                turn_on(*quaffer, ISHASTE);
            break;

        case P_INVIS:
            turn_on(*quaffer, ISINVIS);

            if (cansee(quaffer->t_pos.y, quaffer->t_pos.x))
                seemsg("The monster dissappears!");

            break;

        case P_REGENERATE:
            if (cursed)
            {
                quaff(quaffer, P_HEALING, ISCURSED);
                quaff(quaffer, P_HASTE, ISCURSED);
            }
            else
                turn_on(*quaffer, ISREGEN);
            break;

        case P_SHERO:
            if (on(*quaffer, ISFLEE))
                turn_off(*quaffer, ISFLEE);
            else
            {
                turn_on(*quaffer, SUPERHERO);
                quaff(quaffer, P_HASTE, ISBLESSED);
                quaff(quaffer, P_CLEAR, ISNORMAL);
            }
            break;

        case P_PHASE:
            if (cursed)
                quaffer->t_no_move += HOLDTIME;
            else
                turn_on(*quaffer, CANINWALL);
            break;

        default:
            debug("'%s' is a strange potion for a monster to quaff!",
                p_magic[which].mi_name);
    }
}
