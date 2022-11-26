#include <u.h>
#include <libc.h>
#include <thread.h>
#include <draw.h>
#include <mouse.h>
#include <keyboard.h>
#include <cursor.h>
#include "dat.h"
#include "fns.h"

extern Mousectl *mc;
extern Cursor targetcursor;

/* FIXME: safer: Menu struct with menu state: default, build, etc. */

Tile *selected;
int (*selectfn)(Tile *);
Menuptr menufn;

typedef struct Gmenu Gmenu;
struct Gmenu{
	char *name;
	int (*checkfn)(Tile*);
	Menuptr (*initfn)(Tile*);
	int (*selectfn)(Tile*);
};
enum{
	Gmenubuild,
	Gmenuend,
};
static Gmenu actions[Gmenuend] = {
	[Gmenubuild] {"build", isbuildable, buildmenu},
};
static Gmenu *favail[nelem(actions)];
static int navail;

extern Rectangle scrwin;

void
gsetcursor(int curs)
{
	Cursor *c;

	switch(curs){
	case Curstarget: c = &targetcursor; break;
	case Cursdef: /* wet floor */
	default: c = nil; break;
	}
	setcursor(mc, c);
}

static char *
genmenu(int n)
{
	return n < navail ? favail[n]->name : nil;
}

static Menu menu = {0, genmenu};

static Menuptr
setmenu(Tile *t)
{
	int n;
	Gmenu *f;

	memset(favail, 0, sizeof favail);
	navail = 0;
	for(f=actions; f<actions+nelem(actions); f++)
		if(f->checkfn == nil || f->checkfn(t))
			favail[navail++] = f;
	if(navail == 0)
		return nil;
	if((n = menuhit(3, mc, &menu, nil)) < 0)
		return nil;
	return favail[n]->initfn(t);
}

void
actionmenu(Point p)
{
	Tile *t;

	t = p2t(p);
	assert(t != nil);
	selected = t;
	if(menufn == nil){
		menufn = setmenu(t);
		if(menufn == nil)
			deselect();
	}else if(menufn(t) < 0)
		deselect();
}

void
mouseselect(Point p)
{
	selected = p2t(p);
	if(selectfn != nil && selectfn(selected) < 0)
		deselect();
}

void
deselect(void)
{
	selected = nil;
	menufn = nil;
	mapdrawfn = nil;
	selectfn = nil;
	gsetcursor(Cursdef);
	repaint = 1;
}
