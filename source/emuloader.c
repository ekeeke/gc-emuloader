/****************************************************************************
 *  GC/Wii EmuLoader
 *
 *  Most of the code has been taken from various people original's work,
 *  please look at the readme for the full credits.
 *
 *  The graphical interface has been coded by me.
 *  Compressed Gamecube format images (YCbYCr) have been generated from
 *  24bpp bitmaps using a dedicated converter program.
 *  Contact me if you are interested in using it for your own project.
 *
 *  2007,2008  Eke-Eke
 *
 ***************************************************************************/
#include <stdio.h>
#include <gccore.h>
#include <ogcsys.h>
#include <string.h>
#include <malloc.h>
#include <zlib.h>

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

#include "emuloader.h"

#define MAX_ITEMS 8

#ifndef HW_RVL
/* this calculation of DATA1 section address is based upon the memory size of current dol */
/* any changes in the sourcecode can modify this size so you have to recalculate the correct location */
/* use DOLTOOL to determine the memory occupation of the compiled dol then modify this address */
/* also modify the address in lzmaadd.c and recompile lzmaadd.exe in /pcutils */
#define DOLADDRESS 0x80180000
 
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

/* compressed pictures */
static const u8 *menu_list[MAX_ITEMS] =
{
  &image_gen[0],
  &image_sms[0],
  &image_snes[0],
  &image_nes[0],
  &image_pce[0],
  &image_ngp[0],
  &image_ncd[0],
  &image_gb[0],
};

/* compressed sizes */
static const u32 sizelist[MAX_ITEMS] =
{
  gen_COMPRESSED,
  sms_COMPRESSED,
  snes_COMPRESSED,
  nes_COMPRESSED,
  pce_COMPRESSED,
  ngp_COMPRESSED,
  ncd_COMPRESSED,
  gb_COMPRESSED,
};

/* emulators filename (2 possible names) */
static const char *name_list[MAX_ITEMS][2] = 
{
  {"genplus","GENPLUS"},
  {"smsplus","SMSPLUS"},
  {"snes","SNES"},
  {"fceu","FCEU"},
  {"hugo","HUGO"},
  {"neopop","NEOPOP"},
  {"redux","REDUX"},
  {"gnuboy", "RIN"}
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

#if 0
/*** GX ***/
#define TEX_WIDTH 640
#define TEX_HEIGHT 480
#define DEFAULT_FIFO_SIZE 256 * 1024
#define HASPECT 320
#define VASPECT 240

static u8 gp_fifo[DEFAULT_FIFO_SIZE] ATTRIBUTE_ALIGN (32);
static u8 texturemem[TEX_WIDTH * (TEX_HEIGHT + 8) * 2] ATTRIBUTE_ALIGN (32);
static u16 draw_buffer[640 * 480] ATTRIBUTE_ALIGN (32);

GXTexObj texobj;
static Mtx view;

typedef struct tagcamera
{
  Vector pos;
  Vector up;
  Vector view;
} camera;

/*** Square Matrix
     This structure controls the size of the image on the screen.
	 Think of the output as a -80 x 80 by -60 x 60 graph.
***/
s16 square[] ATTRIBUTE_ALIGN (32) =
{
  /*
   * X,   Y,  Z
   * Values set are for roughly 4:3 aspect
   */
	-HASPECT, VASPECT, 0,	// 0
	HASPECT, VASPECT, 0,	// 1
	HASPECT, -VASPECT, 0,	// 2
	-HASPECT, -VASPECT, 0,	// 3
};

static camera cam = { {0.0F, 0.0F, -100.0F},
{0.0F, -1.0F, 0.0F},
{0.0F, 0.0F, 0.0F}
};

/* init rendering */
/* should be called each time you change quad aspect ratio */
void draw_init (void)
{
  /* Clear all Vertex params */
  GX_ClearVtxDesc ();

  /* Set Position Params (set quad aspect ratio) */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_S16, 0);
  GX_SetVtxDesc (GX_VA_POS, GX_INDEX8);
  GX_SetArray (GX_VA_POS, square, 3 * sizeof (s16));

  /* Set Tex Coord Params */
  GX_SetVtxAttrFmt (GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  GX_SetVtxDesc (GX_VA_TEX0, GX_DIRECT);
  GX_SetTevOp (GX_TEVSTAGE0, GX_REPLACE);
  GX_SetTevOrder (GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
  GX_SetNumTexGens (1);
  GX_SetNumChans(0);

  /** Set Modelview **/
  memset (&view, 0, sizeof (Mtx));
  guLookAt(view, &cam.pos, &cam.up, &cam.view);
  GX_LoadPosMtxImm (view, GX_PNMTX0);
}

/* vertex rendering */
static void draw_vert (u8 pos, f32 s, f32 t)
{
  GX_Position1x8 (pos);
  GX_TexCoord2f32 (s, t);
}

/* textured quad rendering */
static void draw_square ()
{
  GX_Begin (GX_QUADS, GX_VTXFMT0, 4);
  draw_vert (3, 0.0, 0.0);
  draw_vert (2, 1.0, 0.0);
  draw_vert (1, 1.0, 1.0);
  draw_vert (0, 0.0, 1.0);
  GX_End ();
}

/* initialize GX rendering */
static void StartGX (void)
{
  Mtx p;
  GXColor gxbackground = { 0, 0, 0, 0xff };

  /*** Clear out FIFO area ***/
  memset (&gp_fifo, 0, DEFAULT_FIFO_SIZE);

  /*** Initialise GX ***/
  GX_Init (&gp_fifo, DEFAULT_FIFO_SIZE);
  GX_SetCopyClear (gxbackground, 0x00ffffff);
  GX_SetViewport (0.0F, 0.0F, vmode->fbWidth, vmode->efbHeight, 0.0F, 1.0F);
  GX_SetScissor (0, 0, vmode->fbWidth, vmode->efbHeight);
  f32 yScale = GX_GetYScaleFactor(vmode->efbHeight, vmode->xfbHeight);
  u16 xfbHeight = GX_SetDispCopyYScale (yScale);
  GX_SetDispCopySrc (0, 0, vmode->fbWidth, vmode->efbHeight);
  GX_SetDispCopyDst (vmode->fbWidth, xfbHeight);
  GX_SetCopyFilter (vmode->aa, vmode->sample_pattern, GX_TRUE, vmode->vfilter);
  GX_SetFieldMode (vmode->field_rendering, ((vmode->viHeight == 2 * vmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
  GX_SetPixelFmt (GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  GX_SetCullMode (GX_CULL_NONE);
  GX_SetDispCopyGamma (GX_GM_1_0);
  GX_SetZMode(GX_FALSE, GX_ALWAYS, GX_TRUE);
  GX_SetColorUpdate (GX_TRUE);
  guOrtho(p, vmode->efbHeight/2, -(vmode->efbHeight/2), -(vmode->fbWidth/2), vmode->fbWidth/2, 100, 1000);
  GX_LoadProjectionMtx (p, GX_ORTHOGRAPHIC);

  /*** Copy EFB -> XFB ***/
  GX_CopyDisp (xfb[whichfb ^ 1], GX_TRUE);
  GX_Flush ();

  /*** Initialize texture data ***/
  memset (texturemem, 0, TEX_WIDTH * TEX_HEIGHT * 2);
  GX_InitTexObj (&texobj, texturemem, 640, 480, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
}

static void put_image(u16 *image, int width, int height, int xpos, int ypos)
{
  int i,j;
  int offset = 0;
  for (i=ypos; i<ypos+height; i++)
  {
    for (j=xpos; j<xpos+width; j++)
    {
      draw_buffer[i*640 + j] = image[offset];
      offset ++;
    }
  }
}

static void render_screen()
{
  /* fill texture data */
  long long int *dst = (long long int *)(texturemem);
  long long int *src1 = (long long int *)(draw_buffer);
  long long int *src2 = src1 + 160;  /* line n+1 */
  long long int *src3 = src2 + 160;  /* line n+2 */
  long long int *src4 = src3 + 160;  /* line n+3 */

  GX_InvVtxCache ();
  GX_InvalidateTexAll ();
  
  /* update texture data */
  int h, w;
  for (h = 0; h < 480; h += 4)
  {
    for (w = 0; w < 160; w++ )
	  {
      *dst++ = *src1++;
      *dst++ = *src2++;
      *dst++ = *src3++;
      *dst++ = *src4++;
	  }

    src1 += 480;
    src2 += 480;
    src3 += 480;
    src4 += 480;
  }

  /* load texture into GX */
  DCFlushRange (texturemem, 640 * 480 * 2);
  GX_LoadTexObj (&texobj, GX_TEXMAP0);
  
  /* render textured quad */
  draw_square ();
  GX_DrawDone ();

  /* switch external framebuffers then copy EFB to XFB */
  whichfb ^= 1;
  GX_CopyDisp (xfb[whichfb], GX_TRUE);
  GX_Flush ();

  /* set next XFB */
  VIDEO_SetNextFramebuffer (xfb[whichfb]);
  VIDEO_Flush ();
}
#endif

/* buffer to store pictures */
static u8 raw_data[640 * 480 * 2] ATTRIBUTE_ALIGN (32);

/* arrow type flag */
static u8 arrow = 0;

/* basically, we copy decompressed data into the framebuffer */
void drawimage(int xoffset, int yoffset, int width, int height)
{
  int rows, cols;
  int fboffset = (xoffset /2 ) + (yoffset * 320);
  int i = 0;
  for (rows = 0; rows < height; rows++)
  {
    for (cols = 0; cols < (width /2); cols++)
    {
      xfb[whichfb][fboffset + cols] = (raw_data[i] << 24) | (raw_data[i+1] << 16) | (raw_data[i+2] << 8) | raw_data[i+3];
      i+=4;
    }
    fboffset += 320;
  }
}	

/* selection menu */
static int DoMenu()
{
  int quit = 0;
  int redraw = 1;
  short pad_held,pad_up;
  signed char x;
  unsigned long raw_size;
  
  while (quit == 0)
  {
    if (redraw == 1)
    {
      whichfb ^= 1;
      raw_size = 614400;
      uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) &image_background[0], 8668);
      memcpy (xfb[whichfb], &raw_data, 1280 * 480);

      uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) menu_list[selection], sizelist[selection]);
      drawimage(196,109,248,262);

      if (arrow & 1)
      {
        uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) &image_left_arrow_2[0], 5891);
        drawimage(38,203,76,76);
      }
      else
      {
        uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) &image_left_arrow_1[0], 3110);
        drawimage(52,217,48,48);
      }

      if (arrow & 2)
      {
        uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) &image_right_arrow_2[0], 5961);
        drawimage(520,203,76,76);
      }
      else
      {
        uncompress ((Bytef *) &raw_data[0], &raw_size, (Bytef *) &image_right_arrow_1[0], 3033);
        drawimage(540,217,48,48);
      }

      VIDEO_SetNextFramebuffer (xfb[whichfb]);
      VIDEO_Flush ();
      VIDEO_WaitVSync ();
      redraw = 0;
    }

    VIDEO_WaitVSync ();
    PAD_ScanPads();

	  pad_held = PAD_ButtonsHeld (0);
	  pad_up = PAD_ButtonsUp (0);
    x = PAD_StickX (0);
  
    if (pad_held & PAD_BUTTON_LEFT)
    {
      redraw = 1;
      arrow = 1;
    }

    if ((pad_up & PAD_BUTTON_LEFT) || (x < -50))
    {
      arrow = 0;
      selection --;
      if (selection < 0) selection = MAX_ITEMS - 1;
      while (dol_index[selection] == -1)
      {
         selection --;
        if (selection < 0) selection = MAX_ITEMS - 1;
      }
      redraw = 1;
    }

    if (pad_held & PAD_BUTTON_RIGHT)
    {
      redraw = 1;
      arrow = 2;
    }

    if ((pad_up & PAD_BUTTON_RIGHT) || (x > 50))
    {
      arrow = 0;
      selection ++;
      if (selection > MAX_ITEMS - 1) selection = 0;
      while (dol_index[selection] == -1)
      {
        selection ++;
        if (selection > MAX_ITEMS - 1) selection = 0;
      }
      redraw = 1;
    }

    if ((pad_held & PAD_BUTTON_A) || (pad_held & PAD_BUTTON_B) ||(pad_held & PAD_BUTTON_START))
    {
      quit = 1;
    }
  
    if (pad_held & PAD_TRIGGER_Z)
    {
      reload();
    }
  }
  
  return selection;
}


static void SelectDOL ()
{
  int num, i;

  while(PAD_ButtonsHeld(0))
  {
    PAD_ScanPads();
    VIDEO_WaitVSync();
  }

#ifndef HW_RVL
  /* LZMA support (mostly based on Softdev's original code) */
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

  /* Allocate and retrieve the LZMA file */
  if (lzmalength == 0) return;
  if (lzmalength & 0x1f) lzmalength = (lzmalength & ~0x1f) + 32;
  lzmadata = (u8 *) memalign(32, lzmalength);
  ARAMFetch((char *)lzmadata, (char *) 0x8000, lzmalength);

  /* Initialise LZMA */
  archiveStream.File = mem_fopen((char *)lzmadata, lzmalength);

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
      if ((strstr(f->Name,name_list[num][0]) != NULL) ||
          (strstr(f->Name,name_list[num][1]) != NULL))
      {
         dol_index[num] = i;
         nb_dols ++;
         i = db.Database.NumFiles + 1;
      }
    }
  }

#else
  
  /* Front SD slot support */
  char *outbuffer;
  char readbuffer[2048];
  u32 offset;
  u32 blocks;
  void (*ep)();

  /* initialize SDCARD */
  VFAT_mount(FS_SLOTA, &fs);

  /* browse directory */
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

  /* search known dols */
  while ((VFAT_readdir(&fsdir) == FS_SUCCESS) && (nb_dols < MAX_ITEMS))
  {
    for (num = 0; num < MAX_ITEMS; num ++)
    {
      if ((dol_index[num] == -1) && 
          ((strstr((const char *)fsdir.longname,name_list[num][0]) != NULL) ||
           (strstr((const char *)fsdir.longname,name_list[num][1]) != NULL)))
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
        i = dol_index[num] % 2048;
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
  DOLtoARAM((char *)(outbuffer + offset));

#else
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
    while(PAD_ButtonsHeld(0))
    {
      PAD_ScanPads();
      VIDEO_WaitVSync();
    }

    VIDEO_ClearFrameBuffer(vmode, xfb[whichfb], COLOR_BLACK);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    printf("ERROR: file format not supported !\n");
    printf("Press Z to reboot...\n");
    while(1)
    {
      PAD_ScanPads();
      if (PAD_ButtonsDown(0) & PAD_TRIGGER_Z) reload();
		  VIDEO_WaitVSync();
    }
  }
  
  /* shutdown libogc */
  sd_deinit();
  SYS_ResetSystem(SYS_SHUTDOWN, 0, 0); 
  
  /* launch the application */
  ep();
#endif
}

/****************************************************************************
* Initialise Video
*
* Before doing anything in libogc, it's recommended to configure a video
* output.
****************************************************************************/
#ifdef FORCE_EURGB60
extern GXRModeObj TVEurgb60Hz480IntDf;
#endif

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
    ARAMPut((char *)(lzmaptr + 32), (char *) 0x8000, lzmalength);
  }
#endif

  /* Configure TV mode */
  switch (VIDEO_GetCurrentTvMode())
  {
    case VI_PAL:
      /* force 480 lines output (black borders) */
      vmode = &TVPal528IntDf;
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
  console_init (xfb[0], 20, 128, vmode->fbWidth, vmode->xfbHeight, vmode->fbWidth * 2);
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
  while (1); /* should never return here ! */
  return 0;
}
