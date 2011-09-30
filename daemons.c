/*
    daemons.c - All the daemon and fuse functions are in here
  
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

#include "rogue.h"

/*
    doctor()
        A healing daemon that restors spell and hit points after rest
*/

void
doctor(daemon_arg *who)
{
    struct thing *tp = who->thingptr;
    long ohp;   /* turn off ISFLEE? */
    struct stats *curp;  /* current stats pointer */
    struct stats *maxp;  /* max stats pointer */
    int         turns_quiet, new_points;

    curp = &(tp->t_stats);
    maxp = &(tp->maxstats);

    if (on(*tp, ISINWALL))
    {
        tp->t_rest_hpt = 0;
        return;
    }

    /* Check for regenerating spell points first */

    doctor_spell_points(tp);

    if (curp->s_hpt == maxp->s_hpt)
    {
        tp->t_rest_hpt = 0;
        return;
    }

    tp->t_rest_hpt++;

    switch (tp->t_ctype)
    {
        case C_MAGICIAN:
        case C_ILLUSION:
            turns_quiet = 24 - curp->s_lvl;
            new_points = curp->s_lvl / 2 - 4;
            break;

        case C_THIEF:
        case C_ASSASIN:
        case C_NINJA:
            turns_quiet = 16 - curp->s_lvl;
            new_points = curp->s_lvl / 2 - 1;
            break;

        case C_CLERIC:
        case C_DRUID:
            turns_quiet = 16 - curp->s_lvl;
            new_points = curp->s_lvl / 2 - 2;
            break;

        case C_FIGHTER:
        case C_RANGER:
        case C_PALADIN:
            turns_quiet = 8 - curp->s_lvl / 2;
            new_points = curp->s_lvl / 2 - 1;
            break;

        case C_MONSTER:
            turns_quiet = 16 - curp->s_lvl;
            new_points = curp->s_lvl / 2 - 6;
            break;

        default:
            debug("What a strange character you are!");
            return;
    }

    ohp = curp->s_hpt;

    if (off(*tp, HASDISEASE))
    {
        if (curp->s_lvl < 8)
        {
            if (tp->t_rest_hpt > turns_quiet)
                curp->s_hpt++;
        }
        else if (tp->t_rest_hpt >= 15)
            curp->s_hpt += rnd(new_points) + 1;
    }

    if (tp == &player)
    {
        if (curp->s_lvl > 10)
            turns_quiet = 2;
        else
            turns_quiet = rnd(turns_quiet / 6) + 1;

        if (is_wearing(R_REGEN))
            curp->s_hpt += ring_value(R_REGEN);
    }

    if (on(*tp, ISREGEN))
        curp->s_hpt += curp->s_lvl / 5 + 1;

    if (ohp != curp->s_hpt)
    {
        if (curp->s_hpt >= maxp->s_hpt)
        {
            curp->s_hpt = maxp->s_hpt;

            if (off(*tp, WASTURNED) && on(*tp, ISFLEE)
                && tp != &player)
            {
                turn_off(*tp, ISFLEE);
                tp->t_oldpos = tp->t_pos;
                /* Start our trek over */
            }
        }
        tp->t_rest_hpt = 0;
    }

    return;
}


/*
    doctor_spell_points()
        A healing daemon that restors spell points
*/

void
doctor_spell_points(struct thing *tp)
{
    int        turns_quiet, new_points;
    struct stats *curp;   /* current stats pointer */
    struct stats *maxp;   /* max stats pointer */
    int         opower; /* current power */

    curp = &(tp->t_stats);
    maxp = &(tp->maxstats);
    opower = curp->s_power;

    /* The right ring will let you regenerate while wearing bad armor */

    if (off(*tp, CANCAST) ||
        ((tp == &player) &&
         (cur_armor && wear_ok(tp, cur_armor, NOMESSAGE) == FALSE) &&
         !(is_wearing(R_WIZARD) || is_wearing(R_PIETY))))
    {
        tp->t_rest_pow = 0;
        return;
    }

    tp->t_rest_pow++;

    switch (tp->t_ctype)
    {
        case C_MAGICIAN:
        case C_ILLUSION:
            turns_quiet = 18 - curp->s_lvl / 2;
            new_points = curp->s_lvl / 2;
            break;
        case C_CLERIC:
        case C_DRUID:
            turns_quiet = 24 - curp->s_lvl;
            new_points = curp->s_lvl / 2 - 2;
            break;
        case C_THIEF:
        case C_ASSASIN:
        case C_NINJA:
            turns_quiet = 32 - curp->s_lvl;
            new_points = curp->s_lvl / 3 - 3;
            break;
        case C_FIGHTER:
        case C_RANGER:
        case C_PALADIN:
            turns_quiet = 32 - curp->s_lvl;
            new_points = curp->s_lvl / 3 - 4;
            break;
        case C_MONSTER:
            turns_quiet = 24 - curp->s_lvl;
            new_points = curp->s_lvl - 6;
            break;
        default:
            return;
    }

    if (curp->s_lvl < 8)
    {
        if (tp->t_rest_pow > turns_quiet)
            curp->s_power++;
    }
    else if (tp->t_rest_pow >= 15)
        curp->s_power += rnd(new_points) + 1;

    if (tp == &player && (is_wearing(R_WIZARD) || is_wearing(R_PIETY)))
        curp->s_power += ring_value(R_WIZARD) + ring_value(R_PIETY);

    curp->s_power = min(max(0, curp->s_power), maxp->s_power);

    if (curp->s_power != opower)
        tp->t_rest_pow = 0;

    return;
}

/*
    rollwand()
        called to roll to see if a wandering monster starts up
*/

daemon
rollwand(daemon_arg *arg)
{
    NOOP(arg);

    if ((rnd(6) == 0) && (player.t_ctype != C_THIEF ||
        (rnd(30) >= pstats.s_dext)))
    {
        wanderer();
        kill_daemon(DAEMON_ROLLWAND);
        light_fuse(FUSE_SWANDER, 0, WANDERTIME, BEFORE);
    }

    return;
}

/*
    stomach()
        digest the hero's food
*/

daemon
stomach(daemon_arg *arg)
{
    int oldfood, old_hunger;
    int amount;
    int power_scale;

    NOOP(arg);

    old_hunger = hungry_state;

    if (food_left <= 0)
    {
        /* the hero is fainting */

        if (no_command || rnd(100) > 20)
            return;

        no_command = rnd(8) + 4;
        running = FALSE;
        count = 0;
        hungry_state = F_FAINT;
        feed_me(hungry_state);
    }
    else
    {
        oldfood = food_left;

        amount = ring_eat(LEFT_1) + ring_eat(LEFT_2) +
            ring_eat(LEFT_3) + ring_eat(LEFT_4) +
            ring_eat(RIGHT_1) + ring_eat(RIGHT_2) +
            ring_eat(RIGHT_3) + ring_eat(RIGHT_4) +
            foodlev;

        if (on(player, SUPEREAT))   /* artifact or regeneration munchies */
            amount *= 2;

        if (on(player, POWEREAT))  /* Used an artifact power */
        {
            amount += 40;
            turn_off(player, POWEREAT);
        }

        power_scale = (on(player, POWERDEXT) + on(player, POWERSTR) +
            on(player, POWERWISDOM) + on(player, POWERINTEL) +
            on(player, POWERCONST) + 1);

        food_left -= amount * power_scale;

        if (food_left < MORETIME && oldfood >= MORETIME)
        {
            hungry_state = F_WEAK;
            running = FALSE;
            feed_me(hungry_state);
        }
        else if (food_left < 2 * MORETIME && oldfood >= 2 * MORETIME)
        {
            hungry_state = F_HUNGRY;
            running = FALSE;
            feed_me(hungry_state);
        }
    }

    if (old_hunger != hungry_state)
        updpack();

    wghtchk(NULL);
}


/*
    runners()
        Make all the running monsters move. with monsters now fighting
        each other, this routine have been enhanced and may need more work yet
*/

daemon
runners(daemon_arg *arg)
{
    struct linked_list  *item;
    struct thing        *tp;

    NOOP(arg);

    for (item = mlist; item != NULL; item = next_mons)
    {
        curr_mons = item;
        next_mons = next(curr_mons);
        tp        = THINGPTR(item);

        if (on(*tp, ISHELD) && rnd(tp->t_stats.s_str +
            tp->t_stats.s_lvl) > 10 + rnd(50))
        {
            turn_off(*tp, ISHELD);
            turn_off(*tp, ISDISGUISE);
            turn_on(*tp, ISRUN);
            tp->t_ischasing = TRUE;
            tp->t_chasee = &player;
            tp->t_horde  = NULL;

            if (tp->t_stats.s_hpt < rnd(tp->maxstats.s_hpt))
                turn_on(*tp, ISFLEE);

            if (cansee(tp->t_pos.y, tp->t_pos.x))
                msg("The %s breaks free!", monsters[tp->t_index].m_name);
        }

        if (off(*tp, ISHELD) && on(*tp, ISRUN))
        {
            int flee = FALSE;

            flee = on(*tp, ISFLEE) ||
                     ( (tp->t_chasee == &player) &&
                         on(player, ISINWALL) &&
                         off(*tp, CANINWALL) && off(*tp, ISFAMILIAR) );

            if (off(*tp, ISSLOW) || tp->t_turn)
            {
                daemon_arg targ;

                targ.thingptr = tp;
                doctor(&targ);
                do_chase(tp, flee);
            }

            if (curr_mons && (on(*tp, ISHASTE) ||
               ((on(*tp, CANFLY) || on(*tp, ISFAST)) &&
               DISTANCE(hero, tp->t_pos) >= 4)))
            {
                daemon_arg targ;

                targ.thingptr = tp;
                doctor(&targ);
                do_chase(tp, flee);
            }

            if (curr_mons)
            {
                tp->t_turn ^= TRUE;
                tp->t_wasshot ^= FALSE; /* Not shot anymore */
            }

        }
    }

    curr_mons = next_mons = NULL;

    return;
}

/*
    swander()
        called when it is time to start rolling for wandering monsters
*/

fuse
swander(fuse_arg *arg)
{
    NOOP(arg);

    start_daemon(DAEMON_ROLLWAND, 0, BEFORE);
    return;
}

/*
    unconfuse
        release the poor player from his confusion
*/

fuse
unconfuse(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISHUH);
    msg("You feel less confused now.");
    return;
}

/*
    unscent()
        turn of extra smelling ability
*/

fuse
unscent(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, CANSCENT);
    msg("The smell of monsters goes away.");
}

/*
    scent
        give back the players sense of smell
*/

fuse
scent(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISUNSMELL);
    msg("You begin to smell the damp dungeon air again.");
}


/*
    unhear
        player doesn't have extra hearing any more
*/

fuse
unhear(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, CANHEAR);
    msg("The sounds of monsters fades away.");
}

/*
    hear()
        return the players sense of hearing
*/

fuse
hear(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISDEAF);
    msg("You can hear again.");
}


/*
    unsee
        He lost his see invisible power

    Need to make monsters invisible again? This
    was done in Rogue 5.2

    for (th = mlist; th != NULL; th = next(th))
        if (on(*th, ISINVIS) && see_monst(th))
        {
            move(th->t_pos.y, th->t_pos.x);
            addch(th->t_oldch);
        }
*/

fuse
unsee(fuse_arg *arg)
{
    NOOP(arg);

    if (!is_wearing(R_SEEINVIS))
    {
        turn_off(player, CANSEE);
        msg("The tingling feeling leaves your eyes.");
    }
}

/*
    unstink()
        Remove to-hit handicap from player
*/

fuse
unstink(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, HASSTINK);
}

/*
    unclrhead
        Player is no longer immune to confusion
*/

fuse
unclrhead(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISCLEAR);
    msg("The blue aura about your head fades away.");
}

/*
    unphase()
        Player can no longer walk through walls
*/

fuse
unphase(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, CANINWALL);

    msg("Your dizzy feeling leaves you.");

    if (!step_ok(hero.y, hero.x, NOMONST, &player))
        death(D_PETRIFY);
}

/*
    sight()
        He gets his sight back
*/

fuse
sight(fuse_arg *arg)
{
    NOOP(arg);

    if (on(player, ISBLIND))
    {
        extinguish_fuse(FUSE_SIGHT);
        turn_off(player, ISBLIND);
        light(&hero);
        msg("The veil of darkness lifts.");
    }
}

/*
    res_strength()
        Restore player's strength
*/

fuse
res_strength(fuse_arg *arg)
{
    NOOP(arg);

    if (lost_str)
    {
        chg_str(lost_str, FALSE, FALSE);
        lost_str = 0;
    }
    else
        pstats.s_str = max_stats.s_str + ring_value(R_ADDSTR) +
            (on(player, POWERSTR) ? 10 : 0) +
            (on(player, SUPERHERO) ? 10 : 0);

    updpack();
}

/*
    nohaste()
        End the hasting
*/

fuse
nohaste(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISHASTE);
    msg("You feel yourself slowing down.");
}

/*
    noslow()
        End the slowing
*/

fuse
noslow(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISSLOW);
    msg("You feel yourself speeding up.");
}

/*
    suffocate()
        If this gets called, the player has suffocated
*/

fuse
suffocate(fuse_arg *arg)
{
    NOOP(arg);

    death(D_SUFFOCATION);
}

/*
    cure_disease()
        daemon for curing the diseased
*/

fuse
cure_disease(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, HASDISEASE);

    if (off(player, HASINFEST))
        msg("You begin to feel yourself improving again.");
}

/*
    un_itch()
        daemon for adding back dexterity
*/

fuse
un_itch(fuse_arg *arg)
{
    NOOP(arg);

    if (lost_dext)
    {
        chg_dext(lost_dext, FALSE, FALSE);
        lost_dext = 0;
        turn_off(player, HASITCH);
    }
}

/*
    appear()
        Become visible again
*/

fuse
appear(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISINVIS);
    PLAYER = VPLAYER;
    msg("The tingling feeling leaves your body.");
    light(&hero);
}

/*
     unelectrify()
       stop shooting off sparks
*/

fuse
unelectrify(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISELECTRIC);
    msg("The sparks and violet glow from your body fade away.");
    light(&hero);
}

/*
    unshero()
        super heroism wears off, now do nasty effects
*/

fuse
unshero(fuse_arg *arg)
{
    NOOP(arg);

    msg("Your feeling of invulnerability goes away.");
    turn_off(player, SUPERHERO);
    chg_str(-11, FALSE, FALSE);
    chg_dext(-6, FALSE, FALSE);
    food_left -= HEROTIME + rnd(HEROTIME);
    no_command += 5 + rnd(5);
    msg("You fall asleep.");
}

/*
    unbhero()
        blessed super heroism wears off, no bad effects
*/

fuse
unbhero(fuse_arg *arg)
{
    NOOP(arg);

    msg("Your feeling of invincibility goes away.");
    turn_off(player, SUPERHERO);
    chg_str(-10, FALSE, FALSE);
    chg_dext(-5, FALSE, FALSE);
}

/*
    undisguise()
        player stops looking like a monster
*/

fuse
undisguise(fuse_arg *arg)
{
    NOOP(arg);

    msg("Your skin feels itchy for a moment.");
    turn_off(player, ISDISGUISE);
    PLAYER = VPLAYER;
    light(&hero);
}

/*
    unsummon()
        Unsummon a monster
*/

void
unsummon(fuse_arg *monny)
{
    struct linked_list *monst = monny->ll;
    struct linked_list  *sum_monst = (struct linked_list *) monst;
    struct thing    *tp = THINGPTR(sum_monst);
    char    *mname = monsters[tp->t_index].m_name;

    turn_off(*tp, WASSUMMONED);
    turn_off(player, HASSUMMONED);
    msg("Goodbye, master.");
    msg("The summoned %s phases out of existence", mname);
    killed(NULL, sum_monst, NOMESSAGE, NOPOINTS);
    mons_summoned--;
}

/*
    ungaze()
        Turn off gaze reflection
*/

fuse
ungaze(fuse_arg *arg)
{
    NOOP(arg);

    msg("The shiny particles swirl to the floor.");
    turn_off(player, CANREFLECT);
}

/*
    shero()
        restore lost abilities from cursed potion of shero
*/

fuse
shero(fuse_arg *arg)
{
    NOOP(arg);

    msg("You feel normal again.");
    chg_str(2, FALSE, TRUE);
    chg_dext(2, FALSE, TRUE);
    turn_off(player, ISUNHERO);
}

/*
    wghtchk()
	    check that the pack weight is OK 
*/

fuse
wghtchk(fuse_arg *arg)
{
    int dropchk, err = TRUE;
    char    ch;

    NOOP(arg);

    inwhgt = TRUE;

    if (pstats.s_pack > pstats.s_carry)
    {
        ch = CCHAR( mvwinch(stdscr, hero.y, hero.x) );

        if ((ch != FLOOR && ch != PASSAGE))
        {
            extinguish_fuse(FUSE_WGHTCHK);
            light_fuse(FUSE_WGHTCHK, (void *)TRUE, 1, AFTER);
            inwhgt = FALSE;
            return;
        }

        extinguish_fuse(FUSE_WGHTCHK);
        msg("Your pack is too heavy for you.");

        do
        {
            dropchk = drop(NULL);

            if (dropchk == FALSE)
            {
                mpos = 0;
                msg("You must drop something.");
            }

            if (dropchk == TRUE)
                err = FALSE;

        }
        while (err);

    }

    inwhgt = FALSE;
}


/*
    uncold()
        He lost his cold resistance power
*/

fuse
uncold(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, NOCOLD);

    if (!is_wearing(R_COLDRESIST))
        msg("You feel a slight chill in the air.");
}

/*
    unhot()
        He lost his fire resistance power
*/

fuse
unhot(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, NOFIRE);

    if (!is_wearing(R_FIRERESIST))
        msg("You feel a flush of warmth.");
}

/*
    unfly()
        He stopped flying
*/

fuse
unfly(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, CANFLY);

    if (!is_wearing(R_LEVITATION))
        msg("You float gently to the ground.");
}

/*
    unbreathe()
        He started needing oxygen
*/

fuse
unbreathe(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, HASOXYGEN);

    if (!is_wearing(R_BREATHE))
        msg("You start huffing and puffing.");
}

/*
    unregen()
        He stops being regenerative
*/

fuse
unregen(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, ISREGEN);

    if (!is_wearing(R_REGEN))
        msg("Your metabolism slows down.");
}

/*
    unsupereat()
        He stops being excessively hungry
*/

fuse
unsupereat(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, SUPEREAT);
    msg("You stop feeling so hungry.");
}

/*
    unshield()
        He stops having his AC helped by magic
*/

fuse
unshield(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, HASSHIELD);
    pstats.s_arm -= pstats.s_acmod;
    pstats.s_acmod = 0;
    msg("Your skin feels normal.");
}

/*
    unmshield()
        He stops ignoring thrown weapons
*/

fuse
unmshield(fuse_arg *arg)
{
    NOOP(arg);

    turn_off(player, HASMSHIELD);
    msg("The fog dissapates.");
}

/*
    untrue()
        He lost his true sight power
*/

void
untruesee(fuse_arg *arg)
{
    NOOP(arg);

    if (!is_wearing(R_TRUESEE))
    {
        turn_off(player, CANTRUESEE);
        msg("Your sensory perceptions return to normal.");
    }
}
