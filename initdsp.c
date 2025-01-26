/* $Id: initdsp.c,v 1.2 1999/01/03 02:07:08 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/*
 *	file	:	initdsp.c
 *	Author	:	Osamu Nakamura
 */

#ifdef BYTESWAP
#include "byteswapdefs.h"
#endif
#include "dbprint.h"      // for DBPRINT, TPRINT
#include "devconf.h"      // for SUN2BW
#include "devif.h"        // for (anonymous), MRegion, DevRec, DspInterface
#include "display.h"      // for DLWORD_PERLINE, DISPLAYBUFFER
#include "ifpage.h"       // for IFPAGE
#include "initdspdefs.h"  // for clear_display, display_before_exit, flush_d...
#include "lispemul.h"     // for DLword, BITSPER_DLWORD, T
#include "lspglob.h"
#include "lsptypes.h"

/* from /usr/include/sun/fbio.h some machines don't have following def. */
#ifndef FBTYPE_SUNROP_COLOR
#define FBTYPE_SUNROP_COLOR 13 /* MEMCOLOR with rop h/w */
#define FBTYPE_SUNFAST_COLOR 12
#endif


int FrameBufferFd = -1;

extern int sdl_displaywidth, sdl_displayheight;
extern unsigned displaywidth, displayheight, DisplayRasterWidth, DisplayType, DisplayByteSize;
unsigned displaywidth, displayheight, DisplayRasterWidth, DisplayType, DisplayByteSize;
DLword *DisplayRegion68k; /* 68k addr of #{}22,0 */

#ifdef DISPLAYBUFFER
/* both vars has same value. That is the end of Lisp DisplayRegion */
DLword *DisplayRegion68k_end_addr;
#endif

/* some functions use this variable when undef DISPLAYBUFFER */
DLword *DISP_MAX_Address;


extern DLword *EmCursorBitMap68K;

int DebugDSP = T;

#ifdef SDL
extern void sdl_notify_damage(int, int, int, int);
#endif /* SDL */


/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void init_cursor(void) {
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void set_cursor(void) {

  DBPRINT(("After Set cursor\n"));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void clear_display(void) {

}


/*  ================================================================  */
/*  Now takes 68k address, function renamed for safety  */

void init_display2(DLword *display_addr, unsigned display_max)
{



  DisplayRegion68k = (DLword *)display_addr;


#if (defined(SDL))
  displaywidth = sdl_displaywidth;
  displayheight = sdl_displayheight;
#endif /* SDL */
  DisplayRasterWidth = displaywidth / BITSPER_DLWORD;

  if ((displaywidth * displayheight) > display_max) { displayheight = display_max / displaywidth; }
  DISP_MAX_Address = DisplayRegion68k + DisplayRasterWidth * displayheight;
  DBPRINT(("FBIOGTYPE w x h = %d x %d\n", displaywidth, displayheight));

  DBPRINT(("FBIOGTYPE w x h = %d x %d\n", displaywidth, displayheight));

#if defined SDL
  DisplayType = SUN2BW;
#endif /* SDL */
  init_cursor();
  DisplayByteSize = ((displaywidth * displayheight / 8 + ((unsigned)getpagesize() - 1)) & (unsigned)-getpagesize()); // TODO

  DBPRINT(("Display address: %p\n", (void *)DisplayRegion68k));
  DBPRINT(("        length : 0x%x\n", DisplayByteSize));
  DBPRINT(("        pg size: 0x%x\n", getpagesize()));


  clear_display();

  DBPRINT(("after clear_display()\n"));


  DBPRINT(("exiting init_display\n"));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/
void display_before_exit(void) {

  clear_display();
}

#if defined(DISPLAYBUFFER)
/************************************************************************/
/*									*/
/*		    i n _ d i s p l a y _ s e g m e n t			*/
/*									*/
/*	Returns T if the base address for this bitblt is in the 	*/
/*	display segment.						*/
/*									*/
/************************************************************************/
/*  Change as MACRO by osamu '90/02/08
 *  new macro definition is in display.h
in_display_segment(baseaddr)
  DLword *baseaddr;
  {
    if ((DisplayRegion68k <= baseaddr) &&
        (baseaddr <=DISP_MAX_Address))   return(T);
    return(NIL);
  }
------------------ */
#endif /* DISPLAYBUFFER */

/************************************************************************/
/*									*/
/*		 f l u s h _ d i s p l a y _ b u f f e r		*/
/*									*/
/*	Copy the entire Lisp display bank to the real frame buffer 	*/
/*	[Needs to be refined for efficiency.]				*/
/*									*/
/************************************************************************/

void flush_display_buffer(void) {
#ifdef SDL
  sdl_notify_damage(0, 0, sdl_displaywidth, sdl_displayheight);
#endif
}

/************************************************************************/
/*									*/
/*		 f l u s h _ d i s p l a y _ r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	x								*/
/*	y								*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/
void flush_display_region(int x, int y, int w, int h)
{
  //  printf("flush_display_region %d %d %d %d\n", x, y, w, h);
#ifdef SDL
  sdl_notify_damage(x, y, w, h);
#endif
}
#ifdef BYTESWAP
void byte_swapped_displayregion(int x, int y, int w, int h)
{
  unsigned int *longptr;

  /* Get QUAD byte aligned pointer */
  longptr = (unsigned int *)(((UNSIGNED)((DLword *)DisplayRegion68k + (DLWORD_PERLINE * y)) +
                              ((x + 7) >> 3)) &
                             0xfffffffc);

  bit_reverse_region((unsigned short *)longptr, w, h, DLWORD_PERLINE);

  return;

} /* byte_swapped_displayregion end */
#endif /* BYTESWAP */

/************************************************************************/
/*									*/
/*	    f l u s h _ d i s p l a y _ l i n e r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	x								*/
/*	ybase the offset from top of bitmap, as the address of the	*/
/*	       first word of the line to start on.			*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/

void flush_display_lineregion(UNSIGNED x, DLword *ybase, int w, int h)
{
  int y;
  y = ((DLword *)ybase - DisplayRegion68k) / DLWORD_PERLINE;
  //  printf("flush_display_lineregion %d %d %d %d\n", x, y, w, h);
#ifdef SDL
  sdl_notify_damage(x, y, w, h);
#endif
}

/************************************************************************/
/*									*/
/*	    f l u s h _ d i s p l a y _ p t r r e g i o n		*/
/*									*/
/*	Copy a region of the Lisp display bank to the real frame 	*/
/*	buffer.								*/
/*									*/
/*	bitoffset  bit offset into word pointed to by ybase		*/
/*	ybase the offset from top of bitmap, as the address of the	*/
/*	       word containing the upper-leftmost bit changed.		*/
/*	w  the width of the piece to display, in pixels			*/
/*	h  the height of the piece to display, in pixels		*/
/*									*/
/************************************************************************/

#define BITSPERWORD 16

void flush_display_ptrregion(DLword *ybase, UNSIGNED bitoffset, int w, int h)
{
  int y, x, baseoffset;
  baseoffset = (((DLword *)ybase) - DisplayRegion68k);
  y = baseoffset / DLWORD_PERLINE;
  x = bitoffset + (BITSPERWORD * (baseoffset - (DLWORD_PERLINE * y)));
  //  printf("flush_display_ptrregion %d %d %d %d\n", x, y, w, h);
#ifdef SDL
  sdl_notify_damage(x, y, w, h);
#endif
}
