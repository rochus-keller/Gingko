/* $Id: timer.c,v 1.5 2001/12/26 22:17:05 sybalsky Exp $ (C) Copyright Venue, All Rights Reserved */

/************************************************************************/
/*									*/
/*				t i m e r . c				*/
/*									*/
/*	Timer handling routines, plus set-up for the other interrupts	*/
/*	Medley uses on Unix.						*/
/*									*/
/************************************************************************/

/************************************************************************/
/*									*/
/*	(C) Copyright 1989-99 Venue. All Rights Reserved.		*/
/*	Manufactured in the United States of America.			*/
/*									*/
/************************************************************************/

#include "version.h"

#include <errno.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef DOS
#include <sys/resource.h>
#include <sys/time.h>
#endif /* DOS */

#include "lispemul.h"
#include "lspglob.h"
#include "adr68k.h"
#include "lsptypes.h"
#include "arith.h"
#include "lispmap.h"
#include "stack.h"
#include "dbprint.h"
#include "miscstat.h"

#include "timerdefs.h"
#include "commondefs.h"
#include "mkcelldefs.h"
#include "keyeventdefs.h"

#define LISP_UNIX_TIME_DIFF 29969152
#define LISP_ALTO_TIME_MASK 0x80000000
#define UNIX_ALTO_TIME_DIFF 2177452800U

/*	Interlisp time is signed; MIN.FIXP = "01-JAN-01 00:00:00 GMT"
 *	Interlisp 0 is at	"19-Jan-69 12:14:08 PST"
 *	Unix begins at		" 1-Jan-70  0:00:00 GMT"
 *	(CL:- (IL:IDATE	" 1-Jan-70  0:00:00 GMT")
 *	      (IL:IDATE "19-Jan-69 12:14:08 PST"))
 *	=> 29969152, amount to add to Lisp time to get Unix time
 *	Alto time is unsigned; 0 = "01-JAN-01 00:00:00 GMT"
 *      UNIX_ALTO_TIME_DIFF is amount to add to Unix time
 *	to get Alto time.
 */

extern unsigned int TIMEOUT_TIME;
unsigned int TIMEOUT_TIME = 10; /* For file system timeout, seconds, default 10 */

extern volatile sig_atomic_t IO_Signalled;
volatile sig_atomic_t IO_Signalled = FALSE;

static int gettime(int casep);

/************************************************************************/
/*									*/
/*		    u p d a t e _ m i s c s t a t s			*/
/*									*/
/*	Updates counters and timers in the MISCSTATS "page".		*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void update_miscstats(void) {
#if defined(MAIKO_OS_EMSCRIPTEN)
  /* Emscripten does not provide getrusage() functionality */
  struct timeval timev;

  MiscStats->totaltime = gettime(0) - MiscStats->starttime;
  MiscStats->swapwaittime = 0;
  MiscStats->pagefaults = 0;
  MiscStats->swapwrites = 0;
  MiscStats->diskiotime = 0;
  MiscStats->diskops = 0;

  gettimeofday(&timev, NULL);
  MiscStats->secondstmp = MiscStats->secondsclock = (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
#else
  struct timeval timev;
  struct rusage ru;

  getrusage(RUSAGE_SELF, &ru);

  MiscStats->totaltime = ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000;
  MiscStats->swapwaittime = ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000;
  MiscStats->pagefaults = ru.ru_minflt + ru.ru_majflt;
  MiscStats->swapwrites = ru.ru_majflt;
  MiscStats->diskiotime = 0; /* ?? not available ?? */
  MiscStats->diskops = ru.ru_inblock + ru.ru_oublock;
  gettimeofday(&timev, NULL);
  MiscStats->secondstmp = MiscStats->secondsclock = (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
#endif
}

/************************************************************************/
/*									*/
/*			i n i t _ m i s c s t a t s			*/
/*									*/
/*	Called at initialization time to set miscstats words.		*/
/*	?? and to periodically update them ?? [JDS 11/22/89]		*/
/*									*/
/*									*/
/************************************************************************/

void init_miscstats(void) {
  MiscStats->starttime = gettime(0);
  MiscStats->gctime = 0;
  update_miscstats();
}

/************************************************************************/
/*									*/
/*			s u b r _ g e t t i m e				*/
/*									*/
/*	Handler for Lisps GETTIME subr call, dispatched thru		*/
/*	subr.c/miscn.c sub-dispatch.					*/
/*									*/
/*	Calls gettime, and returns the result to Lisp as a SMALLP	*/
/*	or FIXP, as appropriate.					*/
/*									*/
/************************************************************************/

LispPTR subr_gettime(LispPTR args[])
{
  int result;
  result = gettime(args[0] & 0xffff);
  if (args[1]) {
    *((int *)NativeAligned4FromLAddr(args[1])) = result;
    return (args[1]);
  } else
    N_ARITH_SWITCH(result);
}

/************************************************************************/
/*									*/
/*		    		g e t t i m e				*/
/*									*/
/*	Get the value of one of the various time counters, as		*/
/*	specified by the argument casep.  casep's values & meanings:	*/
/*									*/
/*	0 elapsed time, in milliseconds.				*/
/*	1 start of elapsed-time period, in milliseconds			*/
/*	2 this process's run time, in milliseconds			*/
/*	3 total GC time, in milliseconds				*/
/*	4 current time-of-day, in ALTO format				*/
/*	5 current time-of-day, in Interlisp format			*/
/*	6 start of daylight-savings, as day-in-year			*/
/*	7 end of daylight-savings, as day-in-year			*/
/*	8 time zone, as hours of offset from GMT (whole hours only)	*/
/*									*/
/************************************************************************/

static int gettime(int casep)
{
  struct timeval timev;
  switch (casep) {
    case 0: /* elapsed time in alto milliseconds */
      gettimeofday(&timev, NULL);
      return ((timev.tv_sec + UNIX_ALTO_TIME_DIFF) * 1000 + timev.tv_usec / 1000);

    case 1: /* starting elapsed time in milliseconds */ return (MiscStats->starttime);

    case 2: /* run time, this process, in milliseconds */
      update_miscstats();
      return (MiscStats->totaltime);

    case 3: /* total GC time in milliseconds */ return (MiscStats->gctime);

    case 4: /* current time of day in Alto format */
      gettimeofday(&timev, NULL);
      return (timev.tv_sec + UNIX_ALTO_TIME_DIFF);

    case 5: /* current time of day in Interlisp format */
      gettimeofday(&timev, NULL);
      return (timev.tv_sec + LISP_UNIX_TIME_DIFF);

    case 6:
      return (98); /* this is wrong, only works in PST */

    case 7:
      return (305); /* this is wrong, only works in PST */

    case 8: {
      /* other methods of determining timezone offset are unreliable and/or deprecated */
      /* timezone is declared in <time.h>; the time mechanisms must be initialized     */
      /* Unfortunately, FreeBSD does not support the timezone external variable, nor   */
      /* does gettimeofday() seem to produce the correct timezone values.	       */
      tzset();
#if defined(MAIKO_OS_FREEBSD)
      time_t tv = time(NULL);
      struct tm *tm = localtime(&tv);
      return (tm->tm_gmtoff / -3600);
#else
      time_t t = time( NULL );
      struct tm *local = localtime( &t );
      struct tm *zulu  = gmtime( &t );
      return zulu->tm_hour - local->tm_hour;
      // return (timezone / 3600); /* timezone, extern, is #secs diff GMT to local. */
#endif
      }
    default: return (0);
  }
}

/************************************************************************/
/*									*/
/*			s u b r _ s e t t i m e				*/
/*									*/
/*	Converts its argument, a time in ALTO seconds, to the		*/
/*	UNIX time format, and sets the UNIX clock.  You must be		*/
/*	the super-user for this to work.				*/
/*									*/
/*	Implements the SETTIME subr call, sub-dispatched from subr.c	*/
/*									*/
/************************************************************************/

void subr_settime(LispPTR args[])
{
#if defined(MAIKO_OS_EMSCRIPTEN)
  (void)args[0];
#else
  struct timeval timev;
  timev.tv_sec = *((int *)NativeAligned4FromLAddr(args[0])) - UNIX_ALTO_TIME_DIFF;
  timev.tv_usec = 0;
  settimeofday(&timev, NULL); // TODO
#endif
} /* end subr_settime */

/************************************************************************/
/*									*/
/*		    s u b r _ c o p y t i m e s t a t s			*/
/*									*/
/*	Given source and destination MISCSTATS structure pointers,	*/
/*	copy the contents of the source structure into the dest.	*/
/*									*/
/*	Also calls update_miscstats, to keep stats current.		*/
/*									*/
/************************************************************************/

void subr_copytimestats(LispPTR args[])
{
  MISCSTATS *source;
  MISCSTATS *dest;
  source = (MISCSTATS *)NativeAligned4FromLAddr(args[0]);
  dest = (MISCSTATS *)NativeAligned4FromLAddr(args[1]);
  update_miscstats();
  *dest = *source;
}

/************************************************************************/
/*									*/
/*			     N _ O P _ r c l k				*/
/*									*/
/*	Get the current time in UNIX format, convert it to micro-	*/
/*	seconds in ALTO format, and store the low 32 bits into		*/
/*	the FIXP cell passed in to us on the top of stack.		*/
/*									*/
/************************************************************************/

LispPTR N_OP_rclk(LispPTR tos)
{
  unsigned int usec;

  struct timeval timev;

  gettimeofday(&timev, NULL);
  usec = (timev.tv_sec * 1000000UL) + timev.tv_usec;
  *((unsigned int *)(NativeAligned4FromLAddr(tos))) = usec;
  return (tos);
} /* end N_OP_rclk */

/**********************************************************************/
/* update_timer called periodically */

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void update_timer(void) {
  struct timeval timev;
  gettimeofday(&timev, NIL);
  MiscStats->secondstmp = MiscStats->secondsclock = (timev.tv_sec + UNIX_ALTO_TIME_DIFF);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/**********************************************************************/
/* timer interrupt handling system

        int_init() should be called before first entering dispatch loop.
        int_timer_init() is called by int_init() and arms the timer interrupt.
        int_io_init() is called by int_init() and arms the I/O interrupt.
        int_timer_service() catches the timer signal and sets
                Irq_Stk_Check & Irq_Stk_End to 0
                so the rest of the system will see it and respond.
        int_block() and int_unblock() block timer interrupts  and release them.
        int_io_open(fd) should be called whenever a file that should interrupt
                us is opened; it enables the interrupt on that fd.
        int_io_close(fd) should be called whenever a file that should interrupt
                us is closed; it disables the interrupt on that fd.
*/

/* TIMER_INTERVAL usec ~ 20  per second.  This should live in some
        machine-configuration
        file somewhere - it can be changed as the -t parameter to lisp*/
extern int TIMER_INTERVAL;
int TIMER_INTERVAL = 10000;

/************************************************************************/
/*									*/
/*		    i n t _ t i m e r _ s e r v i c e			*/
/*									*/
/*	Handle the virtual-time alarm signal VTALRM.			*/
/*									*/
/*									*/
/************************************************************************/

static void int_timer_service(int sig)
{
  /* this may have to do more in the future, like check for nested interrupts,
          etc... */

  Irq_Stk_Check = 0;
  Irq_Stk_End = 0;
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifndef SA_RESTART
# define SA_RESTART   0x10000000 /* Restart syscall on signal return.  */
#endif

static void int_timer_init(void)
{
  struct itimerval timert;
  struct sigaction timer_action;

  timer_action.sa_handler = int_timer_service;
  sigemptyset(&timer_action.sa_mask);
  timer_action.sa_flags = SA_RESTART;

  if (sigaction(SIGVTALRM, &timer_action, NULL) == -1) {
    perror("sigaction: SIGVTALRM");
  }

  /* then attach a timer to it and turn it loose */
  timert.it_interval.tv_sec = timert.it_value.tv_sec = 0;
  timert.it_interval.tv_usec = timert.it_value.tv_usec = TIMER_INTERVAL;
  setitimer(ITIMER_VIRTUAL, &timert, NULL);

  DBPRINT(("Timer interval set to %ld usec\n", (long)timert.it_value.tv_usec));
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

#ifndef F_SETOWN
# define F_SETOWN	__F_SETOWN /* Get owner (process receiving SIGIO).  */
#endif

void int_io_open(int fd)
{
  DBPRINT(("int_io_open %d\n", fd));
#if defined(O_ASYNC)
  if (fcntl(fd, F_SETOWN, getpid()) == -1) perror("fcntl F_SETOWN error"); // TODO
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_ASYNC) == -1) perror("fcntl F_SETFL on error");
#elif defined(FASYNC) && defined(FNDELAY)
  if (fcntl(fd, F_SETOWN, getpid()) == -1) perror("fcntl F_SETOWN error");
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | FNDELAY | FASYNC) == -1) perror("fcntl F_SETFL on error");
#else
#warning "No async i/o can be enabled - investigate and remedy this"
#endif
}

void int_io_close(int fd)
{
  DBPRINT(("int_io_close %d\n", fd));
#if defined(O_ASYNC)
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_ASYNC) == -1) perror("fcntl_F_SETFL off error");
#elif defined(FASYNC) && defined(FNDELAY)
  if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~(FNDELAY | FASYNC)) == -1) perror("fcntl F_SETFL off error");
#endif
}

/************************************************************************/
/*									*/
/*		    i n t _ i o _ s e r v i c e				*/
/*									*/
/*	Handle SIGIO							*/
/*									*/
/*									*/
/************************************************************************/

static void int_io_service(int sig)
{
  Irq_Stk_Check = 0;
  Irq_Stk_End = 0;
  IO_Signalled = TRUE;
}

/************************************************************************/
/*									*/
/*			i n t _ i o _ i n i t				*/
/*									*/
/*	Set up handling for the SIGIO signals, in			*/
/*	support of keyboard event handling and ethernet incoming-	*/
/*	packet handling.						*/
/*									*/
/*									*/
/************************************************************************/

static void int_io_init(void) {
  struct sigaction io_action;
  io_action.sa_handler = int_io_service;
  sigemptyset(&io_action.sa_mask);
  io_action.sa_flags = 0;

  if (sigaction(SIGIO, &io_action, NULL) == -1) {
    perror("sigaction: SIGIO");
  } else {
    DBPRINT(("I/O interrupts enabled\n"));
  }
}

/************************************************************************/
/*									*/
/*			   i n t _ b l o c k				*/
/*									*/
/*	Temporarily turn off interrupts.				*/
/*									*/
/*	NOTE that these interrupts must also be turned off in ldeboot's	*/
/*	forking code; if you change these, go fix that one too		*/
/*									*/
/************************************************************************/

void int_block(void) {
/* temporarily turn off interrupts */
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGVTALRM);
  sigaddset(&signals, SIGALRM);
  sigaddset(&signals, SIGXFSZ);
#ifdef FLTINT
  sigaddset(&signals, SIGFPE);
#endif
  sigprocmask(SIG_BLOCK, &signals, NULL);
}

/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

void int_unblock(void) {
  sigset_t signals;
  sigemptyset(&signals);
  sigaddset(&signals, SIGVTALRM);
  sigaddset(&signals, SIGIO);
  sigaddset(&signals, SIGALRM);
  sigaddset(&signals, SIGXFSZ);
#ifdef FLTINT
  sigaddset(&signals, SIGFPE);
#endif
  sigprocmask(SIG_UNBLOCK, &signals, NULL);
}

#ifdef FLTINT
/************************************************************************/
/*									*/
/*  F L O A T I N G - P O I N T   I N T E R R U P T   H A N D L I N G	*/
/*									*/
/*	This is the handler for the SIGFPE signal, to catch floating-	*/
/*	point exceptions.  Sets the global 'FP_error' to the error	*/
/*	code passed in by the signal; FP_error is checked by the	*/
/*	Lisp emulator FP code to make sure everything is OK.		*/
/*									*/
/************************************************************************/

/* The global used to signal floating-point errors */
volatile sig_atomic_t FP_error = 0;

void int_fp_service(int sig, siginfo_t *info, void *context)
{
  switch (info->si_code) {
    case FPE_FLTDIV:
    case FPE_FLTUND:
    case FPE_FLTOVF:
    case FPE_FLTINV:

      FP_error = info->si_code;
      break;
    default: {
#ifdef DEBUG
      char stuff[100];
      snprintf(stuff, sizeof(stuff), "Unexpected FP error signal code: %d", info->si_code);
      perror(stuff);
#else
      FP_error = info->si_code;
#endif
    }
  }
}

void int_fp_init(void) {
  struct sigaction fpe_action;

  fpe_action.sa_handler = int_fp_service;
  sigemptyset(&fpe_action.sa_mask);
  fpe_action.sa_flags = SA_SIGINFO;

  if (sigaction(SIGFPE, &fpe_action, NULL) == -1) {
    perror("Sigset for FPE failed");
  } else {
    DBPRINT(("FP interrupts enabled\n"));
  }
}

#endif /* FLTINT */

/************************************************************************/
/*									*/
/*			t i m e o u t _ e r r o r			*/
/*									*/
/*	Error handling routine for SIGALRM.  Called when any		*/
/*	TIMEOUT(...) forms spend more than TIMEOUT_TIME (normally	*/
/*	10 sec.) trying to do an I/O operation.				*/
/*									*/
/*									*/
/************************************************************************/

extern jmp_buf jmpbuf;
jmp_buf jmpbuf;
static void timeout_error(int sig) {
  longjmp(jmpbuf, 1);
}

/************************************************************************/
/*									*/
/*			i n t _ f i l e _ i n i t			*/
/*									*/
/*	Set up the signal handler for SIGALRM, to catch TIMEOUTs:	*/
/*	TIMEOUT(...) forms spend more than TIMEOUT_TIME (normally	*/
/*	10 sec.) trying to do an I/O operation.				*/
/*									*/
/*									*/
/************************************************************************/

static void int_file_init(void) {
  char *envtime;
  int timeout_time;
  struct sigaction timer_action;

  timer_action.sa_handler = timeout_error;
  sigemptyset(&timer_action.sa_mask);
  timer_action.sa_flags = 0;

  if (sigaction(SIGALRM, &timer_action, NULL) == -1) {
    perror("sigaction: SIGALRM");
  }

  /* Set Timeout period */
  envtime = getenv("LDEFILETIMEOUT");
  if (envtime != NULL) {
    errno = 0;
    timeout_time = (int)strtol(envtime, (char **)NULL, 10);
    if (errno == 0 && timeout_time > 0)
      TIMEOUT_TIME = timeout_time;
  }
  DBPRINT(("File timeout interrupts enabled\n"));
}

/************************************************************************/
/*                                                                      */
/*                         p a n i c u r a i d                          */
/*                                                                      */
/*   Most of the unused process-killing interrupts end up here; you     */
/*      can't do a whole lot safely here but dump your sysout for       */
/*      post-mortem analysis, but you MIGHT be able to get a clue       */
/*      about what killed you.                                          */
/*                                                                      */
/************************************************************************/
static void panicuraid(int sig, siginfo_t *info, void *context)
{
  static char errormsg[200];
  static char *stdmsg =
      "Please record the signal and code information\n\
and do a 'v' before trying anything else.";

  switch (sig) {
    case SIGBUS:
    case SIGILL:
    case SIGSEGV:
      snprintf(errormsg, sizeof(errormsg), "%s at address %p.\n%s", strsignal(sig), info->si_addr, stdmsg); // TODO
      break;
    case SIGPIPE:
      snprintf(errormsg, sizeof(errormsg), "Broken PIPE.\n%s", stdmsg);
      break;
    case SIGHUP:
      snprintf(errormsg, sizeof(errormsg), "HANGUP signalled.\n%s", stdmsg);
      /* Assume that a user tried to exit UNIX shell */
      killpg(getpgrp(), SIGKILL); // TODO
      exit(0);
    case SIGFPE:
      snprintf(errormsg, sizeof(errormsg), "%s (%d) at address %p.\n%s", strsignal(sig), info->si_code, info->si_addr, stdmsg);
      break;
    default: snprintf(errormsg, sizeof(errormsg), "Uncaught SIGNAL %s (%d).\n%s", strsignal(sig), sig, stdmsg);
  }

  error(errormsg);
}

/************************************************************************/
/*                                                                      */
/*                     i n t _ p a n i c _ i n i t                      */
/*                                                                      */
/*  A catch for all the deadly interrupts (but KILL, of course)          */
/*  Dumps you into uraid; you probably can't get back from it,          */
/*     but there is hope that you will be able to poke around with      */
/*     uraid and get a clue about why you're dying.                     */
/*                                                                      */
/************************************************************************/
static void int_panic_init(void) {
  struct sigaction panic_action, ignore_action;

  panic_action.sa_sigaction = panicuraid;
  sigemptyset(&panic_action.sa_mask);
  panic_action.sa_flags = SA_SIGINFO;

  ignore_action.sa_handler = SIG_IGN;
  sigemptyset(&ignore_action.sa_mask);
  ignore_action.sa_flags = 0;

  sigaction(SIGHUP, &panic_action, NULL);
  sigaction(SIGQUIT, &panic_action, NULL);
  sigaction(SIGILL, &panic_action, NULL);
#ifdef SIGEMT
  sigaction(SIGEMT, &panic_action, NULL);
#endif
  sigaction(SIGBUS, &panic_action, NULL);
  sigaction(SIGSEGV, &panic_action, NULL);
  sigaction(SIGSYS, &panic_action, NULL);
  sigaction(SIGTERM, &panic_action, NULL);

  /* Ignore SIGPIPE */
  sigaction(SIGPIPE, &ignore_action, NULL);

  DBPRINT(("Panic interrupts enabled\n"));
}

/************************************************************************/
/*									*/
/*			    i n t _ i n i t				*/
/*									*/
/*	Initialize all the interrupts for Lisp & the emulator.		*/
/*									*/
/************************************************************************/

void int_init(void) {
  int_timer_init(); /* periodic interrupt timer */
  int_io_init();    /* SIGIO async I/O handlers */
  int_file_init();  /* file-io TIMEOUT support */
  int_panic_init(); /* catch for all other dangerous interrupts */

#ifdef FLTINT
  int_fp_init(); /* Floating-point exception handler */
#endif

  int_unblock(); /* Turn on interrupts */
}

