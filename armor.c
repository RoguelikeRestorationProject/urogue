/*
    armor.c  -  functions for dealing with armor

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
    wear()
        The player wants to wear something, so let him/her put it on.
*/

void
wear(void)
{
    struct object *obj;

    if (cur_armor != NULL)
    {
        msg("You are already wearing some.");

        after = FALSE;

        return;
    }

    /* What does player want to wear? */

    if ((obj = get_object(pack, "wear", ARMOR, NULL)) == NULL)
        return;

    wear_ok(&player, obj, MESSAGE);
    waste_time();

    cur_armor = obj;
    obj->o_flags |= ISKNOW;

    msg("You are now wearing %s.", inv_name(obj, TRUE));

    return;
}

/*
    take_off()
        Get the armor off of the players back
*/

void
take_off(void)
{
    struct object   *obj;

    if ((obj = cur_armor) == NULL)
    {
        msg("You aren't wearing armor!");
        return;
    }

    if (!dropcheck(cur_armor))
        return;

    msg("You were wearing %c%c) %s.", ARMOR, print_letters[get_ident(obj)],
        inv_name(obj, LOWERCASE));

    cur_armor = NULL;

    if (on(player, STUMBLER))
    {
        msg("Your foot feels a lot better now.");
        turn_off(player, STUMBLER);
    }
}

/*
    wear_ok()
        enforce player class armor restrictions
*/

int
wear_ok(struct thing *wearee, struct object *obj, int print_message)
{
    int which      = obj->o_which;
    int ret_val    = TRUE;
    int class_type = wearee->t_ctype;

    if (obj->o_type != ARMOR)
        return(FALSE);
    else
        switch (class_type)
        {
            case C_MAGICIAN: /* cannot wear metal */
            case C_ILLUSION:
                switch (which)
                {
                    case RING_MAIL:
                    case SCALE_MAIL:
                    case PADDED_ARMOR:
                    case CHAIN_MAIL:
                    case BRIGANDINE:
                    case SPLINT_MAIL:
                    case GOOD_CHAIN:
                    case PLATE_MAIL:
                    case PLATE_ARMOR:
                        ret_val = FALSE;
                        break;
                    default:
                        break;
                }

            case C_THIEF:    /* cannot clank around */
            case C_ASSASIN:
            case C_NINJA:
                switch (which)
                {
                    case CHAIN_MAIL:
                    case BRIGANDINE:
                    case SPLINT_MAIL:
                    case GOOD_CHAIN:
                    case PLATE_MAIL:
                    case PLATE_ARMOR:
                        ret_val = FALSE;
                        break;
                    default:
                        break;
                }

            case C_CLERIC:   /* cannot wear plate */
            case C_DRUID:
                switch (which)
                {
                    case PLATE_MAIL:
                    case PLATE_ARMOR:
                    case MITHRIL:
                        ret_val = FALSE;
                        break;
                    default:
                        break;
                }

            case C_FIGHTER: /* wear anything */
            case C_RANGER:
                break;

            case    C_PALADIN:  /* cannot wear common stuff */
                switch (which)
                {
                    case SOFT_LEATHER:
                    case CUIRBOLILLI:
                    case HEAVY_LEATHER:
                    case STUDDED_LEATHER:
                    case PADDED_ARMOR:
                    case BRIGANDINE:
                        ret_val = FALSE;
                        break;
                    default:
                        break;
                }

            case C_MONSTER:
                break;

            default:      /* Unknown class */
                debug("Unknown class %d.", class_type);
                break;
        }

    if (ret_val == FALSE && print_message == MESSAGE)
        switch (class_type)
        {
            case C_MAGICIAN:
            case C_ILLUSION:
                msg("You cannot regenerate spell points while wearing that!");
                break;

            case C_THIEF:
            case C_ASSASIN:
            case C_NINJA:
                msg("Don't expect to be stealthy while wearing that!");
                break;

            case C_CLERIC:
            case C_DRUID:
            case C_PALADIN:
                msg("Your god strongly disapproves of your wearing that!");
                break;

            case C_FIGHTER:
            case C_RANGER:
            case C_MONSTER:
                break;
        }

    return(ret_val);
}
