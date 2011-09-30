/*
    main.c  -  setup code
 
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

#define _ALL_SOURCE

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include "rogue.h"

FILE *fd_score = NULL;

/* Command line options */

int prscore;        /* Print scores */
int prversion;      /* Print version info */

int
main(int argc, char *argv[])
{
    int x;
    char    *env;
    time_t lowtime;
    time_t    now;
    int rflag = 0;
    char *nm;
	float scale;

    for (x = 1; x < argc; x++)
    {
        if (argv[x][0] != '-')
            break;

        switch (argv[x][1])
        {
            case 's':
                prscore = TRUE;
                break;

            case 'v':
                prversion = TRUE;
                break;

            case 'r':
                rflag = TRUE;
                break;

            default:
                fprintf(stderr,"%s: Unknown option '%c'.\n",argv[0],argv[x][1]);
                exit(1);
        }
    }

    if (!rflag)
    {
        argc -= (x - 1);
        argv += (x - 1);
    }

    /* Get default score file */

    strcpy(score_file, "urogue.scr");

    fd_score = fopen(score_file, "r+");

    if (fd_score == NULL)
        fd_score = fopen(score_file, "a+");

    if ((env = getenv("OPTIONS")) != NULL)
        parse_opts(env);

    nm = getenv("USER");

    if (nm != NULL)
        strcpy(whoami,nm);
    else
        strcpy(whoami,"anonymous");
		
	lowtime = time(&now);
	
	dnum = (wizard && getenv("SEED") != NULL ? atoi( getenv("SEED")) : (int)lowtime);
	
	ur_srandom(dnum);

    if (env == NULL || fruit[0] == '\0')
    {
        static const char *funfruit[] =
        {
            "candleberry", "caprifig", "dewberry", "elderberry",
            "gooseberry", "guanabana", "hagberry", "ilama", "imbu",
            "jaboticaba", "jujube", "litchi", "mombin", "pitanga",
            "prickly pear", "rambutan", "sapodilla", "soursop",
            "sweetsop", "whortleberry"
        };

        strcpy(fruit, funfruit[rnd(sizeof(funfruit) / sizeof(funfruit[0]))]);
    }

    /* put a copy of fruit in the right place */

    fd_data[1].mi_name = md_strdup(fruit); 

    /* print scores */

    if (prscore)
    {
        waswizard = TRUE;
        score(0L, 0, SCOREIT, 0);
        exit(0);
    }

    /* check for version option */

    if (prversion)
    {
        printf("UltraRogue Version %s.\n", release);
        exit(0);
    }

    if (wizard)
        printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
        printf("Hello %s, just a moment while I dig the dungeon...", whoami);

    mem_debug(2);
	mem_tracking(1);

    fflush(stdout);

    init_things();      /* Set up probabilities of things */
    init_fd();          /* Set up food probabilities */
    init_colors();      /* Set up colors of potions */
    init_stones();      /* Set up stone settings of rings */
    init_materials();   /* Set up materials of wands */
    initscr();          /* Start up cursor package */
    refresh();
    init_names();       /* Set up names of scrolls */
    cbreak();
    crmode();           /* Cbreak mode */
    noecho();           /* Echo off */
    nonl();
	
    scale = (float) (LINES * COLS) / (80.0F * 25.0F); /* get food right for     */
	                                              /* different screen sizes */
												  
    food_left = (int) (food_left * scale);

    /* Set up windows */

    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    if (argc == 2 && argv[1][0] != '\0' && !restore(argv[1]))
        /* Note: restore returns on error only */
        exit(1);

    waswizard = wizard; /* set wizard flags */

    init_player();      /* look up things and outfit pack */

    resurrect = pstats.s_const;
    init_exp();         /* set first experience level change */
    init_flags();       /* set initial flags */
    wclear(hw);
    wrefresh(hw);
    new_level(POSTLEV,0); /* Draw current level */

    /* Start up daemons and fuses */

    start_daemon(DAEMON_DOCTOR, &player, AFTER);

    light_fuse(FUSE_SWANDER, 0, WANDERTIME, AFTER);

    start_daemon(DAEMON_STOMACH, 0, AFTER);
    start_daemon(DAEMON_RUNNERS, 0, AFTER);

    char_type = player.t_ctype;
    player.t_oldpos = hero;
    oldrp = roomin(hero);
    after = TRUE;

    signal(SIGINT, quit_handler);

    while(playing)
    {
        do_daemons(BEFORE);
        do_fuses(BEFORE);

        command();  /* Command execution */

        if (after)
            do_after_effects();
    }

    fatal("");

    return(0);
}

/*
    fatal()
        Exit the program, printing a message.
*/

void
fatal(char *s)
{
    clear();
    move(LINES - 2, 0);
    printw("%s", s);
    wrefresh(stdscr);
    endwin();
    printf("\n");       /* So the cursor doesn't stop at the end of the line */
    exit(100);
}

/*
    rnd()
        Pick a very random number.
*/

unsigned char
ucrnd(unsigned char range)
{
    return (unsigned char)(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

short
srnd(short range)
{
    return (short)(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

int
rnd(int range)
{
    return (range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

unsigned long
ulrnd(unsigned long range)
{
    return(range <= 0 ? 0 : (ur_random() & 0x7fffffffL) % range);
}

/*
    roll()
        roll a number of dice
*/

int
roll(int number, int sides)
{
    int dtotal = 0;

    while (number--)
        dtotal += rnd(sides) + 1;

    return(dtotal);
}
