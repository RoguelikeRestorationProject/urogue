/*
    maze.c  -  functions for dealing with armor
    
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <stdlib.h>
#include "rogue.h"

static char *frontier;
static char *bits;
static int urlines;
static int urcols;

/*
    domaze()
        Draw the maze on this level.
*/

void
do_maze(void)
{
    int     i, least;
    struct room *rp;
    struct linked_list  *item;
    struct object   *obj;
    struct thing    *mp;
    int    treas;
    coord  tp;

    for (rp = rooms; rp < &rooms[MAXROOMS]; rp++)
    {
        rp->r_nexits = 0;       /* no exits         */
        rp->r_flags = ISGONE;   /* kill all rooms   */
        rp->r_fires = 0;        /* no fires         */
    }

    rp = &rooms[0];         /* point to only room */
    rp->r_flags = ISDARK;   /* mazes always dark */
    rp->r_pos.x = 0;        /* room fills whole screen */
    rp->r_pos.y = 1;
    rp->r_max.x = COLS - 1;
    rp->r_max.y = LINES - 3;
    draw_maze();            /* put maze into window */

    /* add some gold to make it worth looking for */

    item = spec_item(GOLD, 0, 0, 0);
    obj  = OBJPTR(item);
    obj->o_count *= (rnd(10) + 5);  /* add in one large hunk */
	
	do
	{
	    rnd_pos(rp, &tp);
	}
	while( mvwinch(stdscr, tp.y, tp.x) != FLOOR);
	
    obj->o_pos = tp;
    add_obj(item, tp.y, tp.x);

    /* add in some food to make sure he has enough */

    item = spec_item(FOOD, 0, 0, 0);
    obj  = OBJPTR(item);
	
	do
	{
	    rnd_pos(rp, &tp);
	}
	while( mvwinch(stdscr, tp.y, tp.x) != FLOOR);
	
    obj->o_pos = tp;
    add_obj(item, tp.y, tp.x);

    if (rnd(100) < 5)  /* 5% for treasure maze level */
    {
        treas = TRUE;
        least = 20;
        debug("Treasure maze.");
    }
    else           /* normal maze level */
    {
        least = 1;
        treas = FALSE;
    }

    for (i = 0; i < level + least; i++)
    {
        if (!treas && rnd(100) < 50)    /* put in some little buggers */
            continue;

        /* Put the monster in */

        item = new_item(sizeof *mp);

        mp = THINGPTR(item);

        do
        {
            rnd_pos(rp, &tp);
        }
        while(mvwinch(stdscr, tp.y, tp.x) != FLOOR);

        new_monster(item, randmonster(NOWANDER, NOGRAB), &tp, NOMAXSTATS);

        /* See if we want to give it a treasure to carry around. */

        if (rnd(100) < monsters[mp->t_index].m_carry)
            attach(mp->t_pack, new_thing());

        /* If it carries gold, give it some */

        if (on(*mp, CARRYGOLD))
        {
            item = spec_item(GOLD, 0, 0, 0);
            obj = OBJPTR(item);
            obj->o_count = GOLDCALC + GOLDCALC + GOLDCALC;
            obj->o_pos = mp->t_pos;
            attach(mp->t_pack, item);
        }

    }
}

/*
    draw_maze()
        Generate and draw the maze on the screen
*/

void
draw_maze(void)
{
    int i, j, more;
    char    *ptr;

    urlines = (LINES - 3) / 2;
    urcols = (COLS - 1) / 2;

    bits     = ur_alloc((unsigned int) ((LINES - 3) * (COLS - 1)));
    frontier = ur_alloc((unsigned int) (urlines * urcols));
    ptr      = frontier;

    while (ptr < (frontier + (urlines * urcols)))
        *ptr++ = TRUE;

    for (i = 0; i < LINES - 3; i++)
    {
        for (j = 0; j < COLS - 1; j++)
        {
            if (i % 2 == 1 && j % 2 == 1)
                *moffset(i, j) = FALSE; /* floor */
            else
                *moffset(i, j) = TRUE;  /* wall */
        }
    }

    for (i = 0; i < urlines; i++)
    {
        for (j = 0; j < urcols; j++)
        {
            do
                more = findcells(i, j);
            while (more != 0);
        }
    }

    crankout();
    ur_free(frontier);
    ur_free(bits);
}

/*
    moffset()
        Calculate memory address for bits
*/

char *
moffset(int y, int x)
{
    return (bits + (y * (COLS - 1)) + x);
}

/*
    foffset()
        Calculate memory address for frontier
*/

char    *
foffset(int y, int x)
{
    return (frontier + (y * urcols) + x);
}

/*
    findcells()
        Figure out cells to open up
*/

int
findcells(int y, int x)
{
    int rtpos, i;

    struct
    {
       int    num_pos;        /* number of frontier cells next to you */

       struct
       {
           int y_pos;
           int x_pos;
       } conn[4];              /* the y,x position of above cell       */
    } mborder;

    *foffset(y, x) = FALSE;
    mborder.num_pos = 0;

    if (y < urlines - 1) {    /* look below */
        if (*foffset(y + 1, x))
        {
            mborder.conn[mborder.num_pos].y_pos = y + 1;
            mborder.conn[mborder.num_pos].x_pos = x;
            mborder.num_pos += 1;
        }
    }

    if (y > 0)         /* look above */
    {
        if (*foffset(y - 1, x))
        {
            mborder.conn[mborder.num_pos].y_pos = y - 1;
            mborder.conn[mborder.num_pos].x_pos = x;
            mborder.num_pos += 1;
        }
    }

    if (x < urcols - 1)  /* look right */
    {
        if (*foffset(y, x + 1))
        {
            mborder.conn[mborder.num_pos].y_pos = y;
            mborder.conn[mborder.num_pos].x_pos = x + 1;
            mborder.num_pos += 1;
        }
    }

    if (x > 0)        /* look left */
    {
        if (*foffset(y, x - 1))
        {
            mborder.conn[mborder.num_pos].y_pos = y;
            mborder.conn[mborder.num_pos].x_pos = x - 1;
            mborder.num_pos += 1;
        }
    }

    if (mborder.num_pos == 0)/* no neighbors available */
        return(0);
    else
    {
        i = rnd(mborder.num_pos);
        rtpos = mborder.num_pos - 1;
        rmwall(mborder.conn[i].y_pos, mborder.conn[i].x_pos, y, x);
        return(rtpos);
    }
}

/*
    rmwall()
        Removes appropriate walls from the maze
*/

void
rmwall(int newy, int newx, int oldy, int oldx)
{
    int xdif, ydif;

    xdif = newx - oldx;
    ydif = newy - oldy;

    *moffset((oldy * 2) + ydif + 1, (oldx * 2) + xdif + 1) = FALSE;

    findcells(newy, newx);
}


/*
    crankout()
        Does actual drawing of maze to window
*/

void
crankout(void)
{
    int x, y;

    for (y = 0; y < LINES - 3; y++)
    {
        move(y + 1, 0);

        for (x = 0; x < COLS - 1; x++)
        {
            if (*moffset(y, x))    /* here is a wall */
            {
                if (y == 0 || y == LINES - 4)       /* top or bottom line */
                    addch('-');
                else if (x == 0 || x == COLS - 2)   /* left | right side */
                    addch('|');
                else if (y % 2 == 0 && x % 2 == 0)
                {
                    if (*moffset(y, x - 1) || *moffset(y, x + 1))
                        addch('-');
                    else
                        addch('|');
                }
                else if (y % 2 == 0)
                    addch('-');
                else
                    addch('|');
            }
            else
                addch(FLOOR);
        }
    }
}
