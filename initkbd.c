/* $Id: initkbd.c,v 1.2 1999/01/03 02:07:09 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-1995 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#ifndef DOS
#include <sys/file.h>
#include <sys/select.h>
#endif /* DOS */

#include "lispemul.h"
#include "lispmap.h"
#include "lspglob.h"
#include "adr68k.h"
#include "address.h"

#include "devconf.h"
#include "ifpage.h"
#include "keyboard.h"

#include "initkbddefs.h"
#include "initdspdefs.h"

extern int LispKbdFd;
int LispKbdFd = -1;

extern fd_set LispReadFds;

extern DLword *EmMouseX68K;
extern DLword *EmMouseY68K;
extern DLword *EmCursorX68K;
extern DLword *EmCursorY68K;
extern DLword *EmRealUtilin68K;
extern DLword *EmUtilin68K;
extern DLword *EmKbdAd068K;
extern DLword *EmKbdAd168K;
extern DLword *EmKbdAd268K;
extern DLword *EmKbdAd368K;
extern DLword *EmKbdAd468K;
extern DLword *EmKbdAd568K;
extern DLword *EmDispInterrupt68K;
extern DLword *EmCursorBitMap68K;

DLword *EmMouseX68K;
DLword *EmMouseY68K;
DLword *EmCursorX68K;
DLword *EmCursorY68K;
DLword *EmRealUtilin68K;
DLword *EmUtilin68K;
DLword *EmKbdAd068K;
DLword *EmKbdAd168K;
DLword *EmKbdAd268K;
DLword *EmKbdAd368K;
DLword *EmKbdAd468K;
DLword *EmKbdAd568K;
DLword *EmDispInterrupt68K;
DLword *EmCursorBitMap68K;

/*uint8_t SUNLispKeyMap[128];*/
extern uint8_t *SUNLispKeyMap;
uint8_t *SUNLispKeyMap;

/* keymap for type3 */
static uint8_t SUNLispKeyMap_for3[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 255,
    /*   8 */ 100, 255,  67, 255,  68, 255, 101, 255,
    /*  16 */  66, 104,  80,  47, 255,  73,  74,  75,
    /*  24 */ 255,  92,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  13, 255,  81,  82,  83,
    /*  48 */ 255,  14, 255,  62, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  15, 255,  84,  85,  87, 255,
    /*  72 */ 111,  89, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44, 255,  94,  69,  70, 255,  90,
    /*  96 */ 255,  46, 255,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  98,  76,  72, 255, 255, 255, 255,  56,
    /* 120 */  31,  57,  93, 255, 255, 255, 255, 255
};

/* for type4 */

static uint8_t SUNLispKeyMap_for4[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  30,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  10,  59,  45,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  71,
    /* 112 */  94,  69,  70, 255, 255, 255,  92,  56,
    /* 120 */  86,  57,  88, 255, 103, 102, 255, 255
};

/* for jle */

static uint8_t SUNLispKeyMap_jle[128] = {
    /*   0 */ 255,  61, 255,  91, 255,  97,  99, 106,
    /*   8 */ 100, 107,  67, 108,  68,  47, 101,  71,
    /*  16 */  66, 104,  80,  31, 255,  75, 110,  74,
    /*  24 */ 255, 109,  63, 255, 255,  33,  32,  17,
    /*  32 */  16,   1,   0,   2,   4,  53,  22,   8,
    /*  40 */  59,  45,  30,  15, 255,  64,  65,  95,
    /*  48 */ 255,  14,  13,  89, 255,  34,  19,  18,
    /*  56 */   3,  48,  49,  51,   6,  23,  25,  11,
    /*  64 */  58,  29,  13,  93,  81,  82,  83,  96,
    /*  72 */ 111,  62, 255, 255,  36,  21,  20,   5,
    /*  80 */  35,  50,  52,  38,   9,  26,  43,  28,
    /*  88 */ 105,  44,  76,  84,  85,  87,  98,  90,
    /*  96 */ 255,  46,  73,  41,  40,  24,  37,   7,
    /* 104 */  39,  54,  55,  27,  42,  12,  60,  10,
    /* 112 */  94,  69,  70,  72, 103, 109,  92,  56,
    /* 120 */  86,  57,  88, 255, 255, 102, 255, 255
};
/* [40]   10 -> 59  */
/* [41]   59 -> 45  */
/* [42]   45 -> 30  */
/* [111]  71 -> 10  */
/* [115] 255 -> 72  Kakutei */
/* [116] 255 -> 103 Henkan */
/* [117] 255 -> 109 Nihongo On-Off */

static uint8_t *XGenericKeyMap; /* filled in with malloc if needed */

/* For the IBM-101 kbd FF marks exceptions */

#ifdef NEVER
uint8_t DOSLispKeyMap_101[0x80] = {
    /*         0    1    2    3    4    5    6    7    8    9    a    b    c    d    e    f */

    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   81,   82,   83,   96,   84,   85,   87,   102,  94,
    /* 5*/ 69,   70,   98,   13,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif /* NEVER */

void init_keyboard(int flg) /* if 0 init else re-init */
{
  set_kbd_iopointers();
  /* if using a raw keyboard, LispKbdFd would be the result of opening /dev/kbd
   * and it would be added to LispReadFds to generate keyboard events
   */
  if (flg == 0) { keyboardtype(LispKbdFd); }

}

/*  ----------------------------------------------------------------*/

void device_before_exit(void) {
  display_before_exit();
}

/*  ----------------------------------------------------------------*/

void set_kbd_iopointers(void) {
  EmMouseX68K = (DLword *)&(IOPage->dlmousex);
  EmMouseY68K = (DLword *)&(IOPage->dlmousey);
  EmCursorX68K = (DLword *)&(IOPage->dlcursorx);
  EmCursorY68K = (DLword *)&(IOPage->dlcursory);
  EmRealUtilin68K = (DLword *)&(IOPage->dlutilin);
  /* EmUtilin68K is for KEYDOWNP1 macro or etc. */
  EmUtilin68K = (DLword *)&(InterfacePage->fakemousebits);
  EmKbdAd068K = (DLword *)&(IOPage->dlkbdad0);
  EmKbdAd168K = (DLword *)&(IOPage->dlkbdad1);
  EmKbdAd268K = (DLword *)&(IOPage->dlkbdad2);
  EmKbdAd368K = (DLword *)&(IOPage->dlkbdad3);
  EmKbdAd468K = (DLword *)&(IOPage->dlkbdad4);
  EmKbdAd568K = (DLword *)&(IOPage->dlkbdad5);
  EmDispInterrupt68K = (DLword *)&(IOPage->dldispinterrupt);
  EmCursorBitMap68K = (DLword *)(IOPage->dlcursorbitmap);

  *EmRealUtilin68K = KB_ALLUP;
  *EmKbdAd068K = KB_ALLUP;
  *EmKbdAd168K = KB_ALLUP;
  *EmKbdAd268K = KB_ALLUP;
  *EmKbdAd368K = KB_ALLUP;
  *EmKbdAd468K = KB_ALLUP;
  *EmKbdAd568K = KB_ALLUP;
}

/*  ----------------------------------------------------------------*/


#define MIN_KEYTYPE 3
#define KB_AS3000J (7 + MIN_KEYTYPE)
#define KB_RS6000 (8 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_DEC3100 (9 + MIN_KEYTYPE) /* TODO: Can we remove this? */
#define KB_HP9000 (10 + MIN_KEYTYPE)  /* TODO: Can we remove this? */
#define KB_X (11 + MIN_KEYTYPE)
#define KB_DOS (12 + MIN_KEYTYPE)
#define KB_SDL (13 + MIN_KEYTYPE)

/* KB_SUN4 not defined in older OS versions */
#ifndef KB_SUN4
#define KB_SUN4 4
#endif

#ifndef KB_SUN2
/* These KB types nog defined outside Sun world,so define them here */
#define KB_SUN2 2
#define KB_SUN3 3
#endif /* KB_SUN2 */

/* For the JLE keyboard */
#define KB_JLE 5

/************************************************************************/
/*									*/
/*			  k e y b o a r d t y p e			*/
/*									*/
/*	Determine what kind of keyboard we're dealing with, by		*/
/*	checking the LDEKBDTYPE shell variable.  It it's not set,	*/
/*	either default it (for Sun, IBM), or complain and exit.		*/
/*	Valid LDEKBDTYPE values:					*/
/*		type3	Sun type-3 keyboard				*/
/*		type4	Sun type-4 keyboard				*/
/*		rs6000	IBM RS/6000					*/
/*		x	generic X keyboard map				*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void keyboardtype(int fd)
{
  int type;
  int i;
  char *key;

  /* clear the keyboard field in devconfig */
  InterfacePage->devconfig &= 0xfff8;

  /************************************************************
   Due to the problems of SunOS 4.0 & 4.0.1
   calling ioctl never return the correct keyboard type.
   So,these 2 lines are commented out ...(Take)->AR11100
  *************************************************************/

  /* Get keytype from LDEKBDTYPE  */
  if ((key = getenv("LDEKBDTYPE")) == 0) {
#ifdef SDL
    type = KB_SDL;
#endif /* SDL */
  } /* if end */
  else {
    if (strcmp("as3000j", key) == 0)
      type = KB_AS3000J;
    else if (strcmp("type4", key) == 0)
      type = KB_SUN4;
    else if (strcmp("type2", key) == 0)
      type = KB_SUN2;
    else if (strcmp("jle", key) == 0)
      type = KB_JLE;
    else if (strcmp("X", key) == 0 || strcmp("x", key) == 0)
      type = KB_X;
    else if (strcmp("sdl", key) == 0)
      type = KB_SDL;
    else
      type = KB_SUN3; /* default */
  }

  switch (type) {
    case KB_SUN2: /* type2, we still use keymap for type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      /* MIN_KEYTYPE is 3,so we can't set devconfig correctly */
      /* Therefore type2 may treat as type3 */
      InterfacePage->devconfig |= 0;
      break;
    case KB_SUN3: /* type3 */
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_SUN4: /* type4 */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      break;
    case KB_JLE: /* JLE */
      /*printf("jle\n"); */
      SUNLispKeyMap = SUNLispKeyMap_jle;
      InterfacePage->devconfig |= type - MIN_KEYTYPE;
      /* InterfacePage->devconfig |= 4-MIN_KEYTYPE; */
      break;
    case KB_AS3000J: /* for AS, use type4 map */
      SUNLispKeyMap = SUNLispKeyMap_for4;
      InterfacePage->devconfig |= type - MIN_KEYTYPE; /* 7 */
      break;
#ifdef SDL
  case KB_SDL:
    InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* 10 */
    break;
#endif /* SDL */
    default: {
      char errmsg[200];
      sprintf(errmsg, "Unsupported keyboard type: %d", type);
      printf("%s\n", errmsg);
      printf("Configuring keyboard for type-3\n");
      SUNLispKeyMap = SUNLispKeyMap_for3;
      InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE;
      break;
    }
  }

} /* end keyboardtype*/
