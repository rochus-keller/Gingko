#ifndef LISPMAP_H
#define LISPMAP_H 1

/* $Id: lispmap.h,v 1.3 1999/01/03 02:06:08 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */


/************************************************************************/
/*									*/
/*	(C) Copyright 1989-98 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/
/*
		File Name :	lispmap.h(for TEST)

		**************NOTE*****************
		OLD DEFs are MOVED to lispmap.FULL
		**************NOTE*****************

		Global variables for LispSYSOUT

					Date :		December 18, 1986
					Edited by :	Takeshi Shimizu

*/

// RK: streamlined and reorganized

/* for ATOMSPACE */
#define ATOM_OFFSET	0x00000
#define ATOM_HI	      0

/* for IOPAGE */
#define IOPAGE_OFFSET	0x0FF00

/* for STACKSPACE */
#define STK_OFFSET	0x10000
#define STK_HI		      1

#define PLIS_HI			2 /* place holder, really -- keep the old value, even though it's inconsistent with the OFFSET, because it's known by LISP, and is used as a dispatch constant. */

#define FPTOVP_HI		4	/* again, inconsistent with OFFSET. */

/* for PNPSPACE */
/* Now used to hold initial atoms */
#define PNP_OFFSET	0x80000
#define PNP_HI		      8        /* Fake */

#define ATOMS_SIZE      0x20000

/* for DEFSPACE */
#define DEFS_HI		     10
#define DEFS_OFFSET	0xA0000

/* for VALSPACE */
#define VALS_HI		     12
#define VALS_OFFSET	0xC0000

/* for Small Positive */
#define SPOS_HI		     14
#define S_POSITIVE	0xE0000

/* for Small Negative */
#define S_NEGATIVE	0xF0000

#define S_CHARACTER	0x70000

/* DISPLAYREGION */

#define DISPLAY_HI	     18
#define DISPLAY_OFFSET 0x120000


#ifdef BIGBIGVM

/**********************************************/
/*                                            */
/*    BIG-BIG-VM sysout layout (256Mb sysout) */
/*                                            */
/**********************************************/


/* for PLISTSPACE */
#define PLIS_OFFSET 0x30000

#define FPTOVP_OFFSET 0x20000


/* for InterfacePage */
#define IFPAGE_OFFSET	0x140000

/* for MISCSTATS */
#define MISCSTATS_OFFSET  0x140A00

/* for UFNTable */
#define UFNTBL_OFFSET	0x140C00

/* for DTDspace */
#define DTD_OFFSET	0x141000

/* for MDSTT */

#define MDS_OFFSET	0x180000


/* for AtomHashTable */
#define ATMHT_OFFSET	0x150000

#define ATOMS_HI		     44
#define ATOMS_OFFSET    0x2c0000

/* for HTMAIN */
#define HTMAIN_OFFSET  0x160000
#define HTMAIN_SIZE	 0x10000

/* for HTOVERFLOW */
#define HTOVERFLOW_OFFSET 0x170000

/* for HTBIGCOUNT */
#define HTBIG_OFFSET   0x170100

/* for HTCOLL */
#define HTCOLL_OFFSET  0x1C0000
#define HTCOLL_SIZE	0x100000

#else

/* NOT BIG-BIG VM */


/* for PLISTSPACE */
#ifndef BIGVM
#define PLIS_OFFSET	0x20000
#else
#define PLIS_OFFSET 0x30000
#endif

#ifdef BIGVM
#define FPTOVP_OFFSET 0x20000
#else
#define FPTOVP_OFFSET	0x40000
#endif /* BIGVM */

/* for InterfacePage */
#define IFPAGE_OFFSET	0x60000

#define MISCSTATS_OFFSET  0x60A00

/* for UFNTable */
#define UFNTBL_OFFSET	0x60C00

/* for DTDspace */
#define DTD_OFFSET	0x61000

/* for MDSTT */
#ifdef BIGVM
    /* In BIGVM, MDS type table is at 19.,,0 for 1 segment */
#define MDS_OFFSET	0x140000
#else
#define MDS_OFFSET	0x68000
#endif /* BIGVM */

/* for AtomHashTable */
#define ATMHT_OFFSET	0x70000

#define ATOMS_HI	      8
#define ATOMS_OFFSET    0x80000

#define HTMAIN_OFFSET  0x100000

#ifdef BIGVM
#define HTMAIN_SIZE	 0x10000
#define HTOVERFLOW_OFFSET 0x110000
#define HTBIG_OFFSET   0x110100
#define HTCOLL_OFFSET  0xA0000
#define HTCOLL_SIZE	0x40000
#else
#define HTMAIN_SIZE	 0x8000
#define HTOVERFLOW_OFFSET 0x108000
#define HTBIG_OFFSET   0x108100
#define HTCOLL_OFFSET  0x110000
#define HTCOLL_SIZE	0x10000
#endif /* BIGVM */


#endif /* BIGBIGVM */



#endif
