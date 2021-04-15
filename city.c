#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

static int tdiv;
static Keyboardctl *kc;
static Mousectl *mc;
static Channel *tmc;

void *
emalloc(ulong n)
{
	void *p;

	if((p = mallocz(n, 1)) == nil)
		sysfatal("emalloc: %r");
	setmalloctag(p, getcallerpc(&n));
	return p;
}

static void
timeproc(void *)
{
	tdiv = 1000 / AnimHz;
	for(;;){
		sleep(tdiv);
		nbsendul(tmc, 0);
	}
}

void
threadmain(int argc, char **argv)
{
	Rune r;
	Mouse mo;

	ARGBEGIN{
	}ARGEND
	init();
	initdrw();
	if((kc = initkeyboard(nil)) == nil)
		sysfatal("initkeyboard: %r");
	if((mc = initmouse(nil, screen)) == nil)
		sysfatal("initmouse: %r");
	if((tmc = chancreate(sizeof(ulong), 0)) == nil)
		sysfatal("chancreate: %r");
	if(proccreate(timeproc, nil, 8192) < 0)
		sysfatal("init: %r");
	startsim();
	mo.xy = ZP;
	enum{
		Aresize,
		Amouse,
		Akbd,
		Aanim,
	};
	Alt a[] = {
		{mc->resizec, nil, CHANRCV},
		{mc->c, &mc->Mouse, CHANRCV},
		{kc->c, &r, CHANRCV},
		{tmc, nil, CHANRCV},
		{nil, nil, CHANEND}
	};
	for(;;){
		switch(alt(a)){
		case Aresize:
			if(getwindow(display, Refnone) < 0)
				sysfatal("resize failed: %r");
			mo = mc->Mouse;
			resetdraw();
			break;
		case Amouse:
			if(eqpt(mo.xy, ZP))
				mo = mc->Mouse;
			break;
		case Akbd:
			switch(r){
			case Kdel:
			case 'q': threadexitsall(nil);
			case ' ': paused ^= 1; break;
			}
			break;
		case Aanim:
			updatedraw();
			break;
		}
	}
}
