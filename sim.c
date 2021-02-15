#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

static Tile **objs, **objhead;
static int maxobj;

enum{
	UpkeepΔt = 150,
};
static int stock[Gtot], rstock[Rtot];

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
	o->state = Swait;
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
		if(o->state <= Swait)
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

static void
updateobj(void)
{
	Tile *o, **ol;

	for(ol=objhead-1; ol>=objs; ol--){
		o = *ol;
		switch(o->state){
		case Sbuild:
			/* 
			call for supplies, start travel towards building site
			done?
			start production
			*/
		case Sproduce:
			/*
			call for supplies
			enough supplies? else wait until next tick (loop)
			decrement stocks, start traveling from townhall
			reached building?
			increment supplies
			begin production
			while producing, if we can call for supplies earlier to restart immediately, do it
			finished producing?
			start travel towards building
			reached building?
			call for pickup
			loop
			*/
		default: break;
		}
	}
}

void
step(void)
{
	upkeep();
	updateobj();
}

void
init(void)
{
	int i;

	initmap();
	maxobj = mapwidth * mapheight;
	objs = emalloc(maxobj * sizeof *objs);
	objhead = objs;
	spawn(map + nrand(maxobj), Btownhall);
	for(i=0; i<nelem(initialstock); i++){
		stock[i] = initialstock[i];
		rstock[goods[i].resource] += stock[i];
	}
}
