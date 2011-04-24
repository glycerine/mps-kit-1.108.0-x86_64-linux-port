/* gcc seems to want a header file for ptrace.c  or it complains and won't compile: so I made one-- ptrace.h */

#if (__GNUC__>2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 95))

#ifndef jasons_ptrace_h_header
#define jasons_ptrace_h_header

/*---------------------------------------------------------------------------
   								Includes
 ---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/*---------------------------------------------------------------------------
   							    User Macros
 ---------------------------------------------------------------------------*/
#define PTRACE_PIPENAME	 "TRACE"

/* When using ptrace on a dynamic library, the following must be defined:

#include "any files needed for PTRACE_REFERENCE_FUNCTION"
#define PTRACE_REFERENCE_FUNCTION functionName

`*/


/*---------------------------------------------------------------------------
   								Defines
 ---------------------------------------------------------------------------*/

#define REFERENCE_OFFSET "REFERENCE:"
#define FUNCTION_ENTRY   "enter"
#define FUNCTION_EXIT    "exit"
#define END_TRACE        "EXIT"
#define __NON_INSTRUMENT_FUNCTION__    __attribute__((__no_instrument_function__))
#define PTRACE_OFF        __NON_INSTRUMENT_FUNCTION__
#define STR(_x)          #_x
#define DEF(_x)          _x
#define GET(_x,_y)       _x(_y)
/*---------------------------------------------------------------------------
  							Function codes
 ---------------------------------------------------------------------------*/


/** Final trace close */
static void
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace_close(void);

/** Trace initialization */
static int
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace_init(void);

/** Function called by every function event */
void
__NON_INSTRUMENT_FUNCTION__
gnu_ptrace(char * what, void * p);

/** According to gcc documentation: called upon function entry */
void
__NON_INSTRUMENT_FUNCTION__
__cyg_profile_func_enter(void *this_fn, void *call_site);

/** According to gcc documentation: called upon function exit */
void
__NON_INSTRUMENT_FUNCTION__
__cyg_profile_func_exit(void *this_fn, void *call_site);

#endif

#endif /* jasons_ptrace_h_header */
