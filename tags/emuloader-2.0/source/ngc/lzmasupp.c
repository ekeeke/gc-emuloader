/****************************************************************************
* LZMA SDK 4.43 - Nintendo GameCube Memory File Wrapper
*
* Module to provide in-memory decompression.
****************************************************************************/
#ifndef HW_RVL
#include <gccore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzma.h"
#include "lzmasupp.h"

#define kBufferSize ( 1 << 12 )
static Byte g_buffer[kBufferSize];
/****************************************************************************
* SzFileReadImp
*
* Populate the global buffer with data
****************************************************************************/
SZ_RESULT SzFileReadImp(void *object, void **buffer, size_t maxRequiredSize,
			size_t * processedSize)
{
    CFileInStream *s = (CFileInStream *) object;
    size_t processedSizeLoc;

    if (maxRequiredSize > kBufferSize)
	maxRequiredSize = kBufferSize;

    processedSizeLoc =
	mem_fread((char *) g_buffer, 1, maxRequiredSize, s->File);
    *buffer = &g_buffer[0];

    if (processedSizeLoc != 0)
	*processedSize = processedSizeLoc;

    return SZ_OK;
}

/****************************************************************************
* SzFileSeekImp
*
* General fseek
****************************************************************************/
SZ_RESULT SzFileSeekImp(void *object, CFileSize pos)
{
    CFileInStream *s = (CFileInStream *) object;

    int res = mem_fseek(s->File, pos, SEEK_SET);

    if (res == 0)
	return SZ_OK;

    return SZE_FAIL;
}
#endif
