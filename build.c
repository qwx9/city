#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include "dat.h"
#include "fns.h"

extern Mousectl *mc;

static Tile *site;

int
couldbuild(Building *b)
{
	int i;

	for(i=0; i<nelem(b->buildcost); i++)
		if(b->buildcost[i] > stock[i])
			return 0;
	return 1;
}

int
isbuildable(Tile *t)
{
	return t->b == nil && t->t == terrains + Tplain;
}

static char *
genmenu(int n)
{
	switch(n){
	case 0: return "build";
	default: return nil;
	}
}

static Menu menu = {0, genmenu};

static int
select(Tile *t)
{
	Building *b;

	b = buildings + (t - map);
	return b >= buildings + nelem(buildings) ? -1 : 0;
}

static int
action(Tile *t)
{
	int i;
	Building *b;

	b = buildings + (t - map);
	if(b >= buildings + nelem(buildings))
		return -1;
	selected = t;
	if(menuhit(3, mc, &menu, nil) < 0)
		return -1;
	/* FIXME: show some error message in status bar */
	if(!couldbuild(b))
		return 0;
	for(i=0; i<nelem(b->buildcost); i++){
		stock[i] -= b->buildcost[i];
		assert(stock[i] >= 0);
	}
	spawn(site, b - buildings);
	deselect();
	return 0;
}

Menuptr
buildmenu(Tile *t)
{
	site = t;
	selected = nil;
	gsetcursor(Curstarget);
	mapdrawfn = drawbuildmenu;
	selectfn = select;
	return action;
}
