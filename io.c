/*
    io.c - Various input/output functions
  
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
#include <ctype.h>
#include <stdarg.h>
#include "rogue.h"

char prbuf[2 * LINELEN];    /* Buffer for sprintfs                      */
static char mbuf[2*LINELEN];  /* Current message buffer        */
static int newpos = 0;       /* index in mbuf to end of msg   */

int mpos = 0;                 /* 0  = Overwrite existing message */
                              /* >0 = print --More-- at this pos */
                              /*      and wait for key           */

int line_cnt = 0;
int newpage  = FALSE;

/*
    msg()
        Display a message at the top of the screen.
*/

void
msg(const char *fmt, ...)
{
    va_list ap;

    /* if the string is "", just clear the line */

    if (*fmt == '\0')
    {
        wmove(cw, 0, 0);
        wclrtoeol(cw);
        mpos = 0;
        return;
    }

    /* otherwise add to the message and flush it out */

    va_start(ap, fmt);
    doadd(fmt, ap);
    va_end(ap);

    endmsg();
}

void
vmsg(const char *fmt, va_list ap)
{
    /* if the string is "", just clear the line */

    if (*fmt == '\0')
    {
        wmove(cw, 0, 0);
        wclrtoeol(cw);
        mpos = 0;
        return;
    }

    /* otherwise add to the message and flush it out */

    doadd(fmt, ap);
    endmsg();
}


/*
    addmsg()
        add things to the current message
*/

void
addmsg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    doadd(fmt,ap);
    va_end(ap);
}

/*
    endmsg()
        Display a new msg (giving him a chance to see the previous one
        if it is up there with the --More--)
*/

void
endmsg(void)
{
    strcpy(msgbuf[msg_index], mbuf);

    msg_index = ++msg_index % 10;

    if (mpos)
    {
        wmove(cw, 0, mpos);
        wprintw(cw, (char *) morestr);
        wrefresh(cw);
        wait_for(' ');
    }

    mvwprintw(cw, 0, 0, mbuf);
    wclrtoeol(cw);

    mpos = newpos;
    newpos = 0;

    wrefresh(cw);
}

void
doadd(const char *fmt, va_list ap)
{
    vsprintf(&mbuf[newpos], fmt, ap);
    newpos = (int) strlen(mbuf);
}

/*
    status()
        Display the important stats line.  Keep the cursor where it was.
*/

void
status(int display)
{
    static char buf[1024];             /* Temporary buffer */
    struct stats *stat_ptr, *max_ptr;
    int oy, ox;

    stat_ptr = &pstats;
    max_ptr = &max_stats;

    getyx(cw, oy, ox);
    sprintf(buf,
    "Int:%d(%d) Str:%d(%d) Wis:%d(%d) Dex:%d(%d) Con:%d(%d) Carry:%d(%d) %d",
        stat_ptr->s_intel, max_ptr->s_intel,
        stat_ptr->s_str, max_ptr->s_str,
        stat_ptr->s_wisdom, max_ptr->s_wisdom,
        stat_ptr->s_dext, max_ptr->s_dext,
        stat_ptr->s_const, max_ptr->s_const,
        stat_ptr->s_pack / 10, stat_ptr->s_carry / 10, foodlev );

    mvwaddstr(cw, LINES - 2, 0, buf);
    wclrtoeol(cw);

    sprintf(buf, "Lvl:%d Au:%d Hpt:%3d(%3d) Pow:%d(%d) Ac:%d  Exp:%d+%ld  %s",
        level,
        purse,
        stat_ptr->s_hpt, max_ptr->s_hpt,
        stat_ptr->s_power, max_ptr->s_power,
        (cur_armor != NULL ? (cur_armor->o_ac - 10 + stat_ptr->s_arm)
         : stat_ptr->s_arm) - ring_value(R_PROTECT),
        stat_ptr->s_lvl,
        stat_ptr->s_exp,
        cnames[player.t_ctype][min(stat_ptr->s_lvl - 1, 14)]);

    mvwaddstr(cw, LINES - 1, 0, buf);

    switch(hungry_state)
    {
        case F_OK:     break;
        case F_HUNGRY: waddstr(cw, " Hungry");
                       break;
        case F_WEAK:   waddstr(cw, " Weak");
                       break;
        case F_FAINT:  waddstr(cw, " Fainting");
                       break;
    }

    wclrtoeol(cw);
    wmove(cw, oy, ox);

    if (display)
        wrefresh(cw);
}

/*
 * readchar:
 *	Flushes stdout so that screen is up to date and then returns
 *	getchar().
 */

char
readcharw(WINDOW *win)
{
    char ch;

    ch = (char) md_readchar(win);

    if ((ch == 3) || (ch == 0))
    {
	quit();
	return(27);
    }

    return(ch);
}

char
readchar()
{
    return( readcharw(cw) );
}

/*
    wait_for()
        Sit around until the guy types the right key
*/

void
w_wait_for(WINDOW *w, int ch)
{
    char    c;

    if (ch == '\n')
        while ((c = readcharw(w)) != '\n' && c != '\r')
            continue;
    else
        while (readcharw(w) != ch)
            continue;
}

void
wait_for(int ch)
{
    w_wait_for(cw, ch);
}

/*
    show_win()
        function used to display a window and wait before returning
*/

void
show_win(WINDOW *scr, char *message)
{
    mvwaddstr(scr, 0, 0, message);
    touchwin(scr);
    wmove(scr, hero.y, hero.x);
    wrefresh(scr);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
    restscr()
        Restores the screen to the terminal
*/

void
restscr(WINDOW *scr)
{
    clearok(scr, TRUE);
    touchwin(scr);
}

/*
    add_line()
        Add a line to the list of discoveries
*/

void
add_line(const char *fmt, ...)
{
    WINDOW  *tw;
    va_list ap;

    va_start(ap, fmt);

    if (line_cnt == 0)
    {
        wclear(hw);

        if (inv_type == INV_SLOW)
            mpos = 0;
    }

    if (inv_type == INV_SLOW)
    {
        if ( (fmt != NULL) && (*fmt != '\0') )
            vmsg(fmt, ap);
        line_cnt++;
    }
    else
    {
        if ( (line_cnt >= LINES - 2) || (fmt == NULL)) /* end 'o page */
        {
            if (fmt == NULL && !newpage && inv_type == INV_OVER)
            {
                tw = newwin(line_cnt + 2, COLS, 0, 0);
                overwrite(hw, tw);
                wstandout(tw);
                mvwaddstr(tw, line_cnt, 0, spacemsg);
                wstandend(tw);
                touchwin(tw);
                wrefresh(tw); 
                wait_for(' ');
                delwin(tw);
                touchwin(cw);
            }
            else
            {
                wstandout(hw);
                mvwaddstr(hw, LINES - 1, 0, spacemsg);
                wstandend(hw);
                wrefresh(hw);
                w_wait_for(hw, ' ');
                touchwin(cw);
                wclear(hw);
            }
            newpage = TRUE;
            line_cnt = 0;
        }

        /* draw line */
        if (fmt != NULL && !(line_cnt == 0 && *fmt == '\0'))
        {
            static char tmpbuf[1024];
			
            vsprintf(tmpbuf, fmt, ap);
            mvwprintw(hw, line_cnt++, 0, tmpbuf);
        }
    }
}

/*
    end_line()
        End the list of lines
*/

void
end_line(void)
{
    if (inv_type != INV_SLOW)
        add_line(NULL);

    line_cnt = 0;
    newpage = FALSE;
}

/*
    hearmsg()
        Call msg() only if you are not deaf
*/

void
hearmsg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    if (off(player, ISDEAF))
        vmsg(fmt, ap);
    else if (wizard)
    {
        msg("Couldn't hear: ");
        vmsg(fmt, ap);
    }

    va_end(ap);
}

/*
    seemsg()
        Call msg() only if you are not blind
*/

void
seemsg(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    if (off(player, ISBLIND))
        vmsg(fmt, ap);
    else if (wizard)
    {
        msg("Couldn't see: ");
        vmsg(fmt, ap);
    }

    va_end(ap);
}

int
get_string(char *buffer, WINDOW *win)
{
    char    *sp, c;
    int oy, ox;
    char    buf[2 * LINELEN];

    wrefresh(win);
    getyx(win, oy, ox);

    /* loop reading in the string, and put it in a temporary buffer */

    for (sp = buf; (c = readcharw(win)) != '\n' &&
         c != '\r' &&
         c != '\033' &&
         c != '\007' &&
         sp < &buf[LINELEN - 1];
         wclrtoeol(win), wrefresh(win))
    {
	if ((c == '\b') || (c == 0x7f))
        {
            if (sp > buf)
            {
                size_t i;

                sp--;

                for (i = strlen(unctrl(*sp)); i; i--)
                    waddch(win, '\b');
            }
            continue;
        }
        else if (c == '\0')
        {
            sp = buf;
            wmove(win, oy, ox);
            continue;
        }
        else if (sp == buf && c == '-' && win == hw)
            break;

        *sp++ = c;
        waddstr(win, unctrl(c));
    }

    *sp = '\0';

    if (sp > buf)       /* only change option if something has been typed */
        strncpy(buffer, buf, strlen(buf)+1);

    wmove(win, oy, ox);
    waddstr(win, buffer);
    waddch(win, '\n');
    wrefresh(win);

    if (win == cw)
        mpos += (int)(sp - buf);

    if (c == '-')
        return(MINUS);
    else if (c == '\033' || c == '\007')
        return(QUIT);
    else
        return(NORM);
}

