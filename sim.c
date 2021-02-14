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
upkeep(void)
{
	Tile **o;

	if(clock % UpkeepΔt != 0 || clock == 0)
		return;
	for(o=objhead-1; o>=objs; o--){
		/* if upkeep can't be kept, stop production immediately
		 * else decrease stock
		 */
	}
}

static void
updateobj(void)
{
/*
	iterate through obj stack

	build
		call for supplies, start travel towards building site
		done?
		start production
	upgrade
		finished?
		start production
	destroy
		finished?
		despawn
	produce
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
