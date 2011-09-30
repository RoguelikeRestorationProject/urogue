/*
    memory.c

    UltraRogue: The Ultimate Adventure in the Dungeons of Doom
    Copyright (C) 1995 Herb Chong
    All rights reserved.

    See the file LICENSE.TXT for full copyright and licensing information.
*/

#include <stdio.h>
#include <stdlib.h>

#include "dict.h"
#include "memory.h"
#include "rogue.h"

static char sccsid[] = "%W%\t%G%";

/*	Debugging memory allocation code that tries to trap common memory problems
	like overwriting storage and stepping on memory pointer chains. If code
	doesn't use malloc, free, and realloc a lot, these routines can be left in
	as added protection against undetected storage bugs.
*/

/* 	FENCE_SIZE should be a multiple of sizeof(size_t) to prevent alignment problems.
	The code assumes that malloc and realloc return pointers aligned at least on size_t
	sized boundaries and that a pointer needs alignment no more strict than that of an
	object needed to hold a size_t.
*/

#define FENCE_SIZE (sizeof(size_t) * 1024)

static int memdebug_level = 0;
static DICTIONARY *allocations = NULL;
static FILE *trace_file = NULL;

/* set the debug level */
void mem_debug(const int level)
{
	memdebug_level = level;

	if (trace_file == NULL)
		trace_file = fopen("trace", "w");

	/* all except 0, 1, and unknown fall through */
	switch(memdebug_level) {
	case 2:
		fprintf(trace_file, "+++ Memory tracking possible, ");
	case 1:
		fprintf(trace_file, "+++ Memory debugging enabled, ");
		break;
	case 0:
		fprintf(trace_file, "+++ Memory debugging disabled, ");
		break;
	default:
		fprintf(trace_file, "!!! Unknown memory debug level set, enabling level 1, ");
		memdebug_level = 1;
		break;
	}
	fprintf(trace_file, "fence size = %d\n", FENCE_SIZE);
}

/* set memory tracking on or off */
/* turning it off deletes all tracking data */
void mem_tracking(int flag)
{
	/* do nothing if debuglevel is too low */
	if (memdebug_level < 2)
		return;

	/* turn on tracking */
	if (flag > 0) {
		if (allocations != NULL) {
			dict_destroy(allocations);
			allocations = NULL;
		}
		allocations = dict_create(8, 100, 4, 20);
		if (allocations == NULL) {
			fprintf(trace_file, "!!! Unable to allocate tracking table!\n");
			abort();
		}
	}
	/* turn off tracking */
	else if (allocations != NULL) {
		dict_destroy(allocations);
		allocations = NULL;
	}
}

/* go through all pointers and see if they are OK, aborting if not */
/* always returns 1 if not aborting so that it can be included in  */
/* if statement boolean expressions */
int mem_check(char *fname, int linenum)
{
	STRING_ENTRY *se;

	/* scan of a NULL dictionary always succeeds */
	if (allocations == NULL)
		return TRUE;

	if (!dict_scan_begin(allocations)) {
		fprintf(trace_file, "!!! Dictionary scan initialization failed!\n");
		abort();
	}

	fprintf(trace_file, "\n+++ --- Starting pointer scan\n");
	fprintf(trace_file, "+++ --- At %s, %d\n", fname, linenum);

	/* mem_validate aborts if there is a problem */
	while((se = dict_scan_next(allocations)) != NULL)
		mem_validate(se->any_ptr);

	fprintf(trace_file, "+++ --- Done pointer scan\n\n");

	/* always return a good value if execution arrives here */
	return 1;
}

/* allocate some memory and initialize header and trailer */
void *mem_malloc(const size_t bytes)
{
	char *mem_temp;
	size_t real_size = bytes + (FENCE_SIZE << 1);

	/* allocate including guard bytes to detect some ways of overwriting of memory areas */
	mem_temp = (void *)malloc(real_size);
	if (memdebug_level > 0) {
		fprintf(trace_file, "+++ Requested size of %ld bytes\n", bytes);
		fprintf(trace_file, "+++ Actual malloc of %ld bytes located at %p\n", real_size, mem_temp);
	}

	/* if allocation succeeded, set management data */
	if (mem_temp != NULL) {
		size_t i;
		char *end;

		/* do beginning marker bytes */
		for (i = 0; i < FENCE_SIZE - sizeof(size_t); i++)
			*mem_temp++ = 145;

		/* save size in header too */
		if (memdebug_level > 0)
			fprintf(trace_file, "*** Requested memory size stored at %p\n", mem_temp);
		*(size_t *)mem_temp = bytes;

		/* finally, point to storage we are going to hand out */
		mem_temp += sizeof(size_t);

		/* now, point to trailer bytes and do them */
		end = mem_temp + bytes;
		for (i = 0; i < FENCE_SIZE; i++)
			*end++ = 145;

		/* now zap contents to zero */
		for (i = 0; i < bytes; i++)
			mem_temp[i] = 0;
	}

	/* track pointer if needed */
	if (memdebug_level > 1 && allocations != NULL) {
		char key[16];
		long temp;

		sprintf(key, "%p", mem_temp);
		if (dict_insert(allocations, key, 1, (const unsigned long) bytes, mem_temp, &temp) == NULL) {
			fprintf(trace_file, "!!! Insert of pointer tracking info failed\n");
			abort();
		}
	}

	/* allow caller to do error handling */
	if (memdebug_level > 0) {
		fprintf(trace_file, "--- Returning pointer of %p\n", mem_temp);
		fflush(trace_file);
	}
	return (void *)mem_temp;
}

/* release some memory, making sure that it was properly allocated */
void mem_free(const void *ptr)
{
	char *mem_temp;
	size_t mem_size;
	size_t i;

	if (memdebug_level > 0)
		fprintf(trace_file, "+++ Free of memory located at %p\n", ptr);
	if (ptr == NULL) {
		if (memdebug_level > 0) {
			fprintf(trace_file, "!!! Freeing NULL pointer\n");
			fflush(trace_file);
		}
		abort();
	}

	mem_validate(ptr);	/* doesn't return on error */

	/* get location of size of area */
	mem_temp = (char *)ptr - sizeof(size_t);

	/* get and calculate real size */
	mem_size = *(size_t *)mem_temp + (FENCE_SIZE << 1);

	/* if doing memory tracking */
	if (memdebug_level > 1 && allocations != NULL) {
		char key[16];
		STRING_ENTRY *se;
		long temp;

		sprintf(key, "%p", ptr);

		if ((se = dict_search(allocations, key, &temp)) == NULL) {
			fprintf(trace_file, "!!! Deleting pointer not found in tracking info\n");
			abort();
		}

		if (se->count == 0) {
			fprintf(trace_file, "!!! Freeing a pointer that has already been freed!\n");
			abort();
		}
		else if (se->flags != mem_size - (FENCE_SIZE << 1)) {
			fprintf(trace_file, "!!! Stored size different from tracking size!\n");
			abort();
		}

		/* remember deleted stuff by zeroing the allocation count */
		se->count = 0;
		se->flags = 0;
	}

	/* zap bytes being freed */
	for (i = 0, mem_temp = (char *)ptr - FENCE_SIZE; i < mem_size; i++, mem_temp++)
		*mem_temp = 243;

	if (memdebug_level > 0)
		fflush(trace_file);

	mem_temp = (char *)ptr - FENCE_SIZE;
	free((void *)mem_temp);
}

/* reallocate some memory, making sure that it was properly allocated */
void *mem_realloc(const void *ptr, const size_t new_size)
{
	char *mem_temp = (char *)ptr;
	size_t real_size = new_size + (FENCE_SIZE << 1);
	size_t mem_size;
	long i;

	if (memdebug_level > 0) {
		fprintf(trace_file, "+++ Requested size of %ld bytes\n", new_size);
		fprintf(trace_file, "+++ Actual realloc of %ld bytes located at %p\n", real_size, mem_temp);
	}
	if (ptr == NULL) {
		if (memdebug_level > 0) {
			fprintf(trace_file, "!!! Reallocating NULL pointer\n");
			fflush(trace_file);
		}
		abort();
	}

	mem_validate(ptr);	/* doesn't return on error */

	/* if doing memory tracking */
	if (memdebug_level > 1 && allocations != NULL) {
		char key[16];
		STRING_ENTRY *se;
		long temp;

		sprintf(key, "%p", ptr);

		if ((se = dict_search(allocations, key, &temp)) == NULL) {
			fprintf(trace_file, "!!! Deleting a pointer not found in tracking info!\n");
			abort();
		}

		/* point to size bytes */
		mem_temp = (char *)ptr - sizeof(size_t);

		/* get user size */
		mem_size = *(size_t *)mem_temp;

		if (se->count == 0) {
			fprintf(trace_file, "!!! Freeing a pointer that has already been freed!\n");
			abort();
		}
		else if (se->flags != mem_size) {
			fprintf(trace_file, "!!! Stored size different from tracking size!\n");
			abort();
		}

		/* remember deleted stuff by zeroing the allocation count */
		se->count = 0;
		se->flags = 0;
	}


	/* header marker bytes will be copied by the realloc */
	mem_temp = (char *)ptr - FENCE_SIZE;
	mem_temp = realloc((void *)mem_temp, real_size);

	if (mem_temp != NULL) {
		char *end;

		/* save size in header too */
		mem_temp += FENCE_SIZE - sizeof(size_t);
		if (memdebug_level > 0)
			fprintf(trace_file, "*** Requested memory size stored at %p\n", mem_temp);
		*(size_t *)mem_temp = new_size;

		/* finally, point to storage we are going to hand out */
		mem_temp += sizeof(size_t);

		/* now, point to trailer bytes and do them */
		end = mem_temp + new_size;
		for (i = 0; i < FENCE_SIZE; i++)
			*end++ = 145;
	}

	if (memdebug_level > 1 && allocations != NULL) {
		char key[16];
		long temp;

		sprintf(key, "%p", mem_temp);
		if (dict_insert(allocations, key, 1, (const unsigned long)new_size, mem_temp, &temp) == NULL) {
			fprintf(trace_file, "!!! Insert of pointer tracking info failed\n");
			abort();
		}
	}

	if (memdebug_level > 0) {
		fprintf(trace_file, "--- Returning pointer of %p\n", mem_temp);
		fflush(trace_file);
	}
	return (void *)mem_temp;
}

/* check a pointer to be sure all check bytes are OK. abort if not */
/* always returns 1 if not aborting so that it can be included in  */
/* if statement boolean expressions */
int mem_validate(void *ptr)
{
	unsigned char *mem_temp = (unsigned char *)ptr;
	size_t mem_size;
	size_t i;

	/* NULL pointers are always valid */
	if (ptr == NULL)
		return 1;

	if (memdebug_level > 0)
		fprintf(trace_file, "+++ Checking %p as pointer\n", ptr);


	if (memdebug_level > 1 && allocations != NULL) {
		char key[16];
		STRING_ENTRY *se;
		long temp;

		sprintf(key, "%p", ptr);

		if ((se = dict_search(allocations, key, &temp)) == NULL) {
			fprintf(trace_file, "!!! Pointer not found in tracking info!\n");
			abort();
		}

		/* point to size bytes */
		mem_temp = (unsigned char *)ptr - sizeof(size_t);

		/* get user size */
		mem_size = *(size_t *)mem_temp;

		if (se->count == 0) {
			fprintf(trace_file, "!!! Checking pointer has been freed!\n");
			abort();
		}
		else if (se->flags != mem_size) {
			fprintf(trace_file, "!!! Stored size different from tracking size!\n");
			abort();
		}
	}

	/* check the header bytes */
	mem_temp = (unsigned char *) ptr - FENCE_SIZE;
	if (memdebug_level > 0)
		fprintf(trace_file, "+++ Real pointer at %p\n", mem_temp);


	for (i = 0; i < FENCE_SIZE - sizeof(size_t); i++)
		if (*mem_temp++ != 145) {
			if (memdebug_level > 0) {
				fprintf(trace_file, "!!! The user pointer at %p has been overwritten\n", ptr);
				fprintf(trace_file, "!!! Header offset %ld has been changed\n", i - 1);
				fflush(trace_file);
			}
			abort();
		}

	/* check size */
	i = *(size_t *)mem_temp;
	if (memdebug_level > 0)
		fprintf(trace_file, "*** Stored memory size of %ld bytes in header\n", i);


	/* now point to where trailer should be */
	mem_temp = (unsigned char *)ptr + i;
	for (i = 0; i < FENCE_SIZE; i++)
		if (*mem_temp++ != 145) {
			if (memdebug_level > 0) {
				fprintf(trace_file, "!!! The user pointer at %p has been overwritten\n", ptr);
				fprintf(trace_file, "!!! Trailer offset %ld has been changed\n", i - 1);
				fflush(trace_file);
			}
			abort();
		}
	if (memdebug_level > 0)
		fflush(trace_file);
	return 1;
}
