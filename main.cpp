/* $Id: main.c,v 1.4 2001/12/26 22:17:03 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	main.c
 *	This file includes main()
 */

#define __USE_BSD 1
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <time.h>

#ifndef DOS
#include <sys/param.h>
#include <unistd.h>
#endif /* DOS */

#include "adr68k.h"
#include "stack.h"
#include "return.h"

#include "lispemul.h"
#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "ifpage.h"
#include "iopage.h"

#include "maindefs.h"
#include "commondefs.h"
#include "dirdefs.h"
#include "dspifdefs.h"
#include "initdspdefs.h"
#include "initkbddefs.h"
#include "initsoutdefs.h"
#include "ldsoutdefs.h"
#include "miscstat.h"
#include "storagedefs.h"
#include "timerdefs.h"
#include "unixcommdefs.h"
#include "xcdefs.h"

DLword *Lisp_world; /* lispworld */

/********** 68k address for Lisp Space **********/
DLword *Stackspace;
DLword *Plistspace;
DLword *DTDspace;
DLword *MDStypetbl;
DLword *AtomHT;
DLword *Pnamespace;
DLword *AtomSpace;
DLword *Defspace;
DLword *Valspace;

/********** For Virtual Memory Management **********/
#ifdef BIGVM
LispPTR *FPtoVP;
#else
DLword *FPtoVP;
#endif /* BIGVM */
DLword *PAGEMap;
DLword *PageMapTBL;
DLword *LockedPageTable;

/********** For Interface to LispMicro/Device **********/
DLword *IOCBPage;
IOPAGE *IOPage;
IFPAGE *InterfacePage;
MISCSTATS *MiscStats;

/********** UFN Table **********/
DLword *UFNTable;

/********** Tables for GC **********/
#ifdef BIGVM
LispPTR *HTmain;
LispPTR *HToverflow;
LispPTR *HTbigcount;
LispPTR *HTcoll;
#else
DLword *HTmain;
DLword *HToverflow;
DLword *HTbigcount;
DLword *HTcoll;
#endif /* BIGVM */

/********** Display **********/
DLword *DisplayRegion;
int DisplayInitialized = NIL;

DLword *MDS_space_bottom;
DLword *PnCharspace;
struct dtd *ListpDTD;

/********** For Lisp Emulator **********/
struct state MachineState;

/**********************************/
/*** Share val with LISP code ******/

DLword *MDS_free_page;
DLword *Next_MDSpage;
DLword *Next_Array;
/*******************************************/

/** CACHE LISP SYSVAL ***/
LispPTR *Next_MDSpage_word;
LispPTR *Next_Array_word;
LispPTR *MDS_free_page_word;

LispPTR *Reclaim_cnt_word;

/*** Cache Values for reclaimer by Tomtom 30-Sep-1987 ***/
LispPTR *GcDisabled_word;
LispPTR *CdrCoding_word;
LispPTR *FreeBlockBuckets_word;
LispPTR *Array_Block_Checking_word;
LispPTR *ArrayMerging_word;
LispPTR *ArraySpace_word;
LispPTR *ArraySpace2_word;
LispPTR *ArrayFrLst_word;
LispPTR *ArrayFrLst2_word;
LispPTR *Hunk_word;
LispPTR *System_Buffer_List_word;

/*** The end of the addition of cache values on reclaimer ***/

/*** cache values for the top level reclaimer's implementation ***/

LispPTR *GcMess_word;
LispPTR *ReclaimMin_word;
LispPTR *GcTime1_word;
LispPTR *GcTime2_word;
LispPTR *MaxTypeNumber_word;

/*** The end of the addition of cache values for top reclaimer by Tomtom
                                                15-Oct-1987             ***/

/*  Pointers for closure caching */

LispPTR *Package_from_Index_word;
LispPTR *Package_from_Name_word;
LispPTR *Keyword_Package_word;
LispPTR *Closure_Cache_Enabled_word;
LispPTR *Closure_Cache_word;
LispPTR *Deleted_Implicit_Hash_Slot_word;
LispPTR First_index;

/*** The end of Pointers for closure caching ***/

/* CACHE values for 32Mb MDS/Array by Take */
LispPTR *STORAGEFULLSTATE_word;
LispPTR *STORAGEFULL_word;
LispPTR *PENDINGINTERRUPT_word;
LispPTR *LeastMDSPage_word;
LispPTR *SecondMDSPage_word;
LispPTR *SecondArrayPage_word;
LispPTR *INTERRUPTSTATE_word;
LispPTR *SYSTEMCACHEVARS_word;
LispPTR *MACHINETYPE_word;

LispPTR STORAGEFULLSTATE_index;
LispPTR *LASTVMEMFILEPAGE_word;
LispPTR *VMEM_FULL_STATE_word;

/** Array for N-tran **/

int native_load_address;
LispPTR native_closure_env = NOBIND_PTR;

/** Pipes for Unix Interface **/
int UnixPipeIn;
int UnixPipeOut;
int UnixPID;
int please_fork = 1;
/* disable X11 scroll bars if requested */
int noscroll = 0;
/*** STACK handle staff(Takeshi) **/
LispPTR *STACKOVERFLOW_word;
LispPTR *GuardStackAddr_word;
LispPTR *LastStackAddr_word;
LispPTR *NeedHardreturnCleanup_word;

extern struct sockaddr_nit snit;

#ifdef INIT
int for_makeinit = 1;
#else
int for_makeinit = 0;
#endif /* INIT */

int kbd_for_makeinit = 0;
int save_argc;
char **save_argv;
int display_max = 65536 * 16 * 2;

/* diagnostic flag for sysout dumping */
extern unsigned maxpages;

char sysout_name_cl[MAXPATHLEN] = "\0"; /* sysout name as per -sysout command line arg ; Set by read_Xoption, in the X version. */
char sysout_name_xrm[MAXPATHLEN] = "\0"; /* sysout name as per X resource manager, if any */
char sysout_name_first_arg[MAXPATHLEN] = "\0"; /* sysout name as per 1st command line arg, if any */
char sysout_name[MAXPATHLEN] = "\0"; /* "final" sysout name chosen from above */

unsigned sysout_size = 0;    /* ditto */

int flushing = FALSE; /* see dbprint.h if set, all debug/trace printing will call fflush(stdout) after each printf */

#ifdef SDL
extern int init_SDL(const char*, int, int, int);
#endif
#ifdef QTGUI
extern int qt_init(const char *windowtitle, int w, int h, int s);
#endif

static const time_t MDate = 1727952008;
extern int nokbdflag;
extern int nomouseflag;
#if defined SDL || defined QTGUI
const char *helpstring =
    "\n\
 either setenv LDESRCESYSOUT or do:\n\
 medley [<sysout-name>] [<options>]\n\
 -info                    Print general info about the system\n\
 -help                    Print this message\n\
 -pixelscale <n>          The amount of pixels to show for one Medley screen pixel.\n\
 -fg/-foreground <color>  Screen foreground color, default Black.  X color name or #RRBBGG hex\n\
 -bg/-background <color>  Screen background color, default White.  X color name or #RRBBGG hex\n\
 -sc[reen] <w>x<h>]       The Medley screen geometry\n\
 -t <title>               The window title\n\
 -title <title>           The window title\n";
#endif

#if defined(SDL) || defined(QTGUI)
char foregroundColorName[64] = {0};
extern char foregroundColorName[64];
char backgroundColorName[64] = {0};
extern char backgroundColorName[64];
#endif
/************************************************************************/
/*									*/
/*		     M A I N   E N T R Y   P O I N T			*/
/*									*/
/*									*/
/************************************************************************/

int main(int argc, char *argv[])
{
  int i = 0;
  char *envname;
  extern int TIMER_INTERVAL;
  extern fd_set LispReadFds;
  long tmpint;
  int width = 1024, height = 768;
  int pixelscale = 1;
  const char *windowtitle = "Medley";

#ifdef PROFILE
  moncontrol(0); /* initially stop sampling */
#endif           /* PROFILE */

  //
  //
  //  Process Command Line Arguments
  //
  //

  // First check if the first argument is a sysout name
  // and save it away if it is in case the X windows
  // arg processing changes argc/argv
  if (argc > 1 && argv[1][0] != '-') {
    strncpy(sysout_name_first_arg, argv[1], MAXPATHLEN);
    i++;
  }

  save_argc = argc;
  save_argv = argv;

  i = 1;

  if (argv[i] && ((strcmp(argv[i], "-info") == 0) || (strcmp(argv[i], "-INFO") == 0))) {
    print_info_lines();
    exit(0);
  }

  if (argv[i] && ((strcmp(argv[i], "-help") == 0) || (strcmp(argv[i], "-HELP") == 0))) {
    (void)fprintf(stderr, "%s", helpstring);
    exit(0);
  }


  for (; i < argc; i += 1) { /* step by 1 in case of typo */

   // NOTE:  in the case of X Windows, some of the args being checked for in this loop
   // have already been processed (and removed from argv) by the call to read_Xoption()
   // above.  (See readXoption() in xrdopt.c)

   /* Check for -sysout arg */
    if (!strcmp(argv[i], "-sysout")) {
      if (argc > ++i) {
        strncpy(sysout_name_cl, argv[i], MAXPATHLEN);
      }
    }

    /* -t and -m are undocumented and somewhat dangerous... */

    else if (!strcmp(argv[i], "-t")) { /**** timer interval	****/
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          TIMER_INTERVAL = tmpint;
        } else {
          (void)fprintf(stderr, "Bad value for -t (integer > 0)\n");
          exit(1);
        }
      } else {
        (void)fprintf(stderr, "Missing argument after -t\n");
        exit(1);
      }
    }

    else if (!strcmp(argv[i], "-m")) { /**** sysout size	****/
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          sysout_size = (unsigned)tmpint;
        } else {
          (void)fprintf(stderr, "Bad value for -m (integer > 0)\n");
          exit(1);
        }
      } else {
        (void)fprintf(stderr, "Missing argument after -m\n");
        exit(1);
      }
    }

    else if (!strcmp(argv[i], "-NF")) { /****  Don't fork (for dbxing)	****/
      please_fork = 0;
    }

    else if (!strcmp(argv[i], "-INIT")) { /*** init sysout, no packaged */
      for_makeinit = 1;
    }
#ifdef SDL
    else if ((strcmp(argv[i], "-sc") == 0) || (strcmp(argv[i], "-SC") == 0)) {
      if (argc > ++i) {
        int read = sscanf(argv[i], "%dx%d", &width, &height);
        if(read != 2) {
          (void)fprintf(stderr, "Could not parse -sc argument %s\n", argv[i]);
          exit(1);
        }
      } else {
        (void)fprintf(stderr, "Missing argument after -sc\n");
        exit(1);
      }
    } else if ((strcmp(argv[i], "-pixelscale") == 0) || (strcmp(argv[i], "-PIXELSCALE") == 0)) {
      if (argc > ++i) {
        int read = sscanf(argv[i], "%d", &pixelscale);
        if(read != 1) {
          (void)fprintf(stderr, "Could not parse -pixelscale argument %s\n", argv[i]);
          exit(1);
        }
      } else {
        (void)fprintf(stderr, "Missing argument after -pixelscale\n");
        exit(1);
      }
    } else if ((strcmp(argv[i], "-t") == 0) || (strcmp(argv[i], "-T") == 0)
               || (strcmp(argv[i], "-title") == 0) || (strcmp(argv[i], "-TITLE") == 0)) {
      if (argc > ++i) {
        windowtitle = argv[i];
      } else {
        (void)fprintf(stderr, "Missing argument after -title\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "-fg") == 0 || strcmp(argv[i], "-foreground") == 0) {
      if (argc > ++i) {
        strncpy(foregroundColorName, argv[i], sizeof(foregroundColorName) - 1);
      } else {
        (void)fprintf(stderr, "Missing argument after -fg/-foreground\n");
        exit(1);
      }
    } else if (strcmp(argv[i], "-bg") == 0 || strcmp(argv[i], "-background") == 0) {
      if (argc > ++i) {
        strncpy(backgroundColorName, argv[i], sizeof(backgroundColorName) - 1);
      } else {
        (void)fprintf(stderr, "Missing argument after -bg/-background\n");
        exit(1);
      }
    }
#endif /* SDL */

    /* diagnostic flag for big vmem write() calls */
    else if (!strcmp(argv[i], "-xpages")) {
      if (argc > ++i) {
        errno = 0;
        tmpint = strtol(argv[i], (char **)NULL, 10);
        if (errno == 0 && tmpint > 0) {
          maxpages = (unsigned)tmpint;
        } else {
          (void)fprintf(stderr, "Bad value for -xpages (integer > 0)\n");
          exit(1);
        }
      } else {
        (void)fprintf(stderr, "Missing argument after -xpages\n");
        exit(1);
      }
    }
  }

  //
  //  OK, now we can process the sysout_name
  //  Order of priority:
  //    1. Value of -sysout command line arg
  //    2. Value of the first command line arg
  //    3. Value of LDESRCESYSOUT env variable
  //    4. Value of LDESOURCESYSOUT env variable
  //    5. Value as determined by X resource manager, if any
  //    6. Value of $HOME/lisp.virtualmem (or lisp.vm for DOS)
  //
  int derive_medley_path = 0;
  if (sysout_name_cl[0] != '\0') {
      strncpy(sysout_name, sysout_name_cl, MAXPATHLEN);
      derive_medley_path = 1;
  } else if (sysout_name_first_arg[0] != '\0') {
      strncpy(sysout_name, sysout_name_first_arg, MAXPATHLEN);
      derive_medley_path = 1;
  } else if ((envname = getenv("LDESRCESYSOUT")) != NULL) { strncpy(sysout_name, envname, MAXPATHLEN); }
  else if ((envname = getenv("LDESOURCESYSOUT")) != NULL) { strncpy(sysout_name, envname, MAXPATHLEN); }
  else if (sysout_name_xrm[0] != '\0') { strncpy(sysout_name, sysout_name_xrm, MAXPATHLEN); }
  else {
    if ((envname = getenv("HOME")) != NULL) {
      strncpy(sysout_name, envname, MAXPATHLEN);
      strncat(sysout_name, "/lisp.virtualmem", MAXPATHLEN - 17);
    }
  }
  if ((sysout_name[0] == '\0') || (access(sysout_name, R_OK))) {
    perror("Couldn't find a sysout to run");
    fprintf(stderr, "Looking for: %s\n", sysout_name);
    (void)fprintf(stderr, "%s", helpstring);
    exit(1);
  }
  /* OK, sysout name is now in sysout_name */

  if( derive_medley_path )
  {
       for( size_t i = 0; i < strlen(sysout_name); i++ )
       {
           if( sysout_name[i] == '\\' )
               sysout_name[i] = '/';
       }
       char* found = strstr(sysout_name,"/loadups/");
       if( found )
       {
           *found = 0;
           setenv("MEDLEYDIR", sysout_name, 1);
           printf("Gingko set MEDLEYDIR to '%s'\n", sysout_name);
           *found = '/';
           setenv("LDESOURCESYSOUT", sysout_name, 1);
           setenv("LDEDESTSYSOUT", "${HOME}/lisp.virtualmem", 1);
           setenv("INMEDLEY", "1", 1);
           strcpy(sysout_name_cl,found);
           strcpy(found,"/greetfiles/MEDLEYDIR-INIT");
           setenv("LDEINIT", sysout_name, 1);
           printf("Gingko set LDEINIT to '%s'\n", sysout_name);
           fflush(stdout);
           strcpy(found, sysout_name_cl);
       }
  }


  //
  //
  // End of command line arg processing
  //
  //


/* Sanity checks. */
  if (getuid() != geteuid()) {
    (void)fprintf(stderr, "Effective user is not real user.  Resetting uid\n");
    if (setuid(getuid()) == -1) {
      (void)fprintf(stderr, "Unable to reset user id to real user id\n");
      exit(1);
    }
  }

  FD_ZERO(&LispReadFds);

  /* Fork Unix was called in kickstarter; if we forked, look up the */
  /* pipe handles to the subprocess and set them up.		      */

  if (FindUnixPipes()) /* must call the routine to allocate storage, */
  {                    /* in case we're re-starting a savevm w/open ptys */
    if (please_fork) (void)fprintf(stderr, "Failed to find UNIXCOMM file handles; no processes\n");
  }

#if defined(SDL)
  init_SDL(windowtitle, width, height, pixelscale);
#endif /* SDL */
#ifdef QTGUI
  qt_init(windowtitle, width, height, pixelscale);
#endif
  /* Load sysout to VM space and returns real sysout_size(not 0) */
  sysout_size = sysout_loader(sysout_name, sysout_size);

  build_lisp_map(); /* built up map */

  init_ifpage(sysout_size); /* init interface page */
  init_iopage();
  init_miscstats();
  init_storage();

  set_cursor();

  /* file system directory enumeration stuff */
  if (!init_finfo()) {
    (void)fprintf(stderr, "Cannot allocate internal data.\n");
    exit(1);
  }

  /* Get OS message to ~/lisp.log and print the message to prompt window */
  if (!for_makeinit) {

    init_keyboard(0); /* can't turn on the keyboard yet or you will die
                         in makeinit.  Pilotbitblt will turn it on if
                         you used the proper switches when building LDE.
                            JDS -- 1/18/90 also BITBLTSUB does it now. */
  }

  /* now start up lisp */
  start_lisp();
  return (0);
}

/************************************************************************/
/*									*/
/*		  	   s t a r t _ l i s p				*/
/*									*/
/*	This is the function that actually starts up the lisp emulator.	*/
/*									*/
/*									*/
/************************************************************************/

void start_lisp(void) {
  DLword *freeptr, *next68k;

/*******************************/
/*  First, turn off any pending interrupts from during VMEMSAVE.	*/
/*  This keeps US from trying to handle OLD interrupts.		*/
/*******************************/
#ifndef INIT
  {
    INTSTAT *intstate = ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
    intstate->ETHERInterrupt = 0;
    intstate->LogFileIO = 0;
    intstate->IOInterrupt = 0;
    intstate->waitinginterrupt = 0;
    intstate->intcharcode = 0;
  }
#endif /* INIT */

  TopOfStack = 0;
  Error_Exit = 0;

  PVar = NativeAligned2FromStackOffset(InterfacePage->currentfxp) + FRAMESIZE;

  freeptr = next68k = NativeAligned2FromStackOffset(CURRENTFX->nextblock);

  if (GETWORD(next68k) != STK_FSB_WORD) error("Starting Lisp: Next stack block isn't free!");

  while (GETWORD(freeptr) == STK_FSB_WORD) EndSTKP = freeptr = freeptr + GETWORD(freeptr + 1);

  CurrentStackPTR = next68k - 2;

  FastRetCALL;

  /* JRB - The interrupt initialization must be done right before  */
  /*       entering the bytecode dispatch loop; interrupts get     */
  /*       unblocked here 					   */
  int_init();
  dispatch();
}

void print_info_lines(void) {
#if (RELEASE == 200)
  printf("Emulator for Medley release 2.0\n");
#elif (RELEASE == 201)
  printf("Emulator for Medley release 2.01\n");
#elif (RELEASE == 210)
  printf("Emulator for Medley release 2.1\n");
#elif (RELEASE == 300)
  printf("Emulator for Medley release 3.0\n");
#elif (RELEASE == 350)
  printf("Emulator for Medley release 3.5\n");
#elif (RELEASE == 351)
  printf("Emulator for Medley release 3.51\n");
#endif /* RELEASE */
  printf("Compiled for %s (%s) (%d bit).\n", MAIKO_OS_NAME, MAIKO_ARCH_NAME, MAIKO_ARCH_WORD_BITS);
  printf("Creation date: %s", ctime(&MDate));
#ifdef LPSOLVE
  printf("Contains lp_solve LP solver.\n");
#endif /* LPSOLVE */
#ifdef BIGBIGVM
  printf("Supports 256Mb virtual memory.\n");
#elif defined BIGVM
  printf("Supports 64Mb virtual memory.\n");
#else
  printf("Supports 32Mb virtual memory.\n");
#endif /* BIGVM */
#ifdef NOVERSION
  printf("Does not enforce SYSOUT version matching.\n");
#endif /* NOVERSION */
#ifdef NOEUROKBD
  printf("No support for European keyboards.\n");
#else
  printf("Supports Sun European Type-4/5 keyboards.\n");
#endif /* NOEUROKBD */
}
