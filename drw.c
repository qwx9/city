#include <u.h>
#include <libc.h>
#include <draw.h>
#include "dat.h"
#include "fns.h"

QLock drwlock;

static char *fontname = "/lib/font/bit/fixed/unicode.6x10.font";

static Tile *selected;

enum{
	Cbg,
	Cterrain,
	Cbuild,
	Ctext,
	Cend,
};
static Image *cols[Cend];

static Rectangle fbr, drwwin, hudr;
static Image *fb;
static Point tlsize, tlofs, tlwindow;
static int doupdate;

static Image *
eallocimage(Rectangle r, ulong chan, int repl, ulong col)
{
	Image *i;

	if((i = allocimage(display, r, chan, repl, col)) == nil)
		sysfatal("allocimage: %r");
	return i;
}

static Point
scr2tilexy(Point p)
{
	p = subpt(p, drwwin.min);
	p = Pt(p.x / (tlsize.x + 4), p.y / (tlsize.y + 4));
	assert(p.x < tlwindow.x && p.x >= 0 && p.y < tlwindow.y && p.y >= 0);
	return p;
}

static Tile *
scr2tile(Point p)
{
	p = scr2tilexy(p);
	return map + p.y * mapwidth + p.x;
}

static void
drawhud(void)
{
	char *name;

	if(selected == nil)
		return;
	name = selected->b != nil ? selected->b->name : selected->t->name;
	draw(screen, hudr, cols[Cbg], nil, ZP);
	string(screen, hudr.min, cols[Ctext], ZP, font, name);
}

static void
drawtile(Tile *m)
{
	int hasbuild;
	char *s;
	Point p;

	p.x = ((m - map) % mapwidth) * (tlsize.x + 4);
	p.y = ((m - map) / mapwidth) * (tlsize.y + 4);
	hasbuild = m->b != nil;
	s = hasbuild ? m->b->abbr : m->t->abbr;
	string(fb, p, hasbuild ? cols[Cbuild] : cols[Cterrain], ZP, font, s);
}

void
updatedraw(void)
{
	int x, y;
	Tile *m;

	qlock(&drwlock);
	for(y=0, m=map; y<tlwindow.y; y++){
		for(x=0; x<tlwindow.x; x++, m++)
			if(m->stale){
				drawtile(m);
				m->stale = 0;
				doupdate = 1;
			}
		m += mapwidth - tlwindow.x;
	}
	qunlock(&drwlock);
	if(!doupdate)
		return;
	draw(screen, screen->r, fb, nil, tlofs);
	drawhud();
	flushimage(display, 1);
	doupdate = 0;
}

void
mouseselect(Point p)
{
	doupdate = 1;
	if(!ptinrect(p, drwwin)){
		selected = nil;
		return;
	}
	selected = scr2tile(p);
	doupdate = 1;
	updatedraw();
}

static void
redraw(void)
{
	int x, y;
	Tile *m;

	draw(fb, fb->r, display->black, nil, ZP);
	for(y=0, m=map; y<tlwindow.y; y++){
		for(x=0; x<tlwindow.x; x++, m++)
			m->stale = 1;
		m += mapwidth - tlwindow.x;
	}
	updatedraw();
}

void
resetdraw(void)
{
	int w, h;

	w = Dx(screen->r);
	h = Dy(screen->r);
	fbr.min = ZP;
	/* for fuck's sake */
	fbr.max.x = min(w, tlsize.x * mapwidth + (mapwidth - 1) * 4);
	fbr.max.y = min(h, tlsize.y * mapheight + (mapheight - 1) * 4 + abs(font->height - font->width));
	tlofs.x = w > fbr.max.x ? (fbr.max.x - w) / 2 : 0;
	tlofs.y = h > fbr.max.y ? (fbr.max.y - h) / 2 : 0;
	tlwindow.x = w > fbr.max.x ? mapwidth : min(mapwidth, fbr.max.x / (tlsize.x + 4) + 1);
	tlwindow.y = h > fbr.max.y ? mapheight : min(mapheight, fbr.max.y / (tlsize.y + 4) + 1);
	drwwin.min = subpt(screen->r.min, tlofs);
	drwwin.max = addpt(drwwin.min, fbr.max);
	hudr.min = addpt(drwwin.max, Pt(-fbr.max.x - 16, 16));
	hudr.max = Pt(screen->r.max.x - 16, hudr.min.y + font->height);
	freeimage(fb);
	fb = eallocimage(fbr, screen->chan, 0, DNofill);
	if(!eqpt(tlofs, ZP))
		draw(screen, screen->r, cols[Cbg], nil, ZP);
	doupdate = 1;
	redraw();
}

void
initdrw(void)
{
	if(initdraw(nil, fontname, "city") < 0)
		sysfatal("initdraw: %r");
	cols[Cbg] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x777777FF);
	cols[Cterrain] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x999999FF);
	cols[Cbuild] = eallocimage(Rect(0,0,1,1), screen->chan, 1, 0x9B6917FF);
	cols[Ctext] = display->black;
	tlsize = stringsize(font, "0");
	if(tlsize.x < tlsize.y)
		tlsize.y = tlsize.x;
	else
		tlsize.x = tlsize.y;
	fmtinstall('P', Pfmt);
	fmtinstall('R', Rfmt);
	resetdraw();
}
