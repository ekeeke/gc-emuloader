/*****************************************************************************
* Memory File Support Functions
*****************************************************************************/
#ifndef __MEMFILE__
#define __MEMFILE__

typedef struct {
    int handle;
    char *ptr;
    int currpos;
    int len;
} MEMFILE;

MEMFILE *mem_fopen(char *bufptr, u32 len);
int mem_fread(char *buf, int block, int size, MEMFILE * File);
int mem_fseek(MEMFILE * File, int where, int whence);
void mem_fclose(MEMFILE * File);

#endif
