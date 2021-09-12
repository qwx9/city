#include <u.h>
#include <libc.h>
#include <thread.h>
#include "dat.h"
#include "fns.h"

extern QLock drwlock;

int paused;
vlong clock;

static int tdiv;

static Tile **objs, **objhead;
static int maxobj;

enum{
	UpkeepΔt = 150,
};
static int stock[Gtot], rstock[Rtot];

static void
spawn(Tile *m, int n)
{
	Tile **o;

	if(objhead - objs >= maxobj)
		sysfatal("spawn: out of bounds");
	m->t = terrains + Tplain;
	m->b = buildings + n;
	for(o=objhead-1; o>=objs; o--)
		if((*o)->b - buildings <= n)
			break;
	/* push in front of lower priority object */
	if(++o < objhead)
		memmove(o+1, o, (objhead - o) * sizeof *o);
	*o = m;
	objhead++;
}

static void
starve(Tile *o)
{
	o->state = Svoid;
}

static void
upkeep(void)
{
	int n, r, g, upkeep[nelem(resources)];
	Tile *o, **ol;

	if(clock % UpkeepΔt != 0 || clock == 0)
		return;
	for(ol=objhead-1; ol>=objs; ol--){
		o = *ol;
		if(o->state <= Svoid)
			continue;
		if(o->b == nil)
			sysfatal("empty active tile");
		memcpy(upkeep, o->b->upkeep, sizeof upkeep);
		for(r=0; r<nelem(resources); r++)
			if(upkeep[r] > rstock[r]){
				starve(o);
				goto bail;
			}
		for(g=0; g<nelem(goods); g++){
			r = goods[g].resource;
			if(upkeep[r] <= 0)
				continue;
			n = min(stock[g], upkeep[r]);
			stock[g] -= n;
			rstock[r] -= n;
			upkeep[r] -= n;
		}
		for(r=0; r<nelem(upkeep); r++)
			if(upkeep[r] > 0)
				sysfatal("upkeep: goods/resources phase error");
	bail:;
	}
}

static int
trypickup(Tile *o)
{
	int g;

	if(o->pickupΔt == 0 || o->clock < o->pickupΔt)
		return 0;
	for(g=0; g<nelem(goods); g++)
		stock[g] += o->prodstock[g];
	o->pickupΔt = 0;
	return 1;
}

static int
trysupply(Tile *o)
{
	int g;
	for(g=0; g<nelem(goods); g++)
		if(o->b->prodcost[g] > stock[g])
			return 0;
	for(g=0; g<nelem(goods); g++)
		if(o->b->prodcost[g] > 0)
			stock[g] -= o->b->prodcost[g];
	o->supplyΔt = o->clock + o->distance;
	return 1;
}

static void
updateobj(void)
{
	int g;
	Tile *o, **ol;

	for(ol=objhead-1; ol>=objs; ol--){
		o = *ol;
		o->clock++;
		switch(o->state){
		case Swaitbuild:
			if(o->clock >= o->distance){
				o->clock = 0;
				o->state = Sbuild;
			}
			break;
		case Sbuild:
			if(o->clock >= o->b->buildtime){
				o->clock = 0;
				o->state = o->b->prodtime > 0 ? Sstarved : Svoid;
			}
			break;
		case Sstarved:
			trypickup(o);
			if(trysupply(o))
				o->state = Swaitsupply;
			break;
		case Swaitsupply:
			trypickup(o);
			if(o->clock >= o->supplyΔt){
				o->gotsupply = 0;
				o->supplyΔt = 0;
				o->prodΔt = o->clock + o->b->prodtime;
				o->state = Sproduce;
			}
			break;
		case Sproduce:
			if(!o->gotsupply){
				if(o->supplyΔt == 0){
					if(!trysupply(o))
						return;
					for(g=0; g<nelem(goods); g++)
						if(o->b->prodcost[g] > stock[g])
							break;
				}else if(o->clock >= o->supplyΔt){
					o->gotsupply = 1;
					o->supplyΔt = 0;
				}
			}
			trypickup(o);
			if(o->clock >= o->prodΔt){
				o->pickupΔt = o->clock + o->distance * 2;
				if(!o->gotsupply){
					if(o->supplyΔt == 0)
						o->state = Sstarved;
					else
						o->state = Swaitsupply;
				}else{
					o->prodΔt += o->b->prodtime;
					o->gotsupply = 0;
				}
			}
			break;
		default: break;
		}
	}
}

static void
step(void)
{
	if(paused)
		return;
	qlock(&drwlock);
	upkeep();
	updateobj();
	qunlock(&drwlock);
}

static void
simproc(void *)
{
	int Δtc;
	vlong t, t0, dt;

	tdiv = Te9 / SimHz;
	t0 = nsec();
	for(;;){
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

void
startsim(void)
{
	if(proccreate(simproc, nil, 8192) < 0)
		sysfatal("init: %r");
}

static void
calcdists(int n)
{
	int x, y, x´, y´;
	Tile *o;

	x = n % mapwidth;
	y = n / mapheight;
	for(o=map, x´=0, y´=0; o<map+mapwidth*mapheight; o++, x´++){
		o->distance = mhdist(x, y, x´, y´);
		if(x´ == mapwidth){
			x´ = 0;
			y´++;
		}
	}
}

void
init(void)
{
	int i, n;

	srand(time(nil));
	initmap();
	maxobj = mapwidth * mapheight;
	objs = emalloc(maxobj * sizeof *objs);
	objhead = objs;
	n = nrand(maxobj);
	spawn(map + n, Btownhall);
	calcdists(n);
	for(i=0; i<nelem(initialstock); i++){
		stock[i] = initialstock[i];
		rstock[goods[i].resource] += stock[i];
	}
}
