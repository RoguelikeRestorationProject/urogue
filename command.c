/*
    command.c  -  Read and execute the user commands
 
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
#include <ctype.h>
#include "rogue.h"

/*
    command()
        Process the user commands
*/

void
command(void)
{
    static char repcommand;     /* Command to repeat if we are repeating */
    static int fight_to_death; /* Flags if we are fighting to death     */
    static coord dir;          /* Last direction specified              */

    object  *obj;
    char     ch;
    int      ntimes = 1; /* Number of player moves */
    coord    nullcoord;

    nullcoord.x = nullcoord.y = 0;

    if (on(player, CANFLY) && rnd(2))
        ntimes++;

    if (on(player, ISHASTE))
        ntimes++;

    if (fighting && att_bonus())
        ntimes *= 2;

    if (on(player, ISSLOW))
    {
        if (player.t_turn != TRUE)
            ntimes--;

        player.t_turn ^= TRUE;
    }

    if (ntimes == 0)
        return;

    while (ntimes--)
    {
        moving = FALSE;

        /* If player is infested, take off a hit point */

        if (on(player, HASINFEST) && !is_wearing(R_HEALTH))
        {
            if ((pstats.s_hpt -= infest_dam) <= 0)
            {
                death(D_INFESTATION);
                return;
            }
        }

        look(after);

        if (!running)
            door_stop = FALSE;

        status(FALSE);
        wmove(cw, hero.y, hero.x);

        if (!((running || count) && jump))
            wrefresh(cw);   /* Draw screen */

        take = 0;
        after = TRUE;

        /*
         * Read command or continue run
         */

        if (!no_command)
        {
            if (fighting)
            {
                ch = (fight_to_death) ? 'F' : 'f';
            }
            else if (running)
            {
                /*
                 * If in a corridor, if we are at a turn with
                 * only one way to go, turn that way.
                 */

                if ((winat(hero.y, hero.x) == PASSAGE) && off(player, ISHUH) &&
                    (off(player, ISBLIND)))
                    switch (runch)
                    {
                        case 'h': corr_move(0, -1); break;
                        case 'j': corr_move(1, 0); break;
                        case 'k': corr_move(-1, 0); break;
                        case 'l': corr_move(0, 1); break;
                    }

                ch = runch;
            }
            else if (count)
                ch = repcommand;
            else
            {
                ch = readchar();

                if (mpos != 0 && !running)
                    msg("");    /* Erase message if its there */
            }
        }
        else
        {
            ch = '.';
            fighting = moving = FALSE;
        }

        if (no_command)
        {
            if (--no_command == 0)
                msg("You can move again.");
        }
        else
        {

            /*
             * check for prefixes
             */

            if (isdigit(ch))
            {
                count = 0;
                while (isdigit(ch))
                {
                    count = count * 10 + (ch - '0');
                    ch = readcharw(cw);
                }
                repcommand = ch;

                /*
                 * Preserve count for commands which can be
                 * repeated.
                 */

                switch(ch)
                {
                    case 'h':
                    case 'j':
                    case 'k':
                    case 'l':
                    case 'y':
                    case 'u':
                    case 'b':
                    case 'n':
                    case 'H':
                    case 'J':
                    case 'K':
                    case 'L':
                    case 'Y':
                    case 'U':
                    case 'B':
                    case 'N':
                    case 'q':
                    case 'r':
                    case 's':
                    case 'm':
                    case 't':
                    case 'C':
                    case 'I':
                    case '.':
                    case 'z':
                    case 'p':
                        break;
                    default:
                        count = 0;
                }
            }

            /* Save current direction */

            if (!running)   /* If running, it is already saved */
                switch (ch)
                {
                    case 'h':
                    case 'j':
                    case 'k':
                    case 'l':
                    case 'y':
                    case 'u':
                    case 'b':
                    case 'n':
                        runch = ch;
                        break;
                    case 'H':
                    case 'J':
                    case 'K':
                    case 'L':
                    case 'Y':
                    case 'U':
                    case 'B':
                    case 'N':
                        runch = (char) tolower(ch);
                        break;
                }

            /*
             * execute a command
             */

            if (count && !running)
                count--;

            switch(ch)
            {
                /*
                 * Movement and combat commands
                 */

                case 'h': do_move(0,-1); break;
                case 'j': do_move(1, 0);  break;
                case 'k': do_move(-1, 0); break;
                case 'l': do_move(0, 1); break;
                case 'y': do_move(-1, -1); break;
                case 'u': do_move(-1, 1); break;
                case 'b': do_move(1, -1); break;
                case 'n': do_move(1, 1); break;
                case 'H': do_run('h'); break;
                case 'J': do_run('j'); break;
                case 'K': do_run('k'); break;
                case 'L': do_run('l'); break;
                case 'Y': do_run('y'); break;
                case 'U': do_run('u'); break;
                case 'B': do_run('b'); break;
                case 'N': do_run('n'); break;
                case 'm':
                    moving = TRUE;
                    if (!get_dir())
                    {
                        after = FALSE;
                        break;
                    }
                    do_move(delta.y, delta.x);
                    break;
                case 'F':
                case 'f':
                    fight_to_death = (ch == 'F');
                    if (!fighting)
                    {
                        if (get_dir())
                        {
                            dir = delta;
                            beast = NULL;
                        }
                        else
                        {
                            after = FALSE;
                            break;
                        }
                    }
                    do_fight(dir, (ch == 'F') ? TRUE : FALSE);
                    break;
                case 't':
                    if (get_dir())
                        missile(delta.y, delta.x, get_item("throw", 0),
                            &player);
                    else
                        after = FALSE;

                    /*
                     * Informational commands - Do not do
                     * after daemons
                     */
                     break;

                case 0x7f:            /* sometime generated by */
                                      /* suspend/foreground    */
                case ESCAPE:
                case ' ':
                    after = FALSE;    /* do nothing */
                    break;
                case 'Q':
                    after = FALSE;
                    quit();
                    break;
                case 'i':
                    after = FALSE;
                    inventory(pack, '*');
                    break;
                case 'I':
                    after = FALSE;
                    inventory(pack, 0);
                    break;
                case '~':
                    after = FALSE;
                    next_exp_level(MESSAGE);
                    break;
                case '>':
                    after = FALSE;
                    d_level();
                    break;
                case '<':
                    after = FALSE;
                    u_level();
                    break;
                case '?':
                    after = FALSE;
                    help();
                    break;
                case '/':
                    after = FALSE;
                    identify();
                    break;
                case 'v':
                    after = FALSE;
                    msg("UltraRogue Version %s.", release);
                    break;
                case 'o':
                    after = FALSE;
                    option();
                    strcpy(fd_data[1].mi_name, fruit);
                    break;
                case 12:    /* ctrl-l */
                case 18:    /* ctrl-r */
                    after = FALSE;
                    clearok(cw, TRUE);
					wrefresh(cw);
                    break;
                case 16: /* ctrl-p */
                {
                    int decrement = FALSE;
                    after = FALSE;

                    if (mpos == 0)
                        decrement = TRUE;

                    msg_index = (msg_index + 9) % 10;
                    msg(msgbuf[msg_index]);
                    if (decrement)
                        msg_index = (msg_index + 9) % 10;
                }
                break;

                case 'S':
                    after = FALSE;
                    if (save_game())
                    {
                        wclear(cw);
                        wrefresh(cw);
                        endwin();
                        exit(0);
                    }
                    break;

                /*
                 * Other misc commands
                 */

                case '.':   break;   /* rest */
                case ',':   add_pack(NULL, NOMESSAGE); break;
                case 'q': quaff(&player, -1, ISNORMAL); break;
                case 'r': read_scroll(&player, -1, ISNORMAL); break;
                case 'd': drop(NULL); break;
                case '^': set_trap(&player, hero.y, hero.x); break;
                case 'c': incant(&player, nullcoord); break;
                case 'D': dip_it(); break;
                case 'e': eat(); break;
                case '=': listen(); break;
                case 'A': apply(); break;
                case 'w': wield(); break;
                case 'W': wear(); break;
                case 'T': take_off(); break;
                case 'P': ring_on(); break;
                case 'R': ring_off(); break;
                case 'p': prayer(); break;
                case 'C': call(FALSE); break;
                case 'M': call(TRUE); break;
                case 's': search(FALSE); break;

                /*
                 * Directional commands - get_dir sets delta
                 */
                case 20:    /* ctrl-t */
                    if (get_dir())
                        steal();
                    else
                        after = FALSE;
                    break;

                case 'z':
                    if (get_dir())
                        do_zap(&player, -1, ISNORMAL);
                    else
                        after = FALSE;
                    break;

                case 'a':
                    if (get_dir())
                        affect();
                    else
                        after = FALSE;
                    touchwin(cw);
                    break;

                /*
                 * wizard commands
                 */

                case 0x17:    /* ctrl-w */
                    after = FALSE;

                    if (!wizard)
                    {
                        if (!canwizard)
                        {
                            msg("Illegal command '^W'.");
                            break;
                        }

                        if (passwd())
                        {
                            msg("Welcome, oh mighty wizard.");
                            wizard = waswizard = TRUE;
                        }
                        else
                        {
                            msg("Incorrect password.");
                            break;
                        }
                    }

                    msg("Wizard command: ");
                    mpos = 0;
                    ch = readchar();

                    switch (ch)
                    {
                        case 'v':
                            wiz_verbose = !wiz_verbose;
                            break;

                        case 'e':
                            wizard = FALSE;
                            msg("Not wizard any more.");
                            break;

                        case 's': activity(); break;
                        case 't': teleport(); break;
                        case 'm': overlay(mw, cw); break;
                        case 'f': overlay(stdscr, cw); break;
                        case 'i': inventory(lvl_obj, 0); break;
                        case 'c': buy_it('\0', ISNORMAL); break;
                        case 'I': whatis(NULL); break;
                        case 'F':
                             msg("food left: %d\tfood level: %d",
                                  food_left,foodlev);
                             break;
                        case 'M':
                            creat_mons(&player, get_monster_number("create"),
                                       MESSAGE);
                            break;

                        case 'r':
                            msg("rnd(4)%d, rnd(40)%d, rnd(100)%d",
                                 rnd(4), rnd(40), rnd(100));
                            break;

                        case 'C':
                            obj = get_object(pack, "charge", STICK, NULL);

                            if (obj != NULL)
                                obj->o_charges = 10000;

                            break;

                        case 'w':
                            obj = get_object(pack, "price", 0, NULL);

                            if (obj != NULL)
                                msg("Worth %d.", get_worth(obj));

                            break;

                        case 'g':
                            {
                                int tlev;

                                prbuf[0] = '\0';
                                msg("Which level? ");

                                if (get_string(prbuf, cw) == NORM)
                                {
                                    msg("");

                                    if ((tlev = atoi(prbuf)) < 1)
                                        msg("Illegal level.");
                                    else if (tlev > 3000)
                                    {
                                        levtype = THRONE;
                                        level = tlev - 3000;
                                    }
                                    else if (tlev > 2000)
                                    {
                                        levtype = MAZELEV;
                                        level = tlev - 2000;
                                    }
                                    else if (tlev > 1000)
                                    {
                                        levtype = POSTLEV;
                                        level = tlev - 1000;
                                    }
                                    else
                                    {
                                        levtype = NORMLEV;
                                        level = tlev;
                                    }

                                    new_level(levtype,0);
                                }
                            }
                            break;

                        case 'o':   make_omnipotent(); break;

                        case ESCAPE: /* Escape */
                            door_stop = FALSE;

                            count = 0;
                            after = FALSE;
                            break;

                        default:
                            msg("Illegal wizard command '%s', %d.",
                                unctrl(ch), ch);
                            count = 0;
                            break;

                    }

                break;

                default:
                    msg("Illegal command '%s', %d.",
                        unctrl(ch), ch);
                    count = 0;
                    after = FALSE;
                    break;
            }

            /*
             * turn off flags if no longer needed
             */
            if (!running)
                door_stop = FALSE;
        }

        /*
         * If he ran into something to take, let him pick it up.
         */
        if (take != 0)
            if (!moving)
                pick_up(take);
            else
                show_floor();
        if (!running)
            door_stop = FALSE;
    } /* end while */
}


void
do_after_effects(void)
{
    int i;

    /* Kick off the rest of the daemons and fuses */

    look(FALSE);
    do_daemons(AFTER);
    do_fuses(AFTER);

    /* Special abilities */

    if ((player.t_ctype == C_THIEF || player.t_ctype == C_ASSASIN ||
     player.t_ctype == C_NINJA || player.t_ctype == C_RANGER) &&
     (rnd(100) < (2 * pstats.s_dext + 5 * pstats.s_lvl)))
        search(TRUE);

    for (i = 0; i < ring_value(R_SEARCH); i++)
        search(FALSE);

    if (is_wearing(R_TELEPORT) && rnd(50) < 2)
    {
        teleport();

        if (off(player, ISCLEAR))
        {
            if (on(player, ISHUH))
                lengthen_fuse(FUSE_UNCONFUSE, rnd(8) + HUHDURATION);
            else
                light_fuse(FUSE_UNCONFUSE, 0, rnd(8) + HUHDURATION, AFTER);

            turn_on(player, ISHUH);
        }
        else
            msg("You feel dizzy for a moment, but it quickly passes.");
    }

    /* accidents and general clumsiness */

    if (fighting && rnd(50) == 0)
    {
        msg("You become tired of this nonsense.");
        fighting = after = FALSE;
    }

    if (on(player, ISELECTRIC))
        electrificate();

    if (!fighting && (no_command == 0) && cur_weapon != NULL
        && rnd(on(player, STUMBLER) ? 399 : 9999) == 0
        && rnd(pstats.s_dext) <
        2 - hitweight() + (on(player, STUMBLER) ? 4 : 0))
    {
        msg("You trip and stumble over your weapon.");
        running = after = FALSE;

        if (rnd(8) == 0 && (pstats.s_hpt -= roll(1, 10)) <= 0)
        {
            msg("You break your neck and die.");
            death(D_FALL);
            return;
        }
        else if (cur_weapon->o_flags & ISPOISON && rnd(4) == 0)
        {
            msg("You are cut by your %s!",
                inv_name(cur_weapon, LOWERCASE));

            if (player.t_ctype != C_PALADIN
                && !(player.t_ctype == C_NINJA &&
                pstats.s_lvl > 12)
                && !save(VS_POISON))
            {
                if (pstats.s_hpt == 1)
                {
                    msg("You die from the poison in the cut.");
                    death(D_POISON);
                    return;
                }
                else
                {
                    msg("You feel very sick now.");
                    pstats.s_hpt /= 2;
                    chg_str(-2, FALSE, FALSE);
                }
            }
       }
   }

   /* Time to enforce weapon and armor restrictions */
   if (rnd(9999) == 0)
        if (((cur_weapon == NULL) ||
            (wield_ok(&player, cur_weapon, NOMESSAGE)))
            && ((cur_armor == NULL) ||
            (wear_ok(&player, cur_armor, NOMESSAGE))))
        {
            switch (player.t_ctype)
            {
                case C_CLERIC:
                case C_DRUID:
                case C_RANGER:
                case C_PALADIN:
                    if (rnd(luck) != 0)
                        /* You better have done
                         * little wrong */
                        goto bad_cleric;

                    msg("You are enraptured by the renewed "
                        "power of your god.");
                    break;

                case C_MAGICIAN:
                case C_ILLUSION:
                    msg("You become in tune with the universe.");
                    break;

                case C_THIEF:
                case C_NINJA:
                case C_ASSASIN:
                    msg("You become supernaly sensitive to your "
                        "surroundings.");
                    break;

                case C_FIGHTER:
                    msg("You catch your second wind.");
                    break;

                default:
                    msg("What a strange type you are!");
                    break;
           }
           pstats.s_hpt = max_stats.s_hpt += rnd(pstats.s_lvl) + 1;
            pstats.s_power = max_stats.s_power += rnd(pstats.s_lvl) + 1;
        }
        else
        {   /* he blew it - make him pay */

            int death_cause = 0;

            switch (player.t_ctype)
            {
                case C_CLERIC:
                case C_DRUID:
                case C_RANGER:
                case C_PALADIN:
            bad_cleric:
                    msg("Your god scourges you for your misdeeds.");
                    death_cause = D_GODWRATH;
                    break;

                case C_MAGICIAN:
                case C_ILLUSION:
                    msg("You short out your manna on the unfamiliar %s.",
                        (cur_armor != NULL ? "armor" : "weapon"));

                   death_cause = D_SPELLFUMBLE;
                   break;

                case C_THIEF:
                case C_NINJA:
                case C_ASSASIN:
                    msg("You trip and fall because of the unfamiliar %s.",
                        (cur_armor != NULL ? "armor" : "weapon"));
                    death_cause = D_CLUMSY;
                    break;

                case C_FIGHTER:
                    debug("Fighter getting raw deal?");
                   break;

                default:
                    msg("What a strange type you are!");
                    break;
            }

            aggravate();
            pstats.s_power /= 2;
            pstats.s_hpt /= 2;
            player.t_no_move++;

            if ((pstats.s_hpt -= rnd(pstats.s_lvl)) <= 0)
            {
                death(death_cause);
            }
        }

    if (rnd(500000) == 0)
    {
        new_level(THRONE,0);
        fighting = running = after = FALSE;
        command();
    }
}

void
make_omnipotent(void)
{
    int i;
    struct linked_list  *item;
    struct object *obj;

    for (i = 0; i < 20; i++)
        raise_level();

    max_stats.s_hpt += 1000;
    max_stats.s_power += 1000;
    pstats.s_hpt = max_stats.s_hpt;
    pstats.s_power = max_stats.s_power;
    max_stats.s_str = pstats.s_str = 25;
    max_stats.s_intel = pstats.s_intel = 25;
    max_stats.s_wisdom = pstats.s_wisdom = 25;
    max_stats.s_dext = pstats.s_dext = 25;
    max_stats.s_const = pstats.s_const = 25;

    if (cur_weapon == NULL || cur_weapon->o_which != CLAYMORE)
    {
        item = spec_item(WEAPON, CLAYMORE, 10, 10);
        cur_weapon = OBJPTR(item);
        cur_weapon->o_flags |= ISKNOW;
        add_pack(item, NOMESSAGE);
    }

    /* and a kill-o-zap stick */

    item = spec_item(STICK, WS_DISINTEGRATE, 10000, 0);
    obj = OBJPTR(item);
    obj->o_flags |= ISKNOW;
    know_items[TYP_STICK][WS_DISINTEGRATE] = TRUE;

    if (guess_items[TYP_STICK][WS_DISINTEGRATE])
    {
        ur_free(guess_items[TYP_STICK][WS_DISINTEGRATE]);
        guess_items[TYP_STICK][WS_DISINTEGRATE] = NULL;
    }

    add_pack(item, NOMESSAGE);

    /* and his suit of armor */

    if (cur_armor == NULL ||
        !(cur_armor->o_which == CRYSTAL_ARMOR ||
          cur_armor->o_which == MITHRIL))
    {
        item = spec_item(ARMOR, CRYSTAL_ARMOR, 15, 0);
        obj = OBJPTR(item);
        obj->o_flags |= ISKNOW;
        obj->o_weight =
            (int) (armors[CRYSTAL_ARMOR].a_wght * 0.2);
        cur_armor = obj;
        add_pack(item, NOMESSAGE);
    }

    /* and some rings (have to put them on, for now) */


    if (!is_wearing(R_SEARCH))
    {
        item = spec_item(RING, R_SEARCH, 0, 0);
        obj = OBJPTR(item);
        obj->o_flags |= ISKNOW;
        know_items[TYP_RING][R_SEARCH] = TRUE;

        if (guess_items[TYP_RING][R_SEARCH])
        {
            ur_free(guess_items[TYP_RING][R_SEARCH]);
            guess_items[TYP_RING][R_SEARCH] = NULL;
        }

        add_pack(item, NOMESSAGE);
    }

    if (!is_wearing(R_PIETY))
    {
        item = spec_item(RING, R_PIETY, 0, 0);
        obj = OBJPTR(item);
        obj->o_flags |= ISKNOW;
        know_items[TYP_RING][R_PIETY] = TRUE;

        if (guess_items[TYP_RING][R_PIETY])
        {
            ur_free(guess_items[TYP_RING][R_PIETY]);
            guess_items[TYP_RING][R_PIETY] = NULL;
        }

        add_pack(item, NOMESSAGE);
    }

    item = spec_item(SCROLL, S_ELECTRIFY, 0, 0);
    obj = OBJPTR(item);
    obj->o_flags |= ISKNOW;
    know_items[TYP_SCROLL][S_ELECTRIFY] = TRUE;

    if (guess_items[TYP_SCROLL][S_ELECTRIFY])
    {
        ur_free(guess_items[TYP_SCROLL][S_ELECTRIFY]);
        guess_items[TYP_SCROLL][S_ELECTRIFY] = NULL;
    }

    add_pack(item, NOMESSAGE);

    /* Spiff him up a bit */
    quaff(&player, P_SHERO, ISBLESSED);
    quaff(&player, P_CLEAR, ISBLESSED);
    quaff(&player, P_FIRERESIST, ISBLESSED);
    quaff(&player, P_TRUESEE, ISBLESSED);
    quaff(&player, P_PHASE, ISBLESSED);
    purse += 50000L;
    updpack();
}


/*
    quit()
        Have player make certain, then exit.
*/

void
quit_handler(int sig)
{
    if (signal(SIGINT, quit_handler) != quit_handler)
        mpos = 0;

    sig = 0;

    quit();
}

void
quit(void)
{
    msg("Really quit?");

    wrefresh(cw);

    if (readchar() == 'y')
    {
        clear();
        wclear(cw);
        wrefresh(cw);
        move(LINES - 1, 0);
        wrefresh(stdscr);
        score(pstats.s_exp, pstats.s_lvl, CHICKEN, 0);
        byebye();
    }
    else
    {
        signal(SIGINT, quit_handler);
        wmove(cw, 0, 0);
        wclrtoeol(cw);
        status(FALSE);
        wrefresh(cw);
        mpos = 0;
        count = 0;
        fighting = running = 0;
    }
}

/*
    search()
        Player gropes about him to find hidden things.
*/

void
search(int is_thief)
{
    int x, y;
    char ch;

    /*
     * Look all around the hero, if there is something hidden there, give
     * him a chance to find it.  If its found, display it.
     */

    if (on(player, ISBLIND))
        return;

    for (x = hero.x - 1; x <= hero.x + 1; x++)
        for (y = hero.y - 1; y <= hero.y + 1; y++)
        {
            ch = winat(y, x);

            if (isatrap(ch))
            {
                static char trname[1024]; /* temp scratch space */
                struct trap *tp;
                struct room *rp;

                if (isatrap( mvwinch(cw, y, x)))
                    continue;

                tp = trap_at(y, x);

                if ((tp->tr_flags & ISTHIEFSET) ||
                    (rnd(100) > 50 && !is_thief))
                    break;

                rp = roomin(hero);

                if (tp->tr_type == FIRETRAP && rp != NULL)
                {
                    rp->r_flags &= ~ISDARK;
                    light(&hero);
                }

                tp->tr_flags |= ISFOUND;
                mvwaddch(cw, y, x, ch);
                count = 0;
                running = FALSE;
                msg(tr_name(tp->tr_type,trname));
            }
            else if (ch == SECRETDOOR)
            {
                if (rnd(100) < 20 && !is_thief)
                {
                    mvaddch(y, x, DOOR);
                    count = 0;
                }
            }
        }
}

/*
    help()
        Give single character help, or the whole mess if he wants it
*/

void
help(void)
{
    const struct h_list *strp = helpstr;
    char helpch;
    int cnt;

    msg("Character you want help for (* for all): ");
    helpch = readchar();
    mpos = 0;

    /*
     * If its not a *, print the right help string or an error if he
     * typed a funny character.
     */

    if (helpch != '*')
    {
        wmove(cw, 0, 0);

        while (strp->h_ch)
        {
            if (strp->h_desc == 0)
                if (!wizard)
                    break;
                else
                {
                    strp++;
                    continue;
                }

            if (strp->h_ch == helpch)
            {
                msg("%s%s", unctrl(strp->h_ch), strp->h_desc);
                break;
            }
            strp++;
        }

        if (strp->h_ch != helpch)
            msg("Unknown character '%s'.", unctrl(helpch));

        return;
    }

    /*
     * Here we print help for everything. Then wait before we return to
     * command mode
     */

    wclear(hw);
    cnt = 0;

    while (strp->h_ch)
    {
        if (strp->h_desc == 0)
            if (!wizard)
                break;
            else
            {
                strp++;
                continue;
            }

        mvwaddstr(hw, cnt % 23, cnt > 22 ? 40 : 0, unctrl(strp->h_ch));
        waddstr(hw, strp->h_desc);
        strp++;

        if (++cnt >= 46 && strp->h_ch && (strp->h_desc != NULL || wizard))
        {
            wmove(hw, LINES - 1, 0);
            wprintw(hw, (char *) morestr);
            wrefresh(hw);
            wait_for(' ');
            wclear(hw);
            cnt = 0;
        }
    }

    wmove(hw, LINES - 1, 0);
    wprintw(hw, (char *) morestr);
    wrefresh(hw);
    wait_for(' ');
    wclear(hw);
    wrefresh(hw);

    wmove(cw, 0, 0);
    wclrtoeol(cw);
    status(FALSE);
    touchwin(cw);

    return;
}

/*
    identify()
        Tell the player what a certain thing is.
*/

void
identify(void)
{
    int   ch;
    char *str;

    msg("What do you want identified? ");
    mpos = 0;

    if ((ch = readchar()) == ESCAPE)
    {
        msg("");
        return;
    }

    if (isalpha(ch))
    {
        id_monst(ch);
        return;
    }

    switch (ch)
    {
        case '|':
        case '-':        str = "wall of a room";                 break;
        case GOLD:       str = "gold";                           break;
        case STAIRS:     str = "passage leading down";           break;
        case DOOR:       str = "door";                           break;
        case FLOOR:      str = "room floor";                     break;
        case VPLAYER:    str = "The hero of the game ---> you";  break;
        case IPLAYER:    str = "you (but invisible)";            break;
        case PASSAGE:    str = "passage";                        break;
        case POST:       str = "trading post";                   break;
        case POOL:       str = "a shimmering pool";              break;
        case TRAPDOOR:   str = "trapdoor";                       break;
        case ARROWTRAP:  str = "arrow trap";                     break;
        case SLEEPTRAP:  str = "sleeping gas trap";              break;
        case BEARTRAP:   str = "bear trap";                      break;
        case TELTRAP:    str = "teleport trap";                  break;
        case DARTTRAP:   str = "dart trap";                      break;
        case MAZETRAP:   str = "entrance to a maze";             break;
        case FIRETRAP:   str = "fire trap";                      break;
        case POISONTRAP: str = "poison pool trap";               break;
        case LAIR:       str = "monster lair entrance";          break;
        case RUSTTRAP:   str = "rust trap";                      break;
        case POTION:     str = "potion";                         break;
        case SCROLL:     str = "scroll";                         break;
        case FOOD:       str = "food";                           break;
        case WEAPON:     str = "weapon";                         break;
        case ' ':        str = "solid rock";                     break;
        case ARMOR:      str = "armor";                          break;
        case ARTIFACT:   str = "an artifact from bygone ages";   break;
        case RING:       str = "ring";                           break;
        case STICK:      str = "wand or staff";                  break;
        default:         str = "unknown character";              break;
    }
    msg("'%s'; %s", unctrl(ch), str);
}

/*
    d_level()
        He wants to go down a level
*/

void
d_level(void)
{
    int no_phase = FALSE;

    if (mvinch(hero.y, hero.x) != STAIRS)
    {
        if (off(player, CANINWALL))  /* Must use stairs if can't phase */
        {
            msg("I see no way down.");
            return;
        }

        extinguish_fuse(FUSE_UNPHASE);/*Using phase to go down gets rid of it*/
        no_phase = TRUE;
    }

    if (is_wearing(R_LEVITATION) || on(player, CANFLY))
    {
        msg("You can't!  You're floating in the air.");
        return;
    }

    if (rnd(pstats.s_dext) < 3 * (2 - hitweight() +
        (on(player, STUMBLER) ? 4 : 0)))
    {
        msg("You trip and fall down the stairs.");

        if ((pstats.s_hpt -= roll(1, 10)) <= 0)
        {
            msg("You break your neck and die.");
            death(D_FALL);
            return;
        }
    }

    level++;
    new_level(NORMLEV,0);

    if (no_phase)
        unphase(NULL);

    return;
}

/*
    u_level()
        He wants to go up a level
*/

void
u_level(void)
{
    char ch = 0;

    if (has_artifact && ((ch = CCHAR(mvinch(hero.y, hero.x))) == STAIRS ||
                 (on(player, CANINWALL)
            && (is_wearing(R_LEVITATION) || on(player, CANFLY)))))
    {
        if (--level == 0)
            total_winner();
        else if (rnd(wizard ? 3 : 15) == 0)
            new_level(THRONE,0);
        else
        {
            new_level(NORMLEV,0);
            msg("You feel a wrenching sensation in your gut.");
        }

        if (on(player, CANINWALL) && ch != STAIRS)
        {
            extinguish_fuse(FUSE_UNPHASE);
            unphase(NULL);
        }

        return;
    }
    else if (ch != STAIRS &&
         !(on(player, CANINWALL) && (is_wearing(R_LEVITATION)
                         || on(player, CANFLY))))
        msg("I see no way up.");
    else
        msg("Your way is magically blocked.");

    return;
}

/*
    call()
        allow a user to call a potion, scroll, or ring something
*/

void
call(int mark)
{
    struct object    *obj;
    char             *elsewise;
    int item_type =   numthings;
    char            **item_color = NULL;

    if (mark)
        obj = get_object(pack, "mark", 0, bff_markable);
    else
        obj = get_object(pack, "call", 0, bff_callable);

    if (obj == NULL)
        return;

    switch (obj->o_type)
    {
        case RING:
            item_type = TYP_RING;
            item_color = r_stones;
            break;
        case POTION:
            item_type = TYP_POTION;
            item_color = p_colors;
            break;
        case SCROLL:
            item_type = TYP_SCROLL;
            item_color = s_names;
            break;
        case STICK:
            item_type = TYP_STICK;
            item_color = ws_made;
        default:
            if (!mark)
            {
                msg("You can't call that anything.");
                return;
            }
            break;
    }

    elsewise = (guess_items[item_type][obj->o_which] != NULL ?
       guess_items[item_type][obj->o_which] : item_color[obj->o_which]);

    if (know_items[item_type][obj->o_which] && !mark)
    {
        msg("That has already been identified.");
        return;
    }

    if (mark)
    {
        if (obj->o_mark[0])
            msg("Was marked \"%s\".", obj->o_mark);

        msg("What do you want to mark it? ");
        prbuf[0] = '\0';
    }
    else
    {
        msg("Was called \"%s\".", elsewise);
        msg("What do you want to call it? ");

        if (guess_items[item_type][obj->o_which] != NULL)
            ur_free(guess_items[item_type][obj->o_which]);

        strcpy(prbuf, elsewise);
    }

    if (get_string(prbuf, cw) == NORM)
    {
        if (mark)
        {
            strncpy(obj->o_mark, prbuf, MARKLEN - 1);
            obj->o_mark[MARKLEN - 1] = '\0';
        }
        else
        {
            guess_items[item_type][obj->o_which] = new_alloc(strlen(prbuf) + 1);
            strcpy(guess_items[item_type][obj->o_which], prbuf);
        }
    }

    return;
}

/*
    att_bonus()
        bonus attacks for certain player classes
*/

int
att_bonus(void)
{
    int bonus = FALSE;

    if ((player.t_ctype == C_FIGHTER || player.t_ctype == C_PALADIN)
            && (pstats.s_lvl > 12 ||
            (pstats.s_lvl > 6 && pstats.s_lvl < 13 && rnd(2))))
        bonus = TRUE;

    else if ((player.t_ctype == C_RANGER)
             && (pstats.s_lvl > 14 ||
             (pstats.s_lvl > 7 && pstats.s_lvl < 15 && rnd(2))))
        bonus = TRUE;

    else if ((player.t_ctype == C_NINJA)
             && (pstats.s_lvl > 8 ||
             (pstats.s_lvl > 4 && pstats.s_lvl < 9 && rnd(2))))
        bonus = TRUE;

    return(bonus);
}
