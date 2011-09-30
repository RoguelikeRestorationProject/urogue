/*
    encumb.c - Stuff to do with encumberance
 
    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1985, 1986, 1992, 1993, 1995 Herb Chong
    All rights reserved.

    Based on "Advanced Rogue"
    Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include "rogue.h"

/*
    updpack()
        Update his pack weight and adjust fooduse accordingly
*/

void
updpack(void)
{
    int curcarry = packweight();

    pstats.s_carry = totalenc();    /* update max encumb */

    if (is_carrying(TR_PURSE))
        pstats.s_carry += 1000;

    foodlev = 0;

    switch ((curcarry * 5) / pstats.s_carry)   /* % of total capacity */
    {
        case 5:     /* 100 % */
            foodlev++;

        case 4:     /* 80 % */
            if (rnd(100) < 80)
                foodlev++;

        case 3:     /* 60 % */
            if (rnd(100) < 60)
                foodlev++;

        case 2:     /* 40 % */
            if (rnd(100) < 40)
                foodlev++;

        case 1:     /* 20 % */
            if (rnd(100) < 20)
                foodlev++;

        case 0:     /* 0 % */
            foodlev++;
    }

    pstats.s_pack = curcarry;   /* update pack weight */

    if (is_carrying(TR_PURSE))  /* makes pack lighter */
        foodlev--;
}


/*
    packweight()
        Get the total weight of the hero's pack
*/

int
packweight(void)
{
    struct linked_list  *pc;
    int weight = 0;

    for (pc = pack; pc != NULL; pc = next(pc))
    {
        struct object   *obj = OBJPTR(pc);

        weight += itemweight(obj) * obj->o_count;
    }

    if (weight < 0)     /* caused by artifacts or blessed items */
        weight = 0;

    return (weight);
}


/*
    itemweight()
        Get the weight of an object
*/

int
itemweight(struct object *wh)
{
    int weight = wh->o_weight;  /* get base weight */
    int ac;

    switch (wh->o_type)
    {
        case ARMOR:  /* 10% for each plus or minus*/
            ac = armors[wh->o_which].a_class - wh->o_ac;
            weight *= (10 - ac) / 10;
            break;

        case WEAPON:
            if ((wh->o_hplus + wh->o_dplus) > 0)
                weight /= 2;
    }

    if (wh->o_flags & ISCURSED)
        weight += weight / 2;   /* +50% for cursed */
    else if (wh->o_flags & ISBLESSED)
        weight -= weight / 5;   /* -20% for blessed */

    if (weight < 0)
        weight = 0;

    return (weight);
}


/*
    playenc()
        Get hero's carrying ability above norm 50 units per point of STR
        over 10, 300 units per plus on R_CARRYING 1000 units for TR_PURSE
*/

int
playenc(void)
{
    int ret_val = (pstats.s_str - 10) * 50;

    if (is_wearing(R_CARRYING))
        ret_val += ring_value(R_CARRYING) * 300;

    return (ret_val);
}


/*
    totalenc()
        Get total weight that the hero can carry
*/

int
totalenc(void)
{
    int wtotal = 1400 + playenc();

    switch (hungry_state)
    {
        case F_OK:
        case F_HUNGRY: /* no change */
            break;

        case F_WEAK:
            wtotal -= wtotal / 4;  /* 25% off weak */
            break;

        case F_FAINT:
            wtotal /= 2;    /* 50% off faint */
            break;
    }

    return (wtotal);
}


/*
    hitweight()
        Gets the fighting ability according to current weight This
        returns a  +2 hit for very light pack weight, +1 hit
        for light pack weight, 0 hit for medium pack weight, -1 hit for heavy
        pack weight, -2 hit for very heavy pack weight
*/

int
hitweight(void)
{
    return(3 - foodlev);
}
