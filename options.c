/*
    options.c - This file has all the code for the option command
        
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
#include "rogue.h"

#define NUM_OPTS    (sizeof optlist / sizeof (OPTION))
#define EQSTR(a, b, c)  (strncmp(a, b, c) == 0)

/* description of an option and what to do with it */
static OPTION optlist[] =
{
{"jump","Show position only at end of run (jump): ", &jump,put_bool,get_bool},
{"inven","Style of inventories (inven): ", &inv_type, put_inv, get_inv},
{"askme","Ask me about unidentified things (askme): ",&askme,put_bool,get_bool},
{"doorstop","Stop running when adjacent (doorstop): ",&doorstop,put_bool,get_bool},
{"name", "Name (name): ",               &whoami, put_str, get_str},
{"fruit", "Fruit (fruit): ",            &fruit, put_str, get_str},
{"file", "Save file (file): ",          &file_name, put_str, get_str},
{"score", "Score file (score): ",       &score_file, put_str, get_str},
{"class", "Character class (class): ",&char_type, put_abil, get_abil}
};

/*
    option()
        print and then set options from the terminal
*/

void
option(void)
{
    OPTION  *op;
    int retval;

    wclear(hw);
    touchwin(hw);

    /* Display current values of options */

    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
        waddstr(hw, op->o_prompt);
        (*op->o_putfunc)(&op->o_opt, hw);
        waddch(hw, '\n');
    }

    /* Set values */

    wmove(hw, 0, 0);

    for (op = optlist; op < &optlist[NUM_OPTS]; op++)
    {
        waddstr(hw, op->o_prompt);

	retval = (*op->o_getfunc)(&op->o_opt, hw);

        if (retval)
            if (retval == QUIT)
                break;
            else if (op > optlist)      /* MINUS */
            {
                wmove(hw, (int)(op - optlist) - 1, 0);
                op -= 2;
            }
            else    /* trying to back up beyond the top */
            {
                putchar('\007');
                wmove(hw, 0, 0);
                op--;
            }
    }

    /* Switch back to original screen */

    mvwaddstr(hw, LINES - 1, 0, spacemsg);
    wrefresh(hw);
    wait_for(' ');
    clearok(cw, TRUE);
    touchwin(cw);
}

/*
    put_bool()
        put out a boolean
*/

void
put_bool(opt_arg *opt, WINDOW *win)
{
    waddstr(win, *opt->iarg ? "True" : "False");
}

/*
    put_str()
        put out a string
*/

void
put_str(opt_arg *opt, WINDOW *win)
{
    waddstr(win, opt->str);
}

/*
    put_abil()
        print the character type
*/

void
put_abil(opt_arg *opt, WINDOW *win)
{
    char    *abil;

    switch(*opt->iarg)
    {
        case C_FIGHTER:
            abil = "Fighter";
            break;
        case C_MAGICIAN:
            abil = "Magic User";
            break;
        case C_CLERIC:
            abil = "Cleric";
            break;
        case C_THIEF:
            abil = "Thief";
            break;
        case C_PALADIN:
            abil = "Paladin";
            break;
        case C_RANGER:
            abil = "Ranger";
            break;
        case C_ILLUSION:
            abil = "Illusionist";
            break;
        case C_ASSASIN:
            abil = "Assasin";
            break;
        case C_NINJA:
            abil = "Ninja";
            break;
        case C_DRUID:
            abil = "Druid";
            break;
        default:
            abil = "(unknown)";
    }
    waddstr(win, abil);
}


/*
    get_bool()
        allow changing a boolean option and print it out
*/

int
get_bool(opt_arg *opt, WINDOW *win)
{
    int oy, ox;
    int op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    waddstr(win, *opt->iarg ? "True" : "False");

    while(op_bad)
    {
        wmove(win, oy, ox);
        wrefresh(win);

        switch (readcharw(win))
        {
            case 't':
            case 'T':
                *opt->iarg = TRUE;
                op_bad = FALSE;
                break;

            case 'f':
            case 'F':
                *opt->iarg = FALSE;
                op_bad = FALSE;
                break;

            case '\n':
            case '\r':
                op_bad = FALSE;
                break;

            case '\033':
            case '\007':
                return QUIT;

            case '-':
                return MINUS;

            default:
                mvwaddstr(win, oy, ox + 10, "(T or F)");
        }
    }

    wmove(win, oy, ox);
    wclrtoeol(win);
    waddstr(win, *opt->iarg ? "True" : "False");
    waddch(win, '\n');

    return(NORM);
}

/*
    get_str()
        set a string option
*/

int
get_str(opt_arg *opt, WINDOW *win)
{
    return( get_string(opt->str, win) );
}

/*
    get_abil()
        The ability field is read-only
*/

int
get_abil(opt_arg *opt, WINDOW *win)
{
    int oy, ox, ny, nx;
    int op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    put_abil(opt, win);
    getyx(win, ny, nx);

    while(op_bad)
    {
        wmove(win, oy, ox);
        wrefresh(win);

        switch(readcharw(win))
        {
            case '\n':
            case '\r':
                op_bad = FALSE;
                break;

            case '\033':
            case '\007':
                return(QUIT);

            case '-':
                return(MINUS);

            default:
                mvwaddstr(win, ny, nx + 5, "(no change allowed)");
        }
    }

    wmove(win, ny, nx + 5);
    wclrtoeol(win);
    wmove(win, ny, nx);
    waddch(win, '\n');

    return(NORM);
}


/*
    parse_opts()
        parse options from string, usually taken from the environment. the
        string is a series of comma seperated values, with booleans being
        stated as "name" (true) or "noname" (false), and strings being
        "name=....", with the string being defined up to a comma or the
        end of the entire option string.
 */

void
parse_opts(char *str)
{
    char    *sp;
    const OPTION  *op;
    size_t len;

    while (*str)
    {
        for (sp = str; isalpha(*sp); sp++)
            continue;

        len = sp - str;

        /* Look it up and deal with it */

        for (op = optlist; op < &optlist[NUM_OPTS]; op++)
            if (EQSTR(str, op->o_name, len))
            {
                if (op->o_putfunc == put_bool)
                    *op->o_opt.iarg = TRUE;
                else    /* string option */
                {
                    char    *start;
                    char    value[80];

                    /* Skip to start of string value */

                    for (str = sp + 1; *str == '='; str++)
                        continue;

                    start = (char *) value;

                    /* Skip to end of string value */

                    for (sp = str + 1; *sp && *sp != ','; sp++)
                        continue;

                    strncpy(start, str, sp - str);

                    /* Put the value into the option field */

                    if (op->o_putfunc != put_abil &&
                        op->o_putfunc != put_inv)
                        strcpy(op->o_opt.str, value);

                    if (op->o_putfunc == put_inv)
                    {
                        int *opt = op->o_opt.iarg;

                        len = strlen(value);

                        if (isupper(value[0]))
                            value[0] = (char) tolower(value[0]);
                        if (EQSTR(value, "overwrite",len))
                            *opt = INV_OVER;
                        if (EQSTR(value, "slow", len))
                            *opt = INV_SLOW;
                        if (EQSTR(value, "clear", len))
                            *opt = INV_CLEAR;
                    }
                    else if (*op->o_opt.iarg == -1)
                    {
                        int *opt = op->o_opt.iarg;

                        len = strlen(value);

                        if (isupper(value[0]))
                            value[0] = (char) tolower(value[0]);
                        if (EQSTR(value, "fighter", len))
                            *opt = C_FIGHTER;
                        else if (EQSTR(value, "magic", min(len, 5)))
                            *opt = C_MAGICIAN;
                        else if (EQSTR(value, "illus", min(len, 5)))
                            *opt = C_ILLUSION;
                        else if (EQSTR(value, "cleric", len))
                            *opt = C_CLERIC;
                        else if (EQSTR(value, "thief",  len))
                            *opt = C_THIEF;
                        else if (EQSTR(value, "paladin", len))
                            *opt = C_PALADIN;
                        else if (EQSTR(value, "ranger",  len))
                            *opt = C_RANGER;
                        else if (EQSTR(value, "assasin", len))
                            *opt = C_ASSASIN;
                        else if (EQSTR(value, "druid",   len))
                            *opt = C_DRUID;
                        else if (EQSTR(value, "ninja",   len))
                            *opt = C_NINJA;
                    }
                }
                break;
            }
            else if (op->o_putfunc == put_bool
                 && EQSTR(str, "no", 2) &&
                EQSTR(str + 2, op->o_name, len - 2))
            {
                *op->o_opt.iarg = FALSE;
                break;
            }

        /* skip to start of next option name  */

        while (*sp && !isalpha(*sp))
            sp++;

        str = sp;
    }
}

/*
    put_inv()
        print the inventory type
*/

void
put_inv(opt_arg *opt, WINDOW *win)
{
    char    *style;

    switch(*opt->iarg)
    {
        case INV_OVER:
            style = "Overwrite";
            break;

        case INV_SLOW:
            style = "Slow";
            break;

        case INV_CLEAR:
            style = "Clear Screen";
            break;

        default:
            style = "(unknown)";
    }

    waddstr(win, style);
}

/*
    get_inv()
        The inventory field.
*/

int
get_inv(opt_arg *opt, WINDOW *win)
{
    int oy, ox, ny, nx;
    int op_bad;

    op_bad = TRUE;
    getyx(win, oy, ox);
    put_inv(opt, win);
    getyx(win, ny, nx);

    while(op_bad)
    {
        wmove(win, oy, ox);
        wrefresh(win);

        switch(readcharw(win))
        {
            case '\n':
            case '\r':
                op_bad = FALSE;
                break;

            case '\033':
            case '\007':
                return(QUIT);

            case '-':
                return(MINUS);

            case 'O':
            case 'o':
                *opt->iarg = INV_OVER;
                op_bad = FALSE;
                break;

            case 'S':
            case 's':
                *opt->iarg = INV_SLOW;
                op_bad = FALSE;
                break;

            case 'C':
            case 'c':
                *opt->iarg = INV_CLEAR;
                op_bad = FALSE;
                break;

            default:
                mvwaddstr(win, ny, nx + 5, "(Use: o, s, or c)");
        }
    }

    wmove(win, oy, ox);
    wclrtoeol(win);

    switch(*opt->iarg)
    {
        case INV_SLOW:
            waddstr(win, "Slow\n");
            break;

        case INV_CLEAR:
            waddstr(win, "Clear Screen\n");
            break;

        case INV_OVER:
            waddstr(win, "Overwrite\n");
            break;

        default:
            waddstr(win, "Unknown\n");
            break;
    }

    return(NORM);
}
