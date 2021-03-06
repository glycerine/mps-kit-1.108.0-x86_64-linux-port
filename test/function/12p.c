/* 
TEST_HEADER
 id = $HopeName$
 summary = lots of APs with interleaved reserve and 2-stage commit
 language = c
 link = testlib.o newfmt.o
END_HEADER
*/

/*
   This test needs some explanation. The object 'cells' contains
   NCELL references to other objects. NAPS allocation points
   are created. At each step, we choose one and nudge it on
   a little. Each allocation point goes through the following
   cycle of steps: reserve, init, begin commit, end commit.
   At the init step, each reference in the reserved object is
   set to point randomly to one of the objects referenced by
   cells or (with probability PNULL) to NULL. If commit fails,
   the object is thrown away. If it succeeds, it is written into
   a random reference in cells. Repeat.

   Because it has to deal directly with reserve and commit,
   this test can't use the nice functions provided by newfmt
   all the time.
*/

#include "testlib.h"
#include "mpscamc.h"
#include "newfmt.h"

void *stackpointer;

#define NCELLS 100
#define NAPS 100
#define PNULL (ranint(100)<25)
#define NUMREFS (ranint(20))
#define BLAH 0

mps_ap_t ap[NAPS];
size_t s[NAPS];
int nrefs[NAPS];
int ap_state[NAPS];

static void test(void)
{
 mps_space_t space;
 mps_pool_t pool;
 mps_thr_t thread;
 mps_root_t root;

 mps_fmt_t format;
 mycell *cells;

 int h,i,j,k,l;
 mycell *p[NAPS];
 mycell *pobj;
 size_t bytes;
 size_t alignment;
 mps_addr_t q;
 int nextid = 0x1000000;

/* turn on comments about copying and scanning */
 formatcomments = BLAH;
 fixcomments = BLAH;

 cdie(mps_space_create(&space), "create space");

 cdie(mps_thread_reg(&thread, space), "register thread");

 cdie(
  mps_root_create_reg(&root, space, MPS_RANK_AMBIG, 0, thread,
   mps_stack_scan_ambig, stackpointer, 0),
  "create root");

 cdie(
  mps_fmt_create_A(&format, space, &fmtA),
  "create format");

 cdie(
  mps_pool_create(&pool, space, mps_class_amc(), format),
  "create pool");

 for (i=0; i<NAPS; i++)
 {
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

 for(h=0; h<100; h++)
 {
 comment("%i of 100", h);
 comment("%i collections", (int) mps_collections(space));

 for(j=0; j<1000; j++)
 {
 i = ranint(NAPS);

 switch (ap_state[i])
 {
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
   for (k=0; k<nrefs[i]; k++)
   {
    if PNULL
    { p[i]->data.ref[k].addr = NULL;
      p[i]->data.ref[k].id   = 0;
    }
    else
    {
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
   ap[i]->init = ap[i]->alloc;
   ap_state[i] = 3;
   break;
  case 3: case 4: case 5: case 6: case 7: case 8: case 9:
   ap_state[i]+=1;
   break; 
  case 10:
   commentif(BLAH, "%i: end commit %li", i, p[i]->data.id);
   q=p[i];
   if (ap[i]->limit != 0 || mps_ap_trip(ap[i], p[i], s[i]))
   {
    l = ranint(NCELLS);
    setref(cells, l, q);
    commentif(BLAH, "%i -> %i", i, l);
   }
   ap_state[i] = 0;
   break;
 }
 }
 checkfrom(cells);
 }

 comment("Finished main loop");

 for (i=0; i<NAPS; i++)
 {
  switch (ap_state[i])
  {
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
 comment("Destroyed pool.");

 mps_fmt_destroy(format);
 comment("Destroyed format.");

 mps_root_destroy(root);
 comment("Destroyed root.");

 mps_thread_dereg(thread);
 comment("Deregistered thread.");

 mps_space_destroy(space);
 comment("Destroyed space.");
}

int main(void)
{
 void *m;
 stackpointer=&m; /* hack to get stack pointer */

 easy_tramp(test);
 pass();
 return 0;
}
