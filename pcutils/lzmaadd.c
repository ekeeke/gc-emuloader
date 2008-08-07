#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOLADDRESS 0x80180000

#define MAXTEXTSECTION 7
#define MAXDATASECTION 11

typedef struct {
    unsigned int textOffset[MAXTEXTSECTION];
    unsigned int dataOffset[MAXDATASECTION];

    unsigned int textAddress[MAXTEXTSECTION];
    unsigned int dataAddress[MAXDATASECTION];

    unsigned int textLength[MAXTEXTSECTION];
    unsigned int dataLength[MAXDATASECTION];

    unsigned int bssAddress;
    unsigned int bssLength;

    unsigned int entryPoint;
    unsigned int unused[MAXTEXTSECTION];
} DOLHEADER;

unsigned int FLIP32( unsigned int a )
{
	unsigned int b;

	b = ( a & 0xff000000 ) >> 24;
	b |= ( a & 0xff0000 ) >> 8;
	b |= ( a & 0xff00 ) << 8;
	b |= ( a & 0xff ) << 24;

	return b;
}

int main( int argc, char *argv[] )
{
	FILE *infile;
	FILE *outfile;
	char *dol;
	int len;
	int zlen;
	DOLHEADER dolhdr;
	char nheader[32];
	int t;

	if ( argc != 4 )
	{
		puts("Usage : lzmaadd master.dol comp.7z output.dol");
		return 0;
	}

	infile = fopen(argv[1], "rb");
	if ( infile == NULL )
		return 0;

	fseek(infile, 0, SEEK_END);
	len = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	
	if ( len & 0x1f )
		len = ( len & ~0x1f ) + 32;

	dol = malloc(len);
	memset(dol, 0, len);
	fread(dol, 1, len, infile);
	fclose(infile);

	memcpy(&dolhdr, dol, 256);

	outfile = fopen(argv[3],"wb");
	fwrite(dol, 1, len, outfile);
	free(dol);

	infile = fopen(argv[2],"rb");
	fseek(infile, 0, SEEK_END);
	zlen = ftell(infile);
	fseek(infile, 0, SEEK_SET);
	
	if ( zlen & 0x1f )
		zlen = ( zlen & ~0x1f ) + 32;

	t = htonl(zlen);
	zlen += 32;

	dol = malloc(zlen);
	memset(dol, 0, zlen);
	strcpy(dol,"LZMA SDK 4.43   ");
	strcat(dol,"DOLBOX 1.0");
	memcpy(dol + 28, &t, 4);
	fread(dol + 32, 1, zlen, infile);
	fclose(infile);

	fwrite(dol, 1, zlen, outfile);
	free(dol);

	fseek(outfile, 0, SEEK_SET);

	dolhdr.dataOffset[1] = FLIP32(len);
	dolhdr.dataAddress[1] = FLIP32(DOLADDRESS);
	dolhdr.dataLength[1] = FLIP32(zlen);

	fwrite(&dolhdr, 1, 256, outfile);

	fclose(outfile);

	return 0;
}

