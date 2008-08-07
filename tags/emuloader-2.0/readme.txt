¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤
 
                                     EmuLoader
                                    Version 2.0   
 
¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤
 

-=[ Explanation ]=-
 
This is a program that enables you to load & run the following emulators on your Nintendo Gamecube/Wii:

   - Genesis Plus (Sega Megadrive)
   - SMS Plus     (Sega Master System & Game Gear)
   - FCEuGC       (Nintendo Entertainment System)
   - GnuboyGX     (Gameboy & Gameboy Color)
   - Snes9xGX     (Super Nintendo)
   - HugoGC       (NEC Pc-Engine)
   - NeoPop       (SNK NeoGeo Pocket)
   - Neo-CD Redux (SNK NeoGeo CD)

These emulators are NOT provided with this release.
Last versions of emulator binaries (dol), sourcecode, documentation & original credits 
can be found on Tehskeen's development forum: http://www.tehskeen.com/

This project is now splitted in two separated applications:

1) emuloader.dol is running in GC mode, it needs to be packed with a 7zip archive containing
the emulators you want to use, see the [GAMECUBE Version] section below and follow the installation steps.

2) emuloader_wii.elf, emuloader_wii_pal60.elf are running in Wii mode. You can directly boot
the appropriate version with your favorite elf loader. DOLS version are also provided. See the
[WII] section below to know how to use the application.


-===========================================================================================-
-= WII Version                                                                            =-
-===========================================================================================-

This is just another basic Front SD elf/dol loader that looks for known emulators.
To use it, simply follow the steps below:

1/ create a directory at the root the SDCARD (FAT16 format only) and named it "emuloader" (without the quotes)
2/ place any supported .dol or .elf emulators you want in this directory

Do NOT modify the original emulator filename, the loader is looking for specific strings
inside filenames in order to detect which emulator is present or not:

   - Genesis Plus ---> genplus
   - SMS Plus     ---> smsplus
   - FCEuGC       ---> fceugc
   - GnuboyGX     ---> gnuboy
   - Snes9xGX     ---> snes9x
   - HugoGC       ---> hugo

Obviously, only use "wii mode" versions of these emulators, GC versions won't work properly in Wii mode !
 
3/ launch emuloader_wii*.elf or.dol with the method of your choice (for example, using TP Hack)
In the menu, the following keys are used (Note that you still need a Gamecube controller):

  . Button A: lauch the selected emulator
  . Analog Stick, Left/right button: select another emulator
  . Z Trigger: Reboot TP loader

NOTE: if you have a PAL Wii, you will have to use emuloader_wii_pal60.elf


Credits:
--------
. SD Front access code by _svpe, marcan, bushing
. libOGC by shagkur& wntrmute 
. DOL loading code by shagkur
. ELF loading code by William L. Pitts
. VFAT library by softdev



-===========================================================================================-
-= GAMECUBE Version                                                                        =-
-===========================================================================================-

This is mostly designed for modchip users to be used as bootloader DOL on a Selbooting DVD but
you can also simply load&run emupack.dol with the method of your choice.
See "Usage" section on how to generate emupack.dol and eventually a selbooting DVD.

Features
--------
* Nice GUI (I hope ;) ) which let you choose the console system to play
* "All-in-1" DOL which directly includes all the supported emulators executables
* LZMA (7Zip) support for emulator package in order to reduce the final dol's size
* automatic bootable DVD iso generation (batch file and all needed tools included)

Requirements
------------
* a chip-modded Gamecube/Wii or a way to boot homebrew
* the main program, emuloader.dol (precompiled and included)
* a 7zip archive containing the emulators dols (not included)
* Modchip and/or SoftMod
* 7zip (http://www.7-zip.org/) to create the emulators package

Usage
-----
You can not directly use the provided emuloader.dol: it need first to be packed with a 7zip archive
containing all the supported emulators (gamecube .dol files only) you want to use.
As a result, you will eventually get a new file, called emupack.dol which contain everything and can
now be used as any other dols !

To do this, after extracting the content of the archive somewhere on your hard disc, simply follow the steps below:
 
  1) Using 7zip, create a "emupack.7z" archive with all the needed emulators dols (be careful not to use compressed/dollzed)
     and place this file in the same directory as "emuloader.dol"
     You have to use LZMA method for compressing, as well as the best compression level (ULTRA)

   Important: Don't modify the original emulator filename, the loader is looking for specific strings
   inside filenames in order to detect which emulator is present or not:

   - Genesis Plus ---> genplus
   - SMS Plus     ---> smsplus
   - FCEuGC       ---> fceugc
   - GnuboyGX     ---> gnuboy
   - Snes9xGX     ---> snes9x
   - HugoGC       ---> hugo
   - NeoPop       ---> neopop 
   - Neo-CD Redux ---> redux  

  2) Double-click on emupack.bat: this will create the file "emupack.dol"

You can now directly load/run emupack.dol with the method of your choice (SDLoad,...) if you want but the best way 
is to create a GC-Linux bootable DVD with all your roms and use emupack.dol as the bootdol.

For more infos about how to create a GC-Linux bootable DVD, you may want to read this:
http://www.gc-linux.org/wiki/Building_a_Bootable_Disc

Actually, the /pcutils folder contains everything you need to create the bootable DVD image, including automatic
batch files to generate your bootable DVD ISO file in a simple&quick way

  3) First, put all the roms you want for each systems in their respective /bootdisc subfolders
     You may want to create more subdirectories as the limit is 1000 files max. per directory
     Be aware that the total size for the /bootdisc directory should not exceeded ~1.35GB if you are using Gamecube
     and 4.7GB if you planned to use it on the Wii (still in GC mode).

    Be aware that the minimal size should be at least ~500MB or the DVD won't boot !
    Please refer to the related emulator's documentation for other specific requirements.

  4) Double-click on BuildISO.bat (or BuildISO_PAL.bat in you have a PAL Wii, BuildISO_JAP.bat for japanese Wii)
     and wait for the .iso file to be generated.

  5) Burn the emudisc.iso image file with any DVD software & launch the DVD in your console (a modchip is required)
     In the menu, use D-Pad to navigate & Button-A or START to choose a console system
 
    Once you've started running one emulator, there is no way to come back to the menu, you have to reboot your console

Source Contents
---------------
The sourcecode is full part of the release so that you can easily recompile emuloader.dol yourself.
If you have no idea on how to compile DOLs , please refer to this thread:
http://www.tehskeen.com/forums/showthread.php?t=2968.

 *emuloader.c : main program (Video Init, Menu Display & Dol's extracting)
 *emuloader.h : compressed (using zlib 2.3) BMP pictures for Menu Display
 */ngc/memfile.*, lzmasupp.*: Softdev's sourcecode for LZMA support.
 */ngc/sidestep.*, ssaram.* : Softdev's sourcecode for DOL's loading
 */ngc/lzma/ : Standard LZMA SDK 4.43 for ansi C, linked as library

PCUtils Contents
----------------
 *lzmaadd & lzmaex: Softdev's utilities to add/extract 7zip file to/from a dol (sourcecode included)
 *mkisofs & cygwin.dll: needed to generate a ISO file
 *gbi.hdr: Generic Boot Image from Cubeboot-Tools 0.3

Credits
--------
LZMA support & DOL loading code - softdev
LZMA PC utilities - softdev
DevkitPPC / LibOGC - Wntrmute & Shagkur
gbi.hdr by GCLinux Team

