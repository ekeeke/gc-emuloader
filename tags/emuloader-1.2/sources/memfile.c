/*****************************************************************************
* Memory File Support Functions
*****************************************************************************/
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memfile.h"

static int memhandle = 1;

/****************************************************************************
* mem_fopen
****************************************************************************/
MEMFILE *mem_fopen(char *bufptr, u32 len)
{
    MEMFILE *p;

    p = (MEMFILE *) malloc(sizeof(MEMFILE));
    memset(p, 0, sizeof(MEMFILE));

    p->handle = memhandle++;
    p->len = len;
    p->ptr = bufptr;

    return p;
}

/****************************************************************************
* mem_fread
****************************************************************************/
int mem_fread(char *buf, int block, int size, MEMFILE * File)
{
    int reallength;

    reallength = block * size;
    if ((reallength + File->currpos) > File->len)
	reallength = File->len - File->currpos;

    if (reallength > 0) {
	memcpy(buf, File->ptr + File->currpos, reallength);
	File->currpos += reallength;
    } else
	return 0;

    return reallength;
}

/****************************************************************************
* mem_fseek
****************************************************************************/
int mem_fseek(MEMFILE * File, int where, int whence)
{
    switch (whence) {
    case SEEK_SET:
	if (where < File->len) {
	    File->currpos = where;
	    return 0;
	}
	break;

    case SEEK_CUR:
	if ((where + File->currpos) < File->len) {
	    File->currpos += where;
	    return 0;
	}
	break;

    case SEEK_END:
	if (where < File->len) {
	    File->currpos = File->len - where;
	    return 0;
	}
	break;
    }

    return 0;
}

/****************************************************************************
* mem_fclose
****************************************************************************/
void mem_fclose(MEMFILE * File)
{
    if (File != NULL)
	free(File);
}
