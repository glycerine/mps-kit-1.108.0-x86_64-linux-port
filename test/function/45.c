/* 
TEST_HEADER
 id = $HopeName: MMQA_test_function!45.c(trunk.4) $
 summary = arena_collect when lots of APs are in mid-cycle
 language = c
 link = testlib.o newfmt.o
END_HEADER
*/

/*
 See MMQA_test_function!12.c, on which this test is closely based.
*/

#include "testlib.h"
#include "mpscamc.h"
#include "mpsavm.h"
#include "newfmt.h"


#define NCELLS 100
#define NAPS 100
#define PNULL (ranint(100)<25)
#define NUMREFS (ranint(80))
#define BLAH 0


#define genCOUNT (3)

static mps_gen_param_s testChain[genCOUNT] = {
  { 6000, 0.90 }, { 8000, 0.65 }, { 16000, 0.50 } };


void *stackpointer;

mps_ap_t ap[NAPS];
mycell *p[NAPS];
size_t s[NAPS];
int nrefs[NAPS];
int ap_state[NAPS];


static void test(void)
{
 mps_arena_t arena;
 mps_pool_t pool;
 mps_thr_t thread;
 mps_root_t root;

 mps_fmt_t format;
 mps_chain_t chain;

 mycell *cells;

 int h,i,j,k,l;
 mycell *pobj;

 mycell *ambig[NAPS];

 size_t size0, size1;
 long mdiff = 0;
 size_t bytes;
 size_t alignment;
 mps_addr_t q;
 int nextid = 0x1000000;

 /* turn on comments about copying and scanning */
 formatcomments = BLAH;
 fixcomments = BLAH;

 cdie(mps_arena_create(&arena, mps_arena_class_vm(), mmqaArenaSIZE),
      "create arena");

 die(mps_thread_reg(&thread, arena), "register thread");
 die(mps_root_create_reg(&root, arena, MPS_RANK_AMBIG, 0, thread,
                         mps_stack_scan_ambig, stackpointer, 0),
     "create root");

 die(mps_fmt_create_A(&format, arena, &fmtA), "create format");
 cdie(mps_chain_create(&chain, arena, genCOUNT, testChain), "chain_create");

 die(mmqa_pool_create_chain(&pool, arena, mps_class_amc(), format, chain),
     "create pool");

 for (i = 0; i < NAPS; i++) {
   die(mps_ap_create(&ap[i], pool, MPS_RANK_EXACT), "create ap");
   ap_state[i] = 0;
 }

 cells = allocone(ap[0], NCELLS);

 /* ap_state can have the following values:
  0 before reserve
  1 after reverse
  2 after init
  3 after I=A
  0 after commit
 */

 for(h=0; h<100; h++) {
   comment("%i of 100", h);

   for(j=0; j<1000; j++) {
     if (j == 500) {
       size0 = mps_arena_committed(arena);
       mps_arena_collect(arena);
       size1 = mps_arena_committed(arena);
       asserts(((long) size1)-((long) size0) < 1024*1024,
               "Collection made arena bigger: %lu -> %lu",
               (unsigned long) size0, (unsigned long) size1);
       if (((long) size1)-((long) size0) > mdiff) {
         mdiff = ((long) size1) - ((long) size0);
       }
     }
     i = ranint(NAPS);

     switch (ap_state[i]) {
     case 0:
       nrefs[i] = NUMREFS;
       bytes = offsetof(struct data, ref)+nrefs[i]*sizeof(struct refitem);
       alignment = MPS_PF_ALIGN;
       bytes = (bytes+alignment-1)&~(alignment-1);
       s[i] = bytes;
       die(mps_reserve(&q, ap[i], s[i]), "reserve: ");
       p[i] = q;
       p[i]->data.tag = 0xD033E2A6;
       p[i]->data.id = nextid;
       ap_state[i] = 1;
       commentif(BLAH, "%i: reserve %li at %p", i, nextid, q);
       nextid +=1;
       break;
     case 1:
       commentif(BLAH, "%i: init %li", i, p[i]->data.id);
       p[i]->data.tag = MCdata;
       p[i]->data.numrefs = nrefs[i];
       p[i]->data.size = s[i];
       ap_state[i] = 2;
       for (k=0; k<nrefs[i]; k++) {
         if PNULL {
           p[i]->data.ref[k].addr = NULL;
           p[i]->data.ref[k].id   = 0;
         } else {
           l = ranint(NCELLS);
           pobj = getref(cells, l);
           p[i]->data.ref[k].addr = pobj;
           p[i]->data.ref[k].id = (pobj==NULL ? 0 : pobj->data.id);
         }
         commentif(BLAH, "    ref %i -> %li", k, p[i]->data.ref[k].id);
       }
       break;
     case 2:
       commentif(BLAH, "%i: begin commit %li", i, p[i]->data.id);
       ambig[i] = p[i];
       ap[i]->init = ap[i]->alloc;
       ap_state[i] = 3;
       break;
     case 3:
       commentif(BLAH, "%i: end commit %li", i, p[i]->data.id);
       q=p[i];
       if (ap[i]->limit != 0 || mps_ap_trip(ap[i], p[i], s[i])) {
         l = ranint(NCELLS);
         setref(cells, l, q);
         commentif(BLAH, "%i -> %i", i, l);
       }
       ap_state[i] = 0;
       ambig[i] = NULL;
       break;
     }
   }
   checkfrom(cells);
 }

 comment("Maximum size increase:");
 report("mincr", "%ld", mdiff);

 comment("Finished main loop");

 for (i=0; i<NAPS; i++) {
  switch (ap_state[i]) {
   case 1:
    commentif(BLAH, "%i init", i);
    p[i]->data.tag = MCdata;
    p[i]->data.numrefs = 0;
    p[i]->data.size = s[i];
   case 2:
    commentif(BLAH, "%i begin commit", i);
    ap[i]->init = ap[i]->alloc;
   case 3:
    commentif(BLAH, "% end commit", i);
    (void) (ap[i]->limit != 0 || mps_ap_trip(ap[i], p[i], s[i]));
  }
  mps_ap_destroy(ap[i]);
 }

 mps_pool_destroy(pool);
 mps_chain_destroy(chain);
 mps_fmt_destroy(format);
 mps_root_destroy(root);
 mps_thread_dereg(thread);
 mps_arena_destroy(arena);
 comment("Destroyed arena.");
}


int main(void)
{
 void *m;
 stackpointer=&m; /* hack to get stack pointer */

 easy_tramp(test);
 pass();
 return 0;
}
