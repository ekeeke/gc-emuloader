/****************************************************************************
 *  EMU Loader
 *  most of the code is reused from previous Softdev's work
 *  Copyright (C) 2007  Eke-Eke
 *
 ***************************************************************************/
#include <stdio.h>
#include <gccore.h>		/*** Wrapper to include common libogc headers ***/
#include <ogcsys.h>		/*** Needed for console support ***/
#include <string.h>
#include <malloc.h>
#include <zlib.h>
  
#include "emuloader.h"
#include "ssaram.h"
#include "lzma.h"
#include "lzmasupp.h"
#include "sidestep.h"

#define MAX_ITEMS 8

/* this calculation of DATA1 section address is based upon the memory size of current dol */
/* any changes in the sourcecode can modify this size so you have to recalculate the correct location */
/* use DOLTOOL to determine the memory occupation of the compiled dol then modify this address */
/* also modify the address in lzmaadd.c and recompile lzmaadd.exe in /pcutils */
#define DOLADDRESS 0x80296460 

/* 2D Video Globals */
GXRModeObj *vmode;            /* Graphics Mode Object */
u32 *xfb[2] = { NULL, NULL }; /* Framebuffers */
int whichfb = 0;              /* Frame buffer toggle */

/* Compressed menu images */
unsigned char *menu_list[MAX_ITEMS] = {
	&gen_menu[0],
    &sms_menu[0],
    &snes_menu[0],
    &nes_menu[0],
    &pce_menu[0],
    &ngp_menu[0],
    &ncd_menu[0],
	&gb_menu[0]
};

unsigned long sizelist[MAX_ITEMS] = {
	GEN_COMPRESSED,
	SMS_COMPRESSED,
	SNES_COMPRESSED,
	NES_COMPRESSED,
	PCE_COMPRESSED,
	NGP_COMPRESSED,
	NCD_COMPRESSED,
	GB_COMPRESSED
};

unsigned char name_list[MAX_ITEMS][16] = {
	"genplus",
    "smsplus",
    "snes9x",
    "fceugc",
    "hugo",
    "neopop",
    "redux",
	"gnuboy"
};

char backmenu[(640 * 480 * 2) + 32];
int selection = 0;
u8 *lzmaptr;
int lzmalength=0;
u8 *lzmadata;

static int DoMenu()
{
  int quit = 0;
  int redraw = 1;
  short pad;
  unsigned long raw_size;
  signed char x;

  while (quit == 0)
  {
	  if (redraw == 1)
	  {
		  raw_size = BACK_RAW;
		  uncompress ((char *) &backmenu[0], &raw_size, (char *) menu_list[selection], sizelist[selection]);
		  whichfb ^= 1;
          memcpy (xfb[whichfb], &backmenu, 1280 * 480);
		  VIDEO_SetNextFramebuffer (xfb[whichfb]);
          VIDEO_Flush ();
          VIDEO_WaitVSync ();
		  redraw = 0;
	  }

	  pad = PAD_ButtonsDown (0);
	  x = PAD_StickX (0);
  
	  if ((pad & PAD_BUTTON_LEFT) || (x < -65))
	  {
	      selection --;
	      if (selection < 0) selection = MAX_ITEMS - 1;
	      redraw = 1;
      }

      if ((pad & PAD_BUTTON_RIGHT) || (x > 65))
      {
 	      selection ++;
	      if (selection > MAX_ITEMS - 1) selection = 0;
	      redraw = 1;
	  }

	  if ((pad & PAD_BUTTON_A) || (pad & PAD_BUTTON_B) ||(pad & PAD_BUTTON_START))
	  {
	    quit = 1;
	  }
  }
  return selection;
}
	
static void SelectDOL ()
{ 
  /* This is mostly based on Softdev's code */
  CFileInStream archiveStream;
  CArchiveDatabaseEx db;
  SZ_RESULT res;
  ISzAlloc allocImp;
  ISzAlloc allocTempImp;
  CFileItem *f;
  u32 blockIndex = 0xffffffff;
  Byte *outbuffer = 0;
  size_t outbufferSize = 0;
  size_t offset, outSizeProcessed;
  int num,i;
  int quit = 0;
  
  /* Allocate and retrieve the LZMA file */
  if (lzmalength == 0) return;
  if (lzmalength & 0x1f) lzmalength = (lzmalength & ~0x1f) + 32;
  lzmadata = (u8 *) memalign(32, lzmalength);
  ARAMFetch(lzmadata, (char *) 0x8000, lzmalength);

  /* Initialise LZMA */
  archiveStream.File = mem_fopen(lzmadata, lzmalength);

  /* Set I/O functions */
  archiveStream.InStream.Read = SzFileReadImp;
  archiveStream.InStream.Seek = SzFileSeekImp;

  /* Memory allocs etc */
  allocImp.Alloc = SzAlloc;
  allocImp.Free = SzFree;
  allocTempImp.Alloc = SzAllocTemp;
  allocTempImp.Free = SzFreeTemp;

  /* Initialise CRC */
  InitCrcTable();
  SzArDbExInit(&db);

  /* Open the archive */
  res =	SzArchiveOpen(&archiveStream.InStream, &db, &allocImp, &allocTempImp);
  if (res != SZ_OK) return;

  while (quit == 0)
  {
	  /* Choose a DOL */
      num = DoMenu();

      /* extract selected DOL from archive */
      for (i = 0; i < db.Database.NumFiles; i++)
      {
	    f = db.Database.Files + i;

	    /* searching for the dol entry */
		/* the name of your dols MUST contain the known strings */
		if (strstr(f->Name,name_list[num]) != NULL)
	    {
		  res = SzExtract(&archiveStream.InStream, &db, i,
		                  &blockIndex, &outbuffer, &outbufferSize,
			              &offset, &outSizeProcessed, &allocImp,
			              &allocTempImp);

		  /* if everything's OK, load the DOL iinto ARAM */
		  if ((res == SZ_OK) && (outSizeProcessed == f->Size)) DOLtoARAM(outbuffer + offset);

		  /* exit */
		  i = db.Database.NumFiles + 1;
          quit = 1;
	    }
      }
  }
}

/****************************************************************************
* Initialise Video
*
* Before doing anything in libogc, it's recommended to configure a video
* output.
****************************************************************************/
static void Initialise (void)
{   	   
  /* Generic libOGC initialization */
  VIDEO_Init (); 
  PAD_Init ();

  /* Copy linked LZMA file to ARAM */
  /* lzmaptr & lzmalength are defined in 7zipfile.S */
  AR_Init(NULL, 0);
  lzmaptr = (u8 *) DOLADDRESS;
  if (memcmp(lzmaptr, "LZMA SDK 4.43   DOLBOX 1.0", 26) == 0)
  {
	  memcpy(&lzmalength, lzmaptr + 28, 4);
	  ARAMPut(lzmaptr + 32, (char *) 0x8000, lzmalength);
  }

  /* VIDEO Mode */
  switch (VIDEO_GetCurrentTvMode ())
  {
    case VI_NTSC:
      vmode = &TVNtsc480IntDf;
      break;
    case VI_PAL:
      vmode = &TVPal528IntDf;
      break;
    case VI_MPAL:
      vmode = &TVMpal480IntDf;
      break;
    default:
      vmode = &TVNtsc480IntDf;
      break;
  }
 
  /* Generic libOGC configuration */
  VIDEO_Configure (vmode);
  xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
  xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
  console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);
  VIDEO_SetNextFramebuffer (xfb[0]);
  VIDEO_SetPostRetraceCallback (PAD_ScanPads);
  VIDEO_SetBlack (0);
  VIDEO_Flush ();
  VIDEO_WaitVSync ();
  if (vmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync ();
}
 
int main ()
{
  Initialise ();
  SelectDOL();
  while (1);
  return 0;
}
