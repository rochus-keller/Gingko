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
    /*        0     1    2    3    4    5    6   7    */
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
    // NOTE: these are the Make Codes (press) of the Sun keyboard
    /*        0     1    2    3    4    5    6   7    */
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

static uint8_t *XGenericKeyMap; /* filled in with malloc if needed */

/* For the IBM-101 kbd FF marks exceptions */

#ifdef DOS
/* For the IBM-101 kbd FF marks exceptions */
static u_char DOSLispKeyMap_101[0x80] = {
    /*     0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f */
    /* 0*/ 0xff, 33,   32,   17,   16,   1,    0,    2,    4,    53,   22,   8,    10,   59,   15,   34,
    /* 1*/ 19,   18,   3,    48,   49,   51,   6,    23,   25,   11,   58,   29,   44,   36,   21,   20,
    /* 2*/ 5,    35,   50,   52,   38,   9,    26,   43,   28,   45,   41,   105,  40,   24,   37,   7,
    /* 3*/ 39,   54,   55,   27,   42,   12,   60,   95,   31,   57,   56,   97,   99,   100,  67,   68,
    /* 4*/ 101,  66,   104,  80,   106,  73,   74,   62,   130,  63,   96,   129,  85,   132,  102,  90,
    /* 5*/ 131,  91,   89,   46,   0xff, 0xff, 0xff, 107,  108,  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    /* 6*/ 89,   62,   63,   46,   90,   91,   130,  129,  131,  132,  92,   61,   0xff, 0xff, 0xff, 0xff,
    /* 7*/ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#endif

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
#define KB_SUN3 3

void keyboardtype(int fd)
{

  /* clear the keyboard field in devconfig */
  InterfacePage->devconfig &= 0xfff8;

  // KB_SDL
  // NOP
  InterfacePage->devconfig |= KB_SUN3 - MIN_KEYTYPE; /* does actually nothing */


} /* end keyboardtype*/
