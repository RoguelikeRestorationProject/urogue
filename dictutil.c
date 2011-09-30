/*
    dictutil.c

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

/*************************************************************************
**   Utilities for Dictionary Maintenence Functions
*************************************************************************/

static char sccsid[] = "%W% %G%";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if !defined(OS2) && !defined(_WIN32)
   #include <unistd.h>
#else
   #include <io.h>
   #include <fcntl.h>
#endif

#include "dict.h"
#include "dictutil.h"
#include "rogue.h"

int   trace;
FILE  *ft;


/***********
**  Read 'count' characters into 'buffer' at 'offset' in a binary file
**  Return 0 on success; -1 on failure;
***********/

int block_read( FILE *fi , char *buffer , size_t count , long offset )
{
   if ( fseek(fi,offset,SEEK_SET) == -1 )
      return( -1 );

   if ( fread(buffer,1,count,fi) != count )
      return( -1 );
   return( 0 );
}

/***********
**  Write 'count' characters from 'buffer' to a binary file.
**  Return -1 on failure; 0 on success.
***********/

int block_write( FILE *fo , char *buffer , size_t count )
{
   if ( fwrite(buffer,1,count,fo) != count )
      return( -1 );
   return( 0 );
}

/***********
**  Load a dictionary table entry with id TOC_id into memory pointed to by block.
**  Update the dictionary TOC.
**  If *block=NULL, allocate the block of memory.
**  Return 0 on success; -1 on failure.
**  Set dt_entry->ptr to where the block is stored.
***********/

void *dict_load_block( DICTIONARY *dict , char *toc_id ,
                          FILE *fi , void *block )
{  DICT_TOC_ENTRY  *dt_entry;
   static void     *ptr;
   int             index, ret_code;

   index = dict_toc_index( dict , toc_id );
   if ( index != -1 ) {  /* Found the id */
      dt_entry = &(dict->toc[index]);
   } else {
      signal_error( "dict_load_block: could not find TOC_id" , toc_id , 1 );
      return( NULL );
   } /* endif */

   if ( block == NULL ) {
      ptr = malloc( dt_entry->size );
      if ( trace > 3 ) {
         fprintf( ft , "\ndict_load_block allocates %lx bytes at location %p\n" ,
                   dt_entry->size , ptr );
      } /* endif */
   } else {
      ptr = block;
      if ( trace > 3 ) {
         fprintf( ft , "\ndict_load_block uses memory at location %p\n" , ptr );
      } /* endif */
   } /* endif */
   if ( ptr == NULL ) {
      signal_error( "dict_load_block: alloc failed " , toc_id , 1 );
      return( NULL );
   } /* endif */

   ret_code = block_read( fi ,
                          (char*)ptr ,
                          dt_entry->size ,
                          dt_entry->offset );
   if ( ret_code == -1 )
      return( NULL );

   if ( dt_entry->checksum !=
        compute_checksum( dt_entry->size , (char*)ptr ) ) {
      signal_error( "dict_load_block: invalid checksum ", toc_id, 1);
      return( NULL );
   } /* endif */

   dt_entry->ptr = ptr;

   if ( trace > 3 ) {
      fprintf( ft , "\nLoaded block\nTOC entry:    id:%s  offset:%lx  size:%lx  ptr:%p  checksum:%lx  type:%d\n" ,
               dict->toc[index].id , dict->toc[index].offset ,
               dict->toc[index].size , dict->toc[index].ptr ,
               dict->toc[index].checksum , dict->toc[index].type );
   } /* endif */

   return( ptr );
}

/***********
**  Save a dictionary table entry.
**  Update the dictionary TOC entry offset and checksum fields.
**  Return 0 on success, -1 on failure.
**  Note: It is assumed that the size and pointer fields in TOC entry are
**        already up to date; i.e., that they are consistent with the current
**        location and size of the block being written. This is essential
**        because the table of contents must have already been written
**        into the file.
***********/

BOOLEANC dict_save_block( DICTIONARY *dict , char *toc_id , FILE *fo )
{  DICT_TOC_ENTRY   *dt_entry;
   int              index, ret_code;
   char             *block;

   index = dict_toc_index( dict , toc_id );
   if ( index == -1 ) {
      signal_error( "dict_save_block: id not found " , toc_id , 1 );
      return( FALSE );
   } /* endif */
   dt_entry = &(dict->toc[index]);
   block = (char*)(dt_entry->ptr);

   if ( block == NULL ) {
      signal_error( "dict_save_block: NULL block " , toc_id , 1 );
      return( FALSE );
   } /* endif */

   /* dt_entry->offset = fseek( fo , 0 , SEEK_END ); */
   dt_entry->checksum = compute_checksum( dt_entry->size , block );
   ret_code = block_write( fo , dt_entry->ptr , dt_entry->size );
   if ( ret_code == -1 ) {
      signal_error( "dict_save_block: block_write failed " , toc_id , 1 );
      return( FALSE );
   } /* endif */

   if ( trace > 3 ) {
      fprintf( ft , "\nStored block\nTOC entry:           id:%s  offset:%lx  size:%lx  ptr:%p  checksum:%lx  type:%d\n" ,
               dict->toc[index].id , dict->toc[index].offset ,
               dict->toc[index].size , dict->toc[index].ptr ,
               dict->toc[index].checksum , dict->toc[index].type );
   } /* endif */

   return( TRUE );
}

/***********
**  Look up and id in the table of contents.
**  Return its index (-1 on failure).
***********/

int dict_toc_index( DICTIONARY *dict , char *toc_id )
{  int index;

   for ( index = 0 ; index < dict->sig->toc_size ; index++ ) {
           if ( strcmp(dict->toc[index].id,toc_id) == 0 )
                   return( index );
   } /* endfor */

   return( -1 );
}

/***********
**  Compute a block checksum.
**  (Currently just returns 0.)
***********/

unsigned long compute_checksum( size_t size , char *block )
{
    NOOP(size);
    NOOP(block);
    return( 0 );
}

/***********
**  Create a dictionary paramter entry.
***********/

DICT_PARM_ENTRY *dict_make_parm_entry( char *id , unsigned long value )
{  static DICT_PARM_ENTRY  *entry;

   entry = (DICT_PARM_ENTRY *) malloc( sizeof(DICT_PARM_ENTRY) );
   if ( entry == NULL )
      return(NULL);

   strncpy( entry->id , id , 13 );
   entry->value = value;

   return( entry );
}

/***********
**  Look up and id in the parameter array.
**  Return its index (-1 on failure).
***********/

int dict_parm_index( DICTIONARY *dict , char *parm_id )
{  long index;

   for ( index = 0 ; index < dict->sig->nparms ; index++ ) {
           if ( strcmp( dict->parm[index].id , parm_id ) == 0 )
                   return( (int) index );
   } /* endfor */

   return( -1 );
}

/***********
**  Reset table of contents offsets and checksums
**  in preparation for dict_save().
***********/

BOOLEANC dict_reset_toc_offsets( DICTIONARY *dict )
{  int  i;
   long offset;

   offset = sizeof(DICT_SIG)
          + dict->sig->toc_size * sizeof(DICT_TOC_ENTRY);
   for ( i = 0 ; i < dict->sig->toc_size ; i++ ) {
      dict->toc[i].offset = offset;
      offset += dict->toc[i].size;
      dict->toc[i].checksum =
         compute_checksum( dict->toc[i].size , dict->toc[i].ptr );
   } /* endfor */

   return( TRUE );
}

/***********
**  Load the names of the dictionary parameters.
**  14 parms
***********/

BOOLEANC dict_set_parm_ids( DICTIONARY *dict )
{
   if ( dict==NULL || dict->sig == NULL ) {
      signal_error( "dict_set_parm_ids: Allocate dict and signature first." , "" , 0 );
      return( FALSE );
   }
   dict->sig->nparms = 14;
   strcpy( dict->parm[0].id , "FLAGS_______" );
   strcpy( dict->parm[1].id , "ENTRY_COUNT_" );
   strcpy( dict->parm[2].id , "ARRAY_SIZE__" );
   strcpy( dict->parm[3].id , "ARRAY_USED__" );
   strcpy( dict->parm[4].id , "ARR_GROW_CT_" );
   strcpy( dict->parm[5].id , "STRING_MAX__" );
   strcpy( dict->parm[6].id , "STR_GROW_CT_" );
   strcpy( dict->parm[7].id , "LONG_CHAIN__" );
   strcpy( dict->parm[8].id , "ALLOW_CHAIN_" );
   strcpy( dict->parm[9].id , "HASH_TAB_SIZ" );
   strcpy( dict->parm[10].id , "HASH_MASK___" );
   strcpy( dict->parm[11].id , "HASH_GROW_CT" );
   strcpy( dict->parm[12].id , "CHECK_VALUE_" );
   strcpy( dict->parm[13].id , "SCAN_STR_IX_" );

   return( TRUE );
}

/***********
**  Set the dictionary parm structure from the values in the dict structure.
**  14 parms
***********/

BOOLEANC dict_set_parm_values( DICTIONARY *dict )
{  int  index;

   if ( (index=dict_parm_index(dict,"FLAGS_______")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->flags;

   if ( (index=dict_parm_index(dict,"ENTRY_COUNT_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->entry_count;

   if ( (index=dict_parm_index(dict,"ARRAY_SIZE__")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->array_size;

   if ( (index=dict_parm_index(dict,"ARRAY_USED__")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->array_used;

   if ( (index=dict_parm_index(dict,"ARR_GROW_CT_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->array_growth_count;

   if ( (index=dict_parm_index(dict,"STRING_MAX__")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->string_max;

   if ( (index=dict_parm_index(dict,"STR_GROW_CT_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->string_growth_count;

   if ( (index=dict_parm_index(dict,"LONG_CHAIN__")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->longest_chain_length;

   if ( (index=dict_parm_index(dict,"ALLOW_CHAIN_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->allowable_chain_length;

   if ( (index=dict_parm_index(dict,"HASH_TAB_SIZ")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->table_size;

   if ( (index=dict_parm_index(dict,"HASH_MASK___")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->hash_mask;

   if ( (index=dict_parm_index(dict,"HASH_GROW_CT")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->hash_growth_count;

   if ( (index=dict_parm_index(dict,"CHECK_VALUE_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->check_value;

   if ( (index=dict_parm_index(dict,"SCAN_STR_IX_")) == -1 )
      return( FALSE );
   dict->parm[index].value = (unsigned long)dict->scan_string_index;

   return( TRUE );
}


/***********
**  Set the values in the dict structure from the dictionary parm structure.
**  14 parms
***********/

BOOLEANC dict_set_parm_variables( DICTIONARY *dict )
{  int  index;

   if ( (index=dict_parm_index(dict,"FLAGS_______")) == -1 )
      return( FALSE );
   dict->flags = (unsigned long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"ENTRY_COUNT_")) == -1 )
      return( FALSE );
   dict->entry_count = (long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"ARRAY_SIZE__")) == -1 )
      return( FALSE );
   dict->array_size = (long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"ARRAY_USED__")) == -1 )
      return( FALSE );
   dict->array_used = (long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"ARR_GROW_CT_")) == -1 )
      return( FALSE );
   dict->array_growth_count = (int)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"STRING_MAX__")) == -1 )
      return( FALSE );
   dict->string_max = (long)dict->parm[index].value ;

   if ( (index=dict_parm_index(dict,"STR_GROW_CT_")) == -1 )
      return( FALSE );
   dict->string_growth_count = (int)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"LONG_CHAIN__")) == -1 )
      return( FALSE );
   dict->longest_chain_length = (int)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"ALLOW_CHAIN_")) == -1 )
      return( FALSE );
   dict->allowable_chain_length = (int)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"HASH_TAB_SIZ")) == -1 )
      return( FALSE );
   dict->table_size = (long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"HASH_MASK___")) == -1 )
      return( FALSE );
   dict->hash_mask = (unsigned long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"HASH_GROW_CT")) == -1 )
      return( FALSE );
   dict->hash_growth_count = (int)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"CHECK_VALUE_")) == -1 )
      return( FALSE );
   dict->check_value = (unsigned long)dict->parm[index].value;

   if ( (index=dict_parm_index(dict,"SCAN_STR_IX_")) == -1 )
      return( FALSE );
   dict->scan_string_index = (long)dict->parm[index].value;

   return( TRUE );
}

/***********
**  If trace (global) > 0 , signal an error
**  If severity > 0 , abort
***********/

void signal_error( char *header , char *message , int severity )
{
  FILE *fpe;

  if ( trace > 0 ) {
     printf( "%s: %s\n" , header , message );
     fpe = fopen( "ERROR.FIL" , "a" );
     fprintf( fpe , "\n%s: %s\n" , header , message );
     fclose( fpe );
  } /* endif */

  if ( severity > 0 )
     abort();
}
