/*
    getplay.c - Procedures for saving and retrieving a characters starting
                attributes, armour, and weapon.
  
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/* 11/08/83  ???, S.A. Hester */

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "rogue.h"

#define I_STR       0
#define I_INTEL     1
#define I_WISDOM    2
#define I_DEXT      3
#define I_CONST     4
#define I_CHARISMA  5
#define I_HPT       6
#define I_POWER     7
#define I_CTYPE     8
#define MAXPATT     9   /* Total Number of above defines. */
#define MAXPDEF     10  /* Maximum number of pre-defined chars */

static int def_array[MAXPDEF][MAXPATT];    /* Pre-def'd chars */

static void get_chr_filename(char *filename, int size)
{
    const char *home;
 
    home = getenv("HOME");

    if (home) {
        if ((int)strlen(home) < (size - 12) )
        {
            strcpy(filename, home);
            strcat(filename,"/urogue.chr");
        }
        else
	    strncpy(filename,"urogue.chr",size);
    }    
    else 
        strcpy(filename, "urogue.chr");
}

int
geta_player(void)
{
    int  i;
    FILE *fd;
    char pbuf[2 * LINELEN];
    char filename[200];

    get_chr_filename(filename, sizeof(filename));

    if ((fd = fopen(filename, "r")) == NULL)
        return(FALSE);

    fread(def_array, sizeof(def_array), 1, fd);
    fclose(fd);

    wclear(hw);
    touchwin(hw);

    print_stored();
    mvwaddstr(hw, 0, 0, "Do you wish to select a character? ");
    wrefresh(hw);

    if (readcharw(hw) != 'y')
        return FALSE;

    do
    {
        wmove(hw, LINES - 1, 0);
        wclrtoeol(hw);
        mvwaddstr(hw, 0, 0, "Enter the number of a pre-defined character: ");
        wclrtoeol(hw);
        wrefresh(hw);
        get_string(pbuf, hw);
        i = atoi(pbuf) - 1;

        if (i < 0 || i > MAXPDEF - 1)
        {
            wstandout(hw);
            mvwaddstr(hw, 1, 0, "Please use the range 1 to");
            wprintw(hw, " %d.", MAXPDEF);
            wstandend(hw);
            wclrtoeol(hw);
            wrefresh(hw);
        }
        else if (def_array[i][I_STR] == 0)
        {
            wstandout(hw);
            mvwaddstr(hw,1,0,"Please enter the number of a known character: ");
            wstandend(hw);
            wclrtoeol(hw);
        }
        else
        {
            mvwaddstr(hw, 1, 0, "");
            wclrtoeol(hw);
        }

    }
    while (i < 0 || i > MAXPDEF - 1 || (def_array[i][I_STR] == 0));

    pstats.s_str      = def_array[i][I_STR];
    pstats.s_intel    = def_array[i][I_INTEL];
    pstats.s_wisdom   = def_array[i][I_WISDOM];
    pstats.s_dext     = def_array[i][I_DEXT];
    pstats.s_const    = def_array[i][I_CONST];
    pstats.s_charisma = def_array[i][I_CHARISMA];
    pstats.s_hpt      = def_array[i][I_HPT];
    pstats.s_power    = def_array[i][I_POWER];
    player.t_ctype    = char_type = def_array[i][I_CTYPE];
    max_stats         = pstats;

    return(TRUE);
}

void
puta_player(void)
{
    FILE *fd;
    char    pbuf[2 * LINELEN];
    char filename[200];
    int   i;
    char    *class = which_class(player.t_ctype);

    sprintf(pbuf, "You have a %s with the following attributes:", class);
    mvwaddstr(hw, 2, 0, pbuf);
    wclrtoeol(hw);

    sprintf(pbuf,
        "Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d Pow: %d Hpt: %d",
        pstats.s_intel,
        pstats.s_str,
        pstats.s_wisdom,
        pstats.s_dext,
        pstats.s_const,
        pstats.s_charisma,
        pstats.s_power,
        pstats.s_hpt );

    mvwaddstr(hw, 3, 0, "");
    wclrtoeol(hw);
    mvwaddstr(hw, 4, 0, pbuf);
    wclrtoeol(hw);
    mvwaddstr(hw, 5, 0, "");
    wclrtoeol(hw);
    mvwaddstr(hw, 0, 0, "Would you like to save this character?");
    wclrtoeol(hw);


    wrefresh(hw);

    if ((readcharw(hw) & 0177) != 'y')
        return;

    do
    {
        mvwaddstr(hw, 0, 0, "Overwrite which number? ");
        wclrtoeol(hw);
        wrefresh(hw);
        get_string(pbuf, hw);
        i = atoi(pbuf) - 1;

        if (i < 0 || i > MAXPDEF - 1)
        {
            wstandout(hw);
            mvwaddstr(hw, 1, 0, "Use the range 1 to");
            wprintw(hw, " %d!", MAXPDEF);
            wstandend(hw);
            wclrtoeol(hw);
            wrefresh(hw);
        }
    }
    while (i < 0 || i > MAXPDEF - 1);

    /* Set some global stuff */

    def_array[i][I_STR]      = pstats.s_str;
    def_array[i][I_INTEL]    = pstats.s_intel;
    def_array[i][I_WISDOM]   = pstats.s_wisdom;
    def_array[i][I_DEXT]     = pstats.s_dext;
    def_array[i][I_CONST]    = pstats.s_const;
    def_array[i][I_CHARISMA] = pstats.s_charisma;
    def_array[i][I_HPT]      = pstats.s_hpt;
    def_array[i][I_POWER]    = pstats.s_power;
    def_array[i][I_CTYPE]    = player.t_ctype;

    /* OK. Now let's write this stuff out! */

    get_chr_filename(filename, sizeof(filename));


    if ((fd = fopen(filename, "w")) == NULL)
    {
        sprintf(pbuf, "I can't seem to open/create urogue.chr.");
        mvwaddstr(hw, 5, 5, pbuf);
        mvwaddstr(hw, 6, 5, "However I'll let you play it anyway!");
        mvwaddstr(hw, LINES - 1, 0, spacemsg);
        wrefresh(hw);
        wait_for(' ');

        return;
    }

    fwrite(def_array, sizeof(def_array), 1, fd);
    fclose(fd);
    return;
}

void
do_getplayer(void)
{
	print_stored();

    if (char_type == C_NOTSET)
        do
        {
            /* See what type character will be */

            mvwaddstr(hw, 3, 0, "[a] Fighter\t"
                                "[b] Paladin\t"
                                "[c] Ranger\n"
                                "[d] Cleric\t"
                                "[e] Druid\t"
                                "[f] Magician\n"
                                "[g] Illusionist\t"
                                "[h] Thief\t"
                                "[i] Assasin\t"
                                "[j] Ninja");

            mvwaddstr(hw, 0, 0, "What character class do you desire? ");
            wrefresh(hw);
            char_type = readcharw(hw) - 'a';

            if (char_type < C_FIGHTER || char_type >= C_MONSTER)
            {
                wstandout(hw);
                mvwaddstr(hw, 1, 0, "Please enter a letter from a - j");
                wstandend(hw);
                wclrtoeol(hw);
                wrefresh(hw);
            }
            else
            {
                mvwaddstr(hw, 1, 0, "");
                wclrtoeol(hw);
            }
        }
        while (char_type < C_FIGHTER || char_type >= C_MONSTER);

   player.t_ctype = char_type;
}

void
print_stored(void)
{
    int i;
    char    *class;
    char    pbuf[2 * LINELEN];

    wstandout(hw);
    mvwaddstr(hw, 9, 0, "YOUR CURRENT CHARACTERS:");
    wstandend(hw);
    wclrtoeol(hw);

    for (i = 0; i < MAXPDEF; i++)
    {
        if (def_array[i][I_STR])
        {
            class = which_class(def_array[i][I_CTYPE]);

            sprintf(pbuf,
                "%2d. (%s): Int: %d Str: %d Wis: %d Dex: %d Con: %d Cha: %d"
                " Pow: %d Hpt: %d",
                i + 1,
                class,
                def_array[i][I_INTEL],
                def_array[i][I_STR],
                def_array[i][I_WISDOM],
                def_array[i][I_DEXT],
                def_array[i][I_CONST],
                def_array[i][I_CHARISMA],
                def_array[i][I_POWER],
                def_array[i][I_HPT]);

            mvwaddstr(hw, 11 + i, 0, pbuf);

        }
        else
        {
            sprintf(pbuf, "%2d.  ### NONE ###", i + 1);
            mvwaddstr(hw, 11 + i, 0, pbuf);
        }
    }
}

char *
which_class(int c_class)
{
    char    *class;

    switch (c_class)
    {
        case C_FIGHTER:   
		    class = "Fighter"; 
			break;
        case C_MAGICIAN:  
		    class = "Magician"; 
			break;
        case C_CLERIC:    
		    class = "Cleric"; 
			break;
        case C_THIEF:     
		    class = "Thief"; 
			break;
        case C_PALADIN:   
		    class = "Paladin"; 
			break;
        case C_RANGER:    
		    class = "Ranger"; 
			break;
        case C_DRUID:     
		    class = "Druid"; 
			break;
        case C_ILLUSION:  
		    class = "Illusionist"; 
			break;
        case C_ASSASIN:   
		    class = "Assasin"; 
			break;
        case C_NINJA:     
		    class = "Ninja"; 
			break;
        default:          
		    class = "Monster"; 
			break;
    }

    return (class);
}
