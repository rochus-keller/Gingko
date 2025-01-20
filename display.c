/*
* Copyright 2025 Rochus Keller <mailto:me@rochus-keller.ch>
*
* This file is part of the Gingko project.
*
* The following is the license that applies to this copy of the
* file. For a license to use the file under conditions
* other than those described here, please email to me@rochus-keller.ch.
*
* GNU General Public License Usage
* This file may be used under the terms of the GNU General Public
* License (GPL) versions 2.0 or 3.0 as published by the Free Software
* Foundation and appearing in the file LICENSE.GPL included in
* the packaging of this file. Please review the following information
* to ensure GNU General Public Licensing requirements will be met:
* http://www.fsf.org/licensing/licenses/info/GPLv2.html and
* http://www.gnu.org/copyleft/gpl.html.
*/

#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include "sdldefs.h"
#include "byteswapdefs.h"
#include "lispemul.h"
#include "lsptypes.h"
#include "keyboard.h"
#include "lspglob.h"  // for IOPage
#include "display.h"  // for CURSORHEIGHT, DisplayRegion68k
#include "keyboard.h"

extern int error(const char *s);
extern DLword *EmKbdAd068K, *EmKbdAd168K, *EmKbdAd268K, *EmKbdAd368K, *EmKbdAd468K, *EmKbdAd568K,
    *EmRealUtilin68K;
extern DLword *CTopKeyevent;
extern LispPTR *KEYBUFFERING68k;
extern int URaid_req;
extern DLword *EmCursorX68K, *EmCursorY68K;
extern DLword *EmMouseX68K, *EmMouseY68K;
extern LispPTR *CLastUserActionCell68k;
extern int KBDEventFlg;
/* Mouse buttons */
#define MOUSE_LEFT 13
#define MOUSE_RIGHT 14
#define MOUSE_MIDDLE 15

void display_notify_lisp() {
  DLword w, r;
  KBEVENT *kbevent;

  /* DEL is not generally present on a Mac X keyboard, Ctrl-shift-ESC would be 18496 */
  if (((*EmKbdAd268K) & 2113) == 0) { /*Ctrl-shift-NEXT*/
    error("******  EMERGENCY Interrupt ******");
    *EmKbdAd268K = KB_ALLUP;          /*reset*/
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  } else if (((*EmKbdAd268K) & 2114) == 0) { /* Ctrl-Shift-DEL */
    *EmKbdAd268K = KB_ALLUP;                 /*reset*/
    URaid_req = T;
    ((RING *)CTopKeyevent)->read = 0; /* reset queue */
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
    /*return(0);*/
  }

  r = RING_READ(CTopKeyevent);
  w = RING_WRITE(CTopKeyevent);

  if (r == w) /* event queue FULL */
  {
      printf("event queue FULL\n");
      fflush(stdout);
    goto KBnext;
  }

  kbevent = (KBEVENT *)(CTopKeyevent + w);
  /*    RCLK(kbevent->time); */
  kbevent->W0 = *EmKbdAd068K;
  kbevent->W1 = *EmKbdAd168K;
  kbevent->W2 = *EmKbdAd268K;
  kbevent->W3 = *EmKbdAd368K;
  kbevent->W4 = *EmKbdAd468K;
  kbevent->W5 = *EmKbdAd568K;
  kbevent->WU = *EmRealUtilin68K;

  if (r == 0) /* Queue was empty */
    ((RING *)CTopKeyevent)->read = w;
  if (w >= MAXKEYEVENT)
    ((RING *)CTopKeyevent)->write = MINKEYEVENT;
  else
    ((RING *)CTopKeyevent)->write = w + KEYEVENTSIZE;

KBnext:
  if (*KEYBUFFERING68k == NIL) *KEYBUFFERING68k = ATOM_T;

  if ((KBDEventFlg += 1) > 0) Irq_Stk_End = Irq_Stk_Check = 0;
}

void display_notify_mouse_pos(int x, int y)
{
    *CLastUserActionCell68k = MiscStats->secondstmp;
    *EmCursorX68K = (*((DLword *)EmMouseX68K)) = (short)(x & 0xFFFF);
    *EmCursorY68K = (*((DLword *)EmMouseY68K)) = (short)(y & 0xFFFF);
}

void display_left_mouse_button(int on)
{
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, !on);
}

void display_mid_mouse_button(int on)
{
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, !on);
}

void display_right_mouse_button(int on)
{
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, !on);
}

void display_mouse_state(int left, int mid, int right)
{
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_LEFT, !left);
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_MIDDLE, !mid);
    PUTBASEBIT68K(EmRealUtilin68K, MOUSE_RIGHT, !right);
}
