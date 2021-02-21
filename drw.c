#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

QLock drwlock;

static char *fontname = "/lib/font/bit/fixed/unicode.6x10.font";

static Rectangle fbr;
static Image *fb;

static Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

void
drawtile(Tile *m)
{
	char *s;
	Point p;

	p.x = (m - map) % mapwidth * 16 * font->width;
	p.y = (m - map) / mapwidth * 2 * font->height;
	s = m->b != nil ? m->b->name : m->t->name;
	string(fb, p, display->white, ZP, font, s);
}

void
updatedraw(void)
{
	int bail;
	Tile *m;

	bail = 1;
	qlock(&drwlock);
	for(m=map; m<map+mapwidth*mapheight; m++)
		if(m->stale){
			bail = 0;
			drawtile(m);
			m->stale = 0;
		}
	qunlock(&drwlock);
	if(bail)
		return;
	draw(screen, screen->r, fb, nil, ZP);
	flushimage(display, 1);
}

void
redraw(void)
{
	Tile *m;

	draw(fb, fb->r, display->black, nil, ZP);
	for(m=map; m<map+mapwidth*mapheight; m++)
		m->stale = 1;
	updatedraw();
}

void
resetdraw(void)
{
	freeimage(fb);
	fbr = rectsubpt(screen->r, screen->r.min);
	fb = eallocimage(fbr, screen->chan, 0, DNofill);
	redraw();
}

void
initdrw(void)
{
	if(initdraw(nil, fontname, "city") < 0)
		sysfatal("initdraw: %r");
	resetdraw();
}
