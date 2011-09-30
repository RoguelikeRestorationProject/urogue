/*
    list.c - Functions for dealing with linked lists of goodies
  
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rogue.h"

static char errbuf[2 * LINELEN];

/*
    ur_alloc()
    ur_free()

    These are just calls to the system alloc and free, and they also adjust
    the totals. The buffer is cleared out because idents need to be zero
    before going into the pack, or they will be used as indices!
*/

void *
ur_alloc(size_t size)
{
    char *buf_p;

    total++;

    buf_p = mem_malloc(size);

    if (buf_p == NULL)
        return(NULL);

    memset(buf_p,0,size);

    return(buf_p);
}

void
ur_free(void *buf_p)
{
    mem_free(buf_p);
    total--;
}

/*
    detach()
        Takes an item out of whatever linked list it might be in
        .... function needs to be renamed....
*/

void
_detach(struct linked_list **list, struct linked_list *item)
{
    if (*list == item)
        *list = next(item);

    if (prev(item) != NULL)
        item->l_prev->l_next = next(item);

    if (next(item) != NULL)
        item->l_next->l_prev = prev(item);

    item->l_next = NULL;
    item->l_prev = NULL;
}

/*
    _attach()
        add an item to the head of a list
        ... this needs to be renamed as well ...
*/

void
_attach(struct linked_list  **list, struct linked_list  *item)
{
    if (*list != NULL)
    {
        item->l_next = *list;
        (*list)->l_prev = item;
        item->l_prev = NULL;
    }
    else
    {
        item->l_next = NULL;
        item->l_prev = NULL;
    }

    *list = item;
}

/*
    _attach_after()

    Attaches the given item after the supplied one in the list. If the listed
    item is NULL, the new item is attached at the head of the list.
*/

void
_attach_after(linked_list **list_pp, linked_list *list_p, linked_list *new_p)
{
    if (list_p == NULL)
    {
        _attach(list_pp, new_p);    /* stuff it at the beginning */
        return;
    }

    if (next(list_p) != NULL)  /* something after this one? */
    {
        new_p->l_next = next(list_p);
        list_p->l_next->l_prev = new_p;
    }
    else
        new_p->l_next = NULL;

    list_p->l_next = new_p;
    new_p->l_prev = list_p;
}

/*
    _free_list()
        Throw the whole blamed thing away
*/

void
_free_list(linked_list **ptr)
{
    linked_list *item;

    while(*ptr != NULL)
    {
        item = *ptr;
        *ptr = next(item);
        discard(item);
    }
}

/*
    discard()
        free up an item
*/

void
discard(struct linked_list *item)
{
    throw_away(item->data.obj);
    ur_free(item);
}

/*
    throw_away()
        toss out something (like discard, but without the link_list)
*/

void
throw_away(struct object *ptr)
{
    free_ident(ptr);
    ur_free(ptr);
}

/*
    new_item()
        get a new item with a specified size
*/

struct linked_list *
new_item(int size)
{
    struct linked_list  *item;

    if ((item = new_list()) == NULL)
        msg("Ran out of memory for header after %d items.", total);

    if ((item->data.l_data = new_alloc(size)) == NULL)
        msg("Ran out of memory for data after %d items.", total);

    item->l_next = item->l_prev = NULL;

    return(item);
}

void *
new_alloc(size_t size)
{
    void *space = ur_alloc(size);

    if (space == NULL)
    {
        sprintf(errbuf, "Rogue ran out of memory.");
        fatal(errbuf);
    }

    return(space);
}

struct linked_list *
new_list(void)
{
    union /* ugly_lint_hack */
    {
        struct linked_list *ll;
        void              *vptr;
    } newp;

    newp.vptr = mem_malloc(sizeof(struct linked_list));
    memset(newp.vptr,0,sizeof(struct linked_list));
    return(newp.ll);
}
