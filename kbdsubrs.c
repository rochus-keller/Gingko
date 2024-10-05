/* $Id: kbdsubrs.c,v 1.2 1999/01/03 02:07:10 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <stdio.h>
#ifndef DOS
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/select.h>
#endif /* DOS */


#include "lispemul.h"

#include "kbdsubrsdefs.h"
#include "commondefs.h"
#ifdef XWINDOW
#include "lisp2cdefs.h"
#include "xwinmandefs.h"
#endif

/****************************************************
 *
 *	KB_enable() entry of SUBRCALL 82 1
 *			called from (\KB_enable X)
 *
 ****************************************************/


#ifdef XWINDOW
#include <X11/Xlib.h>
#endif /* XWINDOW */

extern fd_set LispReadFds;

void KB_enable(LispPTR *args) /* args[0] :	ON/OFF flag
                                     *		T -- ON
                                     *		NIL -- OFF
                                     */
{
  if (args[0] == ATOM_T) {
  } else if (args[0] == NIL) {
  } else {
    error("KB_enable: illegal arg \n");
    printf("KB_enable: arg = %d\n", args[0]);
  }
}

/****************************************************
 *
 *	KB_beep() entry of SUBRCALL 80 2
 *			called from (\KB_beep SW FREQ)
 *
 ****************************************************/
/*
struct timeval belltime ={
        0,100
};
*/
extern int LispKbdFd;

void KB_beep(LispPTR *args) /* args[0] :	ON/OFF flag
                                   *		T -- ON
                                   *		NIL -- OFF
                                   * args[1] :	frequency
                                   */
{
}

/****************************************************
 *
 *	KB_setmp() entry of SUBRCALL 81 1
 *
 ****************************************************/

void KB_setmp(LispPTR *args) /* args[0] :	MPCODE	*/
{
#ifdef DEBUG
  printf("MP: %d\n", args[0] & 0xffff);
#endif
}

/****************************************************
 *
 *	KB_setled()
 * Set the status LED's on the kbd.
 * arg[0] Caps lock LED.
 * arg[1] Num lock LED.
 * arg[2] Scroll lock LED.
 * NIL -> LED off
 * not NIL -> LED on.
 *
 ****************************************************/

void KB_setled(LispPTR *args)
{
}
