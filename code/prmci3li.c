/* prmci3li.c: PROTECTION MUTATOR CONTEXT INTEL 386 (LINUX)
 *
 * $Id: //info.ravenbrook.com/project/mps/version/1.108/code/prmci3li.c#1 $
 * Copyright (c) 2001 Ravenbrook Limited.  See end of file for license.
 *
 * .purpose: This module implements the part of the protection module
 * that decodes the MutatorFaultContext. 
 *
 *
 * SOURCES
 *
 * .source.i486: Intel486 Microprocessor Family Programmer's
 * Reference Manual
 *
 * .source.linux.kernel: Linux kernel source files.
 *
 *
 * ASSUMPTIONS
 *
 * .assume.regref: The resisters in the context can be modified by
 * storing into an MRef pointer.
 */

/* prmcli.h will include mpm.h after defining open sesame magic */
#include "prmcli.h"
#include "prmci3.h"

SRCID(prmci3li, "$Id: //info.ravenbrook.com/project/mps/version/1.108/code/prmci3li.c#1 $");


/* Prmci3AddressHoldingReg -- return an address of a register in a context */
#if 0 /* non-64-bit code */
MRef Prmci3AddressHoldingReg(MutatorFaultContext context, unsigned int regnum)
{
  struct sigcontext *scp;

  AVER(regnum <= 7);
  AVER(regnum >= 0);

  scp = context->scp;

  /* .source.i486 */
  /* .assume.regref */
  switch (regnum) {
  case 0: return (MRef)&scp->eax;
  case 1: return (MRef)&scp->ecx;
  case 2: return (MRef)&scp->edx;
  case 3: return (MRef)&scp->ebx;
  case 4: return (MRef)&scp->esp;
  case 5: return (MRef)&scp->ebp;
  case 6: return (MRef)&scp->esi;
  case 7: return (MRef)&scp->edi;
  }
  NOTREACHED;
  return (MRef)NULL;  /* Keep compiler happy. */
}
#endif /* 0 non-64-bit code */


/* begin 64-bit linux gcc code -- see /usr/include/bits/sigcontext.h */
MRef Prmci3AddressHoldingReg(MutatorFaultContext context, unsigned int regnum)
{
  struct sigcontext *scp;

  AVER(regnum <= 7);
  AVER(regnum >= 0);

  scp = context->scp;

  switch (regnum) {
  case 0: return (MRef)&scp->rax;
  case 1: return (MRef)&scp->rcx;
  case 2: return (MRef)&scp->rdx;
  case 3: return (MRef)&scp->rbx;
  case 4: return (MRef)&scp->rsp;
  case 5: return (MRef)&scp->rbp;
  case 6: return (MRef)&scp->rsi;
  case 7: return (MRef)&scp->rdi;

    /* todo: should the rest of the registered be added? - from /usr/include/bits/sigcontext.h , when __WORDSIZE == 64

struct sigcontext
{
  unsigned long r8;
  unsigned long r9;
  unsigned long r10;
  unsigned long r11;
  unsigned long r12;
  unsigned long r13;
  unsigned long r14;
  unsigned long r15;
  unsigned long rdi;
  unsigned long rsi;
  unsigned long rbp;
  unsigned long rbx;
  unsigned long rdx;
  unsigned long rax;
  unsigned long rcx;
  unsigned long rsp;
  unsigned long rip;
  unsigned long eflags;
  unsigned short cs;
  unsigned short gs;
  unsigned short fs;
  unsigned short __pad0;
  unsigned long err;
  unsigned long trapno;
  unsigned long oldmask;
  unsigned long cr2;
  struct _fpstate * fpstate;
  unsigned long __reserved1 [8];
};

     */

  }
  NOTREACHED;
  return (MRef)NULL;  /* Keep compiler happy. */
}


/* Prmci3DecodeFaultContext -- decode fault to find faulting address and IP */

void Prmci3DecodeFaultContext(MRef *faultmemReturn,
                              Byte **insvecReturn,
                              MutatorFaultContext context)
{
  struct sigcontext *scp;

  scp = context->scp;

  /* Assert that this is a page fault exception. The computation of */
  /* faultmem depends on this.  See .source.i486 (9.9.14). */
  AVER(scp->trapno == 14);

  /* cr2 contains the address which caused the fault. */
  /* See .source.i486 (9.9.14) and */
  /* .source.linux.kernel (linux/arch/i386/mm/fault.c). */
  *faultmemReturn = (MRef)scp->cr2;

#ifdef MPS_ARCH_I4
  *insvecReturn = (Byte*)scp->eip;
#endif

#ifdef MPS_ARCH_X86_64
  *insvecReturn = (Byte*)scp->rip;
#endif

}


/* Prmci3StepOverIns -- modify context to step over instruction */

void Prmci3StepOverIns(MutatorFaultContext context, Size inslen)
{
#ifdef MPS_ARCH_I4
  context->scp->eip += (unsigned long)inslen;
#endif

#ifdef MPS_ARCH_X86_64
  context->scp->rip += (unsigned long)inslen;
#endif
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
