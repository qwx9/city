#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

int mapwidth, mapheight;
Tile *map;

int
mhdist(int x, int y, int x´, int y´)
{
	return abs(x - x´) + abs(y - y´);
}

void
loadmap(void)
{
	Tile *m;
	Terrain *t;

	for(m=map; m<map+mapwidth*mapheight; m++){
		t = terrains + nrand(Ttot);
		m->t = t;
		m->state = Snull;
		m->stock = t->initialstock;
	}
}

void
initmap(void)
{
	mapwidth = 16;
	mapheight = 16;
	map = emalloc(mapwidth * mapheight * sizeof *map);
	loadmap();
}
