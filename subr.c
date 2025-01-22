/* $Id: subr.c,v 1.3 1999/05/31 23:35:42 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/***********************************************************/
/*
                File Name :	subr.c
                Including	:	OP_subrcall

                Created	:	May 12, 1987 Takeshi Shimizu
                Changed :	May 15 87 take
                Changed :	Jun 2 87 NMitani
                                Jun. 5 87 take
                                Jun. 29 87 NMitani
                                Oct. 13 87 Hayata
                                Oct. 16 87 take
                                Nov. 18 87 Matsuda
                                Dec. 17 1987 Tomtom
                                2/17/89 Sybalsky (Add SXHASH)

*/
/***********************************************************/

#include <stdio.h>         // for printf, sprintf, NULL
#include <time.h>          // for nanosleep, timespec
#include "adr68k.h"        // for NativeAligned2FromLAddr, NativeAligned4FromLAddr
#include "arith.h"         // for N_GETNUMBER, ARITH_SWITCH
#include "bbtsubdefs.h"    // for bitblt_bitmap, bitbltsub, bitshade_bitmap
#include "cell.h"          // for PNCell, GetPnameCell
#include "chardevdefs.h"   // for CHAR_bin, CHAR_bins, CHAR_bout, CHAR_bouts
#include "commondefs.h"    // for error
#include "dbprint.h"       // for DBPRINT
#include "dirdefs.h"       // for COM_finish_finfo, COM_gen_files, COM_next_...
#include "dskdefs.h"       // for COM_changedir, COM_closefile, COM_getfileinfo
#include "dspsubrsdefs.h"  // for DSP_Cursor, DSP_ScreenHight, DSP_ScreenWidth
#include "gcarraydefs.h"   // for with_symbol
#include "gcrdefs.h"       // for disablegc1, doreclaim
#include "inetdefs.h"      // for subr_TCP_ops
#include "lispemul.h"      // for state, LispPTR, NIL_PTR, PopStackTo, TopOf...
#include "lispmap.h"       // for S_POSITIVE
#include "lspglob.h"
#include "lsptypes.h"
#include "storagedefs.h"   // for newpage
#include "subrdefs.h"      // for OP_subrcall, atom_to_str
#include "subrs.h"         // for sb_BITBLTSUB, sb_BITBLT_BITMAP, sb_BLTCHAR
#include "timerdefs.h"     // for subr_copytimestats, subr_gettime, subr_set...
#include "ufsdefs.h"       // for UFS_deletefile, UFS_directorynamep, UFS_ge...
#include "uutilsdefs.h"    // for suspend_lisp, check_unix_password, unix_fu...
#include "vmemsavedefs.h"  // for lisp_finish, vmem_save0

extern LispPTR *PENDINGINTERRUPT68k;

/***********************************************************/
/*

        Func Name  :	OP_subrcall

        Last Modify :	13-Oct 1987 take

*/
/***********************************************************/

#define POP_SUBR_ARGS                                       \
  do {                                                      \
    args[0] = NIL_PTR;                                      \
    if ((arg_num = (argnum /* = (Get_BYTE(PC+2))*/)) > 0) { \
      while (arg_num > 0) PopStackTo(args[--arg_num]);      \
    }                                                       \
  } while(0)

void OP_subrcall(int subr_no, int argnum) {
  static LispPTR args[30];
  int arg_num;

  PushCStack; /* save TOS in memory */

  DBPRINT(("Subr call to subr 0%o.\n", subr_no));

  switch (subr_no) {
    case sb_SHOWDISPLAY:
      POP_SUBR_ARGS;
      DSP_showdisplay(args);
      break; /* showdisplay */

    case sb_DSPBOUT:
      POP_SUBR_ARGS;
      DSP_dspbout(args);
      break; /*dspbout */

    case sb_RAID:
      POP_SUBR_ARGS;
      //Uraid_mess = args[0];
      PC += 3; /* for the case of hardreset */
      error("URAID Called:");
      //Uraid_mess = NIL;
      TopOfStack = NIL;
      return; /* Direct return;avoid to increment PC */
              /* break; */
    /*************************/
    /* for Local File System */
    /*************************/
    case sb_COM_OPENFILE:
      POP_SUBR_ARGS;
      TopOfStack = COM_openfile(args);
      break;

    case sb_COM_CLOSEFILE:
      POP_SUBR_ARGS;
      TopOfStack = COM_closefile(args);
      break;

    case sb_UFS_GETFILENAME:
      POP_SUBR_ARGS;
      TopOfStack = UFS_getfilename(args);
      break;

    case sb_UFS_DELETEFILE:
      POP_SUBR_ARGS;
      TopOfStack = UFS_deletefile(args);
      break;

    case sb_UFS_RENAMEFILE:
      POP_SUBR_ARGS;
      TopOfStack = UFS_renamefile(args);
      break;

    case sb_COM_READPAGES:
      POP_SUBR_ARGS;
      TopOfStack = COM_readpage(args);
      break;

    case sb_COM_WRITEPAGES:
      POP_SUBR_ARGS;
      TopOfStack = COM_writepage(args);
      break;

    case sb_COM_TRUNCATEFILE:
      POP_SUBR_ARGS;
      TopOfStack = COM_truncatefile(args);
      break;

    case sb_COM_NEXT_FILE:
      POP_SUBR_ARGS;
      TopOfStack = COM_next_file(args);
      break;

    case sb_COM_FINISH_FINFO:
      POP_SUBR_ARGS;
      TopOfStack = COM_finish_finfo(args);
      break;

    case sb_COM_GEN_FILES:
      POP_SUBR_ARGS;
      TopOfStack = COM_gen_files(args);
      break;

    case sb_UFS_DIRECTORYNAMEP:
      POP_SUBR_ARGS;
      TopOfStack = UFS_directorynamep(args);
      break;

    case sb_COM_GETFILEINFO:
      POP_SUBR_ARGS;
      TopOfStack = COM_getfileinfo(args);
      break;

    case sb_COM_CHANGEDIR:
      POP_SUBR_ARGS;
      TopOfStack = COM_changedir(args);
      break;

    case sb_COM_GETFREEBLOCK:
      POP_SUBR_ARGS;
      TopOfStack = COM_getfreeblock(args);
      break;

    case sb_COM_SETFILEINFO:
      POP_SUBR_ARGS;
      TopOfStack = COM_setfileinfo(args);
      break;

    /*************/
    /* for Timer */
    /*************/
    case sb_SETUNIXTIME:
      POP_SUBR_ARGS;
      subr_settime(args);
      /* don't know whether it worked or not */
      TopOfStack = NIL;
      break;

    case sb_GETUNIXTIME:
      POP_SUBR_ARGS;
      TopOfStack = subr_gettime(args);
      break;

    case sb_COPYTIMESTATS:
      POP_SUBR_ARGS;
      subr_copytimestats(args);
      /* no result */
      TopOfStack = NIL; 
      break;

    /*************/
    /* for Ether */
    /*************/
    case sb_CHECK_SUM:
      POP_SUBR_ARGS;
      TopOfStack = S_POSITIVE; // check_sum(args);
      break;

    case sb_ETHER_SUSPEND:
      POP_SUBR_ARGS;
      TopOfStack = ATOM_T; // ether_suspend(args);
      break;

    case sb_ETHER_RESUME:
      POP_SUBR_ARGS;
      TopOfStack = ATOM_T; // ether_resume(args);
      break;

    case sb_ETHER_AVAILABLE:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // ether_ctrlr(args);
      break;

    case sb_ETHER_RESET:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // ether_reset(args);
      break;

    case sb_ETHER_GET:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // ether_get(args);
      break;

    case sb_ETHER_SEND:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // ether_send(args);
      break;

    case sb_ETHER_SETFILTER:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // ether_setfilter(args);
      break;

    case sb_ETHER_CHECK:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // check_ether();
      break;

    /***************/
    /* for Display */
    /***************/
    case sb_DSPCURSOR:
      POP_SUBR_ARGS;
      DSP_Cursor(args, argnum);
      break;

    case sb_SETMOUSEXY:
      POP_SUBR_ARGS;
      DSP_SetMousePos(args);
      break;

    case sb_DSP_VIDEOCOLOR:
      POP_SUBR_ARGS;
      TopOfStack = DSP_VideoColor(args);
      break;

    case sb_DSP_SCREENWIDTH:
      POP_SUBR_ARGS;
      TopOfStack = DSP_ScreenWidth(args);
      break;

    case sb_DSP_SCREENHEIGHT:
      POP_SUBR_ARGS;
      TopOfStack = DSP_ScreenHight(args);
      break;

    /***************************/
    /***  bitbltsub, bltchar ***/
    /***************************/
    case sb_BITBLTSUB:
      POP_SUBR_ARGS;
      bitbltsub(args);
      break;

    case sb_BLTCHAR:
      POP_SUBR_ARGS; /* argnum * DLwordsperCell*/
      bltchar(args);
      break;

    case sb_NEW_BLTCHAR:
      POP_SUBR_ARGS;
      newbltchar(args);
      break;

    case sb_TEDIT_BLTCHAR:
      POP_SUBR_ARGS;
      tedit_bltchar(args);
      break;

    /*	case 209: JDS 4 may 91 - this is code for CHAR-FILLBUFFER?? */
    case sb_BITBLT_BITMAP:
      POP_SUBR_ARGS; /* BITBLT to a bitmap */
      TopOfStack = bitblt_bitmap(args);
      break;

    case 0111 /*sb_BITSHADE_BITMAP*/:
      POP_SUBR_ARGS; /* BITSHADE to a bitmap */
      TopOfStack = bitshade_bitmap(args);
      break;


    /***********/
    /* for K/B */
    /***********/
    case sb_KEYBOARDBEEP:
      POP_SUBR_ARGS;
      // not implemented
      break;

    case sb_KEYBOARDMAP:
      POP_SUBR_ARGS;
      // not implemented
      break;

    case sb_KEYBOARDSTATE:
      POP_SUBR_ARGS;
      if (args[0] == ATOM_T) {
      } else if (args[0] == NIL) {
      } else {
        error("KB_enable: illegal arg \n");
        printf("KB_enable: arg = %d\n", args[0]);
      }
      break;

    case sb_VMEMSAVE:
      POP_SUBR_ARGS;
      TopOfStack = vmem_save0(args);
      break;

    case sb_LISPFINISH:
    case sb_LISP_FINISH:
      POP_SUBR_ARGS;
      if ((argnum > 0) && (args[0] == S_POSITIVE))
      /* 8/03/88 This branch impossible to take, subr has no args */
      {
        TopOfStack = suspend_lisp(args);
      } else
        lisp_finish();
      break;

    case sb_NEWPAGE:
      POP_SUBR_ARGS;
      TopOfStack = newpage(args[0]);
      break;

    case sb_DORECLAIM:
      POP_SUBR_ARGS;
      doreclaim(); /* top-level GC function */
      TopOfStack = NIL_PTR;
      break;

    /* read & write a abs memory address */

    case sb_NATIVE_MEMORY_REFERENCE:
      POP_SUBR_ARGS;

      switch (args[0] & 0xffff) {
        case 00: {
          UNSIGNED iarg;
          if (argnum != 2) goto ret_nil;
          N_GETNUMBER(args[1], iarg, ret_nil);
          ARITH_SWITCH(*((LispPTR *)iarg), TopOfStack);
          break;
        }

        case 01: {
          UNSIGNED iarg, iarg2;
          if (argnum != 3) goto ret_nil;
          N_GETNUMBER(args[1], iarg, ret_nil);
          N_GETNUMBER(args[2], iarg2, ret_nil);
          *((LispPTR *)iarg) = iarg2;
          break;
        }

        /* case 02: Used to be get an emulator address for
         * defunct NATIVETRAN feature. */
      }
      break;

    ret_nil:
      TopOfStack = NIL_PTR;
      break;

    case sb_DISABLEGC:
      POP_SUBR_ARGS;
      disablegc1(NIL);
      TopOfStack = NIL_PTR;
      break;

    case sb_GET_NATIVE_ADDR_FROM_LISP_PTR:
      POP_SUBR_ARGS;
      /* XXX: this WILL NOT WORK if Lisp memory is allocated outside the low 4GB */
      /* not supported since native addresses can't be represented as
         a Lisp FIXP
      ARITH_SWITCH(NativeAligned2FromLAddr(args[0]), TopOfStack);
      */
      TopOfStack = NIL_PTR;
      break;

    case sb_GET_LISP_PTR_FROM_NATIVE_ADDR:
      POP_SUBR_ARGS;
      /* not supported since native addresses can't be represented as
         a Lisp FIXP

      {
        UNSIGNED iarg;
        N_GETNUMBER(args[0], iarg, ret_nil);
        ARITH_SWITCH(LAddrFromNative(iarg), TopOfStack);
        break;
      }
      */
      TopOfStack = NIL_PTR;
      break;
    case sb_DSK_GETFILENAME:
      POP_SUBR_ARGS;
      TopOfStack = DSK_getfilename(args);
      break;

    case sb_DSK_DELETEFILE:
      POP_SUBR_ARGS;
      TopOfStack = DSK_deletefile(args);
      break;

    case sb_DSK_RENAMEFILE:
      POP_SUBR_ARGS;
      TopOfStack = DSK_renamefile(args);
      break;

    case sb_DSK_DIRECTORYNAMEP:
      POP_SUBR_ARGS;
      TopOfStack = DSK_directorynamep(args);
      break;

    /* Communications with Unix Subprocess */

    case sb_UNIX_HANDLECOMM: POP_SUBR_ARGS;
      TopOfStack = NIL; // not implemented
      break;

    case sb_MESSAGE_READP:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // not implemented
      break;

    case sb_MESSAGE_READ:
      POP_SUBR_ARGS;
      TopOfStack = NIL; // not implemented
      break;

    /* RPC routines */

    case sb_RPC_CALL:
      POP_SUBR_ARGS;
      TopOfStack = NIL_PTR; // not implemented
      break;

    /* Unix username/password utilities */

    case sb_CHECKBCPLPASSWORD:
      POP_SUBR_ARGS;
      /* Check Unix username/password */
      TopOfStack = check_unix_password(args);
      break;

    case sb_UNIX_USERNAME:
      POP_SUBR_ARGS;
      /* Get Unix username */
      TopOfStack = unix_username(args);
      break;

    case sb_UNIX_FULLNAME:
      POP_SUBR_ARGS;
      /* Get Unix person-name (GECOS field) */
      TopOfStack = unix_fullname(args);
      break;

    case sb_UNIX_GETENV:
      POP_SUBR_ARGS;
      /* get value of environment variable, or NIL */
      TopOfStack = unix_getenv(args);
      break;

    case sb_UNIX_GETPARM:
      POP_SUBR_ARGS;
      /* get built in parameter */
      TopOfStack = unix_getparm(args);
      break;

    case sb_SUSPEND_LISP:
      POP_SUBR_ARGS;
      /* Suspend Maiko */
      TopOfStack = suspend_lisp(args);
      break;

    case sb_MONITOR_CONTROL:
      POP_SUBR_ARGS;
      break;

    /*****************/
    /* Character I/O */
    /*****************/
    case sb_CHAR_OPENFILE:
      POP_SUBR_ARGS; /* Char-device openfile. */
      TopOfStack = CHAR_openfile(args);
      break;

    case sb_CHAR_BIN:
      POP_SUBR_ARGS; /* Char-device bin. */
      TopOfStack = CHAR_bin(args[0], args[1]);
      break;

    case sb_CHAR_BOUT:
      POP_SUBR_ARGS; /* Char-device bout. */
      TopOfStack = CHAR_bout(args[0], args[1], args[2]);
      break;

    case sb_CHAR_IOCTL:
      POP_SUBR_ARGS; /* Char-device IOCTL. */
      TopOfStack = CHAR_ioctl(args);
      break;

    case sb_CHAR_CLOSEFILE:
      POP_SUBR_ARGS; /* Char-device CLOSEFILE. */
      TopOfStack = CHAR_closefile(args);
      break;

    case sb_CHAR_BINS:
      POP_SUBR_ARGS; /* Char-device \BINS. */
      TopOfStack = CHAR_bins(args);
      break;

    case sb_CHAR_BOUTS:
      POP_SUBR_ARGS; /* Char-device \BOUTS. */
      TopOfStack = CHAR_bouts(args);
      break;

    case sb_TCP_OP:
      POP_SUBR_ARGS; /* TCP operations */
      TopOfStack = subr_TCP_ops(args[0], args[1], args[2], args[3], args[4], args[5]);
      break;

    case sb_PUPLEVEL1STATE:
      POP_SUBR_ARGS; /* Do nothing with PUP on sun */
      break;

    case sb_WITH_SYMBOL:
      POP_SUBR_ARGS; /* Symbol lookup */
      TopOfStack = with_symbol(args[0], args[1], args[2], args[3], args[4], args[5]);
      break;

    case sb_CAUSE_INTERRUPT: /* Cause an interrupt to occur.  Used by */
               /* Lisp INTERRUPTED to re-set an interrupt */
               /* when it's uninterruptible. 		*/
      POP_SUBR_ARGS;
      Irq_Stk_Check = Irq_Stk_End = 0;
      *PENDINGINTERRUPT68k = ATOM_T;
      TopOfStack = ATOM_T;
      break;

  case sb_YIELD: {
      struct timespec rqts = {0, 833333};
      unsigned sleepnanos;
      POP_SUBR_ARGS;
      N_GETNUMBER(args[0], sleepnanos, ret_nil);
      if (sleepnanos > 999999999) {
          TopOfStack = NIL;
          break;
      }
      rqts.tv_nsec = sleepnanos;
      nanosleep(&rqts, NULL);
      TopOfStack = ATOM_T;
      break;
  }
    default: {
      char errtext[200];
      sprintf(errtext, "OP_subrcall: Invalid alpha byte 0%o", ((*(PC + 1)) & 0xff));
      printf("%s\n", errtext);
      error(errtext);
      break;
    }

  } /* switch end */

  PC += 3; /* Move PC forward to next opcode */

} /* OP_subrcall */
