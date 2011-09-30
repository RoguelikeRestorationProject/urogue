/*
    dict.c

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/******************
**  Change history:
**
**  (AK:04/03/95) - In dict_create, initialize hook for extensions to dictionary structure.
**  (AK:04/04/95) - In dict_insert, only set any_ptr when a new string entry is created.
**  (AK:04/17/95) - Added dict_union and dict_merge.
**  (AK:04/18/95) - In dict_create, added code to create signature,
**                  table of contents, and parameter array.
**  (AK:04/18/95) - Revised dict_load, dict_save
**  (AK:04/18/95) - Added dict_import
**
******************/

static char sccsid[] = "%W% %G%";

#include <stdlib.h>
#include <string.h>
#if !defined(OS2) && !defined(_WIN32)
   #include <unistd.h>
#endif

#include "dict.h"
#include "dictutil.h"

#define BUFLEN      300

/*************************************************************************
*       Generic dictionary and hash table functions for word and
*       string storage.
*
*       Implements generic dictionary management by using a
*       hash table with direct chaining.  All terms are stored in a
*       separate, unsorted, string table. When inserting into the
*       hash table exceeds the current size, allocate a new hash table
*       and rehash the old contents into the new table. When the string
*       table fills, append more empty entries onto the old table and
*       continue adding. Inserting a string that already exists increments
*       the count for the string. Deleting a string only decrements the
*       count to no less than 0. This will allow a 0 occurence string to be
*       retrieved. To remove the 0 occurence string table entries, you must
*       rebuild the dictionary. Merging from one dictionary to a new one will
*       do this because it only copies nonzero occurences.
*
*       Assumptions:
*               unsigned long >= 32 bits
*               int >= 16 bits
*               char == 8 bits
*               number of entries < 2^28
*************************************************************************/

/*************************************************************************
*       hash_string: calculate hash value for string, modified
*       version of hashpjw() from the new Dragon book.
*
*       s - string to calculate hash for
*
*       Returns: hash value
*************************************************************************/
static unsigned long hash_string(const char *s)
{
        unsigned long h = 0;
        unsigned long g = 0;

        while (*s) {
                h <<= 4;
                h += *s++;
                if ((g = h & 0xf0000000) != 0)
                        h ^= g >> 24;
        }
        return h ^ g;
}

/*************************************************************************
*       dict_create: create a dictionary and initialize its structures
*
*       toc_size - number of entries in table of contents ( >= 4 )
*       initial_string_count - number of strings to hold as initial allocation
*       initial_hash_chains - size of hash table, must be power of 4
*       max_chain_length - max number of elements in hash chain before signalling growth
*
*       Returns: pointer to dictionary
*               NULL - error creating dictionary
*               non-NULL - sucessful creation
*************************************************************************/

DICTIONARY *dict_create(
        const long toc_size,
        const long initial_string_count,
        const long initial_hash_chains,
        const long max_chain_length )
{
        DICTIONARY      *dtemp = NULL;
        STRING_ENTRY    *stemp = NULL;
        long    *ltemp = NULL;
        char    *sltemp;
        long    i, j, tocsize;

        /* check for a power of 4 in number of initial hash entries */
        switch(initial_hash_chains) {
        case 0x00000004:
        case 0x00000010:
        case 0x00000040:
        case 0x00000100:
        case 0x00000400:
        case 0x00001000:
        case 0x00004000:
        case 0x00010000:
        case 0x00040000:
        case 0x00100000:
        case 0x00400000:
        case 0x01000000:
        case 0x04000000:
                break;
        default:
                return NULL;
        }

        /* Allocate the dictionary structure */
        if ((dtemp = (DICTIONARY *)malloc(sizeof(DICTIONARY))) == NULL)
                goto err_exit;

        /* string count has to be within range */
        j = initial_string_count;
        if (j > 0x04000000)
                return NULL;

        /* force a reasonable value */
        if (j < 100)
                j = 100;

        /* Allocate the string table and string array */
        if ((stemp = (STRING_ENTRY *)malloc(sizeof(STRING_ENTRY) * j)) == NULL)
                goto err_exit;
        if ((sltemp = (char *)malloc(8 * j)) == NULL)
                goto err_exit;

        /* Allocate the hash table */
        if ((ltemp = (long *)malloc(sizeof(long) * initial_hash_chains)) == NULL)
                goto err_exit;

        /* Allocate the parameter array */
        if ( (dtemp->parm = (DICT_PARM_ENTRY *)malloc(14*sizeof(DICT_PARM_ENTRY))) == NULL)
                goto err_exit;

        /* Allocate the signature structure */
        if ( (dtemp->sig=(DICT_SIG*)malloc(sizeof(DICT_SIG))) == NULL )
                goto err_exit;

        /* Allocate the Table of Contents */
        tocsize = toc_size;
        if ( tocsize < 4 )
           tocsize = 4;
        dtemp->sig->toc_size = tocsize;
        if ( (dtemp->toc=(DICT_TOC_ENTRY*)malloc(dtemp->sig->toc_size*sizeof(DICT_TOC_ENTRY))) == NULL )
                goto err_exit;

        dtemp->check_value = DICT_VALIDATE;                     /* validation value */
        dtemp->flags = DICT_NONE;                               /* no flags set */
        dtemp->entry_count = 0;                                 /* nothing in either table */

        dtemp->string_table = stemp;                            /* connect string table */
        dtemp->string_max = j;                                  /* size of string table */
        dtemp->scan_string_index = DICT_ENTRY_NONE;             /* not pointing to anything in scan */
        dtemp->string_growth_count = 0;                         /* haven't grown any */

        dtemp->string_array = sltemp;                           /* character array for strings */
        dtemp->array_size = j * 8;                              /* how many bytes available */
        dtemp->array_used = 0;                                  /* nothing used yet */
        dtemp->array_growth_count = 0;                          /* haven't grown any */

        /* force maximum hash chain length to a reasonable value */
        i = max_chain_length;
        if (i < 1 || i > 200)
                i = 200;
        dtemp->chains = ltemp;                                  /* connect hash table */
        dtemp->longest_chain_length = 0;                        /* nothing in table yet */
        dtemp->allowable_chain_length = i;                      /* chain length limit */
        dtemp->table_size = initial_hash_chains;                /* size of hash table */
        dtemp->hash_mask = initial_hash_chains - 1;             /* mask for mod() function */
        dtemp->hash_growth_count = 0;                           /* haven't grown any */

        /* string table is empty */
        for (i = 0 ; i < j ; i++) {
                dtemp->string_table[i].string_offset = DICT_ENTRY_NONE;
                dtemp->string_table[i].count = 0;
                dtemp->string_table[i].next = DICT_ENTRY_NONE;
                dtemp->string_table[i].flags = 0;
                dtemp->string_table[i].hash_value = 0;
        }

        /* no entries chained */
        for (i = 0; i < initial_hash_chains; i++)
                dtemp->chains[i] = DICT_ENTRY_NONE;

        /* Initialize hook to extended dictionary structure  */
        dtemp->ext = NULL;

        /* Set up the parameter array */
        if ( dict_set_parm_ids(dtemp) == FALSE )
                goto err_exit;
        if ( dict_set_parm_values(dtemp) == FALSE )
                goto err_exit;

        /* Set up the Table of Contents */
        strcpy( dtemp->toc[0].id , "PARM" );
        dtemp->toc[0].size = dtemp->sig->nparms * sizeof(DICT_PARM_ENTRY);
        dtemp->toc[0].ptr = dtemp->parm;
        dtemp->toc[0].type = 0;

        strcpy( dtemp->toc[1].id , "HASH" );
        dtemp->toc[1].size = dtemp->table_size * sizeof(long);
        dtemp->toc[1].ptr = dtemp->chains;
        dtemp->toc[1].type = 0;

        strcpy( dtemp->toc[2].id , "STTB" );
        dtemp->toc[2].size = dtemp->string_max * sizeof(char);
        dtemp->toc[2].ptr = dtemp->string_table;
        dtemp->toc[2].type = 0;

        strcpy( dtemp->toc[3].id , "STAR" );
        dtemp->toc[3].size = dtemp->array_size * sizeof(STRING_ENTRY);
        dtemp->toc[3].ptr = dtemp->string_array;
        dtemp->toc[3].type = 0;

        /* Set up the signature */
        dtemp->sig->check_value = DICT_VALIDATE;
        dtemp->sig->toc_size = tocsize;
        dtemp->sig->nparms = 14;

        return dtemp;

        /* error exit - saves duplicate code */
err_exit:

        dict_destroy( dtemp );
        return NULL;
}

/*************************************************************************
*       dict_destroy: discard a dictionary and its contents
*               multiple calls to destroy the same dictionary is safe
*
*       dict - pointer to a dictionary to discard
*
*       Returns: status code
*               TRUE - dictionary destroyed
*               FALSE - error when destroying dictionary
*       Note: Does free the dictionary structure itself.
*************************************************************************/
BOOLEANC dict_destroy(DICTIONARY *dict)
{
        int  i;

        /* have to point to something */
        if (dict == NULL)
                return FALSE;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return FALSE;

        if ( (dict->sig==NULL) || (dict->toc==NULL) || (dict->parm==NULL) )
                return FALSE;

        /* Free the type=0 tables */
        for ( i = 0 ; i < dict->sig->toc_size ; i++ ) {
           if ( dict->toc[i].ptr != NULL && dict->toc[i].type == 0 )
                free( dict->toc[i].ptr );
        } /* endfor */

        /* Free the Table of Contents and signature */
        free( dict->toc );
        free( dict->sig );

        /* Free the dictionary structure itself */
        free( dict );

        return TRUE;
}

/*************************************************************************
*       dict_insert: add entries into the dictionary, growing as necessary
*
*       dict - pointer to dictionary to insert into
*       s - string to insert
*       count - count of occurences of string
*       flags - flag bits associated with the string
*       number - pointer to long to return word number
*
*       Returns: pointer to new entry in dictionary
*               NULL - error during insert
*               non-NULL - pointer to inserted or updated entry
*
*       Notes: if the entry already exists, increment the count.
*       If NULL is returned, the dictionary state can no longer
*       be trusted. Terminating with an error code would be
*       safest.
*************************************************************************/
STRING_ENTRY *dict_insert(DICTIONARY *dict, char *s,
        const long occurences, const unsigned long flags,
        void *any_ptr, long *number)
{
        unsigned long   hash_value;
        long    hash_index;
        long    string_index = -1;
        long    he2;
        int     chain_len;

        /* have to point to something */
        if (dict == NULL)
                return FALSE;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return FALSE;

        /* must have a string */
        if (s == NULL) {
                *number = -1;
                return FALSE;
        }

        /* must be non-NULL */
        if (s[0] == '\0') {
                *number = -1;
                return FALSE;
        }

        /* figure out which chain it should go into */
        hash_value = hash_string(s);
        hash_index = hash_value & dict->hash_mask;

        /* look for the entry in the chain */
        for (he2 = dict->chains[hash_index], chain_len = 0; he2 != DICT_ENTRY_NONE;
                        he2 = dict->string_table[he2].next, chain_len++) {
                char    *sa = &dict->string_array[dict->string_table[he2].string_offset];

                /* compare hash_value and then string, in that order, for speed */
                if (dict->string_table[he2].hash_value == hash_value && !strcmp(s, sa)) {
                        string_index = he2;
                        break;
                }
        }

        /* string wasn't found */
        if (string_index < 0) {
                /* make sure there is room in string entry table */
                if (dict->entry_count + 1 > dict->string_max) {
                        /* make new table 20% bigger */
                        dict->string_max = (dict->string_max * 6) / 5;
                        dict->string_table = (STRING_ENTRY *)realloc(dict->string_table,
                                        sizeof(STRING_ENTRY) * dict->string_max);
                        if (dict->string_table == NULL)
                                return NULL;
                        dict->string_growth_count++;
                }
                string_index = dict->entry_count;
                dict->entry_count++;

                /* make sure there is room in the string array */
                if (dict->array_used + (long)strlen(s) + 1 > dict->array_size) {
                        dict->array_size = (dict->array_size * 6) / 5;
                        dict->string_array = (char *) realloc(dict->string_array,
                                dict->array_size);
                        if (dict->string_array == NULL)
                                return NULL;
                }

                /* fill in starting values */
                strcpy(&dict->string_array[dict->array_used], s);
                dict->string_table[string_index].string_offset = dict->array_used;
                dict->array_used += (long) strlen(s) + 1;
                dict->string_table[string_index].count = 0 ;
                dict->string_table[string_index].flags = DICT_NONE;
                dict->string_table[string_index].any_ptr = any_ptr; /* (AK:04/04/95) */
                dict->string_table[string_index].hash_value = hash_value;

                /* hook entry at beginning of chain */
                dict->string_table[string_index].next = dict->chains[hash_index];
                dict->chains[hash_index] = string_index;

                /* record chain lengths */
                chain_len++;
                if (chain_len > dict->longest_chain_length)
                        dict->longest_chain_length = chain_len;

                /* if a chain is too long */
                if (chain_len > dict->allowable_chain_length) {
                        long    new_size = dict->table_size * 4;
                        long    new_mask = new_size - 1;
                        long    *hetemp;
                        long    i;
                        int     longest_chain;

                        if ((hetemp = (long *)malloc(sizeof(long) * new_size)) == NULL)
                                return NULL;

                        /* hash table chains are empty */
                        for (i = 0; i < new_size; i++)
                                hetemp[i] = DICT_ENTRY_NONE;

                        /* reset all chains */
                        for (i = 0; i < dict->entry_count; i++)
                                dict->string_table[i].next = DICT_ENTRY_NONE;

                        /* recreate hash table entries by reinserting all strings */
                        for (i = 0; i < dict->entry_count; i++) {
                                long    he = dict->string_table[i].hash_value & new_mask;

                                dict->string_table[i].next = hetemp[he];
                                hetemp[he] = i;
                        }

                        /* find longest chain length */
                        for (i = 0, longest_chain = 0; i < new_size; i++) {
                                int     len;
                                long    cur;

                                for (cur = hetemp[i], len = 0; cur != DICT_ENTRY_NONE;
                                                cur = dict->string_table[cur].next)
                                    len++;
                                        ;
                                if (len > longest_chain)
                                        longest_chain = len;
                        }

                        /* delete old table and attach new one */
                        free(dict->chains);
                        dict->chains = hetemp;
                        dict->longest_chain_length = longest_chain;
                        dict->table_size = new_size;
                        dict->hash_mask = new_mask;

                        /* keep track of growth */
                        dict->hash_growth_count++;
                }
        }

        dict->string_table[string_index].count += occurences;
        dict->string_table[string_index].flags |= flags;
        *number = string_index;

        return &dict->string_table[string_index];
}

/*************************************************************************
*       dict_delete: deletes an entry from the dictionary
*       (Actually, only decrements the entry's count)
*
*       dict - pointer to dictionary to delete
*       s - string to find and delete
*       count - count to decrement entry by
*
*       Returns: status code
*               TRUE - entry has been deleted
*               FALSE - entry wasn't found or error occured
*************************************************************************/
BOOLEANC dict_delete(const DICTIONARY *dict, const char *s, const long count)
{
        STRING_ENTRY    *se;
        long    n;

        /* find the string */
        if ((se = dict_search(dict, s, &n)) == NULL)
                return FALSE;

        /* decrement count and make sure it stays valid */
        se->count -= count;
        if (se->count < 0)
                se->count = 0;
        return TRUE;
}

/*************************************************************************
*       dict_search: look for entries in the dictionary
*
*       dict - pointer to dictionary to search
*       s - string to search for
*       number - pointer to long to return string number
*
*       Returns: pointer to string entry
*               NULL - entry not found
*               non-NULL - pointer to string entry
*************************************************************************/
STRING_ENTRY *dict_search(const DICTIONARY *dict, const char *s,
        long *number)
{
        unsigned long   hash_value;
        long    hash_index;
        long    string_index = -1;
        long    he;

        /* have to point to something */
        if (dict == NULL)
                return NULL;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return NULL;

        /* must have a string */
        if (s == NULL) {
                *number = -1;
                return NULL;
        }

        /* must be non-NULL */
        if (s[0] == '\0') {
                *number = -1;
                return FALSE;
        }

        /* figure out which chain it should be in */
        hash_value = hash_string(s);
        hash_index = hash_value & dict->hash_mask;

        /* look for the entry in the chain */
        for (he = dict->chains[hash_index]; he != DICT_ENTRY_NONE;
                        he = dict->string_table[he].next) {
                char    *sa = (char*)(&dict->string_array[dict->string_table[he].string_offset]);

                /* compare hash_value and then string, in that order, for speed */
                if (dict->string_table[he].hash_value == hash_value && !strcmp(s, sa)) {
                        string_index = he;
                        break;
                }
        }
        if (string_index < 0) {
                *number = -1;
                return NULL;
        }

        *number = string_index;
        return dict->string_table+string_index;
}

/*************************************************************************
*       dict_union: merges contents of 2 dictionaries into
*       a third
*
*       dict1 - pointer to dictionary 1
*       dict2 - pointer to dictionary 2
*
*       Returns: dictionary pointer
*               NULL - failed to merge
*               non-NULL - pointer to new dictionary
*
*       Notes: entries of the same string have their counts
*       added and their flags ORed together.
*************************************************************************/
DICTIONARY *dict_union( const DICTIONARY *dict1 , const DICTIONARY *dict2 )
{
        DICTIONARY    *dict = NULL;
        STRING_ENTRY  *se, *se2;
        long          initial_string_count, initial_hash_chains, max_chain_length;
        long          i, string_index;

        /***********
        **  Initialize the new dictionary.
        ***********/

        if ( dict1==NULL || dict2==NULL )
                goto err_exit;
        if ((dict=(DICTIONARY *)malloc(sizeof(DICTIONARY))) == NULL)
                goto err_exit;

        initial_string_count = dict1->string_max;
        initial_hash_chains = dict1->table_size;
        max_chain_length = dict1->allowable_chain_length;
        dict = dict_create( 4,
                            initial_string_count,
                            initial_hash_chains,
                            max_chain_length );

        /***********
        **  Copy the entries from dict1 into the new dictionary.
        ***********/

        for ( i = 0 ; i < dict1->entry_count ; i++ ) {
                se = (STRING_ENTRY*)&dict1->string_table[i];
                if ( se->count > 0 ) {
                        se2 = dict_insert(
                                  dict,
                                  dict1->string_array+se->string_offset,
                                  se->count,
                                  se->flags,
                                  NULL,
                                  &string_index );
                        if ( se2 == NULL )
                                goto err_exit;
                } /* endif */
        } /* endfor */

        /*  Merge the entries from dict2 into the new dictionary. */
        if ( dict_merge(dict,dict2,FALSE) == FALSE )
                goto err_exit;

        /*  Success. Return a pointer to the new dictionary.  */
        return( dict );

        /*  Failure. Ignominiously erase our tracks and return NULL. */
err_exit:
        dict_destroy( dict );
        return NULL;
}

/*************************************************************************
*       dict_merge: merges the contents of a dictionary into
*       another one, updating the contents of the destination
*
*       dst - dictionary to update
*       src - dictionary to add
*       move - boolean flag
*               TRUE - move to dest
*               FALSE - copy to dest
*
*       Returns: status code
*               TRUE - merge completed
*               FALSE - error on merge
*
*       Notes: entries of the same string have their counts
*       added. At the end of a move, src is empty. If there
*       is an error during a move, the contents of both
*       dictionaries cannot be trusted.
*************************************************************************/
BOOLEANC dict_merge(const DICTIONARY *dst, const DICTIONARY *src, const BOOLEANC move)
{
        STRING_ENTRY  *se, *se2;
        DICTIONARY    *dict1, *dict2;
        long          i, string_index, index;

        dict1 = (DICTIONARY*)src;
        dict2 = (DICTIONARY*)dst;

        /***********
        **  Copy the dictionary entries into the new dictionary.
        ***********/

        for ( i = 0 ; i < dict1->entry_count ; i++ ) {
                se = (STRING_ENTRY*)&dict1->string_table[i];
                if ( se->count > 0 ) {
                        se2 = dict_insert(
                                  dict2,
                                  dict1->string_array+se->string_offset,
                                  se->count,
                                  se->flags,
                                  NULL,
                                  &string_index );
                        if ( se2 == NULL )
                                goto err_exit;
                } /* endif */
        } /* endfor */

        /***********
        **  Set up the dictionary parameter vector.
        ***********/

        if ( dict_set_parm_values(dict2) == FALSE )
                return( FALSE );

        /***********
        **  Update the table of contents.
        **     PARM HASH STTB STAR
        ***********/

        if ( (index=dict_toc_index(dict2,"HASH")) == -1)
                goto err_exit;
        dict2->toc[index].size = dict2->table_size * sizeof(long);
        dict2->toc[index].ptr = dict2->chains;

        if ( (index=dict_toc_index(dict2,"STTB")) == -1)
                goto err_exit;
        dict2->toc[index].size = dict2->string_max * sizeof(char);
        dict2->toc[index].ptr = dict2->string_table;

        if ( (index=dict_toc_index(dict2,"STAR")) == -1)
                goto err_exit;
        dict2->toc[index].size = dict2->array_size * sizeof(STRING_ENTRY);
        dict2->toc[index].ptr = dict2->string_array;

        /***********
        **  Update the signature
        ***********/

        dict2->sig->checksum =
                compute_checksum( 4*sizeof(DICT_TOC_ENTRY) , (char*)(dict2->toc) );

        /***********
        **  If this is a move, destroy the source dictionary
        ***********/

        if ( move == TRUE )
                if ( dict_destroy(dict1) == FALSE )
                        goto err_exit;

        /***********
        **  Success
        ***********/

        return( TRUE );

        /***********
        **  Failure
        ***********/

err_exit:
        dict_destroy( dict2 );
        return FALSE;
}

/*************************************************************************
*       dict_string_by_number: return string pointer for
*       a given string number
*
*       number - string number
*
*       Returns: pointer to string entry
*               NULL - entry not found
*               non-NULL - pointer to string entry
*************************************************************************/
STRING_ENTRY *dict_string_by_number(const DICTIONARY *dict, const long number)
{
        if (dict == NULL)
                return NULL;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return NULL;

        /* string number has to be within range */
        if (number < 0 || number >= dict->entry_count)
                return NULL;

        return dict->string_table+number;
}

/*************************************************************************
*       dict_scan_begin: begin a scan of the dictionary
*
*       dict - pointer to dictionary to scan
*
*       Returns: status code
*               TRUE - scan initialized
*               FALSE - error
*
*       Notes: if a scan is already in progress, it is
*       abandoned and the scan is reset to the
*       beginning.
*************************************************************************/
BOOLEANC dict_scan_begin(DICTIONARY *dict)
{
        /* have to point to something */
        if (dict == NULL)
                return FALSE;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return FALSE;

        /* point to first entry in string table */
        dict->scan_string_index = 0;
        return TRUE;
}

/*************************************************************************
*       dict_scan_next: get the next entry in a scan
*
*       dict - pointer to dictionary to continue scanning
*
*       Returns: pointer to string entry
*               NULL - no more entries
*               non-NULL - next string entry
*************************************************************************/
STRING_ENTRY *dict_scan_next(DICTIONARY *dict)
{
        /* have to point to something */
        if (dict == NULL)
                return NULL;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                return NULL;

        /* scan index has to be within range */
        if (dict->scan_string_index < 0
                        || dict->scan_string_index > dict->entry_count)
                return NULL;

        /* for first non-empty table entry */
        while (dict->scan_string_index < dict->entry_count
                        && dict->string_table[dict->scan_string_index].count == 0)
                dict->scan_string_index++;

        /* past end of table? */
        if (dict->scan_string_index >= dict->entry_count)
                return NULL;

        return &dict->string_table[dict->scan_string_index++];
}


/*************************************************************************
*       dict_load - load a compiled dictionary into memory
*                   creates a new dictionary
*
*       fname - fully qualified file name
*
*       Returns: pointer to created dictionary structure
*                (NULL on failure)
*************************************************************************/
DICTIONARY *dict_load(const char *fname)
{
        DICTIONARY       *dict = NULL;
        int              code, index;
        FILE             *fi;
        int              ntoc;

        if ( (fi=fopen((char*)fname,"rb")) == NULL ) {
                signal_error( "dict_load: could not open file" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        if ((dict = (DICTIONARY *)malloc(sizeof(DICTIONARY))) == NULL) {
                /* signal_error( "dict_load: alloc failed" , "" , 0 ); */
                goto err_exit;
        } /* endif */

        /* Read the dictionary signature record */
        if ((dict->sig = (DICT_SIG *)malloc(sizeof(DICT_SIG))) == NULL) {
                goto err_exit;
        } /* endif */
        code = block_read( fi , (char*)(dict->sig) , sizeof(DICT_SIG) , 0 );
        if ( code == -1 ) {
                signal_error( "dict_load: could not read signature" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        if ( dict->sig->check_value != DICT_VALIDATE ) {
                signal_error( "dict_load: could not validate file" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */
        dict->check_value = dict->sig->check_value;

        /* Read the dictionary Table of Contents */
        ntoc = dict->sig->toc_size;
        dict->toc = (DICT_TOC_ENTRY *) malloc( ntoc * sizeof(DICT_TOC_ENTRY) );
        if ( dict->toc == NULL ) {
                signal_error( "dict_load: alloc of TOC failed" , "" , 0 );
                goto err_exit;
        } /* endif */
        code = block_read( fi ,
                           (char*)(dict->toc) ,
                           ntoc * sizeof(DICT_TOC_ENTRY) ,
                           sizeof(DICT_SIG) );
        if ( code == -1 ) {
                signal_error( "dict_load: could not read Table of Contents" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        /* Read the dictionary parameters */
        dict->parm = (DICT_PARM_ENTRY *)
                         dict_load_block( dict , "PARM" , fi , NULL );
        if ( dict->parm == NULL ) {
                signal_error( "dict_load: could not load parameter table" , "" , 0 );
                goto err_exit;
        } /* endif */
        index = dict_toc_index( dict , "PARM" );
        dict->sig->nparms = dict->toc[index].size / sizeof(DICT_PARM_ENTRY);

        /* Set the parameter values in the dictionary structure */
        if ( dict_set_parm_variables(dict) == FALSE )
                goto err_exit;

        /* Load the string array */
        dict->string_array = (char *)
                dict_load_block( dict , "STAR" , fi , NULL );
        if ( dict->string_array == NULL ) {
                signal_error( "dict_load: could not load string array" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        /* Load the string table */
        dict->string_table = (STRING_ENTRY *)
                dict_load_block( dict , "STTB" , fi , NULL );
        if ( dict->string_table == NULL ) {
                signal_error( "dict_load: could not load string table" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        /* Load the hash table */
        dict->chains = (long *)
                dict_load_block( dict , "HASH" , fi , NULL );
        if ( dict->chains == NULL ) {
                signal_error( "dict_load: could not load hash table" , (char*)fname , 0 );
                goto err_exit;
        } /* endif */

        /*  Initialize the hook for dictionary extensions  */
        dict->ext = NULL;

        /*  Success  */
        fclose( fi );
        return( dict );

        /*  Failure  */
err_exit:
        if ( fi != NULL )
                fclose( fi );
        dict_destroy( dict );
        return NULL;
}


/*************************************************************************
*       dict_save - save a dictionary from memory into a file
*
*       dict - pointer to dictionary to save
*       fname - full qualified file name prefix of dictionary
*
*       Returns: status code
*               TRUE - dictionary was saved sucessfully
*               FALSE - error during save
*************************************************************************/
BOOLEANC dict_save( DICTIONARY *dict, const char *fname )
{
        int   index, ret_code;
        FILE  *fo = NULL;

        /* Have to be pointing at a valid dictionary */
        if ( dict == NULL || dict->sig->check_value != DICT_VALIDATE )
                goto err_exit;

        /* Open the file for output */
        if ( (fo=fopen((char*)fname,"wb")) == NULL )
                goto err_exit;

        /*  Make the table of contents entries current  */
        /*  Note: This will not be necessary once the data is stored in EVECTORs  */

        if ( (index=dict_toc_index(dict,"PARM")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->sig->nparms * sizeof(DICT_PARM_ENTRY);
        dict->toc[index].ptr = dict->parm;

        if ( (index=dict_toc_index(dict,"STAR")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->array_size * sizeof(char);
        dict->toc[index].ptr = dict->string_array;

        if ( (index=dict_toc_index(dict,"STTB")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->string_max * sizeof(STRING_ENTRY);
        dict->toc[index].ptr = dict->string_table;

        if ( (index=dict_toc_index(dict,"HASH")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->table_size * sizeof(long);
        dict->toc[index].ptr = dict->chains;

        /*  Reset the TOC offsets and checksums for ALL tables  */
        /*  (not just type=0 tables)                            */
        dict_reset_toc_offsets( dict );

        /* Set the dictionary parm structure from the parameter values */
        if ( dict_set_parm_values(dict) == FALSE )
                goto err_exit;

        /* Save the signature */
        dict->sig->checksum = compute_checksum( sizeof(DICT_SIG) , (char*)(dict->sig) );
        ret_code = block_write( fo ,
                                (char*)dict->sig ,
                                sizeof(DICT_SIG) );
        if ( ret_code == -1 )
                goto err_exit;

        /* Save the table of contents */
        ret_code = block_write( fo,
                                (char*)dict->toc,
                                dict->sig->toc_size * sizeof(DICT_TOC_ENTRY) );
        if ( ret_code == -1 )
                goto err_exit;

        /* Save the tables */
        /* For now, only save type=0 tables */
        for ( index = 0 ; index < dict->sig->toc_size ; index++ ) {
                if ( dict->toc[index].type == 0 ) {  /* Ordinary table */
                        ret_code = dict_save_block( dict , dict->toc[index].id , fo );
                        if ( ret_code == FALSE )
                                goto err_exit;
                     } /* endif */
        } /* endfor */

        /*  Success  */
        fclose( fo );
        return TRUE;

        /*  Failure  */
err_exit:
        if ( fo != NULL )
                fclose( fo );
        return FALSE;
}


/*************************************************************************
*       dict_import: read in an ASCII dictionary.
*
*       dict_fname - name of dictionary file
*       parameters to create a DICTIONARY structure (see dict_create)
*
*       Returns: pointer to created DICTIONARY structure
*                (NULL on failure)
*
*************************************************************************/
DICTIONARY *dict_import( const char *dict_fname ,
                         const long initial_string_count ,
                         const long initial_hash_entries ,
                         const long max_chain_length )
{
        DICTIONARY     *dict;
        char           buffer[BUFLEN], ch;
        int            index, c, c0;
        long           number;
        FILE           *fi = NULL;

        /***********
        **  Dictionary setup.
        ***********/

        dict = dict_create( 4,
                            initial_string_count ,
                            initial_hash_entries ,
                            max_chain_length );
        if ( dict == NULL )
           goto err_exit;

        /***********
        **  Read the dictionary file
        **  Each line should have one word or a string delimited by '|'
        ***********/

        if ( (fi=fopen(dict_fname,"r")) == NULL )
                goto err_exit;
        while( fgets(buffer,BUFLEN,fi) != NULL ) {
                c0 = 0;
                   /*  Skip to non-blank  */
                while ( (c0<BUFLEN-2) && (buffer[c0]==' ') ) ++c0;
                if ( buffer[c0] == '|' ) {
                        c = ++c0;
                        ch = '|';
                } else {
                        c = c0;
                        ch = ' ';
                } /* endif */
                   /*  Scan to blank or matching '|' */
                while ( (c<BUFLEN-1) && (buffer[c]!='\0') &&
                        (buffer[c]!='\n') && (buffer[c]!=ch)  )
                        ++c;
                buffer[c] = '\0';
                   /*  Insert the word  */
                if ( dict_insert(dict,buffer+c0,1,0,NULL,&number) == NULL )
                        goto err_exit;
        } /* endwhile */

        /***********
        **  Fill in the dictionary parameter vector.
        ***********/

        if ( dict_set_parm_values(dict) == FALSE )
                goto err_exit;

        /***********
        **  Update the table of contents for HASH STTB STAR
        ***********/

        if ( (index=dict_toc_index(dict,"HASH")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->table_size * sizeof(long);

        if ( (index=dict_toc_index(dict,"STTB")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->string_max * sizeof(char);

        if ( (index=dict_toc_index(dict,"STAR")) == -1 )
                goto err_exit;
        dict->toc[index].size = dict->array_size * sizeof(STRING_ENTRY);

           /*  Success. Return a pointer to the new dictionary.  */
        fclose(fi);
        return( dict );

           /*  Failure. Ignominiously erase our tracks and return NULL.  */
err_exit:
        if ( fi != NULL )
           fclose(fi);
        dict_destroy( dict );
        return NULL;
}

/*************************************************************************
*       dict_export - save an extended dictionary from memory into
*                     an ASCII file
*
*       dict - pointer to dictionary to save
*       fname - full qualified file name prefix of dictionary
*
*       Returns: status code
*               TRUE - dictionary was saved sucessfully
*               FALSE - error during save
*
*************************************************************************/
BOOLEANC dict_export( DICTIONARY *dict , const char *fname )
{
        FILE          *fp = NULL;
        STRING_ENTRY  *se;
        int       i;

        /* have to point to something */
        if (dict == NULL)
                goto err_exit;

        /* check value has to be OK */
        if (dict->check_value != DICT_VALIDATE)
                goto err_exit;

        /* must have a filename */
        if (fname == NULL)
                goto err_exit;

        fp = fopen( (char*)fname , "w" );
        if ( fp == NULL )
                goto err_exit;

        for ( i = 0 ; i < dict->entry_count ; i++ ) {
           se = &dict->string_table[i];
           fprintf( fp , "|%s|\n" , dict->string_array + se->string_offset );
        } /* endfor */
        fclose( fp );

        /*  Success.  */
        fclose(fp);
        return TRUE;

        /*  Failure.  */
err_exit:
        if ( fp != NULL )
           fclose(fp);
        dict_destroy( dict );
        return FALSE;
}
