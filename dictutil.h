/*
    dictutil.h

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/****************************************************************/
/*     HEADERS FOR DICTIONARY MAINTENENCE ROUTINE UTILITIES     */
/****************************************************************/

#ifndef dictutil_h_included
#define dictutil_h_included

static char dictutil_sccsid[] = "%W% %G%";

#include <stdio.h>
#include "dict.h"

/*-------------------------------
  Binary read of a block of bytes
  -------------------------------*/
extern int block_read(
        FILE  *fi,
        char  *buffer,
        size_t  count,
        long  offset );

/*--------------------------------
  Binary write of a block of bytes
  --------------------------------*/
extern int block_write(
        FILE  *fo,
        char  *buffer,
        size_t  count );

/*--------------------------------------
  Compute a checksum of a block of bytes
  --------------------------------------*/
extern unsigned long compute_checksum(
        size_t size,
        char  *block );

/*-----------------------------------------------------------------
  Load a block of bytes from a compiled dictionary file into memory
  -----------------------------------------------------------------*/
extern void *dict_load_block(
        DICTIONARY  *dict,
        char        *toc_id,
        FILE        *fi,
        void        *block );

/*-----------------------------------
  Create a dictionary parameter entry
  -----------------------------------*/
extern DICT_PARM_ENTRY *dict_make_parm_entry(
        char *id,
        unsigned long value );

/*------------------------------------
  Look up an id in the parameter array
  ------------------------------------*/
extern int dict_parm_index(
        DICTIONARY  *dict,
        char        *parm_id );

/*-------------------------------
  Reset table of contents offsets
  -------------------------------*/
extern BOOLEANC dict_reset_toc_offsets(
        DICTIONARY  *dict );

/*-----------------------------------------------------------------
  Save a block of bytes from memory into a compiled dictionary file
  -----------------------------------------------------------------*/
extern BOOLEANC dict_save_block(
        DICTIONARY  *dict,
        char        *toc_id,
        FILE        *fo );

/*--------------------------------------------------------------------
  Set the dictionary parm values from the values in the dict structure
  --------------------------------------------------------------------*/
extern BOOLEANC dict_set_parm_values(
        DICTIONARY  *dict );

/*--------------------------------------------------------------------
  Set the values in the dict structure from the dictionary parm values
  --------------------------------------------------------------------*/
extern BOOLEANC dict_set_parm_variables(
        DICTIONARY  *dict );

/*---------------------------
  Set the dictionary parm ids
  ---------------------------*/
extern BOOLEANC dict_set_parm_ids(
        DICTIONARY *dict );

/*--------------------------------------
  Look up an id in the table of contents
  --------------------------------------*/
extern int dict_toc_index(
        DICTIONARY  *dict,
        char        *toc_id );

/*------------------------------------
  Record and error and abort if needed
  ------------------------------------*/
extern void signal_error(
	char *header,
	char *message,
	int severity );
#endif
