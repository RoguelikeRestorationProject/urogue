/*
    bag.c  -  functions for dealing with bags

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*
 * new bag functions
 *
 * This is a simple version of bag.c that uses linked lists to perform the bag
 * functions. The bag is just a linked list of objects (struct object) to be
 * specific, but most of that is supposed to be hidden from the user, who
 * should access the bag only through the functions presented here.
 */

#include <stdlib.h>
#include "rogue.h"

/*
 * apply_to_bag
 *
 * This is the general bag manipulation routine. The bag is subjected to
 * selection criteria and those objects which pass are processed by an action
 * routine. The two criteria are type and filter function. The filter
 * function returns TRUE if the object passes and FALSE otherwise. The filter
 * function is passed the object and the user-supplied argument. This gives
 * the user plenty of flexibility in determining which items will be
 * processed. The action routine is passed the object, the id, and the
 * user-supplied argument given to apply_to_bag. Specifying NULL for either
 * the type or filter function means that criterion always selects. A NULL
 * action routine means no processing is done and the first object which
 * passes the filter is returned to the user. The action routine returns TRUE
 * if processing should continue or FALSE if the current item should be
 * returned to the caller.
 *
 * Returns NULL if the bag is empty or if nothing qualified.
 *
 * linked_list *bag_p;       // linked list of objects
 * int          type;        // what is its type (ARMOR, ...)
 * int        (*bff_p)();    // bag filter function
 * int        (*baf_p)();    // bag action routine
 * long         user_arg;    // user argument for filter, action
 *
 */

struct object *
apply_to_bag(struct linked_list *bag_p,
             int type,
             int (*bff_p)(struct object *obj, bag_arg *user_arg),
             int (*baf_p)(struct object *obj, bag_arg *user_arg, int id),
             void *user_arg)
{
    struct object *bag_obj_p = NULL;  /* qualifying object */
    struct object *cur_obj_p;         /* current object */
    bag_arg arg;

    arg.varg = user_arg;

    if (bag_p == NULL)
        return (NULL);

    for (; bag_p != NULL; bag_p = next(bag_p))
    {
        cur_obj_p = OBJPTR(bag_p);

        if (type != 0 && type != cur_obj_p->o_type)
            continue;

        if (bff_p != NULL && !(*bff_p)(cur_obj_p, &arg))
            continue;

        /*
         * At this point, we have an object which qualifies for
         * processing
         */

        bag_obj_p = cur_obj_p;  /* in case the user wants it */

        if (baf_p != NULL && (*baf_p)(cur_obj_p, &arg, identifier(bag_obj_p)))
            continue;

        /*
         * We have an object which qualifies, quit now!
         */

        break;
    }

    if (bag_p == NULL)
        return (NULL);

    return (bag_obj_p);
}

/*
    count_bag()

        Counts up all bag items which meet the selection criteria
*/

int
count_bag(linked_list *bag_p,
          int type,
          int (*bff_p)(struct object *obj, bag_arg *junk))
{
    int cnt = 0;
    apply_to_bag(bag_p, type, bff_p, baf_increment, &cnt);

    return(cnt);
}

/*
    del_bag()

        Removes an object from a bag and throws it away.
*/

void
del_bag(linked_list *bag_p, object *obj_p)
{
    pop_bag(&bag_p, obj_p);  /* get the thing from the bag */
    ur_free(obj_p);         /* release the memory */
}

/*
    pop_bag()

        Removes an item from a bag and returns it to the user. If the item is
        not in the bag, return NULL.
*/

struct object *
pop_bag(linked_list **bag_pp, object *obj_p)
{
    linked_list *item_p;

    for (item_p = *bag_pp; item_p != NULL && OBJPTR(item_p) != obj_p;
         item_p = next(item_p));

    if (item_p == NULL)
        return (NULL);

    _detach(bag_pp, item_p);

    return (obj_p);
}

/*
    push_bag()

        stuff another item into the bag
*/

void
push_bag(linked_list **bag_pp, object *obj_p)
{
    struct linked_list *item_p = NULL;
    struct linked_list *new_p  = NULL;
    struct linked_list *best_p = NULL;

    new_p             = new_list();
    new_p->data.obj   = obj_p;              /* attach our object   */
    identifier(obj_p) = get_ident(obj_p);   /* tag this object for */
                                            /*     inventory       */
    /*
     * Find a place in the bag - try to match the type, then sort by
     * identifier
     */

    for (item_p = *bag_pp; item_p != NULL; item_p = next(item_p))
    {
        if ((OBJPTR(item_p))->o_type == obj_p->o_type)
        {
            if (best_p == NULL)
                best_p = item_p;
            else if (identifier((OBJPTR(item_p))) >
                     identifier((OBJPTR(best_p))) &&
                     identifier((OBJPTR(item_p))) <
                     identifier(obj_p))
                best_p = item_p;
        }
    }

    _attach_after(bag_pp, best_p, new_p);   /* stuff it in the list */

    return;
}

/*
    scan_bag()

        Gets the object from the bag that matches the type and id. The object
        is not removed from the bag.
*/

struct object *
scan_bag(linked_list *bag_p, int type, int id)
{
    object  *obj_p = NULL;

    for (; bag_p != NULL; bag_p = next(bag_p))
    {
        obj_p = OBJPTR(bag_p);

        if (obj_p->o_type == type && identifier(obj_p) == id)
            break;
    }

    if (bag_p == NULL)
        return(NULL);

    return(obj_p);
}

/*
    baf_decrement_test()

        Assumes the argument is a pointer to int and it just decrements it.
        Returns TRUE, except when the count goes to zero.
*/

int
baf_decrement_test(struct object *obj_p, bag_arg *count_p, int id)
{
    NOOP(obj_p);
    NOOP(id);

    if (*count_p->iarg > 0)
        return(TRUE);

    return(FALSE);
}

/*
    baf_identify()

        Bag action function to identify an object. This is needed to conform
        to bag action routine calling conventions and to put the linked list
        structure on top of the object before calling whatis()
*/

int
baf_identify(struct object *obj_p, bag_arg *junk, int id)
{
    linked_list l;
    linked_list *lp = &l;

    NOOP(junk);
    NOOP(id);

    lp->data.obj = obj_p;    /* stuff object in the right place */
    whatis(lp);

    return(TRUE);
}

/*
    baf_increment()

        Assumes the argument is a pointer to int and it just increments it and
        returns TRUE
*/

int
baf_increment(object *obj_p, bag_arg *count_p, int id)
{
    NOOP(obj_p);
    NOOP(id);

    (*count_p->iarg)++;

    return(TRUE);
}

/*
    baf_print_item()
        Bag action function to print a single item, inventory style.
*/

int
baf_print_item(struct object *obj_p, bag_arg *type, int id)
{
    char inv_temp[3 * LINELEN];  /* plenty of space for paranoid programmers */

    if (*type->iarg == 0)
        sprintf(inv_temp, "%c%c) %s", obj_p->o_type,
              print_letters[id], inv_name(obj_p, LOWERCASE), FALSE);
    else
        sprintf(inv_temp, "%c) %s", print_letters[id],
            inv_name(obj_p, LOWERCASE), FALSE);

    add_line(inv_temp);
    return(TRUE);
}

/*
    bff_group()
        This bag filter function checks to see if two items can be combined by
        adjusting the count. Grouped items can be combined if the group numbers
        match. The only other item that is allowed to have a count is food, and
        there an exact match is required.
*/

int
bff_group(struct object *obj_p, bag_arg *arg)
{
    struct object *new_obj_p = arg->obj;

    if (new_obj_p->o_group > 0 && new_obj_p->o_group == obj_p->o_group)
        return(TRUE);

    if (new_obj_p->o_type == FOOD &&
        obj_p->o_type == new_obj_p->o_type &&
        obj_p->o_which == new_obj_p->o_which)
        return(TRUE);

    return(FALSE);
}

/*
    bff_callable
        Figures out which items can be callable: current rules are:
            potions, scrolls, staffs, and rings.
*/

int
bff_callable(struct object *obj_p, bag_arg *junk)
{
    NOOP(junk);

    if (obj_p->o_type == POTION || obj_p->o_type == RING ||
        obj_p->o_type == STICK || obj_p->o_type == SCROLL)
        return(TRUE);

    return(FALSE);
}

/*
    bff_markable()
        Selects which items can be marked. Current rules exclude only gold.
*/

int
bff_markable(struct object *obj_p, bag_arg *junk)
{
    NOOP(junk);

    if (obj_p->o_type == GOLD)
        return(FALSE);

    return(TRUE);
}

/*
    bffron()
        returns TRUE if hero is wearing this ring
*/

int
bffron(object *obj_p, bag_arg *junk)
{
    NOOP(junk);

    return(cur_ring[LEFT_1] == obj_p || cur_ring[LEFT_2] == obj_p ||
        cur_ring[LEFT_3] == obj_p || cur_ring[LEFT_4] == obj_p ||
        cur_ring[LEFT_5] ||
        cur_ring[RIGHT_1] == obj_p || cur_ring[RIGHT_2] == obj_p ||
        cur_ring[RIGHT_3] == obj_p || cur_ring[RIGHT_4] == obj_p ||
        cur_ring[RIGHT_5]);
}

/*
    bff_zappable()
        Selects which items can be zapped. This includes both sticks and
        magically enhanced weapons with lightning ability.
*/

int
bff_zappable(struct object *obj_p, bag_arg *junk)
{
    NOOP(junk);

    if (obj_p->o_type == STICK)
        return(TRUE);

    if (obj_p->o_type == WEAPON && obj_p->o_flags & ISZAPPED)
        return(TRUE);

    return (FALSE);
}

/*
    baf_curse()
        Curse all non-artifact items in the player's pack
*/

int
baf_curse(struct object *obj_p, bag_arg *junk, int id)
{
    NOOP(junk);
    NOOP(id);

    if (obj_p->o_type != ARTIFACT && rnd(8) == 0)
    {
        obj_p->o_flags |= ISCURSED;
        obj_p->o_flags &= ~ISBLESSED;
    }

    return(TRUE);
}

/*
    bafcweapon()
        bag action routine to fetch the current weapon
*/

int
bafcweapon(struct object *obj_p, bag_arg *junk, int id)
{
    NOOP(junk);
    NOOP(id);

    if (obj_p == cur_weapon)
        return(FALSE); /* found what we wanted - stop and return it */

    return(TRUE);
}

/*
    bafcarmor()
        bag action routine to fetch the current armor
*/

int
bafcarmor(struct object *obj_p, bag_arg *junk, int id)
{
    NOOP(junk);
    NOOP(id);

    if (obj_p == cur_armor)
        return(FALSE); /* found what we wanted - stop and return it */

    return(TRUE);
}
