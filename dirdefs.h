#ifndef DIRDEFS_H
#define DIRDEFS_H 1

#include "lispemul.h"       // for LispPTR
#include "locfile.h"        // for MAXNAMLEN
/*
 * FINFO and FPROP are used to store the information of the enumerated files
 * and directories.  They are arranged in a form of linked list.  Each list is
 * corresponding to the each directory enumeration.  All of the informations
 * Lisp needs are stored in the list.  This list is in the emulator's address space
 * and can be specified by "ID" which is the interface between the emulator and Lisp
 * code.  In this implementation, ID is represented as an integer and is actually
 * an index of the array of the lists.
 *
 * To avoid the overhead of the FINFO and FPROP structure dynamic allocation and
 * deallocation, some number of their instances are pre-allocated when the emulator
 * starts and managed in a free list.  If all of the pre-allocated instances are in
 * use, new instances are allocated.  The new instances are linked to the free list
 * when it is freed.
 *
 * As described above, the linked list result of the enumeration is stored in a
 * array for the subsequent request from Lisp.  Lisp code requests the emulator to
 * release the list when it enumerated all of the entries in the list or the
 * enumerating operation is aborted.
 */


#ifdef FSDEBUG
void print_finfo(FINFO *fp);
#endif
int init_finfo(void);
LispPTR COM_gen_files(LispPTR *args);
LispPTR COM_next_file(LispPTR *args);
LispPTR COM_finish_finfo(LispPTR *args);
#endif
