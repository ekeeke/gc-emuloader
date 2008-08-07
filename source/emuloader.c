/****************************************************************************
 *  EMU Loader
 *  most of the code is reused from previous Softdev's work
 *  Copyright (C) 2007  Eke-Eke
 *
 ***************************************************************************/
#include <stdio.h>
#include <gccore.h>    /*** Wrapper to include common libogc headers ***/
#include <ogcsys.h>    /*** Needed for console support ***/
#include <string.h>
#include <malloc.h>
#include <zlib.h>
  
#include "emuloader.h"

#ifdef HW_RVL
#include "vfat.h"
#include "sdio.h"
#include "elf.h"
#include "dol.h"

#else
#include "ssaram.h"
#include "sidestep.h"
#include "lzma.h"
#include "lzmasupp.h"
#endif

#define MAX_ITEMS 8

#ifndef HW_RVL
/* this calculation of DATA1 section address is based upon the memory size of current dol */
/* any changes in the sourcecode can modify this size so you have to recalculate the correct location */
/* use DOLTOOL to determine the memory occupation of the compiled dol then modify this address */
/* also modify the address in lzmaadd.c and recompile lzmaadd.exe in /pcutils */
#define DOLADDRESS 0x80296460 
#endif

#ifdef HW_RVL
void (*reload)() = (void(*)())0x90000020; /* reboot TPloader */
#else
void (*reload)() = (void(*)())0x80001800; /* reboot SDLOAD */ 
#endif


/* 2D Video Globals */
GXRModeObj *vmode;            /* Graphics Mode Object */
u32 *xfb[2] = { NULL, NULL }; /* Framebuffers */
u8 whichfb = 0;               /* Frame buffer toggle */
extern GXRModeObj TVEurgb60Hz480IntDf;

/* Compressed menu images */
static u8 *menu_list[MAX_ITEMS] =
{
  &gen_menu[0],
  &sms_menu[0],
  &snes_menu[0],
  &nes_menu[0],
  &pce_menu[0],
  &ngp_menu[0],
  &ncd_menu[0],
  &gb_menu[0]
};

static u32 sizelist[MAX_ITEMS] =
{
  GEN_COMPRESSED,
  SMS_COMPRESSED,
  SNES_COMPRESSED,
  NES_COMPRESSED,
  PCE_COMPRESSED,
  NGP_COMPRESSED,
  NCD_COMPRESSED,
  GB_COMPRESSED
};

static char name_list[MAX_ITEMS][16] = 
{
  "genplus",
  "smsplus",
  "snes9x",
  "fceugc",
  "hugo",
  "neopop",
  "redux",
  "gnuboy"
};

static s32 dol_index[MAX_ITEMS] =
{
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1,
  -1
};

static u8 nb_dols = 0;
static char backmenu[(640 * 480 * 2) + 32];
static int selection = 0;

#ifndef HW_RVL
u8 *lzmaptr;
int lzmalength=0;
u8 *lzmadata;
#else
static char dol_pathname[MAX_ITEMS][256];
static FSDIRENTRY fsfile;
static VFATFS fs;
#endif


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
      uncompress ((Bytef *) &backmenu[0], &raw_size, (Bytef *) menu_list[selection], sizelist[selection]);
      whichfb ^= 1;
      memcpy (xfb[whichfb], &backmenu, 1280 * 480);
      VIDEO_SetNextFramebuffer (xfb[whichfb]);
      VIDEO_Flush ();
      VIDEO_WaitVSync ();
      redraw = 0;
    }

    VIDEO_WaitVSync ();
    PAD_ScanPads();
	pad = PAD_ButtonsDown (0);
    x = PAD_StickX (0);
  
    if ((pad & PAD_BUTTON_LEFT) || (x < -50))
    {
      selection --;
      if (selection < 0) selection = MAX_ITEMS - 1;
      while (dol_index[selection] == -1)
      {
         selection --;
        if (selection < 0) selection = MAX_ITEMS - 1;
      }
      redraw = 1;
    }

    if ((pad & PAD_BUTTON_RIGHT) || (x > 50))
    {
      selection ++;
      if (selection > MAX_ITEMS - 1) selection = 0;
      while (dol_index[selection] == -1)
      {
        selection ++;
        if (selection > MAX_ITEMS - 1) selection = 0;
      }
      redraw = 1;
    }

    if ((pad & PAD_BUTTON_A) || (pad & PAD_BUTTON_B) ||(pad & PAD_BUTTON_START))
    {
      quit = 1;
    }
  
    if (pad & PAD_TRIGGER_Z)
    {
      reload();
    }
  }
  
  return selection;
}


static void SelectDOL ()
{
#ifndef HW_RVL
  /* This is mostly based on Softdev's code */
  CFileInStream archiveStream;
  CArchiveDatabaseEx db;
  SZ_RESULT res;
  ISzAlloc allocImp;
  ISzAlloc allocTempImp;
  CFileItem *f;
  u32 blockIndex = 0xffffffff;
  size_t offset, outSizeProcessed;
  size_t outbufferSize = 0;
  Byte *outbuffer = 0;
#else
  char *outbuffer;
  char readbuffer[2048];
  u32 offset;
  u32 blocks;
  void (*ep)();
  u32 level;
#endif  

  int num,i;

#ifndef HW_RVL
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
  res =  SzArchiveOpen(&archiveStream.InStream, &db, &allocImp, &allocTempImp);
  if (res != SZ_OK);
  {
     printf("ERROR: no valid LZMA archive found !\n");
     printf("Press A to reboot...\n");
     while(1)
     {
        if (PAD_ButtonsDown(0) & PAD_BUTTON_A) reload();
        PAD_ScanPads();
		VIDEO_WaitVSync();
     }
  }

  /* Search known dols */
  for (num = 0; num < MAX_ITEMS; num ++)
  {
    for (i = 0; i < db.Database.NumFiles; i++)
    {
      f = db.Database.Files + i;
      if (strstr(f->Name,name_list[num]) != NULL)
      {
         dol_index[num] = i;
         nb_dols ++;
         i = db.Database.NumFiles + 1;
      }
    }
  }

#else
  /* Init Front SD SLOT */
  VFAT_mount(FS_SLOTA, &fs);

  /* Open dir */
  FSDIRENTRY fsdir;
  int res = VFAT_opendir(0, &fsdir, "/emuloader");
  if (res != FS_SUCCESS)
  {
     printf("ERROR: can not open /emuloader directory !\n");
     printf("Press A to reboot...\n");
     while(1)
     {
        if (PAD_ButtonsDown(0) & PAD_BUTTON_A) reload();
        PAD_ScanPads();
		VIDEO_WaitVSync();
     }
  }

  /* Search known dols */
  while ((VFAT_readdir(&fsdir) == FS_SUCCESS) && (nb_dols < MAX_ITEMS))
  {
    for (num = 0; num < MAX_ITEMS; num ++)
    {
      if ((dol_index[num] == -1) && (strstr(fsdir.longname,name_list[num]) != NULL))
      {
        dol_index[num] = fsdir.fsize;
        sprintf(dol_pathname[num],"/emuloader/%s", fsdir.longname);
        nb_dols ++;
        num = MAX_ITEMS;
      }
    }
  }
  VFAT_closedir(&fsdir);
#endif

  if (nb_dols == 0)
  {
     printf("ERROR: no known files found !\n");
     printf("Press A to reboot...\n");
     while(1)
     {
        if (PAD_ButtonsDown(0) & PAD_BUTTON_A) reload();
        PAD_ScanPads();
		VIDEO_WaitVSync();
     }
  }

  /* find first dol entry */
  selection = 0;
  while (dol_index[selection] == -1) selection ++;

  /* let user choose dol */
  int quit = 0;
  while (quit == 0)
  {
    num = DoMenu();

#ifndef HW_RVL
    /* extract selected DOL from archive */
    res = SzExtract(&archiveStream.InStream, &db, dol_index[num],
                    &blockIndex, &outbuffer, &outbufferSize,
                    &offset, &outSizeProcessed, &allocImp,
                    &allocTempImp);

    f = db.Database.Files + dol_index[num];
    if ((res == SZ_OK) && (outSizeProcessed == f->Size))
    {
      quit = 1;
    }

#else
    /* read selected dol from sdcard */
    res = VFAT_fopen(0, &fsfile, dol_pathname[num], FS_READ);
    if (res == FS_SUCCESS )
    {
       /* allocate buffer */
       outbuffer = memalign(32, dol_index[num]);

       /* read 2048 bytes sectors */
       blocks = dol_index[num] >> 11;
       offset = 0;
       for (i = 0; i < blocks; i++)
       {
         VFAT_fread(&fsfile, readbuffer, 2048);
         memcpy (outbuffer + offset, readbuffer, 2048);
         offset += 2048;
       }

       /* final read */
       if (dol_index[num] % 2048)
       {
         i =  dol_index[num] % 2048;
         VFAT_fread(&fsfile, readbuffer, i);
         memcpy (outbuffer + offset, readbuffer, i);
       }

       VFAT_fclose(&fsfile);
       offset = 0;
       quit = 1;
    }
#endif
  }

  /* load DOL */
#ifndef HW_RVL
  DOLtoARAM(outbuffer + offset);

#else

  /* remove any pending buttons */
  while(PAD_ButtonsHeld(0))
  {
     PAD_ScanPads();
	 VIDEO_WaitVSync();
  }

  if (strstr(dol_pathname[num], ".elf") != NULL)
  {
    ep = (void(*)())load_elf_image(outbuffer);
  }
  else if (strstr(dol_pathname[num], ".dol") != NULL)
  {
	ep = (void(*)())load_dol_image(outbuffer);
  }
  else
  {
     printf("ERROR: file format not supported !\n");
     printf("Press A to reboot...\n");
     while(1)
     {
        if (PAD_ButtonsDown(0) & PAD_BUTTON_A) reload();
        PAD_ScanPads();
		VIDEO_WaitVSync();
     }
  }
  
  /* shutdown liboc */
  sd_deinit();
  SYS_ResetSystem(SYS_SHUTDOWN, 0, 0); 
  
  /* launch the application */
  ep();
  
  /* should never return ! */
  while(1) VIDEO_WaitVSync();
#endif
}

/****************************************************************************
* Initialise Video
*
* Before doing anything in libogc, it's recommended to configure a video
* output.
****************************************************************************/
void Initialize ()
{   
  /* Generic libOGC initialization */
  VIDEO_Init (); 
  PAD_Init ();

#ifndef HW_RVL
  /* Copy linked LZMA file to ARAM */
  /* lzmaptr & lzmalength are defined in 7zipfile.S */
  AR_Init(NULL, 0);
  lzmaptr = (u8 *) DOLADDRESS;
  if (memcmp(lzmaptr, "LZMA SDK 4.43   DOLBOX 1.0", 26) == 0)
  {
    memcpy(&lzmalength, lzmaptr + 28, 4);
    ARAMPut(lzmaptr + 32, (char *) 0x8000, lzmalength);
  }
#endif

  switch (VIDEO_GetCurrentTvMode())
  {
    case VI_PAL:
      vmode = &TVPal528IntDf;
	  
	  /* force 480 lines output (black borders) */
	  vmode->xfbHeight = 480;
	  vmode->viYOrigin = (VI_MAX_HEIGHT_PAL - 480)/2;
	  vmode->viHeight = 480;
      break;

#ifdef FORCE_EURGB60
    default:
      vmode = &TVEurgb60Hz480IntDf;
      break;

#else
	case VI_MPAL:
      vmode = &TVMpal480IntDf;
      break;

	default:
      vmode = &TVNtsc480IntDf;
      break;
#endif
  }

  /* Generic libOGC configuration */
  VIDEO_Configure (vmode);
  xfb[0] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
  xfb[1] = (u32 *) MEM_K0_TO_K1 (SYS_AllocateFramebuffer (vmode));
  console_init (xfb[0], 20, 64, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);
  VIDEO_ClearFrameBuffer (vmode, xfb[0], COLOR_BLACK);
  VIDEO_ClearFrameBuffer (vmode, xfb[1], COLOR_BLACK);
  VIDEO_SetNextFramebuffer (xfb[0]);
  VIDEO_SetBlack (0);
  VIDEO_Flush ();
  VIDEO_WaitVSync ();
  VIDEO_WaitVSync ();
}
 
int main(int argc, char **argv)
{
  Initialize ();

  SelectDOL();
  while (1);
  return 0;
}
