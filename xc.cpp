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

#include <sys/time.h>

#include "lispemul.h"
#include "address.h"
#include "adr68k.h"
#include "stack.h"
#include "return.h"
#include "dbprint.h"
#include "opcodes.h"

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
#include "z2defs.h"

#ifdef SDL
extern void process_SDLevents();
#endif
#ifdef QTGUI
extern void qt_process_events();
#endif

typedef struct conspage ConsPage;
typedef ByteCode *InstPtr;

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

  switch (Get_BYTE_PCMAC0) {
    case opc_unused_0: { goto op_ufn; } /* unused */
    case opc_CAR:
      OPCAR;
    case opc_CDR:
      OPCDR;
    case opc_LISP:
      LISTP;
    case opc_NTYPX:
      NTYPEX;
    case opc_TYPEP:
      TYPEP(Get_BYTE_PCMAC1);
    case opc_TYPECHECK:
    case opc_DTEST:
      DTEST(Get_AtomNo_PCMAC1);
    case opc_UNWIND:
      UNWIND(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case opc_FN0:
      FN0;
    case opc_FN1:
      FN1;
    case opc_FN2:
      FN2;
    case opc_FN3:
      FN3;
    case opc_FN4:
      FN4;
    case opc_FNX:
      FNX;
    case opc_APPLYFN:
      APPLY;

    case opc_CHECKAPPLY:
      CHECKAPPLY;
    case opc_RETURN:
      RETURN;

    case opc_BIND:
      /* UB: left shift of negative value -4 */
      BIND;
    case opc_UNBIND:
      UNBIND;
    case opc_DUNBIND:
      DUNBIND;
    case opc_RPLPTR_N:
      RPLPTR(Get_BYTE_PCMAC1);
    case opc_GCREF:
      GCREF(Get_BYTE_PCMAC1);
    case opc_ASSOC:
      ASSOC;
    case opc_GVAR_:
      GVAR_(Get_AtomNo_PCMAC1);
    case opc_RPLACA:
      RPLACA;
    case opc_RPLACD:
      RPLACD;
    case opc_CONS:
      CONS;
    case opc_CMLASSOC:
      CLASSOC;
    case opc_FMEMB:
      FMEMB;
    case opc_CMLMEMBER:
      CLFMEMB;
    case opc_FINDKEY:
      FINDKEY(Get_BYTE_PCMAC1);
    case opc_CREATECELL:
      CREATECELL;
    case opc_BIN:
      BIN;
    case opc_BOUT: { goto op_ufn; } /* BOUT */
    case opc_POPDISP: { goto op_ufn; } /* POPDISP - prolog only */
    case opc_RESTLIST:
      RESTLIST(Get_BYTE_PCMAC1);
    case opc_MISCN:
      MISCN(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case opc_unused_37: { goto op_ufn; } /* unused */
    case opc_RPLCONS:
      RPLCONS;
    case opc_LISTGET:
      LISTGET;
    case opc_unused_40: { goto op_ufn; } /* unused */
    case opc_unused_41: { goto op_ufn; } /* unused */
    case opc_unused_42: { goto op_ufn; } /* unused */
    case opc_unused_43: { goto op_ufn; } /* unused */
    case opc_EVAL:
      EVAL;
    case opc_ENVCALL:
      ENVCALL;

    /*  case 056 : case056: @ 006 */
    case opc_STKSCAN:
      STKSCAN;
    case opc_BUSBLT:{ goto op_ufn; } /* BUSBLT - DLion only */
    case opc_MISC8:{ goto op_ufn; } /* MISC8 - no longer used */
    case opc_UBFLOAT3:
      UBFLOAT3(Get_BYTE_PCMAC1);
    case opc_TYPEMASK_N:
      TYPEMASK(Get_BYTE_PCMAC1);
    case opc_RDPROLOGPTR: { goto op_ufn; } /* rdprologptr */
    case opc_RDPROLOGTAG: { goto op_ufn; } /* rdprologtag */
    case opc_WRTPTRTAG: { goto op_ufn; } /* writeptr&tag */
    case opc_WRTPTR0TAG: { goto op_ufn; } /* writeptr&0tag */
    case opc_MISC7:
      MISC7(Get_BYTE_PCMAC1); /* misc7 (pseudocolor, fbitmapbit) */
    case opc_DOVEMISC: { goto op_ufn; } /* dovemisc - dove only */
    case opc_EQL:
      EQLOP;
    case opc_DRAWLINE:
      DRAWLINE;
    case opc_STORE_N:
      STOREN(Get_BYTE_PCMAC1);
    case opc_COPY_N:
      COPYN(Get_BYTE_PCMAC1);
    case opc_RAID: { goto op_ufn; } /* RAID */
    case opc_SLRETURN: { goto op_ufn; } /* \RETURN */

    case opc_IVAR0:
      IVARMACRO(0);
    case opc_IVAR1:
      IVARMACRO(1);
    case opc_IVAR2:
      IVARMACRO(2);
    case opc_IVAR3:
      IVARMACRO(3);
    case opc_IVAR4:
      IVARMACRO(4);
    case opc_IVAR5:
      IVARMACRO(5);
    case opc_IVAR6:
      IVARMACRO(6);
    case opc_IVARX:
      IVARX(Get_BYTE_PCMAC1);

    case opc_PVAR0:
      PVARMACRO(0);
    case opc_PVAR1:
      PVARMACRO(1);
    case opc_PVAR2:
      PVARMACRO(2);
    case opc_PVAR3:
      PVARMACRO(3);
    case opc_PVAR4:
      PVARMACRO(4);
    case opc_PVAR5:
      PVARMACRO(5);
    case opc_PVAR6:
      PVARMACRO(6);

    case opc_PVARX:
      PVARX(Get_BYTE_PCMAC1);

    case opc_FVAR0:
      FVAR(0);
    case opc_FVAR1:
      FVAR(2);
    case opc_FVAR2:
      FVAR(4);
    case opc_FVAR3:
      FVAR(6);
    case opc_FVAR4:
      FVAR(8);
    case opc_FVAR5:
      FVAR(10);
    case opc_FVAR6:
      FVAR(12);
    case opc_FVARX:
      FVARX(Get_BYTE_PCMAC1);

    case opc_PVAR_0:
      PVARSETMACRO(0);
    case opc_PVAR_1:
      PVARSETMACRO(1);
    case opc_PVAR_2:
      PVARSETMACRO(2);
    case opc_PVAR_3:
      PVARSETMACRO(3);
    case opc_PVAR_4:
      PVARSETMACRO(4);
    case opc_PVAR_5:
      PVARSETMACRO(5);
    case opc_PVAR_6:
      PVARSETMACRO(6);

    case opc_PVARX_:
      PVARX_(Get_BYTE_PCMAC1);

    case opc_GVAR:
      GVAR(Get_AtomNo_PCMAC1);
    case opc_ARG0:
      ARG0;
    case opc_IVARX_:
      IVARX_(Get_BYTE_PCMAC1);
    case opc_FVARX_:
      FVARX_(Get_BYTE_PCMAC1);
    case opc_COPY:
      COPY;
    case opc_MYARGCOUNT:
      MYARGCOUNT;
    case opc_MYALINK:
      MYALINK;

    /******** Aconst	********/
    case opc_ACONST: {
      PUSH(Get_AtomNo_PCMAC1);
      nextop_atom;
    }
    case opc_NIL: { PUSHATOM(NIL_PTR); }
    case opc_T: { PUSHATOM(ATOM_T); }
    case opc_0: { PUSHATOM(S_POSITIVE); } /* '0 */
    case opc_1: { PUSHATOM(0xE0001); } /* '1 */

    /********* SIC		********/
    case opc_SIC: {
      PUSH(S_POSITIVE | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SNIC		********/
    case opc_SNIC: {
      PUSH(S_NEGATIVE | 0xff00 | Get_BYTE_PCMAC1);
      nextop2;
    }

    /********* SICX		********/
    case opc_SICX: {
      PUSH(S_POSITIVE | Get_DLword_PCMAC1);
      nextop3;
    }

    /********* GCONST	********/
    case opc_GCONST: {
      PUSH(Get_Pointer_PCMAC1);
      nextop_ptr;
    }

    case opc_unused_112: { goto op_ufn; } /* unused */
    case opc_READFLAGS: { goto op_ufn; } /* readflags */
    case opc_READRP: { goto op_ufn; } /* readrp */
    case opc_WRITEMAP: { goto op_ufn; } /* writemap */
    case opc_READPRINTERPORT: { goto op_ufn; } /* readprinterport */
    case opc_WRITEPRINTERPORT: { goto op_ufn; } /* writeprinterport */

    case opc_PILOTBITBLT:
      PILOTBITBLT;
    case opc_RCLK:
      RCLK;
    case opc_MISC1: { goto op_ufn; } /* MISC1, dorado only */
    case opc_MISC2: { goto op_ufn; } /* MISC2, dorado only */
    case opc_RECLAIMCELL:
      RECLAIMCELL;
    case opc_GCSCAN1:
      GCSCAN1;
    case opc_GCSCAN2:
      GCSCAN2;
    case opc_SUBRCALL: {
      EXT;
      OP_subrcall(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
      RET;
      nextop0;
    }
    case opc_CONTEXTSWITCH: { CONTEXTSWITCH; }
    case opc_RETCALL: { goto op_ufn; } /* RETCALL */

    /* JUMP */

    case opc_JUMP0: { JUMPMACRO(2); }
    case opc_JUMP1: { JUMPMACRO(3); }
    case opc_JUMP2: { JUMPMACRO(4); }
    case opc_JUMP3: { JUMPMACRO(5); }
    case opc_JUMP4: { JUMPMACRO(6); }
    case opc_JUMP5: { JUMPMACRO(7); }
    case opc_JUMP6: { JUMPMACRO(8); }
    case opc_JUMP7: { JUMPMACRO(9); }
    case opc_JUMP8: { JUMPMACRO(10); }
    case opc_JUMP9: { JUMPMACRO(11); }
    case opc_JUMP10: { JUMPMACRO(12); }
    case opc_JUMP11: { JUMPMACRO(13); }
    case opc_JUMP12: { JUMPMACRO(14); }
    case opc_JUMP13: { JUMPMACRO(15); }
    case opc_JUMP14: { JUMPMACRO(16); }
    case opc_JUMP15: { JUMPMACRO(17); }

    /* FJUMP */

    case opc_FJUMP0: { FJUMPMACRO(2); }
    case opc_FJUMP1: { FJUMPMACRO(3); }
    case opc_FJUMP2: { FJUMPMACRO(4); }
    case opc_FJUMP3: { FJUMPMACRO(5); }
    case opc_FJUMP4: { FJUMPMACRO(6); }
    case opc_FJUMP5: { FJUMPMACRO(7); }
    case opc_FJUMP6: { FJUMPMACRO(8); }
    case opc_FJUMP7: { FJUMPMACRO(9); }
    case opc_FJUMP8: { FJUMPMACRO(10); }
    case opc_FJUMP9: { FJUMPMACRO(11); }
    case opc_FJUMP10: { FJUMPMACRO(12); }
    case opc_FJUMP11: { FJUMPMACRO(13); }
    case opc_FJUMP12: { FJUMPMACRO(14); }
    case opc_FJUMP13: { FJUMPMACRO(15); }
    case opc_FJUMP14: { FJUMPMACRO(16); }
    case opc_FJUMP15: { FJUMPMACRO(17); }

    /* TJUMP */

    case opc_TJUMP0: { TJUMPMACRO(2); }
    case opc_TJUMP1: { TJUMPMACRO(3); }
    case opc_TJUMP2: { TJUMPMACRO(4); }
    case opc_TJUMP3: { TJUMPMACRO(5); }
    case opc_TJUMP4: { TJUMPMACRO(6); }
    case opc_TJUMP5: { TJUMPMACRO(7); }
    case opc_TJUMP6: { TJUMPMACRO(8); }
    case opc_TJUMP7: { TJUMPMACRO(9); }
    case opc_TJUMP8: { TJUMPMACRO(10); }
    case opc_TJUMP9: { TJUMPMACRO(11); }
    case opc_TJUMP10: { TJUMPMACRO(12); }
    case opc_TJUMP11: { TJUMPMACRO(13); }
    case opc_TJUMP12: { TJUMPMACRO(14); }
    case opc_TJUMP13: { TJUMPMACRO(15); }
    case opc_TJUMP14: { TJUMPMACRO(16); }
    case opc_TJUMP15: { TJUMPMACRO(17); }

    /******* JUMPX ********/
    case opc_JUMPX: {
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* JUMPXX ********/
    case opc_JUMPXX: {
      CHECK_INTERRUPT;
      /* UB: left shift of negative value -1 */
      PCMACL += (Get_SBYTE_PCMAC1 << 8) | Get_BYTE_PCMAC2;
      nextop0;
    }

    /******* FJumpx *******/
    case opc_FJUMPX: {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* TJumpx *******/

    case opc_TJUMPX: {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      POP;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NFJumpx *******/

    case opc_NFJUMPX: {
      if (TOPOFSTACK != 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    /******* NTJumpx *******/

    case opc_NTJUMPX: {
      if (TOPOFSTACK == 0) { goto PopNextop2; }
      CHECK_INTERRUPT;
      PCMACL += Get_SBYTE_PCMAC1;
      nextop0;
    }

    case opc_AREF1:
      AREF1;
    case opc_ASET1:
      ASET1;

    case opc_PVARSETPOP0:
      PVARSETPOPMACRO(0);
    case opc_PVARSETPOP1:
      PVARSETPOPMACRO(1);
    case opc_PVARSETPOP2:
      PVARSETPOPMACRO(2);
    case opc_PVARSETPOP3:
      PVARSETPOPMACRO(3);
    case opc_PVARSETPOP4:
      PVARSETPOPMACRO(4);
    case opc_PVARSETPOP5:
      PVARSETPOPMACRO(5);
    case opc_PVARSETPOP6:
      PVARSETPOPMACRO(6);

    case opc_POP: {
      POP;
      nextop1;
    }

    case opc_POP_N:
      POPN(Get_BYTE_PCMAC1);
    case opc_ATOMCELL_N:
      ATOMCELL_N(Get_BYTE_PCMAC1);
    case opc_GETBASEBYTE:
      GETBASEBYTE;
    case opc_INSTANCEP:
      INSTANCEP(Get_AtomNo_PCMAC1);
    case opc_BLT:
      BLT;
    case opc_MISC10: { goto op_ufn; } /* MISC10 */
    case opc_P_MISC2: { goto op_ufn; } /* P-MISC2 ??? */
    case opc_PUTBASEBYTE:
      PUTBASEBYTE;
    case opc_GETBASE_N:
      GETBASE_N(Get_BYTE_PCMAC1);
    case opc_GETBASEPTR_N:
      GETBASEPTR_N(Get_BYTE_PCMAC1);
    case opc_GETBITS_N_FD:
      GETBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);
    case opc_unused_203: { goto op_ufn; } /* unused */
    case opc_CMLEQUAL:
      CLEQUAL;
    case opc_PUTBASE_N:
      PUTBASE_N(Get_BYTE_PCMAC1);
    case opc_PUTBASEPTR_N:
      PUTBASEPTR_N(Get_BYTE_PCMAC1);
    case opc_PUTBITS_N_FD:
      PUTBITS_N_M(Get_BYTE_PCMAC1, Get_BYTE_PCMAC2);

    case opc_ADDBASE:
      N_OP_ADDBASE;
    case opc_VAG2:
      N_OP_VAG2;
    case opc_HILOC:
      N_OP_HILOC;
    case opc_LOLOC:
      N_OP_LOLOC;
    case opc_PLUS2:
      PLUS2; /* PLUS */
    case opc_DIFFERENCE:
      DIFFERENCE; /* DIFFERENCE */
    case opc_TIMES2:
      TIMES2; /* TIMES2 */
    case opc_QUOTIENT:
      QUOTIENT;                          /* QUOTIENT */
    case opc_IPLUS2:
      IPLUS2; /* IPLUS2 only while PLUS has no float */
    case opc_IDIFFERENCE:
      IDIFFERENCE; /* IDIFFERENCE only while no float */
    case opc_ITIMES2:
      ITIMES2; /* ITIMES2 only while no float */
    case opc_IQUOTIENT:
      IQUOTIENT; /* IQUOTIENT */
    case opc_IREMAINDER:
      IREMAINDER;
    case opc_IPLUS_N:
      IPLUS_N(Get_BYTE_PCMAC1);
    case opc_IDIFFERENCE_N:
      IDIFFERENCE_N(Get_BYTE_PCMAC1);
    case opc_BASE_LESSTHAN: { goto op_ufn; } /* BASE-< */
    case opc_LLSH1:
      LLSH1;
    case opc_LLSH8:
      LLSH8;
    case opc_LRSH1:
      LRSH1;
    case opc_LRSH8:
      LRSH8;
    case opc_LOGOR2:
      LOGOR;
    case opc_LOGAND2:
      LOGAND;
    case opc_LOGXOR2:
      LOGXOR;
    case opc_LSH:
      LSH;
    case opc_FPLUS2:
      FPLUS2;
    case opc_FDIFFERENCE:
      FDIFFERENCE;
    case opc_FTIMES2:
      FTIMES2;
    case opc_FQUOTIENT:
      FQUOTIENT;
    case opc_UBFLOAT2:
      UBFLOAT2(Get_BYTE_PCMAC1);
    case opc_UBFLOAT1:
      UBFLOAT1(Get_BYTE_PCMAC1);
    case opc_AREF2:
      AREF2;
    case opc_ASET2:
      ASET2;

    case opc_EQ: {
      if (TOPOFSTACK == POP_TOS_1)
        TOPOFSTACK = ATOM_T;
      else
        TOPOFSTACK = NIL_PTR;
      nextop1;
    }

    case opc_IGREATERP:
      IGREATERP; /* IGREATERP if no float */
    case opc_FGREATERP:
      FGREATERP;
    case opc_GREATERP:
      GREATERP;
    case opc_EQUAL:
      ILEQUAL;
    case opc_MAKENUMBER:
      MAKENUMBER;
    case opc_BOXIPLUS:
      BOXIPLUS;
    case opc_BOXIDIFFERENCE:
      BOXIDIFFERENCE;
    case opc_FLOATBLT: { goto op_ufn; } /* FLOATBLT */
    case opc_FFTSTEP: { goto op_ufn; } /* FFTSTEP */
    case opc_MISC3:
      MISC3(Get_BYTE_PCMAC1);
    case opc_MISC4:
      MISC4(Get_BYTE_PCMAC1);
    case opc_UPCTRACE: { goto op_ufn; } /* upctrace */
    case opc_SWAP:
      SWAP;
    case opc_NOP:
      NOP;
    case opc_CL_EQUAL:
      CLARITHEQUAL;

    default: error("unknown opcode");

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
#ifdef SDL
    process_SDLevents();
#endif
#ifdef QTGUI
    qt_process_events();
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
        if (URaid_req == T) {
          URaid_req = NIL;
          error("Call URaid by User Interrupt");
        }
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
