/* ssixi3.c: UNIX/INTEL STACK SCANNING
 *
 * $Id: //info.ravenbrook.com/project/mps/version/1.108/code/ssixi3.c#1 $
 * Copyright (c) 2001 Ravenbrook Limited.  See end of file for license.
 *
 *  This scans the stack and fixes the registers which may contain
 *  roots.  See <design/thread-manager/>
 *
 *  This code was originally developed and tested on Linux, and then
 *  copied to the FreeBSD and Darwin (OS X) operating systems where it
 *  also seems to work.  Note that on FreeBSD and Darwin it has not
 *  been indepently verified with respect to any ABI documentation.
 *
 *  This code appears is common to more than one Unix implementation on
 *  Intel hardware (but is not portable Unix code).
 *
 *  The registers edi, esi, ebx are the registers defined to be preserved
 *  across function calls and therefore may contain roots.
 *  These are pushed on the stack for scanning.
 *
 * SOURCES
 *
 * .source.callees.saves: Set of callee-saved registers taken from
 * CALL_USED_REGISTERS in <gcc-sources>/config/i386/i386.h.
 *
 * ASSUMPTIONS
 *
 * .assume.align: The stack pointer is assumed to be aligned on a word
 * boundary.
 *
 * .assume.asm.stack: The compiler must not do wacky things with the
 * stack pointer around a call since we need to ensure that the
 * callee-save regs are visible during TraceScanArea.
 *
 * .assume.asm.order: The volatile modifier should prevent movement
 * of code, which might break .assume.asm.stack.
 *
 */


#include "mpm.h"

SRCID(ssixi3, "$Id: //info.ravenbrook.com/project/mps/version/1.108/code/ssixi3.c#1 $");


/* .assume.asm.order */
#define ASMV(x) __asm__ volatile (x)


Res StackScan(ScanState ss, Addr *stackBot)
{
  Addr *stackTop;
  Res res;

#ifdef MPS_ARCH_I4
  /* .assume.asm.stack */
  ASMV("push %ebx");    /* These registers are callee-saved */
  ASMV("push %esi");    /* and so may contain roots.  They are pushed */
  ASMV("push %edi");    /* for scanning.  See .source.callees.saves. */
  ASMV("mov %%esp, %0" : "=r" (stackTop) :);    /* stackTop = esp */

  AVER(AddrIsAligned((Addr)stackTop, sizeof(Addr)));  /* .assume.align */
  res = TraceScanArea(ss, stackTop, stackBot);

  ASMV("add $12, %esp");  /* pop 3 regs to restore the stack pointer */
#endif 

  /* http://www.cs.cmu.edu/~fp/courses/15213-s06/misc/asm64-handout.pdf  
    seems to indicate rbx, rbp, r10, r13, r14, r15 are callee-saved, while
    rdx, r8, r9 and rcx are used for additional arguments like rdi and rsi. 
    So I'm hacking by adding all those. Todo: verify and test this assumption. */

  /* BIG TODO : veryify and test these assumptions in the MPS_ARCH_X86_64 stuff below!!!! */

#ifdef MPS_ARCH_X86_64 

  /* begin total hack that may or may not actually work!! */

  /* From Microsoft Visual studio 2005 documentations:

     The registers RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile and must be considered destroyed on function calls (unless otherwise safety-provable by analysis such as whole program optimization).

     The registers RBX, RBP, RDI, RSI, R12, R13, R14, and R15 are considered nonvolatile and must be saved and restored by a function that uses them.

     http://msdn.microsoft.com/en-us/library/6t169e9c(VS.80).aspx
  */

   /* Certain registers are callee-saved */
   /* and so may contain roots.  They are pushed */
   /* for scanning.  See .source.callees.saves. */

  ASMV("push %rax");  /* return value  */
  ASMV("push %rbx");  /* callee-saved */
  ASMV("push %rcx");  /* 4-th argument */

  ASMV("push %rdx");  /* 3-rd argument */
  ASMV("push %rsi");  /* 2-nd argument */
  ASMV("push %rdi");  /* 1-st argument */

  /*  ASMV("push %rbp"); */ /* callee-saved */
  /*  ASMV("push %rsp");  */ /* stack pointer */
  ASMV("push %r8");   /* 5-th argument */

  ASMV("push %r9");   /* 6-th argument */
  ASMV("push %r10");   /* callee-saved */
  ASMV("push %r11");   /* used for linking */
  
  ASMV("push %r12");  /* unused for C */
  ASMV("push %r13");  /* callee-saved */
  ASMV("push %r14");  /* callee-saved */
  ASMV("push %r15");  /* callee-saved */

  ASMV("mov %%rsp, %0" : "=r" (stackTop) :);    /* stackTop = rsp */

  AVER(AddrIsAligned((Addr)stackTop, sizeof(Addr)));  /* .assume.align */
  res = TraceScanArea(ss, stackTop, stackBot);

  ASMV("add $112, %rsp");  /* pop 14 (14*8== 112) regs to restore the stack pointer */
#endif

  return res;
}


/* C. COPYRIGHT AND LICENSE
 *
 * Copyright (C) 2001-2002 Ravenbrook Limited <http://www.ravenbrook.com/>.
 * All rights reserved.  This is an open source license.  Contact
 * Ravenbrook for commercial licensing options.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * 3. Redistributions in any form must be accompanied by information on how
 * to obtain complete source code for this software and any accompanying
 * software that uses this software.  The source code must either be
 * included in the distribution or be available for no more than the cost
 * of distribution plus a nominal fee, and must be freely redistributable
 * under reasonable conditions.  For an executable file, complete source
 * code means the source code for all modules it contains. It does not
 * include source code for modules or files that typically accompany the
 * major components of the operating system on which the executable file
 * runs.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
