¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤
 
                                   NGC EMULoader
                                    Version 1.0   
 
¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤°`°¤ø,¸,ø¤°`°¤ø,¸¸,ø¤
 

-=[ Explanation ]=-
 
This is a NGC program that enables you to load & run the following emulators on your Nintendo Gamecube:

   - Genesis Plus (Sega Megadrive)
   - SMS Plus     (Sega Master System & Game Gear)
   - FCEuGC       (Nintendo Entertainment System)
   - GnuboyGX     (Gameboy & Gameboy Color)
   - Snes9xGX     (Super Nintendo)
   - HugoGC       (NEC Pc-Engine)
   - NeoPop       (SNK NeoGeo Pocket)
   - Neo-CD Redux (SNK NeoGeo CD)

Last versions of emulator binaries (dol), sourcecode, documentation & original credits 
can be found on Tehskeen's development forum:

Genesis Plus: http://www.tehskeen.com/forums/showpost.php?p=15457&postcount=2
SMS Plus:     http://www.tehskeen.com/forums/showthread.php?t=4209
FCEuGC:       http://www.tehskeen.com/forums/showthread.php?t=3920
Snes9XGX2:    http://www.tehskeen.com/forums/showthread.php?t=3918
HugoGC:       http://www.tehskeen.com/forums/showpost.php?p=15966&postcount=2
GnuboyGX:     http://www.tehskeen.com/forums/showthread.php?t=4443
NeoPop:       http://www.tehskeen.com/forums/showpost.php?p=15967&postcount=3
NeoCDredux:   http://www.tehskeen.com/forums/showpost.php?p=16014&postcount=75

This is mostly designed for modchip users to be used as bootloader DOL on a Selbooting DVD but
you can also simply load&run emupack.dol with the method of your choice.
See "Usage" section on how to generate emupack.dol and eventually a selbooting DVD


-=[ WII Users]=-

All these emulators were initially developped to run on the Gamecube, they run on the Wii 
because of the GC compatible mode but there are still some restrictions that you should take care of:

    * The last version of the NES emulator holds because it didn't recognize the WII drive ID.
    You have to use a special version (however based on an older version), that you can download here.
    Same thing applies for snes9xGX 1.43, use the special version on the link below or 2.0 version:
    http://www.tehskeen.com/forums/showthread.php?t=4176

    * PAL Wii users may experience some display problems when running these emulators.
    You should use special PAL50 versions when they exist.
    Actually only GenesisPlus and Snes9xGX 2.0 denman's special PAL version have been modified to support PAL50.
    Please note that with this Snes9xGX version, NTSC roms will run too slow.

    * There is actually a limit of 1.35Gb for the DVD compilation size, which is hardcoded in the sourcecode of the emulators.
    This means that the majority of the emulators won't be able to load roms which are stored over this limit.
    Actually, this neocdredux modified version is the only one that allows to go up the 4.7Gb limit:
    http://www.tehskeen.com/forums/showpost.php?p=16549&postcount=111

    * To build the ISO, use the appropriate .bat file, according to your console region:
       - all Gamecube's and USA Wii users should use BuildISO.bat
       - PAL (european) Wii users should use BuildISO_PAL.bat
       - Japanese Wii users should use BuildISO_JAP.bat



-=[ Features ]=-

* Nice GUI (I hope ;) ) which let you choose the console system to play
* "All-in-1" DOL which directly includes all the supported emulators executables
* LZMA (7Zip) support for emulator package in order to reduce the final dol's size (thanks to Softdev !)
* automatic bootable DVD iso generation (batch file and all needed tools included)


-=[ Requirements ]=- 
* the main program, emuloader.dol (precompiled and included)
* a 7zip archive containing the emulators dols (not included)
* Modchip and/or SoftMod
* 7zip (http://www.7-zip.org/) to create the emulators package
* DevkitPPC environment & last libOGC if you want to recompile the sources


-=[ Usage ]=-

You have to understand that "emuloader.dol" is useless alone as it doesn't include any emulator executables.
It's only the program that will provide the interface and will load the appropriate emulators for you.
You have to attach a 7zip archive to it, containing all the emulator dols, using Softdev's lzmaadd utility
(included in /pcutils folder).
This is required so that you can easily update yourself the loader with future versions of supported emulators programs.

To do this, after extracting the content of the archive somewhere on your hard disc, just follow the steps below:
 
  1) Using 7zip, create a "emupack.7z" archive with all the needed emulators dols (be careful not to use compressed/dollzed)
     and place this file in the same directory as "emuloader.dol"
     You have to use LZMA method for compressing, as well as the best compression level (ULTRA)

     IMPORTANT: don't modify the name of the original dols that you put inside emupack.7z too much
         as the program try to detect them by looking into filenames for the following strings:
    	 "genplus", "smsplus", "snes9x", "fceugc", "hugo", "neopop", "redux", "gnuboy"

  2) Doubleclick on BuildDOL.bat: this will create the file "emupack.dol"

You can now directly load/run emupack.dol if you want but the best way is to create a GC-Linux bootable DVD
with all your roms and emupack.dol as the bootdol. For more infos about how to create a GC-Linux bootable DVD,
you may want to read this: http://www.gc-linux.org/wiki/Building_a_Bootable_Disc
Actually, the /pcutils folder contains everything you need to create the bootable DVD image, including automatic batch files to
generate your bootable DVD ISO file in a simple&quick way

  3) First, put all the roms you want for each systems in their respective /bootdisc subfolders
     You may want to create more subdirectories as the limit is 1000 files max. per directory
     Be aware that the total size for the /bootdisc directory should not exceeded ~1.35GB if you are using Gamecube
     and 4.7GB if you planned to use it on the Wii. The minimal size should be at least ~500MB or the DVD won't boot.
     Please refer to the related emulator's documentation for specific requirements.

  4) Doubleclick on BuildISO.bat (or BuildISO_PAL.bat in you have a PAL Wii, BuildISO_JAP.bat for japanese Wii)
     and wait for the .iso file to be generated.

  5) Burn the emudisc.iso image file with any DVD software (tested fine with CloneDVD) & insert the burned DVD in your console
     In the menu, use D-Pad to navigate & Button-A or START to choose a console system
     Once you've started running one emulator, there is no way to come back to the menu, you have to reboot your console


-=[ Compilation ]=-

The sourcecode is full part of the release so that you can easily recompile emuloader.dol yourself.
If you have no idea on how to compile DOLs , please refer to this thread: http://www.tehskeen.com/forums/showthread.php?t=2968.


-=[ Source Contents ]=-

 *emuloader.c : main program (Video Init, Menu Display & Dol's extracting)
 *emuloader.h : compressed (using zlib 2.3) BMP pictures for Menu Display
 *memfile.*, lzmasupp.*: Softdev's sourcecode for LZMA support.
 *sidestep.*, ssaram.* : Softdev's sourcecode for DOL's loading
 */lzma/ : Standard LZMA SDK 4.43 for ansi C, linked as library


-=[ PCUtils Contents]=- 
 *lzmaadd & lzmaex: Softdev's utilities to add/extract 7zip file to/from a dol (sourcecode included)
 *mkisofs & cygwin.dll: needed to generate a ISO file
 *gbi.hdr: Generic Boot Image from Cubeboot-Tools 0.3


-=[ Credits ]=-

Emulators Port/Coding - SoftDev, EkeEke
LZMA support & DOL Loading original sourcecode - SoftDev
LZMA PC utilities - Softdev
DevkitPPC / LibOGC by Wntrmute & Shagkur
ZLIB by Jean-loup Gailly
gbi.hdr by GCLinux Team
GUI design & generic code "copypasting" by me ;)


-=[ Changelog ]=-

26/05/2007:
 - corrected a bug that will make the loader crash if no suitable emulator dols is found
 - added specific region ISO builder for PAL/JAP Wii users

 19/05/2007:
  - Initial release

