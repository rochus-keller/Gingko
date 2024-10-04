/* $Id: xc.c,v 1.4 2001/12/26 22:17:06 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved  */

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-95 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

/************************************************************************/
/*									*/
/*		  M A I N   D I S P A T C H   L O O P			*/
/*									*/
/*	This file contains the main dispatch loop for the emulator.	*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifdef MAIKO_OS_EMSCRIPTEN
#include <emscripten.h>
#endif
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#ifdef DOS
#include <i32.h> /* Defines "#pragma interrupt"  */
#include <stk.h> /* _XSTACK struct definition    */
#include <dos.h> /* Defines REGS & other structs */
#else            /* DOS */
#include <sys/time.h>
#endif /* DOS */

#include "lispemul.h"
#include "emlglob.h"
#include "address.h"
#include "adr68k.h"
#include "stack.h"
#include "return.h"
#include "dbprint.h"

#include "lspglob.h"
#include "lsptypes.h"
#include "lispmap.h"
#include "cell.h"
#include "initatms.h"
#include "gcdata.h"
#include "arith.h"
#include "stream.h"

#include "testtooldefs.h"
#include "tos1defs.h"
#include "tosret.h"
#include "tosfns.h"
#include "inlineC.h"

#include "xcdefs.h"
#include "arithopsdefs.h"
#include "arrayopsdefs.h"
#include "bitbltdefs.h"
#include "bltdefs.h"
#include "byteswapdefs.h"
#include "car-cdrdefs.h"
#include "commondefs.h"
#include "conspagedefs.h"
#include "drawdefs.h"
#include "eqfdefs.h"
#include "devif.h"
#include "findkeydefs.h"
#include "fpdefs.h"
#include "fvardefs.h"
#include "gchtfinddefs.h"
#include "gcscandefs.h"
#include "gvar2defs.h"
#include "hardrtndefs.h"
#include "ifpage.h"
#include "intcalldefs.h"
#include "keyeventdefs.h"
#include "llstkdefs.h"
#include "lowlev2defs.h"
#include "lsthandldefs.h"
#include "misc7defs.h"
#include "miscndefs.h"
#include "mkcelldefs.h"
#include "returndefs.h"
#include "rplconsdefs.h"
#include "shiftdefs.h"
#include "subrdefs.h"
#include "timerdefs.h"
#include "typeofdefs.h"
#include "ubf1defs.h"
#include "ubf2defs.h"
#include "ubf3defs.h"
#include "unwinddefs.h"
#include "vars3defs.h"
#ifdef XWINDOW
#include "xwinmandefs.h"
#endif
#include "z2defs.h"

#ifdef DOS
#include "devif.h"
extern KbdInterface currentkbd;
extern DspInterface currentdsp;
extern MouseInterface currentmouse;
#elif defined(XWINDOW)
extern DspInterface currentdsp;
#endif /* DOS */

#ifdef SDL
extern void process_SDLevents();
#endif

typedef struct conspage ConsPage;
typedef ByteCode *InstPtr;

/* This used to be here for including optimized dispatch
 * for SPARC, but it has some other things in it, so we
 * are keeping it around for now until we sort that out. */
#ifdef SPARCDISP
#include "inlnSPARC.h"
#endif /* SPARCDISP */

/* trick now is that pccache points one ahead... */
#define PCMAC (pccache - 1)
#define PCMACL pccache
#define CSTKPTR ((LispPTR *)cspcache)
#define PVAR ((LispPTR *)PVar)
#define IVAR ((LispPTR *)IVar)
#define BCE_CURRENTFX ((struct frameex2 *)((DLword *)PVAR - FRAMESIZE))

/* Define alternative macros for CSTKPTR, PVAR, and IVAR that can be used
 * in an lvalue context, since CSTKPTR = ...; would generate
 * error: assignment to cast is illegal, lvalue casts are not supported
 */

#define CSTKPTRL (cspcache)
#define PVARL PVar
#define IVARL IVar

/* used by SIGIO signal handler to indicate I/O may be possible */
extern volatile sig_atomic_t IO_Signalled;

#ifdef PCTRACE
/* For keeping a trace table (ring buffer) of 100 last PCs */
int pc_table[100],     /* The PC */
    op_table[100];     /* The opcode at that PC */
LispPTR fn_table[100]; /* The code block the PC is in (Lisp ptr) */
int pccounter = 0;     /* ring-buffer counter */
#endif                 /* PCTRACE */

extern int extended_frame;
int extended_frame; /*indicates if soft stack overflow */

static const int n_mask_array[16] = {
    1,     3,     7,     0xf,   0x1f,   0x3f,   0x7f,   0xff,
    0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

extern int TIMER_INTERVAL;

#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)

#  if !defined(MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN)
#    define MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN 20000
#  endif

int insnsCountdownForTimerAsyncEmulation = MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN;
static int pseudoTimerAsyncCountdown = MAIKO_TIMER_ASYNC_EMULATION_INSNS_COUNTDOWN;

#endif

void dispatch(void) {
  InstPtr pccache;
  LispPTR *cspcache;
  LispPTR tscache;

  /* OP_FN_COMMON arguments */

  DefCell *fn_defcell;
  LispPTR fn_atom_index;
  int fn_opcode_size;
  int fn_num_args;
  int fn_apply;

  RET;
  CLR_IRQ;

  goto nextopcode;

  /* OPCODE FAIL ENTRY POINTS, CALL UFNS HERE */
  UFN_CALLS

op_ufn : {
  UFN *entry68k;
  entry68k = (UFN *)GetUFNEntry(Get_BYTE_PCMAC0);
  fn_num_args = entry68k->arg_num;
  fn_opcode_size = entry68k->byte_num + 1;
  fn_atom_index = entry68k->atom_name;
  fn_defcell = (DefCell *)GetDEFCELL68k(fn_atom_index);
  fn_apply = 2 + entry68k->byte_num; /* code for UFN entry */
  goto op_fn_common;
}

  /* FUNCTION CALL TAIL ROUTINE */

  OP_FN_COMMON

/* DISPATCH "LOOP" */

nextopcode:
#ifdef MYOPTRACE
  if ((struct fnhead *)NativeAligned4FromLAddr(0x2ed600) == FuncObj) {
    quick_stack_check();
#endif /* MYOPTRACE */
    OPTPRINT(("PC= %p (fn+%td) op= %02x TOS= 0x%x\n", (void *)PCMAC, PCMAC - (char *)FuncObj, Get_BYTE_PCMAC0, TOPOFSTACK));
#ifdef MYOPTRACE
  }
#endif /* MYOPTRACE */

#ifdef PCTRACE
  /* Tracing PC/Function/Opcode in a ring buffer */
  pc_table[pccounter] = (int)PCMAC - (int)FuncObj;
  fn_table[pccounter] = (LispPTR)LAddrFromNative(FuncObj);
  op_table[pccounter] = Get_BYTE_PCMAC0;
  if (99 == pccounter++) pccounter = 0;
#endif /* PCTRACE */

  /* quick_stack_check();*/ /* JDS 2/12/98 */
  
#if defined(MAIKO_EMULATE_TIMER_INTERRUPTS) || defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
  if (--pseudoTimerAsyncCountdown <= 0) {
	  Irq_Stk_Check = 0;
	  Irq_Stk_End = 0;
#if defined(MAIKO_EMULATE_ASYNC_INTERRUPTS)
	  IO_Signalled = TRUE;
#endif
#ifdef MAIKO_OS_EMSCRIPTEN
	  emscripten_sleep(1);
#endif
	  pseudoTimerAsyncCountdown = insnsCountdownForTimerAsyncEmulation;
  }
#endif

  switch (Get_BYTE_PCMAC0) {
    case 000: { goto op_ufn; } /* unused */
    case 001:
      OPCAR;
    case 002:
      OPCDR;
    case 003:
      LISTP;
    case 004:
      NTYPEX;
    case 005:
      TYPEP(Get_BYTE_PCMAC1);
    case 056:
    case 006:
      DTEST(Get_AtomNo_PCMAC1);
    case 007:
      UNWIND(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 010:
      FN0;
    case 011:
      FN1;
    case 012:
      FN2;
    case 013:
      FN3;
    case 014:
      FN4;
    case 015:
      FNX;
    case 016:
      APPLY;

    case 017:
      CHECKAPPLY;
    case 020:
      RETURN;

    case 021:
      /* UB: left shift of negative value -4 */
      BIND;
    case 022:
      UNBIND;
    case 023:
      DUNBIND;
    case 024:
      RPLPTR(Get_BYTE_PCMAC1);
    case 025:
      GCREF(Get_BYTE_PCMAC1);
    case 026:
      ASSOC;
    case 027:
      GVAR_(Get_AtomNo_PCMAC1);
    case 030:
      RPLACA;
    case 031:
      RPLACD;
    case 032:
      CONS;
    case 033:
      CLASSOC;
    case 034:
      FMEMB;
    case 035:
      CLFMEMB;
    case 036:
      FINDKEY(Get_BYTE_PCMAC1);
    case 037:
      CREATECELL;
    case 040:
      BIN;
    case 041: { goto op_ufn; } /* BOUT */
    case 042: { goto op_ufn; } /* POPDISP - prolog only */
    case 043:
      RESTLIST(Get_BYTE_PCMAC1);
    case 044:
      MISCN(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 045: { goto op_ufn; } /* unused */
    case 046:
      RPLCONS;
    case 047:
      LISTGET;
    case 050: { goto op_ufn; } /* unused */
    case 051: { goto op_ufn; } /* unused */
    case 052: { goto op_ufn; } /* unused */
    case 053: { goto op_ufn; } /* unused */
    case 054:
      EVAL;
    case 055:
      ENVCALL;

    /*  case 056 : case056: @ 006 */
    case 057:
      STKSCAN;
    case 060:{ goto op_ufn; } /* BUSBLT - DLion only */
    case 061:{ goto op_ufn; } /* MISC8 - no longer used */
    case 062:
      UBFLOAT3(Get_BYTE_PCMAC1);
    case 063:
      TYPEMASK(Get_BYTE_PCMAC1);
    case 064: { goto op_ufn; } /* rdprologptr */
    case 065: { goto op_ufn; } /* rdprologtag */
    case 066: { goto op_ufn; } /* writeptr&tag */
    case 067: { goto op_ufn; } /* writeptr&0tag */
    case 070:
      MISC7(Get_BYTE_PCMAC1); /* misc7 (pseudocolor, fbitmapbit) */
    case 071: { goto op_ufn; } /* dovemisc - dove only */
    case 072:
      EQLOP;
    case 073:
      DRAWLINE;
    case 074:
      STOREN(Get_BYTE_PCMAC1);
    case 075:
      COPYN(Get_BYTE_PCMAC1);
    case 076: { goto op_ufn; } /* RAID */
    case 077: { goto op_ufn; } /* \RETURN */

    case 0100:
      IVARMACRO(0);
    case 0101:
      IVARMACRO(1);
    case 0102:
      IVARMACRO(2);
    case 0103:
      IVARMACRO(3);
    case 0104:
      IVARMACRO(4);
    case 0105:
      IVARMACRO(5);
    case 0106:
      IVARMACRO(6);
    case 0107:
      IVARX(Get_BYTE_PCMAC1);

    case 0110:
      PVARMACRO(0);
    case 0111:
      PVARMACRO(1);
    case 0112:
      PVARMACRO(2);
    case 0113:
      PVARMACRO(3);
    case 0114:
      PVARMACRO(4);
    case 0115:
      PVARMACRO(5);
    case 0116:
      PVARMACRO(6);

    case 0117:
      PVARX(Get_BYTE_PCMAC1);

    case 0120:
      FVAR(0);
    case 0121:
      FVAR(2);
    case 0122:
      FVAR(4);
    case 0123:
      FVAR(6);
    case 0124:
      FVAR(8);
    case 0125:
      FVAR(10);
    case 0126:
      FVAR(12);
    case 0127:
      FVARX(Get_BYTE_PCMAC1);

    case 0130:
      PVARSETMACRO(0);
    case 0131:
      PVARSETMACRO(1);
    case 0132:
      PVARSETMACRO(2);
    case 0133:
      PVARSETMACRO(3);
    case 0134:
      PVARSETMACRO(4);
    case 0135:
      PVARSETMACRO(5);
    case 0136:
      PVARSETMACRO(6);

    case 0137:
      PVARX_(Get_BYTE_PCMAC1);

    case 0140:
      GVAR(Get_AtomNo_PCMAC1);
    case 0141:
      ARG0;
    case 0142:
      IVARX_(Get_BYTE_PCMAC1);
    case 0143:
      FVARX_(Get_BYTE_PCMAC1);
    case 0144:
      COPY;
    case 0145:
      MYARGCOUNT;
    case 0146:
      MYALINK;

    /******** Aconst	********/
    case 0147: {
      PUSH(Get_AtomNo_PCMAC1);
      nextop_atom;
    }
    case 0150: { PUSHATOM(NIL_PTR); }
    case 0151: { PUSHATOM(ATOM_T); }
    case 0152: { PUSHATOM(S_POSITIVE); } /* '0 */
    case 0153: { PUSHATOM(0xE0001); } /* '1 */

    /********* SIC		********/
    case 0154: {
      PUSH(S_POSITIVE | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SNIC		********/
    case 0155: {
      PUSH(S_NEGATIVE | 0xff00 | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SICX		********/
    case 0156: {
      PUSH(S_POSITIVE | Get_DLword_PCMAC1);
      nextop3;
    }

    /********* GCONST	********/
    case 0157: {
      PUSH(Get_Pointer_PCMAC1);
      nextop_ptr;
    }

    case 0160: { goto op_ufn; } /* unused */
    case 0161: { goto op_ufn; } /* readflags */
    case 0162: { goto op_ufn; } /* readrp */
    case 0163: { goto op_ufn; } /* writemap */
    case 0164: { goto op_ufn; } /* readprinterport */
    case 0165: { goto op_ufn; } /* writeprinterport */

    case 0166:
      PILOTBITBLT;
    case 0167:
      RCLK;
    case 0170: { goto op_ufn; } /* MISC1, dorado only */
    case 0171: { goto op_ufn; } /* MISC2, dorado only */
    case 0172:
      RECLAIMCELL;
    case 0173:
      GCSCAN1;
    case 0174:
      GCSCAN2;
    case 0175: {
      EXT;
      OP_subrcall(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
      RET;
      nextop0;
    }
    case 0176: { CONTEXTSWITCH; }
    case 0177: { goto op_ufn; } /* RETCALL */

    /* JUMP */

    case 0200: { JUMPMACRO(2); }
    case 0201: { JUMPMACRO(3); }
    case 0202: { JUMPMACRO(4); }
    case 0203: { JUMPMACRO(5); }
    case 0204: { JUMPMACRO(6); }
    case 0205: { JUMPMACRO(7); }
    case 0206: { JUMPMACRO(8); }
    case 0207: { JUMPMACRO(9); }
    case 0210: { JUMPMACRO(10); }
    case 0211: { JUMPMACRO(11); }
    case 0212: { JUMPMACRO(12); }
    case 0213: { JUMPMACRO(13); }
    case 0214: { JUMPMACRO(14); }
    case 0215: { JUMPMACRO(15); }
    case 0216: { JUMPMACRO(16); }
    case 0217: { JUMPMACRO(17); }

    /* FJUMP */

    case 0220: { FJUMPMACRO(2); }
    case 0221: { FJUMPMACRO(3); }
    case 0222: { FJUMPMACRO(4); }
    case 0223: { FJUMPMACRO(5); }
    case 0224: { FJUMPMACRO(6); }
    case 0225: { FJUMPMACRO(7); }
    case 0226: { FJUMPMACRO(8); }
    case 0227: { FJUMPMACRO(9); }
    case 0230: { FJUMPMACRO(10); }
    case 0231: { FJUMPMACRO(11); }
    case 0232: { FJUMPMACRO(12); }
    case 0233: { FJUMPMACRO(13); }
    case 0234: { FJUMPMACRO(14); }
    case 0235: { FJUMPMACRO(15); }
    case 0236: { FJUMPMACRO(16); }
    case 0237: { FJUMPMACRO(17); }

    /* TJUMP */

    case 0240: { TJUMPMACRO(2); }
    case 0241: { TJUMPMACRO(3); }
    case 0242: { TJUMPMACRO(4); }
    case 0243: { TJUMPMACRO(5); }
    case 0244: { TJUMPMACRO(6); }
    case 0245: { TJUMPMACRO(7); }
    case 0246: { TJUMPMACRO(8); }
    case 0247: { TJUMPMACRO(9); }
    case 0250: { TJUMPMACRO(10); }
    case 0251: { TJUMPMACRO(11); }
    case 0252: { TJUMPMACRO(12); }
    case 0253: { TJUMPMACRO(13); }
    case 0254: { TJUMPMACRO(14); }
    case 0255: { TJUMPMACRO(15); }
    case 0256: { TJUMPMACRO(16); }
    case 0257: { TJUMPMACRO(17); }

    /******* JUMPX ********/
    case 0260: {
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* JUMPXX ********/
    case 0261: {
      CHECK_INTERRUPT;
      /* UB: left shift of negative value -1 */
      PCMACL += (Get_SBYTE_PCMAC1 << 8) | Get_BYTE_PCMAC2;
      nextop0;
    }

    /******* FJumpx *******/
    case 0262: {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* TJumpx *******/

    case 0263: {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NFJumpx *******/

    case 0264: {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NTJumpx *******/

    case 0265: {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    case 0266:
      AREF1;
    case 0267:
      ASET1;

    case 0270:
      PVARSETPOPMACRO(0);
    case 0271:
      PVARSETPOPMACRO(1);
    case 0272:
      PVARSETPOPMACRO(2);
    case 0273:
      PVARSETPOPMACRO(3);
    case 0274:
      PVARSETPOPMACRO(4);
    case 0275:
      PVARSETPOPMACRO(5);
    case 0276:
      PVARSETPOPMACRO(6);

    case 0277: {
      POP;
      nextop1;
    }

    case 0300:
      POPN(Get_BYTE_PCMAC1);
    case 0301:
      ATOMCELL_N(Get_BYTE_PCMAC1);
    case 0302:
      GETBASEBYTE;
    case 0303:
      INSTANCEP(Get_AtomNo_PCMAC1);
    case 0304:
      BLT;
    case 0305: { goto op_ufn; } /* MISC10 */
    case 0306: { goto op_ufn; } /* P-MISC2 ??? */
    case 0307:
      PUTBASEBYTE;
    case 0310:
      GETBASE_N(Get_BYTE_PCMAC1);
    case 0311:
      GETBASEPTR_N(Get_BYTE_PCMAC1);
    case 0312:
      GETBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case 0313: { goto op_ufn; } /* unused */
    case 0314:
      CLEQUAL;
    case 0315:
      PUTBASE_N(Get_BYTE_PCMAC1);
    case 0316:
      PUTBASEPTR_N(Get_BYTE_PCMAC1);
    case 0317:
      PUTBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);

    case 0320:
      N_OP_ADDBASE;
    case 0321:
      N_OP_VAG2;
    case 0322:
      N_OP_HILOC;
    case 0323:
      N_OP_LOLOC;
    case 0324:
      PLUS2; /* PLUS */
    case 0325:
      DIFFERENCE; /* DIFFERENCE */
    case 0326:
      TIMES2; /* TIMES2 */
    case 0327:
      QUOTIENT;                          /* QUOTIENT */
    case 0330:
      IPLUS2; /* IPLUS2 only while PLUS has no float */
    case 0331:
      IDIFFERENCE; /* IDIFFERENCE only while no float */
    case 0332:
      ITIMES2; /* ITIMES2 only while no float */
    case 0333:
      IQUOTIENT; /* IQUOTIENT */
    case 0334:
      IREMAINDER;
    case 0335:
      IPLUS_N(Get_BYTE_PCMAC1);
    case 0336:
      IDIFFERENCE_N(Get_BYTE_PCMAC1);
    case 0337: { goto op_ufn; } /* BASE-< */
    case 0340:
      LLSH1;
    case 0341:
      LLSH8;
    case 0342:
      LRSH1;
    case 0343:
      LRSH8;
    case 0344:
      LOGOR;
    case 0345:
      LOGAND;
    case 0346:
      LOGXOR;
    case 0347:
      LSH;
    case 0350:
      FPLUS2;
    case 0351:
      FDIFFERENCE;
    case 0352:
      FTIMES2;
    case 0353:
      FQUOTIENT;
    case 0354:
      UBFLOAT2(Get_BYTE_PCMAC1);
    case 0355:
      UBFLOAT1(Get_BYTE_PCMAC1);
    case 0356:
      AREF2;
    case 0357:
      ASET2;

    case 0360: {
      if (TOPOFSTACK == POP_TOS_1)
        TOPOFSTACK = ATOM_T;
      else
        TOPOFSTACK = NIL_PTR;
      nextop1;
    }

    case 0361:
      IGREATERP; /* IGREATERP if no float */
    case 0362:
      FGREATERP;
    case 0363:
      GREATERP;
    case 0364:
      ILEQUAL;
    case 0365:
      MAKENUMBER;
    case 0366:
      BOXIPLUS;
    case 0367:
      BOXIDIFFERENCE;
    case 0370: { goto op_ufn; } /* FLOATBLT */
    case 0371: { goto op_ufn; } /* FFTSTEP */
    case 0372:
      MISC3(Get_BYTE_PCMAC1);
    case 0373:
      MISC4(Get_BYTE_PCMAC1);
    case 0374: { goto op_ufn; } /* upctrace */
    case 0375:
      SWAP;
    case 0376:
      NOP;
    case 0377:
      CLARITHEQUAL;

    default: error("should not default");

  } /* switch */

/************************************************************************/
/*		TIMER INTERRUPT CHECK ROUTINE				*/
/************************************************************************/
check_interrupt:
  if ((UNSIGNED)CSTKPTR > (UNSIGNED)EndSTKP) {
    EXT;
    error("Unrecoverable Stack Overflow");
    RET;
  }

  /* Check for an IRQ request */

  {
    int need_irq;
    static int period_cnt = 60;
    extern int KBDEventFlg;
    extern int ETHEREventCount;
    extern LispPTR *KEYBUFFERING68k;
    extern LispPTR *PENDINGINTERRUPT68k;
    extern LispPTR ATOM_STARTED;
    extern LispPTR *PERIODIC_INTERRUPT68k;
    extern LispPTR *PERIODIC_INTERRUPT_FREQUENCY68k;
    extern int URaid_req;

  /* Check for an Stack Overflow */
  /* JDS 22 May 96 -- >= below used to be just >, changed because we got
     stack oflows with last frame right at end of stack, leading to loops,
     odd bugs, ... */
  /**** Changed back to > 31 July 97 ****/
  re_check_stack:
    need_irq = 0;
    if (((UNSIGNED)(CSTKPTR + 1) > Irq_Stk_Check) && (Irq_Stk_End > 0) && (Irq_Stk_Check > 0)) {
      HARD_PUSH(TOPOFSTACK);
      EXT;
      extended_frame = NIL;
      if (do_stackoverflow(NIL)) {
      stackoverflow_help:
        period_cnt = 60;
        need_irq = T;
        error("Stack Overflow, MUST HARDRESET!");
        RET;
        TOPOFSTACK = NIL_PTR;
      } else {
        RET;
        POP;
      }
      Irq_Stk_Check = (UNSIGNED)EndSTKP - STK_MIN(FuncObj);
      need_irq = (Irq_Stk_End == 0) || extended_frame;
      *PENDINGINTERRUPT68k |= extended_frame;
      Irq_Stk_End = (UNSIGNED)EndSTKP;
    }

    /* This is a good time to process keyboard/mouse and ethernet I/O
     * X events are not managed in the async/SIGIO code while
     * raw ethernet, serial port, and socket connections are.
     * If the system is configured with SIGIO handling we have a hint
     * that allows us to cheaply skip if there's nothing to do
     */
#ifdef XWINDOW
    process_Xevents(currentdsp);
#endif
#ifdef SDL
    process_SDLevents();
#endif
    if (IO_Signalled) {
      IO_Signalled = FALSE;
      process_io_events();
    }

    if ((Irq_Stk_End <= 0) || (Irq_Stk_Check <= 0) || need_irq) {
      if (StackOffsetFromNative(CSTKPTR) > InterfacePage->stackbase) {
        /* Interrupts not Disabled */
        EXT;
        update_timer();

        /*** If SPY is running, check to see if it ***/
        /*** needs an interrupt; give it one, if so. ***/
        if (*PERIODIC_INTERRUPT68k != NIL) {
          if (period_cnt > 0)
            period_cnt--;
          else {
            cause_interruptcall(PERIODIC_INTERRUPTFRAME_index);
            if (*PERIODIC_INTERRUPT_FREQUENCY68k == NIL)
              period_cnt = 0;
            else
              period_cnt =
                  (*PERIODIC_INTERRUPT_FREQUENCY68k & 0xffff) * (1000000 / 60) / TIMER_INTERVAL;
            /* number of 1/60 second periods between interrupts.
            TIMER_INTERVAL is the number of microseconds between
            timer interrupts. The calculation here avoids some
            overflow errors although there is some roundoff
            if the interrupt frequency number is too low,
            it will bottom out and just set period_cnt to 0 */
          }
        }

#ifdef DOS
        if (currentkbd->URaid == TRUE) {
          currentkbd->URaid = NIL;
          (currentkbd->device.exit)(currentkbd); /* Install the original handler */
          error("Call URaid by User Interrupt");
        } else if (currentmouse->Cursor.Moved) {
          union REGS regs;

          currentdsp->device.locked++;

          /* Remove the mouse from the old place on the screen */
          (currentdsp->mouse_invisible)(currentdsp, IOPage);

          /* Find the new delta */
          regs.w.eax = 0x000B; /* Function 0xB = get delta mickeys */
          int86(0x33, &regs, &regs);
          currentmouse->Cursor.New.x += (short)regs.w.ecx;
          currentmouse->Cursor.New.y += (short)regs.w.edx;

          if (currentmouse->Cursor.New.x < 0)
            currentmouse->Cursor.New.x = 0;
          else if (currentmouse->Cursor.New.x > (currentdsp->Display.width - 1))
            currentmouse->Cursor.New.x = currentdsp->Display.width - 1;

          if (currentmouse->Cursor.New.y < 0)
            currentmouse->Cursor.New.y = 0;
          else if (currentmouse->Cursor.New.y > (currentdsp->Display.height - 1))
            currentmouse->Cursor.New.y = currentdsp->Display.height - 1;

          IOPage->dlmousex = IOPage->dlcursorx = currentmouse->Cursor.New.x;
          IOPage->dlmousey = IOPage->dlcursory = currentmouse->Cursor.New.y;

          /* Paint the mouse back up on the screen on the new place */
          (currentdsp->mouse_visible)(currentmouse->Cursor.New.x, currentmouse->Cursor.New.y);
          currentmouse->Cursor.Moved = FALSE;
          currentdsp->device.locked--;
        }
#else
        if (URaid_req == T) {
          URaid_req = NIL;
          error("Call URaid by User Interrupt");
        }
#endif /* DOS */
        else if ((KBDEventFlg > 0) && (*KEYBUFFERING68k == ATOM_T)) {
          *KEYBUFFERING68k = ATOM_STARTED;
          cause_interruptcall(DOBUFFEREDTRANSITION_index);
          KBDEventFlg--;
        } else if (*Reclaim_cnt_word == S_POSITIVE) {
          *Reclaim_cnt_word = NIL;
          cause_interruptcall(DORECLAIM_index);
        } else if (*PENDINGINTERRUPT68k != NIL) {
          INTSTAT2 *intstate = ((INTSTAT2 *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
          /*unsigned char newints = (intstate->pendingmask) & ~(intstate->handledmask);
          if (newints) */
          {
            intstate->handledmask |= intstate->pendingmask;
            *PENDINGINTERRUPT68k = NIL;
            cause_interruptcall(INTERRUPTFRAME_index);
          }
        } else if (ETHEREventCount > 0) {
          INTSTAT *intstate = ((INTSTAT *)NativeAligned4FromLAddr(*INTERRUPTSTATE_word));
          if (!(intstate->ETHERInterrupt) && !(((INTSTAT2 *)intstate)->handledmask & 0x40)) {
            intstate->ETHERInterrupt = 1;
            ((INTSTAT2 *)intstate)->handledmask |= ((INTSTAT2 *)intstate)->pendingmask;
            cause_interruptcall(INTERRUPTFRAME_index);
            ETHEREventCount--;
          } else
            *PENDINGINTERRUPT68k = ATOM_T;
        }
        RET;
        CLR_IRQ;
      } /* Interrupts not Disabled */
      else {
        /* Clear out IRQ (loses pending interrupt request
           if interrupts are disabled) */
        CLR_IRQ;
        goto re_check_stack;
      }
    }
  }

  nextop0;

/************************************************************************/
/* Common Jump Tails (they have to jump anyway, so use common Tail) */
/************************************************************************/
PopNextop1:
  POP;
  nextop1;

PopNextop2:
  POP;
  nextop2;
}

void do_brk(void) {}
