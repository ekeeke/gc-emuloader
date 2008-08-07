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
	DOLHEADER dolhdr;
	char nheader[1024];
	char *p;

	if ( argc != 2 )
	{
		puts("Usage : lzmaex master.dol"); 
		return 0;
	}

	infile = fopen(argv[1], "rb");
	if ( infile == NULL )
		return 0;

	/*** Read dol header ***/
	fread(&dolhdr, 1, 256, infile);
	
	if ( dolhdr.dataOffset[1] && dolhdr.dataLength[1] )
	{
		dolhdr.dataOffset[1] = FLIP32(dolhdr.dataOffset[1]);
		dolhdr.dataLength[1] = FLIP32(dolhdr.dataLength[1]);

		dol = malloc(dolhdr.dataLength[1]);

		fseek(infile, dolhdr.dataOffset[1], SEEK_SET);
		fread(dol, 1, dolhdr.dataLength[1], infile);
		fclose(infile);
		
		strcpy(nheader,argv[1]);
		p = strrchr(nheader,'.');
		if ( p == NULL )
			strcat(nheader,".7z");
		else
			strcpy(p,".7z");

		outfile = fopen(nheader, "wb");
		fwrite(dol, 1, dolhdr.dataLength[1], outfile);
		fclose(outfile);

		printf("Output written to %s\n", nheader);

		free(dol);
	}
	else
		printf("No data block!\n");

	return 0;
}

