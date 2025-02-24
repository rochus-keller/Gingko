/* $Id: bitblt.c,v 1.2 1999/01/03 02:06:47 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>

#include "lispemul.h"
#include "lspglob.h"
#include "lispmap.h"
#include "adr68k.h"
#include "address.h"

#include "pilotbbt.h"
#include "display.h"
#include "bitblt.h"
#include "bb.h"

#include "bitbltdefs.h"
#include "initdspdefs.h"

#if defined(INIT)
#include "initkbddefs.h"
extern int kbd_for_makeinit;
#endif

extern int ScreenLocked;

/*****************************************************************************/
/**                                                                         **/
/**                             N_OP_pilotbitblt                            **/
/**                                                                         **/
/**       The Native-code compatible version of the opcode for bitblt.      **/
/**                                                                         **/
/**                                                                         **/
/*****************************************************************************/

LispPTR N_OP_pilotbitblt(LispPTR pilot_bt_tbl, LispPTR tos)
{
  PILOTBBT *pbt;
  DLword *srcbase, *dstbase;
  int sx, dx, w, h, srcbpl, dstbpl, backwardflg;
  int src_comp, op, gray, num_gray, curr_gray_line;

#ifdef INIT

  /* for init, we have to initialize the pointers at the
     first call to pilotbitblt or we die.  If we do it
     earlier we die also.  We set a new flag so we don't
     do it more than once which is a lose also.

     I put this in an ifdef so there won't be any extra
     code when making a regular LDE.  */

  if (!kbd_for_makeinit) {
    init_keyboard(0);
    kbd_for_makeinit = 1;
  }

#endif

  pbt = (PILOTBBT *)NativeAligned4FromLAddr(pilot_bt_tbl);

  w = pbt->pbtwidth;
  h = pbt->pbtheight;
  if ((h <= 0) || (w <= 0)) return (pilot_bt_tbl);
  dx = pbt->pbtdestbit;
  sx = pbt->pbtsourcebit;
  backwardflg = pbt->pbtbackward;
/* if displayflg != 0 then source or destination is DisplayBitMap */
  ScreenLocked = T;

  srcbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtsourcehi, pbt->pbtsourcelo));
  dstbase = (DLword *)NativeAligned2FromLAddr(VAG2(pbt->pbtdesthi, pbt->pbtdestlo));

  srcbpl = pbt->pbtsourcebpl;
  dstbpl = pbt->pbtdestbpl;
  src_comp = pbt->pbtsourcetype;
  op = pbt->pbtoperation;
  gray = pbt->pbtusegray;
  num_gray = ((TEXTUREBBT *)pbt)->pbtgrayheightlessone + 1;
  curr_gray_line = ((TEXTUREBBT *)pbt)->pbtgrayoffset;

  new_bitblt_code;

  ScreenLocked = NIL;

  return (pilot_bt_tbl);

} /* end of N_OP_pilotbitblt */

/************************************************************************/
/*                                                                      */
/*                              c u r s o r i n                                 */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

/* for MONO only */
int cursorin(DLword addrhi, DLword addrlo, int w, int h, int backward)
{
  int x, y;
  if (addrhi == DISPLAY_HI) {
    y = addrlo / DisplayRasterWidth;
    x = (addrlo - y * DisplayRasterWidth) << 4;
  } else if (addrhi == DISPLAY_HI + 1) {
    y = (addrlo + DLWORDSPER_SEGMENT) / DisplayRasterWidth;
    x = ((addrlo + DLWORDSPER_SEGMENT) - y * DisplayRasterWidth) << 4;
  } else
    return (NIL);

  if (backward) y -= h;

  if ((x < MOUSEXR) && (x + w > MOUSEXL) && (y < MOUSEYH) && (y + h > MOUSEYL))
    return (T);
  else
    return (NIL);
}
