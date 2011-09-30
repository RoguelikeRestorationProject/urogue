/*
    lint-curses.h

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/


/* Sufficient info to pass lint */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define getyx(win,y,x)   y = win->_cury, x = win->_curx
struct screen { int opque_data_type; };
typedef struct { int _cury; int _curx; } WINDOW;
extern WINDOW *stdscr;
extern WINDOW *curscr;
extern int LINES;
extern int COLS;
extern char *unctrl(char c);
extern void initscr(void);
extern int wmove(WINDOW *window, int Line, int Column);
extern int move(int Line, int Column);
extern int addch(char c);
extern int mvaddch(int y, int x, char c);
extern int waddch(WINDOW *window, char c);
extern int mvwaddch(WINDOW *window, int y, int x, char c);
extern int mvwinch(WINDOW *window, int y, int x);
extern int winch(WINDOW *window);
extern int mvinch(int y, int x);
extern int getch(void);
extern int wgetch(WINDOW *window);
extern void clear(void);
extern void wclear(WINDOW *window);
extern void refresh(void);
extern void wrefresh(WINDOW *window);
extern void clearok(WINDOW *window, int flag);
extern void endwin(void);
extern void touchwin(WINDOW *window);
extern void overlay(WINDOW *w1, WINDOW *w2);
extern void wclrtoeol(WINDOW *window);
extern void wprintw(WINDOW *window, const char *fmt, ...);
extern void mvprintw(int line, int col, char *fmt, ...);
extern void mvwprintw(WINDOW *window, int line, int col, char *fmt, ...);
extern int  mvwaddstr(WINDOW *window, int y, int x, const char *str);
extern int  mvaddstr(int y, int x, char *str);
extern int  waddstr(WINDOW *window, char *str);
extern int  addstr(char *str);
extern void standout(void);
extern void wstandout(WINDOW *window);
extern void standend(void);
extern void wstandend(WINDOW *window);
extern void noecho(void);
extern void cbreak(void);
extern void crmode(void);
extern void nonl(void);
extern void nl(void);
extern int wgetch(WINDOW *window);
extern WINDOW *newwin(int lines, int cols, int y, int x);
extern void overwrite(WINDOW *w1, WINDOW *w2);
extern void delwin(WINDOW *window);
extern void printw(char *fmt, ...);

