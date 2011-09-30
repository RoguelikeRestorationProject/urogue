/*
    random.c - random and associated routines
   
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1992, 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

void
ur_srandom(unsigned x)
{
    md_srandom(x);
}

long
ur_random(void)
{
    return( md_random() );
}
