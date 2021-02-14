#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

int tdiv;
vlong clock;

void *
emalloc(ulong n)
{
	void *p;

	if((p = mallocz(n, 1)) == nil)
		sysfatal("emalloc: %r");
	setmalloctag(p, getcallerpc(&n));
	return p;
}

void
input(void)
{

}

void
main(int argc, char **argv)
{
	vlong t, t0, dt, Δtc;

	ARGBEGIN{
	}ARGEND
	srand(time(nil));
	init();
	tdiv = Te9 / THz;
	t0 = nsec();
	for(;;){
		input();
		step();
		clock++;
		t = nsec();
		Δtc = (t - t0) / tdiv;
		if(Δtc <= 0)
			Δtc = 1;
		t0 += Δtc * tdiv;
		dt = (t0 - t) / Te6;
		if(dt > 0)
			sleep(dt);
	}
}
