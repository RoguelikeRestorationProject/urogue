/*
    save.c - save and restore routines
 
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

#define _ALL_SOURCE /* need to remove need for this AIXism */

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "rogue.h"

int
save_game(void)
{
    FILE *savefd;
    char    buf[2 * LINELEN];
    char    oldfile[2*LINELEN];

    /* get file name */

    strcpy(oldfile,file_name);

    do
    {
        mpos = 0;

        if (oldfile[0] != '\0')
            msg("Save file [%s]: ", file_name);
        else
            msg("Save file as: ");

        mpos = 0;
        buf[0] = '\0';

        if (get_string(buf, cw) == QUIT)
        {
            msg("");
            return(FALSE);
        }

        if ( (buf[0] == 0) && (oldfile[0] != 0) )
            strcpy(file_name, oldfile);
        else if (buf[0] != 0)
            strcpy(file_name, buf);
        else
        {
            msg("");
            return(FALSE);
        }

        wclear(hw);
        wmove(hw, LINES - 1, 0);
        wrefresh(hw);

        if ((savefd = fopen(file_name, "w")) == NULL)
            msg(strerror(errno));    /* fake perror() */
    }
    while (savefd == NULL);

    /* write out [compressed?] file */

    save_file(savefd);
    return(TRUE);
}

int
restore(char *file)
{
    FILE *infd;
    char    *sp;

    if (strcmp(file, "-r") == 0)
        file = file_name;

    if ((infd = fopen(file, "r")) == NULL)
    {
        perror(file);
        return(FALSE);
    }

    if ( restore_file(infd) == FALSE )
        return(FALSE);

    /*
     * we do not close the file so that we will have a hold of the inode
     * for as long as possible
     */

    if (remove(file) < 0)
    {
        printf("Cannot unlink file\n");
        return(FALSE);
    }

    if ((sp = getenv("OPTIONS")) != NULL)
        parse_opts(sp);

    strcpy(file_name, file);

    clearok(cw, TRUE);
    touchwin(cw);
    noecho();
    nonl();

    while(playing)
    {
        do_daemons(BEFORE);
        do_fuses(BEFORE);

        command();  /* Command execution */

        if (after)
            do_after_effects();
    }

    fatal("");

    return(FALSE);
}
