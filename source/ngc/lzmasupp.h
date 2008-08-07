/****************************************************************************
* LZMA SDK 4.43 - Nintendo GameCube Memory File Wrapper
*
* Module to provide in-memory decompression.
****************************************************************************/
#ifndef __LZMA443SUPP__
#define __LZMA443SUPP__

#include "lzma.h"
#include "memfile.h"

typedef struct {
    ISzInStream InStream;
    MEMFILE *File;
} CFileInStream;

SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize,
			size_t * processedSize);
SZ_RESULT SzFileSeekImp(void *object, CFileSize pos);

#endif
