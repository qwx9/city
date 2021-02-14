#include <u.h>
#include <libc.h>
#include "dat.h"
#include "fns.h"

int mapwidth, mapheight;
Tile *map;

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
