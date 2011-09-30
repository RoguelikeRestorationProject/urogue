/*
    verify.c - exiting functions

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.
*/

static char sccsid[] = "%W% %G%";

#include "rogue.h"

void verify_function(const char *file, const int line)
{
	char s[80];

	sprintf(s, "Verify failure in %s at line %d\n", file, line);
	fatal(s);
}
