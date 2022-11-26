#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

Mousectl *mc;
Keyboardctl *kc;

static int tdiv;
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

int
max(int a, int b)
{
	return a > b ? a : b;
}

int
min(int a, int b)
{
	return a < b ? a : b;
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
	uint b, bo;
	Rune r;
	Mouse mo;
	Point p;

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
	readfs();
	resetdraw();
	startsim();
	mo.xy = EP;
	memset(&mo, 0, sizeof mo);
	bo = 0;
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
		/* FIXME: clean up */
		case Amouse:
			if(eqpt(mo.xy, EP))
				mo = mc->Mouse;
			b = mc->buttons;
			p = s2p(mc->xy);
			if(eqpt(p, EP) || b == 0){
				//if(b != 0)
				//	deselect();
				mo = mc->Mouse;
				bo = b;
				break;
			}
			if((b & (1<<0|1<<2)) != 0
			&& (b & (1<<0|1<<2) != (bo & (1<<0|1<<2)))){
				mouseselect(p);
				updatedraw(1);
			}
			if((mc->buttons & 1<<2) != 0
			&& (b & 1<<2) != (bo & 1<<2)){
				actionmenu(p);
				updatedraw(1);
			}
			mo = mc->Mouse;
			bo = mo.buttons;
			break;
		case Akbd:
			switch(r){
			case Kdel:
			case 'q': threadexitsall(nil);
			case ' ': paused ^= 1; break;
			case '+':
			case '=':
				if(scale < 16){
					scale++;
					resetdraw();
				}
				break;
			case '-':
				if(scale > 1){
					scale--;
					resetdraw();
				}
				break;
			}
			break;
		case Aanim:
			updatedraw(0);
			break;
		}
	}
}
