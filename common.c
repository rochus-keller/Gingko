/* $Id: common.c,v 1.2 1999/01/03 02:06:52 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved
 */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989, 1990, 1990, 1991, 1992, 1993, 1994, 1995 Venue.	*/
/*	    All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <stdio.h>         // for fflush, fprintf, printf, getchar, stderr
#include <stdlib.h>        // for exit
#include <string.h>        // for memset
#include "commondefs.h"    // for error, stab, warn
#include "dbprint.h"       // for DBPRINT
#include "kprintdefs.h"    // for print
#include "lispemul.h"      // for NIL, DLword, LispPTR
#include "lspglob.h"
#include "tinydir.h"

void stab(void) { DBPRINT(("Now in stab\n")); }

/***************************************************************
error
        common sub-routine.

        Printout error message.
        Enter URAID.
        And exit.(takeshi)

******************************************************************/

extern int LispKbdFd;
extern struct screen LispScreen;
extern int displaywidth, displayheight;
extern DLword *DisplayRegion68k;
extern int FrameBufferFd;
extern int BT_temp; /* holds the continue-character the user typed */

/* Currentry Don't care Ether re-initial */
/* Medley only */

/************************************************************************/
/*									*/
/*				e r r o r				*/
/*									*/
/*	Last-ditch error handling; enters URAID, low-level debug.	*/
/*									*/
/************************************************************************/

LispPTR Uraid_mess = NIL;

int error(const char *cp) {
#if 0
  if (device_before_raid() < 0) {
    (void)fprintf(stderr, "Can't Enter URAID.\n");
    exit(-1);
  }
#endif
  /* comm read */
  (void)fprintf(stderr, "\n*Error* %s\n", cp);
  fflush(stdin);

  exit(-1);
  return (0);
}

/************************************************************************/
/*									*/
/*				w a r n					*/
/*									*/
/*	Print a warning message, but don't stop running.		*/
/*									*/
/************************************************************************/

void warn(const char *s)
{ printf("\nWARN: %s \n", s); }



int file_exists(const char* path)
{
    tinydir_file file;
    return tinydir_file_open(&file, path);
}

int can_read_file(const char* path)
{
    tinydir_file file;
    const int res = tinydir_file_open(&file, path);
    if( res < 0 )
        return res;
    if( !(file._s.st_mode & S_IRUSR) )
    {
        errno = EACCES;
        return -1;
    }
    return 0;
}
