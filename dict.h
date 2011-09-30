/*
    dict.h

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*-----------------
  Change history:
  (AK:04/03/95) - Added hook for extensions to dictionary structure.
  (AK:04/04/95) - Added dictionary signature, table of contents and parameter
                  structure defintions and fields in DICTIONARY structure
-------------------*/

#ifndef dict_h_included
#define dict_h_included

static char dict_sccsid[] = "%W% %G%";

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define DICT_NONE       0x00000000              /* no flags set */
#define DICT_VALIDATE   0xdeadbeef              /* look for this in every dictionary structure */
#define DICT_ENTRY_NONE -1                      /* index of invalid entry */

typedef unsigned char BOOLEANC;

/*------------------
  string table entry
  ------------------*/
typedef struct string_entry {
        long    string_offset;       /* offset in string array */
        long    count;               /* number of occurrences of string */
        long    next;                /* offset of next in hash chain */
        void    *any_ptr;            /* free pointer */
        unsigned long flags;         /* user definable flag value for string */
        unsigned long hash_value;    /* hash value of string using hash function */
} STRING_ENTRY;

/*--------------------
  Dictionary signature (AK:04/04/95)
  --------------------*/
typedef struct dict_sig_ {      /*  Dictionary signature               */
   unsigned long  check_value;  /*  0xdeadbeef                         */
   int            toc_size;     /*  # of entries in Table of Contents  */
   long           nparms;       /*  # of parameters                    */
   unsigned long  checksum;     /*  Checksum for TOC                   */
} DICT_SIG;

/*----------------------------------
  Dictionary table of contents entry (AK:04/04/95)
  ----------------------------------*/
typedef struct TOC_entry_ {     /*  Dictionary table of contents entry  */
   char           id[5];        /*  Field identifier:                   */
   long           offset;       /*  Offset (bytes) of entry in file     */
   long           size;         /*  Size (bytes) of entry               */
   void           *ptr;         /*  Where entry is stored in memory     */
   unsigned long  checksum;     /*  Checksum of entry                   */
   int            type;         /*  0=ordinary ; 1=EVECTOR ; 2=NULL     */
} DICT_TOC_ENTRY;

/*--------------------------------
  Dictionary parameter table entry (AK:04/04/95)
  --------------------------------*/
typedef struct dict_parm_entry_ {   /*  Dictionary parameter table entry  */
   char           id[13];           /*  Parameter identifier              */
   unsigned long  value;            /*  Parameter value                   */
} DICT_PARM_ENTRY;


/*---------------------------
  Hash dictionary information
  ---------------------------*/
typedef struct dictionary {
        unsigned long   check_value;            /* check validation value */
        unsigned long   flags;                  /* flag values */
        long    entry_count;                    /* number of used entries in each table */

        char    *string_array;                  /* storage for strings */
        long    array_size;                     /* number of bytes allocated */
        long    array_used;                     /* number of bytes occupied */
        int     array_growth_count;             /* number of times grown */

        STRING_ENTRY    *string_table;          /* string table */
        long    string_max;                     /* max number of entries in string table */
        long    scan_string_index;              /* current index into string table for scan */
        int     string_growth_count;            /* number of times had to grow string table */

        long            *chains;                /* vector of array indices to hash entries */
        int     longest_chain_length;           /* longest chain length in hash table */
        int     allowable_chain_length;         /* chain lengths always < this */
        long    table_size;                     /* number of elements in hash entries vector */
        unsigned long   hash_mask;              /* mask for doing mod() function */
        int     hash_growth_count;              /* number of times had to grow hash table */

        void    *ext;                           /* Hook for extensions to the dictionary (AK:04/03/95) */

        DICT_SIG         *sig;                  /* Signature (AK:04/04/95) */
        DICT_TOC_ENTRY   *toc;                  /* Table of contents (AK:04/04/95) */
        DICT_PARM_ENTRY  *parm;                 /* Parameters (AK:04/04/95) */

} DICTIONARY;

/*--------------------------------------------------------------
  dict_create: create a dictionary and initialize its structures
  --------------------------------------------------------------*/
extern DICTIONARY *dict_create(
        const long  toc_size,
        const long  initial_string_count,
        const long  initial_hash_entries,
        const long  max_chain_length );

/*-------------------------------------------------
  dict_delete: deletes an entry from the dictionary
  -------------------------------------------------*/
extern BOOLEANC dict_delete(
        const DICTIONARY  *dict,
        const char        *s,
        const long        count );

/*---------------------------------------------------
  dict_destroy: discard a dictionary and its contents
  ---------------------------------------------------*/
extern BOOLEANC dict_destroy(
        DICTIONARY  *dict );

/*------------------------------------------------
  dict_export: write a dictionary to an ASCII file
  ------------------------------------------------*/
extern BOOLEANC dict_export(
        DICTIONARY  *dict,
        const char  *fname );

/*-------------------------------------------------
  dict_import: read a dictionary from an ASCII file
  -------------------------------------------------*/
extern DICTIONARY *dict_import(
        const char  *dict_fname,
        const long  initial_string_count,
        const long  initial_hash_entries,
        const long  max_chain_length );

/*------------------------------------------------------------------
  dict_insert: add entries into the dictionary, growing as necessary
  ------------------------------------------------------------------*/
extern STRING_ENTRY *dict_insert(
        DICTIONARY           *dict,
        char                 *s,
        const long           occurences,
        const unsigned long  flags,
        void                 *any_ptr,
        long                 *number );

/*----------------------------------------------------
  dict_load: read a dictionary from a file into memory
  ----------------------------------------------------*/
extern DICTIONARY *dict_load(
        const char  *fname );

/*-----------------------------------------------------------------
  dict_merge: merges the contents of a dictionary into another one,
  updating the contents of the destination
  -----------------------------------------------------------------*/
extern BOOLEANC dict_merge(
        const DICTIONARY  *dst,
        const DICTIONARY  *src,
        const BOOLEANC    move );

/*----------------------------------------------------
  dict_save: save a dictionary from memory into a file
  ----------------------------------------------------*/
extern BOOLEANC dict_save(
        DICTIONARY  *dict,
        const char  *fname );

/*-----------------------------------------------
  dict_scan_begin: begin a scan of the dictionary
  -----------------------------------------------*/
extern BOOLEANC dict_scan_begin(
        DICTIONARY  *dict );

/*--------------------------------------------
  dict_scan_next: get the next entry in a scan
  --------------------------------------------*/
extern STRING_ENTRY *dict_scan_next(
        DICTIONARY  *dict );

/*-----------------------------------------------
  dict_search: look for entries in the dictionary
  -----------------------------------------------*/
extern STRING_ENTRY *dict_search(
        const DICTIONARY  *dict,
        const char        *s,
        long              *number );

/*----------------------------------------------------------------------
  dict_string_by_number: return string pointer for a given string number
  ----------------------------------------------------------------------*/
extern STRING_ENTRY *dict_string_by_number(
        const DICTIONARY  *dict,
        const long        number );

/*----------------------------------------------------------
  dict_union: merges contents of 2 dictionaries into a third
  ----------------------------------------------------------*/
extern DICTIONARY *dict_union(
        const DICTIONARY  *dict1,
        const DICTIONARY  *dict2 );

#endif  /* dict_h_included */
