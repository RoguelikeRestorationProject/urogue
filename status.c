/*
    status.c - functions for complex status determination of monsters/objects
         
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1992, 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

/*
    has_defensive_spell()
        has monster cast a defensive spell.
        Any flags added here must also be in player_powers[].
*/

int
has_defensive_spell(struct thing th)
{
    if (on(th, HASOXYGEN))
        return(TRUE);
    if (on(th, CANFLY))
        return(TRUE);
    if (on(th, CANINWALL))
        return(TRUE);
    if (on(th, CANREFLECT))
        return(TRUE);
    if (on(th, CANSEE))
        return(TRUE);
    if (on(th, HASMSHIELD))
        return(TRUE);
    if (on(th, HASSHIELD))
        return(TRUE);
    if (on(th, ISHASTE))
        return(TRUE);
    if (on(th, ISREGEN))
        return(TRUE);
    if (on(th, ISDISGUISE))
        return(TRUE);
    if (on(th, ISINVIS))
        return(TRUE);
    if (on(th, NOCOLD))
        return(TRUE);
    if (on(th, NOFIRE))
        return(TRUE);
    if (on(th, ISELECTRIC))
        return(TRUE);

    return(FALSE);
}
